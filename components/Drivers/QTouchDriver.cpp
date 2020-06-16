#include "driver/gpio.h"

#include "QTouchDriver.h"
#include "HardwareSetup.h"
#include "Constants.h"

/* Compile time check if number of buttons has changed */
static_assert(NUMBER_OF_BUTTONS == 3U, "Loop is unrolled for 3 buttons. Please update file.");


QTouchDriver::QTouchDriver(void)
{
  for (uint8_t i = 0; i < NUMBER_OF_BUTTONS; ++i)
  {
    m_buttonTouchCounter[i] = 0U;
  }
}

void QTouchDriver::getTouchStatus(bool (&isButtonTouched)[NUMBER_OF_BUTTONS])
{
  bool qTouchActive[NUMBER_OF_BUTTONS];
  qTouchActive[0u] = (gpio_get_level(QTOUCH_SW1_PIN) == 1);
  qTouchActive[1u] = (gpio_get_level(QTOUCH_SW2_PIN) == 1);
  qTouchActive[2u] = (gpio_get_level(QTOUCH_SW3_PIN) == 1);

  for (uint8_t i = 0u; i < NUMBER_OF_BUTTONS; ++i)
  {
    if (qTouchActive[i])
    {
      if (m_buttonTouchCounter[i] < Constants::BUTTON_DOWN_COUNTER_LIMIT)
      {
        ++m_buttonTouchCounter[i];
      }
    }
    else
    {
      m_buttonTouchCounter[i] = 0U;
    }
  }

  isButtonTouched[0u] = (m_buttonTouchCounter[0u] >= Constants::BUTTON_DOWN_COUNTER_LIMIT);
  isButtonTouched[1u] = (m_buttonTouchCounter[1u] >= Constants::BUTTON_DOWN_COUNTER_LIMIT);
  isButtonTouched[2u] = (m_buttonTouchCounter[2u] >= Constants::BUTTON_DOWN_COUNTER_LIMIT);
}
