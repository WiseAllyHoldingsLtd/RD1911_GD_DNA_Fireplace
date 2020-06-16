#pragma once

#include "SystemTimeDriverInterface.h"


class SystemTimeDriver : public SystemTimeDriverInterface
{
public:
  SystemTimeDriver(void);

  virtual void setUnixTime(uint32_t unixTime);
  virtual uint32_t getUnixTime(void) const;

  virtual uint32_t getUnixTimeFromDateTime(const DateTime & dateTime) const;
  virtual void getDateTimeFromUnixTime(uint32_t unixTime, DateTime & dateTime) const;

private:
  uint8_t convertSysTimeYearToYear(int year) const;
  int convertYearToSysTimeYear(uint8_t year) const;

  uint8_t convertSysTimeMonToMonth(int mon) const;
  int convertMonthToSysTimeMon(uint8_t month) const;

  WeekDay::Enum convertSysTimeWdayToWeekDay(int wday) const;
};
