#pragma once
#include <cstdint>
#include "Constants.h"

class FrameBuilder
{
public:
  FrameBuilder(void);
  void reset(void);

  bool addUInt8(uint8_t value);
  bool addUInt16AsMSB(uint16_t value);
  bool addUInt16AsLSB(uint16_t value);
  bool addBytes(const uint8_t * bytes, uint32_t size);

  bool modifyUInt8(uint32_t index, uint8_t newValue);
  bool modifyUInt16AsMSB(uint32_t index, uint16_t newValue);
  bool modifyUInt16AsLSB(uint32_t index, uint16_t newValue);

  uint32_t getSize(void) const;
  uint32_t getBytes(uint8_t buffer[], uint32_t size) const;

private:
  bool isSufficientSpace(uint32_t numOfBytes) const;
  void addByte(uint8_t value);

  uint32_t m_size;
  uint8_t m_bytes[Constants::CLOUD_RESPONSE_MSG_MAX_SIZE];
};
