
#include "EspCpp.hpp"
#include "DiscoveryRequest.h"
#include "DiscoveryResponse.h"

#include "HTTPJson.h"
#include "StaticStrings.h"
#include "DiscoveryService.hpp"


namespace {
  const char LOG_TAG[] = "DiscoveryService";
}

DiscoveryService::DiscoveryService(const CloudDriverInterface &cloud,
                                   DeviceMetaInterface &meta,
                                   QueueInterface<ConnectionStrings_t> &queue,
                                   QueueInterface<WifiConnectionStatus::Enum> &resultQueue)
: m_cloudDriver(cloud),
  m_deviceMeta(meta),
  m_connectionStringQueue(queue),
  m_resultQueue(resultQueue)
{

}

bool DiscoveryService::registerDevice()
{
  bool result = false;
  WifiConnectionStatus::Enum discoveryResult = WifiConnectionStatus::IN_PROGRESS;

  char deviceGDID[Constants::GDID_SIZE_MAX] = {};
  m_deviceMeta.getGDID(deviceGDID, sizeof(deviceGDID));

  DiscoveryRequest discoveryRequest(deviceGDID, Constants::SW_VERSION, Constants::SW_TEST_NO);
  char * requestString = discoveryRequest.getJSONString();
  ESP_LOGI(LOG_TAG, "DiscoveryRequest '%s', payload '%s'\n", Constants::DISCOVERY_URL, requestString);

  if (requestString != nullptr)
  {
    HTTPJson sendMessage(requestString);
    HTTPJson recvMessage(m_buffer, sizeof(m_buffer));
    bool wasRequestSent = m_cloudDriver.postRequest(Constants::DISCOVERY_URL, sendMessage, recvMessage);

    free(requestString); // allocated with malloc

    if (wasRequestSent)
    {
      DiscoveryResponse discoveryResponse;

      if (discoveryResponse.parseJSONString(m_buffer))
      {
        ConnectionStrings_t cStrings;
        cStrings.primary[0] = '\0';
        cStrings.secondary[0] = '\0';
        strncat(cStrings.primary, discoveryResponse.getPrimaryConnectionString(), Constants::CLOUD_CONNECTION_STRING_MAX_LENGTH-1);
        strncat(cStrings.secondary, discoveryResponse.getSecondaryConnectionString(), Constants::CLOUD_CONNECTION_STRING_MAX_LENGTH-1);
        ESP_LOGI(LOG_TAG, "Registered!\nPrimary: %s\nSecondary: %s", cStrings.primary, cStrings.secondary);

        m_connectionStringQueue.reset(); // drop any previous set of connection strings
        m_connectionStringQueue.push(cStrings);
        discoveryResult = WifiConnectionStatus::FULLY_FUNCTIONAL;
        result = true;
      }
      else
      {
        discoveryResult = WifiConnectionStatus::DISCOVERY_AMQP_SERVICE_SERVER_INACCESSIBLE;
        ESP_LOGE(LOG_TAG, "Missing connectionString data - DiscoveryResponse parse failed.");
      }
    }
    else
    {
      discoveryResult = WifiConnectionStatus::DISCOVERY_AMQP_SERVICE_SERVER_INACCESSIBLE;
      ESP_LOGE(LOG_TAG, "Failed to send request");
    }
  }
  else
  {
    discoveryResult = WifiConnectionStatus::DISCOVERY_AMQP_SERVICE_SERVER_INACCESSIBLE;
    ESP_LOGE(LOG_TAG, "Failed to create Discovery Request");
  }

  m_resultQueue.reset(); // drop any previous results
  if ( !m_resultQueue.push(discoveryResult) )
  {
    ESP_LOGE(LOG_TAG, "Failed to push discovery result");
  }

  return result;
}
