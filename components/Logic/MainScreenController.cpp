#include "MainScreenController.h"


bool MainScreenController::control(uint64_t timeSinceBootMs, UIState &uiState, const ButtonStatus &buttonStatus, SettingsInterface &settings)
{
  ScreenControllerBase::controlTimeout(timeSinceBootMs, uiState, buttonStatus);

  /* These are unused on purpose, so use these statements without side effects to please the compiler */
  static_cast<void>(timeSinceBootMs);

  bool isButtonResetRequested = false;

  isButtonResetRequested = controlBluetoothSetupRedirect(uiState, buttonStatus);

  if ((buttonHoldEvent(Constants::PLUS_BUTTON_INDEX, buttonStatus))
      || (buttonStatus.wasButtonPressedAndReleased[Constants::PLUS_BUTTON_INDEX]))
  {
    if (!uiState.isScreenOff)
    {
      settings.increaseSetPoint();
    }

    isButtonResetRequested = true;
  }

  if ((buttonHoldEvent(Constants::MINUS_BUTTON_INDEX, buttonStatus))
      || (buttonStatus.wasButtonPressedAndReleased[Constants::MINUS_BUTTON_INDEX]))
  {
    if (!uiState.isScreenOff)
    {
      settings.decreaseSetPoint();
    }

    isButtonResetRequested = true;
  }

  if ((buttonHoldEvent(Constants::BACK_BUTTON_INDEX, buttonStatus))
      ||
      (buttonStatus.wasButtonPressedAndReleased[Constants::BACK_BUTTON_INDEX]))
  {
    if (!uiState.isScreenOff)
    {
      settings.toggleOverride();
    }

    isButtonResetRequested = true;
  }

  return isButtonResetRequested;
}

void MainScreenController::enter(UIState &uiState, SettingsInterface &settings)
{
  /* These are unused on purpose, so use these statements without side effects to please the compiler */
  static_cast<void>(uiState);
  static_cast<void>(settings);
}

bool MainScreenController::controlBluetoothSetupRedirect(UIState &uiState, const ButtonStatus &buttonStatus)
{
  bool isButtonResetRequested = false;

  if ((buttonStatus.isButtonHeld[Constants::BACK_BUTTON_INDEX]) &&
    (buttonStatus.isButtonHeld[Constants::PLUS_BUTTON_INDEX]) &&
    (!buttonStatus.isButtonHeld[Constants::MINUS_BUTTON_INDEX]) &&
    (!buttonStatus.isButtonStartedBeingHeld[Constants::MINUS_BUTTON_INDEX]) &&
    ((buttonStatus.buttonHeldDurationMs[Constants::BACK_BUTTON_INDEX]) >= Constants::BUTTON_COMBO_HOLD_TIME_MS) &&
    ((buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX]) >= Constants::BUTTON_COMBO_HOLD_TIME_MS))
  {
    uiState.currentScreen = Screen::bluetoothSetupScreen;
    isButtonResetRequested = true;
  }
  return isButtonResetRequested;
}
