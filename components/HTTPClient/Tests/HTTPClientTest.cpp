#include <stdint.h>
#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include "SocketMock.h"
#include "SocketDummy.h"
#include "HTTPClient.h"
#include "HTTPBinary.h"
#include "HTTPHead.h"

static SocketMock socket;
static HTTPClient * httpClient;


TEST_GROUP(httpClientSocketMock)
{
  SocketMock socketMock;

  TEST_SETUP()
  {
    httpClient = new HTTPClient(socketMock);
  }

  TEST_TEARDOWN()
  {
    delete httpClient;
    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
  }
};


TEST_GROUP(httpClientSocketDummy)
{
  SocketDummy socketDummy;

  TEST_SETUP()
  {
    httpClient = new HTTPClient(socketDummy);
  }

  TEST_TEARDOWN()
  {
    delete httpClient;
    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
  }
};


TEST(httpClientSocketMock, checkRequestConnectsToCorrectHTTPHostAndPort)
{
  char buffer[1024U];

  mock().expectOneCall("SocketMock::connect")
      .withStringParameter("host", "url.com")
      .withIntParameter("port", 80);

  mock().ignoreOtherCalls();
  httpClient->get("http://url.com/path/to/file", buffer, sizeof(buffer));
}

TEST(httpClientSocketMock, checkRequestConnectsToCorrectHTTPSHostAndPort)
{
  char buffer[1024U];

  mock().expectOneCall("SocketMock::connect")
      .withStringParameter("host", "otherUrl.com")
      .withIntParameter("port", 443);

  mock().ignoreOtherCalls();
  httpClient->get("https://otherUrl.com/path/to/file", buffer, sizeof(buffer));
}

TEST(httpClientSocketMock, checkThatInvalidURLGivesParseError)
{
  char buffer[1024U];
  HTTPResult result = httpClient->get("invalid", buffer, sizeof(buffer));
  CHECK_EQUAL(HTTP_PARSE, result);
}


TEST(httpClientSocketMock, checkThatRequestsAreNonBlockingWithDefaultTimeout)
{
  char buffer[1024U];

  mock().expectOneCall("SocketMock::connect").ignoreOtherParameters();
  mock().expectOneCall("SocketMock::isConnected").ignoreOtherParameters();
  mock().expectOneCall("SocketMock::setBlocking")
      .withBoolParameter("blocking", false)
      .withUnsignedIntParameter("timeout", HTTP_CLIENT_DEFAULT_TIMEOUT);

  mock().ignoreOtherCalls();
  httpClient->get("http://url.com/path/to/file", buffer, sizeof(buffer));
}


TEST(httpClientSocketDummy, testSendingGetRequest)
{
  const char expectedRequest[] =
      "GET /path/to/file HTTP/1.1\r\n"
      "Host: url.com:80\r\n"
      "\r\n";

  char responseBuffer[1U];
  HTTPText textResponse(responseBuffer, sizeof(responseBuffer));
  HTTPResult result = httpClient->get("http://url.com/path/to/file", &textResponse);

  CHECK_TRUE(socketDummy.sendBufferCompare(expectedRequest, sizeof(expectedRequest) - 1U));
}


TEST(httpClientSocketDummy, testSendingPostRequest)
{
  const char expectedRequest[] =
      "POST /path/to/file HTTP/1.1\r\n"
      "Host: url.com:80\r\n"
      "Content-Length: 9\r\n"   /* "post data" length = 9*/
      "Content-Type: text/plain\r\n"
      "\r\n"
      "post data";

  char postBuffer[] = "post data";
  HTTPText postData(postBuffer);

  char responseBuffer[1U];
  HTTPText textResponse(responseBuffer, sizeof(responseBuffer));

  HTTPResult result = httpClient->post("http://url.com/path/to/file", postData, &textResponse);

  CHECK_TRUE(socketDummy.sendBufferCompare(expectedRequest, sizeof(expectedRequest) - 1U));
}


TEST(httpClientSocketDummy, testSendingHeadRequest)
{
  const char expectedRequest[] =
      "HEAD /path/to/file HTTP/1.1\r\n"
      "Host: url.com:80\r\n"
      "\r\n";

  HTTPHead headerResponse;
  HTTPResult result = httpClient->head("http://url.com/path/to/file", &headerResponse);

  CHECK_TRUE(socketDummy.sendBufferCompare(expectedRequest, sizeof(expectedRequest) - 1U));
}


