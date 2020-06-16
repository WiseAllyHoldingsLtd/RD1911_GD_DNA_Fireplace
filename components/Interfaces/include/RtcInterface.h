#pragma once

#include "DateTime.h"

class RtcInterface
{
public:
  /*
   * Configures the RTC
   */
  virtual bool setup() = 0;

  /**
    * Set the current time. The week day may or may not be adjusted according to the date.
    */
  virtual void setCurrentTime(const DateTime &dateTime) = 0;
  virtual TimeStatus::Enum getCurrentTime(DateTime &dateTime) = 0;

  virtual ~RtcInterface() {}
};
