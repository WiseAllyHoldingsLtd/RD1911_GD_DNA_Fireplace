#include "CppUTest\TestHarness.h"
#include "CppUTestExt\MockSupport.h"

#include "FactoryResetScreenController.h"
#include "SettingsMock.h"
#include "Constants.h"

static SettingsMock settings;
FactoryResetScreenController *factoryResetScreenController;
static UIState uiState = { Screen::mainScreen };
static ButtonStatus buttonStatus = {};

TEST_GROUP(factoryResetScreenController)
{
  TEST_SETUP()
  {
    factoryResetScreenController = new FactoryResetScreenController();
    uiState = { Screen::factoryResetScreen };
    buttonStatus = {};
  }

  TEST_TEARDOWN()
  {
    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();

    delete factoryResetScreenController;
  }
};


TEST(factoryResetScreenController, testNoButtonInputRedirectsToMainScreen)
{
  mock().expectOneCall("getForgetMeState")
      .andReturnValue(static_cast<uint32_t>(ForgetMeState::noResetRequested));
  factoryResetScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::mainScreen, uiState.currentScreen);
}


TEST(factoryResetScreenController, testWrongButtonComboInputRedirectsToMainScreenAfterTimeout)
{
  mock().expectOneCall("getForgetMeState")
      .andReturnValue(static_cast<uint32_t>(ForgetMeState::noResetRequested));
  buttonStatus.isButtonHeld[Constants::DOWN_BUTTON_INDEX] = true;
  buttonStatus.isButtonHeld[Constants::UP_BUTTON_INDEX] = true;
  buttonStatus.isButtonStartedBeingHeld[Constants::DOWN_BUTTON_INDEX] = true;
  buttonStatus.isButtonStartedBeingHeld[Constants::UP_BUTTON_INDEX] = true;
  buttonStatus.buttonHeldDurationMs[Constants::DOWN_BUTTON_INDEX] = 3001u;
  buttonStatus.buttonHeldDurationMs[Constants::UP_BUTTON_INDEX] = 3000u;
  factoryResetScreenController->control(3500U, uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::mainScreen, uiState.currentScreen);
}

TEST(factoryResetScreenController, testCorrectButtonComboInputSetsForgetMeStateToRequested)
{
  mock().expectOneCall("getForgetMeState")
      .andReturnValue(static_cast<uint32_t>(ForgetMeState::noResetRequested));
  mock().expectOneCall("setForgetMeState")
      .withUnsignedIntParameter("forgetMeState", static_cast<uint32_t>(ForgetMeState::resetRequested));
  buttonStatus.isButtonHeld[Constants::DOWN_BUTTON_INDEX] = true;
  buttonStatus.isButtonHeld[Constants::BACK_BUTTON_INDEX] = true;
  buttonStatus.isButtonStartedBeingHeld[Constants::DOWN_BUTTON_INDEX] = true;
  buttonStatus.isButtonStartedBeingHeld[Constants::BACK_BUTTON_INDEX] = true;
  buttonStatus.buttonHeldDurationMs[Constants::DOWN_BUTTON_INDEX] = 3001u;
  buttonStatus.buttonHeldDurationMs[Constants::BACK_BUTTON_INDEX] = 3001u;
  factoryResetScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::factoryResetScreen, uiState.currentScreen);
}

TEST(factoryResetScreenController, testFactoryResetPerformedRedirectsToFactoryResetConfirmScreen)
{
  mock().expectOneCall("getForgetMeState")
      .andReturnValue(static_cast<uint32_t>(ForgetMeState::noResetRequested));
  mock().expectOneCall("setForgetMeState")
      .withUnsignedIntParameter("forgetMeState", static_cast<uint32_t>(ForgetMeState::resetRequested));
  buttonStatus.isButtonHeld[Constants::DOWN_BUTTON_INDEX] = true;
  buttonStatus.isButtonHeld[Constants::BACK_BUTTON_INDEX] = true;
  buttonStatus.isButtonStartedBeingHeld[Constants::DOWN_BUTTON_INDEX] = true;
  buttonStatus.isButtonStartedBeingHeld[Constants::BACK_BUTTON_INDEX] = true;
  buttonStatus.buttonHeldDurationMs[Constants::DOWN_BUTTON_INDEX] = 3001u;
  buttonStatus.buttonHeldDurationMs[Constants::BACK_BUTTON_INDEX] = 3001u;
  factoryResetScreenController->control(0U, uiState, buttonStatus, settings);

  buttonStatus.isButtonHeld[Constants::DOWN_BUTTON_INDEX] = false;
  buttonStatus.isButtonHeld[Constants::BACK_BUTTON_INDEX] = false;
  buttonStatus.isButtonStartedBeingHeld[Constants::DOWN_BUTTON_INDEX] = false;
  buttonStatus.isButtonStartedBeingHeld[Constants::BACK_BUTTON_INDEX] = false;
  buttonStatus.buttonHeldDurationMs[Constants::DOWN_BUTTON_INDEX] = 0u;
  buttonStatus.buttonHeldDurationMs[Constants::BACK_BUTTON_INDEX] = 0u;

  uiState.currentScreenEnterTimestampMs = 0u;
  uiState.timeSinceBootMs = 0u;
  uiState.timestampLastActiveMs = 0u;

  mock().expectOneCall("getForgetMeState")
      .andReturnValue(static_cast<unsigned int>(ForgetMeState::noResetRequested));
  factoryResetScreenController->control(1U, uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::confirmFactoryResetScreen, uiState.currentScreen);
}

TEST(factoryResetScreenController, testBackButtonCancelsForgetMe)
{
  mock().expectOneCall("getForgetMeState")
      .andReturnValue(static_cast<uint32_t>(ForgetMeState::resetRequested));
  mock().expectOneCall("setForgetMeState")
      .withUnsignedIntParameter("forgetMeState", static_cast<uint32_t>(ForgetMeState::noResetRequested));
  buttonStatus.isButtonHeld[Constants::BACK_BUTTON_INDEX] = true;
  buttonStatus.isButtonStartedBeingHeld[Constants::BACK_BUTTON_INDEX] = true;
  buttonStatus.buttonHeldDurationMs[Constants::BACK_BUTTON_INDEX] = 2500u;
  factoryResetScreenController->control(0U, uiState, buttonStatus, settings);
  CHECK_EQUAL(Screen::mainScreen, uiState.currentScreen);
}
