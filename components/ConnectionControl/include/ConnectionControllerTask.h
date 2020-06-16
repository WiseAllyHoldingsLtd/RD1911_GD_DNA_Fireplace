#pragma once

#include "Task.hpp"

#include "CloudServiceInterface.h"
#include "WifiDriverInterface.h"
#include "AzureDriverInterface.h"
#include "BTWifiSetupDriverInterface.h"
#include "SettingsInterface.h"
#include "QueueInterface.hpp"
#include "ConnectionStatusChangeRequest.h"
#include "DiscoveryServiceInterface.h"
#include "FirmwareUpgradeTaskInterface.h"
#include "SoftwareResetDriverInterface.h"
#include "TimerDriverInterface.h"
#include "ConnectionControllerTaskInterface.h"


class ConnectionControllerTask : public Task, public ConnectionControllerTaskInterface
{
public:
  ConnectionControllerTask(WifiDriverInterface &wifiDriver,
    CloudServiceInterface &cloudService,
    AzureDriverInterface &azureDriver,
    BTWifiSetupDriverInterface &btWifiSetupDriver,
    SettingsInterface &settings,
    QueueInterface<ConnectionStatusChangeRequest::Enum> &connStatusChangeRequestQueue,
    QueueInterface<ConnectionStrings_t> &connectionStringsQueue,
    FirmwareUpgradeTaskInterface &fwUpgradeTask,
    SoftwareResetDriverInterface &resetDriver,
    TimerDriverInterface &timerDriver,
    QueueInterface<WifiConnectionStatus::Enum> &wifiSetupResultQueue);
                           // TODO add some interface for feeding watchdog
  virtual ~ConnectionControllerTask() = default;
  virtual bool startTask();
  virtual void run(bool eternalLoop);
  virtual ConnectionState::Enum getState() const;
private:
  void setState(ConnectionState::Enum newState);

  void onEnterState(ConnectionState::Enum state);
  void onExitState(ConnectionState::Enum state);

  void updateNotConnected();
  void updateWifiConnecting();
  void updateWifiConnected();
  void updateDiscoveryConnecting();
  void updateDiscoveryConnected();
  void updateIotHubConnecting();
  void updateIotHubConnected();
  void updateActivatingWifiSetup();
  void updateWaitingForBleOff();
  void updateFwUpgradeInProgress();
  void updateFactoryReset();

  void restartAzureDriverAndCloudService();
  void shutdownAzureDriverAndCloudService();
  uint64_t getTimeSinceEnterState() const;
  void respondToWifiSetupResults();

  ConnectionState::Enum m_state;
  bool m_stateChangedInCurrentIteration;

  uint8_t m_numIotHubTries;
  bool m_fwUpgradeReady;
  bool m_bluetoothShouldBeActive;
  uint64_t m_stateEnteredTimestamp;

  WifiDriverInterface &m_wifiDriver;
  CloudServiceInterface &m_cloudService;
  AzureDriverInterface &m_azureDriver;
  BTWifiSetupDriverInterface &m_btWifiSetupDriver;
  SettingsInterface &m_settings;
  QueueInterface<ConnectionStatusChangeRequest::Enum>
    &m_connectionStatusChangeRequestQueue;
  QueueInterface<ConnectionStrings_t> &m_connectionStringQueue;
  FirmwareUpgradeTaskInterface &m_fwUpgradeTask;
  SoftwareResetDriverInterface &m_resetDriver;
  TimerDriverInterface &m_timerDriver;
  QueueInterface<WifiConnectionStatus::Enum> &m_wifiSetupResultQueue;
  ConnectionStrings_t m_connectionStrings;
};
