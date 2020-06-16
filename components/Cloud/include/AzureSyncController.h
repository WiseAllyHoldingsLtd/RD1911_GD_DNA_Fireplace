#pragma once

#include "AzureCloudSyncControllerInterface.h"
#include "SettingsInterface.h"
#include "DeviceMetaInterface.h"
#include "ConnectionControllerTaskInterface.h"
#include "AsyncMessageQueueInterface.h"
#include "ParameterDataResponse.h"
#include "Override.h"


class AzureSyncController : public AzureCloudSyncControllerInterface
{
public:
  AzureSyncController(SettingsInterface & settings,
                      DeviceMetaInterface &deviceMeta,
                      AsyncMessageQueueWriteInterface &asyncQueue);

  virtual void updateComfortTemperatureFromCloud(int32_t temperatureFP);
  virtual void updateEcoTemperatureFromCloud(int32_t temperatureFP);
  virtual void updateOverrideFromCloud(const Override & override);
  virtual void updateButtonLockStatusFromCloud(bool buttonLockEnabled);

  void updateFromSettings(void);

private:
  bool addDiscardOverrideParameter(ParameterDataResponse & paramData) const;
  bool addNowOverrideParameter(CloudTimerHeatingMode::Enum mode, ParameterDataResponse & paramData) const;
  bool addPayloadToAsyncQueue(const ParameterDataResponse & paramData);
  bool addForgetMeFrameToAsyncQueue();

  SettingsInterface & m_settings;
  DeviceMetaInterface & m_deviceMeta;
  AsyncMessageQueueWriteInterface & m_asyncQueue;

  int32_t m_cachedComfortSetpoint;
  int32_t m_cachedEcoSetpoint;
  Override m_cachedOverride;
  bool m_cachedButtonLockEnabled;

  bool m_forgetMeFrameSent;
};
