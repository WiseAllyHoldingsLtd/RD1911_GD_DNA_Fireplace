#include "WeekProgramNode.h"


uint16_t WeekProgramNode::getBestFittingMinutesIntoWeek(DateTime time) const
{
  uint8_t weekDayValue = 0u;

  bool hasTimePassedNodeTime = hasTimePassed(time);

  if (static_cast<uint16_t>(interval) == static_cast<uint16_t>(Interval::INTERVAL_MONSUN))
  {
    weekDayValue = getBestFittingWeekDayValueForIntervalMonSun(time, hasTimePassedNodeTime);
  }
  else if (static_cast<uint16_t>(interval) == static_cast<uint16_t>(Interval::INTERVAL_MONFRI))
  {
    weekDayValue = getBestFittingWeekDayValueForIntervalMonFri(time, hasTimePassedNodeTime);
  }
  else if (static_cast<uint16_t>(interval) == static_cast<uint16_t>(Interval::INTERVAL_SATSUN))
  {
    weekDayValue = getBestFittingWeekDayValueForIntervalSatSun(time, hasTimePassedNodeTime);

  }
  else
  {
    weekDayValue = convertIntervalEnumToWeekDayValue(interval);
  }

  // We need to make monday = 0, and sunday = 6
  weekDayValue = adjustWeekDayValueFromEnumsToZeroIndex(weekDayValue);

  return static_cast<uint16_t>(weekDayValue)* 1440u +
    static_cast<uint16_t>(hours)* 60u +
    static_cast<uint16_t>(minutes);
}

/* This function gives us the num of minutes from monday 00:00 till the nodes's time.
 * If the node has an interval, ie. MON-FRI 12:00, and the node's start time on monday has passed (12:01++)
 * then the returned time (in minutes) are Tuesday 12:00.
 * DateTime struct has Sunday = 0, Monday = 1 etc.
 * While the algorithm that uses this function needs monday to be 0 and sunday = 6
 */
uint16_t WeekProgramNode::getNumOfMinIntoWeekForThisNode(DateTime time) const
{
  uint8_t weekDayValue;

  bool hasTimePassedNodeTime = hasTimePassed(time);

  if (static_cast<uint16_t>(interval) == static_cast<uint16_t>(Interval::INTERVAL_MONSUN))
  {
    weekDayValue = getWeekDayValueForIntervalMonSun(time, hasTimePassedNodeTime);
  }
  else if (static_cast<uint16_t>(interval) == static_cast<uint16_t>(Interval::INTERVAL_MONFRI))
  {
    weekDayValue = getWeekDayValueForIntervalMonFri(time, hasTimePassedNodeTime);
  }
  else if (static_cast<uint16_t>(interval) == static_cast<uint16_t>(Interval::INTERVAL_SATSUN))
  {
    weekDayValue = getWeekDayValueForIntervalSatSun(time, hasTimePassedNodeTime);
  }
  else
  {
    weekDayValue = convertIntervalEnumToWeekDayValue(interval);
  }


  // We need to make monday = 0, and sunday = 6
  weekDayValue = adjustWeekDayValueFromEnumsToZeroIndex(weekDayValue);

  return static_cast<uint16_t>(weekDayValue)* 1440u +
    static_cast<uint16_t>(hours)* 60u +
    static_cast<uint16_t>(minutes);
}



uint8_t WeekProgramNode::buildBitMap()
{
  uint8_t bm;
  if (interval == static_cast<int16_t>(Interval::INTERVAL_MONSUN))
  {
    /* All but the highest bit */
    bm = 0x7Fu;
  }
  else if (interval == static_cast<int16_t>(Interval::INTERVAL_MONFRI))
  {
    /* All but the highest three bits */
    bm = 0x1Fu;
  }
  else if (interval == static_cast<int16_t>(Interval::INTERVAL_SATSUN))
  {
    /* Only bit 5 and 6 */
    bm = 0x60u;
  }
  else
  {
    /* INTERVAL_MON might not be 0 in the enum so we make it so. We do assume,
    though, that INTERVAL_MON+1u==INTERVAL_TUE etc. */
    uint8_t val = static_cast<uint8_t>(interval)-Interval::INTERVAL_MON;
    bm = (1u << val);
  }
  return bm;
}

uint8_t WeekProgramNode::convertIntervalEnumToWeekDayValue(Interval::Enum inInterval) const
{
  uint8_t weekDayValue = 0u;
  switch (inInterval)
  {
  case Interval::INTERVAL_MON:
    weekDayValue = static_cast<uint8_t>(WeekDay::MONDAY);
    break;
  case Interval::INTERVAL_TUE:
    weekDayValue = static_cast<uint8_t>(WeekDay::TUESDAY);
    break;
  case Interval::INTERVAL_WED:
    weekDayValue = static_cast<uint8_t>(WeekDay::WEDNESDAY);
    break;
  case Interval::INTERVAL_THU:
    weekDayValue = static_cast<uint8_t>(WeekDay::THURSDAY);
    break;
  case Interval::INTERVAL_FRI:
    weekDayValue = static_cast<uint8_t>(WeekDay::FRIDAY);
    break;
  case Interval::INTERVAL_SAT:
    weekDayValue = static_cast<uint8_t>(WeekDay::SATURDAY);
    break;
  case Interval::INTERVAL_SUN:
    weekDayValue = static_cast<uint8_t>(WeekDay::SUNDAY);
    break;
  default:
    weekDayValue = static_cast<uint8_t>(WeekDay::MONDAY);
    break;
  }
  return weekDayValue;
}

