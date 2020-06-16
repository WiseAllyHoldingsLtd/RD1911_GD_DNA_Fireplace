#include "UIController.h"
#include "Constants.h"
//#include "ButtonSystemTestDriver.h"
#include "ButtonStatus.h"
#include "SettingsInterface.h"
#include "GlobalHoldButtonController.h"
#include "MainScreenController.h"
#include "VerifyScreenController.h"
#include "ButtonTestScreenController.h"
#include "FactoryResetScreenController.h"
#include "BluetoothSetupScreenController.h"
#include "FirmwareUpgradeScreenController.h"

struct StateMapping
{
  const Screen::Enum screenState;
  ScreenControllerInterface &controller;
};

//factory reset screen either goes to factory confirm screen or the connected-branching screen.
FactoryResetScreenController factoryResetScreenController;

static GlobalHoldButtonController globalHoldButtonController;
static MainScreenController mainScreen;
static VerifyScreenController swVersionScreen(Screen::mainScreen, Constants::SW_VERSION_SCREEN_VISIBLE_TIME_S, true);
static ButtonTestScreenController buttonTestScreen;
static BluetoothSetupScreenController bluetoothScreen;
static FirmwareUpgradeScreenController fwUpgradeScreen;
static VerifyScreenController buttonLockedScreen(Screen::mainScreen, Constants::BUTTON_LOCK_SCREEN_VISIBLE_TIME_S, false);
static VerifyScreenController buttonUnlockedScreen(Screen::mainScreen, Constants::BUTTON_LOCK_SCREEN_VISIBLE_TIME_S, false);
static VerifyScreenController factoryResetConfirmScreen(Screen::mainScreen, Constants::CONFIRM_SCREEN_VISIBLE_TIME_S, true);

/* This table maps from screen/state to screenController: */
static const StateMapping stateMapping[] =
{
  { Screen::mainScreen, mainScreen },
  { Screen::buttonTestScreen, buttonTestScreen},
  { Screen::bluetoothSetupScreen, bluetoothScreen},
  { Screen::swVersionScreen, swVersionScreen},
  { Screen::buttonLockedScreen, buttonLockedScreen},
  { Screen::buttonUnlockedScreen, buttonUnlockedScreen},
  { Screen::confirmFactoryResetScreen, factoryResetConfirmScreen },
  { Screen::factoryResetScreen, factoryResetScreenController},
  { Screen::firmwareUpgradeScreen, fwUpgradeScreen }
};

UIController::UIController(TimerDriverInterface &timerDriver, ButtonDriverInterface &buttonDriver, BTDriverInterface &btDriver, const WifiDriverInterface &wifiDriver,
                            const AzureDriverInterface & azureDriver, const FirmwareUpgradeTask &fwTask, Screen::Enum startupScreen) :
                           m_timerDriver(timerDriver),
                           m_buttonDriver(buttonDriver),
                           m_btDriver(btDriver),
                           m_wifiDriver(wifiDriver),
                           m_azureDriver(azureDriver),
                           m_fwUpgrade(fwTask),
                           m_prevScreen(Screen::undefinedScreen)
{
  m_uiState.currentScreen = startupScreen;
  m_uiState.currentScreenEnterTimestampMs = timerDriver.getTimeSinceBootMS();
  m_uiState.timestampLastActiveMs = m_uiState.currentScreenEnterTimestampMs;
  m_uiState.isBluetoothActive = m_btDriver.isServiceRunning();
  m_uiState.isBluetoothConnected = m_btDriver.isConnected();
  m_uiState.isWifiConnected = m_wifiDriver.isConnectedToWlan();
  m_uiState.isAzureConnected = m_azureDriver.isConnected();
  m_uiState.isScreenOff = false;
}

bool UIController::screenShouldIgnoreButtonLock(const Screen::Enum currentScreen) const
{
  // Screens that should never be locked if needed:
  return (currentScreen == static_cast<int>(Screen::buttonLockedScreen))
      || (currentScreen == static_cast<int>(Screen::buttonUnlockedScreen))
      || (currentScreen == static_cast<int>(Screen::factoryResetScreen));
}

