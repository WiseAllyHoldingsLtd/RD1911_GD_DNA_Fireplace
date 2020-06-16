#include "FirmwareUpgradeScreenController.h"
#include "Constants.h"
#include "Settings.h"
#include "ButtonStatus.h"


bool FirmwareUpgradeScreenController::control(uint64_t timeSinceBootMs, UIState &uiState, const ButtonStatus &buttonStatus, SettingsInterface &settings)
{
  /* Nothing should be done, all buttons should be ignored */

  /* To make this classe more consistent with the other, we'll call controlTimeout even if it does nothing: */
  ScreenControllerBase::controlTimeout(timeSinceBootMs, uiState, buttonStatus);

  /* Unused on purpose, so use these statements without side effects to please the compiler */
  static_cast<void>(settings);
  return true;
}

void FirmwareUpgradeScreenController::enter(UIState &uiState, SettingsInterface &settings)
{
  /* These are unused on purpose, so use these statements without side effects to please the compiler */
  static_cast<void>(settings);
  static_cast<void>(uiState);
}
