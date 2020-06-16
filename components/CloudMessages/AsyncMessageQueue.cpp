#include "AsyncMessageQueue.h"
#include <cstring>


AsyncMessageQueue::AsyncMessageQueue(void)
  : m_readPos(0U), m_availableSpace(Constants::CLOUD_ASYNC_MSG_QUEUE_SIZE)
{
  for (uint8_t i = 0U; i < Constants::CLOUD_ASYNC_MSG_QUEUE_SIZE; ++i)
  {
    m_queue[i] = nullptr;
  }
}

AsyncMessageQueue::~AsyncMessageQueue(void)
{
  for (uint8_t i = 0U; i < Constants::CLOUD_ASYNC_MSG_QUEUE_SIZE; ++i)
  {
    removeItem(i);
  }
}

bool AsyncMessageQueue::pushItem(const uint8_t * bytes, uint32_t size)
{
  m_lock.take();
  bool canPush = ((!isFull()) && (size > 0U) && (size <= Constants::CLOUD_RESPONSE_MSG_MAX_SIZE));

  if (canPush)
  {
    uint8_t pushIndex = ((m_readPos + getNumOfItems()) % Constants::CLOUD_ASYNC_MSG_QUEUE_SIZE);
    addItem(pushIndex, bytes, size);
  }

  m_lock.give();
  return canPush;
}

bool AsyncMessageQueue::isFull(void) const
{
  return (m_availableSpace == 0U);
}

bool AsyncMessageQueue::popItem(uint8_t * bytes, uint32_t & size)
{
  m_lock.take();
  bool canPop = (!isEmpty() && (bytes != nullptr));

  if (canPop)
  {
    canPop = (size >= (m_queue[m_readPos]->size));

    if (canPop)
    {
      memcpy(bytes, m_queue[m_readPos]->bytes, m_queue[m_readPos]->size);
      size = m_queue[m_readPos]->size;
      removeItem(m_readPos);
      m_readPos = ((m_readPos + 1U) % Constants::CLOUD_ASYNC_MSG_QUEUE_SIZE);
    }
  }

  m_lock.give();
  return canPop;
}

bool AsyncMessageQueue::popItem()
{
  m_lock.take();
  bool canPop = !isEmpty();

  if (canPop)
  {
    removeItem(m_readPos);
    m_readPos = ((m_readPos + 1U) % Constants::CLOUD_ASYNC_MSG_QUEUE_SIZE);
  }

  m_lock.give();
  return canPop;
}

bool AsyncMessageQueue::peekItem(uint8_t * bytes, uint32_t &size)
{
  m_lock.take();
  bool canPeek = (!isEmpty() && (bytes != nullptr));

  if (canPeek)
  {
    canPeek = (size >= (m_queue[m_readPos]->size));

    if (canPeek)
    {
      memcpy(bytes, m_queue[m_readPos]->bytes, m_queue[m_readPos]->size);
      size = m_queue[m_readPos]->size;
    }
  }

  m_lock.give();
  return canPeek;
}

uint32_t AsyncMessageQueue::getPopItemSize(void)
{
  m_lock.take();
  uint8_t size = 0U;

  if (m_queue[m_readPos] != nullptr)
  {
    size = m_queue[m_readPos]->size;
  }

  m_lock.give();
  return size;
}

bool AsyncMessageQueue::isEmpty(void) const
{
  return (m_availableSpace == Constants::CLOUD_ASYNC_MSG_QUEUE_SIZE);
}


uint8_t AsyncMessageQueue::getNumOfItems(void) const
{
  return (Constants::CLOUD_ASYNC_MSG_QUEUE_SIZE - m_availableSpace);
}


void AsyncMessageQueue::addItem(uint8_t itemIdx, const uint8_t * itemData, uint32_t size)
{
  if (m_queue[itemIdx] != nullptr)
  {
    removeItem(itemIdx);
  }

  m_queue[itemIdx] = new StorageItem();
  m_queue[itemIdx]->bytes = new uint8_t[size];
  memcpy(m_queue[itemIdx]->bytes, itemData, size);
  m_queue[itemIdx]->size = size;
  --m_availableSpace;
}


void AsyncMessageQueue::removeItem(uint8_t itemIdx)
{
  if (m_queue[itemIdx] != nullptr)
  {
    delete [] m_queue[itemIdx]->bytes;
    delete m_queue[itemIdx];
    m_queue[itemIdx] = nullptr;
    ++m_availableSpace;
  }
}
