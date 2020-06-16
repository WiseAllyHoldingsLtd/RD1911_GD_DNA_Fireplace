#include <cstring>

#include "driver/gpio.h"
#include "driver/timer.h"
#include "driver/ledc.h"
#include "soc/gpio_struct.h"
#include "soc/ledc_struct.h"

#include "HardwareSetup.h"
#include "DisplayDriver.h"

#define TIMER_DIVIDER 16u  //  Hardware timer clock divider
#define TIMER_SCALE (TIMER_BASE_CLK / TIMER_DIVIDER)  // convert counter value to seconds

namespace {
  /*
   * This table contains GPIO PIN mapping for the sectors.
   */
  const DRAM_ATTR gpio_num_t SECTORS[3] = {
      DISPLAY_DIG_3_PIN,
      DISPLAY_DIG_4_PIN,
      DISPLAY_DIG_2_PIN,
  };

  // Alarm intervals
  volatile uint32_t alarm_low_standard; // set in init_timer
  const uint32_t alarm_low_short = (2 * TIMER_SCALE/10000u); // 200 microseconds

  /* instance used by interrupt */
  DisplayDriver *instance = nullptr;

  /*
   * Timer interrupt group 0 handler
   * NOTE: each timer group has 2 timers, but we only use one for display interrupt.
   * IF the need for another timer arises, consider moving this into a separate TimerInterruptDriver.
   */
  void IRAM_ATTR timer_group0_isr(void *para)
  {
    timer_idx_t timer_idx = static_cast<timer_idx_t>(reinterpret_cast<uintptr_t>(para));
    uint32_t intr_status = TIMERG0.int_st_timers.val;

    if ((intr_status & BIT(timer_idx)) && timer_idx == TIMER_0) {
      if (instance != nullptr) {
        bool retrigger = instance->interruptHandler();
        TIMERG0.hw_timer[timer_idx].alarm_low = retrigger ? alarm_low_short : alarm_low_standard;
      }
      TIMERG0.int_clr_timers.t0 = 1;
    }
    // Enable timer
    TIMERG0.hw_timer[timer_idx].config.alarm_en = TIMER_ALARM_EN;
  }
}

DisplayDriver::DisplayDriver()
: m_data{0,0,0}, m_currentSector(0u), m_dutyCycle(0u), m_displayActive(true)
{
}

void DisplayDriver::reset()
{
  // Make sure interrupt is not running
  stopTimer(TIMER_0);

  // Set all display pins high (inactive)
  gpio_set_level(DISPLAY_DIG_2_PIN, 1u);
  gpio_set_level(DISPLAY_DIG_3_PIN, 1u);
  gpio_set_level(DISPLAY_DIG_4_PIN, 1u);

  // Segment PWMs at max duty (inactive)
  ledc_set_duty(LEDC_HIGH_SPEED_MODE, DISPLAY_SEG_A_LEDC, DISPLAY_LEDC_MAX_DUTY);
  ledc_set_duty(LEDC_HIGH_SPEED_MODE, DISPLAY_SEG_B_LEDC, DISPLAY_LEDC_MAX_DUTY);
  ledc_set_duty(LEDC_HIGH_SPEED_MODE, DISPLAY_SEG_C_LEDC, DISPLAY_LEDC_MAX_DUTY);
  ledc_set_duty(LEDC_HIGH_SPEED_MODE, DISPLAY_SEG_D_LEDC, DISPLAY_LEDC_MAX_DUTY);
  ledc_set_duty(LEDC_HIGH_SPEED_MODE, DISPLAY_SEG_E_LEDC, DISPLAY_LEDC_MAX_DUTY);
  ledc_set_duty(LEDC_HIGH_SPEED_MODE, DISPLAY_SEG_F_LEDC, DISPLAY_LEDC_MAX_DUTY);
  ledc_set_duty(LEDC_HIGH_SPEED_MODE, DISPLAY_SEG_G_LEDC, DISPLAY_LEDC_MAX_DUTY);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, DISPLAY_SEG_A_LEDC);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, DISPLAY_SEG_B_LEDC);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, DISPLAY_SEG_C_LEDC);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, DISPLAY_SEG_D_LEDC);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, DISPLAY_SEG_E_LEDC);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, DISPLAY_SEG_F_LEDC);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, DISPLAY_SEG_G_LEDC);

  // Reset screen buffer
  memset(m_data, 0, sizeof(m_data));
  m_currentSector = 0u;
  instance = nullptr;
}

void DisplayDriver::init()
{
  instance = this;

  // Naively sets all bits in screen buffer
  memset(m_data, 0xFFu, sizeof(m_data));

  initTimer(TIMER_0, 3u); // Every 3 millisecond
}

void DisplayDriver::resetAndInit(void)
{
  reset();
  init();
}

void DisplayDriver::setData(uint8_t data[3])
{
  memcpy(m_data, data, sizeof(m_data));
}

void IRAM_ATTR DisplayDriver::setGpioISR(gpio_num_t gpio_num, uint32_t level) const
{
  /* Copied from esp-idf\components\driver\gpio.c */
  if (level)
  {
    if (gpio_num < 32)
    {
      GPIO.out_w1ts = (1 << gpio_num);
    }
    else
    {
      GPIO.out1_w1ts.data = (1 << (gpio_num - 32));
    }
  }
  else
  {
    if (gpio_num < 32)
    {
      GPIO.out_w1tc = (1 << gpio_num);
    }
    else
    {
      GPIO.out1_w1tc.data = (1 << (gpio_num - 32));
    }
  }
}

