#include "GlobalHoldButtonController.h"
#include "Constants.h"
#include "Settings.h"


GlobalHoldButtonController::GlobalHoldButtonController()
: m_lockScreen(Screen::buttonLockedScreen),
  m_unlockScreen(Screen::buttonUnlockedScreen),
  m_buttonTestScreen(Screen::buttonTestScreen)
{
}


bool GlobalHoldButtonController::control(UIState &uiState, 
                                         const ButtonStatus &buttonStatus, 
                                         ButtonDriverInterface &buttonDriver,
                                         SettingsInterface &settings)
{
  /* These are unused on purpose, so use these statements without side effects to please the compiler */
  static_cast<void>(buttonDriver);

  bool isButtonResetRequested = false;
    isButtonResetRequested |= controlButtonLock(uiState, buttonStatus, settings);

  if (!settings.isButtonLockOn())
  {
    isButtonResetRequested |= controlButtonTest(uiState, buttonStatus);
  }

  return isButtonResetRequested;
}


bool GlobalHoldButtonController::shouldDisplayButtonLockText(const ButtonStatus &buttonStatus)
{
  bool shouldDisplayLockText = true;

  if ((buttonStatus.isButtonStartedBeingHeld[Constants::PLUS_BUTTON_INDEX]) &&
    (buttonStatus.isButtonStartedBeingHeld[Constants::MINUS_BUTTON_INDEX]))
  {
    shouldDisplayLockText = false;
  }
  else if (buttonStatus.isAnyButtonPressedAndReleased())
  {
    //true
  }
  else if (!buttonStatus.isButtonActivity())
  {
    shouldDisplayLockText = false;
  }
  else
  {
    //true
  }

  return shouldDisplayLockText;
}


bool GlobalHoldButtonController::controlButtonLock(UIState &uiState, const ButtonStatus &buttonStatus, SettingsInterface &settings)
{
  bool isButtonResetRequested = false;

  if ((buttonStatus.isButtonHeld[Constants::PLUS_BUTTON_INDEX]) &&
    (buttonStatus.isButtonHeld[Constants::MINUS_BUTTON_INDEX]) &&
    ((buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX]) >= Constants::BUTTON_COMBO_HOLD_TIME_MS) &&
    ((buttonStatus.buttonHeldDurationMs[Constants::MINUS_BUTTON_INDEX]) >= Constants::BUTTON_COMBO_HOLD_TIME_MS) &&
    (!(buttonStatus.isButtonHeld[Constants::BACK_BUTTON_INDEX])) &&
    (!(buttonStatus.isButtonStartedBeingHeld[Constants::BACK_BUTTON_INDEX])))
  {
    /* We don't want to enable lockscreen when button testing or using bluetooth. */
    if (uiState.currentScreen != static_cast<int>(Screen::buttonTestScreen) &&
        uiState.currentScreen != static_cast<int>(Screen::bluetoothSetupScreen)) /* Cast is needed to please misra. */
    {
      settings.toggleButtonLock();
      if (settings.isButtonLockOn()) {
        uiState.currentScreen = m_lockScreen;
      } else {
        uiState.currentScreen = m_unlockScreen;
      }
      // Update new screen's enter time because control() will be called before enter()
      uiState.currentScreenEnterTimestampMs = uiState.timeSinceBootMs;
      isButtonResetRequested = true;
    }
  }

  return isButtonResetRequested;
}

bool GlobalHoldButtonController::controlButtonTest(UIState &uiState, const ButtonStatus &buttonStatus)
{
  bool isButtonResetRequested = false;
  if ((buttonStatus.isButtonHeld[Constants::MINUS_BUTTON_INDEX]) &&
    (buttonStatus.isButtonHeld[Constants::PLUS_BUTTON_INDEX]) &&
    (buttonStatus.isButtonHeld[Constants::BACK_BUTTON_INDEX]) &&
    ((buttonStatus.buttonHeldDurationMs[Constants::MINUS_BUTTON_INDEX]) >= Constants::BUTTON_TEST_HOLD_TIME_MS) &&
    ((buttonStatus.buttonHeldDurationMs[Constants::PLUS_BUTTON_INDEX]) >= Constants::BUTTON_TEST_HOLD_TIME_MS) &&
    ((buttonStatus.buttonHeldDurationMs[Constants::BACK_BUTTON_INDEX]) >= Constants::BUTTON_TEST_HOLD_TIME_MS))
  {
    uiState.currentScreen = m_buttonTestScreen;
    /*we want the buttons to still be held on the next screen so we don't set isButtonResetRequested to true*/
  }

  return isButtonResetRequested; 
}

