#pragma once

#include "ScreenControllerBase.h"
#include "SettingsInterface.h"
#include "ButtonStatus.h"
#include "Constants.h"
#include "UIState.h"


/** Controller for main screen, changes set point and local override.
  *
  */
class MainScreenController : public ScreenControllerBase
{
public:
  /**
   * The constructor must be placed here in the header file in stead of the .cpp file, or else the IAR stack analysis will fail.
   */
  MainScreenController() : ScreenControllerBase(Screen::mainScreen, Screen::mainScreen, Constants::NEXT_SCREEN_DELAY_NEVER)
  {
  }

  void enter(UIState &uiState, SettingsInterface &settings);

  bool control(uint64_t timeSinceBootMs, UIState &uiState, const ButtonStatus &buttonStatus, SettingsInterface &settings);

private:
  bool controlBluetoothSetupRedirect(UIState &uiState, const ButtonStatus &buttonStatus);
};
