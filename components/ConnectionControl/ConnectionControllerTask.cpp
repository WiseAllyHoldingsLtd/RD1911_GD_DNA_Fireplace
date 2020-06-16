#include "TaskProperties.h"
#include "ConnectionControllerTask.h"

#include "EspCpp.hpp"

namespace
{
  const char LOG_TAG[] = "ConnectionController";

  const char *STATE_STRINGS[] = {
      "NOT_CONNECTED",
      "WIFI_CONNECTING",
      "WIFI_CONNECTED",
      "DISCOVERY_CONNECTING",
      "DISCOVERY_CONNECTED",
      "IOTHUB_CONNECTING",
      "IOTHUB_CONNECTED",
      "ACTIVATING_WIFI_SETUP",
      "WAITING_FOR_BLE_OFF",
      "FW_UPGRADE_IN_PROGRESS",
      "FACTORY_RESET"
  };
  const char UNDEFINED_STATE_STRING[] = "Undefined state";

  const char *stateToString(ConnectionState::Enum state)
  {
    switch ( state )
    {
    case ConnectionState::NotConnected:
    case ConnectionState::WifiConnecting:
    case ConnectionState::WifiConnected:
    case ConnectionState::DiscoveryConnecting:
    case ConnectionState::DiscoveryConnected:
    case ConnectionState::IotHubConnecting:
    case ConnectionState::IotHubConnected:
    case ConnectionState::ActivatingWifiSetup:
    case ConnectionState::WaitingForBleOff:
    case ConnectionState::FwUpgradeInProgress:
    case ConnectionState::FactoryReset:
      return STATE_STRINGS[state];
    default:
      return UNDEFINED_STATE_STRING;
    }
  }
}

ConnectionControllerTask::ConnectionControllerTask(
  WifiDriverInterface &wifiDriver,
  CloudServiceInterface &cloudService,
  AzureDriverInterface &azureDriver,
  BTWifiSetupDriverInterface &btWifiSetupDriver,
  SettingsInterface &settings,
  QueueInterface<ConnectionStatusChangeRequest::Enum> &connStatusChangeRequestQueue,
  QueueInterface<ConnectionStrings_t> &connectionStringQueue,
  FirmwareUpgradeTaskInterface &fwUpgradeTask,
  SoftwareResetDriverInterface &resetDriver,
  TimerDriverInterface &timerDriver,
  QueueInterface<WifiConnectionStatus::Enum> &wifiSetupResultQueue):
m_state(ConnectionState::NotConnected),
m_stateChangedInCurrentIteration(false),
m_numIotHubTries(0u),
m_fwUpgradeReady(false),
m_bluetoothShouldBeActive(false),
m_stateEnteredTimestamp(0u),
m_wifiDriver(wifiDriver),
m_cloudService(cloudService),
m_azureDriver(azureDriver),
m_btWifiSetupDriver(btWifiSetupDriver),
m_settings(settings),
m_connectionStatusChangeRequestQueue(connStatusChangeRequestQueue),
m_connectionStringQueue(connectionStringQueue),
m_fwUpgradeTask(fwUpgradeTask),
m_resetDriver(resetDriver),
m_timerDriver(timerDriver),
m_wifiSetupResultQueue(wifiSetupResultQueue)
{
  m_stateEnteredTimestamp = m_timerDriver.getTimeSinceBootMS();
}

bool ConnectionControllerTask::startTask()
{
  return start("ConnectionController", TaskStackSize::connectionController, TaskPriority::connectionController);
}

