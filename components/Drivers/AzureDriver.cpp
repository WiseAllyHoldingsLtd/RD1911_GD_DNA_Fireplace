#include <cstdint>
#include "iothub_client.h"
#include "iothub_message.h"
#include "iothub_client_options.h"
#include "iothubtransportmqtt.h"
#include "iothubtransportamqp.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "azure_c_shared_utility/tickcounter.h" // tickcounter_ms_t
#include "certificates.h"
#include "certs.h" // azure-sdk/certs/certs.h

#include "EspCpp.hpp"
#include "FrameType.h"
#include "TimeSyncInfo.h"
#include "FirmwareUpdateReady.h"
#include "MultipleFrameRequest.h"
#include "AzureDriver.hpp"


namespace
{
  const char LOG_TAG[] = "AzureDriver";
  const char DEVICE_METHOD_NAME[] = "transferFrame";
}


AzureDriver::AzureDriver(QueueInterface<char[Constants::CLOUD_CONNECTION_STRING_MAX_LENGTH]> &queue, const CryptoDriverInterface &crypto,
                         AsyncMessageQueueReadInterface &asyncQueue, TimerDriverInterface &timerDriver, AzurePayload &azurePayload)
: m_connectionStringQueue(queue),
  m_cryptoDriver(crypto),
  m_asyncMsgQueue(asyncQueue),
  m_timerDriver(timerDriver),
  m_azurePayload(azurePayload),
  m_status(CloudStatus::Idle),
  m_shutdownRequested(false),
  m_asyncSendMessageHandle(nullptr),
  m_asyncSendMessageCounter(0U),
  m_numConsecutiveSendTimeouts(0u),
  m_isCurrentlySendingAsync(false),
  m_currentAsyncSendStartTimestamp(0u)
{
}

bool AzureDriver::isConnected() const
{
  return getStatus() == CloudStatus::Connected;
}

bool AzureDriver::isIdle() const
{
  CloudStatus::Enum currentStatus = getStatus();
  return ((currentStatus == static_cast<int>(CloudStatus::Idle))
      || (currentStatus == static_cast<int>(CloudStatus::Stopped))
      || (currentStatus == static_cast<int>(CloudStatus::Error)));
}

CloudStatus::Enum AzureDriver::getStatus() const
{
  return m_status;
}

void AzureDriver::requestShutdown()
{
  m_shutdownRequested = true;
}

bool AzureDriver::isUnableToSend() const
{
  bool timeoutReached = isSendConfirmationTimeoutReached();
  bool tooManyConsecutiveFailuresToSend = (m_numConsecutiveSendTimeouts > Constants::AZURE_MAX_CONSECUTIVE_SEND_TIMEOUTS);

  return timeoutReached || tooManyConsecutiveFailuresToSend;
}

bool AzureDriver::connect(const char (&connectionString)[Constants::CLOUD_CONNECTION_STRING_MAX_LENGTH])
{
  bool result = false;
  if (isIdle()) {
    m_connectionStringQueue.reset();
    result = m_connectionStringQueue.push(connectionString);
  }
  else {
    ESP_LOGI(LOG_TAG, "Got connection request, but driver not idle! (%i)", static_cast<int>(getStatus()));
  }
  return result;
}

void AzureDriver::run(bool eternalLoop)
{
  char connectionString[Constants::CLOUD_CONNECTION_STRING_MAX_LENGTH];
  do {
    m_shutdownRequested = false;
    ESP_LOGI(LOG_TAG, "Waiting for connection string");
    if (m_connectionStringQueue.pop(connectionString)) {
      if ( !m_shutdownRequested )
      {
        ESP_LOGI(LOG_TAG, "Trying to connect");
        doConnectionLoop(connectionString);
      } else
      {
        m_status = CloudStatus::Idle;
      }
    }
  } while (eternalLoop);
}

