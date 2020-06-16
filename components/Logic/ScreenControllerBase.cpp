#include "ScreenControllerBase.h"
#include "Constants.h"

void ScreenControllerBase::controlTimeout(uint64_t timeSinceBootMs, UIState &uiState, const ButtonStatus &buttonStatus)
{
  /*if the lastActiveTime is less than the enterScreen time, then we need to update the lastActiveTime*/
  if (uiState.currentScreenEnterTimestampMs > uiState.timestampLastActiveMs)
  {
    uiState.timestampLastActiveMs = uiState.currentScreenEnterTimestampMs;
  }
  if (buttonStatus.isButtonActivity())
  {
    uiState.timestampLastActiveMs = timeSinceBootMs;
  }
  uint64_t timeInactiveMs = timeSinceBootMs - uiState.timestampLastActiveMs;

  /* We need to check if there should be a timeout or not, then check inactive time(ms) against timeout time (s),
     that's why we need to multiply m_timeoutTimeS with 1000. */
  if ((m_timeoutTimeS != 0U) && (timeInactiveMs >= static_cast<uint64_t>(m_timeoutTimeS)*1000U))
  {
    uiState.currentScreen = m_timeoutScreen;
  }
 
}

bool ScreenControllerBase::buttonHoldEvent(uint8_t buttonIndex, const ButtonStatus &buttonStatus, uint32_t fireRate)
{
  bool isButtonHoldEvent = true;
  bool isAnyButtonCurrentlyHeld = false;
  for (uint16_t i = 0U; i < NUMBER_OF_BUTTONS; i++)
  {
    if (i == buttonIndex)
    {
      if (!buttonStatus.isButtonHeld[i])
      {
        isButtonHoldEvent = false;
      }
    }
    else
    {
      if ((buttonStatus.isButtonStartedBeingHeld[i]) || (buttonStatus.isButtonHeld[i])) /* Other buttons cannot be started being held simultaneously */
      {
        isButtonHoldEvent = false;
      }
    }

    if (buttonStatus.isButtonHeld[i])
    {
      isAnyButtonCurrentlyHeld = true;
    }
  }

  if (!isAnyButtonCurrentlyHeld)
  {
    /* Reset counter if no buttons are held, to be ready for next time: */
    m_buttonHoldCounter = 0U;
  }

  if (isButtonHoldEvent)
  {
    if (buttonStatus.buttonHeldDurationMs[buttonIndex] >= 
        (Constants::BUTTON_HOLD_FIRST_FIRE_MS + (static_cast<uint32_t>(m_buttonHoldCounter) * fireRate)))
    {
      /*
      heldDuration = holdThreshold + c * fireRate
      c = (heldDuration - holdThreshold) / fireRate.  Round down to nearest integer.
      Then increase c by one to know when to expect next hold event to occur.
      */
      m_buttonHoldCounter = static_cast<uint8_t>((buttonStatus.buttonHeldDurationMs[buttonIndex] - Constants::BUTTON_HOLD_FIRST_FIRE_MS) / fireRate);
      m_buttonHoldCounter++;
    }
    else
    {
      isButtonHoldEvent = false;
    }
  }

  return isButtonHoldEvent;
}