void ConnectionControllerTask::run(bool eternalLoop)
{
  do
  {
    m_stateChangedInCurrentIteration = false;

    ConnectionState::Enum state = getState();
    const char *stateString = stateToString(state);
    ESP_LOGD(LOG_TAG, "ConnectionController currently in state %s", stateString);

    if ( m_settings.getForgetMeState() == ForgetMeState::resetConfirmed && state != ConnectionState::FactoryReset )
    {
      setState(ConnectionState::FactoryReset);
    }
    else
    {
      respondToWifiSetupResults();

      ConnectionStatusChangeRequest::Enum request;
      if ( m_connectionStatusChangeRequestQueue.pop(request, 0u) )
      {
        switch ( request )
        {
        case ConnectionStatusChangeRequest::noChange:
          ESP_LOGV(LOG_TAG, "ConnectionStatusChangeRequest::noChange received, ignoring");
          break;
        case ConnectionStatusChangeRequest::bluetoothOff:
          m_bluetoothShouldBeActive = false;
          if ( m_btWifiSetupDriver.isGattServiceRunning() )
          {
            m_btWifiSetupDriver.stopGattService();
          }
          if ( m_fwUpgradeReady )
          {
            setState(ConnectionState::FwUpgradeInProgress);
          }
          else
          {
            setState(ConnectionState::NotConnected);
          }
          break;
        case ConnectionStatusChangeRequest::bluetoothOn:
          if ( !m_fwUpgradeReady )
          {
            m_bluetoothShouldBeActive = true;
            setState(ConnectionState::ActivatingWifiSetup);
          }
          break;
        case ConnectionStatusChangeRequest::fwUpgradeReady:
          ESP_LOGI(LOG_TAG, "ConnectionStatusChangeRequest::fwUpgradeReady received...");
          m_fwUpgradeReady = true;
          if ( !m_bluetoothShouldBeActive )
          {
            setState(ConnectionState::FwUpgradeInProgress);
          }
          break;
        default:
          ESP_LOGW(LOG_TAG, "Received unsupported status change request: %u",
              static_cast<uint32_t>(request));
          break;
        }
      }
    }

    if ( !m_stateChangedInCurrentIteration )
    {
      switch ( state )
      {
      case ConnectionState::NotConnected:
        updateNotConnected();
        break;
      case ConnectionState::WifiConnecting:
        updateWifiConnecting();
        break;
      case ConnectionState::WifiConnected:
        updateWifiConnected();
        break;
      case ConnectionState::DiscoveryConnecting:
        updateDiscoveryConnecting();
        break;
      case ConnectionState::DiscoveryConnected:
        updateDiscoveryConnected();
        break;
      case ConnectionState::IotHubConnecting:
        updateIotHubConnecting();
        break;
      case ConnectionState::IotHubConnected:
        updateIotHubConnected();
        break;
      case ConnectionState::ActivatingWifiSetup:
        updateActivatingWifiSetup();
        break;
      case ConnectionState::WaitingForBleOff:
        updateWaitingForBleOff();
        break;
      case ConnectionState::FwUpgradeInProgress:
        updateFwUpgradeInProgress();
        break;
      case ConnectionState::FactoryReset:
        updateFactoryReset();
        break;
      default:
        ESP_LOGE(LOG_TAG, "Undefined state (this should never happen!): %u",
            static_cast<uint32_t>(state));
        setState(ConnectionState::NotConnected);
        break;
      }

      delayMS(Constants::CONNECTION_CONTROLLER_PERIOD);
    }
    else
    {
      delayMS(50u);
    }
  } while ( eternalLoop );
}

ConnectionState::Enum ConnectionControllerTask::getState() const
{
  return m_state;
}

void ConnectionControllerTask::setState(ConnectionState::Enum newState)
{
  ESP_LOGI(LOG_TAG, "[%s] -> [%s]",
      stateToString(m_state), stateToString(newState));
  onExitState(m_state);
  m_state = newState;
  onEnterState(m_state);

  m_stateChangedInCurrentIteration = true;
}

void ConnectionControllerTask::onEnterState(ConnectionState::Enum state)
{
  switch ( state )
  {
  case ConnectionState::IotHubConnecting:
    m_numIotHubTries = 0u;
    break;
  default:
    break;
  }

  // Any state can use this to see how long the state has been active
  m_stateEnteredTimestamp = m_timerDriver.getTimeSinceBootMS();
}

