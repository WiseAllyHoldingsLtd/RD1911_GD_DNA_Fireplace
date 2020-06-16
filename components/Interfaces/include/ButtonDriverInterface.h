#pragma once

#include "ButtonStatus.h"

class ButtonDriverInterface
{
public:
  /**
   * Returns status of buttons, answering the question "what has happened
   * since last time resetButtonStatus() was called?"
   */
  virtual ButtonStatus getButtonStatus(void) = 0;

  /**
   * Makes getButtonStatus() return no info about buttons previously pressed/held
   * until new button events occur.
   *
   * NOTE: If buttons are pressed/held when resetButtonStatus() is called, no new actions
   *       must be detected for this button until it is released and pressed again!
   */
  virtual void resetButtonStatus(void) = 0;

  virtual ~ButtonDriverInterface() {};
};
