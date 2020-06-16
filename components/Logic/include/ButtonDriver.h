#pragma once

#include "Task.hpp"

#include "ButtonDriverInterface.h"
#include "TimerDriverInterface.h"
#include "TouchDriverInterface.h"


class ButtonDriver : public ButtonDriverInterface, public Task
{
public:
  ButtonDriver(TouchDriverInterface & touchDriver, TimerDriverInterface & timerDriver);
  virtual ButtonStatus getButtonStatus(void);
  virtual void resetButtonStatus(void);

  /*
   * Checks buttons and updates internal status (pressed, hold, duration..)
   * Should be called regularly.
   */
  void update();

  /*
   * If driver is started as a Task, this will handle calling update()
   */
  virtual void run(bool eternalLoop);

private:
  /**
   * clears m_isReset flag when all buttons are released/up.
   */
  inline void clearResetFlagIfNoButtonsDown(bool isButtonTouched[NUMBER_OF_BUTTONS]);

  inline bool areAnyButtonsCurrentlyHeld() const;

  bool m_wasButtonPressedAndReleased[NUMBER_OF_BUTTONS];
  uint16_t m_buttonDownDurationMS[NUMBER_OF_BUTTONS];

  bool m_isButtonDown[NUMBER_OF_BUTTONS];
  bool m_isButtonHeld[NUMBER_OF_BUTTONS];
  bool m_isButtonStartedBeingHeld[NUMBER_OF_BUTTONS];
  bool m_isReset;

  uint64_t m_buttonDownTimestampMS[NUMBER_OF_BUTTONS];
  uint16_t m_cachedButtonDownDurationMS[NUMBER_OF_BUTTONS];
  bool m_cachedIsButtonHeld[NUMBER_OF_BUTTONS];
  
  TouchDriverInterface & m_touchDriver;
  TimerDriverInterface & m_timerDriver;
};
