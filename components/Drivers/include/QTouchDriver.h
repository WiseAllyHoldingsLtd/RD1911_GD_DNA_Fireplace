#pragma once

#include "Task.hpp"

#include "TouchDriverInterface.h"
#include "TimerDriverInterface.h"


class QTouchDriver : public TouchDriverInterface
{
public:
  QTouchDriver(void);
  virtual void getTouchStatus(bool (&isButtonTouched)[NUMBER_OF_BUTTONS]);

private:
  uint8_t m_buttonTouchCounter[NUMBER_OF_BUTTONS];
};