void ConnectionControllerTask::onExitState(ConnectionState::Enum state)
{

}

void ConnectionControllerTask::updateNotConnected()
{
  if ( !m_wifiDriver.isConnectedToWlan() )
  {
    if ( !m_cloudService.isPaused() )
    {
      if ( this->getTimeSinceEnterState() > Constants::CLOUD_SERVICE_PAUSE_TIMEOUT )
      {
        ESP_LOGW(LOG_TAG, "Waited too long for cloud service to pause, rebooting");
        m_resetDriver.reset();
      }

      m_cloudService.requestPause();
    }
    else if ( !m_azureDriver.isIdle() )
    {
      m_azureDriver.requestShutdown();

      if (m_timerDriver.getTimeSinceTimestampMS(m_stateEnteredTimestamp) > Constants::AZURE_MAX_SHUTDOWN_WAIT_TIME_MS)
      {
        ESP_LOGI(LOG_TAG, "Waited too long in NotConnected for AzureDriver to stop. Giving up, trying to reconnect to Wifi anyway.");
        setState(ConnectionState::WifiConnecting);
      }
    }
    else
    {
      // TODO if m_wifiDriver.connectionConfigured? Or look in settings?
      setState(ConnectionState::WifiConnecting);
    }
  }
  else
  {
    ESP_LOGW(LOG_TAG, "State not in sync: wifi connected but in state NotConnected");
    setState(ConnectionState::WifiConnected);
  }
}

void ConnectionControllerTask::updateWifiConnecting()
{
  if ( m_wifiDriver.isConnectedToWlan() )
  {
    setState(ConnectionState::WifiConnected);
  } else
  {
    bool result = m_wifiDriver.connectToWlan();
    if ( result )
    {
      setState(ConnectionState::WifiConnected);
    }
    else
    {
      ESP_LOGE(LOG_TAG, "Failed to connect to wlan");
      if ( m_btWifiSetupDriver.isGattServiceRunning() )
      {
        // TODO find out why connectToWlan failed
        // - incorrect password? Not discovered? No ip?
        m_btWifiSetupDriver.setConnectionStatus(
            WifiConnectionStatus::WIFI_NOT_DISCOVERED, "Error");
        setState(ConnectionState::ActivatingWifiSetup);
      }
    }
  }
}

void ConnectionControllerTask::updateWifiConnected()
{
  if ( m_wifiDriver.isConnectedToWlan() )
  {
    setState(ConnectionState::DiscoveryConnecting);
  }
  else
  {
    setState(ConnectionState::NotConnected);
  }
}

void ConnectionControllerTask::updateDiscoveryConnecting()
{
  if ( m_wifiDriver.isConnectedToWlan() )
  {
    if ( m_cloudService.isPaused() )
    {
      if ( !m_cloudService.requestUnpause() )
      {
        ESP_LOGW(LOG_TAG, "Failed to request unpause of cloud service");
      }
    }
    else if ( m_cloudService.wasDiscoverySuccessfulSinceLastRestart() )
    {
      if ( m_btWifiSetupDriver.isGattServiceRunning() )
      {
        setState(ConnectionState::WaitingForBleOff);
      }
      if ( m_cloudService.getCurrentHealth() == Constants::HEALTHCHECK_FULL_HEALTH )
      {
        setState(ConnectionState::DiscoveryConnected);
      }
    }
  }
  else
  {
    setState(ConnectionState::NotConnected);
  }
}

void ConnectionControllerTask::updateDiscoveryConnected()
{
  if ( m_wifiDriver.isConnectedToWlan() )
  {
    setState(ConnectionState::IotHubConnecting);
  }
  else
  {
    setState(ConnectionState::NotConnected);
  }
}

