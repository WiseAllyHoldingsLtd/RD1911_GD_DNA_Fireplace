#include "sdkconfig.h"
#include "esp_log.h"

#include "mbedtls/error.h"

#include "TLSSocket.h"

// DEBUG (enable in menuconfig)
#ifdef CONFIG_MBEDTLS_DEBUG
#include "mbedtls/debug.h"
#define TLS_DEBUG_LEVEL 4  // DEBUG: set this between 1-4 to select verbosity.
#endif

namespace {
 const char LOG_TAG[] = "TLSSocket";
 const char CTR_DRBG_PERSONALIZATION[] = "GD Nordic Wifi ECU-2Rw";
}


/*
 * Allowed cipher suites
 * Only ECDHE_ECDSA will use the ATECC cryptochip!
 */
static const int cipherSuites[] =
{
   MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256,
   MBEDTLS_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256,
   MBEDTLS_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA,
   MBEDTLS_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA,
   MBEDTLS_TLS_RSA_WITH_AES_128_GCM_SHA256,
   MBEDTLS_TLS_RSA_WITH_AES_128_CBC_SHA,
   MBEDTLS_TLS_RSA_WITH_AES_256_CBC_SHA,
   MBEDTLS_TLS_RSA_WITH_3DES_EDE_CBC_SHA
};


TLSSocket::TLSSocket()
: TCPSocket(), m_lastErrorCode(0)
{
  mbedtls_ssl_init(&m_tls_context);
  mbedtls_x509_crt_init(&m_tls_cacert);
  mbedtls_x509_crt_init(&m_tls_clientcert);
  mbedtls_pk_init(&m_tls_clientpriv);
  mbedtls_ctr_drbg_init(&m_tls_ctr_drbg);

  mbedtls_ssl_config_init(&m_tls_config);
  mbedtls_entropy_init(&m_tls_entropy);
  m_isInitialized = true;
}

TLSSocket::~TLSSocket()
{
  close();
}

bool TLSSocket::setup(const char * clientCertificate, size_t clientCertSize,
                      const char * rootCertificate, size_t rootCertSize)
{
  bool result = false;
  int retval;

  ESP_LOGD(LOG_TAG, "Seeding the random number generator");
  retval = mbedtls_ctr_drbg_seed(&m_tls_ctr_drbg, mbedtls_entropy_func, &m_tls_entropy,
      reinterpret_cast<const unsigned char*>(CTR_DRBG_PERSONALIZATION), sizeof(CTR_DRBG_PERSONALIZATION));

  if (retval == 0) {
    ESP_LOGD(LOG_TAG, "Loading the CA root certificate...");
    // FIXME: allow custom CA ?
    retval = mbedtls_x509_crt_parse(&m_tls_cacert, reinterpret_cast<const unsigned char*>(rootCertificate), rootCertSize);

    if (retval == 0) {
      ESP_LOGD(LOG_TAG, "Loading client certificate");
      if (clientCertificate != nullptr) {
        // Load custom cert
        retval = mbedtls_x509_crt_parse(&m_tls_clientcert, reinterpret_cast<const unsigned char*>(clientCertificate), clientCertSize);

        if (retval == 0) {
          // FIXME: Should this also be available as parameter?
          ESP_LOGD(LOG_TAG, "Loading dummy client private key");
          retval = mbedtls_pk_parse_key(&m_tls_clientpriv, reinterpret_cast<const unsigned char*>(x509_dummy_pk_pem), sizeof(x509_dummy_pk_pem), nullptr, 0);
        }
      }
    }

    if (retval == 0) {
      ESP_LOGD(LOG_TAG, "Setting up the SSL/TLS structure...");
      // FIXME: pass in or allow to customize?
      retval = mbedtls_ssl_config_defaults(&m_tls_config,
          MBEDTLS_SSL_IS_CLIENT,
          MBEDTLS_SSL_TRANSPORT_STREAM,
          MBEDTLS_SSL_PRESET_DEFAULT);

      if (retval == 0) {
        // Require TLS1.2+
        mbedtls_ssl_conf_min_version(&m_tls_config, MBEDTLS_SSL_MAJOR_VERSION_3, MBEDTLS_SSL_MINOR_VERSION_3);
        // Require valid host certificate
        mbedtls_ssl_conf_authmode(&m_tls_config, MBEDTLS_SSL_VERIFY_REQUIRED);

        // Set allowed cipher suites
        mbedtls_ssl_conf_ciphersuites(&m_tls_config, cipherSuites);

        // Load client certificate
        retval = mbedtls_ssl_conf_own_cert(&m_tls_config, &m_tls_clientcert, &m_tls_clientpriv);

        // Allow renegotiation (required for Azure's poor implementation of client certificates)
        mbedtls_ssl_conf_renegotiation(&m_tls_config, MBEDTLS_SSL_RENEGOTIATION_ENABLED);

        // DEBUG
      #ifdef CONFIG_MBEDTLS_DEBUG
        mbedtls_ssl_conf_verify(&m_tls_config, _certificatePrint, NULL);
        mbedtls_ssl_conf_dbg(&m_tls_config, _debugPrint, NULL);
        mbedtls_debug_set_threshold(TLS_DEBUG_LEVEL);
      #endif

        // Set root CA. NOTE: we dont use CRL
        mbedtls_ssl_conf_ca_chain(&m_tls_config, &m_tls_cacert, nullptr);

        // Set random generator. (We could use ATECC508A as random source, possibly with a speed impact)
        mbedtls_ssl_conf_rng(&m_tls_config, mbedtls_ctr_drbg_random, &m_tls_ctr_drbg);

        retval = mbedtls_ssl_setup(&m_tls_context, &m_tls_config);
        if (retval == 0) {
          result = true;
          ESP_LOGD(LOG_TAG, "Setup complete");
        }
        else {
          ESP_LOGE(LOG_TAG, "mbedtls_ssl_setup returned -0x%x\n\n", -retval);
        }
      }
      else {
        ESP_LOGE(LOG_TAG, "mbedtls_ssl_config_defaults returned %d", retval);
      }
    }
    else {
      ESP_LOGE(LOG_TAG, "mbedtls_x509_crt_parse returned -0x%x\n\n", -retval);
    }
  }
  else {
    ESP_LOGE(LOG_TAG, "mbedtls_ctr_drbg_seed returned %d", retval);
  }

  if (!result) {
    m_lastErrorCode = retval;
  }

  return result;
}

