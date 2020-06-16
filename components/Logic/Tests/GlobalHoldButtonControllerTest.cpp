#include "CppUTest\TestHarness.h"
#include "CppUTestExt\MockSupport.h"

#include "GlobalHoldButtonController.h"
#include "SettingsMock.h"
#include "ButtonDriverMock.h"
#include "Constants.h"

TEST_GROUP(globalHoldButtonController)
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


TEST(globalHoldButtonController, testReturnsFalseWhenNoActionPerformed)
{
  SettingsMock settings;
  ButtonDriverMock buttonDriver;
  GlobalHoldButtonController globalHoldButtonController;
  UIState uiState = { Screen::mainScreen };
  ButtonStatus buttonStatus = {};
  mock().ignoreOtherCalls();
  bool isButtonResetRequested = globalHoldButtonController.control(uiState, buttonStatus, buttonDriver, settings);
  CHECK_FALSE(isButtonResetRequested);
}

TEST(globalHoldButtonController, testScreenIsUnchangedWhenNoActionPerformed)
{
  SettingsMock settings;
  ButtonDriverMock buttonDriver;
  GlobalHoldButtonController globalHoldButtonController;
  UIState uiState = { Screen::mainScreen };
  ButtonStatus buttonStatus = {};
  mock().ignoreOtherCalls();
  bool isButtonResetRequested = globalHoldButtonController.control(uiState, buttonStatus, buttonDriver, settings);
  CHECK_EQUAL(Screen::mainScreen, uiState.currentScreen);
}

TEST(globalHoldButtonController, testReturnsTrueWhenButtonLockIsToggled)
{
  SettingsMock settings;
  ButtonDriverMock buttonDriver;
  GlobalHoldButtonController globalHoldButtonController;
  UIState uiState = { Screen::mainScreen };
  ButtonStatus buttonStatus = {};
  buttonStatus.isButtonHeld[Constants::PLUS_BUTTON_INDEX] = true;
  buttonStatus.isButtonHeld[Constants::MINUS_BUTTON_INDEX] = true;
  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_COMBO_HOLD_TIME_MS;
  buttonStatus.buttonHeldDurationMs[Constants::MINUS_BUTTON_INDEX] = Constants::BUTTON_COMBO_HOLD_TIME_MS;
  mock().ignoreOtherCalls();
  bool isButtonResetRequested = globalHoldButtonController.control(uiState, buttonStatus, buttonDriver, settings);
  CHECK_TRUE(isButtonResetRequested);
}

TEST(globalHoldButtonController, testGotoLockScreenWhenButtonsHeld)
{
  SettingsMock settings;
  ButtonDriverMock buttonDriver;
  GlobalHoldButtonController globalHoldButtonController;
  UIState uiState = { Screen::mainScreen };
  ButtonStatus buttonStatus = {};
  buttonStatus.isButtonHeld[Constants::PLUS_BUTTON_INDEX] = true;
  buttonStatus.isButtonHeld[Constants::MINUS_BUTTON_INDEX] = true;
  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_COMBO_HOLD_TIME_MS;
  buttonStatus.buttonHeldDurationMs[Constants::MINUS_BUTTON_INDEX] = Constants::BUTTON_COMBO_HOLD_TIME_MS;
  mock().expectNCalls(2, "isButtonLockOn").andReturnValue(true);
  mock().ignoreOtherCalls();
  bool isButtonResetRequested = globalHoldButtonController.control(uiState, buttonStatus, buttonDriver, settings);
  CHECK_EQUAL(Screen::buttonLockedScreen, uiState.currentScreen);
}

TEST(globalHoldButtonController, testGotoUnlockScreenWhenButtonsHeldWhenButtonsLocked)
{
  SettingsMock settings;
  ButtonDriverMock buttonDriver;
  GlobalHoldButtonController globalHoldButtonController;
  UIState uiState = { Screen::mainScreen };
  ButtonStatus buttonStatus = {};
  buttonStatus.isButtonHeld[Constants::PLUS_BUTTON_INDEX] = true;
  buttonStatus.isButtonHeld[Constants::MINUS_BUTTON_INDEX] = true;
  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_COMBO_HOLD_TIME_MS;
  buttonStatus.buttonHeldDurationMs[Constants::MINUS_BUTTON_INDEX] = Constants::BUTTON_COMBO_HOLD_TIME_MS;
  mock().expectNCalls(2, "isButtonLockOn").andReturnValue(false);
  mock().ignoreOtherCalls();
  bool isButtonResetRequested = globalHoldButtonController.control(uiState, buttonStatus, buttonDriver, settings);
  CHECK_EQUAL(Screen::buttonUnlockedScreen, uiState.currentScreen);
}

