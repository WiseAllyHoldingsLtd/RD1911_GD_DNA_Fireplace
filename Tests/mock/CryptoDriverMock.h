#pragma once

#include "CppUTestExt\MockSupport.h"
#include "CryptoDriverInterface.h"

#define NAME(method) "CryptoDriverMock::" method


class CryptoDriverMock : public CryptoDriverInterface
{
public:
  virtual bool resetAndInit(void)
  {
    return mock().actualCall(NAME("resetAndInit")).returnBoolValueOrDefault(true);
  }

  virtual bool isProvisioned() const
  {
    return mock().actualCall(NAME("isProvisioned")).returnBoolValueOrDefault(true);
  }

  virtual uint32_t readChipRevision() const
  {
    return mock().actualCall(NAME("readChipRevision")).returnUnsignedIntValue();
  }

  virtual bool readSerialNumber(uint8_t (&buffer)[9]) const
  {
    return mock().actualCall(NAME("readSerialNumber"))
        .withOutputParameter("buffer", buffer)
        .returnBoolValueOrDefault(true);
  }

  virtual bool readCertificates()
  {
    return mock().actualCall(NAME("readCertificates")).returnBoolValueOrDefault(true);
  }

  virtual const char *getClientCertificate() const
  {
    return mock().actualCall(NAME("getClientCertificate")).returnStringValue();
  }

  virtual size_t getClientCertificateSize() const
  {
    return mock().actualCall(NAME("getClientCertificateSize")).returnUnsignedIntValue();
  }

  virtual const char *getSignerCertificate() const
  {
    return mock().actualCall(NAME("getSignerCertificate")).returnStringValue();
  }

  virtual size_t getSignerCertificateSize() const
  {
    return mock().actualCall(NAME("getSignerCertificateSize")).returnUnsignedIntValue();
  }
};