bool TLSSocket::connect(const char* host, const int port)
{
  bool result = false;
  int retval;

  ESP_LOGD(LOG_TAG, "Setting hostname for TLS session...");
  /* Hostname set here should match CN in server certificate */
  retval = mbedtls_ssl_set_hostname(&m_tls_context, host);

  if (retval == 0) {
    if (TCPSocket::connect(host, port)) {
      ESP_LOGD(LOG_TAG, "Connected.");

      // Connect socket comm callbacks
      mbedtls_ssl_set_bio(&m_tls_context, this, _tls_send, _tls_recv, nullptr);

      ESP_LOGD(LOG_TAG, "Performing the SSL/TLS handshake...");
      retval = mbedtls_ssl_handshake(&m_tls_context);

      if ((retval == 0)
          || (retval == MBEDTLS_ERR_SSL_WANT_READ)
          || (retval == MBEDTLS_ERR_SSL_WANT_WRITE)) {

        ESP_LOGD(LOG_TAG, "Cipher suite is %s", mbedtls_ssl_get_ciphersuite(&m_tls_context));
        result = true;
      }
      else {
        ESP_LOGE(LOG_TAG, "mbedtls_ssl_handshake returned -0x%x", -retval);
        close();
        m_lastErrorCode = retval;
      }
    }
    else {
      ESP_LOGE(LOG_TAG, "Failed to connect");
      m_lastErrorCode = 0;
      // Could get error code from TCPSocket?
    }
  }
  else {
    ESP_LOGE(LOG_TAG, "mbedtls_ssl_set_hostname returned -0x%x", -retval);
    m_lastErrorCode = retval;
  }

  return result;
}

int TLSSocket::send(const char* data, int length)
{
  int result = 0;
  int retval = mbedtls_ssl_write(&m_tls_context, reinterpret_cast<const unsigned char *>(data), static_cast<size_t>(length));

  if (retval >= 0) {
    result = retval;
  }
  else {
    m_lastErrorCode = retval;
    if ((retval == MBEDTLS_ERR_SSL_WANT_READ) || (retval == MBEDTLS_ERR_SSL_WANT_WRITE)) {
      result = -2; // SOCKET_ERROR_WOULD_BLOCK;
    }
    else {
      result = -1; //SOCKET_ERROR_UNKNOWN;
    }
  }

  return result;
}

