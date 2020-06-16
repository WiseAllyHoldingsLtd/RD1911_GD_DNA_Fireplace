#pragma once

#include "FrameParser.h"
#include "ParameterDataRequestInterface.h"


class ParameterDataRequest : public ParameterDataRequestInterface
{
public:
  ParameterDataRequest(const FrameParser & frame);
  virtual bool isValid(void) const;

  virtual uint8_t getNumOfParamDataItems(void) const;
  virtual uint16_t getParamDataID(uint8_t paramDataIndex) const;
  virtual uint8_t getParamDataSize(uint8_t paramDataIndex) const;
  virtual uint8_t getParamData(uint8_t paramDataIndex, uint8_t * paramDataBytes, uint8_t size) const;

private:
  void validate(void);
  uint8_t getSizeBytePos(uint8_t payloadIndex) const;

  const FrameParser & m_frame;
  uint8_t m_numOfParamData;
};
