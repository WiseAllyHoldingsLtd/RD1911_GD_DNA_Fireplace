#include "HealthCheckResponse.h"


namespace
{
  const char HEALTH_KEY[] = "Health";
  const char TIME_SYNC_KEY[] = "TimeSynchronization";
  const char UNIX_TIME_KEY[] = "UNIX_TIME";
  const char DAY_OF_WEEK_KEY[] = "DAY_OF_WEEK";
  const char UTC_OFFSET_KEY[] = "UTC_OFFSET";
}


HealthCheckResponse::HealthCheckResponse(void)
{
}

bool HealthCheckResponse::parseJSONString(const char * jsonString)
{
  bool parseResult = m_message.parse(jsonString);

  if (parseResult)
  {
    int32_t dummyValue;
    parseResult = (getHealthValue(dummyValue))
                    && (getTimeSyncValue(UNIX_TIME_KEY, dummyValue))
                    && (getTimeSyncValue(DAY_OF_WEEK_KEY, dummyValue))
                    && (getTimeSyncValue(UTC_OFFSET_KEY, dummyValue));
  }

  return parseResult;
}


uint8_t HealthCheckResponse::getHealth(void) const
{
  int32_t returnValue = 0;
  getHealthValue(returnValue);
  return static_cast<uint8_t>(returnValue);
}


uint32_t HealthCheckResponse::getUnixTimestamp(void) const
{
  int32_t returnValue = 0;
  getTimeSyncValue(UNIX_TIME_KEY, returnValue);
  return static_cast<uint32_t>(returnValue);
}


uint8_t HealthCheckResponse::getDayOfWeek(void) const
{
  int32_t returnValue = 0;
  getTimeSyncValue(DAY_OF_WEEK_KEY, returnValue);
  return static_cast<uint8_t>(returnValue);
}


int32_t HealthCheckResponse::getUtcOffset(void) const
{
  int32_t returnValue = 0;
  getTimeSyncValue(UTC_OFFSET_KEY, returnValue);
  return returnValue;
}


bool HealthCheckResponse::getHealthValue(int32_t & value) const
{
  return m_message.getValue(HEALTH_KEY, value);
}


bool HealthCheckResponse::getTimeSyncValue(const char * key, int32_t & value) const
{
  return m_message.getValueFromSubObject(TIME_SYNC_KEY, key, value);
}

