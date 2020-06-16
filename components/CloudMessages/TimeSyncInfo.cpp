#include "TimeSyncInfo.h"
#include "FrameType.h"


namespace
{
  const uint32_t REQUIRED_FRAME_SIZE = 11U;
}


TimeSyncInfo::TimeSyncInfo(const FrameParser & frame)
  : m_frame(frame)
{
}

bool TimeSyncInfo::isValid(void) const
{
  return ((m_frame.getID() == static_cast<uint16_t>(FrameType::TIME_SYNC_INFO))
         && (m_frame.getSize() == REQUIRED_FRAME_SIZE));
}

uint32_t TimeSyncInfo::getUnixTime(void) const
{
  return m_frame.getUInt32AsLSB(2U);
}

uint8_t TimeSyncInfo::getDayOfWeek(void) const
{
  return m_frame.getUInt8(6U);
}

int32_t TimeSyncInfo::getUtcOffset(void) const
{
  return m_frame.getInt32AsLSB(7U);
}
