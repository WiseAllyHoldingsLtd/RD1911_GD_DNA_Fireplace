// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// based on chained tlsio adapter tlsio_mbed.c from Microsoft azure c-utility
// experimentation Anton/GDN
// rewriting this as a direct tlsio adapter using mbed and TLSSocket
// heavily inspired by tlsio_openssl_compact

#include <stdlib.h>
#include "esp_log.h"

#include "mbedtls/config.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
#include "mbedtls/entropy_poll.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/tlsio.h"
#include "azure_c_shared_utility/tlsio_mbedtls.h"
//#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "azure_c_shared_utility/singlylinkedlist.h"

#include "TLSSocket.h"
#include "tlsio_pal.h"

#include "certs.h"

#include "azure_c_shared_utility/tlsio_options.h"

#define OPTION_UNDERLYING_IO_OPTIONS        "underlying_io_options"
#define MAX_HOSTNAME_SIZE 256

namespace
{
  char LOG_TAG[] = "tlsio_mbedtls_tlssocket";
  const size_t CERT_SIZE_MAX { 2048 };
}

static int tlsio_mbed_send_async(CONCRETE_IO_HANDLE tls_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context);


typedef struct
{
    unsigned char* bytes;
    size_t size;
    size_t unsent_size;
    ON_SEND_COMPLETE on_send_complete;
    void* callback_context;
} PENDING_TRANSMISSION;

enum TLSIO_STATE_ENUM
{
    TLSIO_STATE_CLOSED,
    TLSIO_STATE_OPEN,
    TLSIO_STATE_ERROR
};

struct TlsIoInstance
{
  TlsIoInstance(const char *hostname, uint16_t remotePort):
    on_bytes_received(nullptr),
    on_io_open_complete(nullptr),
    on_io_close_complete(nullptr),
    on_io_error(nullptr),
    on_bytes_received_context(nullptr),
    on_io_open_complete_context(nullptr),
    on_io_close_complete_context(nullptr),
    on_io_error_context(nullptr),
    tlsio_state(TLSIO_STATE_CLOSED),
    port(remotePort),
    pending_transmission_list(nullptr)
  {
       strncpy_s(hostnameSetting,
                 sizeof(hostnameSetting),
                 hostname,
                 sizeof(hostnameSetting)-1);
  }

  ON_BYTES_RECEIVED on_bytes_received;
  ON_IO_OPEN_COMPLETE on_io_open_complete;
  ON_IO_CLOSE_COMPLETE on_io_close_complete;
  ON_IO_ERROR on_io_error;
  void* on_bytes_received_context;
  void* on_io_open_complete_context;
  void* on_io_close_complete_context;
  void* on_io_error_context;
  TLSIO_STATE_ENUM tlsio_state;
  unsigned char* socket_io_read_bytes;
  size_t socket_io_read_byte_count;
  ON_SEND_COMPLETE on_send_complete;
  void* on_send_complete_callback_context;
  char hostnameSetting[MAX_HOSTNAME_SIZE];
  uint16_t port;
  SINGLYLINKEDLIST_HANDLE pending_transmission_list; // for async send
  char tls_clientcert[CERT_SIZE_MAX];
  TLSSocket tlsSocket;
  TLSIO_OPTIONS options;
};

static const IO_INTERFACE_DESCRIPTION tlsio_mbedtls_interface_description =
{
    tlsio_mbedtls_retrieveoptions,
    tlsio_mbedtls_create,
    tlsio_mbedtls_destroy,
    tlsio_mbedtls_open,
    tlsio_mbedtls_close,
    tlsio_mbed_send_async,
    tlsio_mbedtls_dowork,
    tlsio_mbedtls_setoption
};

static void dowork_send(TlsIoInstance* tls_io_instance);