void ConnectionControllerTask::updateIotHubConnecting()
{
  uint64_t timeInThisState = getTimeSinceEnterState();
  if ( timeInThisState > Constants::IOTHUB_CONNECT_TIMEOUT_MS )
  {
    ESP_LOGW(LOG_TAG, "In state IoTHubConnecting for too long, rebooting");
    m_resetDriver.reset();
  }

  if ( m_wifiDriver.isConnectedToWlan() )
  {
    while ( m_connectionStringQueue.numWaitingItems() > 0 )
    {
      m_connectionStringQueue.pop(m_connectionStrings, 0u);
    }

    auto status = m_azureDriver.getStatus();
    switch ( status )
    {
    case CloudStatus::Connecting:
      // TODO timeout -> reboot? If connecting for too long, it has probably
      // frozen
      break;
    case CloudStatus::Connected:
      setState(ConnectionState::IotHubConnected);
      break;
    case CloudStatus::Error:
    case CloudStatus::Idle:
    case CloudStatus::Stopped:
      switch ( m_numIotHubTries )
      {
      case 0u:
        m_azureDriver.connect(m_connectionStrings.primary);
        m_numIotHubTries += 1u;
        break;
      case 1u:
        m_azureDriver.connect(m_connectionStrings.secondary);
        m_numIotHubTries += 1u;
        break;
      default:
        // GD IoT Field node and Cloud interactions_v1.2.docx says reboot
        // when unable to connect to IoT Hub.
        ESP_LOGE(LOG_TAG, "Failed to connect using both primary and secondary connection string");
        break;
      }
      break;
    default:
      break;
    }
  }
  else
  {
    setState(ConnectionState::NotConnected);
  }
}

void ConnectionControllerTask::updateIotHubConnected()
{
  if ( m_wifiDriver.isConnectedToWlan() )
  {
    if ( m_azureDriver.isConnected() )
    {

      // This implements the reliability and robustness procedure suggested by Allan
      if ( m_azureDriver.isUnableToSend() )
      {
        ESP_LOGW(LOG_TAG, "Resetting software because azure driver is unable to send messages");
        m_resetDriver.reset();
      }

      if ( m_cloudService.getCurrentHealth() < Constants::HEALTHCHECK_FULL_HEALTH )
      {
        m_azureDriver.requestShutdown();
        setState(ConnectionState::DiscoveryConnecting);
      }
    }
    else
    {
      m_azureDriver.requestShutdown();
      setState(ConnectionState::DiscoveryConnecting);
      // TODO reboot on disconnect from iot hub according to specs?
    }
  }
  else
  {
    shutdownAzureDriverAndCloudService();

    setState(ConnectionState::NotConnected);
  }
}

