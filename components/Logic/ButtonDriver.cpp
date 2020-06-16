#include "ButtonDriver.h"
#include "Constants.h"


ButtonDriver::ButtonDriver(TouchDriverInterface & touchDriver, TimerDriverInterface & timerDriver) : m_touchDriver(touchDriver), m_timerDriver(timerDriver)
{
  resetButtonStatus();
  m_isReset = false; /* This is the only difference between what happens in resetButtonStatus() and this constructor. */
}

ButtonStatus ButtonDriver::getButtonStatus(void)
{
  ButtonStatus buttonStatus = {};
  for (uint16_t i = 0U; i < NUMBER_OF_BUTTONS; i++)
  {
    buttonStatus.wasButtonPressedAndReleased[i] = m_wasButtonPressedAndReleased[i];
    buttonStatus.isButtonStartedBeingHeld[i] = m_isButtonStartedBeingHeld[i];
    buttonStatus.isButtonHeld[i] = m_cachedIsButtonHeld[i];
    if (m_cachedIsButtonHeld[i])
    {
      buttonStatus.buttonHeldDurationMs[i] = m_cachedButtonDownDurationMS[i];

      //Reset cached value that is now "used":
      m_cachedIsButtonHeld[i] = false;
    }
  }
  return buttonStatus;
}

void ButtonDriver::resetButtonStatus(void)
{
  m_isReset = true;
  for (uint16_t i = 0U; i < NUMBER_OF_BUTTONS; i++)
  {
    m_wasButtonPressedAndReleased[i] = false;
    m_isButtonDown[i] = false;
    m_isButtonHeld[i] = false;
    m_cachedIsButtonHeld[i] = false;
    m_isButtonStartedBeingHeld[i] = false;
    m_buttonDownDurationMS[i] = 0U;
  }
}

void ButtonDriver::update()
{
  bool isButtonTouched[NUMBER_OF_BUTTONS];
  m_touchDriver.getTouchStatus(isButtonTouched);

  if (m_isReset) {
    clearResetFlagIfNoButtonsDown(isButtonTouched);
  }
  else {
    for (uint16_t i = 0U; i < NUMBER_OF_BUTTONS; i++) {
      if (isButtonTouched[i]) { /* Button currently down? */
        if (m_isButtonDown[i]) {
          // Repeated sighting
          m_buttonDownDurationMS[i] = static_cast<uint16_t>(m_timerDriver.getTimeSinceTimestampMS(m_buttonDownTimestampMS[i]));

          // Check holding
          if (m_buttonDownDurationMS[i] > Constants::BUTTON_HELD_THRESHOLD_MS) {
            m_isButtonHeld[i] = true;
          }
          if (m_buttonDownDurationMS[i] > Constants::BUTTON_STARTED_BEING_HELD_THRESHOLD_MS) {
            m_isButtonStartedBeingHeld[i] = true;
          }
        }
        else {
          // First sighting
          m_isButtonDown[i] = true;
          m_buttonDownTimestampMS[i] = m_timerDriver.getTimeSinceBootMS();
        }
      }
      else {
        if ((!areAnyButtonsCurrentlyHeld()) && (m_isButtonDown[i])) {
          m_wasButtonPressedAndReleased[i] = true;
        }
        m_buttonDownDurationMS[i] = 0U;
        m_isButtonDown[i] = false;
        m_isButtonHeld[i] = false;
        m_isButtonStartedBeingHeld[i] = false;
      }

      /* Set cached button held value to true if button is held, because we want to ensure that
         this information is not lost if getButtonStatus() is called only after the buttons is no longer held. */
      if (m_isButtonHeld[i]) {
        m_cachedIsButtonHeld[i] = true;
        m_cachedButtonDownDurationMS[i] = m_buttonDownDurationMS[i];
      }
    }
  }
}

void ButtonDriver::run(bool eternalLoop)
{
  while (eternalLoop) {
    update();
    delayMS(Constants::BUTTON_UPDATE_INTERVAL_MS);
  }
}


inline void ButtonDriver::clearResetFlagIfNoButtonsDown(bool isButtonTouched[NUMBER_OF_BUTTONS])
{
  bool areAnyButtonsCurrentlyDown = false;
  for (uint16_t i = 0U; i < NUMBER_OF_BUTTONS; i++)
  {
    if (isButtonTouched[i])
    {
      areAnyButtonsCurrentlyDown = true;
      break;
    }
  }
  if (!areAnyButtonsCurrentlyDown)
  {
    m_isReset = false;
  }
}

inline bool ButtonDriver::areAnyButtonsCurrentlyHeld() const
{
  bool areAnyButtonsCurrentlyHeld = false;
  for (uint16_t i = 0U; i < NUMBER_OF_BUTTONS; i++)
  {
    if (m_isButtonHeld[i])
    {
      areAnyButtonsCurrentlyHeld = true;
      break;
    }
  }
  return areAnyButtonsCurrentlyHeld;
}

