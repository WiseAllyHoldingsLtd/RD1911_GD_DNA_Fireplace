#pragma once
#include "CppUTestExt\MockSupport.h"
#include "AzureCloudSyncControllerInterface.h"

#define NAME(method) "AzureCloudSyncControllerMock::" method


class AzureCloudSyncControllerMock : public AzureCloudSyncControllerInterface
{
  virtual void updateComfortTemperatureFromCloud(int32_t temperatureFP)
  {
    mock().actualCall(NAME("updateComfortTemperatureFromCloud")).withIntParameter("temperatureFP", temperatureFP);
  }

  virtual void updateEcoTemperatureFromCloud(int32_t temperatureFP)
  {
    mock().actualCall(NAME("updateEcoTemperatureFromCloud")).withIntParameter("temperatureFP", temperatureFP);
  }

  virtual void updateOverrideFromCloud(const Override & override)
  {
    mock().actualCall(NAME("updateOverrideFromCloud")).withParameterOfType("Override", "override", static_cast<const void*>(&override));
  }

  virtual void updateButtonLockStatusFromCloud(bool buttonLockEnabled)
  {
    mock().actualCall(NAME("updateButtonLockStatusFromCloud")).withBoolParameter("buttonLockEnabled", buttonLockEnabled);
  }
};
