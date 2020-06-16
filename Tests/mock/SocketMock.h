#pragma once

#include "CppUTestExt\MockSupport.h"
#include "SocketInterface.h"

#define NAME(method) "SocketMock::" method


class SocketMock : public SocketInterface
{
  virtual bool connect(const char* host, const int port)
  {
    return mock().actualCall(NAME("connect"))
        .withStringParameter("host", host)
        .withIntParameter("port", port)
        .returnBoolValueOrDefault(true);
  }

  virtual bool isConnected(void) const
  {
    return mock().actualCall(NAME("isConnected")).returnBoolValueOrDefault(true);
  }

  virtual int send(const char* data, int length)
  {
    return mock().actualCall(NAME("send"))
        .withMemoryBufferParameter("data", reinterpret_cast<const unsigned char*>(data), length)
        .withIntParameter("length", length)
        .returnIntValue();
  }

  virtual int sendAll(const char* data, int length)
  {
    return mock().actualCall(NAME("sendAll"))
        .withMemoryBufferParameter("data", reinterpret_cast<const unsigned char*>(data), length)
        .withIntParameter("length", length)
        .returnIntValue();
  }

  virtual int receive(char* data, int length)
  {
    return mock().actualCall(NAME("receive"))
        .withOutputParameter("data", (void *)data)
        .withIntParameter("length", length)
        .returnIntValue();
  }

  virtual int receiveAll(char* data, int length)
  {
    return mock().actualCall(NAME("receiveAll"))
        .withOutputParameter("data", (void *)data)
        .withIntParameter("length", length)
        .returnIntValue();
  }

  virtual void setBlocking(bool blocking, unsigned int timeout=1500)
  {
    mock().actualCall(NAME("setBlocking"))
        .withBoolParameter("blocking", blocking)
        .withUnsignedIntParameter("timeout", timeout);
  }

  virtual void close()
  {
    mock().actualCall(NAME("close"));
  }
};
