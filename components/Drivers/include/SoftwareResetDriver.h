#pragma once

#include "SoftwareResetDriverInterface.h"


class SoftwareResetDriver : public SoftwareResetDriverInterface
{
public:
  SoftwareResetDriver(void);
  virtual void reset(void);
};
