#pragma once

#include <stdint.h>

#define NUMBER_OF_BUTTONS 3U


class TouchDriverInterface
{
public:
  /**
   * Returns array of bools that are true if button is currently touched.
   */
  virtual void getTouchStatus(bool (&isButtonTouched)[NUMBER_OF_BUTTONS]) = 0;

  virtual ~TouchDriverInterface() {};
};
