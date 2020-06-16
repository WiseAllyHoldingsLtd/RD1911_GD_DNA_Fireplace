
#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include "ConnectionControllerTask.h"

#include "WifiDriverMock.h"
#include "CloudServiceMock.h"
#include "AzureDriverMock.h"
#include "BTWifiSetupDriverMock.h"
#include "SettingsMock.h"
#include "QueueMock.hpp"
#include "DiscoveryServiceInterface.h"
#include "FirmwareUpgradeTaskMock.h"
#include "SoftwareResetDriverMock.h"
#include "TimerDriverMock.h"

namespace
{
  ConnectionControllerTask *connectionController = nullptr;
  WifiDriverMock *wifiDriver = nullptr;
  CloudServiceMock *cloudService = nullptr;
  AzureDriverMock *azureDriver = nullptr;
  BTWifiSetupDriverMock *btWifiSetupDriver = nullptr;
  SettingsMock *settings = nullptr;
  QueueMock<ConnectionStatusChangeRequest::Enum> *connStatusChangeRequestQueue = nullptr;
  QueueMockStruct<ConnectionStrings_t> *connectionStringQueue = nullptr;
  FirmwareUpgradeTaskMock *fwUpgradeTask = nullptr;
  SoftwareResetDriverMock *softwareResetDriver = nullptr;
  TimerDriverMock *timerDriver = nullptr;
  QueueMock<WifiConnectionStatus::Enum> *wifiSetupResultQueue = nullptr;

  void commonTestSetup()
  {
    wifiDriver = new WifiDriverMock();
    cloudService = new CloudServiceMock();
    azureDriver = new AzureDriverMock();
    btWifiSetupDriver = new BTWifiSetupDriverMock();
    settings = new SettingsMock();
    connStatusChangeRequestQueue = new QueueMock<ConnectionStatusChangeRequest::Enum>();
    connectionStringQueue = new QueueMockStruct<ConnectionStrings_t>();
    fwUpgradeTask = new FirmwareUpgradeTaskMock();
    softwareResetDriver = new SoftwareResetDriverMock();
    timerDriver = new TimerDriverMock();
    wifiSetupResultQueue = new QueueMock<WifiConnectionStatus::Enum>();

    mock().ignoreOtherCalls();
    connectionController = new ConnectionControllerTask(*wifiDriver,
        *cloudService,
        *azureDriver,
        *btWifiSetupDriver,
        *settings,
        *connStatusChangeRequestQueue,
        *connectionStringQueue,
        *fwUpgradeTask,
        *softwareResetDriver,
        *timerDriver,
        *wifiSetupResultQueue);
  }

  void commonTestTearDown()
  {
    delete connectionController;
    connectionController = nullptr;

    delete wifiDriver;
    delete cloudService;
    delete azureDriver;
    delete btWifiSetupDriver;
    delete settings;
    delete connStatusChangeRequestQueue;
    delete connectionStringQueue;
    delete fwUpgradeTask;
    delete softwareResetDriver;
    delete timerDriver;
    delete wifiSetupResultQueue;

    wifiDriver = nullptr;
    cloudService = nullptr;
    azureDriver = nullptr;
    btWifiSetupDriver = nullptr;
    settings = nullptr;
    connStatusChangeRequestQueue = nullptr;
    connectionStringQueue = nullptr;
    fwUpgradeTask = nullptr;
    softwareResetDriver = nullptr;
    timerDriver = nullptr;
    wifiSetupResultQueue = nullptr;

    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
  }

}

TEST_GROUP(connectionController)
{
  TEST_SETUP()
  {
    commonTestSetup();
  }

  TEST_TEARDOWN()
  {
    commonTestTearDown();
  }
};

///////////////////////////////////////////////////////////////////////////////
// Tests from state NotConnected

TEST(connectionController, testInitialStateIsNotConnected)
{
  auto state = connectionController->getState();
  CHECK_EQUAL(state, ConnectionState::NotConnected);
}

TEST(connectionController, notConnectedLeadsToConnecting)
{
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(false);
  mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(true);
  mock().expectOneCall("AzureDriverMock::isIdle").andReturnValue(true);
  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::WifiConnecting, state);
}

TEST(connectionController, notConnectedLeadsToConnectedIfWifiAlreadyConnected)
{
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);
  mock().ignoreOtherCalls();
  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(state, ConnectionState::WifiConnected);
}

TEST(connectionController, notConnectedPausesCloudServiceIfNotPaused)
{
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(false);
  mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(false);
  mock().expectOneCall("getTimeSinceTimestampMS")
      .ignoreOtherParameters()
      .andReturnValue(static_cast<uint32_t>(0u));
  mock().expectOneCall("CloudServiceMock::requestPause").andReturnValue(true);
  mock().ignoreOtherCalls();
  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(state, ConnectionState::NotConnected);
}

TEST(connectionController, noPauseRequestIfAlreadyPaused)
{
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(false);
  mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(true);
  mock().expectNoCall("CloudServiceMock::requestPause");
  mock().expectOneCall("AzureDriverMock::isIdle").andReturnValue(true);
  mock().ignoreOtherCalls();
  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(state, ConnectionState::WifiConnecting);
}

TEST(connectionController, swResetIfTimeoutWaitingForCloudServicePause)
{
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(false);
  mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(false);
  mock().expectOneCall("getTimeSinceTimestampMS")
      .ignoreOtherParameters()
      .andReturnValue(static_cast<uint32_t>(Constants::CLOUD_SERVICE_PAUSE_TIMEOUT+1u));
  mock().expectOneCall("SoftwareResetDriverMock::reset");
  mock().ignoreOtherCalls();
  connectionController->run(false);
}

TEST(connectionController, noSwResetBeforeTimeoutWaitingForCloudServicePause)
{
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(false);
  mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(false);
  mock().expectOneCall("getTimeSinceTimestampMS")
      .ignoreOtherParameters()
      .andReturnValue(static_cast<uint32_t>(Constants::CLOUD_SERVICE_PAUSE_TIMEOUT-1u));
  mock().expectNoCall("SoftwareResetDriverMock::reset");
  mock().ignoreOtherCalls();
  connectionController->run(false);
}

TEST(connectionController, shutdownAzureDriverIfRunning)
{
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(false);
  mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(true);
  mock().expectOneCall("AzureDriverMock::isIdle").andReturnValue(false);
  mock().expectOneCall("AzureDriverMock::requestShutdown");
  mock().ignoreOtherCalls();
  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(state, ConnectionState::NotConnected);
}

TEST(connectionController, fullyFunctionalWifiResultIsReported)
{
  WifiConnectionStatus::Enum wifiSetupResult = WifiConnectionStatus::FULLY_FUNCTIONAL;
  mock().expectOneCall("QueueMock::pop")
      .withOutputParameterReturning("item",
                                    static_cast<void *>(&wifiSetupResult),
                                    sizeof(wifiSetupResult))
      .ignoreOtherParameters()
      .andReturnValue(true);

  mock().expectOneCall("BTWifiSetupDriverMock::isGattServiceRunning")
      .andReturnValue(true);

  mock().expectOneCall("BTWifiSetupDriverMock::setConnectionStatus")
        .withIntParameter("connectionStatus", static_cast<int32_t>(WifiConnectionStatus::FULLY_FUNCTIONAL))
        .ignoreOtherParameters() // statusString ignored
        .andReturnValue(true);

  ConnectionStatusChangeRequest::Enum noChange = ConnectionStatusChangeRequest::noChange;
  mock().expectOneCall("QueueMock::pop")
      .withOutputParameterReturning("item",
                                    static_cast<void *>(&noChange),
                                    sizeof(noChange))
      .ignoreOtherParameters()
      .andReturnValue(false);

  connectionController->run(false);

  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::NotConnected, state);
}

