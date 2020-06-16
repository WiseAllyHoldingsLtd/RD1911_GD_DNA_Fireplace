#include "CppUTest\TestHarness.h"
#include "CppUTestExt\MockSupport.h"

#include "MainScreenController.h"
#include "SettingsMock.h"
#include "Constants.h"

static SettingsMock settings;
MainScreenController *mainScreenController;
static UIState uiState = { Screen::mainScreen };
static ButtonStatus buttonStatus = {};
//static EepromContents content;
//static Eeprom eeprom(content);

TEST_GROUP(mainScreenController)
{
  TEST_SETUP()
  {
    mainScreenController = new MainScreenController();
    uiState = { Screen::mainScreen };
    buttonStatus = {};
  }

  TEST_TEARDOWN()
  {
    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();

    delete mainScreenController;
  }
};


TEST(mainScreenController, testReturnsFalseWhenNoActionPerformed)
{
  bool isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_FALSE(isButtonResetRequested);
}

TEST(mainScreenController, testIncreaseSetPoint)
{
  buttonStatus.wasButtonPressedAndReleased[Constants::PLUS_BUTTON_INDEX] = true;
 
  mock().expectOneCall("increaseSetPoint");
  bool isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested);
}

TEST(mainScreenController, testDecreaseSetPoint)
{
  buttonStatus.wasButtonPressedAndReleased[Constants::MINUS_BUTTON_INDEX] = true;
 
  mock().expectOneCall("decreaseSetPoint");
  bool isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested);
}


TEST(mainScreenController, testIncreaseSetPointWhenButtonHeld)
{
  buttonStatus.isButtonHeld[Constants::PLUS_BUTTON_INDEX] = true;
  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_HELD_THRESHOLD_MS;
  mock().expectOneCall("increaseSetPoint");
  bool isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested);
}


TEST(mainScreenController, testDecreaseSetPointWhenButtonHeld)
{
  buttonStatus.isButtonHeld[Constants::MINUS_BUTTON_INDEX] = true;
  buttonStatus.buttonHeldDurationMs[Constants::MINUS_BUTTON_INDEX] = Constants::BUTTON_HELD_THRESHOLD_MS;
  mock().expectOneCall("decreaseSetPoint");
  bool isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested);
}


TEST(mainScreenController, testDontIncreaseSetPointWhenTwoButtonsHeld)
{
  buttonStatus.isButtonHeld[Constants::PLUS_BUTTON_INDEX] = true;
  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_HELD_THRESHOLD_MS;
  buttonStatus.isButtonHeld[Constants::MINUS_BUTTON_INDEX] = true;
  buttonStatus.buttonHeldDurationMs[Constants::MINUS_BUTTON_INDEX] = Constants::BUTTON_HELD_THRESHOLD_MS;
  mock().expectNCalls(0, "getMaxSetPoint").andReturnValue(12345);
  mock().expectNCalls(0, "increaseSetPoint").withLongIntParameter("maxSetPoint", 12345);
  bool isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_FALSE(isButtonResetRequested);
}


TEST(mainScreenController, testDontIncreaseSetPointWhenButtonsHeldWhileOtherButtonStartedBeingHeld)
{
  buttonStatus.isButtonHeld[Constants::PLUS_BUTTON_INDEX] = true;
  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_HELD_THRESHOLD_MS;
  buttonStatus.isButtonStartedBeingHeld[Constants::MINUS_BUTTON_INDEX] = true;
  buttonStatus.buttonHeldDurationMs[Constants::MINUS_BUTTON_INDEX] = 0U;
  mock().expectNCalls(0, "getMaxSetPoint").andReturnValue(12345);
  mock().expectNCalls(0, "increaseSetPoint").withLongIntParameter("maxSetPoint", 12345);
  bool isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_FALSE(isButtonResetRequested);
}


TEST(mainScreenController, testIncreaseSetPointOnceButtonHeldUnderTwoIntervals)
{
  buttonStatus.isButtonHeld[Constants::PLUS_BUTTON_INDEX] = true;
  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_HELD_THRESHOLD_MS;
 
  mock().expectOneCall("increaseSetPoint");
  bool isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested);

  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = 650U;
  isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_FALSE(isButtonResetRequested);
}


TEST(mainScreenController, testIncreaseSetPointTwiceButtonHeldTwoIntervals)
{
  buttonStatus.isButtonHeld[Constants::PLUS_BUTTON_INDEX] = true;
  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_HELD_THRESHOLD_MS;
 
  mock().expectOneCall("increaseSetPoint");
  bool isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested);

  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_HELD_THRESHOLD_MS + Constants::BUTTON_HOLD_FIRE_RATE_MS;
 
  mock().expectOneCall("increaseSetPoint");
  isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested);
}