// Some functions borrowed from tlsio_openssl_compact /Anton
/* Codes_SRS_TLSIO_30_005: [ The phrase "enter TLSIO_STATE_EXT_ERROR" means the adapter shall call the on_io_error function and pass the on_io_error_context that was supplied in tlsio_open_async. ]*/
static void enter_tlsio_error_state(TlsIoInstance* tls_io_instance)
{
    if (tls_io_instance->tlsio_state != TLSIO_STATE_ERROR)
    {
        tls_io_instance->tlsio_state = TLSIO_STATE_ERROR;
        tls_io_instance->on_io_error(tls_io_instance->on_io_error_context);
    }
}

// verbatim copy from tlsio_openssl_compact
// Return true if a message was available to remove
static bool process_and_destroy_head_message(TlsIoInstance* tls_io_instance, IO_SEND_RESULT send_result)
{
    bool result;
    LIST_ITEM_HANDLE head_pending_io;
    if (send_result == IO_SEND_ERROR)
    {
        /* Codes_SRS_TLSIO_30_095: [ If the send process fails before sending all of the bytes in an enqueued message, the tlsio_dowork shall call the message's on_send_complete along with its associated callback_context and IO_SEND_ERROR. ]*/
        enter_tlsio_error_state(tls_io_instance);
    }
    head_pending_io = singlylinkedlist_get_head_item(tls_io_instance->pending_transmission_list);
    if (head_pending_io != NULL)
    {
        PENDING_TRANSMISSION* head_message = (PENDING_TRANSMISSION*)singlylinkedlist_item_get_value(head_pending_io);
        // Must remove the item from the list before calling the callback because 
        // SRS_TLSIO_30_091: [ If  tlsio_dowork  is able to send all the bytes in an enqueued message, it shall first dequeue the message then call the messages's  on_send_complete  along with its associated  callback_context  and  IO_SEND_OK . ]
        if (singlylinkedlist_remove(tls_io_instance->pending_transmission_list, head_pending_io) != 0)
        {
            // This particular situation is a bizarre and unrecoverable internal error
            /* Codes_SRS_TLSIO_30_094: [ If the send process encounters an internal error or calls on_send_complete with IO_SEND_ERROR due to either failure or timeout, it shall also call on_io_error and pass in the associated on_io_error_context. ]*/
            enter_tlsio_error_state(tls_io_instance);
            LogError("Failed to remove message from list");
        }
        // on_send_complete is checked for NULL during PENDING_TRANSMISSION creation
        /* Codes_SRS_TLSIO_30_095: [ If the send process fails before sending all of the bytes in an enqueued message, the tlsio_dowork shall call the message's on_send_complete along with its associated callback_context and IO_SEND_ERROR. ]*/
        head_message->on_send_complete(head_message->callback_context, send_result);

        free(head_message->bytes);
        free(head_message);
        result = true;
    }
    else
    {
        result = false;
    }
    return result;
}

// DEPRECATED: debug functions do not belong in the tree.
#if defined (MBED_TLS_DEBUG_ENABLE)
void mbedtls_debug(void *ctx, int level, const char *file, int line, const char *str)
{
    ((void)level);
    printf("%s (%d): %s\r\n", file, line, str);
}
#endif

static void indicate_open_complete(TlsIoInstance* tls_io_instance, IO_OPEN_RESULT open_result)
{
    if (tls_io_instance->on_io_open_complete != NULL)
    {
        tls_io_instance->on_io_open_complete(tls_io_instance->on_io_open_complete_context, open_result);
    }
}

