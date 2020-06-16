#pragma once

#include "TimerDriverInterface.h"
#include "CppUTestExt\MockSupport.h"


class TimerDriverMock : public TimerDriverInterface
{
public:
  TimerDriverMock(void)
  {
  }

  virtual uint64_t getTimeSinceBootMS()
  {
    return mock().actualCall("getTimeSinceBootMS").returnUnsignedLongIntValue(); /* Note: Testing will not work for values larger than 32-bit, because of the test framework. */
  }

  virtual uint64_t getTimeSinceTimestampMS(uint64_t timestamp)
  {
    return mock().actualCall("getTimeSinceTimestampMS")
        .withUnsignedLongIntParameter("timestamp", timestamp)
        .returnUnsignedLongIntValue();
  }

  virtual void waitMS(uint64_t milliseconds)
  {
    mock().actualCall("waitMS").withUnsignedLongIntParameter("milliseconds", milliseconds);
  }
};


class TimerDriverDummy : public TimerDriverInterface
{
public:
  TimerDriverDummy(uint64_t startTimeMS, uint64_t incrementsMS)
    : m_currentTime(startTimeMS), m_increment(incrementsMS)
  {
  }

  virtual uint64_t getTimeSinceBootMS()
  {
    uint64_t retval = m_currentTime;
    m_currentTime += m_increment;
    return retval;
  }

  virtual uint64_t getTimeSinceTimestampMS(uint64_t timestampMS)
  {
    return getTimeSinceBootMS() - timestampMS;
  }

  virtual void waitMS(uint64_t milliseconds)
  {
    return;
  }

  void setNewCurrentTime(uint64_t timestampMS)
  {
    m_currentTime = timestampMS;
  }

  void setNewIncrementValue(uint64_t incrementsMS)
  {
    m_increment = incrementsMS;
  }

  void incrementTime(uint64_t increaseMS)
  {
    m_currentTime += increaseMS;
  }

private:
  uint64_t m_currentTime;
  uint64_t m_increment;
};



