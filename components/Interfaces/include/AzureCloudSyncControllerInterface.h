#pragma once
#include <cstdint>
#include "Override.h"

class AzureCloudSyncControllerInterface
{
public:
  virtual void updateComfortTemperatureFromCloud(int32_t temperatureFP) = 0;
  virtual void updateEcoTemperatureFromCloud(int32_t temperatureFP) = 0;
  virtual void updateOverrideFromCloud(const Override & override) = 0;
  virtual void updateButtonLockStatusFromCloud(bool buttonLockEnabled) = 0;

  virtual ~AzureCloudSyncControllerInterface() {}
};

