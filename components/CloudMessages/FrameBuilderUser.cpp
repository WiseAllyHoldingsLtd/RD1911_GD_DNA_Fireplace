#include "FrameBuilderUser.h"


FrameBuilderUser::FrameBuilderUser(void)
{
}


uint32_t FrameBuilderUser::getSize(void) const
{
  return m_frame.getSize();
}


uint32_t FrameBuilderUser::getBytes(uint8_t buffer[], uint32_t size) const
{
  // Only allow copy of entire frame, anything else would be pointless
  uint32_t returnedBytes = 0U;

  if (size >= getSize())
  {
    returnedBytes = m_frame.getBytes(buffer, size);
  }

  return returnedBytes;
}
