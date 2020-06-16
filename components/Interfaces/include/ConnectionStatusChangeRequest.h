#pragma once
#include <cstdint>

struct ConnectionStatusChangeRequest
{
  enum Enum : uint8_t
  {
    noChange,
    bluetoothOff,
    bluetoothOn,
    fwUpgradeReady,
    disconnectEverything
  };
};
