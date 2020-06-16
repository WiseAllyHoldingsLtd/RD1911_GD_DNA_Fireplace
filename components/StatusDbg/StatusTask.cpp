#include <esp_log.h>
#include <esp_system.h>
#include <esp_spi_flash.h>
#include <esp_heap_caps.h>
#include <esp_ota_ops.h>
#include <soc/soc.h>
#include <sdkconfig.h>
#include "FreeRTOS.hpp"
#include "StatusTask.hpp"


const static char LOG_TAG[] = "STATUS_DBG";

void StatusTask::run(bool eternalLoop)
{
  delayMS(350u);
  ESP_LOGI(LOG_TAG, "Starting StatusTask");
  printChipInfo();
  printIDFVersion();
  printPartitionTable(ESP_PARTITION_TYPE_APP);
  printPartitionTable(ESP_PARTITION_TYPE_DATA);
  printf("\n");
  printRunningPartitionInfo();

  do {
    printf("\n");
    printHeapInformation();
    printTaskStatus();

    fflush(stdout);
    delayMS(30000u);

    } while(eternalLoop);
}


void StatusTask::printChipInfo(void) const
{
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);

  ESP_LOGI(LOG_TAG, "This is ESP32 chip with %d CPU cores, WiFi%s%s, silicon revision %d, %dMB %s flash",
             chip_info.cores,
             (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
             (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "",
             chip_info.revision,
             spi_flash_get_chip_size() / (1024 * 1024),
             (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

  uint8_t macAddress[6U] = {};
  esp_err_t macResult = esp_efuse_mac_get_default(macAddress);

  if (macResult == ESP_OK)
  {
    ESP_LOGI(LOG_TAG, "Device base MAC: %02x:%02x:%02x:%02x:%02x:%02x",
                        macAddress[0U], macAddress[1U], macAddress[2U],
                        macAddress[3U], macAddress[4U], macAddress[5U]);
  }
}


void StatusTask::printIDFVersion(void) const
{
  ESP_LOGI(LOG_TAG, "IDF version: %s", esp_get_idf_version());
}


void StatusTask::printRunningPartitionInfo(void) const
{
  const esp_partition_t *bootPartition = esp_ota_get_boot_partition();
  const esp_partition_t *runningPartition = esp_ota_get_running_partition();
  const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);

  if (bootPartition != runningPartition)
  {
    ESP_LOGW(LOG_TAG, "Configured boot partition and actual running partition does not match. Possible corrupt image or OTA data!");
    ESP_LOGI(LOG_TAG, "Boot partition: %s at offset 0x%08x", bootPartition->label, bootPartition->address);
  }

  ESP_LOGI(LOG_TAG, "Running partition: %s at offset 0x%08x", runningPartition->label, runningPartition->address);
  ESP_LOGI(LOG_TAG, "Next update partition: %s at offset 0x%08x", update_partition->label, update_partition->address);
}


void StatusTask::printPartitionTable(esp_partition_type_t type) const
{
  ESP_LOGI(LOG_TAG, "%s partitions:", type == ESP_PARTITION_TYPE_APP ? "APP" : "DATA");

  const esp_partition_t * partition;
  esp_partition_iterator_t iterator = esp_partition_find(type, ESP_PARTITION_SUBTYPE_ANY, nullptr);

  while (iterator != nullptr)
  {
    partition = esp_partition_get(iterator);


    ESP_LOGI(LOG_TAG, "%-16saddr:0x%08x  size[kB]:%-6dtype:%-6dsubtype:%c  encrypted:%s",
              partition->label,
              partition->address,
              partition->size / 1024,
              partition->type,
              getPartitionSubTypeCode(partition->type, partition->subtype),
              partition->encrypted ? "yes" : "no");

    iterator = esp_partition_next(iterator);
  }

  esp_partition_iterator_release(iterator);
}


void StatusTask::printHeapInformation(void) const
{
  ESP_LOGI(LOG_TAG, "Current free heap size [kB]: %d", heap_caps_get_free_size(MALLOC_CAP_8BIT)/1024);
  ESP_LOGI(LOG_TAG, "Minimum free heap size since boot [kB]: %d", heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT)/1024);
}


void StatusTask::printTaskStatus(void) const
{
  TaskStatus_t *taskStatusArray;
  UBaseType_t numberOfTasks;

  /* Allocate space for task status list */
  numberOfTasks = uxTaskGetNumberOfTasks();
  taskStatusArray = static_cast<TaskStatus_t*>(pvPortMalloc(numberOfTasks * sizeof(TaskStatus_t)));
  ESP_LOGI(LOG_TAG, "Number of running tasks: %d", numberOfTasks);

  /* Get task status list (note that number of tasks might have changed since we allocated size) */
  if (taskStatusArray != nullptr)
  {
    numberOfTasks = uxTaskGetSystemState(taskStatusArray, numberOfTasks, nullptr);

    if (numberOfTasks > 0U)
    {
      for (UBaseType_t i = 0U; i < numberOfTasks; i++)
      {
        ESP_LOGI(LOG_TAG, "%-14sstate: %-4cpri:%-5dbasepri:%-5dminStackLeft: %d",
                  taskStatusArray[i].pcTaskName,
                  getTaskStateCode(taskStatusArray[i].eCurrentState),
                  taskStatusArray[i].uxCurrentPriority,
                  taskStatusArray[i].uxBasePriority,
                  taskStatusArray[i].usStackHighWaterMark);
      }
    }
    else
    {
      ESP_LOGI(LOG_TAG, "Task status list not generated, allocated size was too small (a task was maybe created after allocation)");
    }
  }
  else
  {
    ESP_LOGI(LOG_TAG, "Task status list was returned empty");
  }

  vPortFree(static_cast<void*>(taskStatusArray));
}


char StatusTask::getTaskStateCode(eTaskState state) const
{
  char retCode;

  switch (state)
  {
  case eBlocked:
    retCode = 'B';
    break;

  case eSuspended:
    retCode = 'S';
    break;

  case eDeleted:
    retCode = 'D';
    break;

  case eRunning:
  case eReady:
    retCode = 'R'; /* Running/Ready */
    break;

  default:
    retCode = '?';
    break;
  }

  return retCode;
}


char StatusTask::getPartitionSubTypeCode(esp_partition_type_t type, esp_partition_subtype_t subType) const
{
  char retCode;

  if (type == ESP_PARTITION_TYPE_APP)
  {
    if ((subType >= ESP_PARTITION_SUBTYPE_APP_OTA_MIN) && (subType <= ESP_PARTITION_SUBTYPE_APP_OTA_MAX))
    {
      retCode = (subType == ESP_PARTITION_SUBTYPE_APP_OTA_0 ? 'O' : 'o');
    }
    else
    {
      switch (subType)
      {
      case ESP_PARTITION_SUBTYPE_APP_FACTORY:
        retCode = 'F';
        break;

      case ESP_PARTITION_SUBTYPE_APP_TEST:
        retCode = 'T';
        break;

      default:
        retCode = '?';
        break;
      }
    }
  }
  else
  {
    switch (subType)
    {
    case ESP_PARTITION_SUBTYPE_DATA_FAT:
      retCode = 'F';
      break;

    case ESP_PARTITION_SUBTYPE_DATA_OTA:
      retCode = 'O';
      break;

    case ESP_PARTITION_SUBTYPE_DATA_PHY:
      retCode = 'P';
      break;

    case ESP_PARTITION_SUBTYPE_DATA_NVS:
      retCode = 'N';
      break;

    case ESP_PARTITION_SUBTYPE_DATA_COREDUMP:
      retCode = 'C';
      break;

    case ESP_PARTITION_SUBTYPE_DATA_ESPHTTPD:
      retCode = 'E';
      break;

    case ESP_PARTITION_SUBTYPE_DATA_SPIFFS:
      retCode = 'S';
      break;

    default:
      retCode = '?';
      break;
    }
  }

  return retCode;
}
