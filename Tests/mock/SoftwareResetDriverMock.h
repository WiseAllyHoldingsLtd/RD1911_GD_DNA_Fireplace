#pragma once

#include "CppUTestExt\MockSupport.h"
#include "SoftwareResetDriverInterface.h"

#define NAME(method) "SoftwareResetDriverMock::" method



class SoftwareResetDriverMock : public SoftwareResetDriverInterface
{
public:
  virtual void reset(void)
  {
    mock().actualCall(NAME("reset"));
  }
};
