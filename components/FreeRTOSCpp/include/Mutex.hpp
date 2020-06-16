#pragma once

#include "FreeRTOS.hpp"
#include "MutexInterface.hpp"


class Mutex : public MutexInterface
{
public:
  Mutex(void)
  {
    m_handle = xSemaphoreCreateMutex();
  }

  virtual bool give(void)
  {
    BaseType_t result = pdFALSE;

    if (m_handle != NULL)
    {
      result = xSemaphoreGive(m_handle);
    }

    return (result == pdTRUE);
  }

  virtual bool take(TickType_t timeout)
  {
    BaseType_t result = pdFALSE;

    if (m_handle != NULL)
    {
      result = xSemaphoreTake(m_handle, timeout);
    }

    return (result == pdTRUE);
  }

  virtual bool take(void)
  {
    return take(portMAX_DELAY);
  }

  ~Mutex(void)
  {
    vSemaphoreDelete(m_handle);
  }

private:
  SemaphoreHandle_t m_handle;
};
