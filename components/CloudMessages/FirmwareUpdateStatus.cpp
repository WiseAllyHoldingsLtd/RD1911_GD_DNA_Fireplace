#include "FirmwareUpdateStatus.h"
#include "FrameType.h"


FirmwareUpdateStatus::FirmwareUpdateStatus()
{
  m_frame.reset();
  m_frame.addUInt16AsMSB(FrameType::FW_UPDATE_STATUS);
}


bool FirmwareUpdateStatus::addData(FirmwareUpdateResult::Enum status, uint8_t major, uint8_t minor, uint8_t test, uint16_t numOfRetries)
{
  bool result = m_frame.addUInt8(static_cast<uint8_t>(status));
  result = m_frame.addUInt8(major) && result;
  result = m_frame.addUInt8(minor) && result;
  result = m_frame.addUInt8(test) && result;
  result = m_frame.addUInt16AsMSB(numOfRetries) && result;
  return result;
}
