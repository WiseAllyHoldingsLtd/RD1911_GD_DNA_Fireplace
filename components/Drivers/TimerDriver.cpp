#include "FreeRTOS.hpp"
#include "TimerDriver.h"


TimerDriver::TimerDriver(void)
  : m_timeSinceBoot(0U), m_lastTickCount(0U)
{
}

uint64_t TimerDriver::getTimeSinceBootMS(void)
{
  m_lock.take();
  uint32_t currentTickCount = xTaskGetTickCount(); // Cannot be called from ISR (must use ...FromISR if required)
  m_timeSinceBoot += (currentTickCount - m_lastTickCount); // Difference is correct even after overflow
  m_lastTickCount = currentTickCount;
  uint64_t returnVal = m_timeSinceBoot;
  m_lock.give();

  return returnVal;
}


uint64_t TimerDriver::getTimeSinceTimestampMS(uint64_t timestamp)
{
  return getTimeSinceBootMS() - timestamp;
}


void TimerDriver::waitMS(uint64_t milliseconds)
{
  FreeRTOS_delay_ms(milliseconds);
}
