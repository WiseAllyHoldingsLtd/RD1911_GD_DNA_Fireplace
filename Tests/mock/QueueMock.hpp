#pragma once

#include "CppUTestExt\MockSupport.h"

#include "QueueInterface.hpp"

#define NAME(method) "QueueMock::" method


template<class T>
class QueueMock : public QueueInterface<T>
{
  virtual bool push(const T &item, uint32_t timeout=portMAX_DELAY)
  {
    return mock().actualCall(NAME("push"))
        .withParameter("item", item)
        .withParameter("timeout", timeout)
        .returnBoolValueOrDefault(true);
  }
  virtual bool pop(T &item, uint32_t timeout=portMAX_DELAY)
  {
    return mock().actualCall(NAME("pop"))
        .withOutputParameter("item", static_cast<void*>(&item))
        .withParameter("timeout", timeout)
        .returnBoolValueOrDefault(true);
  }

  virtual uint32_t numWaitingItems()
  {
    return mock().actualCall(NAME("numWaitingItems"))
        .returnUnsignedIntValue();
  }
  virtual uint32_t numAvailableSpace()
  {
    return mock().actualCall(NAME("numAvailableSpace"))
        .returnUnsignedIntValue();
  }
  virtual void reset()
  {
    mock().actualCall(NAME("reset"));
  }

};

#undef NAME
#define NAME(method) "QueueMockStruct::" method

template<typename T>
class QueueMockStruct : public QueueInterface<T>
{
  virtual bool push(const T &item, uint32_t timeout=portMAX_DELAY)
  {
    return mock().actualCall(NAME("push"))
        .withConstPointerParameter("item", &item)
        .withParameter("timeout", timeout)
        .returnBoolValueOrDefault(true);
  }
  virtual bool pop(T &item, uint32_t timeout=portMAX_DELAY)
  {
    return mock().actualCall(NAME("pop"))
        .withOutputParameter("item", static_cast<void*>(&item))
        .withParameter("timeout", timeout)
        .returnBoolValueOrDefault(true);
  }

  virtual uint32_t numWaitingItems()
  {
    return mock().actualCall(NAME("numWaitingItems"))
        .returnUnsignedIntValue();
  }
  virtual uint32_t numAvailableSpace()
  {
    return mock().actualCall(NAME("numAvailableSpace"))
        .returnUnsignedIntValue();
  }
  virtual void reset()
  {
    mock().actualCall(NAME("reset"));
  }
};

#undef NAME
