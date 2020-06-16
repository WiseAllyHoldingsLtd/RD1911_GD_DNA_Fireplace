#include "FrameBuilder.h"
#include <cstring>


namespace
{
  const uint32_t FRAMESIZE_MAX = 1024U;
}


FrameBuilder::FrameBuilder()
{
  reset();
}


void FrameBuilder::reset(void)
{
  m_size = 0U;
}

bool FrameBuilder::addUInt8(uint8_t value)
{
  bool isSpace = isSufficientSpace(sizeof(value));

  if (isSpace)
  {
    addByte(value);
  }

  return isSpace;
}


bool FrameBuilder::addUInt16AsMSB(uint16_t value)
{
  bool isSpace = isSufficientSpace(sizeof(value));

  if (isSpace)
  {
    addByte(static_cast<uint8_t>(value >> 8U));
    addByte(static_cast<uint8_t>(value & 0xffU));
  }

  return isSpace;
}

bool FrameBuilder::addUInt16AsLSB(uint16_t value)
{
  bool isSpace = isSufficientSpace(sizeof(value));

  if (isSpace)
  {
    addByte(static_cast<uint8_t>(value & 0xffU));
    addByte(static_cast<uint8_t>(value >> 8U));
  }

  return isSpace;
}


bool FrameBuilder::addBytes(const uint8_t * bytes, uint32_t size)
{
  bool isSpace = isSufficientSpace(size);

  if (isSpace)
  {
    memcpy(&m_bytes[m_size], bytes, size);
    m_size += size;
  }

  return isSpace;
}


bool FrameBuilder::modifyUInt8(uint32_t index, uint8_t newValue)
{
  bool wasModified = false;

  if (getSize() > index)
  {
    uint32_t originalSize = m_size;
    m_size = index;
    wasModified = addUInt8(newValue);
    m_size = originalSize;
  }

  return wasModified;
}


bool FrameBuilder::modifyUInt16AsMSB(uint32_t index, uint16_t newValue)
{
  bool wasModified = false;

  if (getSize() > (index + 1U))
  {
    uint32_t originalSize = m_size;
    m_size = index;
    wasModified = addUInt16AsMSB(newValue);
    m_size = originalSize;
  }

  return wasModified;
}


bool FrameBuilder::modifyUInt16AsLSB(uint32_t index, uint16_t newValue)
{
  bool wasModified = false;

  if (getSize() > (index + 1U))
  {
    uint32_t originalSize = m_size;
    m_size = index;
    wasModified = addUInt16AsLSB(newValue);
    m_size = originalSize;
  }

  return wasModified;
}


uint32_t FrameBuilder::getSize(void) const
{
  return m_size;
}


uint32_t FrameBuilder::getBytes(uint8_t buffer[], uint32_t size) const
{
  uint32_t bytesCopied = 0U;

  if (size > 0U)
  {
    if (size >= m_size)
    {
      memcpy(buffer, m_bytes, m_size);
      bytesCopied = m_size;
    }
    else
    {
      memcpy(buffer, m_bytes, size);
      bytesCopied = size;
    }
  }

  return bytesCopied;
}


bool FrameBuilder::isSufficientSpace(uint32_t numOfBytes) const
{
  return ((numOfBytes + m_size) <= FRAMESIZE_MAX);
}


void FrameBuilder::addByte(uint8_t value)
{
  m_bytes[m_size] = value;
  ++m_size;
}
