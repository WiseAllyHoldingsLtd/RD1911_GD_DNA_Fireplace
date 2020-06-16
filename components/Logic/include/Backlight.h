#pragma once

#include <stdint.h>

#include "BacklightDriverInterface.h"
#include "BacklightDimOptions.h"


class Backlight
{
public:
  Backlight(uint64_t currentTimeMs, BacklightDriverInterface &backlightDriver);

  /**
  *   dims the backlight if gui is inactive after x seconds, fades in and out smoothly.
  *   Expects to be called once per iteration of the main loop.
  */
  bool dim(bool wasGuiActive, uint64_t currentTimeMs, BacklightOptions::Enum backlightDimOption);

  /**
  *   Returns whether the screen is fully dimmed or not (return value of last call to 'dim'.)
  */
  bool isScreenFullyDimmed(void) const;

private:
  uint8_t convertEnumToUint(BacklightOptions::Enum enumToConvert);

  BacklightDriverInterface &m_backlightDriver;

  uint64_t m_timestampLastActiveScreenMs;
  uint64_t m_timestampFirstActiveScreenAfterInactiveMs;
  int64_t m_currentBacklightLevel;

  bool m_isActiveScreen; /* Used to prevent visual glitches when pressing a button while already fading backlight in and to determine when it should start to fade in */
  bool m_isFullyDimmed;
};

