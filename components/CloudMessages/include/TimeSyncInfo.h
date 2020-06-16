#pragma once

#include "FrameParser.h"


class TimeSyncInfo
{
public:
  TimeSyncInfo(const FrameParser & frame);
  bool isValid(void) const;

  uint32_t getUnixTime(void) const;
  uint8_t getDayOfWeek(void) const;
  int32_t getUtcOffset(void) const;

private:
  const FrameParser & m_frame;
};
