#pragma once


class SoftwareResetDriverInterface
{
public:
  virtual void reset(void) = 0;

  virtual ~SoftwareResetDriverInterface() {}
};
