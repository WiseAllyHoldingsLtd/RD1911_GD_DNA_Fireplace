#pragma once
#include <cstdint>

struct FrameType  // Defined in Architecture Components_Appliance Cloud Interfaces v1.5
{
  enum Enum : uint16_t
  {
    FW_UPDATE_READY = 0x0001,
    FW_UPDATE_STATUS = 0x0002,
    APPLIANCE_PARAMS_REQUEST = 0x0020,
    APPLIANCE_PARAMS_RESPONSE = 0x0021,
    TIME_SYNC_REQUEST = 0x0007,
    TIME_SYNC_INFO = 0x0008,
    APPLIANCE_FORGET_ME = 0x0013,
    OPERATION_INFO = 0x0014,
    MULTIPLE_FRAME_REQUEST = 0x0022,
    MULTIPLE_FRAME_RESPONSE = 0x0023,
  };
};
