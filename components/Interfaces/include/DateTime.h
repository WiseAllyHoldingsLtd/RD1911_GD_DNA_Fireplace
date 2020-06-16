#pragma once
#include <stdint.h>

class WeekDay
{
public:
  enum Enum
  {
    MONDAY = 1,
    TUESDAY = 2,
    WEDNESDAY = 3,
    THURSDAY = 4,
    FRIDAY = 5,
    SATURDAY = 6,
    SUNDAY = 0
  };
};

class TimeStatus
{
public:
  enum Enum
  {
    OK = 0,
    TIMEOUT = 1,
    INTEGRITY_LOST = 2
  };
};

struct DateTime
{
  uint8_t year;
  uint8_t month;
  uint8_t days;
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
  WeekDay::Enum weekDay;
  
  void increaseMinute(uint8_t minute)
  {
    minutes += minute;
    minutes = minutes%60u;
  }
  void decreaseMinute(uint8_t minute)
  {
    // Minute and minutes are uint, we therefore needs to avoid underflow
    if (minute > minutes)
    {
      minutes += 60u;
    }
    minutes -= minute;
    minutes = minutes%60u;
  }
  
  void increaseHour(uint8_t hour)
  {
    hours += hour;
    hours = hours%24u;
  }
  void decreaseHour(uint8_t hour)
  {
    // Hour and hours are uint, we therefore needs to avoid underflow
    if (hour > hours)
    {
      hours += 24u;
    }
    hours -= hour;
    hours = hours%24u;
  }

  void nextDay()
  {
    weekDay = static_cast<WeekDay::Enum>((static_cast<int16_t>(weekDay)+1) % 7);
  }
  void previousDay()
  {
    // -1 % 7 becomes -1 which is not what we want, so instead of subtracting 1, we add 6.
    weekDay = static_cast<WeekDay::Enum>((static_cast<int16_t>(weekDay)+6) % 7);
  }

  bool isValid() const
  {
    bool validate = (year < 100U)  // RTC use only two digits for year
                    && ((month > 0U) && (month < 13U))
                    && ((days > 0U) && (days < 32U))  // not complete check for days per month...
                    && (hours < 24U)
                    && (minutes < 60U)
                    && (seconds < 60)
                    && (static_cast<uint8_t>(weekDay) < 7U);

    if (validate)
    {
      // extra check for days in a month
      if ((month == 4U) || (month == 6U) || (month == 9U) || (month == 11U))
      {
        validate = (days < 31U);
      }
      else if (month == 2U)
      {
        if ((year % 4U) == 0U)
        {
          // cannot decide leap year correctly as long as we have only two digits for year,
          // but next day which would give false positive is 2100-02-29...
          validate = (days < 30U);
        }
        else
        {
          validate = (days < 29U);
        }
      }
    }

    return validate;
  }
};