uint8_t WeekProgramNode::getWeekDayValueForIntervalMonSun(const DateTime &time, bool hasTimePassedNodeTime) const
{
  uint8_t weekDayValue = 0u;
  if (hasTimePassedNodeTime)
  {
    switch (time.weekDay)
    {
    case WeekDay::MONDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::TUESDAY);
      break;
    case WeekDay::TUESDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::WEDNESDAY);
      break;
    case WeekDay::WEDNESDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::THURSDAY);
      break;
    case WeekDay::THURSDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::FRIDAY);
      break;
    case WeekDay::FRIDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::SATURDAY);
      break;
    case WeekDay::SATURDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::SUNDAY);
      break;
    case WeekDay::SUNDAY:
    default:
      weekDayValue = static_cast<uint8_t>(WeekDay::MONDAY);
      break;
    }

  }
  else
  {
    switch (time.weekDay)
    {
    case WeekDay::MONDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::MONDAY);
      break;
    case WeekDay::TUESDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::TUESDAY);
      break;
    case WeekDay::WEDNESDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::WEDNESDAY);
      break;
    case WeekDay::THURSDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::THURSDAY);
      break;
    case WeekDay::FRIDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::FRIDAY);
      break;
    case WeekDay::SATURDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::SATURDAY);
      break;
    case WeekDay::SUNDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::SUNDAY);
      break;
    default:
      weekDayValue = static_cast<uint8_t>(WeekDay::MONDAY);
      break;
    }
  }
  return weekDayValue;
}

uint8_t WeekProgramNode::getWeekDayValueForIntervalMonFri(const DateTime &time, bool hasTimePassedNodeTime) const
{
  uint8_t weekDayValue = 0u;
  if (hasTimePassedNodeTime)
  {
    switch (time.weekDay)
    {
    case WeekDay::MONDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::TUESDAY);
      break;
    case WeekDay::TUESDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::WEDNESDAY);
      break;
    case WeekDay::WEDNESDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::THURSDAY);
      break;
    case WeekDay::THURSDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::FRIDAY);
      break;
    case WeekDay::FRIDAY:
    case WeekDay::SATURDAY:
    case WeekDay::SUNDAY:
    default:
      weekDayValue = static_cast<uint8_t>(WeekDay::MONDAY);
      break;
    }
  }
  else
  {
    switch (time.weekDay)
    {
    case WeekDay::MONDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::MONDAY);
      break;
    case WeekDay::TUESDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::TUESDAY);
      break;
    case WeekDay::WEDNESDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::WEDNESDAY);
      break;
    case WeekDay::THURSDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::THURSDAY);
      break;
    case WeekDay::FRIDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::FRIDAY);
      break;
    case WeekDay::SATURDAY:
    case WeekDay::SUNDAY:
    default:
      weekDayValue = static_cast<uint8_t>(WeekDay::MONDAY);
      break;
    }
  }
  return weekDayValue;
}
uint8_t WeekProgramNode::getWeekDayValueForIntervalSatSun(const DateTime &time, bool hasTimePassedNodeTime) const
{
  uint8_t weekDayValue = 0u;
  if (time.weekDay == static_cast<int16_t>(WeekDay::SATURDAY))
  {
    if (hasTimePassedNodeTime) //time is saturday and has already passed node's starting time, so we aim towards sunday
    {
      weekDayValue = static_cast<uint8_t>(WeekDay::SUNDAY);
    }
    else // it has not passed, so we aim towards saturday
    {
      weekDayValue = static_cast<uint8_t>(WeekDay::SATURDAY);
    }
  }
  else if (time.weekDay == static_cast<int16_t>(WeekDay::SUNDAY))
  {
    if (hasTimePassedNodeTime) //time is sunday and has already passed node's starting time, so we aim towards saturday "next week" instead
    {
      weekDayValue = static_cast<uint8_t>(WeekDay::SATURDAY);
    }
    else // it has not passed, so we aim towards sunday
    {
      weekDayValue = static_cast<uint8_t>(WeekDay::SUNDAY);
    }
  }
  else // monday - friday
  {
    weekDayValue = static_cast<uint8_t>(WeekDay::SATURDAY);
  }
  return weekDayValue;
}

