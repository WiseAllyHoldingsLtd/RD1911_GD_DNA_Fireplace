#pragma once

#include "HeatingMode.h"
#include "DateTime.h"
#include <stdint.h>

#define NODES_MAX           80u

struct Interval
{
  enum Enum
  {
    INTERVAL_MONSUN = 0u,
    INTERVAL_MONFRI = 1u,
    INTERVAL_SATSUN = 2u,
    INTERVAL_MON = 3u,
    INTERVAL_TUE = 4u,
    INTERVAL_WED = 5u,
    INTERVAL_THU = 6u,
    INTERVAL_FRI = 7u,
    INTERVAL_SAT = 8u,
    INTERVAL_SUN = 9u
  };
};


struct WeekProgramNode
{
public:
  WeekProgramNode(){};

  WeekProgramNode(Interval::Enum inInterval, HeatingMode::Enum inStatus, uint8_t inHours, uint8_t inMinutes):
    interval(inInterval),
    status(inStatus),
    hours(inHours),
    minutes(inMinutes)
  {
  };

  Interval::Enum interval;
  HeatingMode::Enum status;
  uint8_t hours;
  uint8_t minutes;

  uint16_t getBestFittingMinutesIntoWeek(DateTime time) const;
  uint16_t getNumOfMinIntoWeekForThisNode(DateTime time) const;

  /* Builds a bit map of the given interval. Bit 0 corresponds to monday, bit 1
  to tuesday etc.
  Ex: INTERVAL_MON: 0b00000001, INTERVAL_MONSUN: 0b01111111 */
  uint8_t buildBitMap();

  bool operator !=(const WeekProgramNode &rhs) const;
private:
  uint8_t adjustWeekDayValueFromEnumsToZeroIndex(uint8_t weekDayValue) const;
  bool hasTimePassed(DateTime time) const;

  uint8_t convertIntervalEnumToWeekDayValue(Interval::Enum inInterval) const;
  uint8_t getWeekDayValueForIntervalMonSun(const DateTime &time, bool hasTimePassedNodeTime) const;
  uint8_t getWeekDayValueForIntervalMonFri(const DateTime &time, bool hasTimePassedNodeTime) const;
  uint8_t getWeekDayValueForIntervalSatSun(const DateTime &time, bool hasTimePassedNodeTime) const;

  uint8_t getBestFittingWeekDayValueForIntervalSatSun(const DateTime &time, bool hasTimePassedNodeTime) const;
  uint8_t getBestFittingWeekDayValueForIntervalMonSun(const DateTime &time, bool hasTimePassedNodeTime) const;
  uint8_t getBestFittingWeekDayValueForIntervalMonFri(const DateTime &time, bool hasTimePassedNodeTime) const;
};
