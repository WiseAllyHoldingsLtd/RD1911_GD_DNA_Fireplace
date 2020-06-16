#include "CppUTest\TestHarness.h"
#include "CppUTestExt\MockSupport.h"

#include "BluetoothSetupScreenController.h"
#include "SettingsMock.h"
#include "Constants.h"
#include "ButtonStatus.h"


TEST_GROUP(BluetoothSetupScreenController)
{
  TEST_SETUP()
  {
  }

  TEST_TEARDOWN()
  {
    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
  }
};


TEST(BluetoothSetupScreenController, testReturnsFalseWhenNothingIsDone)
{
  SettingsMock settings;
  BluetoothSetupScreenController controller;
  UIState uiState = { Screen::bluetoothSetupScreen };
  ButtonStatus buttonStatus = {};

  controller.enter(uiState, settings);
  bool isButtonResetRequested = controller.control(0U, uiState, buttonStatus, settings);
  CHECK_FALSE(isButtonResetRequested);
}

TEST(BluetoothSetupScreenController, testDefaultTimeoutIfBluetoothActiveButNoConnection)
{
  SettingsMock settings;
  BluetoothSetupScreenController controller;
  UIState uiState = { Screen::bluetoothSetupScreen };
  uiState.currentScreenEnterTimestampMs = 0U;
  uiState.isBluetoothActive = true;
  ButtonStatus buttonStatus = {};

  controller.enter(uiState, settings);
  bool isButtonResetRequested = controller.control(Constants::BLUETOOTH_SCREEN_DEFAULT_CONN_WAIT_TIME_MS - 1U, uiState, buttonStatus, settings);
  CHECK_FALSE(isButtonResetRequested);
  CHECK_EQUAL(Screen::bluetoothSetupScreen, uiState.currentScreen);

  isButtonResetRequested = controller.control(Constants::BLUETOOTH_SCREEN_DEFAULT_CONN_WAIT_TIME_MS, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested);
  CHECK_EQUAL(Screen::mainScreen, uiState.currentScreen);
}

TEST(BluetoothSetupScreenController, testShortTimeoutIfBluetoothDisconnectsAfterItHasBeenConnectedForAShortPeriod)
{
  // This test assumes that app has been connected then disconnects after 10 secs.
  // A shorter timeout value is applied then.

  SettingsMock settings;
  BluetoothSetupScreenController controller;
  UIState uiState = { Screen::bluetoothSetupScreen };
  uiState.currentScreenEnterTimestampMs = 0U;
  uiState.isBluetoothActive = true;
  ButtonStatus buttonStatus = {};

  controller.enter(uiState, settings);
  controller.control(1U, uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::bluetoothSetupScreen, uiState.currentScreen);

  uiState.isBluetoothConnected = true;
  controller.control(2U, uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::bluetoothSetupScreen, uiState.currentScreen);

  // Disconnected after 20 sec
  uiState.isBluetoothConnected = false;
  controller.control(20000U, uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::bluetoothSetupScreen, uiState.currentScreen);

  // Don't leave yet
  controller.control(19999U + Constants::BLUETOOTH_SCREEN_SHORT_CONN_WAIT_TIME_MS, uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::bluetoothSetupScreen, uiState.currentScreen);

  // Leave screen 'shortTimeout' after disconnection @ 20 secs
  controller.control(20000U + Constants::BLUETOOTH_SCREEN_SHORT_CONN_WAIT_TIME_MS, uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::mainScreen, uiState.currentScreen);
}

TEST(BluetoothSetupScreenController, testShortTimeoutIfBluetoothDisconnectsAfterItHasBeenConnectedForAWhile)
{
  SettingsMock settings;
  BluetoothSetupScreenController controller;
  UIState uiState = { Screen::bluetoothSetupScreen };
  uiState.currentScreenEnterTimestampMs = 0U;
  uiState.isBluetoothActive = true;
  ButtonStatus buttonStatus = {};

  controller.enter(uiState, settings);
  controller.control(1U, uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::bluetoothSetupScreen, uiState.currentScreen);

  uiState.isBluetoothConnected = true;
  controller.control(2U, uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::bluetoothSetupScreen, uiState.currentScreen);

  // Disconnected after 70 sec
  uiState.isBluetoothConnected = false;
  controller.control(70000U, uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::bluetoothSetupScreen, uiState.currentScreen);

  // Don't leave yet
  controller.control(69999U + Constants::BLUETOOTH_SCREEN_SHORT_CONN_WAIT_TIME_MS, uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::bluetoothSetupScreen, uiState.currentScreen);

  // Leave screen 'shortTimeout' after disconnection @ 70 secs
  controller.control(70000U + Constants::BLUETOOTH_SCREEN_SHORT_CONN_WAIT_TIME_MS, uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::mainScreen, uiState.currentScreen);
}