bool UIController::control(SettingsInterface &settings, QueueInterface<ConnectionStatusChangeRequest::Enum> &btStatusQueue, bool isScreenOff )
{
  ButtonStatus buttonStatus = m_buttonDriver.getButtonStatus();

  bool isButtonResetRequested = false;

  m_uiState.timeSinceBootMs = m_timerDriver.getTimeSinceBootMS();
  m_uiState.isBluetoothActive = m_btDriver.isServiceRunning();
  m_uiState.isBluetoothConnected = m_btDriver.isConnected();
  m_uiState.isWifiConnected = m_wifiDriver.isConnectedToWlan();
  m_uiState.isAzureConnected = m_azureDriver.isConnected();
  m_uiState.isScreenOff = isScreenOff;


  if (m_fwUpgrade.getCurrentState() != FirmwareUpgradeState::inProgress)
  {
    isButtonResetRequested = globalHoldButtonController.control(m_uiState, buttonStatus, m_buttonDriver, settings);

    if ((!settings.isButtonLockOn()) || (screenShouldIgnoreButtonLock(m_uiState.currentScreen)))
    {
      /* Find the current screen controller and let it handle buttons */
      bool isScreenControllerFound = false;
      for (uint16_t i = 0u; i < sizeof(stateMapping) / sizeof(stateMapping[0]); i++)
      {
        if (stateMapping[i].screenState == m_uiState.currentScreen)
        {
          isButtonResetRequested |= stateMapping[i].controller.control(m_uiState.timeSinceBootMs, m_uiState, buttonStatus, settings);
          isScreenControllerFound = true;
          break;
        }
      }

      /* Error handling: Reboot if current screen is unknown/unhandled: */
      if (!isScreenControllerFound)
      {
        while (true)
        {
          //Eternal loop to provoke a reset because of the watch dog.
        }
      }
    }
    else if ((settings.isButtonLockOn()) && (globalHoldButtonController.shouldDisplayButtonLockText(buttonStatus)))
    {
      m_uiState.currentScreen = Screen::buttonLockedScreen;
    }
    else { /* MISRA else */ }
  }
  else
  {
    // Firmware upgrade in progress
    m_uiState.currentScreen = Screen::firmwareUpgradeScreen;
  }

  /* If the current screen controller has navigated to another screen, call its enter method so that the view is rendered for the new screen */
  if (m_prevScreen != m_uiState.currentScreen)
  {
    reportEnteringOrLeavingBluetoothScreen(m_uiState.currentScreen, m_prevScreen, btStatusQueue);

    m_uiState.currentScreenEnterTimestampMs = m_uiState.timeSinceBootMs;
    m_prevScreen = m_uiState.currentScreen;

    for (uint16_t i = 0u; i < sizeof(stateMapping) / sizeof(stateMapping[0]); i++)
    {
      if (stateMapping[i].screenState == m_uiState.currentScreen)
      {
        stateMapping[i].controller.enter(m_uiState, settings);
        break;
      }
    }
  }

  if ((buttonStatus.isAnyButtonPressedAndReleased()) || isButtonResetRequested)
  {
    m_buttonDriver.resetButtonStatus();
  }

  return buttonStatus.isButtonActivity()
      || (m_uiState.currentScreen == Screen::bluetoothSetupScreen)  // we want this screen to stay fully on
      || (m_uiState.currentScreen == Screen::firmwareUpgradeScreen)  // we want this screen to stay fully on
      || (m_uiState.currentScreen == Screen::factoryResetScreen);  // we want this screen to stay fully on
}

UIState UIController::getUIState()
{
  return m_uiState;
}

void UIController::reportEnteringOrLeavingBluetoothScreen(Screen::Enum newScreen, Screen::Enum previousScreen, QueueInterface<ConnectionStatusChangeRequest::Enum> &queue) const
{
  if (newScreen != previousScreen)
  {
    if (newScreen == Screen::bluetoothSetupScreen)
    {
      // Entering bluetoothScreen - enable bluetooth
      queue.push(ConnectionStatusChangeRequest::bluetoothOn, 1U);
    }
    else if(previousScreen == Screen::bluetoothSetupScreen)
    {
      // Leaving bluetoothScreen - disable bluetooth
      queue.push(ConnectionStatusChangeRequest::bluetoothOff, 1U);
    }
    else
    {
      // no action
    }
  }
}

