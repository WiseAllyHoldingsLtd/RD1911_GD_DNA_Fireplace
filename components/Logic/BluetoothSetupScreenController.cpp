#include "BluetoothSetupScreenController.h"
#include "Constants.h"
#include "Settings.h"
#include "ButtonStatus.h"


bool BluetoothSetupScreenController::control(uint64_t timeSinceBootMs, UIState &uiState, const ButtonStatus &buttonStatus, SettingsInterface &settings)
{
  /* Buttons are checked inside the parent's class, so we need to send a non-active buttonstatus. */
  const ButtonStatus noButtonsPressed = {0};
  ScreenControllerBase::controlTimeout(timeSinceBootMs, uiState, noButtonsPressed);

  bool isButtonResetRequested = false;
  m_hasBeenConnected = (m_hasBeenConnected || uiState.isBluetoothConnected);

  /* If we have been connected at least once we should use a shorter timeout */
  if (m_hasBeenConnected && (!uiState.isBluetoothConnected))
  {
    m_timeoutLimit = Constants::BLUETOOTH_SCREEN_SHORT_CONN_WAIT_TIME_MS;
  }

  /* If we are disconnecting now we should set a new time to check timeout against */
  if (m_previousIsConnected && (!uiState.isBluetoothConnected))
  {
    m_timeoutStartTime = timeSinceBootMs;
  }

  /* Consume plus and minus as they do nothing */
  if (buttonStatus.wasButtonPressedAndReleased[Constants::PLUS_BUTTON_INDEX]
      || buttonStatus.wasButtonPressedAndReleased[Constants::MINUS_BUTTON_INDEX])
  {
    isButtonResetRequested = true;
  }

  /* Go to main screen if back is pressed, timeout occurred or of an active BT has been deactivated externally */
  if (isBackButtonPressed(buttonStatus)
      || isConnectionTimeoutTrigged(timeSinceBootMs, uiState)
      || isSetupCompleted(uiState))
  {
    uiState.currentScreen = m_backScreen;
    isButtonResetRequested = true;
  }

  m_previousIsConnected = uiState.isBluetoothConnected;

  /* These are unused on purpose, so use these statements without side effects to please the compiler */
  static_cast<void>(settings);

  return isButtonResetRequested;
}

void BluetoothSetupScreenController::enter(UIState &uiState, SettingsInterface &settings)
{
  /* These are unused on purpose, so use these statements without side effects to please the compiler */
  static_cast<void>(settings);
  static_cast<void>(uiState);
  m_hasBeenConnected = false;
  m_previousIsConnected = false;
  m_timeoutStartTime = uiState.currentScreenEnterTimestampMs;
  m_timeoutLimit = Constants::BLUETOOTH_SCREEN_DEFAULT_CONN_WAIT_TIME_MS;
}


bool BluetoothSetupScreenController::isBackButtonPressed(const ButtonStatus &buttonStatus)
{
  return (buttonHoldEvent(Constants::BACK_BUTTON_INDEX, buttonStatus)) ||(buttonStatus.wasButtonPressedAndReleased[Constants::BACK_BUTTON_INDEX]);
}

bool BluetoothSetupScreenController::isConnectionTimeoutTrigged(uint64_t timeSinceBootMs, const UIState &uiState) const
{
  return (((timeSinceBootMs - m_timeoutStartTime) >= static_cast<uint64_t>(m_timeoutLimit))
      && (!uiState.isBluetoothConnected));
}

bool BluetoothSetupScreenController::isSetupCompleted(const UIState &uiState) const
{
  // Under normal conditions a BT deactivation should not occur, so this is a 'better safe than sorry' check.
  return (m_hasBeenConnected && !uiState.isBluetoothActive);
}

