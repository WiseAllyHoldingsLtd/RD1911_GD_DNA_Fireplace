/*
 * NOTE: This file should only contain functions that
 * can be compiled in both app and unittest mode
 */

#include "FreeRTOS.hpp"

void FreeRTOS_delay_ms(uint32_t delay) {
  vTaskDelay(delay/portTICK_PERIOD_MS);
}