TEST(connectionController, wifiNotDiscoveredWifiResultIsReportedAndStateChangesToActivatingWifiSetup)
{
  WifiConnectionStatus::Enum wifiSetupResult = WifiConnectionStatus::WIFI_NOT_DISCOVERED;
  mock().expectOneCall("QueueMock::pop")
      .withOutputParameterReturning("item",
                                    static_cast<void *>(&wifiSetupResult),
                                    sizeof(wifiSetupResult))
      .ignoreOtherParameters()
      .andReturnValue(true);

  mock().expectOneCall("BTWifiSetupDriverMock::isGattServiceRunning")
      .andReturnValue(true);

  mock().expectOneCall("BTWifiSetupDriverMock::setConnectionStatus")
        .withIntParameter("connectionStatus", static_cast<int32_t>(WifiConnectionStatus::WIFI_NOT_DISCOVERED))
        .ignoreOtherParameters() // statusString ignored
        .andReturnValue(true);

  ConnectionStatusChangeRequest::Enum noChange = ConnectionStatusChangeRequest::noChange;
  mock().expectOneCall("QueueMock::pop")
      .withOutputParameterReturning("item",
                                    static_cast<void *>(&noChange),
                                    sizeof(noChange))
      .ignoreOtherParameters()
      .andReturnValue(false);

  connectionController->run(false);

  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::ActivatingWifiSetup, state);
}

//////////////////////////////////
// Test fw upgrade interactions

TEST(connectionController, goesToFwUpgradeInProgressOnFwUpgradeReadyMessage)
{
  WifiConnectionStatus::Enum wifiSetupResult = WifiConnectionStatus::IN_PROGRESS;
  mock().expectOneCall("QueueMock::pop")
      .withOutputParameterReturning("item",
                                    static_cast<void *>(&wifiSetupResult),
                                    sizeof(wifiSetupResult))
      .ignoreOtherParameters()
      .andReturnValue(false);

  ConnectionStatusChangeRequest::Enum requestFwUpgradeReady
    = ConnectionStatusChangeRequest::fwUpgradeReady;
  mock().expectOneCall("QueueMock::pop")
      .withOutputParameterReturning("item",
                                    static_cast<void *>(&requestFwUpgradeReady),
                                    sizeof(requestFwUpgradeReady))
      .ignoreOtherParameters()
      .andReturnValue(true);

  mock().ignoreOtherCalls();
  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(state, ConnectionState::FwUpgradeInProgress);
}

TEST(connectionController, doesNotGoToFwUpgradeIfBluetoothShouldBeActive)
{
  WifiConnectionStatus::Enum wifiSetupResult = WifiConnectionStatus::IN_PROGRESS;
  mock().expectOneCall("QueueMock::pop")
      .withOutputParameterReturning("item",
                                    static_cast<void *>(&wifiSetupResult),
                                    sizeof(wifiSetupResult))
      .ignoreOtherParameters()
      .andReturnValue(false);

  ConnectionStatusChangeRequest::Enum requestBluetoothOn
    = ConnectionStatusChangeRequest::bluetoothOn;
  mock().expectOneCall("QueueMock::pop")
      .withOutputParameterReturning("item",
                                    static_cast<void *>(&requestBluetoothOn),
                                    sizeof(requestBluetoothOn))
      .ignoreOtherParameters()
      .andReturnValue(true);

  mock().ignoreOtherCalls();
  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::ActivatingWifiSetup, state);

  mock().expectOneCall("QueueMock::pop")
      .withOutputParameterReturning("item",
                                    static_cast<void *>(&wifiSetupResult),
                                    sizeof(wifiSetupResult))
      .ignoreOtherParameters()
      .andReturnValue(false);

  ConnectionStatusChangeRequest::Enum requestFwUpgradeReady
    = ConnectionStatusChangeRequest::fwUpgradeReady;
  mock().expectOneCall("QueueMock::pop")
      .withOutputParameterReturning("item",
                                    static_cast<void *>(&requestFwUpgradeReady),
                                    sizeof(requestFwUpgradeReady))
      .ignoreOtherParameters()
      .andReturnValue(true);

  mock().ignoreOtherCalls();
  connectionController->run(false);
  state = connectionController->getState();
  CHECK_FALSE(state == ConnectionState::FwUpgradeInProgress);
}

TEST(connectionController, goesToFwUpgradeStateAfterBleOffIfFwReady)
{
  WifiConnectionStatus::Enum wifiSetupResult = WifiConnectionStatus::IN_PROGRESS;
  mock().expectOneCall("QueueMock::pop")
      .withOutputParameterReturning("item",
                                    static_cast<void *>(&wifiSetupResult),
                                    sizeof(wifiSetupResult))
      .ignoreOtherParameters()
      .andReturnValue(false);

  ConnectionStatusChangeRequest::Enum bluetoothOn
    = ConnectionStatusChangeRequest::bluetoothOn;
  mock().expectOneCall("QueueMock::pop")
      .withOutputParameterReturning("item",
                                    static_cast<void *>(&bluetoothOn),
                                    sizeof(bluetoothOn))
      .ignoreOtherParameters()
      .andReturnValue(true);

  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_FALSE(state == ConnectionState::FwUpgradeInProgress);

  mock().expectOneCall("QueueMock::pop")
      .withOutputParameterReturning("item",
                                    static_cast<void *>(&wifiSetupResult),
                                    sizeof(wifiSetupResult))
      .ignoreOtherParameters()
      .andReturnValue(false);

  ConnectionStatusChangeRequest::Enum fwReady
    = ConnectionStatusChangeRequest::fwUpgradeReady;
  mock().expectOneCall("QueueMock::pop")
      .withOutputParameterReturning("item",
                                    static_cast<void *>(&fwReady),
                                    sizeof(fwReady))
      .ignoreOtherParameters()
      .andReturnValue(true);
  mock().ignoreOtherCalls();
  connectionController->run(false);
  state = connectionController->getState();
  CHECK_FALSE(state == ConnectionState::FwUpgradeInProgress);

  mock().expectOneCall("QueueMock::pop")
      .withOutputParameterReturning("item",
                                    static_cast<void *>(&wifiSetupResult),
                                    sizeof(wifiSetupResult))
      .ignoreOtherParameters()
      .andReturnValue(false);

  ConnectionStatusChangeRequest::Enum bluetoothOff
    = ConnectionStatusChangeRequest::bluetoothOff;
  mock().expectOneCall("QueueMock::pop")
      .withOutputParameterReturning("item",
                                    static_cast<void *>(&bluetoothOff),
                                    sizeof(bluetoothOff))
      .ignoreOtherParameters()
      .andReturnValue(true);
  mock().expectOneCall("BTWifiSetupDriverMock::isGattServiceRunning")
      .andReturnValue(true);
  mock().expectOneCall("BTWifiSetupDriverMock::stopGattService");
  connectionController->run(false);
  state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::FwUpgradeInProgress, state);
}

