#ifdef __BSD_VISIBLE
#undef __BSD_VISIBLE
#else
#define __BSD_VISIBLE 1
#endif

#include <time.h>
#include <sys/time.h>

#include <EspCpp.hpp>
#include "Constants.h"
#include "SystemTimeDriver.h"


namespace
{
  const char * LOG_TAG = "SystemTime";
}

SystemTimeDriver::SystemTimeDriver(void)
{

}

void SystemTimeDriver::setUnixTime(uint32_t unixTime)
{
  timeval currentTime = {
      .tv_sec = static_cast<time_t>(unixTime),
      .tv_usec = 0u
  };

  if (settimeofday(&currentTime, nullptr) == 0)
  {
    ESP_LOGD(LOG_TAG, "Time set to %li seconds", currentTime.tv_sec);
  }
  else
  {
    ESP_LOGE(LOG_TAG, "Failed to set system time");
  }
}


uint32_t SystemTimeDriver::getUnixTime(void) const
{
  timeval now;

  if (gettimeofday(&now, nullptr) != 0)
  {
    ESP_LOGE(LOG_TAG, "Failed to read system time. Returning default.");
    DateTime dateTime = Constants::DEFAULT_DATETIME;
    now.tv_sec = static_cast<time_t>(getUnixTimeFromDateTime(dateTime));
  }

  return static_cast<uint32_t>(now.tv_sec);
}


uint32_t SystemTimeDriver::getUnixTimeFromDateTime(const DateTime & dateTime) const
{
  struct tm timeinfo;
  DateTime validDateTime = Constants::DEFAULT_DATETIME;

  if (dateTime.isValid())
  {
    validDateTime = dateTime;
  }
  else
  {
    ESP_LOGW(LOG_TAG, "Conversion from DateTime to unixTime with invalid DateTime => using default.");
  }

  timeinfo.tm_year = convertYearToSysTimeYear(validDateTime.year);
  timeinfo.tm_mon = convertMonthToSysTimeMon(validDateTime.month);
  timeinfo.tm_mday = static_cast<int>(validDateTime.days);
  timeinfo.tm_hour = static_cast<int>(validDateTime.hours);
  timeinfo.tm_min = static_cast<int>(validDateTime.minutes);
  timeinfo.tm_sec = static_cast<int>(validDateTime.seconds);

  return static_cast<uint32_t>(mktime(&timeinfo));
}


void SystemTimeDriver::getDateTimeFromUnixTime(uint32_t unixTime, DateTime & dateTime) const
{
  time_t unixTimeAsTimeT = static_cast<time_t>(unixTime);
  struct tm timeinfo;

  gmtime_r(&unixTimeAsTimeT, &timeinfo);
  dateTime.year = convertSysTimeYearToYear(timeinfo.tm_year);
  dateTime.month = convertSysTimeMonToMonth(timeinfo.tm_mon);
  dateTime.days = static_cast<uint8_t>(timeinfo.tm_mday);
  dateTime.hours = static_cast<uint8_t>(timeinfo.tm_hour);
  dateTime.minutes = static_cast<uint8_t>(timeinfo.tm_min);
  dateTime.seconds = static_cast<uint8_t>(timeinfo.tm_sec);
  dateTime.weekDay = convertSysTimeWdayToWeekDay(timeinfo.tm_wday);

  if (!dateTime.isValid())
  {
    ESP_LOGW(LOG_TAG, "Conversion from unixTime to DateTime resulted in invalid DateTime => using default.");
    dateTime = Constants::DEFAULT_DATETIME;
  }
}


uint8_t SystemTimeDriver::convertSysTimeYearToYear(int year) const
{
  /* tm_year int years since 1900 */
  /* Our 'year' is years since 2000 */
  uint8_t returnYear = 0U;

  if (year > 100)
  {
    returnYear = static_cast<uint8_t>(year - 100);
  }

  return returnYear;
}


int SystemTimeDriver::convertYearToSysTimeYear(uint8_t year) const
{
  return static_cast<int>(year + 100U);
}


uint8_t SystemTimeDriver::convertSysTimeMonToMonth(int mon) const
{
  /* tm_mon  int months since January  0-11 */
  /* Our 'month' is 1 for Jan, 2 for Feb ... */
  return static_cast<uint8_t>(mon + 1);
}


int SystemTimeDriver::convertMonthToSysTimeMon(uint8_t month) const
{
  return static_cast<int>(month - 1U);
}


WeekDay::Enum SystemTimeDriver::convertSysTimeWdayToWeekDay(int wday) const
{
  /* tm_wday  int days since Sunday 0-6 */
  /* Actually our implementation fits the sys/time.h implementation, so simple cast here */

  return static_cast<WeekDay::Enum>(wday);
}
