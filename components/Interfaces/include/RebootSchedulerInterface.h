#pragma once

#include "DateTime.h"
#include "Constants.h"

class RebootSchedulerInterface
{
public:
  virtual ~RebootSchedulerInterface() {}

  virtual bool resetAndInit(
      const uint8_t (&gdid)[Constants::GDID_BCD_SIZE],
      const DateTime &bootTime,
      const DateTime &preferredRebootTimeSpanStart,
      uint32_t preferredRebootTimeSpanLength,
      const uint32_t maxUptimeS) = 0;
  virtual bool shouldReboot(const DateTime &localTime) = 0;
  virtual uint32_t getNextRebootTimeUnixLocal() = 0;
};