TEST_GROUP(fwUpgradeInProgress)
{
  TEST_SETUP()
  {
    commonTestSetup();

    WifiConnectionStatus::Enum wifiSetupResult = WifiConnectionStatus::IN_PROGRESS;
    mock().expectOneCall("QueueMock::pop")
        .withOutputParameterReturning("item",
                                      static_cast<void *>(&wifiSetupResult),
                                      sizeof(wifiSetupResult))
        .ignoreOtherParameters()
        .andReturnValue(false);

    ConnectionStatusChangeRequest::Enum requestFwUpgradeReady
      = ConnectionStatusChangeRequest::fwUpgradeReady;
    mock().expectOneCall("QueueMock::pop")
        .withOutputParameterReturning("item",
                                      static_cast<void *>(&requestFwUpgradeReady),
                                      sizeof(requestFwUpgradeReady))
        .ignoreOtherParameters()
        .andReturnValue(true);

    mock().expectOneCall("getTimeSinceBootMS")
          .andReturnValue(static_cast<uint32_t>(0u));
    mock().ignoreOtherCalls();
    connectionController->run(false);
    auto state = connectionController->getState();
    CHECK_EQUAL(state, ConnectionState::FwUpgradeInProgress);

    mock().expectOneCall("QueueMock::pop")
        .withOutputParameterReturning("item",
                                      static_cast<void *>(&wifiSetupResult),
                                      sizeof(wifiSetupResult))
        .ignoreOtherParameters()
        .andReturnValue(false);

    ConnectionStatusChangeRequest::Enum noChange = ConnectionStatusChangeRequest::noChange;
    mock().expectOneCall("QueueMock::pop")
        .withOutputParameterReturning("item",
                                      static_cast<void *>(&noChange),
                                      sizeof(noChange))
        .ignoreOtherParameters()
        .andReturnValue(false);
  }

  TEST_TEARDOWN()
  {
    commonTestTearDown();
  }
};

TEST(fwUpgradeInProgress, noResetIfNoTimeout)
{
  mock().expectOneCall("getTimeSinceTimestampMS")
      .ignoreOtherParameters()
      .andReturnValue(static_cast<uint32_t>(Constants::FW_UPGRADE_TIMEOUT-1u));

  mock().expectNoCall("SoftwareResetDriverMock::reset");
  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(state, ConnectionState::FwUpgradeInProgress);
}

TEST(fwUpgradeInProgress, resetOnTimeout)
{
  mock().expectOneCall("getTimeSinceTimestampMS")
      .ignoreOtherParameters()
      .andReturnValue(static_cast<uint32_t>(Constants::FW_UPGRADE_TIMEOUT+1u));

  mock().expectOneCall("SoftwareResetDriverMock::reset");
  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(state, ConnectionState::FwUpgradeInProgress);
}

TEST(fwUpgradeInProgress, shutdownAzureDriverIfRunning)
{
  mock().expectOneCall("getTimeSinceTimestampMS")
      .ignoreOtherParameters()
      .andReturnValue(static_cast<uint32_t>(1u));

  mock().expectOneCall("AzureDriverMock::isIdle").andReturnValue(false);
  mock().expectOneCall("AzureDriverMock::requestShutdown");
  mock().ignoreOtherCalls();
  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(state, ConnectionState::FwUpgradeInProgress);
}

TEST(fwUpgradeInProgress, pausesCloudServiceIfNotPaused)
{
  mock().expectOneCall("getTimeSinceTimestampMS")
      .ignoreOtherParameters()
      .andReturnValue(static_cast<uint32_t>(1u));

  mock().expectOneCall("AzureDriverMock::isIdle").andReturnValue(true);
  mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(false);
  mock().expectOneCall("CloudServiceMock::requestPause").andReturnValue(true);
  mock().ignoreOtherCalls();
  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(state, ConnectionState::FwUpgradeInProgress);
}

TEST(fwUpgradeInProgress, reconnectsToWifiIfDisconnected)
{
  mock().expectOneCall("getTimeSinceTimestampMS")
      .ignoreOtherParameters()
      .andReturnValue(static_cast<uint32_t>(1u));

  mock().expectOneCall("AzureDriverMock::isIdle").andReturnValue(true);
  mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(true);
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(false);
  mock().expectOneCall("WifiDriverMock::connectToWlan").andReturnValue(true);
  mock().ignoreOtherCalls();
  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(state, ConnectionState::FwUpgradeInProgress);
}

TEST(fwUpgradeInProgress, startsFwUpgradeTaskIfNotRunning)
{
  mock().expectOneCall("getTimeSinceTimestampMS")
      .ignoreOtherParameters()
      .andReturnValue(static_cast<uint32_t>(1u));

  mock().expectOneCall("AzureDriverMock::isIdle").andReturnValue(true);
  mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(true);
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);
  mock().expectOneCall("FirmwareUpgradeTaskMock::getCurrentState")
      .andReturnValue(static_cast<uint32_t>(FirmwareUpgradeState::notRequested));
  mock().expectOneCall("FirmwareUpgradeTaskMock::startTask");
  mock().ignoreOtherCalls();
  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(state, ConnectionState::FwUpgradeInProgress);
}

///////////////////////////////////////
// Connecting

TEST_GROUP(connectionControllerWifiConnecting)
{
  TEST_SETUP()
  {
    commonTestSetup();

    mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(false);
    mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(true);
    mock().expectOneCall("AzureDriverMock::isIdle").andReturnValue(true);
    mock().ignoreOtherCalls();
    connectionController->run(false);
    auto state = connectionController->getState();
    CHECK_EQUAL(state, ConnectionState::WifiConnecting);
  }

  TEST_TEARDOWN()
  {
    commonTestTearDown();
  }
};

TEST(connectionControllerWifiConnecting, connectsToWifiIfNotConnected)
{
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(false);
  mock().expectOneCall("WifiDriverMock::connectToWlan").andReturnValue(true);
  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(state, ConnectionState::WifiConnected);
}

TEST(connectionControllerWifiConnecting, retriesIfWifiConnectionFails)
{
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(false);
  mock().expectOneCall("WifiDriverMock::connectToWlan").andReturnValue(false);
  mock().expectOneCall("BTWifiSetupDriverMock::isGattServiceRunning")
      .andReturnValue(false);

  connectionController->run(false);

  auto state = connectionController->getState();
  CHECK_EQUAL(state, ConnectionState::WifiConnecting);
}

TEST(connectionControllerWifiConnecting,
    goesToActivatingWifiSetupIfWifiConnectionFailsAndGattServiceIsRunning)
{
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(false);
  mock().expectOneCall("WifiDriverMock::connectToWlan").andReturnValue(false);
  mock().expectOneCall("BTWifiSetupDriverMock::isGattServiceRunning")
      .andReturnValue(true);
  mock().expectOneCall("BTWifiSetupDriverMock::setConnectionStatus")
        .withIntParameter("connectionStatus", static_cast<int32_t>(WifiConnectionStatus::WIFI_NOT_DISCOVERED))
        .ignoreOtherParameters() // statusString ignored
        .andReturnValue(true);

  connectionController->run(false);

  auto state = connectionController->getState();
  CHECK_EQUAL(state, ConnectionState::ActivatingWifiSetup);
}

