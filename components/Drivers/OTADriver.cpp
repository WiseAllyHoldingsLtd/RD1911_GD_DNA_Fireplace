#include "EspCpp.hpp"
#include "OTADriver.h"

static const char * LOG_TAG = "OTADriver";

OTADriver::OTADriver(void)
  : m_originalBootPartition(esp_ota_get_boot_partition()),
    m_updateTargetPartition(esp_ota_get_next_update_partition(nullptr)),
    m_updateHandle(0U),
    m_isUpdateCompleted(false),
    m_isUpdateActivated(false)
{
}


void OTADriver::resetAndInit(void)
{
  if (m_isUpdateActivated)
  {
    esp_ota_set_boot_partition(m_originalBootPartition);
    m_isUpdateActivated = false;
  }

  if (isUpdateOngoing())
  {
    esp_ota_end(m_updateHandle);
    m_updateHandle = 0U;
  }

  m_isUpdateCompleted = false;
}


bool OTADriver::performSelfTest(void) const
{
  return isOTAEnabled();
}


bool OTADriver::isOTAEnabled(void) const
{
  return (m_originalBootPartition != nullptr) && (m_updateTargetPartition != nullptr);
}


bool OTADriver::isUpdateOngoing(void) const
{
  return (m_updateHandle > 0U);
}


bool OTADriver::beginUpdate(void)
{
  bool wasStarted = false;

  if (isOTAEnabled() && !isUpdateOngoing())
  {
    esp_err_t beginResult = esp_ota_begin(m_updateTargetPartition, OTA_SIZE_UNKNOWN, &m_updateHandle);

    if (beginResult == ESP_OK)
    {
      wasStarted = true;
    }
    else
    {
      ESP_LOGE(LOG_TAG, "Unable to prepare update. Error code %d.", beginResult);
    }
  }

  return wasStarted;
}


bool OTADriver::write(const char * data, uint32_t size)
{
  bool wasWritten = false;

  if (isOTAEnabled() && isUpdateOngoing())
  {
    esp_err_t writeResult = esp_ota_write(m_updateHandle, data, size);

    if (writeResult == ESP_OK)
    {
      wasWritten = true;
    }
    else
    {
      ESP_LOGE(LOG_TAG, "Unable to write to partition. Error code %d.", writeResult);
    }
  }

  return wasWritten;
}


bool OTADriver::endUpdate(void)
{
  if (isOTAEnabled() && isUpdateOngoing())
  {
    esp_err_t endResult = esp_ota_end(m_updateHandle);

    if (endResult == ESP_OK)
    {
      m_isUpdateCompleted = true;
    }
    else
    {
      ESP_LOGE(LOG_TAG, "Unable to finalize update. Error code %d.", endResult);
    }

    m_updateHandle = 0U;
  }

  return m_isUpdateCompleted;
}


bool OTADriver::activateUpdatedPartition(void)
{
  if (isOTAEnabled() && m_isUpdateCompleted)
  {
    esp_err_t updateResult = esp_ota_set_boot_partition(m_updateTargetPartition);

    if (updateResult == ESP_OK)
    {
      m_isUpdateActivated = true;
    }
    else
    {
      ESP_LOGE(LOG_TAG, "Unable to active update partition. Error code %d.", updateResult);
    }
  }

  return m_isUpdateActivated;
}