void AzureDriver::doConnectionLoop(const char * connectionString)
{
  IOTHUB_CLIENT_LL_HANDLE clientHandle = nullptr;

  m_status = CloudStatus::Connecting;

  int result = platform_init();
  if (result == 0) {
    ESP_LOGI(LOG_TAG, "Using connection string: %s", connectionString);
    clientHandle = IoTHubClient_LL_CreateFromConnectionString(connectionString, MQTT_Protocol);
    if (clientHandle != nullptr) {
      IOTHUB_CLIENT_RESULT retval = IOTHUB_CLIENT_ERROR;

      {
        // TODO: DEBUG - Should probably be disabled for production
        bool traceOn = false;
        retval = IoTHubClient_LL_SetOption(clientHandle, OPTION_LOG_TRACE, &traceOn);
        if (retval != static_cast<int>(IOTHUB_CLIENT_OK)) {
          ESP_LOGE(LOG_TAG, "Failed to set trace on");
        }

        tickcounter_ms_t messageTimeoutParam = static_cast<tickcounter_ms_t>(Constants::AZURE_OPTION_MESSAGE_TIMEOUT_MS);
        retval = IoTHubClient_LL_SetOption(clientHandle, OPTION_MESSAGE_TIMEOUT, &messageTimeoutParam);
        if (retval != static_cast<int>(IOTHUB_CLIENT_OK)) {
          ESP_LOGE(LOG_TAG, "Failed to set send/message timeout");
        }

        retval = IoTHubClient_LL_SetConnectionStatusCallback(clientHandle, sConnectionStatusCallback, this);
        if (retval != static_cast<int>(IOTHUB_CLIENT_OK)) {
          ESP_LOGE(LOG_TAG, "Failed to set connection status callback: %i", static_cast<int>(retval));
        }

        char certChain[2048] = { };
        ESP_LOGD(LOG_TAG, "\n---- Cert sizes: %u and %u ----\n",
            static_cast<uint32_t>(m_cryptoDriver.getClientCertificateSize()),
            static_cast<uint32_t>(m_cryptoDriver.getSignerCertificateSize()));

        if ( sizeof(certChain) >= m_cryptoDriver.getClientCertificateSize()+m_cryptoDriver.getSignerCertificateSize() )
        {
          ESP_LOGD(LOG_TAG, "\n--- Client cert:\n\n%s\n", m_cryptoDriver.getClientCertificate());
          ESP_LOGD(LOG_TAG, "\n--- Signer cert:\n\n%s\n", m_cryptoDriver.getSignerCertificate());

          memcpy(certChain, m_cryptoDriver.getClientCertificate(), m_cryptoDriver.getClientCertificateSize());
          memcpy(certChain+m_cryptoDriver.getClientCertificateSize()-1, m_cryptoDriver.getSignerCertificate(), m_cryptoDriver.getSignerCertificateSize());
          retval = IoTHubClient_LL_SetOption(clientHandle, OPTION_X509_CERT, certChain);
          if (retval != static_cast<int>(IOTHUB_CLIENT_OK)) {
            ESP_LOGE(LOG_TAG, "Failed to set client certificate: %i", static_cast<int>(retval));
          }
        } else
        {
          ESP_LOGE(LOG_TAG, "Certificate buffer size too small for cert chain");
        }
      }

      // Set message callback so we can receive data
      retval = IoTHubClient_LL_SetDeviceMethodCallback(clientHandle, sDeviceMethodCallback, this);

      if (retval == static_cast<int>(IOTHUB_CLIENT_OK)){

        ESP_LOGI(LOG_TAG, "Successfully set message callback");


        do {
          IoTHubClient_LL_DoWork(clientHandle);
          ThreadAPI_Sleep(Constants::CLOUD_AZURE_DOWORK_DEFAULT_INTERVAL_MS);

          if (sendDeviceToCloudAsyncIfPossible(clientHandle))
          {
            IoTHubClient_LL_DoWork(clientHandle);
            ThreadAPI_Sleep(Constants::CLOUD_AZURE_DOWORK_SHORT_INTERVAL_MS);
          }

        }
        while (!m_shutdownRequested);

        m_status = CloudStatus::Stopped;
      }
      else {
        ESP_LOGE(LOG_TAG, "Failed to set message callback: %i", static_cast<int>(retval));
        m_status = CloudStatus::Error;
      }

      IoTHubClient_LL_Destroy(clientHandle);
    }
    else {
      ESP_LOGE(LOG_TAG, "Failed to create IoTHubClient");
      m_status = CloudStatus::Error;
    }

    platform_deinit();
  }
  else {
    ESP_LOGE(LOG_TAG, "Failed to initialize platform: %i", result);
    m_status = CloudStatus::Error;
  }

  // Fallback if we arrive here without setting proper status
  if (!isIdle()) {
    m_status = CloudStatus::Stopped;
  }
}