TEST(connectionControllerWifiConnecting, changesStateToWifiConnectedIfAlreadyConnected)
{
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);
  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(state, ConnectionState::WifiConnected);
}

/////////////////
// Wifi connected

TEST_GROUP(connectionControllerWifiConnected)
{
  TEST_SETUP()
  {
    commonTestSetup();

    mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(false);
    mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(true);
    mock().expectOneCall("AzureDriverMock::isIdle").andReturnValue(true);
    mock().ignoreOtherCalls();
    connectionController->run(false);
    auto state = connectionController->getState();
    CHECK_EQUAL(state, ConnectionState::WifiConnecting);

    mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(false);
    mock().expectOneCall("WifiDriverMock::connectToWlan").andReturnValue(true);
    connectionController->run(false);
    state = connectionController->getState();
    CHECK_EQUAL(state, ConnectionState::WifiConnected);
  }

  TEST_TEARDOWN()
  {
    commonTestTearDown();
  }
};

TEST(connectionControllerWifiConnected, goesBackToNotConnectedIfWifiDisconnected)
{
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(false);
  connectionController->run(false);
  ConnectionState::Enum state = connectionController->getState();
  CHECK_EQUAL(state, ConnectionState::NotConnected);
}

TEST(connectionControllerWifiConnected, goesOnToDiscoveryConnectingIfStillConnectedToWifi)
{
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);
  connectionController->run(false);
  ConnectionState::Enum state = connectionController->getState();
  CHECK_EQUAL(state, ConnectionState::DiscoveryConnecting);
}

///////////////////////
// DiscoveryConnecting

TEST_GROUP(connectionControllerDiscoveryConnecting)
{
  TEST_SETUP()
  {
    commonTestSetup();

    mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(false);
    mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(true);
    mock().expectOneCall("AzureDriverMock::isIdle").andReturnValue(true);
    mock().ignoreOtherCalls();
    connectionController->run(false);
    auto state = connectionController->getState();
    CHECK_EQUAL(state, ConnectionState::WifiConnecting);

    mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(false);
    mock().expectOneCall("WifiDriverMock::connectToWlan").andReturnValue(true);
    connectionController->run(false);
    state = connectionController->getState();
    CHECK_EQUAL(state, ConnectionState::WifiConnected);

    mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);
    connectionController->run(false);
    state = connectionController->getState();
    CHECK_EQUAL(state, ConnectionState::DiscoveryConnecting);
  }

  TEST_TEARDOWN()
  {
    commonTestTearDown();
  }
};

TEST(connectionControllerDiscoveryConnecting, unpausesCloudServiceIfPaused)
{
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);
  mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(true);
  mock().expectOneCall("CloudServiceMock::requestUnpause").andReturnValue(true);
  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::DiscoveryConnecting, state);
}

TEST(connectionControllerDiscoveryConnecting,
     goesOnToDiscoveryConnectedIfDiscoveryAndHealthCheckOk)
{
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);
  mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(false);
  mock().expectOneCall("CloudServiceMock::wasDiscoverySuccessfulSinceLastRestart")
      .andReturnValue(true);
  mock().expectOneCall("CloudServiceMock::getCurrentHealth")
      .andReturnValue(100u);
  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::DiscoveryConnected, state);
}

TEST(connectionControllerDiscoveryConnecting,
     staysInDiscoveryConnectingIfDiscoveryNotOk)
{
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);
  mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(false);
  mock().expectOneCall("CloudServiceMock::wasDiscoverySuccessfulSinceLastRestart")
      .andReturnValue(false);
  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::DiscoveryConnecting, state);
}

TEST(connectionControllerDiscoveryConnecting,
     staysInDiscoveryConnectingIfHealthCheckIsLessThan100)
{
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);
  mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(false);
  mock().expectOneCall("CloudServiceMock::wasDiscoverySuccessfulSinceLastRestart")
      .andReturnValue(true);
  mock().expectOneCall("CloudServiceMock::getCurrentHealth")
      .andReturnValue(99u);
  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::DiscoveryConnecting, state);
}

TEST(connectionControllerDiscoveryConnecting,
     staysInDiscoveryConnectingIfHealthCheckIs0)
{
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);
  mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(false);
  mock().expectOneCall("CloudServiceMock::wasDiscoverySuccessfulSinceLastRestart")
      .andReturnValue(true);
  mock().expectOneCall("CloudServiceMock::getCurrentHealth")
      .andReturnValue(0u);
  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::DiscoveryConnecting, state);
}

TEST(connectionControllerDiscoveryConnecting,
     goesToWaitingForBleOffWhenDiscoveryOkIfBleActive)
{
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);
  mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(false);
  mock().expectOneCall("CloudServiceMock::wasDiscoverySuccessfulSinceLastRestart")
      .andReturnValue(true);
  mock().expectOneCall("BTWifiSetupDriverMock::isGattServiceRunning")
      .andReturnValue(true);
  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::WaitingForBleOff, state);
}

TEST(connectionControllerDiscoveryConnecting,
     goesBackToNotConnectedIfWifiDisconnected)
{
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(false);
  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::NotConnected, state);
}

/////////////////////////
// DiscoveryConnected

TEST_GROUP(connectionControllerDiscoveryConnected)
{
  TEST_SETUP()
  {
    commonTestSetup();

    mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(false);
    mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(true);
    mock().expectOneCall("AzureDriverMock::isIdle").andReturnValue(true);
    mock().ignoreOtherCalls();
    connectionController->run(false);
    auto state = connectionController->getState();
    CHECK_EQUAL(state, ConnectionState::WifiConnecting);

    mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(false);
    mock().expectOneCall("WifiDriverMock::connectToWlan").andReturnValue(true);
    connectionController->run(false);
    state = connectionController->getState();
    CHECK_EQUAL(state, ConnectionState::WifiConnected);

    mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);
    connectionController->run(false);
    state = connectionController->getState();
    CHECK_EQUAL(state, ConnectionState::DiscoveryConnecting);

    mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);
    mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(false);
    mock().expectOneCall("CloudServiceMock::wasDiscoverySuccessfulSinceLastRestart")
        .andReturnValue(true);
    mock().expectOneCall("CloudServiceMock::getCurrentHealth")
        .andReturnValue(100u);
    connectionController->run(false);
    state = connectionController->getState();
    CHECK_EQUAL(ConnectionState::DiscoveryConnected, state);
  }

  TEST_TEARDOWN()
  {
    commonTestTearDown();
  }
};

TEST(connectionControllerDiscoveryConnected, goesOnToIotHubConnecting)
{
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);
  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::IotHubConnecting, state);
}

TEST(connectionControllerDiscoveryConnected, goesBackToNotConnectedIfWifiDisconnected)
{
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(false);
  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::NotConnected, state);
}

/////////////////////////
// IotHubConnecting