TEST(BluetoothSetupScreenController, testThatWeDoNotLeaveScreenAfterConnectDisconnectConnect)
{
  SettingsMock settings;
  BluetoothSetupScreenController controller;
  UIState uiState = { Screen::bluetoothSetupScreen };
  uiState.currentScreenEnterTimestampMs = 0U;
  uiState.isBluetoothActive = true;
  ButtonStatus buttonStatus = {};

  controller.enter(uiState, settings);
  controller.control(1U, uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::bluetoothSetupScreen, uiState.currentScreen);

  uiState.isBluetoothConnected = true;
  controller.control(2U, uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::bluetoothSetupScreen, uiState.currentScreen);

  // Disconnected after 20 sec
  uiState.isBluetoothConnected = false;
  controller.control(20000U, uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::bluetoothSetupScreen, uiState.currentScreen);

  // Connect again
  uiState.isBluetoothConnected = true;
  controller.control(19999U + Constants::BLUETOOTH_SCREEN_SHORT_CONN_WAIT_TIME_MS, uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::bluetoothSetupScreen, uiState.currentScreen);

  // Don't leave
  controller.control(19999U + (2*Constants::BLUETOOTH_SCREEN_SHORT_CONN_WAIT_TIME_MS), uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::bluetoothSetupScreen, uiState.currentScreen);

  controller.control(19999U + Constants::BLUETOOTH_SCREEN_SHORT_CONN_WAIT_TIME_MS + Constants::BLUETOOTH_SCREEN_DEFAULT_CONN_WAIT_TIME_MS, uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::bluetoothSetupScreen, uiState.currentScreen);
}

TEST(BluetoothSetupScreenController, testThatWeLeaveScreenAfterShortDelayAfterLastDisconnect)
{
  SettingsMock settings;
  BluetoothSetupScreenController controller;
  UIState uiState = { Screen::bluetoothSetupScreen };
  uiState.currentScreenEnterTimestampMs = 0U;
  uiState.isBluetoothActive = true;
  ButtonStatus buttonStatus = {};

  controller.enter(uiState, settings);
  controller.control(1U, uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::bluetoothSetupScreen, uiState.currentScreen);

  uiState.isBluetoothConnected = true;
  controller.control(2U, uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::bluetoothSetupScreen, uiState.currentScreen);

  // Disconnected after 20 sec
  uiState.isBluetoothConnected = false;
  controller.control(20000U, uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::bluetoothSetupScreen, uiState.currentScreen);

  // Connect again
  uiState.isBluetoothConnected = true;
  controller.control(21000U, uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::bluetoothSetupScreen, uiState.currentScreen);

  // Disconnected again
  uiState.isBluetoothConnected = false;
  controller.control(30000U, uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::bluetoothSetupScreen, uiState.currentScreen);

  // Don't leave
  controller.control(29999U + Constants::BLUETOOTH_SCREEN_SHORT_CONN_WAIT_TIME_MS, uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::bluetoothSetupScreen, uiState.currentScreen);

  // Leave
  controller.control(30000U + Constants::BLUETOOTH_SCREEN_SHORT_CONN_WAIT_TIME_MS, uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::mainScreen, uiState.currentScreen);
}

TEST(BluetoothSetupScreenController, testNoTimeoutIfBluetoothActiveAndConnected)
{
  SettingsMock settings;
  BluetoothSetupScreenController controller;
  UIState uiState = { Screen::bluetoothSetupScreen };
  uiState.currentScreenEnterTimestampMs = 0U;
  uiState.isBluetoothActive = true;
  uiState.isBluetoothConnected = true;
  ButtonStatus buttonStatus = {};

  controller.enter(uiState, settings);
  bool isButtonResetRequested = controller.control(Constants::BLUETOOTH_SCREEN_DEFAULT_CONN_WAIT_TIME_MS, uiState, buttonStatus, settings);
  CHECK_FALSE(isButtonResetRequested);
  CHECK_EQUAL(Screen::bluetoothSetupScreen, uiState.currentScreen);
}

TEST(BluetoothSetupScreenController, testMaximumTimeoutEvenIfBluetoothActivity)
{
  SettingsMock settings;
  BluetoothSetupScreenController controller;
  UIState uiState = { Screen::bluetoothSetupScreen };
  uiState.currentScreenEnterTimestampMs = 0U;
  uiState.isBluetoothActive = true;
  uiState.isBluetoothConnected = true;
  ButtonStatus buttonStatus = {};

  controller.enter(uiState, settings);
  bool isButtonResetRequested = controller.control(Constants::BLUETOOTH_SCREEN_DEFAULT_CONN_WAIT_TIME_MS, uiState, buttonStatus, settings);
  CHECK_FALSE(isButtonResetRequested);
  CHECK_EQUAL(Screen::bluetoothSetupScreen, uiState.currentScreen);

  isButtonResetRequested = controller.control((Constants::BLUETOOTH_SCREEN_MAX_VISIBLE_TIME_S * 1000U) - 1U, uiState, buttonStatus, settings);
  CHECK_FALSE(isButtonResetRequested);
  CHECK_EQUAL(Screen::bluetoothSetupScreen, uiState.currentScreen);

  isButtonResetRequested = controller.control(Constants::BLUETOOTH_SCREEN_MAX_VISIBLE_TIME_S * 1000U, uiState, buttonStatus, settings);
  CHECK_FALSE(isButtonResetRequested);
  CHECK_EQUAL(Screen::mainScreen, uiState.currentScreen);
}

