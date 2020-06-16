#include <esp_log.h>
#include <sdkconfig.h>
#include "PersistentStorageDriver.h"

static const char * LOG_TAG = "PersistentStorageDriver";
static const char * STORAGE_NAMESPACE = "dataStore";


PersistentStorageDriver::PersistentStorageDriver(void)
  : m_initialized(false), m_mode(NVS_READONLY), m_partition(nullptr)
{
}


bool PersistentStorageDriver::init(const char* partition, bool readOnly)
{
  if (!m_initialized)
  {
    m_partition = partition;
    m_mode = (readOnly ? NVS_READONLY : NVS_READWRITE);

    esp_err_t initResult = nvs_flash_init_partition(m_partition);
    m_initialized = (initResult == ESP_OK);

    if (!m_initialized)
    {
      ESP_LOGE(LOG_TAG, "Unable to initialize non-volatile storage '%s'. Error code %d.", m_partition, initResult);
    }
  }

  return m_initialized;
}


bool PersistentStorageDriver::erase(const char* partition)
{
  bool wasErased = false;

  if (!m_initialized)
  {
    esp_err_t eraseResult = nvs_flash_erase_partition(partition);
    wasErased = (eraseResult == ESP_OK);

    if (!wasErased)
    {
      ESP_LOGE(LOG_TAG, "Unable to erase non-volatile storage '%s'. Error code %d.", partition, eraseResult);
    }
  }

  return wasErased;
}


bool PersistentStorageDriver::readValue(const char* id, uint32_t * value)
{
  bool wasRead = false;
  nvs_handle storageHandle;

  m_lock.take();

  if (open(&storageHandle))
  {
    esp_err_t readResult = nvs_get_u32(storageHandle, id, value);
    wasRead = (readResult == ESP_OK);

    if (!wasRead)
    {
      ESP_LOGI(LOG_TAG, "Failed to read non-volatile storage value '%s'. Error code %u.", id, readResult);
    }

    close(storageHandle);
  }

  m_lock.give();
  return wasRead;
}


bool PersistentStorageDriver::writeValue(const char* id, uint32_t value)
{
  bool wasWritten = false;
  nvs_handle storageHandle;

  m_lock.take();

  if (open(&storageHandle))
  {
    esp_err_t writeResult = nvs_set_u32(storageHandle, id, value);
    wasWritten = (writeResult == ESP_OK);

    if (!wasWritten)
    {
      ESP_LOGE(LOG_TAG, "Failed to write non-volatile storage value '%s'. Error code %u.", id, writeResult);
    }

    commit(storageHandle);
    close(storageHandle);
  }

  m_lock.give();
  return wasWritten;
}


bool PersistentStorageDriver::readString(const char* id, char * str, uint32_t size)
{
  bool wasRead = false;
  nvs_handle storageHandle;

  m_lock.take();

  if (open(&storageHandle))
  {
    esp_err_t readResult = nvs_get_str(storageHandle, id, str, &size);
    wasRead = (readResult == ESP_OK);

    if (!wasRead)
    {
      ESP_LOGI(LOG_TAG, "Failed to read non-volatile storage string '%s'. Error code %d.", id, readResult);
    }

    close(storageHandle);
  }

  m_lock.give();
  return wasRead;
}


bool PersistentStorageDriver::writeString(const char* id, const char * str)
{
  bool wasWritten = false;
  nvs_handle storageHandle;

  m_lock.take();

  if (open(&storageHandle))
  {
    esp_err_t writeResult = nvs_set_str(storageHandle, id, str);
    wasWritten = (writeResult == ESP_OK);

    if (!wasWritten)
    {
      ESP_LOGE(LOG_TAG, "Failed to write non-volatile storage string '%s'. Error code %d.", id, writeResult);
    }

    commit(storageHandle);
    close(storageHandle);
  }

  m_lock.give();
  return wasWritten;
}


bool PersistentStorageDriver::readBlob(const char* id, uint8_t data[], uint32_t & size)
{
  bool wasRead = false;

  if (size > 0U)
  {
    nvs_handle storageHandle;

    m_lock.take();

    if (open(&storageHandle))
    {
      uint32_t requiredSize = 0U;
      esp_err_t sizeResult = nvs_get_blob(storageHandle, id, nullptr, &requiredSize);

      if ((sizeResult == ESP_OK) && (size >= requiredSize))
      {
        esp_err_t readResult = nvs_get_blob(storageHandle, id, static_cast<void*>(data), &size);
        wasRead = (readResult == ESP_OK);

        if (!wasRead)
        {
          ESP_LOGE(LOG_TAG, "Failed to read non-volatile storage blob '%s'. Error code %d.", id, readResult);
          size = 0U;
        }
      }
      else
      {
        ESP_LOGE(LOG_TAG, "Failed to get non-volatile storage blob '%s' size. Error code %d, required size %d, available size %d.",
            id, sizeResult, requiredSize, size);

        size = 0U;
      }

      close(storageHandle);
    }

    m_lock.give();
  }

  return wasRead;
}


bool PersistentStorageDriver::writeBlob(const char* id, const uint8_t data[], uint32_t size)
{
  bool wasWritten = false;

  if (size > 0U)
  {
    nvs_handle storageHandle;
    m_lock.take();

    if (open(&storageHandle))
    {
      esp_err_t writeResult = nvs_set_blob(storageHandle, id, static_cast<const void*>(data), size);
      wasWritten = (writeResult == ESP_OK);

      if (!wasWritten)
      {
        ESP_LOGE(LOG_TAG, "Failed to write non-volatile storage blob '%s'. Error code %d.", id, writeResult);
      }

      commit(storageHandle);
      close(storageHandle);
    }

    m_lock.give();
  }

  return wasWritten;
}


bool PersistentStorageDriver::open(nvs_handle * handle) const
{
  bool wasOpened = false;

  if (m_initialized)
  {
    esp_err_t openResult = nvs_open_from_partition(m_partition, STORAGE_NAMESPACE, m_mode, handle);

    if (openResult == ESP_OK)
    {
      wasOpened = true;
    }
    else
    {
      ESP_LOGE(LOG_TAG, "Unable to open non-volatile storage. Error code %d.", openResult);
    }
  }

  return wasOpened;
}


bool PersistentStorageDriver::commit(nvs_handle handle) const
{
  bool wasCommitted = false;

  if (m_initialized)
  {
    esp_err_t commitResult = nvs_commit(handle);

    if (commitResult == ESP_OK)
    {
      wasCommitted = true;
    }
    else
    {
      ESP_LOGE(LOG_TAG, "Unable to commit to non-volatile storage. Error code %d.", commitResult);
    }
  }

  return wasCommitted;
}


void PersistentStorageDriver::close(nvs_handle handle) const
{
  if (m_initialized)
  {
    nvs_close(handle);
  }
}
