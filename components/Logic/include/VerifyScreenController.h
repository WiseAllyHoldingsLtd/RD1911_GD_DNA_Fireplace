#pragma once

#include "ScreenControllerBase.h"


class VerifyScreenController : public ScreenControllerBase
{
public:
  /**
   * The constructor must be placed here in the header file in stead of the .cpp file, or else the IAR stack analysis will fail.
   */
  VerifyScreenController(Screen::Enum nextScreen, uint16_t nextScreenDelayS, bool consumeButtonPresses)
    : ScreenControllerBase(Screen::mainScreen, nextScreen, nextScreenDelayS), m_shouldConsumeButtonPresses(consumeButtonPresses)
  {
  }

  virtual void enter(UIState &uiState, SettingsInterface &settings);

  bool control(uint64_t timeSinceBootMs, UIState &uiState, const ButtonStatus &buttonStatus, SettingsInterface &settings);

private:
  bool m_shouldConsumeButtonPresses;

};
