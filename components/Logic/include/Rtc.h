#pragma once

#include "RtcInterface.h"
#include "TWIDriverInterface.h"


class Rtc : public RtcInterface
{
public:
  static const uint8_t CLOCK_SLAVE_ADDRESS = 104u;
  static const uint8_t RETRY_COUNT = 5u;

  Rtc(TWIDriverInterface &twiDriver);
  virtual bool setup(void);
  virtual void setCurrentTime(const DateTime &dateTime);
  virtual TimeStatus::Enum getCurrentTime(DateTime &dateTime);

private:
  TWIDriverInterface &m_twiDriver;
};
