
#include <cstdint>

struct FgtWifiStatus
{
  enum Enum : uint8_t
  {
    NotOperating              = 0x0u,
    ScanningInProgress        = 0x1u,
    ConnectionSuccessful      = 0x2u,
    SsidNotFound              = 0x3u,
    ConnectionRequestRejected = 0x4u,
    ConnectionFailed          = 0x5u
  };
};
