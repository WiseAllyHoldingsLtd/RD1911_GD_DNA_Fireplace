#pragma once
#include <cstdint>


struct OverrideType
{
  enum Enum : uint8_t
  {
    none = 0U,     // no override, i.e. follow weekly schedule
    now = 1U,      // until next schedule change or 03:00 am
    constant = 2U, // until disabled
    timed = 3U,    // from now until a end time
  };
};