TEST(httpClientSocketDummy, testAddingExtraHeaders)
{
  const char expectedRequest[] =
      "GET /path/to/file HTTP/1.1\r\n"
      "Host: url.com:80\r\n"
      "Range: bytes 0-511\r\n"
      "Accept: text/plain\r\n"
      "\r\n";

  char responseBuffer[1U];
  HTTPText textResponse(responseBuffer, sizeof(responseBuffer));

  const char * customHeader[4U] = { "Range", "bytes 0-511", "Accept", "text/plain" };
  httpClient->customHeaders(customHeader, 2U);

  HTTPResult result = httpClient->get("http://url.com/path/to/file", &textResponse);

  CHECK_TRUE(socketDummy.sendBufferCompare(expectedRequest, sizeof(expectedRequest) - 1U));
}


TEST(httpClientSocketDummy, testReceivingTextResponse)
{
  const char expectedResponse[] =
      "HTTP/1.1 200 OK\r\n"
      "Connection: Keepalive\r\n"
      "Content-Length: 33\r\n"
      "Content-Type: text/plain\r\n"
      "\r\n"
      "This is 33 bytes of response text";

  socketDummy.setReceiveBuffer(expectedResponse, strlen(expectedResponse));
  char responseBuffer[4096U] = {};

  HTTPText textResponse(responseBuffer, sizeof(responseBuffer));
  HTTPResult result = httpClient->get("http://url.com/path/to/file", &textResponse);

  CHECK_EQUAL(HTTP_OK, result);
  CHECK_EQUAL(33U, textResponse.getContentLength());
  CHECK_EQUAL(33U, textResponse.getDataLength());
  MEMCMP_EQUAL("This is 33 bytes of response text", responseBuffer, 33U);
}


TEST(httpClientSocketDummy, testReceivingInvalidResponse)
{
  const char expectedResponse[] =
      "Bogus 200 OK\r\n"
      "Content-Length: 33\r\n";

  socketDummy.setReceiveBuffer(expectedResponse, strlen(expectedResponse));
  char responseBuffer[4096U] = {};

  HTTPText textResponse(responseBuffer, sizeof(responseBuffer));
  HTTPResult result = httpClient->get("http://url.com/path/to/file", &textResponse);

  CHECK_EQUAL(HTTP_PRTCL, result);
  CHECK_EQUAL(0U, textResponse.getContentLength());
  CHECK_EQUAL(0U, textResponse.getDataLength());
}


TEST(httpClientSocketDummy, testReceivingBinaryResponse)
{
  const char expectedResponse[] =
      "HTTP/1.1 200 OK\r\n"
      "Connection: Keepalive\r\n"
      "Content-Length: 9\r\n"
      "Content-Type: application/octet-stream\r\n"
      "\r\n"
      "\tabc\0def\r";

  socketDummy.setReceiveBuffer(expectedResponse, sizeof(expectedResponse) - 1U);
  char responseBuffer[4096U] = {};

  HTTPBinary binaryResponse(responseBuffer, sizeof(responseBuffer));
  HTTPResult result = httpClient->get("http://url.com/path/to/file", &binaryResponse);

  CHECK_EQUAL(HTTP_OK, result);
  CHECK_EQUAL(9U, binaryResponse.getContentLength());
  CHECK_EQUAL(9U, binaryResponse.getDataLength());
  MEMCMP_EQUAL("\tabc\0def\r", responseBuffer, 9U);
}


TEST(httpClientSocketDummy, testReceivingHeadResponse)
{
  const char expectedResponse[] =
      "HTTP/1.1 200 OK\r\n"
      "Content-Length: 123\r\n"
      "\r\n";

  socketDummy.setReceiveBuffer(expectedResponse, sizeof(expectedResponse) - 1U);

  HTTPHead headerResponse;
  HTTPResult result = httpClient->head("http://url.com/path/to/file", &headerResponse);

  CHECK_EQUAL(HTTP_OK, result);
  CHECK_EQUAL(123U, headerResponse.getContentLength());
  CHECK_EQUAL(0U, headerResponse.getDataLength());
}
