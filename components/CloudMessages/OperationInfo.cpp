
#include "OperationInfo.h"
#include "FrameType.h"

namespace
{
  const uint32_t REQUIRED_FRAME_SIZE_MIN = 4U;
}

OperationInfo::OperationInfo(const FrameParser &frame):
    m_frame(frame)
{

}

bool OperationInfo::isValid(void) const
{
  return (m_frame.getID() == static_cast<uint16_t>(FrameType::OPERATION_INFO)
      && m_frame.getSize() >= REQUIRED_FRAME_SIZE_MIN
      && getType() != OperationType::invalid
      && getStatus() != OperationStatus::invalid);
}

OperationType::Enum OperationInfo::getType() const
{
  OperationType::Enum type = static_cast<OperationType::Enum>(m_frame.getUInt8(2U));

  switch (type)
  {
  case OperationType::forgetMe:
    break;
  default:
    type = OperationType::invalid;
  }

  return type;
}

OperationStatus::Enum OperationInfo::getStatus() const
{
  OperationStatus::Enum status = static_cast<OperationStatus::Enum>(m_frame.getUInt8(3U));

  switch (status)
  {
  case OperationStatus::success:
  case OperationStatus::forgetMeError:
    break;
  default:
    status = OperationStatus::invalid;
  }

  return status;
}
