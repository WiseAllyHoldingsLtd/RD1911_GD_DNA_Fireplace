#include <algorithm>
#include <cstring>
#include <stdint.h>
#include "EspCpp.hpp"
#include "TaskProperties.h"
#include "FirmwareUpgradeTask.hpp"


namespace
{
  const char * LOG_TAG = "FwUpdate";
}


FirmwareUpgradeTask::FirmwareUpgradeTask(FirmwareUpgradeControllerInterface &controller)
  : m_controller(controller), m_state(FirmwareUpgradeState::notRequested)
{
}


void FirmwareUpgradeTask::run(bool eternalLoop)
{
  do
  {
    if ((m_state == FirmwareUpgradeState::started) && m_controller.isFirmwareUpgradeRequested())
    {
      ESP_LOGI(LOG_TAG, "Preparing firmware update.");
      m_state = FirmwareUpgradeState::inProgress;

      bool otaSuccess = m_controller.initOTA();

      if (otaSuccess)
      {
        ESP_LOGI(LOG_TAG, "OTA initialized OK.");
        const uint32_t fwUpgradeImageSize = m_controller.getOTADataSize();

        if (fwUpgradeImageSize > 0U)
        {
          ESP_LOGI(LOG_TAG, "Firmware upgrade image size: %d kB", fwUpgradeImageSize / 1024U);

          uint32_t startIndex = 0U;
          uint32_t totalBytesDownloaded = 0U;
          const uint32_t maxChunkSize = Constants::FW_DOWNLOAD_CHUNK_SIZE;

          do
          {
            uint32_t bytesToDownload = std::min(fwUpgradeImageSize - totalBytesDownloaded, maxChunkSize);

            otaSuccess = m_controller.downloadAndWriteOTAData(startIndex, startIndex + bytesToDownload - 1U, Constants::FW_DOWNLOAD_ATTEMPTS_MAX);

            totalBytesDownloaded += bytesToDownload;
            startIndex += bytesToDownload;
            delayMS(5U);

          } while (otaSuccess && (totalBytesDownloaded < fwUpgradeImageSize));

        }
        else
        {
          ESP_LOGE(LOG_TAG, "Firmware upgrade image size not found");
          otaSuccess = false;
        }
      }
      else
      {
        ESP_LOGE(LOG_TAG, "OTA initialization failed.");
      }

      if (otaSuccess)
      {
        otaSuccess = m_controller.commitOTA();

        if (otaSuccess)
        {
          ESP_LOGI(LOG_TAG, "OTA commited OK.");
        }
        else
        {
          ESP_LOGE(LOG_TAG, "OTA commit failed.");
        }
      }

      if (otaSuccess)
      {
        m_state = FirmwareUpgradeState::completed;
      }
      else
      {
        m_state = FirmwareUpgradeState::failed;
      }
    }
    else
    {
      // Task started without reason!
    }

    // TODO: Feed watchdog
    delayMS(1000U);

  } while(eternalLoop);
}

void FirmwareUpgradeTask::startTask()
{
  m_state = FirmwareUpgradeState::started;
  start("FWUpgrade", TaskStackSize::fwUpgrade, TaskPriority::fwUpgrade);
}

FirmwareUpgradeState::Enum FirmwareUpgradeTask::getCurrentState(void) const
{
  return m_state;
}
