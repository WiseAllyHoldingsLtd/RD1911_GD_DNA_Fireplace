#pragma once

#include "FreeRTOS.hpp"


class BaseTask
{
public:
  BaseTask(TaskHandle_t handle) : m_handle(handle) {}

  BaseTask(TaskFunction_t func, const char * name, uint16_t stackDepth, unsigned priority, void * params) {
     start(func, name, stackDepth, priority, params);
  }
  bool start(TaskFunction_t func, const char * name, uint16_t stackDepth, unsigned priority, void * params) {
    BaseType_t result = pdFAIL;
    if (m_handle == nullptr) {
      result = xTaskCreatePinnedToCore(func, name, stackDepth, params, priority, &m_handle, 1);
    }
    return result == pdPASS;
  }
  virtual ~BaseTask() {
    stop();
  }
protected:
  TaskHandle_t m_handle;

  void inline delay(TickType_t ticks) {
    vTaskDelay(ticks);
  }

  void inline delayMS(uint32_t millis) {
    delay(millis/portTICK_PERIOD_MS);
  }

  void stop() {
    if (m_handle != nullptr) {
      TaskHandle_t temp = m_handle;
      m_handle = nullptr;
      vTaskDelete(temp);
    }
  }
private:
  BaseTask(const BaseTask &other) = delete;
  BaseTask& operator=(const BaseTask &other) = delete;
};


class Task : public BaseTask
{
public:
  Task() : BaseTask(nullptr) {}

  bool start(const char * name, uint16_t stackDepth, unsigned priority) {
    return BaseTask::start(&runTask, name, stackDepth, priority, this);
  }

  virtual void run(bool eternalLoop) = 0;
private:
  static void runTask(void * params) {
    static_cast<Task *>(params)->run(true);
    static_cast<Task *>(params)->stop();
  }
};

class AutoStartingTask : public Task
{
public:
  AutoStartingTask(const char * name, uint16_t stackDepth, unsigned priority)
  {
    /*bool result = */start(name, stackDepth, priority);
    // TODO: log if result is false
  }
};
