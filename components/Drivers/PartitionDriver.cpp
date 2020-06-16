#include <esp_partition.h>
#include <esp_ota_ops.h>
#include "PartitionDriver.h"


namespace
{
  const char NOT_FOUND_STR[] = "";
  const uint32_t PARTITION_LABEL_MAX_SIZE = 17U;
}


PartitionDriver::PartitionDriver(void)
{}


const char * PartitionDriver::getBootPartitionName(void) const
{
  const esp_partition_t *partition = esp_ota_get_boot_partition();

  if (partition != nullptr)
  {
    return partition->label;
  }
  else
  {
    return NOT_FOUND_STR;
  }
}


const char * PartitionDriver::getRunningPartitionName(void) const
{
  const esp_partition_t *partition = esp_ota_get_running_partition();

  if (partition != nullptr)
  {
    return partition->label;
  }
  else
  {
    return NOT_FOUND_STR;
  }
}


const char * PartitionDriver::getUpdatePartitionName(void) const
{
  const esp_partition_t *partition = esp_ota_get_next_update_partition(nullptr);

  if (partition != nullptr)
  {
    return partition->label;
  }
  else
  {
    return NOT_FOUND_STR;
  }
}


uint32_t PartitionDriver::getBootPartitionAddress(void) const
{
  const esp_partition_t *partition = esp_ota_get_boot_partition();

  if (partition != nullptr)
  {
    return partition->address;
  }
  else
  {
    return 0U;
  }
}


uint32_t PartitionDriver::getRunningPartitionAddress(void) const
{
  const esp_partition_t *partition = esp_ota_get_running_partition();

  if (partition != nullptr)
  {
    return partition->address;
  }
  else
  {
    return 0U;
  }
}


uint32_t PartitionDriver::getUpdatePartitionAddress(void) const
{
  const esp_partition_t *partition = esp_ota_get_next_update_partition(nullptr);

  if (partition != nullptr)
  {
    return partition->address;
  }
  else
  {
    return 0U;
  }
};