CONCRETE_IO_HANDLE tlsio_mbedtls_create(void* io_create_parameters)
{
  TLSIO_CONFIG* tls_io_config = static_cast<TLSIO_CONFIG *>(io_create_parameters);
  TlsIoInstance* result = NULL;

  if (tls_io_config == NULL)
  {
    LogError("NULL tls_io_config");
    // Codes_SRS_TLSIO_30_017: [ For direct designs, if either the underlying_io_interface or underlying_io_parameters of io_create_parameters is non-NULL, tlsio_create shall log an error and return NULL. ]

  } else if ( tls_io_config->underlying_io_interface != NULL )
  {
    LogError("Non-NULL underlying_io_interface");
  } else if ( tls_io_config->underlying_io_parameters != NULL )
  {
    LogError("Non-NULL underlying_io_parameters");
  }
  else
  {
    result = new TlsIoInstance(tls_io_config->hostname, static_cast<uint16_t>(tls_io_config->port));
    if (result != NULL)
    {
      // Create the message queue
      result->pending_transmission_list = singlylinkedlist_create();
      if (result->pending_transmission_list == NULL)
      {
        /* Codes_SRS_TLSIO_30_011: [ If any resource allocation fails, tlsio_create shall return NULL. ]*/
        ESP_LOGE(LOG_TAG, "Failed singlylinkedlist_create");
        delete result;
        result = nullptr;
      } else
      {
        tlsio_options_initialize(&result->options, TLSIO_OPTIONS_x509_TYPE_RSA);
        result->tlsio_state = TLSIO_STATE_CLOSED;
      }
    }
  }

  return result;
}

void tlsio_mbedtls_destroy(CONCRETE_IO_HANDLE tls_io)
{
  if (tls_io != NULL)
  {
    TlsIoInstance* tls_io_instance = reinterpret_cast<TlsIoInstance *>(tls_io);

    tls_io_instance->tlsSocket.close();

    tlsio_options_release_resources(&tls_io_instance->options);

    delete tls_io_instance;
    tls_io_instance = nullptr;
  }
}

int tlsio_mbedtls_open(CONCRETE_IO_HANDLE tls_io,
                       ON_IO_OPEN_COMPLETE on_io_open_complete,
                       void* on_io_open_complete_context,
                       ON_BYTES_RECEIVED on_bytes_received,
                       void* on_bytes_received_context,
                       ON_IO_ERROR on_io_error,
                       void* on_io_error_context)
{
  int result = 0;

  if (tls_io == nullptr)
  {
    LogError("NULL tls_io");
    result = __FAILURE__;
  }
  else
  {
    TlsIoInstance* tls_io_instance = (TlsIoInstance*)tls_io;

    if (tls_io_instance->tlsio_state != TLSIO_STATE_CLOSED)
    {
      ESP_LOGE(LOG_TAG, "TLSIO_STATE not CLOSED");
      result = __FAILURE__;
    }
    else
    {
      tls_io_instance->on_bytes_received = on_bytes_received;
      tls_io_instance->on_bytes_received_context = on_bytes_received_context;

      tls_io_instance->on_io_open_complete = on_io_open_complete;
      tls_io_instance->on_io_open_complete_context = on_io_open_complete_context;

      tls_io_instance->on_io_error = on_io_error;
      tls_io_instance->on_io_error_context = on_io_error_context;

      ESP_LOGD(LOG_TAG, "tlsio_mbedtls_open called with hostname=%s", tls_io_instance->hostnameSetting);

      // certificates variable is a set of trusted certificates
      // defined by the Azure SDK (certs/certs.h)
      const char *trustedCerts = certificates;
      if ( tls_io_instance->tlsSocket.setup(tls_io_instance->tls_clientcert,
                                            strlen(tls_io_instance->tls_clientcert)+1,
                                            trustedCerts,
                                            strlen(trustedCerts)+1) )
      {
        if ( tls_io_instance->tlsSocket.connect(tls_io_instance->hostnameSetting, static_cast<int>(tls_io_instance->port)) )
        {
          ESP_LOGD(LOG_TAG, "Successfully connected");
          tls_io_instance->tlsio_state = TLSIO_STATE_OPEN;
          tls_io_instance->tlsSocket.setBlocking(false, 10);
          indicate_open_complete(tls_io_instance, IO_OPEN_OK);
        } else
        {
          ESP_LOGE(LOG_TAG, "Failed to connect");
          result = __FAILURE__;
          indicate_open_complete(tls_io_instance, IO_OPEN_ERROR);
        }
      } else
      {
        ESP_LOGE(LOG_TAG, "TLS setup failed");
        result = __FAILURE__;
        indicate_open_complete(tls_io_instance, IO_OPEN_ERROR);
      }
    }
  }

  return result;
}

