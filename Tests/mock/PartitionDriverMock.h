#pragma once
#include "CppUTestExt\MockSupport.h"
#include "PartitionDriverInterface.h"

#define NAME(method) "PartitionDriverMock::" method


class PartitionDriverMock : public PartitionDriverInterface
{
public:
  virtual const char * getBootPartitionName(void) const
  {
    return mock().actualCall(NAME("getBootPartitionName")).returnStringValue();
  }

  virtual const char * getRunningPartitionName(void) const
  {
    return mock().actualCall(NAME("getRunningPartitionName")).returnStringValue();
  }

  virtual const char * getUpdatePartitionName(void) const
  {
    return mock().actualCall(NAME("getUpdatePartitionName")).returnStringValue();
  }

  virtual uint32_t getBootPartitionAddress(void) const
  {
    return mock().actualCall(NAME("getBootPartitionAddress")).returnUnsignedIntValue();
  }

  virtual uint32_t getRunningPartitionAddress(void) const
  {
    return mock().actualCall(NAME("getRunningPartitionAddress")).returnUnsignedIntValue();
  }

  virtual uint32_t getUpdatePartitionAddress(void) const
  {
    return mock().actualCall(NAME("getUpdatePartitionAddress")).returnUnsignedIntValue();
  }
};