void AzureDriver::updateConnectionStatus(IOTHUB_CLIENT_CONNECTION_STATUS status, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason)
{
  int statusCode = static_cast<int>(status);
  int reasonCode = static_cast<int>(reason);

  if (statusCode == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED)
  {
    // We are connected!
    m_status = CloudStatus::Connected;
    ESP_LOGI(LOG_TAG, "Received connection status change: Connected (IOTHUB_CLIENT_CONNECTION_AUTHENTICATED)");
  }
  else if (statusCode == IOTHUB_CLIENT_CONNECTION_UNAUTHENTICATED)
  {
    // We are not connected - only change our own status if we are Connected! The other statuses are handled equally in ConnectionController
    if (getStatus() == CloudStatus::Connected)
    {
      m_status = CloudStatus::Connecting;
    }

    ESP_LOGI(LOG_TAG, "Received connection status change: Not connected (IOTHUB_CLIENT_CONNECTION_UNAUTHENTICATED)");

    switch (reasonCode)
    {
    case IOTHUB_CLIENT_CONNECTION_EXPIRED_SAS_TOKEN:
      ESP_LOGI(LOG_TAG, "Connection status change reason: IOTHUB_CLIENT_CONNECTION_EXPIRED_SAS_TOKEN");
      break;
    case IOTHUB_CLIENT_CONNECTION_DEVICE_DISABLED:
      ESP_LOGI(LOG_TAG, "Connection status change reason: IOTHUB_CLIENT_CONNECTION_DEVICE_DISABLED");
      break;
    case IOTHUB_CLIENT_CONNECTION_BAD_CREDENTIAL:
      ESP_LOGI(LOG_TAG, "Connection status change reason: IOTHUB_CLIENT_CONNECTION_BAD_CREDENTIAL");
      break;
    case IOTHUB_CLIENT_CONNECTION_RETRY_EXPIRED:
      ESP_LOGI(LOG_TAG, "Connection status change reason: IOTHUB_CLIENT_CONNECTION_RETRY_EXPIRED");
      break;
    case IOTHUB_CLIENT_CONNECTION_NO_NETWORK:
      ESP_LOGI(LOG_TAG, "Connection status change reason: IOTHUB_CLIENT_CONNECTION_NO_NETWORK");
      break;
    case IOTHUB_CLIENT_CONNECTION_COMMUNICATION_ERROR:
      ESP_LOGI(LOG_TAG, "Connection status change reason: IOTHUB_CLIENT_CONNECTION_COMMUNICATION_ERROR");
      break;
    case IOTHUB_CLIENT_CONNECTION_OK:
      ESP_LOGI(LOG_TAG, "Connection status change reason: IOTHUB_CLIENT_CONNECTION_OK");
      break;
    default:
      ESP_LOGD(LOG_TAG, "Unknown reason");
      break;
    }
  }
  else
  {
    ESP_LOGI(LOG_TAG, "Received connection status change: Unrecognized (%d)", statusCode);
  }
}


int AzureDriver::processMessage(const char *methodName, const unsigned char *payload, size_t size, unsigned char **response, size_t *responseSize)
{
  CloudDeviceMethodStatus::Enum messageStatus = CloudDeviceMethodStatus::METHOD_NOT_ALLOWED;
  TransferFrameResponse responseFrameJson;  // generates a { "frame": Null } by default
  uint64_t startProcessingTimeMS = m_timerDriver.getTimeSinceBootMS();
  uint32_t timeoutMS = Constants::CLOUD_DEFAULT_PROCESSING_TIMEOUT_MS;

  if (strncmp(methodName, DEVICE_METHOD_NAME, sizeof(DEVICE_METHOD_NAME)) == 0)
  {
    /* We have received data with supported method name. */

    FrameParser frameReader;
    bool wasParsed = m_azurePayload.parseTransferFrame(payload, size, timeoutMS, frameReader);

    if (wasParsed)
    {
      messageStatus = processReceivedFrame(frameReader, responseFrameJson);
    }
    else
    {
      ESP_LOGE(LOG_TAG, "Request parse failed.");
      messageStatus = CloudDeviceMethodStatus::METHOD_NOT_ALLOWED; // TODO: Decide error value
    }
  }

  if (m_timerDriver.getTimeSinceTimestampMS(startProcessingTimeMS) > static_cast<uint64_t>(timeoutMS))
  {
    ESP_LOGW(LOG_TAG, "Processing time exceeds specified timeout of %u ms.", timeoutMS);
    messageStatus = CloudDeviceMethodStatus::TIMEOUT;
  }


  /* Register response with Azure SDK */

  char * frameResponseString = responseFrameJson.getJSONString();

  if (frameResponseString)
  {
    *responseSize = strlen(frameResponseString);
    *response = static_cast<unsigned char*>(malloc(*responseSize));

    if (*response != nullptr)
    {
      memcpy(*response, frameResponseString, *responseSize);
      ESP_LOGI(LOG_TAG, "Generated JSON response: %s", frameResponseString);
    }

    free(frameResponseString);
  }
  else
  {
    ESP_LOGW(LOG_TAG, "Generating JSON response failed.");
  }

  return static_cast<int>(messageStatus);
}