TEST(globalHoldButtonController, testLockingButtonsSetsCurrentScreenEntryTime)
{
  SettingsMock settings;
  ButtonDriverMock buttonDriver;
  GlobalHoldButtonController globalHoldButtonController;
  UIState uiState = { Screen::mainScreen };
  uiState.timeSinceBootMs = 100;
  uiState.currentScreenEnterTimestampMs = 50;
  ButtonStatus buttonStatus = {};
  buttonStatus.isButtonHeld[Constants::PLUS_BUTTON_INDEX] = true;
  buttonStatus.isButtonHeld[Constants::MINUS_BUTTON_INDEX] = true;
  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_COMBO_HOLD_TIME_MS;
  buttonStatus.buttonHeldDurationMs[Constants::MINUS_BUTTON_INDEX] = Constants::BUTTON_COMBO_HOLD_TIME_MS;
  mock().ignoreOtherCalls();
  bool isButtonResetRequested = globalHoldButtonController.control(uiState, buttonStatus, buttonDriver, settings);
  CHECK_EQUAL(100, uiState.currentScreenEnterTimestampMs);
}

TEST(globalHoldButtonController, testButtonLockToggledWhenButtonsHeld)
{
  SettingsMock settings;
  ButtonDriverMock buttonDriver;
  GlobalHoldButtonController globalHoldButtonController;
  UIState uiState = { Screen::mainScreen };
  ButtonStatus buttonStatus = {};
  buttonStatus.isButtonHeld[Constants::PLUS_BUTTON_INDEX]  = true;
  buttonStatus.isButtonHeld[Constants::MINUS_BUTTON_INDEX] = true;
  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX]  = Constants::BUTTON_COMBO_HOLD_TIME_MS;
  buttonStatus.buttonHeldDurationMs[Constants::MINUS_BUTTON_INDEX] = Constants::BUTTON_COMBO_HOLD_TIME_MS;
  mock().expectOneCall("toggleButtonLock");
  mock().ignoreOtherCalls();
  bool isButtonResetRequested = globalHoldButtonController.control(uiState, buttonStatus, buttonDriver, settings);
}

TEST(globalHoldButtonController, testButtonLockNotToggledWhenButtonsNotHeldLongEnough)
{
  SettingsMock settings;
  ButtonDriverMock buttonDriver;
  GlobalHoldButtonController globalHoldButtonController;
  UIState uiState = { Screen::mainScreen };
  ButtonStatus buttonStatus = {};
  buttonStatus.isButtonHeld[Constants::PLUS_BUTTON_INDEX] = true;
  buttonStatus.isButtonHeld[Constants::MINUS_BUTTON_INDEX] = true;
  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_COMBO_HOLD_TIME_MS-1;
  buttonStatus.buttonHeldDurationMs[Constants::MINUS_BUTTON_INDEX] = Constants::BUTTON_COMBO_HOLD_TIME_MS;
  mock().expectNCalls(0, "toggleButtonLock");
  mock().ignoreOtherCalls();
  bool isButtonResetRequested = globalHoldButtonController.control(uiState, buttonStatus, buttonDriver, settings);
}

TEST(globalHoldButtonController, testButtonLockNotToggledWhenThreeButtonsAreHeld)
{
  SettingsMock settings;
  ButtonDriverMock buttonDriver;
  GlobalHoldButtonController globalHoldButtonController;
  UIState uiState = { Screen::mainScreen };
  ButtonStatus buttonStatus = {};
  buttonStatus.isButtonHeld[Constants::MINUS_BUTTON_INDEX] = true;
  buttonStatus.isButtonHeld[Constants::BACK_BUTTON_INDEX] = true;
  buttonStatus.isButtonHeld[Constants::PLUS_BUTTON_INDEX] = true;
  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_COMBO_HOLD_TIME_MS;
  buttonStatus.buttonHeldDurationMs[Constants::MINUS_BUTTON_INDEX] = Constants::BUTTON_COMBO_HOLD_TIME_MS;
  mock().expectNCalls(0, "toggleButtonLock");
  mock().ignoreOtherCalls();
  bool isButtonResetRequested = globalHoldButtonController.control(uiState, buttonStatus, buttonDriver, settings);
}


