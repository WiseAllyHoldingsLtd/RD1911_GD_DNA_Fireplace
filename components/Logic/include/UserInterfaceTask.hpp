#pragma once

#include "Task.hpp"
#include "DisplayInterface.h"
#include "ButtonDriverInterface.h"
#include "TimerDriverInterface.h"
#include "BTDriverInterface.h"
#include "WifiDriverInterface.h"
#include "AzureDriverInterface.h"
#include "SettingsInterface.h"
#include "DeviceMetaInterface.h"
#include "BacklightDriverInterface.h"
#include "ConnectionStatusChangeRequest.h"
#include "QueueInterface.hpp"
#include "FirmwareUpgradeTask.hpp"


class UserInterfaceTask : public Task
{
public:
  UserInterfaceTask(DisplayInterface &display,
                    ButtonDriverInterface &button,
                    BTDriverInterface &btDriver,
                    const WifiDriverInterface &wifiDriver,
                    const AzureDriverInterface &azureDriver,
                    TimerDriverInterface &timer,
                    SettingsInterface &settings,
                    DeviceMetaInterface &deviceMeta,
                    BacklightDriverInterface &backlight,
                    const FirmwareUpgradeTask & fwTask,
                    QueueInterface<bool> &isHeatElementOnQueue,
                    QueueInterface<ConnectionStatusChangeRequest::Enum> &connStatusChangeQueue);

  virtual void run(bool eternalLoop);

private:
  bool getHeatElementStateFromQueue(bool currentState);

  DisplayInterface &m_display;
  ButtonDriverInterface &m_buttonDriver;
  BTDriverInterface &m_btDriver;
  const WifiDriverInterface &m_wifiDriver;
  const AzureDriverInterface &m_azureDriver;
  TimerDriverInterface &m_timerDriver;
  SettingsInterface &m_settings;
  DeviceMetaInterface &m_meta;
  BacklightDriverInterface &m_backlightDriver;
  const FirmwareUpgradeTask & m_firmwareUpgrade;
  QueueInterface<bool> & m_isHeatElementOnQueue;
  QueueInterface<ConnectionStatusChangeRequest::Enum> &m_btStatusQueue;
};
