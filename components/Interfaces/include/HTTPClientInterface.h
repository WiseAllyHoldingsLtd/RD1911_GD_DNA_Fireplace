#pragma once
#include <stddef.h>
#include <stdint.h>

#include "IHTTPData.h"

#define HTTP_CLIENT_DEFAULT_TIMEOUT 15000

/// HTTP client result codes
enum HTTPResult {
    HTTP_OK = 0,        ///<Success
    HTTP_PROCESSING,    ///<Processing
    HTTP_PARSE,         ///<url Parse error
    HTTP_DNS,           ///<Could not resolve name
    HTTP_PRTCL,         ///<Protocol error
    HTTP_NOTFOUND,      ///<HTTP 404 Error
    HTTP_REFUSED,       ///<HTTP 403 Error
    HTTP_ERROR,         ///<HTTP xxx error
    HTTP_TIMEOUT,       ///<Connection timeout
    HTTP_CONN,          ///<Connection error
    HTTP_CLOSED,        ///<Connection was closed by remote host
};


class HTTPClientInterface
{
public:
  virtual ~HTTPClientInterface(void) {}

  virtual void customHeaders(const char** headers, size_t pairs) = 0;
  virtual HTTPResult get(const char* url, IHTTPDataIn* pDataIn, int timeout = HTTP_CLIENT_DEFAULT_TIMEOUT) = 0;
  virtual HTTPResult get(const char* url, char* result, size_t maxResultLen, int timeout = HTTP_CLIENT_DEFAULT_TIMEOUT) = 0;
  virtual HTTPResult post(const char* url, const IHTTPDataOut& dataOut, IHTTPDataIn* pDataIn, int timeout = HTTP_CLIENT_DEFAULT_TIMEOUT) = 0;
  virtual HTTPResult put(const char* url, const IHTTPDataOut& dataOut, IHTTPDataIn* pDataIn, int timeout = HTTP_CLIENT_DEFAULT_TIMEOUT) = 0;
  virtual HTTPResult del(const char* url, IHTTPDataIn* pDataIn, int timeout = HTTP_CLIENT_DEFAULT_TIMEOUT) = 0;
  virtual HTTPResult head(const char* url, IHTTPDataIn* pDataIn, int timeout = HTTP_CLIENT_DEFAULT_TIMEOUT) = 0;

  virtual int getHTTPResponseCode(void) = 0;
  virtual void setMaxRedirections(int i = 1) = 0;
  virtual const char * getLocation(void) = 0;
};
