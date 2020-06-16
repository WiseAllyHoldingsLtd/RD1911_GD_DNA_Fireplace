
#include "include/RebootScheduler.h"

#include "EspCpp.hpp"

namespace
{
  const char LOG_TAG[] = "RebootScheduler";
}

RebootScheduler::RebootScheduler(
    SoftwareResetDriverInterface &softwareResetDriver,
    SystemTimeDriverInterface &systemTimeDriver):

    m_resetDriver(softwareResetDriver),
    m_systemTimeDriver(systemTimeDriver),
    m_nextRebootTimeUnix(0u)
{

}

bool RebootScheduler::resetAndInit(
    const uint8_t (&gdid)[Constants::GDID_BCD_SIZE],
    const DateTime &bootTime,
    const DateTime &preferredRebootTimeSpanStart,
    uint32_t preferredRebootTimeSpanLength,
    const uint32_t maxUptimeS)
{
  bool foundRebootTime = calculateNextRebootTime(bootTime,
                                                 preferredRebootTimeSpanStart,
                                                 maxUptimeS);
  if ( foundRebootTime )
  {
    makeRebootTimeUnique(gdid, preferredRebootTimeSpanLength);
  }

  return foundRebootTime;
}

bool RebootScheduler::shouldReboot(const DateTime &localTime)
{
  /* TODO unit tests + logging of calculated reboots + (last) real testing on hw */

  bool shouldRebootNow = false;
  uint32_t localTimeUnix = m_systemTimeDriver.getUnixTimeFromDateTime(localTime);

  if ( localTimeUnix >= m_nextRebootTimeUnix )
  {
    shouldRebootNow = true;
  }

  return shouldRebootNow;
}

uint32_t RebootScheduler::getNextRebootTimeUnixLocal()
{
  return m_nextRebootTimeUnix;
}

bool RebootScheduler::calculateNextRebootTime(
    const DateTime &bootTime,
    const DateTime &preferredRebootTimeSpanStart,
    const uint32_t maxUptimeS)
{
  bool foundRebootTime = false;

  uint32_t bootTimeUnix = m_systemTimeDriver.getUnixTimeFromDateTime(bootTime);
  uint32_t latestRebootUnix = bootTimeUnix + maxUptimeS;

  // Search forward from boot time to latestReboot to find closest preferred
  // reboot time interval

  DateTime candidateRebootTime = bootTime;

  // Calculate first candidate
  // preferred time span is defined in local time...
  candidateRebootTime.hours = preferredRebootTimeSpanStart.hours;
  candidateRebootTime.minutes = preferredRebootTimeSpanStart.minutes;
  candidateRebootTime.seconds = preferredRebootTimeSpanStart.seconds;

  uint32_t candidateRebootTimeUnix =
      m_systemTimeDriver.getUnixTimeFromDateTime(candidateRebootTime);

  for ( ;
        candidateRebootTimeUnix <= latestRebootUnix;
        candidateRebootTimeUnix += 24u*60u*60u )
  {
    // Test candidate
    if ( candidateRebootTimeUnix > bootTimeUnix )
    {
      m_nextRebootTimeUnix = candidateRebootTimeUnix;
      foundRebootTime = true;
    }
  }

  if ( foundRebootTime )
  {
    ESP_LOGI(LOG_TAG, "Found reboot time: %u", m_nextRebootTimeUnix);
  }

  return foundRebootTime;
}

/* Make reboot time unique to avoid DDOSing cloud services */
void RebootScheduler::makeRebootTimeUnique(
    const uint8_t (&gdid)[Constants::GDID_BCD_SIZE],
    uint32_t timespan)
{
  // Assumes gdid is LSB first
  uint32_t dateValue = static_cast<uint32_t>(*reinterpret_cast<const uint16_t *>(gdid+2));
  uint32_t sequenceNumber = static_cast<uint32_t>(*reinterpret_cast<const uint16_t *>(gdid));

  uint32_t uniqueOffset = (sequenceNumber*Constants::REBOOT_SCHEDULER_SEQNUM_COEFFICIENT
                         + dateValue*Constants::REBOOT_SCHEDULER_DATEVALUE_COEFFICIENT) % timespan;

  m_nextRebootTimeUnix += uniqueOffset;

  ESP_LOGI(LOG_TAG, "Unique offset: %u and final reboot time: %u",
      uniqueOffset, m_nextRebootTimeUnix);

  DateTime dateTime = Constants::DEFAULT_DATETIME;
  m_systemTimeDriver.getDateTimeFromUnixTime(m_nextRebootTimeUnix, dateTime);
  ESP_LOGI(LOG_TAG, "Next reboot: %04i-%02i-%02i %02i:%02i:%02i",
                    2000U + dateTime.year, dateTime.month, dateTime.days,
                    dateTime.hours, dateTime.minutes, dateTime.seconds);
}