uint8_t WeekProgramNode::getBestFittingWeekDayValueForIntervalSatSun(const DateTime &time, bool hasTimePassedNodeTime) const
{
  uint8_t weekDayValue = 0u;
  if (time.weekDay == static_cast<int16_t>(WeekDay::SATURDAY))
  {
    if (hasTimePassedNodeTime) //time is saturday and has already passed node's starting time, so we aim towards sunday
    {
      weekDayValue = static_cast<uint8_t>(WeekDay::SATURDAY);
    }
    else // it has not passed
    {
      weekDayValue = static_cast<uint8_t>(WeekDay::SUNDAY);
    }
  }
  else if (time.weekDay == static_cast<int16_t>(WeekDay::SUNDAY))
  {
    if (hasTimePassedNodeTime) //time is sunday and has already passed node's starting time
    {
      weekDayValue = static_cast<uint8_t>(WeekDay::SUNDAY);
    }
    else // it has not passed
    {
      weekDayValue = static_cast<uint8_t>(WeekDay::SATURDAY);
    }
  }
  else // monday - friday
  {
    weekDayValue = static_cast<uint8_t>(WeekDay::SUNDAY);
  }
  return weekDayValue;
}
uint8_t WeekProgramNode::getBestFittingWeekDayValueForIntervalMonSun(const DateTime &time, bool hasTimePassedNodeTime) const
{
  uint8_t weekDayValue = 0u;
  if (hasTimePassedNodeTime)
  {
    switch (time.weekDay)
    {
    case WeekDay::MONDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::MONDAY);
      break;
    case WeekDay::TUESDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::TUESDAY);
      break;
    case WeekDay::WEDNESDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::WEDNESDAY);
      break;
    case WeekDay::THURSDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::THURSDAY);
      break;
    case WeekDay::FRIDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::FRIDAY);
      break;
    case WeekDay::SATURDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::SATURDAY);
      break;
    case WeekDay::SUNDAY:
    default:
      weekDayValue = static_cast<uint8_t>(WeekDay::SUNDAY);
      break;
    }

  }
  else
  {
    switch (time.weekDay)
    {
    case WeekDay::MONDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::SUNDAY);
      break;
    case WeekDay::TUESDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::MONDAY);
      break;
    case WeekDay::WEDNESDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::TUESDAY);
      break;
    case WeekDay::THURSDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::WEDNESDAY);
      break;
    case WeekDay::FRIDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::THURSDAY);
      break;
    case WeekDay::SATURDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::FRIDAY);
      break;
    case WeekDay::SUNDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::SATURDAY);
      break;
    default:
      weekDayValue = static_cast<uint8_t>(WeekDay::SUNDAY);
      break;
    }
  }

  return weekDayValue;
}
uint8_t WeekProgramNode::getBestFittingWeekDayValueForIntervalMonFri(const DateTime &time, bool hasTimePassedNodeTime) const
{
  uint8_t weekDayValue = 0u;
  if (hasTimePassedNodeTime)
  {
    switch (time.weekDay)
    {
    case WeekDay::MONDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::MONDAY);
      break;
    case WeekDay::TUESDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::TUESDAY);
      break;
    case WeekDay::WEDNESDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::WEDNESDAY);
      break;
    case WeekDay::THURSDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::THURSDAY);
      break;
    case WeekDay::FRIDAY:
    case WeekDay::SATURDAY:
    case WeekDay::SUNDAY:
    default:
      weekDayValue = static_cast<uint8_t>(WeekDay::FRIDAY);
      break;
    }
  }
  else
  {
    switch (time.weekDay)
    {
    case WeekDay::MONDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::FRIDAY);
      break;
    case WeekDay::TUESDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::MONDAY);
      break;
    case WeekDay::WEDNESDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::TUESDAY);
      break;
    case WeekDay::THURSDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::WEDNESDAY);
      break;
    case WeekDay::FRIDAY:
      weekDayValue = static_cast<uint8_t>(WeekDay::THURSDAY);
      break;
    case WeekDay::SATURDAY:
    case WeekDay::SUNDAY:
    default:
      weekDayValue = static_cast<uint8_t>(WeekDay::FRIDAY);
      break;
    }
  }
  return weekDayValue;
}

bool WeekProgramNode::hasTimePassed(DateTime time) const
{
  bool hasTimePassedNodeTime = false;
  if ((time.hours > hours) ||
    ((time.hours == hours) && (time.minutes >= minutes)))
  {
    hasTimePassedNodeTime = true; // dateTime has passed this nodes time (only applies to minutes and hours)
  }
  return hasTimePassedNodeTime;
}

uint8_t WeekProgramNode::adjustWeekDayValueFromEnumsToZeroIndex(uint8_t weekDayValue) const
{
  if (weekDayValue == static_cast<uint8_t>(WeekDay::SUNDAY))
  {
    weekDayValue = 6u;
  }
  else
  {
    weekDayValue--;
  }
  return weekDayValue;
}


bool WeekProgramNode::operator !=(const WeekProgramNode &rhs) const
{
  return
    !((interval == rhs.interval) &&
    (status == rhs.status) &&
    (hours == rhs.hours) &&
    (minutes == rhs.minutes));

}
