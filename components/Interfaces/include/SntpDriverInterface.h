#pragma once

#include <cstdint>

class SntpDriverInterface
{
public:
  virtual ~SntpDriverInterface() { }
  virtual bool runSntpOnce(const char *ntpUrl1,
                           const char *ntpUrl2,
                           uint64_t &sysTimeUnix) = 0;
};
