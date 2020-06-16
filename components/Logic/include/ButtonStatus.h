#pragma once

#include <stdint.h>

#define NUMBER_OF_BUTTONS 3U

struct ButtonStatus
{
  /**
   * True for each of the buttons if it has been pressed and released again.
   */
  bool wasButtonPressedAndReleased[NUMBER_OF_BUTTONS];

  /**
  * True for each of the buttons if it has been held and NOT YET released again.
  */
  bool isButtonHeld[NUMBER_OF_BUTTONS];

  /**
  * True for each of the buttons if it has started being held (lower threshold than "normal" isButtonHeld).
  * This is used to prevent handling a button held event if other buttons are also started being held at the 
  * same time, even if they did not start being pressed at the exact same time.
  */
  bool isButtonStartedBeingHeld[NUMBER_OF_BUTTONS];

  /**
  * How long each button has currently been held down (milliseconds).
  */
  uint32_t buttonHeldDurationMs[NUMBER_OF_BUTTONS];

  /**
  *        Returns whether a buttons was pressed and released or held.
  */
  bool isButtonActivity() const;

  /**
  *        Returns whether a buttons was pressed and released.
  */
  bool isAnyButtonPressedAndReleased() const;
};

