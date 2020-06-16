#pragma once
#include <cstdint>
#include "FrameBuilderUser.h"


struct FirmwareUpdateResult
{
  enum Enum : uint8_t
  {
    /* These numbers are used in cloud comm., so do not mess with them! */
    success = 0x00,
    authError = 0x01,
    retryError = 0x02
  };
};


class FirmwareUpdateStatus : public FrameBuilderUser
{
public:
  FirmwareUpdateStatus(void);
  bool addData(FirmwareUpdateResult::Enum status, uint8_t major, uint8_t minor, uint8_t test, uint16_t numOfRetries);
};
