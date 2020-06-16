#pragma once

#ifndef UNITTESTS
//#include "iar_misra_c_utils.h"
//MISRAC_DISABLE
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
//MISRAC_ENABLE

#else
#include <cstdint>
// FIXME: can we include FreeRTOSConfig.h safely without getting dependencies to FreeRTOS?
#define pdTRUE (1)
#define pdFALSE (0)
#define pdPASS pdTRUE
#define pdFAIL pdFALSE

#define portTICK_PERIOD_MS (1)
typedef long BaseType_t;
typedef void * TaskHandle_t;
typedef void * SemaphoreHandle_t;
typedef void (*TaskFunction_t)( void * );
typedef uint32_t TickType_t;
#define portMAX_DELAY ((TickType_t)0xffffffffUL)

BaseType_t xTaskCreate(TaskFunction_t, const char * const,const uint16_t, void * const, uint32_t, TaskHandle_t * const);
BaseType_t xTaskCreatePinnedToCore(TaskFunction_t, const char * const, const uint16_t, void * const, uint32_t, TaskHandle_t * const, const BaseType_t xCoreID);

SemaphoreHandle_t xSemaphoreCreateMutex();
void vSemaphoreDelete(SemaphoreHandle_t);
BaseType_t xSemaphoreGive(SemaphoreHandle_t);
BaseType_t xSemaphoreTake(SemaphoreHandle_t, TickType_t);

void vTaskDelete(TaskHandle_t);
void vTaskDelay(TickType_t);

#endif

// Convenience function to abstract port-details
void FreeRTOS_delay_ms(uint32_t delay);
