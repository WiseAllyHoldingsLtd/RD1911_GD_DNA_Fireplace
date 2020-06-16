#include "VerifyScreenController.h"
#include "Constants.h"
#include "Settings.h"
//#include "MenuChoices.h"


bool VerifyScreenController::control(uint64_t timeSinceBootMs, UIState &uiState, const ButtonStatus &buttonStatus, SettingsInterface &settings)
{
  /* Buttons are checked inside the parent's class, so we need to send a non-active buttonstatus. */
  const ButtonStatus noButtonsPressed = {0};

  ScreenControllerBase::controlTimeout(timeSinceBootMs, uiState, noButtonsPressed);

  /* These are unused on purpose, so use these statements without side effects to please the compiler */
  static_cast<void>(timeSinceBootMs);
  static_cast<void>(uiState);
  static_cast<void>(buttonStatus);
  static_cast<void>(settings);

  bool isButtonResetRequested = m_shouldConsumeButtonPresses; /* When true buttons are reset and buttons presses consumed */
  
  return isButtonResetRequested;
}

void VerifyScreenController::enter(UIState &uiState, SettingsInterface &settings)
{
  /* These are unused on purpose, so use these statements without side effects to please the compiler */
  static_cast<void>(uiState);
  static_cast<void>(settings);
}
