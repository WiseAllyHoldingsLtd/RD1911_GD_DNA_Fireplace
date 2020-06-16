
#include "EspCpp.hpp"
#include "EspCpp.hpp"
#include "HealthCheckRequest.h"
#include "HealthCheckResponse.h"
#include "HTTPJson.h"
#include "Constants.h"

#include "HealthCheckService.hpp"

namespace {
  const char LOG_TAG[] = "HealthCheckService";
}

HealthCheckService::HealthCheckService(const CloudDriverInterface &cloud,
                                       DeviceMetaInterface &meta,
                                       SettingsInterface &settings,
                                       EventFlagsInterface &eventFlags)
: m_cloudDriver(cloud), m_deviceMeta(meta), m_settings(settings), m_eventFlags(eventFlags)
{

}

bool HealthCheckService::checkHealth(uint8_t &health)
{
  bool result = false;
  uint8_t tempHealth = 0u;

  char deviceGDID[Constants::GDID_SIZE_MAX] = {};
  m_deviceMeta.getGDID(deviceGDID, sizeof(deviceGDID));

  HealthCheckRequest healthCheckRequest(deviceGDID);
  char * requestString = healthCheckRequest.getJSONString();
  ESP_LOGI(LOG_TAG, "healthCheckRequest '%s', payload '%s'\n", Constants::HEALTHCHECK_URL, requestString);

  if (requestString != nullptr)
  {
    HTTPJson sendMessage(requestString);
    HTTPJson recvMessage(m_buffer, sizeof(m_buffer));
    bool wasRequestSent = m_cloudDriver.postRequest(Constants::HEALTHCHECK_URL, sendMessage, recvMessage);

    free(requestString); // allocated with malloc

    if (wasRequestSent)
    {
      HealthCheckResponse healthCheckResponse;

      if (healthCheckResponse.parseJSONString(m_buffer))
      {
        tempHealth = healthCheckResponse.getHealth();
        uint32_t unixTime = healthCheckResponse.getUnixTimestamp();
        int32_t utcOffset = healthCheckResponse.getUtcOffset();

        m_settings.setTimeFromUnixTime(unixTime);
        ESP_LOGI(LOG_TAG, "SetTime called: %u.", unixTime);

        if (m_settings.getUtcOffset() != utcOffset)
        {
          m_settings.setUtcOffset(utcOffset);
          ESP_LOGI(LOG_TAG, "SetUTCOffset: (%i).", utcOffset);
        }

        result = true;
      }
      else
      {
        ESP_LOGE(LOG_TAG, "Missing healthCheck data - HealthCheckResponse parse failed.");
      }
    }
    else
    {
      ESP_LOGE(LOG_TAG, "Failed to send request");
    }
  }
  else
  {
    ESP_LOGE(LOG_TAG, "Failed to create HealthCheck Request");
  }

  if ( tempHealth < Constants::HEALTHCHECK_FULL_HEALTH )
  {
    ESP_LOGW(LOG_TAG, "Bad health: %u", static_cast<uint32_t>(tempHealth));
    m_eventFlags.clear(Constants::HEALTH_CHECK_FLAG_OK);
  }
  else
  {
    m_eventFlags.set(Constants::HEALTH_CHECK_FLAG_OK);
  }
  health = tempHealth;
  return result;
}