TEST_GROUP(connectionControllerIotHubConnecting)
{
  TEST_SETUP()
  {
    commonTestSetup();

    mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(false);
    mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(true);
    mock().expectOneCall("AzureDriverMock::isIdle").andReturnValue(true);
    mock().ignoreOtherCalls();
    connectionController->run(false);
    auto state = connectionController->getState();
    CHECK_EQUAL(state, ConnectionState::WifiConnecting);

    mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(false);
    mock().expectOneCall("WifiDriverMock::connectToWlan").andReturnValue(true);
    connectionController->run(false);
    state = connectionController->getState();
    CHECK_EQUAL(state, ConnectionState::WifiConnected);

    mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);
    connectionController->run(false);
    state = connectionController->getState();
    CHECK_EQUAL(state, ConnectionState::DiscoveryConnecting);

    mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);
    mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(false);
    mock().expectOneCall("CloudServiceMock::wasDiscoverySuccessfulSinceLastRestart")
        .andReturnValue(true);
    mock().expectOneCall("CloudServiceMock::getCurrentHealth")
        .andReturnValue(100u);
    connectionController->run(false);
    state = connectionController->getState();
    CHECK_EQUAL(ConnectionState::DiscoveryConnected, state);

    mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);
    connectionController->run(false);
    state = connectionController->getState();
    CHECK_EQUAL(ConnectionState::IotHubConnecting, state);
  }

  TEST_TEARDOWN()
  {
    commonTestTearDown();
  }
};

TEST(connectionControllerIotHubConnecting, goesToIotHubConnectedIfConnectedToIotHub)
{
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);
  mock().expectOneCall("AzureDriverMock::getStatus")
      .andReturnValue(static_cast<int16_t>(CloudStatus::Connected));
  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::IotHubConnected, state);
}

TEST(connectionControllerIotHubConnecting, staysInIotHubConnectingIfAzureDriverIsConnecting)
{
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);
  mock().expectOneCall("AzureDriverMock::getStatus")
      .andReturnValue(static_cast<int16_t>(CloudStatus::Connecting));
  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::IotHubConnecting, state);
}

TEST(connectionControllerIotHubConnecting, tellsAzureDriverToConnectIfIdle)
{
  ConnectionStrings_t dummyStrings { "primary-connection-string", "secondary-connection-string" };

  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);
  mock().expectOneCall("QueueMockStruct::numWaitingItems").andReturnValue(1u);
  mock().expectOneCall("QueueMockStruct::pop")
      .withOutputParameterReturning("item", static_cast<void *>(&dummyStrings), sizeof(dummyStrings))
      .ignoreOtherParameters()
      .andReturnValue(true);
  mock().expectOneCall("QueueMockStruct::numWaitingItems").andReturnValue(0u);
  mock().expectOneCall("AzureDriverMock::getStatus")
      .andReturnValue(static_cast<int16_t>(CloudStatus::Idle));
  mock().expectOneCall("AzureDriverMock::connect")
      .withStringParameter("connectionString", dummyStrings.primary)
      .andReturnValue(true);
  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::IotHubConnecting, state);

  // Also check that secondary connection string is used if the first one doesn't work
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);
  mock().expectOneCall("QueueMockStruct::numWaitingItems").andReturnValue(1u);
  mock().expectOneCall("QueueMockStruct::pop")
      .withOutputParameterReturning("item", &dummyStrings, sizeof(dummyStrings))
      .ignoreOtherParameters()
      .andReturnValue(true);
  mock().expectOneCall("QueueMockStruct::numWaitingItems").andReturnValue(0u);
  mock().expectOneCall("AzureDriverMock::getStatus")
      .andReturnValue(static_cast<int16_t>(CloudStatus::Error));
  mock().expectOneCall("AzureDriverMock::connect")
      .withStringParameter("connectionString", dummyStrings.secondary)
      .andReturnValue(true);
  connectionController->run(false);
  state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::IotHubConnecting, state);

  // Check that there is no third attempt to connect
  // but rather, a reboot

  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);
  mock().expectOneCall("QueueMockStruct::numWaitingItems").andReturnValue(0u);
  mock().expectOneCall("AzureDriverMock::getStatus")
      .andReturnValue(static_cast<int16_t>(CloudStatus::Error));
  mock().expectNoCall("AzureDriverMock::connect");
  connectionController->run(false);
}

TEST(connectionControllerIotHubConnecting, rebootIfStuckTooLongInIotHubConnecting)
{
  mock().expectOneCall("getTimeSinceTimestampMS")
      .ignoreOtherParameters()
      .andReturnValue(static_cast<uint32_t>(Constants::IOTHUB_CONNECT_TIMEOUT_MS+1));
  mock().expectOneCall("SoftwareResetDriverMock::reset");
  // This expect is necessary because of test setup
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);
  connectionController->run(false);
}

/////////////////////////
// IotHubConnected

TEST_GROUP(connectionControllerIotHubConnected)
{
  TEST_SETUP()
  {
    commonTestSetup();

    mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(false);
    mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(true);
    mock().expectOneCall("AzureDriverMock::isIdle").andReturnValue(true);
    mock().ignoreOtherCalls();
    connectionController->run(false);
    auto state = connectionController->getState();
    CHECK_EQUAL(state, ConnectionState::WifiConnecting);

    mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(false);
    mock().expectOneCall("WifiDriverMock::connectToWlan").andReturnValue(true);
    connectionController->run(false);
    state = connectionController->getState();
    CHECK_EQUAL(state, ConnectionState::WifiConnected);

    mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);
    connectionController->run(false);
    state = connectionController->getState();
    CHECK_EQUAL(state, ConnectionState::DiscoveryConnecting);

    mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);
    mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(false);
    mock().expectOneCall("CloudServiceMock::wasDiscoverySuccessfulSinceLastRestart")
        .andReturnValue(true);
    mock().expectOneCall("CloudServiceMock::getCurrentHealth")
        .andReturnValue(100u);
    connectionController->run(false);
    state = connectionController->getState();
    CHECK_EQUAL(ConnectionState::DiscoveryConnected, state);

    mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);
    mock().expectOneCall("AzureDriverMock::getStatus").andReturnValue(static_cast<int16_t>(CloudStatus::Connected));
    connectionController->run(false);
    state = connectionController->getState();
    CHECK_EQUAL(ConnectionState::IotHubConnecting, state);

    mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);
    connectionController->run(false);
    state = connectionController->getState();
    CHECK_EQUAL(ConnectionState::IotHubConnected, state);
  }

  TEST_TEARDOWN()
  {
    commonTestTearDown();
  }
};

TEST(connectionControllerIotHubConnected, goesBackToNotConnectedIfWifiDisconnected)
{
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(false);
  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::NotConnected, state);
}

TEST(connectionControllerIotHubConnected, goesBackToDiscoveryConnectingIfIotHubDisconnected)
{
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);
  mock().expectOneCall("AzureDriverMock::isConnected").andReturnValue(false);
  mock().expectOneCall("AzureDriverMock::requestShutdown");
  mock().expectNoCall("CloudServiceMock::requestPause");
  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::DiscoveryConnecting, state);
}

TEST(connectionControllerIotHubConnected, goesBackToDiscoveryConnectingIfHealthIs0)
{
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);
  mock().expectOneCall("AzureDriverMock::isConnected").andReturnValue(true);
  mock().expectOneCall("CloudServiceMock::getCurrentHealth").andReturnValue(0u);
  mock().expectOneCall("AzureDriverMock::requestShutdown");
  mock().expectNoCall("CloudServiceMock::requestPause");
  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::DiscoveryConnecting, state);
}

