#pragma once

#include "RebootSchedulerInterface.h"
#include "SystemTimeDriverInterface.h"
#include "SoftwareResetDriverInterface.h"
#include "Constants.h"

class RebootScheduler: public RebootSchedulerInterface
{
public:
  RebootScheduler(SoftwareResetDriverInterface &softwareResetDriver,
                    SystemTimeDriverInterface &systemTimeDriver);

  virtual bool resetAndInit(
      const uint8_t (&gdid)[Constants::GDID_BCD_SIZE],
      const DateTime &bootTime,
      const DateTime &preferredRebootTimeSpanStart,
      uint32_t preferredRebootTimeSpanLength,
      uint32_t maxUptimeS);

  virtual bool shouldReboot(const DateTime &localTime);
  virtual uint32_t getNextRebootTimeUnixLocal();
private:
  bool calculateNextRebootTime(const DateTime &bootTime,
                               const DateTime &preferredRebootTimeSpanStart,
                               const uint32_t maxUptimeS);
  void makeRebootTimeUnique(const uint8_t (&gdid)[Constants::GDID_BCD_SIZE],
                            uint32_t timespan);

  SoftwareResetDriverInterface &m_resetDriver;
  SystemTimeDriverInterface &m_systemTimeDriver;
  uint32_t m_nextRebootTimeUnix; // "local" unix time, i.e. unix time with time zone offset
};
