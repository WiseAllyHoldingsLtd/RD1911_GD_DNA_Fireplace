#include "MultipleFrameRequest.h"
#include "FrameType.h"


namespace
{
  const uint32_t REQUIRED_MIN_FRAME_SIZE = 4U; // FrameID and numOfFrames
}


MultipleFrameRequest::MultipleFrameRequest(const FrameParser & frame) : m_frame(frame)
{
  validate();
}


bool MultipleFrameRequest::isValid(void) const
{
  return m_isValid;
}


uint16_t MultipleFrameRequest::getNumOfPayloadFrames(void) const
{
  return m_frame.getUInt16AsLSB(2U);
}


uint8_t MultipleFrameRequest::getPayloadFrameSize(uint16_t frameIndex) const
{
  return m_frame.getUInt8(getSizeBytePos(frameIndex));
}


uint8_t MultipleFrameRequest::getPayloadFrame(uint16_t frameIndex, uint8_t * payloadFrameBytes, uint8_t size) const
{
  uint8_t returnedBytes = 0U;
  uint16_t frameSizePos = getSizeBytePos(frameIndex);

  if ((size > 0U) && (frameSizePos > 0U))
  {
    returnedBytes = m_frame.getBytes(frameSizePos + 1U, payloadFrameBytes, size);
  }

  return returnedBytes;
}


void MultipleFrameRequest::validate(void)
{
  m_isValid = false;

  if ((m_frame.getID() == static_cast<uint16_t>(FrameType::MULTIPLE_FRAME_REQUEST))
     && (m_frame.getSize() >= REQUIRED_MIN_FRAME_SIZE))
  {
    uint16_t numOfFrames = m_frame.getUInt16AsLSB(2U); // note: cannot use getNumOfPayloadFrames here as we haven't validated yet

    if (numOfFrames > 0U)
    {
      uint16_t lastPayloadSizePos = getSizeBytePos(numOfFrames - 1U); // zero-based index

      if (lastPayloadSizePos > 0U)
      {
        uint8_t lastPayloadSize = m_frame.getUInt8(lastPayloadSizePos);
        uint32_t expectedFrameSize = static_cast<uint32_t>(lastPayloadSize) + static_cast<uint32_t>(lastPayloadSizePos) + 1U;
        m_isValid = (m_frame.getSize() == expectedFrameSize);
      }
    }
    else
    {
      // Zero payload frames, but considered valid if size is correct
      m_isValid = (m_frame.getSize() == REQUIRED_MIN_FRAME_SIZE);
    }
  }
}


uint16_t MultipleFrameRequest::getSizeBytePos(uint16_t payloadIndex) const
{
  uint32_t frameSize = m_frame.getSize();
  uint16_t currentPayloadNum = 0U;
  uint16_t returnPos = REQUIRED_MIN_FRAME_SIZE; // bytePos for payloadNum = 0

  while (currentPayloadNum < payloadIndex)
  {
    ++currentPayloadNum;

    if (frameSize > returnPos)
    {
      // safe to read size contents
      uint8_t currentPayloadSize = m_frame.getUInt8(returnPos);
      returnPos += currentPayloadSize + 1U;
    }
    else
    {
      returnPos = 0U;
      break;
    }
  }

  return returnPos;
}