TEST(connectionControllerIotHubConnected, goesBackToDiscoveryConnectingIfHealthIs99)
{
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);
  mock().expectOneCall("AzureDriverMock::isConnected").andReturnValue(true);
  mock().expectOneCall("CloudServiceMock::getCurrentHealth").andReturnValue(99u);
  mock().expectOneCall("AzureDriverMock::requestShutdown");
  mock().expectNoCall("CloudServiceMock::requestPause");
  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::DiscoveryConnecting, state);
}

TEST(connectionControllerIotHubConnected, staysInIotHubConnectedIfHealthIs100)
{
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);
  mock().expectOneCall("AzureDriverMock::isConnected").andReturnValue(true);
  mock().expectOneCall("CloudServiceMock::getCurrentHealth").andReturnValue(100u);
  mock().expectNoCall("AzureDriverMock::requestShutdown");
  mock().expectNoCall("CloudServiceMock::requestPause");
  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::IotHubConnected, state);
}

/////////////////////////
// ActivatingWifiSetup

TEST_GROUP(connectionControllerActivatingWifiSetup)
{
  TEST_SETUP()
  {
    commonTestSetup();

    WifiConnectionStatus::Enum wifiSetupResult = WifiConnectionStatus::IN_PROGRESS;
    mock().expectOneCall("QueueMock::pop")
        .withOutputParameterReturning("item",
                                      static_cast<void *>(&wifiSetupResult),
                                      sizeof(wifiSetupResult))
        .ignoreOtherParameters()
        .andReturnValue(false);

    ConnectionStatusChangeRequest::Enum bluetoothOn
      = ConnectionStatusChangeRequest::bluetoothOn;
    mock().expectOneCall("QueueMock::pop")
        .withOutputParameterReturning("item",
                                      static_cast<void *>(&bluetoothOn),
                                      sizeof(bluetoothOn))
        .ignoreOtherParameters()
        .andReturnValue(true);
    connectionController->run(false);
    auto state = connectionController->getState();
    CHECK_EQUAL(state, ConnectionState::ActivatingWifiSetup);

    mock().expectOneCall("QueueMock::pop")
        .withOutputParameterReturning("item",
                                      static_cast<void *>(&wifiSetupResult),
                                      sizeof(wifiSetupResult))
        .ignoreOtherParameters()
        .andReturnValue(false);

    ConnectionStatusChangeRequest::Enum noChange = ConnectionStatusChangeRequest::noChange;
    mock().expectOneCall("QueueMock::pop")
        .withOutputParameterReturning("item",
                                      static_cast<void *>(&noChange),
                                      sizeof(noChange))
        .ignoreOtherParameters()
        .andReturnValue(false);
  }

  TEST_TEARDOWN()
  {
    commonTestTearDown();
  }
};

TEST(connectionControllerActivatingWifiSetup, shutsdownAzureIfRunning)
{
  mock().expectOneCall("AzureDriverMock::isIdle").andReturnValue(false);
  mock().expectOneCall("AzureDriverMock::requestShutdown");
  connectionController->run(false);

  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::ActivatingWifiSetup, state);
}

TEST(connectionControllerActivatingWifiSetup, pausesCloudServiceTaskIfRunning)
{
  mock().expectOneCall("AzureDriverMock::isIdle").andReturnValue(true);
  mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(false);
  mock().expectOneCall("CloudServiceMock::requestPause").andReturnValue(true);
  connectionController->run(false);

  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::ActivatingWifiSetup, state);
}

TEST(connectionControllerActivatingWifiSetup, disconnectsWifiIfConnected)
{
  mock().expectOneCall("AzureDriverMock::isIdle").andReturnValue(true);
  mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(true);
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);
  mock().expectOneCall("WifiDriverMock::disconnectFromWlan");
  connectionController->run(false);

  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::ActivatingWifiSetup, state);
}

TEST(connectionControllerActivatingWifiSetup, enablesBleGattServiceIfNotEnabled)
{
  mock().expectOneCall("AzureDriverMock::isIdle").andReturnValue(true);
  mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(true);
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(false);
  mock().expectOneCall("BTWifiSetupDriverMock::isGattServiceRunning").andReturnValue(false);
  mock().expectOneCall("BTWifiSetupDriverMock::startGattService").andReturnValue(true);
  connectionController->run(false);

  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::ActivatingWifiSetup, state);
}

TEST(connectionControllerActivatingWifiSetup, waitsForReconnectCommandOverBleIfGattServiceIsRunning)
{
  mock().expectOneCall("AzureDriverMock::isIdle").andReturnValue(true);
  mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(true);
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(false);
  mock().expectOneCall("BTWifiSetupDriverMock::isGattServiceRunning").andReturnValue(true);
  mock().expectOneCall("BTWifiSetupDriverMock::waitForReconnectCmdReceived").ignoreOtherParameters().andReturnValue(false);
  connectionController->run(false);

  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::ActivatingWifiSetup, state);
}

TEST(connectionControllerActivatingWifiSetup, doesNotResetDeviceWhenAbleToPrepareForWifiSetupInTime)
{
  mock().expectOneCall("AzureDriverMock::isIdle").andReturnValue(true);
  mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(true);
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(false);
  mock().expectOneCall("BTWifiSetupDriverMock::isGattServiceRunning").andReturnValue(true);
  mock().expectOneCall("BTWifiSetupDriverMock::waitForReconnectCmdReceived").ignoreOtherParameters().andReturnValue(false);

  mock().expectOneCall("getTimeSinceTimestampMS")
      .ignoreOtherParameters()
      .andReturnValue(static_cast<uint32_t>(Constants::ACTIVATING_WIFI_SETUP_TIMEOUT_MS+1u));
  mock().expectNoCall("SoftwareResetDriverMock::reset");
  connectionController->run(false);

  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::ActivatingWifiSetup, state);
}

TEST(connectionControllerActivatingWifiSetup, resetsDeviceWhenUnableToStartGattServiceInTime)
{
  mock().expectOneCall("AzureDriverMock::isIdle").andReturnValue(true);
  mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(true);
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(false);
  mock().expectOneCall("BTWifiSetupDriverMock::isGattServiceRunning").andReturnValue(false);

  mock().expectOneCall("getTimeSinceTimestampMS")
      .ignoreOtherParameters()
      .andReturnValue(static_cast<uint32_t>(Constants::ACTIVATING_WIFI_SETUP_TIMEOUT_MS+1u));
  mock().expectOneCall("SoftwareResetDriverMock::reset");
  connectionController->run(false);

  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::ActivatingWifiSetup, state);
}

TEST(connectionControllerActivatingWifiSetup, resetsDeviceWhenUnableToDisconnectWlanInTime)
{
  mock().expectOneCall("AzureDriverMock::isIdle").andReturnValue(true);
  mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(true);
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);

  mock().expectOneCall("getTimeSinceTimestampMS")
      .ignoreOtherParameters()
      .andReturnValue(static_cast<uint32_t>(Constants::ACTIVATING_WIFI_SETUP_TIMEOUT_MS+1u));
  mock().expectOneCall("SoftwareResetDriverMock::reset");
  connectionController->run(false);

  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::ActivatingWifiSetup, state);
}

