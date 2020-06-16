#pragma once

#include "UIState.h"
#include "ButtonStatus.h"
#include "ButtonDriverInterface.h"
#include "Settings.h"


class GlobalHoldButtonController
{
public:
  GlobalHoldButtonController();
  virtual ~GlobalHoldButtonController() {}

  /**
  * Handles special hold button actions like toggling button lock or entering button test.
  * Returns true if requesting button reset.
  * Out parameter uiState is changed if the UI state changes (e.g. to indicate navigation to another screen).
  */
  virtual bool control(UIState &uiState, 
                       const ButtonStatus &buttonStatus, 
                       ButtonDriverInterface &buttonDriver, 
                       SettingsInterface &settings);

  bool shouldDisplayButtonLockText(const ButtonStatus &buttonStatus);
  
private:
  Screen::Enum m_lockScreen; /** Screen to go to when button lock is enabled. */
  Screen::Enum m_unlockScreen; /** Screen to go to when button lock is deactivated. */
  Screen::Enum m_buttonTestScreen; /** Screen to go to when button test is activated. */
  virtual bool controlButtonLock(UIState &uiState, const ButtonStatus &buttonStatus, SettingsInterface &settings);
  virtual bool controlButtonTest(UIState &uiState, const ButtonStatus &buttonStatus);
};