int tlsio_mbedtls_close(CONCRETE_IO_HANDLE tls_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context)
{
  int result = 0;

  ESP_LOGD(LOG_TAG, "Closing tlsio adapter");

  if (tls_io == NULL)
  {
    result = __FAILURE__;
  }
  else
  {
    TlsIoInstance* tls_io_instance = (TlsIoInstance*)tls_io;

    if ( (tls_io_instance->tlsio_state == TLSIO_STATE_CLOSED) )
    {
      result = __FAILURE__;
    }
    else
    {
      tls_io_instance->tlsSocket.close();
      tls_io_instance->tlsio_state = TLSIO_STATE_CLOSED;
      on_io_close_complete(callback_context);
    }
  }

  return result;
}

// Copied verbatim from tlsio_openssl_compact and renamed TODO remove references to openssl (also in log messages)
static int tlsio_mbed_send_async(CONCRETE_IO_HANDLE tls_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
    int result;

    if (on_send_complete == NULL)
    {
        /* Codes_SRS_TLSIO_30_062: [ If the on_send_complete is NULL, tlsio_openssl_compact_send shall log the error and return FAILURE. ]*/
        result = __FAILURE__;
        LogError("NULL on_send_complete");
    }
    else
    {
        if (tls_io == NULL)
        {
            /* Codes_SRS_TLSIO_30_060: [ If the tlsio_handle parameter is NULL, tlsio_openssl_compact_send shall log an error and return FAILURE. ]*/
            result = __FAILURE__;
            LogError("NULL tlsio");
        }
        else
        {
            if (buffer == NULL)
            {
                /* Codes_SRS_TLSIO_30_061: [ If the buffer is NULL, tlsio_openssl_compact_send shall log the error and return FAILURE. ]*/
                result = __FAILURE__;
                LogError("NULL buffer");
            }
            else
            {
                if (size == 0)
                {
                    /* Codes_SRS_TLSIO_30_067: [ If the  size  is 0,  tlsio_send  shall log the error and return FAILURE. ]*/
                    result = __FAILURE__;
                    LogError("0 size");
                }
                else
                {
                    TlsIoInstance* tls_io_instance = (TlsIoInstance*)tls_io;
                    if (tls_io_instance->tlsio_state != TLSIO_STATE_OPEN)
                    {
                        /* Codes_SRS_TLSIO_30_065: [ If tlsio_openssl_compact_open has not been called or the opening process has not been completed, tlsio_openssl_compact_send shall log an error and return FAILURE. ]*/
                        result = __FAILURE__;
                        LogError("tlsio_openssl_send_async without a prior successful open");
                    }
                    else
                    {
                        PENDING_TRANSMISSION* pending_transmission = (PENDING_TRANSMISSION*)malloc(sizeof(PENDING_TRANSMISSION));
                        if (pending_transmission == NULL)
                        {
                            /* Codes_SRS_TLSIO_30_064: [ If the supplied message cannot be enqueued for transmission, tlsio_openssl_compact_send shall log an error and return FAILURE. ]*/
                            result = __FAILURE__;
                            LogError("malloc failed");
                        }
                        else
                        {
                            /* Codes_SRS_TLSIO_30_063: [ The tlsio_openssl_compact_send shall enqueue for transmission the on_send_complete, the callback_context, the size, and the contents of buffer. ]*/
                            pending_transmission->bytes = (unsigned char*)malloc(size);

                            if (pending_transmission->bytes == NULL)
                            {
                                /* Codes_SRS_TLSIO_30_064: [ If the supplied message cannot be enqueued for transmission, tlsio_openssl_compact_send shall log an error and return FAILURE. ]*/
                                LogError("malloc failed");
                                free(pending_transmission);
                                result = __FAILURE__;
                            }
                            else
                            {
                                pending_transmission->size = size;
                                pending_transmission->unsent_size = size;
                                pending_transmission->on_send_complete = on_send_complete;
                                pending_transmission->callback_context = callback_context;
                                (void)memcpy(pending_transmission->bytes, buffer, size);

                                if (singlylinkedlist_add(tls_io_instance->pending_transmission_list, pending_transmission) == NULL)
                                {
                                    /* Codes_SRS_TLSIO_30_064: [ If the supplied message cannot be enqueued for transmission, tlsio_openssl_compact_send shall log an error and return FAILURE. ]*/
                                    LogError("Unable to add socket to pending list.");
                                    free(pending_transmission->bytes);
                                    free(pending_transmission);
                                    result = __FAILURE__;
                                }
                                else
                                {
                                    /* Codes_SRS_TLSIO_30_063: [ On success,  tlsio_send  shall enqueue for transmission the  on_send_complete , the  callback_context , the  size , and the contents of  buffer  and then return 0. ]*/
                                    dowork_send(tls_io_instance);
                                    result = 0;
                                }
                            }
                        }
                    }
                }
            }
        }
        /* Codes_SRS_TLSIO_30_066: [ On failure, on_send_complete shall not be called. ]*/
    }
    return result;
}

