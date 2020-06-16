#pragma once

#include "CppUTestExt\MockSupport.h"
#include "OTADriverInterface.h"

#define NAME(method) "OTADriverMock::" method


class OTADriverMock : public OTADriverInterface
{
public:
  virtual void resetAndInit(void)
  {
    mock().actualCall(NAME("resetAndInit"));
  }


  virtual bool performSelfTest(void) const
  {
    return mock().actualCall(NAME("performSelfTest")).returnBoolValueOrDefault(true);
  }


  virtual bool beginUpdate(void)
  {
    return mock().actualCall(NAME("beginUpdate")).returnBoolValueOrDefault(true);
  }


  virtual bool endUpdate(void)
  {
    return mock().actualCall(NAME("endUpdate")).returnBoolValueOrDefault(true);
  }


  virtual bool write(const char * data, uint32_t size)
  {
    return mock().actualCall(NAME("write"))
        .withMemoryBufferParameter("data", reinterpret_cast<const uint8_t*>(data), size)
        .withUnsignedIntParameter("size", size)
        .returnBoolValueOrDefault(true);
  }


  virtual bool activateUpdatedPartition(void)
  {
    return mock().actualCall(NAME("activateUpdatedPartition")).returnBoolValueOrDefault(true);
  }
};
