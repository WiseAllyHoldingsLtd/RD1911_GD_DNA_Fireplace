#pragma once

#include "CppUTestExt\MockSupport.h"
#include "ConnectionControllerTaskInterface.h"

#define NAME(method) "ConnectionControllerTaskMock::" method


class ConnectionControllerTaskMock : public ConnectionControllerTaskInterface
{
public:

  virtual bool startTask()
  {
    return mock().actualCall(NAME("startTask")).returnBoolValueOrDefault(true);
  }

  virtual ConnectionState::Enum getState(void) const
  {
    return static_cast<ConnectionState::Enum>(mock().actualCall(NAME("getState")).returnUnsignedIntValue());
  }
};
