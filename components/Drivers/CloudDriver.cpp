#include "EspCpp.hpp"

#include "TLSSocket.h"
#include "HTTPClient.h"

#include "CloudDriver.hpp"

namespace {
  const char LOG_TAG[] = "CloudDriver";
}

CloudDriver::CloudDriver(const CryptoDriverInterface &crypto)
: m_cryptoDriver(crypto)
{

}

bool CloudDriver::postRequest(const char * url, IHTTPDataOut &sendData, IHTTPDataIn &receiveData) const
{
  bool result = false;
  TLSSocket sock;

  if (!sock.setup(m_cryptoDriver.getClientCertificate(), m_cryptoDriver.getClientCertificateSize()))
  {
    ESP_LOGE(LOG_TAG, "Failed to setup TLS sock");
  }
  else
  {
    HTTPClient https(sock);
    int ret = https.post(url, sendData, &receiveData);
    if (ret == HTTP_OK)
    {
      result = true;
    }
    else
    {
      ESP_LOGE(LOG_TAG, "HTTP req Failed");
      sock.getLastError();
    }
  }
  return result;
}


bool CloudDriver::getRequest(const char * url, IHTTPDataIn &receiveData) const
{
  const char *noHeader[] = { nullptr, nullptr };
  return getRequestWithCustomHeader(url, receiveData, noHeader);
}


bool CloudDriver::headRequest(const char * url, IHTTPDataIn &receiveData) const
{
  bool result = false;
  TLSSocket sock;

  if (!sock.setup(m_cryptoDriver.getClientCertificate(), m_cryptoDriver.getClientCertificateSize()))
  {
    ESP_LOGE(LOG_TAG, "Failed to setup TLS sock");
  }
  else
  {
    HTTPClient https(sock);
    int ret = https.head(url, &receiveData);

    if (ret == HTTP_OK)
    {
      result = true;
    }
    else
    {
      ESP_LOGE(LOG_TAG, "HTTP req Failed");
      sock.getLastError();
    }
  }
  return result;
}


bool CloudDriver::getRequestWithCustomHeader(const char * url, IHTTPDataIn &receiveData, const char * (&customHeader)[2U]) const
{
  bool result = false;
  TLSSocket sock;

  if (!sock.setup(m_cryptoDriver.getClientCertificate(), m_cryptoDriver.getClientCertificateSize()))
  {
    ESP_LOGE(LOG_TAG, "Failed to setup TLS sock");
  }
  else
  {
    HTTPClient https(sock);

    if ((customHeader[0U] != nullptr) && (customHeader[1U] != nullptr))
    {
      https.customHeaders(customHeader, 1U);
    }

    int ret = https.get(url, &receiveData);

    if (ret == HTTP_OK)
    {
      result = true;
    }
    else
    {
      ESP_LOGE(LOG_TAG, "HTTP req Failed");
      sock.getLastError();
    }
  }
  return result;
}
