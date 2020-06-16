#pragma once

#include "CppUTestExt\MockSupport.h"
#include "ParameterDataRequestInterface.h"

#define NAME(method) "ParameterDataRequestMock::" method


class ParameterDataRequestMock : public ParameterDataRequestInterface
{
public:
  virtual bool isValid(void) const
  {
    return mock().actualCall(NAME("isValid")).returnBoolValue();
  }

  virtual uint8_t getNumOfParamDataItems(void) const
  {
    return mock().actualCall(NAME("getNumOfParamDataItems")).returnUnsignedIntValue();
  }

  virtual uint16_t getParamDataID(uint8_t paramDataIndex) const
  {
    return mock().actualCall(NAME("getParamDataID"))
        .withUnsignedIntParameter("paramDataIndex", paramDataIndex)
        .returnUnsignedIntValue();
  }

  virtual uint8_t getParamDataSize(uint8_t paramDataIndex) const
  {
    return mock().actualCall(NAME("getParamDataSize"))
        .withUnsignedIntParameter("paramDataIndex", paramDataIndex)
        .returnUnsignedIntValue();
  }

  virtual uint8_t getParamData(uint8_t paramDataIndex, uint8_t * paramDataBytes, uint8_t size) const
  {
    return mock().actualCall(NAME("getParamData"))
        .withUnsignedIntParameter("paramDataIndex", paramDataIndex)
        .withOutputParameter("paramDataBytes", static_cast<void*>(paramDataBytes))
        .withUnsignedIntParameter("size", size)
        .returnUnsignedIntValue();
  }
};
