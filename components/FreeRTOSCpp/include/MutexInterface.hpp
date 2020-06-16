#pragma once

#include "FreeRTOS.hpp"


class MutexInterface
{
public:
  /* Release mutex semaphore */
  virtual bool give(void) = 0;

  /* Take mutex semaphore (if already taken, wait indefinetly long for it to become available) */
  virtual bool take(void) = 0;

  /* Take mutex semaphore (if already taken, wait timeout ticks for it to become available) */
  virtual bool take(TickType_t timeout) = 0;

  virtual ~MutexInterface(void) {};
};
