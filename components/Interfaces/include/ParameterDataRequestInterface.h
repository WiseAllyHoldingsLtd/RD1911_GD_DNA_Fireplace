#pragma once
#include <cstdint>

class ParameterDataRequestInterface
{
public:
  virtual bool isValid(void) const = 0;
  virtual uint8_t getNumOfParamDataItems(void) const = 0;
  virtual uint16_t getParamDataID(uint8_t paramDataIndex) const = 0;
  virtual uint8_t getParamDataSize(uint8_t paramDataIndex) const = 0;
  virtual uint8_t getParamData(uint8_t paramDataIndex, uint8_t * paramDataBytes, uint8_t size) const = 0;

  virtual ~ParameterDataRequestInterface(void) {}
};
