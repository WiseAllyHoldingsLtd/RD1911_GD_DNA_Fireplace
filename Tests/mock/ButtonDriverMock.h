#pragma once

#include "CppUTestExt\MockSupport.h"
#include "ButtonDriverInterface.h"

class ButtonDriverMock : public ButtonDriverInterface
{
public:
  virtual ButtonStatus getButtonStatus()
  {
    return *(ButtonStatus*)(mock().actualCall("getButtonStatusB").returnPointerValue());
  }

  virtual void resetButtonStatus(void)
  {
    mock().actualCall("resetButtonStatusB");
  }
};
