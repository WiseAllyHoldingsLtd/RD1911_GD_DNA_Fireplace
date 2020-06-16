#include "TimeSyncRequest.h"
#include "FrameType.h"


TimeSyncRequest::TimeSyncRequest(void)
{
  m_frame.reset();
  m_frame.addUInt16AsMSB(FrameType::TIME_SYNC_REQUEST);
}
