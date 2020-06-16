#pragma once

#include "ScreenControllerBase.h"
#include "Constants.h"


class ButtonTestScreenController : public ScreenControllerBase
{
public:
  /**
  * The constructor must be placed here in the header file in stead of the .cpp file, or else the IAR stack analysis will fail.
  */
  ButtonTestScreenController()
    : ScreenControllerBase(Screen::mainScreen, Screen::swVersionScreen, Constants::NEXT_SCREEN_DELAY_NEVER),
    m_nextScreen(Screen::swVersionScreen)
  {
  }

  virtual void enter(UIState &uiState, SettingsInterface &settings);

  bool control(uint64_t timeSinceBootMs, UIState &uiState, const ButtonStatus &buttonStatus, SettingsInterface &settings);


private:
  Screen::Enum m_nextScreen; /** Screen to go to after delay. */
};