void IRAM_ATTR DisplayDriver::setLEDCDutyISR(ledc_channel_t channel, uint32_t duty) const
{
  // We only use High speed mode, so it is hardcoded here
  static const ledc_mode_t speed_mode = LEDC_HIGH_SPEED_MODE;

  // from ledc_set_duty / ledc_duty_config
  LEDC.channel_group[speed_mode].channel[channel].duty.duty = static_cast<uint32_t>(duty << 4u); // the least 4 bits are decimal part

  // from ledc_update_duty
  LEDC.channel_group[speed_mode].channel[channel].conf0.sig_out_en = 1;
  LEDC.channel_group[speed_mode].channel[channel].conf1.duty_start = 1;
}

bool IRAM_ATTR DisplayDriver::interruptHandler()
{
  bool retrigger = false;
  if (m_displayActive) {
    // Clear last sector
    setGpioISR(SECTORS[(m_currentSector+2u)%3u], 1u);  // (x+(n-1))%n gives n-1, 0, 1, ...., n-2

    // Set pins according to data segment pins
    if ((m_data[m_currentSector] & 1) != 0) {
      setLEDCDutyISR(DISPLAY_SEG_A_LEDC, m_dutyCycle);
    } else {
      setLEDCDutyISR(DISPLAY_SEG_A_LEDC, DISPLAY_LEDC_MAX_DUTY);
    }

    if ((m_data[m_currentSector] & 2) != 0) {
      setLEDCDutyISR(DISPLAY_SEG_B_LEDC, m_dutyCycle);
    } else {
      setLEDCDutyISR(DISPLAY_SEG_B_LEDC, DISPLAY_LEDC_MAX_DUTY);
    }

    if ((m_data[m_currentSector] & 4) != 0) {
      setLEDCDutyISR(DISPLAY_SEG_C_LEDC, m_dutyCycle);
    } else {
      setLEDCDutyISR(DISPLAY_SEG_C_LEDC, DISPLAY_LEDC_MAX_DUTY);
    }

    if ((m_data[m_currentSector] & 8) != 0) {
      setLEDCDutyISR(DISPLAY_SEG_D_LEDC, m_dutyCycle);
    } else {
      setLEDCDutyISR(DISPLAY_SEG_D_LEDC, DISPLAY_LEDC_MAX_DUTY);
    }

    if ((m_data[m_currentSector] & 16) != 0) {
      setLEDCDutyISR(DISPLAY_SEG_E_LEDC, m_dutyCycle);
    } else {
      setLEDCDutyISR(DISPLAY_SEG_E_LEDC, DISPLAY_LEDC_MAX_DUTY);
    }

    if ((m_data[m_currentSector] & 32) != 0) {
      setLEDCDutyISR(DISPLAY_SEG_F_LEDC, m_dutyCycle);
    } else {
      setLEDCDutyISR( DISPLAY_SEG_F_LEDC, DISPLAY_LEDC_MAX_DUTY);
    }

    if ((m_data[m_currentSector] & 64) != 0) {
      setLEDCDutyISR(DISPLAY_SEG_G_LEDC, m_dutyCycle);
    } else {
      setLEDCDutyISR(DISPLAY_SEG_G_LEDC, DISPLAY_LEDC_MAX_DUTY);
    }

    retrigger = true;
  }
  else {
    // Enable current sector
    setGpioISR(SECTORS[m_currentSector], 0u);

    // Advance to next sector
    m_currentSector = (m_currentSector + 1u) % 3u;
  }

  m_displayActive = !m_displayActive;

  return retrigger;
}

void DisplayDriver::setBacklightLevel(uint8_t level)
{
  // invert level due to active low
  // Scale 0-200 to 0-256
  m_dutyCycle = static_cast<uint8_t>(200u-level)*128/100; // multiplies with 1.28
}

void DisplayDriver::initTimer(timer_idx_t timerIdx, uint16_t intervalMS) const
{
  /* Select and initialize basic parameters of the timer */
  timer_config_t config;
  config.divider = TIMER_DIVIDER;
  config.counter_dir = TIMER_COUNT_UP;
  config.counter_en = TIMER_PAUSE;
  config.alarm_en = TIMER_ALARM_EN;
  config.intr_type = TIMER_INTR_LEVEL;
  config.auto_reload = true;
  timer_init(TIMER_GROUP_0, timerIdx, &config);

  // Initial/reset value
  timer_set_counter_value(TIMER_GROUP_0, timerIdx, 0u);

  /* Configure the alarm value and the interrupt on alarm. */
  alarm_low_standard = (intervalMS * TIMER_SCALE/1000u);
  timer_set_alarm_value(TIMER_GROUP_0, timerIdx, alarm_low_standard);
  timer_enable_intr(TIMER_GROUP_0, timerIdx);
  timer_isr_register(TIMER_GROUP_0, timerIdx, timer_group0_isr,
      reinterpret_cast<void*>(static_cast<uintptr_t>(timerIdx)), ESP_INTR_FLAG_IRAM, NULL);

  timer_start(TIMER_GROUP_0, timerIdx);
}

void DisplayDriver::stopTimer(timer_idx_t timerIdx) const
{
  timer_disable_intr(TIMER_GROUP_0, timerIdx);
}
