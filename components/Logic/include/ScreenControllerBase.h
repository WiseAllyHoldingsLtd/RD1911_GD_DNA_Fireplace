#pragma once

#include "ScreenControllerInterface.h"
#include "Constants.h"
#include <stdint.h>


class ScreenControllerBase : public ScreenControllerInterface
{
public:
  /**
  * The constructor must be placed here in the header file in stead of the .cpp file, or else the IAR stack analysis will fail.
  */
  ScreenControllerBase(Screen::Enum backScreen, Screen::Enum timeoutScreen, uint16_t timeoutTimeS) : m_timeoutScreen(timeoutScreen),
                                                                                                      m_buttonHoldCounter(0), 
                                                                                                      m_timeoutTimeS(timeoutTimeS),
                                                                                                      m_backScreen(backScreen)
  {
  };
  virtual ~ScreenControllerBase(){}


protected:
  /**
  *   Changes the screen to the timeout screen if the inactivity-time exceeds the m_timeoutTimeS variable defined in this class.
  */
  void controlTimeout(uint64_t timeSinceBootMs, UIState &uiState, const ButtonStatus &buttonStatus);

  /**
  * A buttonHoldEvent occurs if the buttonIndex represents the only button currently being held.
  *
  * Uses Constants::BUTTON_HOLD_FIRE_RATE_MS to determine how often events are triggered.
  * First event is triggered after Constants::BUTTON_HELD_THRESHOLD_MS has passed.
  * Fire rate can be overruled with the default fireRate parameter
  *
  * Returns true if a new button hold event is fired.
  * Returns true e.g. after button is held 500ms, 800ms, 1100ms, 1400ms etc.
  */
  bool buttonHoldEvent(uint8_t buttonIndex, const ButtonStatus &buttonStatus, uint32_t fireRate=Constants::BUTTON_HOLD_FIRE_RATE_MS);

private:
  
  Screen::Enum m_timeoutScreen;
  uint8_t m_buttonHoldCounter;

  uint16_t m_timeoutTimeS;

protected:
  Screen::Enum m_backScreen;
};
