#pragma once
#include "OverrideType.h"
#include "HeatingMode.h"
#include "CloudTimerHeatingMode.h"
#include "DateTime.h"

class Override
{
public:
  Override(void);
  Override(OverrideType::Enum type, HeatingMode::Enum mode);
  Override(OverrideType::Enum type, HeatingMode::Enum mode, const DateTime & endDateTime);

  Override(const Override& other) = default;
  Override& operator=(const Override& rhs) = default;
  bool operator==(const Override& rhs);
  bool operator!=(const Override& rhs);

  OverrideType::Enum getType(void) const;
  HeatingMode::Enum getMode(void) const;
  CloudTimerHeatingMode::Enum getModeAsCloudEnum(void) const;
  bool getEndDateTime(DateTime & dateTime) const;

private:
  OverrideType::Enum m_type;
  HeatingMode::Enum m_mode;
  DateTime m_endDateTime;
};
