#pragma once

#include <cstdint>
#include "DateTime.h"


class SystemTimeDriverInterface
{
public:
  virtual ~SystemTimeDriverInterface(void) {}

  virtual void setUnixTime(uint32_t unixTime) = 0;
  virtual uint32_t getUnixTime(void) const = 0;

  virtual uint32_t getUnixTimeFromDateTime(const DateTime & dateTime) const = 0;
  virtual void getDateTimeFromUnixTime(uint32_t unixTime, DateTime & dateTime) const = 0;
};
