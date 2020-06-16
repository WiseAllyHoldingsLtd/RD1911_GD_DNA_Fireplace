#include "MultipleFrameResponse.h"
#include "FrameType.h"


namespace
{
  const uint32_t NUM_OF_FRAMES_INDEX = 2U;
}


MultipleFrameResponse::MultipleFrameResponse(void) : m_numFrames(0U)
{
  m_frame.reset();
  m_frame.addUInt16AsMSB(FrameType::MULTIPLE_FRAME_RESPONSE); // FrameID - main frame
  m_frame.addUInt16AsLSB(0U); // placeHolder for numOfFrames
}


bool MultipleFrameResponse::addParameterDataResponse(const ParameterDataResponse & parameterData)
{
  bool wasAdded = false;

  uint32_t size = parameterData.getSize();

  if (size > 0U)
  {
    wasAdded = m_frame.addUInt8(size);
    uint8_t * buffer = new uint8_t[size];
    parameterData.getBytes(buffer, size);
    wasAdded = m_frame.addBytes(buffer, size);

    if (wasAdded)
    {
      ++m_numFrames;
      m_frame.modifyUInt16AsLSB(NUM_OF_FRAMES_INDEX, m_numFrames);
    }

    delete [] buffer;
  }

  return wasAdded;
}


uint16_t MultipleFrameResponse::getNumberOfFrames(void) const
{
  return m_numFrames;
}