static void dowork_receive(TlsIoInstance *adapter)
{
  int ret;

  char receiveBuffer[128] = {};

  do
  {
    ret = adapter->tlsSocket.receive(receiveBuffer, sizeof(receiveBuffer));
    if ( ret > 0 )
    {
      adapter->on_bytes_received(adapter->on_bytes_received_context,
                                 reinterpret_cast<unsigned char *>(receiveBuffer),
                                 ret);
    }
  } while ( ret > 0 );

  if ( ret == -2 )
  {
    // SOCKET_ERROR_WOULD_BLOCK
  }
  else if ( ret == 0 )
  {
    // 0 means connection closed for standard recv from <sys/socket.h>
    // in SocketInterface 0 is not an error
  }
  else
  {
    ESP_LOGW(LOG_TAG, "Failed to receive data: %d. socket broken?", ret);
    enter_tlsio_error_state(adapter);
  }
}

// Verbatim copy from tlsio_openssl_compact (with some modifications to use TLSSocket)
static void dowork_send(TlsIoInstance* tls_io_instance)
{
    LIST_ITEM_HANDLE first_pending_io = singlylinkedlist_get_head_item(tls_io_instance->pending_transmission_list);

    if (first_pending_io != NULL)
    {
        PENDING_TRANSMISSION* pending_message = (PENDING_TRANSMISSION*)singlylinkedlist_item_get_value(first_pending_io);
        uint8_t* buffer = ((uint8_t*)pending_message->bytes) +
            pending_message->size - pending_message->unsent_size;
        int write_result = tls_io_instance->tlsSocket.send(reinterpret_cast<char *>(buffer), pending_message->unsent_size);

        if (write_result > 0)
        {
            pending_message->unsent_size -= write_result;
            if (pending_message->unsent_size == 0)
            {
                /* Codes_SRS_TLSIO_30_091: [ If tlsio_openssl_compact_dowork is able to send all the bytes in an enqueued message, it shall call the messages's on_send_complete along with its associated callback_context and IO_SEND_OK. ]*/
                // The whole message has been sent successfully
                process_and_destroy_head_message(tls_io_instance, IO_SEND_OK);
            }
            else
            {
                /* Codes_SRS_TLSIO_30_093: [ If the TLS connection was not able to send an entire enqueued message at once, subsequent calls to tlsio_dowork shall continue to send the remaining bytes. ]*/
                // Repeat the send on the next pass with the rest of the message
                // This empty else compiles to nothing but helps readability
            }
        }
        else
        {
            if ( write_result == -1 )
            {
                /* Codes_SRS_TLSIO_30_002: [ The phrase "destroy the failed message" means that the adapter shall remove the message from the queue and destroy it after calling the message's on_send_complete along with its associated callback_context and IO_SEND_ERROR. ]*/
                /* Codes_SRS_TLSIO_30_005: [ When the adapter enters TLSIO_STATE_EXT_ERROR it shall call the  on_io_error function and pass the on_io_error_context that were supplied in  tlsio_open . ]*/
                /* Codes_SRS_TLSIO_30_095: [ If the send process fails before sending all of the bytes in an enqueued message, tlsio_dowork shall destroy the failed message and enter TLSIO_STATE_EX_ERROR. ]*/
                // This is an unexpected error, and we need to bail out. Probably lost internet connection.
                //LogInfo("Error from SSL_write: %d", hard_error);
                ESP_LOGE(LOG_TAG, "Error from TLSSocket->send: %d", write_result);
                process_and_destroy_head_message(tls_io_instance, IO_SEND_ERROR);
            } // if write_result == -2, WOULD BLOCK
        }
    }
    else
    {
        /* Codes_SRS_TLSIO_30_096: [ If there are no enqueued messages available, tlsio_openssl_compact_dowork shall do nothing. ]*/
    }
}

