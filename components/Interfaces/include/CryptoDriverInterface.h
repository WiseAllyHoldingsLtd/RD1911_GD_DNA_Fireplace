#pragma once

#include <cstdint>
#include <cstddef>


class CryptoDriverInterface
{
public:
  virtual ~CryptoDriverInterface(){};

  virtual bool resetAndInit() = 0;
  virtual bool isProvisioned() const = 0;
  virtual uint32_t readChipRevision() const = 0;
  virtual bool readSerialNumber(uint8_t (&buffer)[9]) const = 0;
  virtual bool readCertificates() = 0;
  virtual const char *getClientCertificate() const = 0;
  virtual size_t getClientCertificateSize() const = 0;
  virtual const char *getSignerCertificate() const = 0;
  virtual size_t getSignerCertificateSize() const = 0;
};
