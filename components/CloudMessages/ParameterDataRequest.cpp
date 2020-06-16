#include "ParameterDataRequest.h"
#include "FrameType.h"


namespace
{
  const uint32_t REQUIRED_MIN_FRAME_SIZE = 7U; // ID + EmptyParam + EndOfFrame =  2 + 3 + 2
  const uint32_t FIRST_SIZE_BYTE_POS = 4U;     // after 16bit frameID and 16bit paramID
  const uint16_t END_OF_FRAME = 0U;
  const uint32_t END_OF_FRAME_SIZE = sizeof(END_OF_FRAME);
  const uint16_t PARAM_DATA_ID_SIZE = 2U;        // uint16
}


ParameterDataRequest::ParameterDataRequest(const FrameParser & frame)
  : m_frame(frame)
{
  validate();
}

bool ParameterDataRequest::isValid(void) const
{
  return (m_numOfParamData > 0U);
}

uint8_t ParameterDataRequest::getNumOfParamDataItems(void) const
{
  return m_numOfParamData;
}


uint16_t ParameterDataRequest::getParamDataID(uint8_t paramDataIndex) const
{
  uint8_t sizePos = getSizeBytePos(paramDataIndex);
  uint16_t paramID = 0U;

  if (sizePos > 0U) // i.e. we know the paramDataItem exists, and thus sizePos is guarenteed to be at least FIRST_SIZE_BYTE_POS
  {
    paramID = m_frame.getUInt16AsLSB(sizePos - PARAM_DATA_ID_SIZE);
  }

  return paramID;
}


uint8_t ParameterDataRequest::getParamDataSize(uint8_t paramDataIndex) const
{
  return m_frame.getUInt8(getSizeBytePos(paramDataIndex));
}


uint8_t ParameterDataRequest::getParamData(uint8_t paramDataIndex, uint8_t * paramDataBytes, uint8_t size) const
{
  uint8_t returnedBytes = 0U;
  uint16_t paramDataSizePos = getSizeBytePos(paramDataIndex);

  if ((size > 0U) && (paramDataSizePos > 0U))
  {
    returnedBytes = m_frame.getBytes(paramDataSizePos + 1U, paramDataBytes, size);
  }

  return returnedBytes;
}


void ParameterDataRequest::validate(void)
{
  m_numOfParamData = 0U;

  if ((m_frame.getUInt16AsLSB(0U) == static_cast<uint16_t>(FrameType::APPLIANCE_PARAMS_REQUEST)) // Note in ApplianceParamsRequests frameID is LSB...
       && (m_frame.getSize() >= REQUIRED_MIN_FRAME_SIZE))
  {
    if (m_frame.getUInt16AsLSB(m_frame.getSize() - static_cast<uint16_t>(END_OF_FRAME_SIZE)) == END_OF_FRAME)
    {
      // We don't know how many paramData-items we have, must scan through bytes to check
      while (getSizeBytePos(m_numOfParamData) > 0U)
      {
        ++m_numOfParamData;
      }

      uint16_t lastPayloadSizePos = getSizeBytePos(m_numOfParamData - 1U); // zero-based index
      uint8_t lastPayloadSize = m_frame.getUInt8(lastPayloadSizePos);
      uint32_t expectedFrameSize = static_cast<uint32_t>(lastPayloadSize) + static_cast<uint32_t>(lastPayloadSizePos) + 1U + END_OF_FRAME_SIZE;

      if (expectedFrameSize != m_frame.getSize())
      {
        m_numOfParamData = 0;
      }
    }
    else
    {
      // invalid endOfFrame;
    }
  }
}


uint8_t ParameterDataRequest::getSizeBytePos(uint8_t paramDataIndex) const
{
  uint32_t frameSize = m_frame.getSize();
  uint8_t currentParamDataNum = 0U;
  uint8_t returnPos = FIRST_SIZE_BYTE_POS;

  while (currentParamDataNum < paramDataIndex)
  {
    ++currentParamDataNum;
    uint8_t currentParamDataSize = m_frame.getUInt8(returnPos);
    returnPos += currentParamDataSize + PARAM_DATA_ID_SIZE + 1U;

    if (returnPos >= (frameSize - END_OF_FRAME_SIZE)) // EndOfFrame is two bytes, so we must ignore them here.
    {
      returnPos = 0U;
      break;
    }
  }

  return returnPos;
}
