#pragma once

#include <stdint.h>
#include "FreeRTOS.hpp"
#include "Task.hpp"
#include "MutexInterface.hpp"
#include "FirmwareUpgradeState.h"
#include "FirmwareUpgradeControllerInterface.h"
#include "FirmwareUpgradeTaskInterface.h"


class FirmwareUpgradeTask : public Task, public FirmwareUpgradeTaskInterface
{
public:
  FirmwareUpgradeTask(FirmwareUpgradeControllerInterface &controller);
  virtual void run(bool eternalLoop);
  virtual void startTask();
  virtual FirmwareUpgradeState::Enum getCurrentState(void) const;

private:
  FirmwareUpgradeControllerInterface &m_controller;
  FirmwareUpgradeState::Enum m_state;
};