CloudDeviceMethodStatus::Enum AzureDriver::processReceivedFrame(const FrameParser & frame, TransferFrameResponse & responseFrame)
{
  CloudDeviceMethodStatus::Enum status;
  FrameType::Enum frameID = static_cast<FrameType::Enum>(frame.getID());

  switch (frameID)
  {
  case FrameType::FW_UPDATE_READY:
    status = handleFirmwareUpdateReadyRequest(frame, responseFrame);
    break;
  case FrameType::TIME_SYNC_INFO:
    status = handleTimeSyncInfoRequest(frame, responseFrame);
    break;
  case FrameType::MULTIPLE_FRAME_REQUEST:
    status = handleMultipleFrameRequest(frame, responseFrame);
    break;
  case FrameType::OPERATION_INFO:
    status = handleOperationInfoRequest(frame, responseFrame);
    break;
  default:
    status = handleUnsupportedRequest(frame);
    break;
  }

  return status;
}


CloudDeviceMethodStatus::Enum AzureDriver::handleTimeSyncInfoRequest(const FrameParser & frame, TransferFrameResponse & responseFrame)
{
  CloudDeviceMethodStatus::Enum returnStatus = CloudDeviceMethodStatus::OK;
  TimeSyncInfo timeSync(frame);

  if (timeSync.isValid())
  {
    ESP_LOGI(LOG_TAG, "TimeSyncInfo received.");
    m_azurePayload.processTimeSyncInfo(timeSync);
  }
  else
  {
    ESP_LOGE(LOG_TAG, "Ignoring TimeSyncInfo - frame validation failed.");
    returnStatus = CloudDeviceMethodStatus::METHOD_NOT_ALLOWED; // TODO: Examples use -1 for "serious" errors, maybe adapt?
  }

  return returnStatus;
}


CloudDeviceMethodStatus::Enum AzureDriver::handleFirmwareUpdateReadyRequest(const FrameParser & frame, TransferFrameResponse & responseFrame)
{
  CloudDeviceMethodStatus::Enum returnStatus = CloudDeviceMethodStatus::OK;
  FirmwareUpdateReady fwUpdate(frame);

  if (fwUpdate.isValid())
  {
    ESP_LOGI(LOG_TAG, "FirmwareUpdateReady received.");
    bool wasProcessed = m_azurePayload.processFirmwareUpdateRequest(fwUpdate);

    if (wasProcessed)
    {
      ESP_LOGD(LOG_TAG, "FirmwareUpdateReady processed.");
    }
    else
    {
      ESP_LOGE(LOG_TAG, "FirmwareUpdateReady rejected.");
      returnStatus = CloudDeviceMethodStatus::METHOD_NOT_ALLOWED; // TODO: Examples use -1 for "serious" errors, maybe adapt?
    }
  }
  else
  {
    ESP_LOGE(LOG_TAG, "FirmwareUpdate rejected - frame validation failed.");
    returnStatus = CloudDeviceMethodStatus::METHOD_NOT_ALLOWED; // TODO: Examples use -1 for "serious" errors, maybe adapt?
  }

  return returnStatus;
}


