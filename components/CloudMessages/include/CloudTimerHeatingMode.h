#pragma once

struct CloudTimerHeatingMode
{
  enum Enum
  {
    /* These numbers are used in Azure cloud communication, so do not mess with them! */
    away = 0,
    eco = 1,
    comfort = 2,
    shutdown = 3,
    unknown = 4
  };
};

struct CloudTimerHeatingModeExt
{
  enum Enum
  {
    /* These numbers are used in Azure cloud communication, so do not mess with them! */
    away = 0,
    eco = 1,
    comfort = 2,
    shutdown = 3,
    standby = 4
  };
};
