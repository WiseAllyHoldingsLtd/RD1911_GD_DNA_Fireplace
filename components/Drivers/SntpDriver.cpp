
#include "SntpDriver.h"

#include "apps/sntp/sntp.h" // should be "lwip/apps/sntp.h" on esp-idf 3.2
#include "EspCpp.hpp"
#include "FreeRTOS.hpp"

#include "Constants.h"
#include <cstring>

namespace
{
  const char LOG_TAG[] = "SntpDriver";
}

bool SntpDriver::runSntpOnce(const char *ntpUrl1,
                             const char *ntpUrl2,
                             uint64_t &sysTimeUnix)
{
  // Before 2018 is uninitialized
  const time_t initializedDate = 48u * 365u * 24u * 3600u;
  char ntpUrlCopy[64] = {};

  strncpy(ntpUrlCopy, ntpUrl1, sizeof(ntpUrlCopy)-1);
  ntpUrlCopy[sizeof(ntpUrlCopy)-1] = 0;
  sntp_setservername(0, ntpUrlCopy);

  strncpy(ntpUrlCopy, ntpUrl2, sizeof(ntpUrlCopy)-1);
  ntpUrlCopy[sizeof(ntpUrlCopy)-1] = 0;
  sntp_setservername(1, ntpUrlCopy);

  time_t ts = time(NULL);
  bool wasInitialized = false;
  if ( ts >= initializedDate )
  {
    wasInitialized = true;
  }
  ts = 0;

  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_init();

  const uint32_t timeout =
    static_cast<uint32_t>(Constants::NTP_RUN_ONCE_TIMEOUT_SECONDS)*2u;

  for (uint32_t i=0; i<timeout && ts < initializedDate; ++i)
  {
      FreeRTOS_delay_ms(500u);
      ts = time(NULL);
  }

  sntp_stop();

  bool didAdjustTime = (!wasInitialized && ts >= initializedDate);
  sysTimeUnix = static_cast<uint64_t>(ts);
  return didAdjustTime;
}
