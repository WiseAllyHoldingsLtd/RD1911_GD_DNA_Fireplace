#pragma once

#include "Constants.h"
#include "AsyncMessageQueueInterface.h"
#include "Mutex.hpp"


struct StorageItem
{
  uint8_t * bytes;
  uint32_t size;
};


class AsyncMessageQueue : public AsyncMessageQueueWriteInterface, public AsyncMessageQueueReadInterface
{
public:
  AsyncMessageQueue(void);
  virtual ~AsyncMessageQueue(void);

  virtual bool pushItem(const uint8_t * bytes, uint32_t size);
  virtual bool isFull(void) const;

  virtual bool popItem(uint8_t * bytes, uint32_t & size);
  virtual bool popItem();
  virtual uint32_t getPopItemSize(void);
  virtual bool peekItem(uint8_t * bytes, uint32_t &size);
  virtual bool isEmpty(void) const;

private:
  uint8_t getNumOfItems(void) const;
  void addItem(uint8_t itemIdx, const uint8_t * itemData, uint32_t size);
  void removeItem(uint8_t itemIdx);

  uint8_t m_readPos;
  uint8_t m_availableSpace;

  StorageItem * m_queue[Constants::CLOUD_ASYNC_MSG_QUEUE_SIZE];
  Mutex m_lock;
};
