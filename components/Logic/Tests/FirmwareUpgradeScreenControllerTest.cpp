#include "CppUTest\TestHarness.h"
#include "CppUTestExt\MockSupport.h"

#include "FirmwareUpgradeScreenController.h"
#include "SettingsMock.h"
#include "Constants.h"
#include "ButtonStatus.h"


TEST_GROUP(FirmwareUpgradeScreenController)
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


TEST(FirmwareUpgradeScreenController, testReturnsTrueWhenNothingIsDone)
{
  SettingsMock settings;
  FirmwareUpgradeScreenController controller;
  UIState uiState = { Screen::firmwareUpgradeScreen };
  ButtonStatus buttonStatus = {};

  controller.enter(uiState, settings);
  bool isButtonResetRequested = controller.control(0U, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested);
}

TEST(FirmwareUpgradeScreenController, testButtonsPressedAreIgnored)
{
  SettingsMock settings;
  FirmwareUpgradeScreenController controller;
  UIState uiState = { Screen::firmwareUpgradeScreen };
  ButtonStatus buttonStatus = {};
  controller.enter(uiState, settings);

  buttonStatus.wasButtonPressedAndReleased[Constants::BACK_BUTTON_INDEX] = true;
  bool isButtonResetRequested = controller.control(0U, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested);
  CHECK_EQUAL(Screen::firmwareUpgradeScreen, uiState.currentScreen);

  buttonStatus.wasButtonPressedAndReleased[Constants::BACK_BUTTON_INDEX] = false;
  buttonStatus.wasButtonPressedAndReleased[Constants::PLUS_BUTTON_INDEX] = true;
  isButtonResetRequested = controller.control(0U, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested);
  CHECK_EQUAL(Screen::firmwareUpgradeScreen, uiState.currentScreen);

  buttonStatus.wasButtonPressedAndReleased[Constants::PLUS_BUTTON_INDEX] = false;
  buttonStatus.wasButtonPressedAndReleased[Constants::MINUS_BUTTON_INDEX] = true;
  isButtonResetRequested = controller.control(0U, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested);
  CHECK_EQUAL(Screen::firmwareUpgradeScreen, uiState.currentScreen);
}
