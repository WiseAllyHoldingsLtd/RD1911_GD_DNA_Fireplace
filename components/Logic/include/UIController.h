#pragma once

#include <stdint.h>

#include "UIState.h"
#include "DateTime.h"
#include "SettingsInterface.h"
#include "ButtonDriverInterface.h"
#include "TimerDriverInterface.h"
#include "BTDriverInterface.h"
#include "WifiDriverInterface.h"
#include "AzureDriverInterface.h"
#include "FirmwareUpgradeTask.hpp"
#include "QueueInterface.hpp"
#include "ConnectionStatusChangeRequest.h"

/** Controls the ui, has a state mapping from screen enums to screen controller.
  */
class UIController
{
public:
  UIController(TimerDriverInterface &timerDriver,
    ButtonDriverInterface &buttonDriver,
    BTDriverInterface &btDriver,
    const WifiDriverInterface &wifiDriver,
    const AzureDriverInterface &azureDriver,
    const FirmwareUpgradeTask &fwTask,
    Screen::Enum startupScreen);

  /** 
  *     Calls the control function of the current screen controller and the enter function of the controller if
  *     the previous controller is different than the current one (new controller).
  *     returns true if there is a button activity.
  */
  bool control(SettingsInterface &settings, QueueInterface<ConnectionStatusChangeRequest::Enum> &btStatusQueue, bool isScreenOff);
  UIState getUIState();

private:
  /**
  *     returns whether the given screen should ignore button lock.
  */
  bool screenShouldIgnoreButtonLock(const Screen::Enum currentScreen) const;

  /**
   *  Checks whether we are entering or has left bluetoothSetupScreen, and if so sends a status change request to queue
   */
  void reportEnteringOrLeavingBluetoothScreen(Screen::Enum newScreen, Screen::Enum previousScreen, QueueInterface<ConnectionStatusChangeRequest::Enum> &queue) const;

  TimerDriverInterface &m_timerDriver;
  ButtonDriverInterface &m_buttonDriver;
  BTDriverInterface &m_btDriver;
  const WifiDriverInterface &m_wifiDriver;
  const AzureDriverInterface &m_azureDriver;
  const FirmwareUpgradeTask &m_fwUpgrade;
  UIState m_uiState;
  Screen::Enum m_prevScreen;
};
