#include <cstdio>
#include <cstring>

#include "esp_log.h"
#include "sdkconfig.h"

#include "TaskProperties.h"
#include "Task.hpp"
#include "Queue.hpp"
#include "MainTask.h"

#include "RebootScheduler.h"
#include "HardwareSetup.h"
#include "SoftwareResetDriver.h"
#include "TimerDriver.h"
#include "QTouchDriver.h"
#include "ButtonDriver.h"
#include "DisplayDriver.h"
#include "Display.h"
#include "TWIDriver.h"
#include "Rtc.h"
#include "RegulatorUnit.h"
#include "PersistentStorageDriver.h"
#include "PartitionDriver.h"
#include "PowerOnSelfTest.h"

#include "SettingsStorage.h"
#include "Settings.h"
#include "DeviceMetaStorage.h"
#include "DeviceMeta.h"

#include "BTDriver.h"
#include "BTWifiSetupDriver.h"
#include "FGT.h"
#include "WifiDriver.h"
#include "UIController.h"
#include "ViewModel.h"

#include "ConnectionStatusChangeRequest.h"
#include "FGT.h"

#include "UserInterfaceTask.hpp"
#include "HeatControllerTask.hpp"

#include "OTADriver.h"
#include "HTTPClient.h"
#include "TCPSocket.h"
#include "SystemTestCommunicator.h"
#include "DisplaySystemTest.h"
#include "ButtonSystemTestDriver.h"
#include "SpiDriver.h"
#include "FirmwareUpgradeTask.hpp"
#include "FirmwareUpgradeController.h"
#include "FirmwareVerifier.h"

#include "SystemResourceDriver.h"
#include "SystemTimeDriver.h"

#include "CryptoDriver.h"
#include "CloudDriver.hpp"
#include "CloudServiceTask.hpp"
#include "DiscoveryService.hpp"
#include "AzureDriver.hpp"
#include "SntpDriver.h"
#include "AsyncMessageQueue.h"
#include "DiscoveryServiceInterface.h"

#include "AzurePayload.h"
#include "TimeSyncRequest.h"
#include "FirmwareUpdateStatus.h"
#include "ConnectionControllerTask.h"
#include "AzureSyncController.h"


namespace
{
  const char LOG_TAG[] = "MainTask";
}

/* TODO:
 * Use function below during development to set DeviceMeta for your prototype device. See Confluence, WifiHeater, GDIDs for development
 */
void setDeviceMeta(Settings & settings, DeviceMeta & deviceMeta, const char * gdid, uint32_t btPasskey);


