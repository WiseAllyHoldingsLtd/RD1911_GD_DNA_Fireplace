#pragma once

#include "CppUTestExt\MockSupport.h"
#include "RtcMock.h"
#include "SystemTimeDriverInterface.h"

#define NAME(method) "SystemTimeDriverMock::" method


class SystemTimeDriverMock : public SystemTimeDriverInterface
{
  virtual void setUnixTime(uint32_t unixTime)
  {
    mock().actualCall(NAME("setUnixTime")).withUnsignedIntParameter("unixTime", unixTime);
  }

  virtual uint32_t getUnixTime(void) const
  {
    return mock().actualCall(NAME("getUnixTime")).returnUnsignedIntValue();
  }

  virtual uint32_t getUnixTimeFromDateTime(const DateTime & dateTime) const
  {
    return mock().actualCall(NAME("getUnixTimeFromDateTime"))
        .withParameterOfType("DateTime", "dateTime", (void*)&dateTime)
        .returnUnsignedIntValue();
  }

  virtual void getDateTimeFromUnixTime(uint32_t unixTime, DateTime & dateTime) const
  {
    mock().actualCall(NAME("getDateTimeFromUnixTime"))
        .withUnsignedIntParameter("unixTime", unixTime)
        .withOutputParameterOfType("DateTime", "dateTime", (void*)&dateTime);
  }
};