int TLSSocket::_tls_send(void *ctx, const unsigned char *buffer, size_t length)
{
  int result = 0;
  // Context is TLSSocket instance
  TLSSocket *sock = static_cast<TLSSocket *>(ctx);
  // Forward to raw socket send
  int retval = sock->TCPSocket::send(reinterpret_cast<const char *>(buffer), static_cast<int>(length));

  if (retval >= 0) {
    result = retval;
  }
  else {
    logError(retval);
    if (retval == ERR_WOULDBLOCK) {
      result = MBEDTLS_ERR_SSL_WANT_WRITE;
    }
    else {
      result = -1;
    }
  }
  return result;
}

int TLSSocket::receive(char* data, int length)
{
  int result = 0;
  int retval = mbedtls_ssl_read(&m_tls_context, reinterpret_cast<unsigned char *>(data), length);

  if (retval >= 0) {
    result = retval;
  }
  else {
    m_lastErrorCode = retval;
    if ((retval == MBEDTLS_ERR_SSL_WANT_READ) || (retval == MBEDTLS_ERR_SSL_WANT_WRITE)) {
      result = -2; // SOCKET_ERROR_WOULD_BLOCK;
    }
    else {
      result = -1; //SOCKET_ERROR_UNKNOWN;
    }
  }

  return result;
}

int TLSSocket::_tls_recv(void *ctx, unsigned char *buffer, size_t length)
{
  int result = 0;
  // Context is TLSSocket instance
  TLSSocket *sock = static_cast<TLSSocket *>(ctx);
  // Forward to raw socket receive
  int retval = sock->TCPSocket::receive(reinterpret_cast<char *>(buffer), static_cast<int>(length));

  if (retval >= 0) {
    result = retval;
  }
  else {
    logError(retval);
    if (retval == ERR_WOULDBLOCK) {
      result = MBEDTLS_ERR_SSL_WANT_READ;
    }
    else {
      result = -1;
    }
  }

  return result;
}

int TLSSocket::receive_all(char* data, int length)
{
  int receivedBytes = 0;
  int bytesAvailable = 0;
  do {
    int retval = receive(&data[receivedBytes], length-receivedBytes);
    if (retval <= 0) {
      break;
    }
    receivedBytes += retval;
    bytesAvailable = mbedtls_ssl_get_bytes_avail(&m_tls_context);
  }
  while ((bytesAvailable > 0 ) && (receivedBytes < length));

  return receivedBytes;
}

void TLSSocket::close()
{
  if (m_isInitialized) {
    if (isConnected()) {
      mbedtls_ssl_close_notify(&m_tls_context);

      TCPSocket::close();
    }

    mbedtls_ssl_free(&m_tls_context);
    mbedtls_ssl_config_free(&m_tls_config);
    mbedtls_x509_crt_free(&m_tls_cacert);
    mbedtls_x509_crt_free(&m_tls_clientcert);
    mbedtls_pk_free(&m_tls_clientpriv);
    mbedtls_ctr_drbg_free(&m_tls_ctr_drbg);
    mbedtls_entropy_free(&m_tls_entropy);
  }
}

int TLSSocket::getLastError() const
{
  if (m_lastErrorCode != 0) {
    logError(m_lastErrorCode);
  }
  return m_lastErrorCode;
}

void TLSSocket::logError(int errorCode)
{
  char buffer[100];
  mbedtls_strerror(errorCode, buffer, sizeof(buffer));
  ESP_LOGE(LOG_TAG, "Last error was: -0x%x - %s", -errorCode, buffer);
}


/* Debug callbacks: */
/**
 * Debug callback for mbed TLS
 * Just prints on the USB serial port
 */
void TLSSocket::_debugPrint(void *ctx, int level, const char *file, int line, const char *str)
{
    const char *p, *basename;

    /* Extract basename from file */
    for(p = basename = file; *p != '\0'; p++) {
        if(*p == '/' || *p == '\\') {
            basename = p + 1;
        }
    }

    printf("%s:%04d: |%d| %s", basename, line, level, str);
}

/**
 * Certificate verification callback for mbed TLS
 * Here we only use it to display information on each cert in the chain
 */
int TLSSocket::_certificatePrint(void *data, mbedtls_x509_crt *crt, int depth, uint32_t *flags)
{
    char buf[1024];

    printf("\nVerifying certificate at depth %d:\n", depth);
    mbedtls_x509_crt_info(buf, sizeof (buf) - 1, "  ", crt);
    printf("%s", buf);

    if (*flags == 0)
        printf("No verification issue for this certificate\n");
    else
    {
        mbedtls_x509_crt_verify_info(buf, sizeof (buf), "  ! ", *flags);
        printf("%s\n", buf);
    }

    return 0;
}
