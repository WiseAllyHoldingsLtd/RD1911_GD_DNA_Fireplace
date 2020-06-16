
#pragma once

#include "CppUTestExt\MockSupport.h"
#include "AzureDriverInterface.h"

#define NAME(method) "AzureDriverMock::" method

class AzureDriverMock : public AzureDriverInterface
{
public:
  virtual bool isConnected() const
  {
    return mock().actualCall(NAME("isConnected")).returnBoolValueOrDefault(false);
  }

  virtual bool isIdle() const
  {
    return mock().actualCall(NAME("isIdle")).returnBoolValueOrDefault(true);
  }

  virtual CloudStatus::Enum getStatus() const
  {
    return static_cast<CloudStatus::Enum>(mock().actualCall(NAME("getStatus")).returnIntValue());
  }

  virtual void requestShutdown()
  {
    mock().actualCall(NAME("requestShutdown"));
  }

  virtual bool isUnableToSend() const
  {
    mock().actualCall(NAME("isUnableToSend")).returnBoolValueOrDefault(false);
  }

  virtual bool connect(const char (&connectionString)[Constants::CLOUD_CONNECTION_STRING_MAX_LENGTH])
  {
    return mock().actualCall(NAME("connect"))
        .withStringParameter("connectionString", connectionString)
        .returnBoolValueOrDefault(true);
  }
};

#undef NAME
