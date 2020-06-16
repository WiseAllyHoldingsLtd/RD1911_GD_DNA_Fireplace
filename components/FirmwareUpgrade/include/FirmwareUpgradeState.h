#pragma once

struct FirmwareUpgradeState
{
  enum Enum : uint32_t
  {
    notRequested,
    started,
    inProgress,
    completed,
    failed
  };
};
