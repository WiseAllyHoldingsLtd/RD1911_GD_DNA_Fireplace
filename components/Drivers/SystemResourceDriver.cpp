#include <esp_heap_caps.h>
#include "SystemResourceDriver.h"


uint32_t SystemResourceDriver::getCurrentFreeHeap(void)
{
  return heap_caps_get_free_size(MALLOC_CAP_8BIT);
}

uint32_t SystemResourceDriver::getMinimumFreeHeapSinceBoot(void)
{
  return heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT);
}
