#include "esp_log.h"
#include "MainTask.h"
#include "TaskProperties.h"

/* Static allocation of tasks */
namespace {
  MainTask s_mainTask;
}

/* Main entry point from startup task */
extern "C" void app_main(void)
{
  static_assert(sizeof(uint8_t) == sizeof(char), "Size mismatch");

  // Setting log levels for some system logging to remove annoying messages...
  esp_log_level_set("system_api", ESP_LOG_WARN);
  esp_log_level_set("event", ESP_LOG_WARN);
  esp_log_level_set("BTDM_INIT", ESP_LOG_WARN);
  esp_log_level_set("wifi", ESP_LOG_WARN);
  esp_log_level_set("phy", ESP_LOG_WARN);

  // Wave the magic wand to start the party!
  s_mainTask.start("MainTask", TaskStackSize::mainTask, TaskPriority::mainTask);

  return;
}
