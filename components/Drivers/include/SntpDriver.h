#pragma once

#include "SntpDriverInterface.h"

class SntpDriver: public SntpDriverInterface
{
public:
  virtual bool runSntpOnce(const char *ntpUrl1,
                           const char *ntpUrl2,
                           uint64_t &sysTimeUnix);
  };

