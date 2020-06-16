#pragma once
#include <cstdint>


class AsyncMessageQueueWriteInterface
{
public:
  virtual ~AsyncMessageQueueWriteInterface(void) {}
  virtual bool pushItem(const uint8_t * bytes, uint32_t size) = 0;
  virtual bool isFull(void) const = 0;
};


class AsyncMessageQueueReadInterface
{
public:
  virtual ~AsyncMessageQueueReadInterface(void) {}

  virtual bool popItem(uint8_t * bytes, uint32_t & size) = 0;
  virtual bool popItem() = 0;
  virtual uint32_t getPopItemSize(void) = 0;
  virtual bool peekItem(uint8_t * bytes, uint32_t &size) = 0;

  virtual bool isEmpty(void) const = 0;
};
