#include "Override.h"
#include "Constants.h"


Override::Override(void)
  : Override(Constants::DEFAULT_OVERRIDE_TYPE, Constants::DEFAULT_HEATING_MODE, Constants::DEFAULT_DATETIME)
{
}


Override::Override(OverrideType::Enum type, HeatingMode::Enum mode)
  : Override(type, mode, Constants::DEFAULT_DATETIME)
{
}

Override::Override(OverrideType::Enum type, HeatingMode::Enum mode, const DateTime & endDateTime)
  : m_type(type), m_mode(mode), m_endDateTime(endDateTime)
{
}

bool Override::operator==(const Override& rhs)
{
  return (m_type == rhs.m_type)
          && (m_mode == rhs.m_mode)
          && (m_endDateTime.year == rhs.m_endDateTime.year)
          && (m_endDateTime.month == rhs.m_endDateTime.month)
          && (m_endDateTime.days == rhs.m_endDateTime.days)
          && (m_endDateTime.hours == rhs.m_endDateTime.hours)
          && (m_endDateTime.minutes == rhs.m_endDateTime.minutes)
          && (m_endDateTime.seconds == rhs.m_endDateTime.seconds)
          && (m_endDateTime.weekDay == rhs.m_endDateTime.weekDay);
}
bool Override::operator!=(const Override& rhs)
{
  return !(*this == rhs);
}


OverrideType::Enum Override::getType(void) const
{
  return m_type;
}


HeatingMode::Enum Override::getMode(void) const
{
  return m_mode;
}


CloudTimerHeatingMode::Enum Override::getModeAsCloudEnum() const
{
  HeatingMode::Enum mode = getMode();
  CloudTimerHeatingMode::Enum cloudMode = CloudTimerHeatingMode::unknown;

  switch (mode)
  {
  case HeatingMode::comfort:
    cloudMode = CloudTimerHeatingMode::comfort;
    break;
  case HeatingMode::eco:
    cloudMode = CloudTimerHeatingMode::eco;
    break;
  case HeatingMode::antiFrost:
    cloudMode = CloudTimerHeatingMode::away;
    break;
  case HeatingMode::off:
    cloudMode = CloudTimerHeatingMode::shutdown;
    break;
  default:
    break;
  }

  return cloudMode;
}


bool Override::getEndDateTime(DateTime & dateTime) const
{
  bool isEndTimeDefined = false;
  dateTime = m_endDateTime;

  if (m_type == OverrideType::timed)
  {
    isEndTimeDefined = true;
  }

  return isEndTimeDefined;
}
