#pragma once

#include "FreeRTOS.hpp"
#include "EventFlagsInterface.hpp"


class EventFlags : public EventFlagsInterface
{
public:
  EventFlags(void)
  {
      m_handle = xEventGroupCreate();
  }

  virtual bool get(uint8_t flagNo) const
  {
    bool returnValue = false;

    if (isWithinRange(flagNo))
    {
      EventBits_t flags = xEventGroupGetBits(m_handle);
      returnValue = static_cast<bool>((flags & (1U << flagNo)));
    }

    return returnValue;
  }

  virtual void set(uint8_t flagNo) const
  {
    if (isWithinRange(flagNo))
    {
      xEventGroupSetBits(m_handle, (1U << flagNo));
    }
  }

  virtual void clear(uint8_t flagNo) const
  {
    if (isWithinRange(flagNo))
    {
      xEventGroupClearBits(m_handle, (1U << flagNo));
    }
  }

  virtual void waitForSet(uint8_t flagNo) const
  {
    if (isWithinRange(flagNo))
    {
      xEventGroupWaitBits(m_handle, (1U << flagNo), pdFALSE, pdTRUE, portMAX_DELAY);
    }
  }

  virtual bool waitForSet(uint8_t flagNo, uint32_t timeToWaitMs) const
  {
    bool wasSet = false;

    if (isWithinRange(flagNo))
    {
      EventBits_t resultBits = xEventGroupWaitBits(m_handle, (1U << flagNo), pdFALSE, pdTRUE, timeToWaitMs / portTICK_PERIOD_MS);
      wasSet = (resultBits & (1U << flagNo));
    }

    return wasSet;
  }

  virtual ~EventFlags()
  {
    vEventGroupDelete(m_handle);
  }

private:
  bool isWithinRange(uint8_t flagNo) const
  {
    return (flagNo < maxNumberOfFlags);
  }

  EventGroupHandle_t m_handle;

#if configUSE_16_BIT_TICKS == 1
  const uint8_t maxNumberOfFlags = 16U;
#else
  const uint8_t maxNumberOfFlags = 32U;
#endif
};