CloudDeviceMethodStatus::Enum AzureDriver::handleMultipleFrameRequest(const FrameParser & frame, TransferFrameResponse & responseFrame)
{
  CloudDeviceMethodStatus::Enum returnStatus = CloudDeviceMethodStatus::OK;
  MultipleFrameRequest multipleFrame(frame);

  if (multipleFrame.isValid())
  {
    ESP_LOGI(LOG_TAG, "MultipleFrameTransportFrame received. NumOfFrames is %u.", multipleFrame.getNumOfPayloadFrames());

    bool wasProcessed = m_azurePayload.processMultipleFrame(multipleFrame, responseFrame);

    if (wasProcessed)
    {
      ESP_LOGD(LOG_TAG, "MultipleFrameTransportFrame processed.");
    }
    else
    {
      ESP_LOGW(LOG_TAG, "MultipleFrameTransportFrame processing failed.");
      returnStatus = CloudDeviceMethodStatus::METHOD_NOT_ALLOWED; // TODO: Examples use -1 for "serious" errors, maybe adapt?
    }
  }
  else
  {
    ESP_LOGW(LOG_TAG, "Ignoring MultipleFrameTransportFrame - frame validation failed.");
    returnStatus = CloudDeviceMethodStatus::METHOD_NOT_ALLOWED; // TODO: Examples use -1 for "serious" errors, maybe adapt?
  }

  return returnStatus;
}

CloudDeviceMethodStatus::Enum AzureDriver::handleOperationInfoRequest(
    const FrameParser & frame,
    TransferFrameResponse & responseFrame)
{
  CloudDeviceMethodStatus::Enum returnStatus = CloudDeviceMethodStatus::OK;
  OperationInfo operationInfo(frame);

  if (operationInfo.isValid())
  {
    ESP_LOGI(LOG_TAG, "OperationInfo received.");

    bool wasProcessed = m_azurePayload.processOperationInfo(operationInfo);

    if (wasProcessed)
    {
      ESP_LOGI(LOG_TAG, "OperationInfo processed.");
      responseFrame.makeEmptyJson();
    }
    else
    {
      ESP_LOGW(LOG_TAG, "OperationInfo processing failed.");
      returnStatus = CloudDeviceMethodStatus::METHOD_NOT_ALLOWED; // TODO: Examples use -1 for "serious" errors, maybe adapt?
    }
  }
  else
  {
    ESP_LOGW(LOG_TAG, "Ignoring OperationInfo - frame validation failed.");
    returnStatus = CloudDeviceMethodStatus::METHOD_NOT_ALLOWED; // TODO: Examples use -1 for "serious" errors, maybe adapt?
  }

  return returnStatus;
}


CloudDeviceMethodStatus::Enum AzureDriver::handleUnsupportedRequest(const FrameParser & frame) const
{
  ESP_LOGW(LOG_TAG, "Received unsupported frame 0x%04x", frame.getID());
  return CloudDeviceMethodStatus::METHOD_NOT_ALLOWED;
}


bool AzureDriver::sendDeviceToCloudAsyncIfPossible(IOTHUB_CLIENT_LL_HANDLE iotHubHandle)
{
  bool wasSent = false;

  if ((getStatus() == CloudStatus::Connected) && (!m_isCurrentlySendingAsync) && (!m_asyncMsgQueue.isEmpty()))
  {
    uint32_t msgSize = m_asyncMsgQueue.getPopItemSize();
    uint8_t * msgBytes = static_cast<uint8_t*>(malloc(msgSize));


    if (msgBytes != nullptr)
    {
      if (m_asyncMsgQueue.peekItem(msgBytes, msgSize))
      {
        m_asyncSendMessageHandle = IoTHubMessage_CreateFromByteArray(msgBytes, msgSize);

        if (m_asyncSendMessageHandle != nullptr)
        {
          ++m_asyncSendMessageCounter;

          MAP_HANDLE propertyMap = IoTHubMessage_Properties(m_asyncSendMessageHandle);
          Map_AddOrUpdate(propertyMap, "RoutingType", "DirectAppliance"); // Requested by Keith
          IOTHUB_CLIENT_RESULT sendResult = IoTHubClient_LL_SendEventAsync(iotHubHandle, m_asyncSendMessageHandle, sSendConfirmationCallback, this);

          if (sendResult == IOTHUB_CLIENT_OK)
          {
            m_isCurrentlySendingAsync = true;
            m_currentAsyncSendStartTimestamp = m_timerDriver.getTimeSinceBootMS();
            ESP_LOGI(LOG_TAG, "Send DeviceToCloud OK. Message %u.", m_asyncSendMessageCounter);
          }
          else
          {
            ESP_LOGE(LOG_TAG, "Send DeviceToCloud failed. Message %u. Error %d", m_asyncSendMessageCounter, sendResult);
          }
        }
        else
        {
          ESP_LOGW(LOG_TAG, "Send DeviceToCloud failed - failed to create message from byte array.");
        }
      }
      else
      {
        ESP_LOGW(LOG_TAG, "Send DeviceToCloud failed - failed to get message from queue.");
      }

      free(msgBytes);
    }
    else
    {
      ESP_LOGW(LOG_TAG, "Send DeviceToCloud failed - insufficient memory.");
    }
  }

  return wasSent;
}