void tlsio_mbedtls_dowork(CONCRETE_IO_HANDLE tls_io)
{
  if (tls_io != NULL)
  {
    TlsIoInstance* tls_io_instance = (TlsIoInstance*)tls_io;

    switch ( tls_io_instance->tlsio_state )
    {
    case TLSIO_STATE_CLOSED:
      // Waiting to be opened
      break;
    case TLSIO_STATE_OPEN:
      // 1. receive all that is available without blocking
      dowork_receive(tls_io_instance);
      // 2. send as much as possible without blocking
      dowork_send(tls_io_instance);
      break;
    default:
      LogError("Unexpected internal tlsio state");
      break;
    }
  }
}

extern "C" const IO_INTERFACE_DESCRIPTION* tlsio_mbedtls_get_interface_description(void)
{
    return &tlsio_mbedtls_interface_description;
}

OPTIONHANDLER_HANDLE tlsio_mbedtls_retrieveoptions(CONCRETE_IO_HANDLE tls_io)
{
  TlsIoInstance* tls_io_instance = reinterpret_cast<TlsIoInstance *>(tls_io);
  /* Codes_SRS_TLSIO_30_160: [ If the tlsio_handle parameter is NULL, tlsio_openssl_compact_retrieveoptions shall do nothing except log an error and return FAILURE. ]*/
  OPTIONHANDLER_HANDLE result;
  if (tls_io_instance == NULL)
  {
      LogError("NULL tlsio");
      result = NULL;
  }
  else
  {
      result = tlsio_options_retrieve_options(&tls_io_instance->options, tlsio_mbedtls_setoption);
  }
  return result;
}

int tlsio_mbedtls_setoption(CONCRETE_IO_HANDLE tls_io, const char* optionName, const void* value)
{
  int result;

  if (tls_io == NULL || optionName == NULL)
  {
    result = __FAILURE__;
  }
  else
  {
    TlsIoInstance* tls_io_instance = (TlsIoInstance*)tls_io;
    if ( strcmp(SU_OPTION_X509_CERT, optionName) == 0 )
    {
      ESP_LOGD(LOG_TAG, "Received X509_CERT: %s", reinterpret_cast<const char *>(value));

      strncpy(tls_io_instance->tls_clientcert,
              reinterpret_cast<const char *>(value),
              sizeof(tls_io_instance->tls_clientcert));
      tls_io_instance->tls_clientcert[sizeof(tls_io_instance->tls_clientcert)-1] = 0;

      TLSIO_OPTIONS_RESULT options_result = tlsio_options_set(&tls_io_instance->options, optionName, value);
      if ( options_result == TLSIO_OPTIONS_RESULT_SUCCESS )
      {
        result = 0;
      } else
      {
        ESP_LOGE(LOG_TAG, "tlsio_options_set failed");
        result = __FAILURE__;
      }
    }
    else
    {
      ESP_LOGW(LOG_TAG, "Unhandled option: %s", optionName);
      result = 0;
    }
  }

  return result;
}

