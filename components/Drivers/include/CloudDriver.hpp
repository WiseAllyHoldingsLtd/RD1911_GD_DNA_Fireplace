#pragma once

#include "CryptoDriverInterface.h"
#include "CloudDriverInterface.h"


class CloudDriver : public CloudDriverInterface
{
public:
  CloudDriver(const CryptoDriverInterface &crypto);
  virtual bool postRequest(const char * url, IHTTPDataOut &sendData, IHTTPDataIn &receiveData) const;
  virtual bool getRequest(const char * url, IHTTPDataIn &receiveData) const;
  virtual bool headRequest(const char * url, IHTTPDataIn &receiveData) const;

  virtual bool getRequestWithCustomHeader(const char * url, IHTTPDataIn &receiveData, const char * (&customHeader)[2U]) const;

private:
  const CryptoDriverInterface &m_cryptoDriver;
};
