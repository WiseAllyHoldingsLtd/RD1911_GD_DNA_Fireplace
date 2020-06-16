#include "FreeRTOS.hpp"

BaseType_t xTaskCreatePinnedToCore(TaskFunction_t, const char * const, const uint16_t, void * const, uint32_t, TaskHandle_t * const, const BaseType_t xCoreID)
{
  return pdPASS;
}

void vTaskDelete(TaskHandle_t) {}
void vTaskDelay(TickType_t) {}

SemaphoreHandle_t mockHandle;

SemaphoreHandle_t xSemaphoreCreateMutex()
{
  return mockHandle;
}

void vSemaphoreDelete(SemaphoreHandle_t) {}

BaseType_t xSemaphoreGive(SemaphoreHandle_t)
{
 return pdTRUE;
}

BaseType_t xSemaphoreTake(SemaphoreHandle_t, TickType_t)
{
  return pdTRUE;
}
