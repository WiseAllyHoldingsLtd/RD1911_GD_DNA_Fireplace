#pragma once
#include <cstdint>

#define FRAME_MAX_SIZE 2048U // TODO: Move to constants


class FrameParser
{
public:
  FrameParser(void);
  FrameParser(const uint8_t * bytes, uint32_t size);
  FrameParser(const FrameParser& other);
  FrameParser& operator=( const FrameParser& rhs );
  ~FrameParser(void);

  uint32_t getSize(void) const;
  uint16_t getID(void) const;

  uint8_t getUInt8(uint32_t offset) const;
  uint16_t getUInt16AsLSB(uint32_t offset) const;
  uint32_t getUInt32AsLSB(uint32_t offset) const;
  int32_t getInt32AsLSB(uint32_t offset) const;
  uint16_t getUInt16AsMSB(uint32_t offset) const;
  uint32_t getUInt32AsMSB(uint32_t offset) const;
  int32_t getInt32AsMSB(uint32_t offset) const;
  uint16_t getBytes(uint32_t offset, uint8_t * buffer, uint16_t size) const;

private:
  bool isWithinLimits(uint32_t offset, uint16_t size) const;

  uint32_t m_size;
  uint8_t * m_bytes;

};
