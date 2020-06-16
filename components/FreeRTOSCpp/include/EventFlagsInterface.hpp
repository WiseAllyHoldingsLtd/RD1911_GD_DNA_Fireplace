#pragma once

#include <cstdint>

class EventFlagsInterface
{
public:
  /* Set event flag - do not use in interrupt */
  virtual bool get(uint8_t flagNo) const = 0;

  /* Set event flag - do not use in interrupt */
  virtual void set(uint8_t flagNo) const = 0;

  /* Clear event flag - do not use in interrupt */
  virtual void clear(uint8_t flagNo) const = 0;

  /* Wait for event flag to be set - do not use in interrupt */
  virtual void waitForSet(uint8_t flagNo) const = 0;

  virtual ~EventFlagsInterface(void) {};
};
