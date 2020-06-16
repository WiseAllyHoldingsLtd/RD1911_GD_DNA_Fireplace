#include "FactoryResetScreenController.h"
#include "Constants.h"
#include "Settings.h"
#include "UIState.h"
#include "ButtonStatus.h"
#include "EspCpp.hpp"

namespace
{
  const char LOG_TAG[] = "FactoryResetScrnCntrl";
}

bool FactoryResetScreenController::control(uint64_t timeSinceBootMs,
                                           UIState &uiState,
                                           const ButtonStatus &buttonStatus,
                                           SettingsInterface &settings)
{
  bool isButtonResetRequested = false;

  ForgetMeState::Enum currForgetMeState = settings.getForgetMeState();

  if ( currForgetMeState != ForgetMeState::noResetRequested || m_factoryResetRequested )
  {
    // stay in this screen until message
    // has been sent and confirmation received, or back button is pressed
    m_factoryResetRequested = true;

    // ConnectionController will perform the factory reset
    if ( currForgetMeState == ForgetMeState::noResetRequested )
    {
      // Factory reset has been performed
      uiState.currentScreen = Screen::confirmFactoryResetScreen;
      m_factoryResetRequested = false;
    }
    else if ( isBackButtonPressed(buttonStatus) )
    {
      settings.setForgetMeState(ForgetMeState::noResetRequested);
      uiState.currentScreen = m_nextScreen;
      isButtonResetRequested = true;
      m_factoryResetRequested = false;
      ESP_LOGI(LOG_TAG, "Back button pressed, cancelling forget me + factory reset");
    }
  } else
  {
    if ((buttonStatus.isButtonHeld[Constants::MINUS_BUTTON_INDEX]) &&
      (buttonStatus.isButtonHeld[Constants::BACK_BUTTON_INDEX]) &&
      (!(buttonStatus.isButtonHeld[Constants::PLUS_BUTTON_INDEX])))
    {
      ESP_LOGI(LOG_TAG, "Setting reset requested flag");
      settings.setForgetMeState(ForgetMeState::resetRequested);
      m_factoryResetRequested = true;
      isButtonResetRequested = true;

    }
    else
    {
      uiState.currentScreen = m_nextScreen;
    }
  }

  return isButtonResetRequested;
}

void FactoryResetScreenController::enter(UIState &uiState, SettingsInterface &settings)
{
  /* These are unused on purpose, so use these statements without side effects to please the compiler */
  static_cast<void>(uiState);
  static_cast<void>(settings);

  // NOTE that as startup screen, this is actually NOT called before the first call to control.
}

bool FactoryResetScreenController::isBackButtonPressed(const ButtonStatus &buttonStatus)
{
  return (buttonHoldEvent(Constants::BACK_BUTTON_INDEX, buttonStatus)) || (buttonStatus.wasButtonPressedAndReleased[Constants::BACK_BUTTON_INDEX]);
}
