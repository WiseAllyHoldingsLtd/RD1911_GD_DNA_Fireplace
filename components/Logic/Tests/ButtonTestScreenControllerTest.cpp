#include "CppUTest\TestHarness.h"
#include "CppUTestExt\MockSupport.h"

#include "ButtonTestScreenController.h"
#include "SettingsMock.h"
#include "Constants.h"

TEST_GROUP(ButtonTestScreenController)
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


TEST(ButtonTestScreenController, buttonTestScreenIsShownAndDoesNotChangeWhenButtonsAreHeld)
{
  SettingsMock settings;
  ButtonTestScreenController controller;
  UIState uiState = { Screen::buttonTestScreen};
  ButtonStatus buttonStatus = {};
  buttonStatus.isButtonHeld[Constants::PLUS_BUTTON_INDEX] = true;
  buttonStatus.isButtonHeld[Constants::BACK_BUTTON_INDEX] = true;
  buttonStatus.isButtonHeld[Constants::MINUS_BUTTON_INDEX] = true;
  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_TEST_HOLD_TIME_MS*2;
  buttonStatus.buttonHeldDurationMs[Constants::BACK_BUTTON_INDEX] = Constants::BUTTON_TEST_HOLD_TIME_MS*2;
  buttonStatus.buttonHeldDurationMs[Constants::MINUS_BUTTON_INDEX] = Constants::BUTTON_TEST_HOLD_TIME_MS*2;
  controller.control(123, uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::buttonTestScreen, uiState.currentScreen);
}

TEST(ButtonTestScreenController, buttonTestScreenIsShownAndNotChangedWhenSomeButtonsStillHeld)
{
  SettingsMock settings;
  ButtonTestScreenController controller;
  UIState uiState = { Screen::buttonTestScreen };
  ButtonStatus buttonStatus = {};
  buttonStatus.isButtonHeld[Constants::PLUS_BUTTON_INDEX] = true;
  buttonStatus.isButtonHeld[Constants::BACK_BUTTON_INDEX] = true;
  buttonStatus.isButtonHeld[Constants::MINUS_BUTTON_INDEX] = true;
  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_TEST_HOLD_TIME_MS * 2;
  buttonStatus.buttonHeldDurationMs[Constants::BACK_BUTTON_INDEX] = Constants::BUTTON_TEST_HOLD_TIME_MS * 2;
  buttonStatus.buttonHeldDurationMs[Constants::MINUS_BUTTON_INDEX] = Constants::BUTTON_TEST_HOLD_TIME_MS * 2;
  controller.control(123, uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::buttonTestScreen, uiState.currentScreen);

  buttonStatus.isButtonHeld[Constants::PLUS_BUTTON_INDEX] = true;
  buttonStatus.isButtonHeld[Constants::BACK_BUTTON_INDEX] = false;
  buttonStatus.isButtonHeld[Constants::MINUS_BUTTON_INDEX] = false;
  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_TEST_HOLD_TIME_MS * 3;
  controller.control(124, uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::buttonTestScreen, uiState.currentScreen);
}

TEST(ButtonTestScreenController, buttonTestScreenIsShownAndChangedWhenAllButtonsAreReleased)
{
  SettingsMock settings;
  ButtonTestScreenController controller;
  UIState uiState = { Screen::buttonTestScreen };
  ButtonStatus buttonStatus = {};
  buttonStatus.isButtonHeld[Constants::PLUS_BUTTON_INDEX] = true;
  buttonStatus.isButtonHeld[Constants::BACK_BUTTON_INDEX] = true;
  buttonStatus.isButtonHeld[Constants::MINUS_BUTTON_INDEX] = true;
  buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX] = Constants::BUTTON_TEST_HOLD_TIME_MS * 2;
  buttonStatus.buttonHeldDurationMs[Constants::BACK_BUTTON_INDEX] = Constants::BUTTON_TEST_HOLD_TIME_MS * 2;
  buttonStatus.buttonHeldDurationMs[Constants::MINUS_BUTTON_INDEX] = Constants::BUTTON_TEST_HOLD_TIME_MS * 2;
  controller.control(124, uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::buttonTestScreen, uiState.currentScreen);

  buttonStatus.isButtonHeld[Constants::PLUS_BUTTON_INDEX] = false;
  buttonStatus.isButtonHeld[Constants::BACK_BUTTON_INDEX] = false;
  buttonStatus.isButtonHeld[Constants::MINUS_BUTTON_INDEX] = false;
  controller.control(1234, uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::swVersionScreen, uiState.currentScreen);
}
