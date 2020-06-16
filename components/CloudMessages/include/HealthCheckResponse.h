#pragma once

#include <cstdint>

#include "CloudInboundMessageInterface.h"
#include "JSONObject.h"


class HealthCheckResponse : public CloudInboundMessageInterface
{
public:
  HealthCheckResponse(void);
  virtual bool parseJSONString(const char * jsonString);

  uint8_t getHealth(void) const;
  uint32_t getUnixTimestamp(void) const;
  uint8_t getDayOfWeek(void) const;
  int32_t getUtcOffset(void) const;

private:
  bool getHealthValue(int32_t & value) const;
  bool getTimeSyncValue(const char * key, int32_t & value) const;
  JSONObject m_message;
};
