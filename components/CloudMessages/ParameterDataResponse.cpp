#include "ParameterDataResponse.h"
#include "FrameType.h"
#include "Constants.h"


ParameterDataResponse::ParameterDataResponse(void) : m_isFinalized(false)
{
  m_frame.reset();
  m_frame.addUInt16AsLSB(FrameType::APPLIANCE_PARAMS_RESPONSE); // FrameID, note this ID is LSB
}


bool ParameterDataResponse::addParameterData(uint16_t id, const uint8_t * data, uint8_t size)
{
  bool wasAdded = false;

  if (!m_isFinalized)
  {
    wasAdded = m_frame.addUInt16AsLSB(id);

    if (wasAdded)
    {
      wasAdded = m_frame.addUInt8(size);
    }

    if (wasAdded)
    {
      wasAdded = m_frame.addBytes(data, size);
    }
  }

  return wasAdded;
}


bool ParameterDataResponse::finalize(void)
{
  if (!m_isFinalized)
  {
    m_isFinalized = m_frame.addUInt16AsLSB(Constants::CLOUD_PARAMETER_END_OF_FRAME);
  }

  return m_isFinalized;
}