TEST(globalHoldButtonController, testButtonTestScreenIsShown)
{
  SettingsMock settings;
  ButtonDriverMock buttonDriver;
  GlobalHoldButtonController globalHoldButtonController;
  UIState uiState = { Screen::mainScreen };
  ButtonStatus buttonStatus = {};
  buttonStatus.isButtonHeld[Constants::PLUS_BUTTON_INDEX] = true;
  buttonStatus.isButtonHeld[Constants::BACK_BUTTON_INDEX] = true;
  buttonStatus.isButtonHeld[Constants::MINUS_BUTTON_INDEX] = true;
  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_TEST_HOLD_TIME_MS;
  buttonStatus.buttonHeldDurationMs[Constants::BACK_BUTTON_INDEX] = Constants::BUTTON_TEST_HOLD_TIME_MS;
  buttonStatus.buttonHeldDurationMs[Constants::MINUS_BUTTON_INDEX] = Constants::BUTTON_TEST_HOLD_TIME_MS;

  mock().ignoreOtherCalls();

  globalHoldButtonController.control(uiState, buttonStatus, buttonDriver, settings);
  CHECK_EQUAL(Screen::buttonTestScreen, uiState.currentScreen);
}


TEST(globalHoldButtonController, testButtonTestScreenIsShownThenLockComboIsHeldNoScreenChange)
{
  SettingsMock settings;
  ButtonDriverMock buttonDriver;
  GlobalHoldButtonController globalHoldButtonController;
  UIState uiState = { Screen::mainScreen };
  ButtonStatus buttonStatus = {};
  buttonStatus.isButtonHeld[Constants::PLUS_BUTTON_INDEX] = true;
  buttonStatus.isButtonHeld[Constants::BACK_BUTTON_INDEX] = true;
  buttonStatus.isButtonHeld[Constants::MINUS_BUTTON_INDEX] = true;
  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_TEST_HOLD_TIME_MS;
  buttonStatus.buttonHeldDurationMs[Constants::BACK_BUTTON_INDEX] = Constants::BUTTON_TEST_HOLD_TIME_MS;
  buttonStatus.buttonHeldDurationMs[Constants::MINUS_BUTTON_INDEX] = Constants::BUTTON_TEST_HOLD_TIME_MS;

  mock().expectNCalls(0, "toggleButtonLock");
  mock().ignoreOtherCalls();

  globalHoldButtonController.control(uiState, buttonStatus, buttonDriver, settings);
  CHECK_EQUAL(Screen::buttonTestScreen, uiState.currentScreen);

  buttonStatus.isButtonHeld[Constants::BACK_BUTTON_INDEX] = false;
  buttonStatus.buttonHeldDurationMs[Constants::BACK_BUTTON_INDEX] = 0;
  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_COMBO_HOLD_TIME_MS*2;
  buttonStatus.buttonHeldDurationMs[Constants::MINUS_BUTTON_INDEX] = Constants::BUTTON_COMBO_HOLD_TIME_MS*2;

  globalHoldButtonController.control(uiState, buttonStatus, buttonDriver, settings);
  CHECK_EQUAL(Screen::buttonTestScreen, uiState.currentScreen)

}

TEST(globalHoldButtonController, testDoNotDisplayLockTextWhenNoButtonHeld)
{
  GlobalHoldButtonController globalHoldButtonController;
  ButtonStatus buttonStatus = {};
  CHECK_FALSE(globalHoldButtonController.shouldDisplayButtonLockText(buttonStatus));
}

TEST(globalHoldButtonController, testDisplayLockTextWhenNoButtonPress0)
{
  GlobalHoldButtonController globalHoldButtonController;
  ButtonStatus buttonStatus = {};
  buttonStatus.wasButtonPressedAndReleased[Constants::UP_BUTTON_INDEX] = true;
  CHECK_TRUE(globalHoldButtonController.shouldDisplayButtonLockText(buttonStatus));
}

TEST(globalHoldButtonController, testDisplayLockTextWhenNoButtonPress1)
{
  GlobalHoldButtonController globalHoldButtonController;
  ButtonStatus buttonStatus = {};
  buttonStatus.wasButtonPressedAndReleased[Constants::DOWN_BUTTON_INDEX] = true;
  CHECK_TRUE(globalHoldButtonController.shouldDisplayButtonLockText(buttonStatus));
}

