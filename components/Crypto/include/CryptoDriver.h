#pragma once

#include "CryptoDriverInterface.h"
#include "Constants.h"

namespace {
  typedef struct {
      uint32_t    device_der_size;
      uint8_t*    device_der;
      uint32_t    device_pem_size;
      uint8_t*    device_pem;
      uint8_t*    device_pubkey;
      uint32_t    signer_der_size;
      uint8_t*    signer_der;
      uint32_t    signer_pem_size;
      uint8_t*    signer_pem;
      uint8_t*    signer_pubkey;
  } atcert_t;
}


class CryptoDriver : public CryptoDriverInterface
{
public:
  CryptoDriver();

  // Default I2C address is provided as pre-shifted in documentation,
  // hence we must shift down to fit our driver's expectation of unshifted address
  static const uint8_t CRYPTO_SLAVE_ADDRESS = (0xC0U >> 1U);

  virtual bool resetAndInit();
  virtual bool isProvisioned() const;
  virtual uint32_t readChipRevision() const;
  virtual bool readSerialNumber(uint8_t (&buffer)[9]) const;
  virtual bool readCertificates();
  virtual const char *getClientCertificate() const;
  virtual size_t getClientCertificateSize() const;
  virtual const char *getSignerCertificate() const;
  virtual size_t getSignerCertificateSize() const;


  // DEBUG
  void DEBUG_testingCode();

protected:
  void freeCerts(atcert_t *cert);
  bool rebuildCerts(atcert_t *cert);
  bool rebuildSignerCert(atcert_t* cert);
  bool rebuildDeviceCert(atcert_t* cert);
  bool buildClientTrustedChain(atcert_t *cert);

private:
  char m_clientCertificate[Constants::CRYPTO_CLIENT_CERT_SIZE_MAX];
  size_t m_clientCertificateSize;
  char m_signerCertificate[Constants::CRYPTO_CLIENT_CERT_SIZE_MAX];
  size_t m_signerCertificateSize;
};
