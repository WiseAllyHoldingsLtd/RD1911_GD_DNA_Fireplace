#include "FrameParser.h"

#include <cstring>
#include <cstdlib>


FrameParser::FrameParser(void)
  : m_size(0U), m_bytes(nullptr)
{
}

FrameParser::FrameParser(const uint8_t * bytes, uint32_t size)
{
  if (size > 0U && size < FRAME_MAX_SIZE)
  {
    m_bytes = new uint8_t[size];

    if (m_bytes != nullptr)
    {
      memcpy(m_bytes, bytes, size);
      m_size = size;
    }
    else
    {
      m_size = 0U;
    }
  }
  else
  {
    m_bytes = nullptr;
    m_size = 0U;
  }
}

FrameParser::FrameParser(const FrameParser& other)
{
  if (other.m_size > 0U)
  {
    m_bytes = new uint8_t[other.m_size];

    if (m_bytes != nullptr)
    {
      memcpy(m_bytes, other.m_bytes, other.m_size);
      m_size = other.m_size;
    }
    else
    {
      m_size = 0U;
    }
  }
  else
  {
    m_bytes = nullptr;
    m_size = 0U;
  }
}

FrameParser& FrameParser::operator=( const FrameParser& rhs )
{
  if (rhs.m_size > 0U)
  {
    m_bytes = new uint8_t[rhs.m_size];

    if (m_bytes != nullptr)
    {
      memcpy(m_bytes, rhs.m_bytes, rhs.m_size);
      m_size = rhs.m_size;
    }
    else
    {
      m_size = 0U;
    }
  }
  else
  {
    m_bytes = nullptr;
    m_size = 0U;
  }

  return *this;
}


FrameParser::~FrameParser(void)
{
  if (m_size > 0U)
  {
    delete [] m_bytes;
    m_bytes = nullptr;
    m_size = 0U;
  }
}


uint32_t FrameParser::getSize(void) const
{
  return m_size;
}


uint16_t FrameParser::getID(void) const
{
  return getUInt16AsMSB(0U);
}


uint8_t FrameParser::getUInt8(uint32_t offset) const
{
  uint8_t returnVal = 0U;

  if (isWithinLimits(offset, 1U))
  {
    returnVal = m_bytes[offset];
  }

  return returnVal;
}


uint16_t FrameParser::getUInt16AsLSB(uint32_t offset) const
{
  uint16_t returnVal = 0U;

  if (isWithinLimits(offset, 2U))
  {
    returnVal = ((m_bytes[offset + 1U] << 8U) + m_bytes[offset]);
  }

  return returnVal;
}


uint32_t FrameParser::getUInt32AsLSB(uint32_t offset) const
{
  uint32_t returnVal = 0U;

  if (isWithinLimits(offset, 4U))
  {
    returnVal = ((m_bytes[offset + 3U] << 24U) + (m_bytes[offset + 2U] << 16U) + (m_bytes[offset + 1U] << 8U) + m_bytes[offset]);
  }

  return returnVal;
}


int32_t FrameParser::getInt32AsLSB(uint32_t offset) const
{
  return static_cast<int32_t>(getUInt32AsLSB(offset));
}


uint16_t FrameParser::getUInt16AsMSB(uint32_t offset) const
{
  uint16_t returnVal = 0U;

  if (isWithinLimits(offset, 2U))
  {
    returnVal = ((m_bytes[offset] << 8U) + m_bytes[offset + 1U]);
  }

  return returnVal;
}


uint32_t FrameParser::getUInt32AsMSB(uint32_t offset) const
{
  uint32_t returnVal = 0U;

  if (isWithinLimits(offset, 4U))
  {
    returnVal = ((m_bytes[offset] << 24U) + (m_bytes[offset + 1U] << 16U) + (m_bytes[offset + 2U] << 8U) + m_bytes[offset + 3U]);
  }

  return returnVal;
}


int32_t FrameParser::getInt32AsMSB(uint32_t offset) const
{
  return static_cast<int32_t>(getUInt32AsMSB(offset));
}


uint16_t FrameParser::getBytes(uint32_t offset, uint8_t * buffer, uint16_t size) const
{
  uint16_t returnedBytes = 0U;

  if (isWithinLimits(offset, size))
  {
    memcpy(buffer, &m_bytes[offset], size);
    returnedBytes = size;
  }

  return returnedBytes;
}


bool FrameParser::isWithinLimits(uint32_t offset, uint16_t size) const
{
  return ((offset + size) <= m_size);
}
