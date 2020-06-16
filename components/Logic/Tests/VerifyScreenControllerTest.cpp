#include "CppUTest\TestHarness.h"
#include "CppUTestExt\MockSupport.h"

#include "VerifyScreenController.h"
#include "SettingsMock.h"
#include "Constants.h"


TEST_GROUP(VerifyScreenController)
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


TEST(VerifyScreenController, testReturnsTrue)
{
  SettingsMock settings;
  VerifyScreenController controller(Screen::swVersionScreen, 123U, true);
  UIState uiState = { Screen::mainScreen};
  ButtonStatus buttonStatus = {};

  bool isButtonResetRequested = controller.control(0U, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested);
}

TEST(VerifyScreenController, testReturnsFalseIfNotConsumingButtonPresses)
{
	SettingsMock settings;
	VerifyScreenController controller(Screen::swVersionScreen, 123U, false);
	UIState uiState = { Screen::mainScreen };
	ButtonStatus buttonStatus = {};

	bool isButtonResetRequested = controller.control(0U, uiState, buttonStatus, settings);
	CHECK_FALSE(isButtonResetRequested);
}

TEST(VerifyScreenController, gotoNextScreenIfEnoughTimeHasPassed)
{
  SettingsMock settings;
  VerifyScreenController controller(Screen::swVersionScreen, 123U, true);
  UIState uiState = { Screen::mainScreen };
  uiState.currentScreenEnterTimestampMs = 0U;
  ButtonStatus buttonStatus = {};

  bool isButtonResetRequested = controller.control(123U*1000, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested);
  CHECK_EQUAL(Screen::swVersionScreen, uiState.currentScreen);
}

TEST(VerifyScreenController, stayOnScreenUntilEnoughTimeHasPassed)
{
  SettingsMock settings;
  VerifyScreenController controller(Screen::swVersionScreen, 123U, true);
  UIState uiState = { Screen::mainScreen};
  uiState.currentScreenEnterTimestampMs = 0U;
  ButtonStatus buttonStatus = {};

  bool isButtonResetRequested = controller.control(122U*1000, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested);
  CHECK_EQUAL(Screen::mainScreen, uiState.currentScreen);
}

TEST(VerifyScreenController, changeScreenEvenIfButtonsArePressed)
{
  SettingsMock settings;
  VerifyScreenController controller(Screen::swVersionScreen, 123U, true);
  UIState uiState = { Screen::mainScreen};
  uiState.currentScreenEnterTimestampMs = 0U;
  ButtonStatus buttonStatus = {};
  buttonStatus.wasButtonPressedAndReleased[1] = true;

  bool isButtonResetRequested = controller.control(122U*1000, uiState, buttonStatus, settings);
  isButtonResetRequested = controller.control(123U*1000, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested);
  CHECK_EQUAL(Screen::swVersionScreen, uiState.currentScreen);
}

TEST(VerifyScreenController, changeScreenEvenIfButtonsAreHeld)
{
  SettingsMock settings;
  VerifyScreenController controller(Screen::swVersionScreen, 123U, true);
  UIState uiState = { Screen::mainScreen};
  uiState.currentScreenEnterTimestampMs = 0U;
  ButtonStatus buttonStatus = {};
  buttonStatus.isButtonHeld[1] = true;
  buttonStatus.buttonHeldDurationMs[1] = 5000U;

  bool isButtonResetRequested = controller.control(122U * 1000, uiState, buttonStatus, settings);
  isButtonResetRequested = controller.control(123U * 1000, uiState, buttonStatus, settings);
  CHECK_TRUE(isButtonResetRequested);
  CHECK_EQUAL(Screen::swVersionScreen, uiState.currentScreen);
}
