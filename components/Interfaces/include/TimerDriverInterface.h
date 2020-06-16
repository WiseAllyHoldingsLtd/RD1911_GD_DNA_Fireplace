#pragma once

#include <stdint.h>


class TimerDriverInterface
{
public:
  /*
   * Returns number of milliseconds since system power on
   * 64-bit value lasts 585 million years worth of ms.
   */
  virtual uint64_t getTimeSinceBootMS() = 0;

  /*
   * Returns number of milliseconds since provided timestamp.
   */
  virtual uint64_t getTimeSinceTimestampMS(uint64_t timestamp) = 0;

  /*
   * Wait X number of milliseconds before returning. (spin lock)
   */
  virtual void waitMS(uint64_t milliseconds) = 0;

  virtual ~TimerDriverInterface() {};
};
