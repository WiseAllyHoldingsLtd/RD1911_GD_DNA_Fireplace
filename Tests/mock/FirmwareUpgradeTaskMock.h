#pragma once

#include "CppUTestExt\MockSupport.h"

#include "FirmwareUpgradeTaskInterface.h"

#define NAME(method) "FirmwareUpgradeTaskMock::" method



class FirmwareUpgradeTaskMock : public FirmwareUpgradeTaskInterface
{
public:
  virtual void startTask()
  {
    mock().actualCall(NAME("startTask"));
  }

  virtual FirmwareUpgradeState::Enum getCurrentState(void) const
  {
    return static_cast<FirmwareUpgradeState::Enum>(mock().actualCall(NAME("getCurrentState")).returnUnsignedIntValue());
  }
};

#undef NAME
