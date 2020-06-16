#include "Backlight.h"
#include "Constants.h"
#include "LinearInterpolator.h"


Backlight::Backlight(uint64_t currentTimeMs, BacklightDriverInterface &backlightDriver) : m_backlightDriver(backlightDriver),
                                                                                          m_currentBacklightLevel(Constants::DIM_MAX_VALUE),
                                                                                          m_isActiveScreen(false), m_isFullyDimmed(false)
{
  m_timestampLastActiveScreenMs = currentTimeMs;
  m_timestampFirstActiveScreenAfterInactiveMs = currentTimeMs; 
}

bool Backlight::dim(bool wasGuiActive, uint64_t currentTimeMs, BacklightOptions::Enum backlightDimOption)
{
  uint8_t dimLowLimitPercentage = convertEnumToUint(backlightDimOption);

  if (wasGuiActive){
    m_timestampLastActiveScreenMs = currentTimeMs;
    if (!m_isActiveScreen)
    {
      m_timestampFirstActiveScreenAfterInactiveMs = currentTimeMs;
      m_isActiveScreen = true;
    }
  }
  uint64_t timeSinceActiveMs = currentTimeMs - m_timestampLastActiveScreenMs;
  
  if (timeSinceActiveMs >= Constants::DIM_TIMEOUT_MS)
  {
    const int64_t timeValues[] = { 0, Constants::DIM_TIMEOUT_MS, Constants::DIM_TIMEOUT_MS + Constants::DIM_FADE_OUT_TIME_MS};
    const int64_t backlightValues[] = { Constants::DIM_MAX_VALUE, Constants::DIM_MAX_VALUE, dimLowLimitPercentage};

    uint8_t numPairs = sizeof(timeValues) / sizeof(timeValues[0]);

    LinearInterpolator <int64_t, int64_t>li(timeValues, backlightValues, numPairs);

    li.interpolate(static_cast<int64_t>(timeSinceActiveMs), m_currentBacklightLevel);
    
    m_isActiveScreen = false;
  }
  else
  {
    const int64_t timeValues[] = {0, Constants::DIM_FADE_IN_TIME_MS};
    const int64_t backlightValues[] = { m_currentBacklightLevel, Constants::DIM_MAX_VALUE };

    uint8_t numPairs = sizeof(timeValues) / sizeof(timeValues[0]);

    LinearInterpolator <int64_t, int64_t>li(timeValues, backlightValues, numPairs);

    li.interpolate(static_cast<int64_t>(currentTimeMs - m_timestampFirstActiveScreenAfterInactiveMs), m_currentBacklightLevel);
  }


  m_backlightDriver.setBacklightLevel(static_cast<uint8_t>(m_currentBacklightLevel));
  m_isFullyDimmed = (static_cast<uint8_t>(m_currentBacklightLevel) == dimLowLimitPercentage) && (dimLowLimitPercentage != Constants::DIM_MAX_VALUE);
  return m_isFullyDimmed;
}


bool Backlight::isScreenFullyDimmed(void) const
{
  return m_isFullyDimmed;
}


uint8_t Backlight::convertEnumToUint(BacklightOptions::Enum enumToConvert)
{
  uint8_t level = Constants::DIM_NORMAL_VALUE;
  switch (enumToConvert)
  {
  case BacklightOptions::Dark:
    level = Constants::DIM_DARK_VALUE;
    break;
  case BacklightOptions::Normal:
    level = Constants::DIM_NORMAL_VALUE;
    break;
  case BacklightOptions::Bright:
    level = Constants::DIM_BRIGHT_VALUE;
    break;
  case BacklightOptions::Max:
    level = Constants::DIM_MAX_VALUE;
    break;
  default:
    //do nothing
    break;
  }
  
  return level;
}