TEST(mainScreenController, testIncreaseSetPointOnButtonHeldSixTimes)
{
  buttonStatus.isButtonHeld[Constants::PLUS_BUTTON_INDEX] = true;
  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_HELD_THRESHOLD_MS;
 
  mock().expectOneCall("increaseSetPoint");
  bool isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested);

  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_HOLD_FIRST_FIRE_MS + Constants::BUTTON_HOLD_FIRE_RATE_MS - 1U;
  isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_FALSE(isButtonResetRequested);

  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_HOLD_FIRST_FIRE_MS + Constants::BUTTON_HOLD_FIRE_RATE_MS;
 
  mock().expectOneCall("increaseSetPoint");
  isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested);
  
  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_HOLD_FIRST_FIRE_MS + Constants::BUTTON_HOLD_FIRE_RATE_MS * 2 - 1U;
  isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_FALSE(isButtonResetRequested);

  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_HOLD_FIRST_FIRE_MS + Constants::BUTTON_HOLD_FIRE_RATE_MS * 2;
 
  mock().expectOneCall("increaseSetPoint");
  isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested);
  
  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_HOLD_FIRST_FIRE_MS + Constants::BUTTON_HOLD_FIRE_RATE_MS * 3 - 1U;
  isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_FALSE(isButtonResetRequested);

  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_HOLD_FIRST_FIRE_MS + Constants::BUTTON_HOLD_FIRE_RATE_MS * 3;
 
  mock().expectOneCall("increaseSetPoint");
  isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested);

  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_HOLD_FIRST_FIRE_MS + Constants::BUTTON_HOLD_FIRE_RATE_MS * 3 + 199U;
  isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_FALSE(isButtonResetRequested);

  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_HOLD_FIRST_FIRE_MS + Constants::BUTTON_HOLD_FIRE_RATE_MS * 3 + 299U;
  isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_FALSE(isButtonResetRequested);

  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_HOLD_FIRST_FIRE_MS + Constants::BUTTON_HOLD_FIRE_RATE_MS * 4 - 1U;
  isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_FALSE(isButtonResetRequested);

  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_HOLD_FIRST_FIRE_MS + Constants::BUTTON_HOLD_FIRE_RATE_MS * 4;
 
  mock().expectOneCall("increaseSetPoint");
  isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested);

  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_HOLD_FIRST_FIRE_MS + Constants::BUTTON_HOLD_FIRE_RATE_MS * 5 - 1U;
  isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_FALSE(isButtonResetRequested);

  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_HOLD_FIRST_FIRE_MS + Constants::BUTTON_HOLD_FIRE_RATE_MS * 5;
 
  mock().expectOneCall("increaseSetPoint");
  isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested);
}


TEST(mainScreenController, testButtonHeldTimerResetsOkAfter1HoldEvent)
{
  buttonStatus.isButtonHeld[Constants::PLUS_BUTTON_INDEX] = true;
  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_HELD_THRESHOLD_MS;
 
  mock().expectOneCall("increaseSetPoint");
  bool isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested);

  buttonStatus.isButtonHeld[Constants::PLUS_BUTTON_INDEX] = false;
  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = 0U;
  isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_FALSE(isButtonResetRequested);

  buttonStatus.isButtonHeld[Constants::PLUS_BUTTON_INDEX] = true;
  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_HELD_THRESHOLD_MS;
 
  mock().expectOneCall("increaseSetPoint");
  isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested);
}


TEST(mainScreenController, testButtonHeldTimerResetsOkAfter2HoldEvents)
{
  buttonStatus.isButtonHeld[Constants::PLUS_BUTTON_INDEX] = true;
  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_HELD_THRESHOLD_MS;
 
  mock().expectOneCall("increaseSetPoint");
  bool isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested);
  
  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_HOLD_FIRST_FIRE_MS + Constants::BUTTON_HOLD_FIRE_RATE_MS - 1U;
  isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_FALSE(isButtonResetRequested);

  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_HOLD_FIRST_FIRE_MS + Constants::BUTTON_HOLD_FIRE_RATE_MS;
 
  mock().expectOneCall("increaseSetPoint");
  isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested);

  buttonStatus.isButtonHeld[Constants::PLUS_BUTTON_INDEX] = false;
  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = 0U;
  isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_FALSE(isButtonResetRequested);

  buttonStatus.isButtonHeld[Constants::PLUS_BUTTON_INDEX] = true;
  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_HELD_THRESHOLD_MS;
 
  mock().expectOneCall("increaseSetPoint");
  isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested);
}


