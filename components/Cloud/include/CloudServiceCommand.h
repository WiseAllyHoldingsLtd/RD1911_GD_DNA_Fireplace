
#pragma once

struct CloudServiceCommand
{
  enum Enum : uint8_t
  {
    PAUSE,
    UNPAUSE,
  };

  Enum type;
};
