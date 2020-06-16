#pragma once

#include "ScreenControllerBase.h"
#include "Constants.h"


class BluetoothSetupScreenController : public ScreenControllerBase
{
public:
  /**
  * The constructor must be placed here in the header file in stead of the .cpp file, or else the IAR stack analysis will fail.
  */
  BluetoothSetupScreenController(void)
    : ScreenControllerBase(Screen::mainScreen, Screen::mainScreen, Constants::BLUETOOTH_SCREEN_MAX_VISIBLE_TIME_S), m_nextScreen(Screen::mainScreen),
      m_hasBeenConnected(false), m_previousIsConnected(false), m_timeoutStartTime(0U), m_timeoutLimit(0U)
  {
  }

  virtual void enter(UIState &uiState, SettingsInterface &settings);

  bool control(uint64_t timeSinceBootMs, UIState &uiState, const ButtonStatus &buttonStatus, SettingsInterface &settings);


private:
  bool isBackButtonPressed(const ButtonStatus &buttonStatus);
  bool isConnectionTimeoutTrigged(uint64_t timeSinceBootMs, const UIState &uiState) const;
  bool isSetupCompleted(const UIState &uiState) const;

  Screen::Enum m_nextScreen; /** Screen to go to after delay or Bluetooth has been deactivated */
  bool m_hasBeenConnected;
  bool m_previousIsConnected;
  uint64_t m_timeoutStartTime;
  uint32_t m_timeoutLimit;
};
