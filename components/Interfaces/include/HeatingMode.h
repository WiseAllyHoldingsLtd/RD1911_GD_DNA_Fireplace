#pragma once

struct HeatingMode
{
  enum Enum
  {
    /* These numbers are used in payload and radio, so do not mess with them! */
    eco = 0,
    comfort = 1,
    antiFrost = 2,
    off = 3
  };
};