void MainTask::run(bool eternalLoop)
{
  HardwareSetup::setup();
  SoftwareResetDriver resetDriver;


  /* Initialize buttons early (for capture of factory reset / "forget me" combo). */

  TimerDriver timerDriver;
  QTouchDriver touchDriver;
  ButtonDriver realButtonDriver(touchDriver, timerDriver);
  realButtonDriver.start("ButtonDriver", TaskStackSize::buttonDriver, TaskPriority::buttonDriver);


  /* Initialize drivers target for power on self test */

  DisplayDriver displayDriver;
  Display realDisplay(displayDriver);

  TWIDriver twiDriver;
  twiDriver.setup(Constants::TWI_BUS_MAX_FREQUENCY);

  Rtc rtc(twiDriver);
  SystemTimeDriver systemTime;

  RegulatorUnit regUnit(twiDriver);

  PersistentStorageDriver settingsStorageDriver;
  PersistentStorageDriver deviceMetaStorageDriver;

  CryptoDriver cryptoDriver;
  WifiDriver wifiDriver;
  BTDriver btDriver;


  /* Run PowerOnSelfTest */

  {
    PartitionDriver partition;
    PowerOnSelfTest powerOnSelfTest;

    bool isSelfTestOK =
        powerOnSelfTest.run(partition, timerDriver, realDisplay, rtc, systemTime, regUnit, cryptoDriver,
                             btDriver, wifiDriver, settingsStorageDriver, deviceMetaStorageDriver);

    if (!isSelfTestOK)
    {
      /*
       * Error code is flashed, first Ex is shown, where 'x' is first digit in three-digit code.
       * Next, 'yz' is shown, where 'yz' is the two last digits. Example: Error 243 => 'E2', then '43'
      */
      char errCode[Constants::TEXT_MAX_LENGTH];
      snprintf(errCode, Constants::TEXT_MAX_LENGTH, "E%1d", static_cast<int>(powerOnSelfTest.getErrorCode() / 100));
      ViewModel errorPart1(errCode);

      snprintf(errCode, Constants::TEXT_MAX_LENGTH, "%02d", static_cast<int>(powerOnSelfTest.getErrorCode() % 100));
      ViewModel errorPart2(errCode);


      /* Show error code three times then:
       * - continue if not fatal
       * - reboot if fatal
       */
      uint8_t loopCount = 0U;
      uint64_t errorMessageDisplayTime = 2000u;

      if ( !powerOnSelfTest.isFatalError() )
      {
        loopCount = 2u;
        errorMessageDisplayTime = 500u;
      }

      do
      {
        ++loopCount;
        realDisplay.drawScreen(errorPart1);
        timerDriver.waitMS(errorMessageDisplayTime);

        realDisplay.drawScreen(errorPart2);
        timerDriver.waitMS(errorMessageDisplayTime);

      } while (loopCount < 3U);

      if (powerOnSelfTest.isFatalError())
      {
        /*
         * Continuous reset isn't a problem even if old RU is used, because the power consumption is
         * limited by POST until a HPRU has been positively detected.
         */
        resetDriver.reset();
      }
    }
  }

  /* System test init */

  SpiDriver spiDriver;
  SystemTestCommunicator systemTestCommunicator(spiDriver);
  ButtonSystemTestDriver buttonDriver(systemTestCommunicator, realButtonDriver);
  DisplaySystemTest display(systemTestCommunicator, realDisplay);


  /* Settings and DeviceMeta init */

  SettingsStorage settingsStore(settingsStorageDriver);
  Settings settings(settingsStore, rtc, systemTime, timerDriver);
  settings.retrieveSettings();

  DeviceMetaStorage deviceMetaStore(deviceMetaStorageDriver);
  DeviceMeta deviceMeta(deviceMetaStore);
  deviceMeta.retrieveMeta();

  /* Check if we are in the action of installing a FW upgrade */
  FirmwareVerifier fwVerifier(settings);
  fwVerifier.init();

  if (fwVerifier.isUpgrading())
  {
    ESP_LOGI(LOG_TAG, "Verifying FW upgrade ID '%s' ...", fwVerifier.getFirmwareUpgradeID());

    if (fwVerifier.verifyUpgrade())
    {
      // We have successfully installed an upgrade!
      ESP_LOGI(LOG_TAG, "Firmware upgrade verified!");
      fwVerifier.saveToSettings();
      settings.storeSettings();
    }
    else
    {
      // We have attempted to install upgrade, but failed for some reason.
      if (fwVerifier.isUpgradeBlocked())
      {
        ESP_LOGE(LOG_TAG, "Firmware upgrade failed and is now blocked for this version. Attempt no. %u, versionCode 0x%08x.",
            fwVerifier.getFirmwareUpgradeCount(),
            fwVerifier.getFirmwareVersionCode());
      }
      else
      {
        ESP_LOGE(LOG_TAG, "Firmware upgrade failed. Attempt no. %u, versionCode 0x%08x.", fwVerifier.getFirmwareUpgradeCount(), fwVerifier.getFirmwareVersionCode());
      }
    }
  }

  OTADriver otaDriver;

  /* Run FGT if we are in test mode */

  bool isTestMode = settings.isTestMode();

  if(isTestMode)
  {
    // Run FGT
    FGT fgt(deviceMeta, btDriver, settings, resetDriver, cryptoDriver, displayDriver, touchDriver, regUnit, otaDriver, wifiDriver);
    fgt.startGattService();
    fgt.runTest();
    fgt.stopGattService();
    FreeRTOS_delay_ms(100u);
  }

  BTWifiSetupDriver btWifiSetupDriver(btDriver, deviceMeta.getBluetoothPasskey(), settings, resetDriver);

  /* Define queues using in application */

  Queue<bool> isHeatElementOnQueue(1U); // Used by HeatController to signal UIController to control 'heating On LED'
  Queue<ConnectionStatusChangeRequest::Enum> connStatusChangeRequestQueue(5U); // Used by FwUpgradeController and UIController to request state changes from ConnController
  Queue<ConnectionStrings_t> connectionStringQueue(1U);
  Queue<WifiConnectionStatus::Enum> wifiSetupResultQueue(1U);
  Queue<char[Constants::CLOUD_CONNECTION_STRING_MAX_LENGTH]> azureConnectionStringQueue(1U);
  Queue<CloudServiceCommand> cloudServiceCommandQueue(1u);
  AsyncMessageQueue azureAsyncSendQueue;


  /* Firmware Upgrade requirements */

  CloudServiceTask cloudService;
  CloudDriver cloudDriver(cryptoDriver);

  FirmwareUpgradeController fwUpgradeController(otaDriver, cloudDriver, fwVerifier, connStatusChangeRequestQueue);
  FirmwareUpgradeTask fwUpgrade(fwUpgradeController);


  /* IoT Hub drivers etc. */

  AzureSyncController azureSync(settings, deviceMeta, azureAsyncSendQueue);
  AzurePayload azurePayload(settings, deviceMeta, azureSync, fwUpgradeController, systemTime);
  AzureDriver azureDriver(azureConnectionStringQueue, cryptoDriver, azureAsyncSendQueue, timerDriver, azurePayload);

  UserInterfaceTask userInterface(display,
                                  buttonDriver,
                                  btDriver,
                                  wifiDriver,
                                  azureDriver,
                                  timerDriver,
                                  settings,
                                  deviceMeta,
                                  displayDriver,
                                  fwUpgrade,
                                  isHeatElementOnQueue,
                                  connStatusChangeRequestQueue);
  userInterface.start("UI", TaskStackSize::userInterface, TaskPriority::userInterface);

  HeatControllerTask heatController(regUnit, timerDriver, systemTime, settings, isHeatElementOnQueue, twiDriver, resetDriver);
  heatController.start("HeatCtrl", TaskStackSize::heatController, TaskPriority::heatController);

  // Comment out during 'normal' use. To be used to set read-only device info during development.
  // Replace two last args in setDeviceMeta with your GDID and BTPasskey respectively before use.
  //settings.setWifiSSID("gdnlab");
  //settings.setWifiPassword("GDNEC800");
  //setDeviceMeta(settings, deviceMeta, "090219950060", Constants::DEFAULT_BT_PASSKEY);

  {
    bool wifiConfigResult = false;

    char wifiSSID[Constants::WIFI_SSID_SIZE_MAX];
    char wifiPass[Constants::WIFI_PASSWORD_SIZE_MAX];
    settings.getWifiSSID(wifiSSID, sizeof(wifiSSID));
    settings.getWifiPassword(wifiPass, sizeof(wifiPass));

    wifiConfigResult = wifiDriver.configureConnection(wifiSSID, wifiPass);

    uint32_t staticIp = settings.getStaticIp();
    uint32_t staticNetmask = settings.getStaticNetmask();
    uint32_t staticGateway = settings.getStaticGateway();
    uint32_t staticDns1 = settings.getStaticDns1();
    uint32_t staticDns2 = settings.getStaticDns2();

    if ( staticIp != 0u )
    {
      wifiConfigResult = wifiDriver.configureStaticIp(staticIp,
                                                      staticNetmask,
                                                      staticGateway,
                                                      staticDns1,
                                                      staticDns2);
      if ( !wifiConfigResult )
      {
        ESP_LOGE(LOG_TAG, "Failed to configure static ip");
      }
    }
    else
    {
      wifiConfigResult = wifiDriver.configureDynamicIp();
      if ( !wifiConfigResult )
      {
        ESP_LOGE(LOG_TAG, "Failed to configure dynamic ip");
      }
    }
  }

  /* IoT Hub tasks */

  EventFlags healthCheckEventFlags;
  SntpDriver sntpDriver;
  cloudService.init(&cloudDriver, &deviceMeta, &settings, &connectionStringQueue, &wifiSetupResultQueue, &healthCheckEventFlags, &cloudServiceCommandQueue, &sntpDriver);
  cloudService.start("CloudServive", TaskStackSize::cloudService, TaskPriority::cloudService);
  azureDriver.start("AzureService", TaskStackSize::azureService, TaskPriority::azureService);

  ConnectionControllerTask connectionControllerTask(wifiDriver,
                                                    cloudService,
                                                    azureDriver,
                                                    btWifiSetupDriver,
                                                    settings,
                                                    connStatusChangeRequestQueue,
                                                    connectionStringQueue,
                                                    fwUpgrade,
                                                    resetDriver,
                                                    timerDriver,
                                                    wifiSetupResultQueue);
  connectionControllerTask.startTask();

  DateTime systemDateTime;
  DateTime localDateTime;
  uint64_t timeSinceLastSyncCheckMs = 0U;
  uint8_t lastSecond = 60U;
  SystemResourceDriver sysResource;

  RebootScheduler rebootScheduler(resetDriver, systemTime);

  {
    uint8_t gdid[Constants::GDID_BCD_SIZE] = {};
    deviceMeta.getGDIDAsBCD(gdid);

    DateTime bootTime;
    settings.getLocalTime(bootTime);

    DateTime preferredRebootTime = Constants::DEFAULT_DATETIME;
    preferredRebootTime.hours = Constants::REBOOT_SCHEDULER_PREFERRED_TIME_SPAN_START_HOURS;
    preferredRebootTime.minutes = Constants::REBOOT_SCHEDULER_PREFERRED_TIME_SPAN_START_MINUTES;
    preferredRebootTime.seconds = Constants::REBOOT_SCHEDULER_PREFERRED_TIME_SPAN_START_SECONDS;

    rebootScheduler.resetAndInit(gdid,
                                 bootTime,
                                 preferredRebootTime,
                                 Constants::REBOOT_SCHEDULER_PREFERRED_TIME_SPAN_LENGTH_S,
                                 Constants::REBOOT_SCHEDULER_MAX_UPTIME_S);
  }

  ESP_LOGI(LOG_TAG, "Setup complete, entering control loop");

  do {
    bool shouldReboot = false;

    settings.getUnixTime(systemDateTime);
    settings.getLocalTime(localDateTime);

    if (systemDateTime.seconds < lastSecond)
    {
      ESP_LOGI(LOG_TAG, "SystemTime (UTC): %04i-%02i-%02i %02i:%02i:%02i",
                          2000U + systemDateTime.year, systemDateTime.month, systemDateTime.days,
                          systemDateTime.hours, systemDateTime.minutes, systemDateTime.seconds);

      ESP_LOGI(LOG_TAG, "LocalTime: %04i-%02i-%02i %02i:%02i:%02i",
                          2000U + localDateTime.year, localDateTime.month, localDateTime.days,
                          localDateTime.hours, localDateTime.minutes, localDateTime.seconds);

      ESP_LOGI(LOG_TAG, "Current free RAM is %u bytes.", sysResource.getCurrentFreeHeap());
      ESP_LOGI(LOG_TAG, "Minimum free RAM since boot is %u bytes", sysResource.getMinimumFreeHeapSinceBoot());
    }

    lastSecond = systemDateTime.seconds;

    shouldReboot = rebootScheduler.shouldReboot(localDateTime)
        && timerDriver.getTimeSinceBootMS() > static_cast<uint64_t>(Constants::REBOOT_SCHEDULER_MINIMUM_UPTIME_MS);

    // Send our updated state to IoTHub if anything has changed every 10 seconds (on sec = 5, 15, ..., 55)
    if (timerDriver.getTimeSinceTimestampMS(timeSinceLastSyncCheckMs) > 10000U)
    {
      if (connectionControllerTask.getState() == ConnectionState::IotHubConnected)
      {
        azureSync.updateFromSettings();
      }

      timeSinceLastSyncCheckMs = timerDriver.getTimeSinceBootMS();
    }

    if (settings.isModified())
    {
      settings.storeSettings();
    }

    if ( shouldReboot )
    {
      ESP_LOGI(LOG_TAG, "Performing scheduled reset");
      resetDriver.reset();
    }

    FreeRTOS_delay_ms(1000u);

  } while (eternalLoop);
}


void setDeviceMeta(Settings & settings, DeviceMeta & deviceMeta, const char * gdid, uint32_t btPasskey)
{
  bool isTestMode = settings.isTestMode();

  if (isTestMode)
  {
    deviceMeta.setGDID(gdid);
    deviceMeta.setBluetoothPasskey(btPasskey);
    deviceMeta.storeMeta();

    ESP_LOGI(LOG_TAG, "Setting DeviceMeta completed. Rebooting in 5 secs to disable test mode ...");
    settings.setRunMode(RunMode::normal);
  }
  else
  {
    ESP_LOGW(LOG_TAG, "Setting DeviceMeta failed as we aren't in testMode. Rebooting in 5 secs to enable test mode and try again ...");
    settings.setRunMode(RunMode::test);
  }

  FreeRTOS_delay_ms(5000u);
  settings.storeSettings();
  esp_restart();
}
