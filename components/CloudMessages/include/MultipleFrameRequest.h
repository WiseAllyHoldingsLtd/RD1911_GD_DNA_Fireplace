#pragma once

#include "FrameParser.h"


class MultipleFrameRequest
{
public:
  MultipleFrameRequest(const FrameParser & frame);
  bool isValid(void) const;

  uint16_t getNumOfPayloadFrames(void) const;
  uint8_t getPayloadFrameSize(uint16_t frameIndex) const;
  uint8_t getPayloadFrame(uint16_t frameIndex, uint8_t * payloadFrameBytes, uint8_t size) const;

private:
  void validate(void);
  uint16_t getSizeBytePos(uint16_t payloadIndex) const;


  const FrameParser & m_frame;
  bool m_isValid;
};
