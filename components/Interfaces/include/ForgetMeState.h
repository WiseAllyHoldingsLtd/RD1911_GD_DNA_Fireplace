#pragma once

struct ForgetMeState
{
  enum Enum : uint8_t
  {
    noResetRequested = 0u,
    resetRequested = 1u,
    resetConfirmed = 2u
  };
};
