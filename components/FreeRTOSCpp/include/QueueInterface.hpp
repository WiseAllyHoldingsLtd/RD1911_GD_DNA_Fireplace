#pragma once

#include <cstdint>
#include "FreeRTOS.hpp"


template<class T>
class QueueInterface
{
public:
  virtual bool push(const T &item, uint32_t timeout=portMAX_DELAY) = 0;
  virtual bool pop(T &item, uint32_t timeout=portMAX_DELAY) = 0;

  virtual uint32_t numWaitingItems() = 0;
  virtual uint32_t numAvailableSpace() = 0;
  virtual void reset() = 0;

  // overwrite? (isr comp)
  // peek? (isr comp)
  // push to back/front? (isr comp)

  virtual ~QueueInterface() {};
};
