#pragma once

#include "IHTTPData.h"

class CloudDriverInterface
{
public:
  virtual ~CloudDriverInterface() {}

  virtual bool postRequest(const char * url, IHTTPDataOut &sendData, IHTTPDataIn &receiveData) const = 0;
  virtual bool getRequest(const char * url, IHTTPDataIn &receiveData) const = 0;
  virtual bool headRequest(const char * url, IHTTPDataIn &receiveData) const = 0;

  virtual bool getRequestWithCustomHeader(const char * url, IHTTPDataIn &receiveData, const char * (&customHeader)[2U]) const = 0;
};
