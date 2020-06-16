#pragma once

struct CloudStatus
{
  enum Enum : uint32_t  // 32-bit system, read/write should be atomic
  {
    Idle = 0,
    Connecting,
    Connected,
    Stopped,
    Error,
  };
};