TEST(connectionControllerActivatingWifiSetup, resetsDeviceWhenUnableToPauseCloudServiceInTime)
{
  mock().expectOneCall("AzureDriverMock::isIdle").andReturnValue(true);
  mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(false);

  mock().expectOneCall("getTimeSinceTimestampMS")
      .ignoreOtherParameters()
      .andReturnValue(static_cast<uint32_t>(Constants::ACTIVATING_WIFI_SETUP_TIMEOUT_MS+1u));
  mock().expectOneCall("SoftwareResetDriverMock::reset");
  connectionController->run(false);

  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::ActivatingWifiSetup, state);
}

TEST(connectionControllerActivatingWifiSetup, resetsDeviceWhenUnableToShutdownAzureInTime)
{
  mock().expectOneCall("AzureDriverMock::isIdle").andReturnValue(false);

  mock().expectOneCall("getTimeSinceTimestampMS")
      .ignoreOtherParameters()
      .andReturnValue(static_cast<uint32_t>(Constants::ACTIVATING_WIFI_SETUP_TIMEOUT_MS+1u));
  mock().expectOneCall("SoftwareResetDriverMock::reset");
  connectionController->run(false);

  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::ActivatingWifiSetup, state);
}

TEST(connectionControllerActivatingWifiSetup, doesNotResetDeviceWhenUnableToShutdownAzureIfTimeoutIsntReached)
{
  mock().expectOneCall("AzureDriverMock::isIdle").andReturnValue(false);

  mock().expectOneCall("getTimeSinceTimestampMS")
      .ignoreOtherParameters()
      .andReturnValue(static_cast<uint32_t>(Constants::ACTIVATING_WIFI_SETUP_TIMEOUT_MS-1u));
  mock().expectNoCall("SoftwareResetDriverMock::reset");
  connectionController->run(false);

  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::ActivatingWifiSetup, state);
}

TEST(connectionControllerActivatingWifiSetup, configuresConnectionDynamicAndGoesToNotConnectedIfReconnectCommandReceivedOverBle)
{
  mock().expectOneCall("AzureDriverMock::isIdle").andReturnValue(true);
  mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(true);
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(false);
  mock().expectOneCall("BTWifiSetupDriverMock::isGattServiceRunning").andReturnValue(true);
  mock().expectOneCall("BTWifiSetupDriverMock::waitForReconnectCmdReceived").ignoreOtherParameters().andReturnValue(true);

  char testSsid[] = "my_wifi_ssid";
  mock().expectOneCall("BTWifiSetupDriverMock::getWifiSSID")
      .withOutputParameterReturning("ssid", testSsid, sizeof(testSsid))
      .ignoreOtherParameters();
  char testPassword[] = "my_wifi_password";
  mock().expectOneCall("BTWifiSetupDriverMock::getWifiPassword")
      .withOutputParameterReturning("password", testPassword, sizeof(testPassword))
      .ignoreOtherParameters();
  mock().expectOneCall("BTWifiSetupDriverMock::getStaticIp").andReturnValue(0u);
  mock().expectOneCall("BTWifiSetupDriverMock::registerReconnectCmdConsumed");

  mock().expectOneCall("setWifiSSID").withStringParameter("ssid", testSsid);
  mock().expectOneCall("setWifiPassword").withStringParameter("password", testPassword);

  mock().expectOneCall("WifiDriverMock::configureConnection")
      .ignoreOtherParameters()
      .andReturnValue(true); // TODO test that false generates an error response to BLE master

  mock().expectOneCall("WifiDriverMock::configureDynamicIp").andReturnValue(true);
  mock().expectOneCall("BTWifiSetupDriverMock::setDynamicIpInUse");

  connectionController->run(false);

  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::NotConnected, state);
}

TEST(connectionControllerActivatingWifiSetup, configuresConnectionStaticAndGoesToNotConnectedIfReconnectCommandReceivedOverBle)
{
  mock().expectOneCall("AzureDriverMock::isIdle").andReturnValue(true);
  mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(true);
  mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(false);
  mock().expectOneCall("BTWifiSetupDriverMock::isGattServiceRunning").andReturnValue(true);
  mock().expectOneCall("BTWifiSetupDriverMock::waitForReconnectCmdReceived").ignoreOtherParameters().andReturnValue(true);

  mock().expectOneCall("BTWifiSetupDriverMock::getWifiSSID").ignoreOtherParameters();
  mock().expectOneCall("BTWifiSetupDriverMock::getWifiPassword").ignoreOtherParameters();
  mock().expectOneCall("BTWifiSetupDriverMock::getStaticIp").andReturnValue(1u);
  mock().expectOneCall("BTWifiSetupDriverMock::getStaticNetmask").andReturnValue(0x00ffffffu);
  mock().expectOneCall("BTWifiSetupDriverMock::getStaticGateway").andReturnValue(2u);
  mock().expectOneCall("BTWifiSetupDriverMock::getStaticDns1").andReturnValue(3u);
  mock().expectOneCall("BTWifiSetupDriverMock::getStaticDns2").andReturnValue(4u);
  mock().expectOneCall("BTWifiSetupDriverMock::registerReconnectCmdConsumed");

  mock().expectOneCall("setWifiSSID").ignoreOtherParameters();
  mock().expectOneCall("setWifiPassword").ignoreOtherParameters();

  mock().expectOneCall("setStaticIp").withUnsignedIntParameter("staticIp", 1u);
  mock().expectOneCall("setStaticNetmask").withUnsignedIntParameter("staticNetmask", 0x00ffffffu);
  mock().expectOneCall("setStaticGateway").withUnsignedIntParameter("staticGateway", 2u);
  mock().expectOneCall("setStaticDns1").withUnsignedIntParameter("staticDns1", 3u);
  mock().expectOneCall("setStaticDns2").withUnsignedIntParameter("staticDns2", 4u);

  mock().expectOneCall("WifiDriverMock::configureConnection")
      .ignoreOtherParameters()
      .andReturnValue(true);

  mock().expectOneCall("WifiDriverMock::configureStaticIp").withUnsignedIntParameter("staticIp", 1u)
                                                           .withUnsignedIntParameter("staticNetmask", 0x00ffffffu)
                                                           .withUnsignedIntParameter("staticGateway", 2u)
                                                           .withUnsignedIntParameter("staticDns1", 3u)
                                                           .withUnsignedIntParameter("staticDns2", 4u)
                                                           .andReturnValue(true);
  mock().expectOneCall("BTWifiSetupDriverMock::setStaticIpInUse");

  connectionController->run(false);

  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::NotConnected, state);
}

///////////////////////
// WaitingForBleOff

