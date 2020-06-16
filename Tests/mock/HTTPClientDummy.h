#pragma once

#include <cstring>
#include <cassert>
#include "HTTPText.h"
#include "HTTPClientInterface.h"

class HTTPClientDummy : public HTTPClientInterface
{
public:
  HTTPClientDummy(void)
    : m_responseBufferSize(0U), m_responseResult(HTTP_OK), m_contentLength(0U)
  {
    m_location[0U] = '\0';
    m_customHeaderKey[0U] = '\0';
    m_customHeaderValue[0U] = '\0';
  }

  virtual void customHeaders(const char** headers, size_t pairs)
  {
    assert(pairs == 1U);  /* Being lazy */
    assert(strnlen(headers[0U], CUSTOM_HEADER_SIZE) < CUSTOM_HEADER_SIZE);
    assert(strnlen(headers[1U], CUSTOM_HEADER_SIZE) < CUSTOM_HEADER_SIZE);

    strncpy(m_customHeaderKey, headers[0U], CUSTOM_HEADER_SIZE);
    strncpy(m_customHeaderValue, headers[1U], CUSTOM_HEADER_SIZE);
  }

  virtual HTTPResult get(const char* url, IHTTPDataIn* pDataIn, int timeout = HTTP_CLIENT_DEFAULT_TIMEOUT)
  {
    if (m_responseBufferSize > 0U)
    {
      pDataIn->write(m_responseBuffer, m_responseBufferSize);
    }

    pDataIn->setDataLen(m_contentLength);
    return m_responseResult;
  }

  virtual HTTPResult get(const char* url, char* result, size_t maxResultLen, int timeout = HTTP_CLIENT_DEFAULT_TIMEOUT)
  {
    HTTPText str(result, maxResultLen);
    return get(url, &str, timeout);
  }

  virtual HTTPResult post(const char* url, const IHTTPDataOut& dataOut, IHTTPDataIn* pDataIn, int timeout = HTTP_CLIENT_DEFAULT_TIMEOUT)
  {
    /* We only care about the data to be returned, so we use get() to avoid duplicate code */
    return get(url, pDataIn, timeout);
  }

  virtual HTTPResult put(const char* url, const IHTTPDataOut& dataOut, IHTTPDataIn* pDataIn, int timeout = HTTP_CLIENT_DEFAULT_TIMEOUT)
  {
    return m_responseResult;
  }

  virtual HTTPResult del(const char* url, IHTTPDataIn* pDataIn, int timeout = HTTP_CLIENT_DEFAULT_TIMEOUT)
  {
    return m_responseResult;
  }

  virtual HTTPResult head(const char* url, IHTTPDataIn* pDataIn, int timeout = HTTP_CLIENT_DEFAULT_TIMEOUT)
  {
    pDataIn->setDataLen(m_contentLength);
    return m_responseResult;
  }

  virtual int getHTTPResponseCode(void)
  {
    return 0;
  }

  virtual void setMaxRedirections(int i = 1) {}

  virtual const char * getLocation(void)
  {
    return m_location;
  }

  /* Test helpers */
  void setResponseResult(HTTPResult result)
  {
    m_responseResult = result;
  }

  void setResponseData(const char* payLoad, uint32_t payLoadSize, uint32_t contentLength)
  {
    assert(payLoadSize <= BUFFER_SIZE);

    if (payLoadSize > 0U)
    {
      memcpy(m_responseBuffer, payLoad, payLoadSize);
    }

    m_responseBufferSize = payLoadSize;
    m_contentLength = contentLength;
  }

  const char* getCustomHeaderKey(void) const
  {
    return m_customHeaderKey;
  }

  const char* getCustomHeaderValue(void) const
  {
    return m_customHeaderValue;
  }

private:
  static const uint32_t BUFFER_SIZE = 4096U;
  char m_responseBuffer[BUFFER_SIZE];
  uint32_t m_responseBufferSize;
  uint32_t m_contentLength;

  char m_location[1U];
  HTTPResult m_responseResult;

  static const uint32_t CUSTOM_HEADER_SIZE = 128U;
  char m_customHeaderKey[CUSTOM_HEADER_SIZE];
  char m_customHeaderValue[CUSTOM_HEADER_SIZE];
};
