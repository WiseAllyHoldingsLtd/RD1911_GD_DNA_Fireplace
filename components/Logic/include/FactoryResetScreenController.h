#pragma once

#include "ScreenControllerBase.h"
#include "Constants.h"

class FactoryResetScreenController : public ScreenControllerBase
{
public:
  /**
   * The constructor must be placed here in the header file in stead of the .cpp file, or else the IAR stack analysis will fail.
   */
  // FIXME: timeout to cloud connection branching screen?
  FactoryResetScreenController()
  : ScreenControllerBase(Screen::factoryResetScreen, Screen::mainScreen, Constants::NEXT_SCREEN_DELAY_NEVER),
    m_nextScreen(Screen::mainScreen),
    m_factoryResetRequested(false)
  {
  }

  void enter(UIState &uiState, SettingsInterface &settings);

  bool control(uint64_t timeSinceBootMs, UIState &uiState, const ButtonStatus &buttonStatus, SettingsInterface &settings);

private:
  bool isBackButtonPressed(const ButtonStatus &buttonStatus);

  Screen::Enum m_nextScreen; /** Screen to go to when menu button is pressed. */
  bool m_factoryResetRequested;
};