void ConnectionControllerTask::updateActivatingWifiSetup()
{
  bool success = false;
  bool waitingForReconnectCmd = false;

  if ( !m_azureDriver.isIdle() )
  {
    ESP_LOGI(LOG_TAG, "AzureDriver isn't idle, requesting shutdown");
    m_azureDriver.requestShutdown();
  }
  else if ( !m_cloudService.isPaused() )
  {
    ESP_LOGI(LOG_TAG, "CloudService isn't paused, requesting pause");
    m_cloudService.requestPause();
  }
  else if ( m_wifiDriver.isConnectedToWlan() )
  {
    ESP_LOGI(LOG_TAG, "Not disconnected from wlan, requesting disconnect");
    m_wifiDriver.disconnectFromWlan();
  }
  else if ( !m_btWifiSetupDriver.isGattServiceRunning() )
  {
    ESP_LOGI(LOG_TAG, "WifiSetup GATT service not running, requesting start");
    success = m_btWifiSetupDriver.startGattService();
    if ( !success )
    {
      ESP_LOGE(LOG_TAG, "Failed to start wifi setup gatt service");
    }
  }
  else
  {
    waitingForReconnectCmd = true;
    if ( m_btWifiSetupDriver.waitForReconnectCmdReceived(100u) )
    {
      ESP_LOGI(LOG_TAG, "Reconnect command received over BLE, attempting to reconnect");

      char ssid[Constants::WIFI_SSID_SIZE_MAX] = {};
      char password[Constants::WIFI_PASSWORD_SIZE_MAX] = {};

      m_btWifiSetupDriver.getWifiSSID(ssid, sizeof(ssid));
      m_btWifiSetupDriver.getWifiPassword(password, sizeof(password));

      uint32_t staticIp = m_btWifiSetupDriver.getStaticIp();
      uint32_t staticNetmask = m_btWifiSetupDriver.getStaticNetmask();
      uint32_t staticGateway = m_btWifiSetupDriver.getStaticGateway();
      uint32_t staticDns1 = m_btWifiSetupDriver.getStaticDns1();
      uint32_t staticDns2 = m_btWifiSetupDriver.getStaticDns2();

      m_btWifiSetupDriver.registerReconnectCmdConsumed();

      m_settings.setWifiSSID(ssid);
      m_settings.setWifiPassword(password);

      m_settings.setStaticIp(staticIp);
      m_settings.setStaticNetmask(staticNetmask);
      m_settings.setStaticGateway(staticGateway);
      m_settings.setStaticDns1(staticDns1);
      m_settings.setStaticDns2(staticDns2);

      success = m_wifiDriver.configureConnection(ssid, password);
      if ( success )
      {
        if ( staticIp != 0u )
        {
          ESP_LOGI(LOG_TAG, "Configuring static ip");
          success = m_wifiDriver.configureStaticIp(staticIp,
                                                   staticNetmask,
                                                   staticGateway,
                                                   staticDns1,
                                                   staticDns2);
          m_btWifiSetupDriver.setStaticIpInUse();
        }
        else
        {
          ESP_LOGI(LOG_TAG, "Configuring dynamic ip");
          success = m_wifiDriver.configureDynamicIp();
          m_btWifiSetupDriver.setDynamicIpInUse();
        }

        if ( !success )
        {
          ESP_LOGE(LOG_TAG, "Configuring static/dynamic ip failed");
          // TODO error reporting to app
          // sendWifiResultOverBle();
        }

        setState(ConnectionState::NotConnected);
      }
      else
      {
        ESP_LOGE(LOG_TAG, "Failed to configure wifi connection");
        // TODO what to do in this situation? report error to phone and give up?
      }
    }
  }

  uint64_t timeInThisState = getTimeSinceEnterState();
  if ( !waitingForReconnectCmd && timeInThisState > Constants::ACTIVATING_WIFI_SETUP_TIMEOUT_MS )
  {
    ESP_LOGE(LOG_TAG, "Activating wifi setup took too long, rebooting");
    m_resetDriver.reset();
  }
}

void ConnectionControllerTask::updateWaitingForBleOff()
{
  // (do nothing - queue is polled in the run-loop of this task)
}

