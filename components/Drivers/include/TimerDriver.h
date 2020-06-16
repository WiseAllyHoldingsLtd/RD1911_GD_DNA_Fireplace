#pragma once

#include "TimerDriverInterface.h"
#include "Mutex.hpp"


class TimerDriver : public TimerDriverInterface
{
public:
  TimerDriver(void);
  virtual uint64_t getTimeSinceBootMS();
  virtual uint64_t getTimeSinceTimestampMS(uint64_t timestamp);
  virtual void waitMS(uint64_t milliseconds);

private:

  Mutex m_lock;
  uint64_t m_timeSinceBoot;
  uint32_t m_lastTickCount;
};
