#pragma once

#include "SettingsInterface.h"
#include "ButtonStatus.h"
#include "UIState.h"


class ScreenControllerInterface
{
public:

  virtual ~ScreenControllerInterface(){};
  /**
  * Controls one type of screen, returning true if requesting button reset (call resetButtonStatus() afterwards).
  * Out parameter uiState is changed if the UI state changes (e.g. to indicate navigation to another screen).
  */
  virtual bool control(uint64_t timeSinceBootMs, UIState &uiState, const ButtonStatus &buttonStatus, SettingsInterface &settings) = 0;
  
  /**
  * Called whenever the view changes to this screen.
  */
  virtual void enter(UIState &uiState, SettingsInterface &settings) = 0;
};

