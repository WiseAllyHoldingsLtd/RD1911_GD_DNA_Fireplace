#pragma once

#include <cstdint>

#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/certs.h"
#include "certificates.h"

#include "TCPSocket.h"


/**
 * A Socket encrypted with mbedTLS
 */
class TLSSocket : public TCPSocket
{
public:
  TLSSocket();
  virtual ~TLSSocket();

  /*
   * Set up necessary structures.
   * Must be called before connect.
   */
  bool setup(const char * clientCertificate=nullptr,
             size_t clientCertSize=0u,
             const char * rootCertificate=x509_root_cert_pem,
             size_t rootCertSize=sizeof(x509_root_cert_pem));
  virtual bool connect(const char* host, const int port);
  virtual int send(const char* data, int length);
  virtual int receive(char* data, int length);
  virtual int receive_all(char* data, int length);
  virtual void close();

  /*
   * Returns (and logs) the last internal error code.
   */
  int getLastError() const;

protected:
  // Receive callback for mbed TLS
  static int _tls_recv(void *ctx, unsigned char *buffer, size_t length);

  // Send callback for mbed TLS
  static int _tls_send(void *ctx, const unsigned char *buffer, size_t length);

  static void logError(int errorCode);

  /* Debug callbacks */
  static void _debugPrint(void *ctx, int level, const char *file, int line, const char *str);
  static int _certificatePrint(void *data, mbedtls_x509_crt *crt, int depth, uint32_t *flags);

private:
  mbedtls_entropy_context m_tls_entropy;
  mbedtls_ctr_drbg_context m_tls_ctr_drbg;
  mbedtls_ssl_context m_tls_context;
  mbedtls_x509_crt m_tls_cacert;
  mbedtls_x509_crt m_tls_clientcert;
  mbedtls_pk_context m_tls_clientpriv;
  mbedtls_ssl_config m_tls_config;
  bool m_isInitialized;
  int m_lastErrorCode;
};