bool AzureDriver::isSendConfirmationTimeoutReached() const
{
  bool result = false;

  if ( m_isCurrentlySendingAsync )
  {
    uint64_t timeWaitedForConfirmation =
        m_timerDriver.getTimeSinceBootMS() - m_currentAsyncSendStartTimestamp;

    result =
        timeWaitedForConfirmation > Constants::AZURE_SEND_CONFIRMATION_APP_MAX_WAIT_MS;

    ESP_LOGW(LOG_TAG, "Waited %u ms for send confirmation",
        static_cast<uint32_t>(timeWaitedForConfirmation));
  }

  return result;
}

void AzureDriver::processSendConfirmation(IOTHUB_CLIENT_CONFIRMATION_RESULT result)
{

  switch (result)
  {
  case IOTHUB_CLIENT_CONFIRMATION_OK:
    ESP_LOGI(LOG_TAG, "Send confirmation callback for message %u. Result: OK (IOTHUB_CLIENT_CONFIRMATION_OK)", m_asyncSendMessageCounter);
    m_asyncMsgQueue.popItem();
    break;
  case IOTHUB_CLIENT_CONFIRMATION_BECAUSE_DESTROY:
    ESP_LOGW(LOG_TAG, "Send confirmation callback for message %u. Result: Failed (IOTHUB_CLIENT_CONFIRMATION_BECAUSE_DESTROY)", m_asyncSendMessageCounter);
    break;
  case IOTHUB_CLIENT_CONFIRMATION_MESSAGE_TIMEOUT:
    ESP_LOGW(LOG_TAG, "Send confirmation callback for message %u. Result: Failed (IOTHUB_CLIENT_CONFIRMATION_MESSAGE_TIMEOUT)", m_asyncSendMessageCounter);
    ++m_numConsecutiveSendTimeouts;
    break;
  case IOTHUB_CLIENT_CONFIRMATION_ERROR:
    ESP_LOGW(LOG_TAG, "Send confirmation callback for message %u. Result: Failed (IOTHUB_CLIENT_CONFIRMATION_ERROR)", m_asyncSendMessageCounter);
    break;
  default:
    ESP_LOGW(LOG_TAG, "Send confirmation callback for message %u. Result: Unknown (%d)", m_asyncSendMessageCounter, result);
    break;
  }

  if ( result != IOTHUB_CLIENT_CONFIRMATION_MESSAGE_TIMEOUT )
  {
    m_numConsecutiveSendTimeouts = 0;
  }

  if (m_asyncSendMessageHandle != nullptr)
  {
    IoTHubMessage_Destroy(m_asyncSendMessageHandle);
  }

  m_isCurrentlySendingAsync = false;
  m_currentAsyncSendStartTimestamp = 0u;
}


/* Static callback handler for connection status changes */
void AzureDriver::sConnectionStatusCallback(IOTHUB_CLIENT_CONNECTION_STATUS status, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void *userContextCallback)
{
  static_cast<AzureDriver *>(userContextCallback)->updateConnectionStatus(status, reason);
}


/* Static callback handler for DeviceMethod message received (transferFrames) */
int AzureDriver::sDeviceMethodCallback(const char *methodName, const unsigned char *payload, size_t size, unsigned char **response, size_t *responseSize, void *userContextCallback)
{
  return static_cast<AzureDriver *>(userContextCallback)->processMessage(methodName, payload, size, response, responseSize);
}


/* Static callback handler for send confirmation after we have sent async message */
void AzureDriver::sSendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void *userContextCallback)
{
  static_cast<AzureDriver *>(userContextCallback)->processSendConfirmation(result);
}