TEST(BluetoothSetupScreenController, testMaximumTimeoutEvenIfBluetoothActivityAndPlusMinusBeingPressed)
{
  SettingsMock settings;
  BluetoothSetupScreenController controller;
  UIState uiState = { Screen::bluetoothSetupScreen };
  uiState.currentScreenEnterTimestampMs = 0U;
  uiState.isBluetoothActive = true;
  uiState.isBluetoothConnected = true;
  ButtonStatus buttonStatus = {};

  controller.enter(uiState, settings);
  buttonStatus.wasButtonPressedAndReleased[Constants::PLUS_BUTTON_INDEX] = true;
  bool isButtonResetRequested = controller.control(Constants::BLUETOOTH_SCREEN_MAX_VISIBLE_TIME_S * 500U, uiState, buttonStatus, settings);
  buttonStatus.wasButtonPressedAndReleased[Constants::PLUS_BUTTON_INDEX] = false;
  CHECK_TRUE(isButtonResetRequested);

  buttonStatus.wasButtonPressedAndReleased[Constants::MINUS_BUTTON_INDEX] = true;
  isButtonResetRequested = controller.control(Constants::BLUETOOTH_SCREEN_MAX_VISIBLE_TIME_S * 800U, uiState, buttonStatus, settings);
  buttonStatus.wasButtonPressedAndReleased[Constants::MINUS_BUTTON_INDEX] = false;
  CHECK_TRUE(isButtonResetRequested);

  isButtonResetRequested = controller.control(Constants::BLUETOOTH_SCREEN_MAX_VISIBLE_TIME_S * 1000U, uiState, buttonStatus, settings);
  CHECK_FALSE(isButtonResetRequested);
  CHECK_EQUAL(Screen::mainScreen, uiState.currentScreen);
}

TEST(BluetoothSetupScreenController, testReturnsToMainScreenIfBackIsPressedAndReleased)
{
  SettingsMock settings;
  BluetoothSetupScreenController controller;
  UIState uiState = { Screen::bluetoothSetupScreen};
  uiState.currentScreenEnterTimestampMs = 0U;
  ButtonStatus buttonStatus = {};

  controller.enter(uiState, settings);
  buttonStatus.wasButtonPressedAndReleased[Constants::BACK_BUTTON_INDEX] = true;
  bool isButtonResetRequested = controller.control(0U, uiState, buttonStatus, settings);

  CHECK_TRUE(isButtonResetRequested);
  CHECK_EQUAL(Screen::mainScreen, uiState.currentScreen);
}

TEST(BluetoothSetupScreenController, testReturnsToMainScreenIfBackIsHeld)
{
  SettingsMock settings;
  BluetoothSetupScreenController controller;
  UIState uiState = { Screen::bluetoothSetupScreen};
  uiState.currentScreenEnterTimestampMs = 0U;
  ButtonStatus buttonStatus = {};

  controller.enter(uiState, settings);
  buttonStatus.isButtonHeld[Constants::BACK_BUTTON_INDEX] = true;
  buttonStatus.buttonHeldDurationMs[Constants::BACK_BUTTON_INDEX] = Constants::BUTTON_HELD_THRESHOLD_MS;
  bool isButtonResetRequested = controller.control(0U, uiState, buttonStatus, settings);

  CHECK_TRUE(isButtonResetRequested);
  CHECK_EQUAL(Screen::mainScreen, uiState.currentScreen);
}

TEST(BluetoothSetupScreenController, testReturnToMainScreenIfBluetoothWasActiveThenNotActive)
{
  // Logic behind behaviour is that deactivation probably happended because BT setup is completed
  SettingsMock settings;
  BluetoothSetupScreenController controller;
  UIState uiState = { Screen::bluetoothSetupScreen };
  uiState.currentScreenEnterTimestampMs = 0U;
  uiState.isBluetoothActive = true;
  uiState.isBluetoothConnected = true;
  ButtonStatus buttonStatus = {};

  controller.enter(uiState, settings);
  bool isButtonResetRequested = controller.control(0U, uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::bluetoothSetupScreen, uiState.currentScreen);

  uiState.isBluetoothActive = false;
  isButtonResetRequested = controller.control(1U, uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::mainScreen, uiState.currentScreen);
}
