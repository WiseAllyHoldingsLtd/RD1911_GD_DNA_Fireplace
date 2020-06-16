#include "FirmwareUpdateReady.h"
#include "FrameType.h"


namespace
{
  const uint32_t REQUIRED_FRAME_SIZE_MIN = 7U;
}


FirmwareUpdateReady::FirmwareUpdateReady(const FrameParser & frame)
  : m_frame(frame)
{
}

bool FirmwareUpdateReady::isValid(void) const
{
  return ((m_frame.getID() == static_cast<uint16_t>(FrameType::FW_UPDATE_READY))
         && (m_frame.getSize() >= REQUIRED_FRAME_SIZE_MIN));
}


uint16_t FirmwareUpdateReady::getRetryCount(void) const
{
  return m_frame.getUInt16AsMSB(2U);
}

uint16_t FirmwareUpdateReady::getUrlLength(void) const
{
  /* NOTE: Returns number of chars, not including the zero-term. This must be added to size used to retrieve string. */
  return m_frame.getUInt16AsMSB(4U);
}

uint16_t FirmwareUpdateReady::getUrlString(char * buffer, uint16_t size) const
{
  uint16_t returnedBytes = 0U;

  if (size > 0U)
  {
    returnedBytes = m_frame.getBytes(6U, reinterpret_cast<uint8_t *>(buffer), size - 1U);

    if (returnedBytes >= size)
    {
      returnedBytes = size - 1U;
    }

    buffer[returnedBytes] = '\0';
  }

  return returnedBytes;
}
