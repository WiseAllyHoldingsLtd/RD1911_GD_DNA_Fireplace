#pragma once

#include "FrameParser.h"


class FirmwareUpdateReady
{
public:
  FirmwareUpdateReady(const FrameParser & frame);
  bool isValid(void) const;

  uint16_t getRetryCount(void) const;
  uint16_t getUrlLength(void) const;
  uint16_t getUrlString(char * buffer, uint16_t size) const;

private:
  const FrameParser & m_frame;
};