TEST(mainScreenController, testTwoButtonsHeldThenOneButtonHeld)
{
  buttonStatus.isButtonHeld[Constants::MINUS_BUTTON_INDEX]  = true;
  buttonStatus.isButtonHeld[Constants::BACK_BUTTON_INDEX] = true;
  buttonStatus.buttonHeldDurationMs[Constants::MINUS_BUTTON_INDEX]  = Constants::BUTTON_HELD_THRESHOLD_MS;
  buttonStatus.buttonHeldDurationMs[Constants::BACK_BUTTON_INDEX] = Constants::BUTTON_HELD_THRESHOLD_MS;
  bool isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_FALSE(isButtonResetRequested);

  buttonStatus.buttonHeldDurationMs[Constants::MINUS_BUTTON_INDEX]  = Constants::BUTTON_HOLD_FIRST_FIRE_MS + Constants::BUTTON_HOLD_FIRE_RATE_MS;
  buttonStatus.buttonHeldDurationMs[Constants::BACK_BUTTON_INDEX] = Constants::BUTTON_HOLD_FIRST_FIRE_MS + Constants::BUTTON_HOLD_FIRE_RATE_MS;
  isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_FALSE(isButtonResetRequested);
  
  buttonStatus.isButtonHeld[Constants::BACK_BUTTON_INDEX] = false;
  buttonStatus.buttonHeldDurationMs[Constants::BACK_BUTTON_INDEX] = 0U;
  buttonStatus.buttonHeldDurationMs[Constants::MINUS_BUTTON_INDEX]  = Constants::BUTTON_HOLD_FIRST_FIRE_MS + Constants::BUTTON_HOLD_FIRE_RATE_MS + 10U;
 
  mock().expectOneCall("decreaseSetPoint");
  isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested);

  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_HOLD_FIRST_FIRE_MS + Constants::BUTTON_HOLD_FIRE_RATE_MS + 20U;
  isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_FALSE(isButtonResetRequested);
}


TEST(mainScreenController, testToggleOverride)
{
  buttonStatus.wasButtonPressedAndReleased[Constants::BACK_BUTTON_INDEX] = true;
  mock().expectOneCall("toggleOverride");
  bool isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested);
}


TEST(mainScreenController, testToggleOverrideWhenButtonHeld)
{
  buttonStatus.isButtonHeld[Constants::BACK_BUTTON_INDEX] = true;
  buttonStatus.buttonHeldDurationMs[Constants::BACK_BUTTON_INDEX] = Constants::BUTTON_HELD_THRESHOLD_MS;
  mock().expectOneCall("toggleOverride");
  bool isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested);
}

TEST(mainScreenController, testPlusAndBackRedirectsToBluetoothSetupScreen)
{
  buttonStatus.isButtonHeld[Constants::BACK_BUTTON_INDEX] = true;
  buttonStatus.isButtonHeld[Constants::PLUS_BUTTON_INDEX] = true;
  buttonStatus.buttonHeldDurationMs[Constants::BACK_BUTTON_INDEX] = Constants::BUTTON_COMBO_HOLD_TIME_MS - 1U;
  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_COMBO_HOLD_TIME_MS - 1U;
  bool isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_FALSE(isButtonResetRequested);

  buttonStatus.buttonHeldDurationMs[Constants::BACK_BUTTON_INDEX] = Constants::BUTTON_COMBO_HOLD_TIME_MS;
  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_COMBO_HOLD_TIME_MS;
  isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested);
  CHECK_EQUAL(Screen::bluetoothSetupScreen, uiState.currentScreen);
}

TEST(mainScreenController, testHolding3ButtonsShouldNotRedirect)
{
  buttonStatus.isButtonHeld[Constants::BACK_BUTTON_INDEX] = true;
  buttonStatus.isButtonHeld[Constants::PLUS_BUTTON_INDEX] = true;
  buttonStatus.isButtonHeld[Constants::MINUS_BUTTON_INDEX] = true;

  buttonStatus.buttonHeldDurationMs[Constants::BACK_BUTTON_INDEX] = Constants::BUTTON_COMBO_HOLD_TIME_MS;
  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_COMBO_HOLD_TIME_MS;
  buttonStatus.buttonHeldDurationMs[Constants::MINUS_BUTTON_INDEX] = Constants::BUTTON_COMBO_HOLD_TIME_MS;
  bool isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_FALSE(isButtonResetRequested);
  CHECK_EQUAL(Screen::mainScreen, uiState.currentScreen);
}

TEST(mainScreenController, testSetpointNotIncreasedIfScreenIsDimmedDown)
{
  uiState.isScreenOff = true;
  buttonStatus.wasButtonPressedAndReleased[Constants::PLUS_BUTTON_INDEX] = true;

  mock().expectNoCall("increaseSetPoint");
  bool isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested); // returns true in order to turn screen on
}

TEST(mainScreenController, testSetpointNotDecreasedIfScreenIsDimmedDown)
{
  uiState.isScreenOff = true;
  buttonStatus.wasButtonPressedAndReleased[Constants::MINUS_BUTTON_INDEX] = true;

  mock().expectNoCall("decreaseSetPoint");
  bool isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested);
}

TEST(mainScreenController, testToggleOverrideNotDoneIfScreenIsDimmedDown)
{
  uiState.isScreenOff = true;
  buttonStatus.wasButtonPressedAndReleased[Constants::BACK_BUTTON_INDEX] = true;

  mock().expectNoCall("toggleOverride");
  bool isButtonResetRequested = mainScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested);
}
