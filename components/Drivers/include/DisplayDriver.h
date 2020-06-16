#pragma once

// include system headers for structs
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/timer.h"

#include "DisplayDriverInterface.h"
#include "BacklightDriverInterface.h"


class DisplayDriver : public DisplayDriverInterface, public BacklightDriverInterface
{
public:
	DisplayDriver();

  virtual void resetAndInit(void);
  /*
   * Set the data to display.
   */
  virtual void setData(uint8_t data[3]);

  /*
   * This method draws the display with current data.
   * Must be called periodically to ensure proper visibility.
   */
  bool interruptHandler();

  /*
   * Sets the backlight level
   * a value of 0-200 sets level in half-percent steps (100 => 50%)
   */
  virtual void setBacklightLevel(uint8_t level);

private:
  void reset();
  void init();
  void setGpioISR(gpio_num_t gpio_num, uint32_t level) const;
  void setLEDCDutyISR(ledc_channel_t channel, uint32_t duty) const;
  void initTimer(timer_idx_t timerIdx, uint16_t intervalMS) const;
  void stopTimer(timer_idx_t timerIdx) const;

  uint8_t m_data[6];
  uint8_t m_currentSector;
  uint16_t m_dutyCycle;
  bool m_displayActive;
};