void ConnectionControllerTask::updateFwUpgradeInProgress()
{
  uint64_t timeInFwUpgrade = getTimeSinceEnterState();
  if ( timeInFwUpgrade >= Constants::FW_UPGRADE_TIMEOUT )
  {
    ESP_LOGE(LOG_TAG, "Firmware upgrade has taken too long (%u), resetting firmware", static_cast<uint32_t>(timeInFwUpgrade));
    m_resetDriver.reset();
  }

  if ( !m_azureDriver.isIdle() )
  {
    m_azureDriver.requestShutdown();
  }
  else if ( !m_cloudService.isPaused() )
  {
    m_cloudService.requestPause();
  }
  else if ( !m_wifiDriver.isConnectedToWlan() )
  {
    m_wifiDriver.connectToWlan();
  } else
  {
    auto fwUpgradeState = m_fwUpgradeTask.getCurrentState();
    switch ( fwUpgradeState )
    {
    case FirmwareUpgradeState::notRequested:
      m_fwUpgradeTask.startTask();
      break;
    case FirmwareUpgradeState::started:
      ESP_LOGV(LOG_TAG, "Firmware upgrade started, should transition to 'inProgress' soon");
      break;
    case FirmwareUpgradeState::inProgress:
      ESP_LOGV(LOG_TAG, "Firmware upgrade in progress...");
      break;
    case FirmwareUpgradeState::completed:
      ESP_LOGI(LOG_TAG, "Firmware upgrade completed! Rebooting in a few seconds ...");
      delayMS(Constants::FW_UPGRADE_REBOOT_DELAY_MS);
      m_resetDriver.reset();
      break;
    case FirmwareUpgradeState::failed:
      ESP_LOGE(LOG_TAG, "Firmware upgrade failed! Rebooting in a few seconds ...");
      delayMS(Constants::FW_UPGRADE_REBOOT_DELAY_MS);
      m_resetDriver.reset();
      break;
    default:
      ESP_LOGE(LOG_TAG, "Firmware upgrade task is in an unknown state! Rebooting in a few seconds");
      delayMS(Constants::FW_UPGRADE_REBOOT_DELAY_MS);
      m_resetDriver.reset();
      break;
    }
  }
}

void ConnectionControllerTask::updateFactoryReset()
{
  if ( m_settings.getForgetMeState() == ForgetMeState::noResetRequested )
  {
    // Assume user cancelled operation using back button
    setState(ConnectionState::NotConnected);
  }
  else
  {
    if ( !m_azureDriver.isIdle() && this->getTimeSinceEnterState() <= Constants::CONNECTION_CONTROLLER_DISCONNECTING_TIMEOUT1_MS )
    {
      m_azureDriver.requestShutdown();
    }
    else if ( !m_cloudService.isPaused() && this->getTimeSinceEnterState() <= Constants::CONNECTION_CONTROLLER_DISCONNECTING_TIMEOUT1_MS )
    {
      m_cloudService.requestPause();
    }
    else if ( m_wifiDriver.isConnectedToWlan() && this->getTimeSinceEnterState() <= Constants::CONNECTION_CONTROLLER_DISCONNECTING_TIMEOUT2_MS )
    {
      m_wifiDriver.disconnectFromWlan();
    }
    else
    {
      // Note: factoryReset also resets the ForgetMeState to noResetRequested
      ESP_LOGI(LOG_TAG, "Performing factory reset");
      m_settings.factoryReset();
      m_wifiDriver.configureConnection(Constants::DEFAULT_WIFI_SSID, Constants::DEFAULT_WIFI_PASSWORD);
      ESP_LOGI(LOG_TAG, "Factory reset performed");

      setState(ConnectionState::NotConnected);
    }
  }
}

void ConnectionControllerTask::restartAzureDriverAndCloudService()
{
  m_azureDriver.requestShutdown();
  m_cloudService.requestPause();
  m_cloudService.waitUntilPausedOrUnpaused(true);
  m_cloudService.requestUnpause();
}

void ConnectionControllerTask::shutdownAzureDriverAndCloudService()
{
  m_cloudService.requestPause();
  m_azureDriver.requestShutdown();
}

uint64_t ConnectionControllerTask::getTimeSinceEnterState() const
{
  return m_timerDriver.getTimeSinceTimestampMS(m_stateEnteredTimestamp);
}

void ConnectionControllerTask::respondToWifiSetupResults()
{
  WifiConnectionStatus::Enum wifiResult;

  if ( m_wifiSetupResultQueue.pop(wifiResult, 0u) )
  {
    if ( m_btWifiSetupDriver.isGattServiceRunning() )
    {
      if ( wifiResult == WifiConnectionStatus::FULLY_FUNCTIONAL )
      {
        m_btWifiSetupDriver.setConnectionStatus(wifiResult, "Ok!");
      }
      else
      {
        m_btWifiSetupDriver.setConnectionStatus(wifiResult, "Error");
        setState(ConnectionState::ActivatingWifiSetup);
      }
    }
  }
}
