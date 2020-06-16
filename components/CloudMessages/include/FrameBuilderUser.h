#pragma once
#include <cstdint>
#include "FrameBuilder.h"


class FrameBuilderUser
{
public:
  FrameBuilderUser(void);

  uint32_t getSize(void) const;
  uint32_t getBytes(uint8_t buffer[], uint32_t size) const;

protected:
  FrameBuilder m_frame;
};
