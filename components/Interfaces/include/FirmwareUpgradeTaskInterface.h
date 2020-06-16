#pragma once

#include "FirmwareUpgradeState.h"

class FirmwareUpgradeTaskInterface
{
public:
  virtual ~FirmwareUpgradeTaskInterface() { }
  virtual void startTask() = 0;
  virtual FirmwareUpgradeState::Enum getCurrentState(void) const = 0;
};
