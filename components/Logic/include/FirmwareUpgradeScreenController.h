#pragma once

#include "ScreenControllerBase.h"
#include "Constants.h"


class FirmwareUpgradeScreenController : public ScreenControllerBase
{
public:
  /**
  * The constructor must be placed here in the header file in stead of the .cpp file, or else the IAR stack analysis will fail.
  */
  FirmwareUpgradeScreenController(void)
    : ScreenControllerBase(Screen::mainScreen, Screen::mainScreen, Constants::NEXT_SCREEN_DELAY_NEVER)
  {
  }

  virtual void enter(UIState &uiState, SettingsInterface &settings);

  bool control(uint64_t timeSinceBootMs, UIState &uiState, const ButtonStatus &buttonStatus, SettingsInterface &settings);
};
