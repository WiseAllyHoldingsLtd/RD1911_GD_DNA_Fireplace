#include "ButtonTestScreenController.h"
#include "Constants.h"
#include "Settings.h"
//#include "MenuChoices.h"


bool ButtonTestScreenController::control(uint64_t timeSinceBootMs, UIState &uiState, const ButtonStatus &buttonStatus, SettingsInterface &settings)
{
  bool isButtonResetRequested = false; 
  /* To make this classe more consistent with the other, we'll call controlTimeout even if it does nothing: */
  ScreenControllerBase::controlTimeout(timeSinceBootMs, uiState, buttonStatus);
  /* If no buttons are held or pressed it will move on to the next screen immediately. */
  if (!buttonStatus.isButtonActivity())
  {
    uiState.currentScreen = m_nextScreen;
    isButtonResetRequested = true;
  }
  /* These are unused on purpose, so use these statements without side effects to please the compiler */
  static_cast<void>(settings);

  return isButtonResetRequested;
}

void ButtonTestScreenController::enter(UIState &uiState, SettingsInterface &settings)
{
  /* These are unused on purpose, so use these statements without side effects to please the compiler */
  static_cast<void>(settings);
  static_cast<void>(uiState);
}