TEST_GROUP(connectionControllerWaitingForBleOff)
{
  TEST_SETUP()
  {
    commonTestSetup();

    mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(false);
    mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(true);
    mock().expectOneCall("AzureDriverMock::isIdle").andReturnValue(true);
    mock().ignoreOtherCalls();
    connectionController->run(false);
    auto state = connectionController->getState();
    CHECK_EQUAL(state, ConnectionState::WifiConnecting);

    mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(false);
    mock().expectOneCall("WifiDriverMock::connectToWlan").andReturnValue(true);
    connectionController->run(false);
    state = connectionController->getState();
    CHECK_EQUAL(state, ConnectionState::WifiConnected);

    mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);
    connectionController->run(false);
    state = connectionController->getState();
    CHECK_EQUAL(state, ConnectionState::DiscoveryConnecting);

    mock().expectOneCall("WifiDriverMock::isConnectedToWlan").andReturnValue(true);
    mock().expectOneCall("CloudServiceMock::isPaused").andReturnValue(false);
    mock().expectOneCall("CloudServiceMock::wasDiscoverySuccessfulSinceLastRestart")
        .andReturnValue(true);
    mock().expectOneCall("BTWifiSetupDriverMock::isGattServiceRunning")
        .andReturnValue(true);
    connectionController->run(false);
    state = connectionController->getState();
    CHECK_EQUAL(ConnectionState::WaitingForBleOff, state);
  }

  TEST_TEARDOWN()
  {
    commonTestTearDown();
  }
};

TEST(connectionControllerWaitingForBleOff, disablesWifiSetupServiceAndGoesToNotConnectedOnBleOffCommand)
{
  WifiConnectionStatus::Enum wifiSetupResult = WifiConnectionStatus::IN_PROGRESS;
  mock().expectOneCall("QueueMock::pop")
      .withOutputParameterReturning("item",
                                    static_cast<void *>(&wifiSetupResult),
                                    sizeof(wifiSetupResult))
      .ignoreOtherParameters()
      .andReturnValue(false);

  ConnectionStatusChangeRequest::Enum bluetoothOff
    = ConnectionStatusChangeRequest::bluetoothOff;
  mock().expectOneCall("QueueMock::pop")
      .withOutputParameterReturning("item",
                                    static_cast<void *>(&bluetoothOff),
                                    sizeof(bluetoothOff))
      .ignoreOtherParameters()
      .andReturnValue(true);
  mock().expectOneCall("BTWifiSetupDriverMock::isGattServiceRunning")
      .andReturnValue(true);
  mock().expectOneCall("BTWifiSetupDriverMock::stopGattService");
  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::NotConnected, state);
}

///////////////////////
//

TEST_GROUP(connectionControllerFactoryReset)
{
  TEST_SETUP()
  {
    commonTestSetup();

    mock().expectOneCall("getForgetMeState")
        .andReturnValue(static_cast<unsigned int>(ForgetMeState::resetConfirmed));
    mock().ignoreOtherCalls();
    connectionController->run(false);
  }

  TEST_TEARDOWN()
  {
    commonTestTearDown();
  }
};

TEST(connectionControllerFactoryReset, entersCorrectly)
{
  auto state = connectionController->getState();
  CHECK_EQUAL(state, ConnectionState::FactoryReset);
}

TEST(connectionControllerFactoryReset, leavesIfNoResetRequested)
{
  mock().expectNCalls(2u, "getForgetMeState")
      .andReturnValue(static_cast<unsigned int>(ForgetMeState::noResetRequested));

  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(state, ConnectionState::NotConnected);
}

TEST(connectionControllerFactoryReset, disconnectAzure)
{
  mock().expectNCalls(2u, "getForgetMeState")
      .andReturnValue(static_cast<unsigned int>(ForgetMeState::resetConfirmed));

  mock().expectOneCall("AzureDriverMock::isIdle")
      .andReturnValue(false);
  mock().expectOneCall("AzureDriverMock::requestShutdown");

  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(state, ConnectionState::FactoryReset);
}

TEST(connectionControllerFactoryReset, pauseCloudService)
{
  mock().expectNCalls(2u, "getForgetMeState")
      .andReturnValue(static_cast<unsigned int>(ForgetMeState::resetConfirmed));

  mock().expectOneCall("AzureDriverMock::isIdle")
      .andReturnValue(true);

  mock().expectOneCall("CloudServiceMock::isPaused")
      .andReturnValue(false);
  mock().expectOneCall("CloudServiceMock::requestPause");

  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(state, ConnectionState::FactoryReset);
}

TEST(connectionControllerFactoryReset, disconnectsFromWlan)
{
  mock().expectNCalls(2u, "getForgetMeState")
      .andReturnValue(static_cast<unsigned int>(ForgetMeState::resetConfirmed));

  mock().expectOneCall("AzureDriverMock::isIdle")
      .andReturnValue(true);

  mock().expectOneCall("CloudServiceMock::isPaused")
      .andReturnValue(true);

  mock().expectOneCall("WifiDriverMock::isConnectedToWlan")
      .andReturnValue(true);
  mock().expectOneCall("WifiDriverMock::disconnectFromWlan");

  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(state, ConnectionState::FactoryReset);
}

TEST(connectionControllerFactoryReset, performsFactoryResetWhenEverythingIsDisconnected)
{
  mock().expectNCalls(2u, "getForgetMeState")
      .andReturnValue(static_cast<unsigned int>(ForgetMeState::resetConfirmed));

  mock().expectOneCall("AzureDriverMock::isIdle")
      .andReturnValue(true);

  mock().expectOneCall("CloudServiceMock::isPaused")
      .andReturnValue(true);

  mock().expectOneCall("WifiDriverMock::isConnectedToWlan")
      .andReturnValue(false);

  mock().expectOneCall("factoryResetS");
  mock().expectOneCall("WifiDriverMock::configureConnection")
      .withStringParameter("ssID", Constants::DEFAULT_WIFI_SSID)
      .withStringParameter("password", Constants::DEFAULT_WIFI_PASSWORD)
      .andReturnValue(true);

  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(state, ConnectionState::NotConnected);
}

TEST(connectionControllerFactoryReset, performsFactoryResetIfDisconnectTakesTooLong)
{
  mock().expectNCalls(2u, "getForgetMeState")
      .andReturnValue(static_cast<unsigned int>(ForgetMeState::resetConfirmed));

  mock().expectOneCall("AzureDriverMock::isIdle")
      .andReturnValue(false);
  mock().expectOneCall("getTimeSinceTimestampMS")
      .andReturnValue(static_cast<long unsigned int>(Constants::CONNECTION_CONTROLLER_DISCONNECTING_TIMEOUT1_MS+1u))
      .ignoreOtherParameters();

  mock().expectOneCall("CloudServiceMock::isPaused")
      .andReturnValue(false);
  mock().expectOneCall("getTimeSinceTimestampMS")
      .andReturnValue(static_cast<long unsigned int>(Constants::CONNECTION_CONTROLLER_DISCONNECTING_TIMEOUT1_MS+1u))
      .ignoreOtherParameters();;

  mock().expectOneCall("WifiDriverMock::isConnectedToWlan")
      .andReturnValue(true);
  mock().expectNCalls(1, "getTimeSinceTimestampMS")
      .andReturnValue(static_cast<long unsigned int>(Constants::CONNECTION_CONTROLLER_DISCONNECTING_TIMEOUT2_MS+1u))
      .ignoreOtherParameters();;

  mock().expectOneCall("factoryResetS");
  mock().expectOneCall("WifiDriverMock::configureConnection")
      .withStringParameter("ssID", Constants::DEFAULT_WIFI_SSID)
      .withStringParameter("password", Constants::DEFAULT_WIFI_PASSWORD)
      .andReturnValue(true);

  connectionController->run(false);
  auto state = connectionController->getState();
  CHECK_EQUAL(ConnectionState::NotConnected, state);
}