TEST(globalHoldButtonController, testDisplayLockTextWhenNoButtonPress2)
{
  GlobalHoldButtonController globalHoldButtonController;
  ButtonStatus buttonStatus = {};
  buttonStatus.wasButtonPressedAndReleased[Constants::BACK_BUTTON_INDEX] = true;
  CHECK_TRUE(globalHoldButtonController.shouldDisplayButtonLockText(buttonStatus));
}

TEST(globalHoldButtonController, testDisplayLockTextWhenButtonHeld0)
{
  GlobalHoldButtonController globalHoldButtonController;
  ButtonStatus buttonStatus = {};
  buttonStatus.isButtonHeld[Constants::UP_BUTTON_INDEX] = true;
  CHECK_TRUE(globalHoldButtonController.shouldDisplayButtonLockText(buttonStatus));
}

TEST(globalHoldButtonController, testDisplayLockTextWhenButtonHeld1)
{
  GlobalHoldButtonController globalHoldButtonController;
  ButtonStatus buttonStatus = {};
  buttonStatus.isButtonHeld[Constants::MINUS_BUTTON_INDEX] = true;
  CHECK_TRUE(globalHoldButtonController.shouldDisplayButtonLockText(buttonStatus));
}

TEST(globalHoldButtonController, testDisplayLockTextWhenButtonHeld2)
{
  GlobalHoldButtonController globalHoldButtonController;
  ButtonStatus buttonStatus = {};
  buttonStatus.isButtonHeld[Constants::BACK_BUTTON_INDEX] = true;
  CHECK_TRUE(globalHoldButtonController.shouldDisplayButtonLockText(buttonStatus));
}

TEST(globalHoldButtonController, testDisplayLockTextWhenBackAndMinusHeld)
{
  GlobalHoldButtonController globalHoldButtonController;
  ButtonStatus buttonStatus = {};
  buttonStatus.isButtonHeld[Constants::BACK_BUTTON_INDEX] = true;
  buttonStatus.isButtonHeld[Constants::MINUS_BUTTON_INDEX] = true;
  CHECK_TRUE(globalHoldButtonController.shouldDisplayButtonLockText(buttonStatus));
}

TEST(globalHoldButtonController, testDisplayLockTextWhenBackAndPlusHeld)
{
  GlobalHoldButtonController globalHoldButtonController;
  ButtonStatus buttonStatus = {};
  buttonStatus.isButtonHeld[Constants::BACK_BUTTON_INDEX] = true;
  buttonStatus.isButtonHeld[Constants::PLUS_BUTTON_INDEX] = true;
  CHECK_TRUE(globalHoldButtonController.shouldDisplayButtonLockText(buttonStatus));
}

TEST(globalHoldButtonController, testDontDisplayLockTextWhenMinusAndPlusStartedHeld)
{
  GlobalHoldButtonController globalHoldButtonController;
  ButtonStatus buttonStatus = {};
  buttonStatus.isButtonStartedBeingHeld[Constants::MINUS_BUTTON_INDEX] = true;
  buttonStatus.isButtonStartedBeingHeld[Constants::PLUS_BUTTON_INDEX] = true;
  CHECK_FALSE(globalHoldButtonController.shouldDisplayButtonLockText(buttonStatus));
}

TEST(globalHoldButtonController, testBottonLockNotAvailableInBluetoothSetupScreen)
{
  SettingsMock settings;
  ButtonDriverMock buttonDriver;
  GlobalHoldButtonController globalHoldButtonController;
  UIState uiState = { Screen::bluetoothSetupScreen };
  ButtonStatus buttonStatus = {};

  mock().expectNCalls(0, "toggleButtonLock");
  mock().ignoreOtherCalls();

  buttonStatus.isButtonHeld[Constants::PLUS_BUTTON_INDEX] = true;
  buttonStatus.isButtonHeld[Constants::MINUS_BUTTON_INDEX] = true;
  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_COMBO_HOLD_TIME_MS*2;
  buttonStatus.buttonHeldDurationMs[Constants::MINUS_BUTTON_INDEX] = Constants::BUTTON_COMBO_HOLD_TIME_MS*2;

  globalHoldButtonController.control(uiState, buttonStatus, buttonDriver, settings);
  CHECK_EQUAL(Screen::bluetoothSetupScreen, uiState.currentScreen)

}
