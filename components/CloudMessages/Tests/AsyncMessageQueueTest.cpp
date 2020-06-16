#include "CppUTest/TestHarness.h"
#include "AsyncMessageQueue.h"
#include "Constants.h"


static AsyncMessageQueue * queue;

TEST_GROUP(AsyncMessageQueue)
{
  TEST_SETUP()
  {
    queue = new AsyncMessageQueue();
  }

  TEST_TEARDOWN()
  {
    delete queue;
  }
};

TEST(AsyncMessageQueue, testQueueIsInitiallyEmpty)
{
  CHECK_TRUE(queue->isEmpty());
}

TEST(AsyncMessageQueue, testPopItemSizeIsZeroIfEmpty)
{
  CHECK_EQUAL(0U, queue->getPopItemSize());
}

TEST(AsyncMessageQueue, testPopReturnsFalseIfEmpty)
{
  uint8_t buffer[1];
  uint32_t size = 1U;
  CHECK_FALSE(queue->popItem(buffer, size));
}

TEST(AsyncMessageQueue, testPushReturnsTrueWhenMessageIsAdded)
{
  uint8_t buffer[] = "Message";
  CHECK_TRUE(queue->pushItem(buffer, sizeof(buffer) - 1U));
}

TEST(AsyncMessageQueue, testIsEmptyIsTrueAfterAddingAMessage)
{
  uint8_t buffer[] = "Message";
  queue->pushItem(buffer, sizeof(buffer) - 1U);
  CHECK_FALSE(queue->isEmpty());
}


TEST(AsyncMessageQueue, testPushSucceedsUntilQueueIsFullThenFalse)
{
  uint8_t buffer[] = "Message";

  for (uint8_t i = 0U; i < Constants::CLOUD_ASYNC_MSG_QUEUE_SIZE; ++i)
  {
    CHECK_TRUE(queue->pushItem(buffer, sizeof(buffer) - 1U));
  }

  CHECK_FALSE(queue->pushItem(buffer, sizeof(buffer) - 1U));
}

TEST(AsyncMessageQueue, testIsFullReturnsFalseWhenNotFullThenTrueWhenFull)
{
  uint8_t buffer[] = "Message";

  for (uint8_t i = 0U; i < Constants::CLOUD_ASYNC_MSG_QUEUE_SIZE - 1U; ++i)
  {
    queue->pushItem(buffer, sizeof(buffer) - 1U);
    CHECK_FALSE(queue->isFull());
  }

  queue->pushItem(buffer, sizeof(buffer) - 1U);
  CHECK_TRUE(queue->isFull());
}

TEST(AsyncMessageQueue, testGetPopSizeReturnsCorrectSizeIncludingZeroTerm)
{
  uint8_t buffer[] = "Message";
  uint32_t expectedSize = sizeof(buffer) - 1U;

  queue->pushItem(buffer, sizeof(buffer) - 1U);

  CHECK_EQUAL(expectedSize, queue->getPopItemSize());
}


TEST(AsyncMessageQueue, testPopReturnsTrueWhenMessageRead)
{
  uint8_t buffer[] = "Message";
  uint32_t size = sizeof(buffer) - 1U;

  queue->pushItem(buffer, size);
  CHECK_TRUE(queue->popItem(buffer, size));
}

TEST(AsyncMessageQueue, testPopReturnsFalseWhenSizeTooSmall)
{
  uint8_t buffer[] = "Message";
  uint32_t size = sizeof(buffer) - 1U;

  queue->pushItem(buffer, size);
  uint32_t smallSize = 1U;
  CHECK_FALSE(queue->popItem(buffer, smallSize));
  CHECK_FALSE(queue->isEmpty());
}

TEST(AsyncMessageQueue, testPopNoArgsReturnsTrueWhenMessageRead)
{
  uint8_t buffer[] = "Message";
  uint32_t size = sizeof(buffer) - 1U;

  queue->pushItem(buffer, size);
  CHECK_FALSE(queue->isEmpty());
  CHECK_TRUE(queue->popItem());
  CHECK_TRUE(queue->isEmpty());
}

TEST(AsyncMessageQueue, testPopReturnsCorrectContentsWhenMessageRead)
{
  uint8_t writeBuffer[8U] = "Message";
  uint8_t readBuffer[7U] = {};

  uint32_t size = sizeof(writeBuffer) - 1U;
  queue->pushItem(writeBuffer, size);
  queue->popItem(readBuffer, size);

  MEMCMP_EQUAL(readBuffer, writeBuffer, size);
}

TEST(AsyncMessageQueue, testPopReadsInCorrectOrder)
{
  uint8_t readBuffer[32U] = {};

  uint8_t itemOne[] = { 2, 4, 6, 4, 2 };
  uint8_t itemTwo[] = { 12, 45, 34, 236 };
  uint8_t itemThree[] = { 12, 64, 54, 14, 20 };

  queue->pushItem(itemOne, sizeof(itemOne));
  queue->pushItem(itemTwo, sizeof(itemTwo));
  queue->pushItem(itemThree, sizeof(itemThree));

  uint32_t readSize = sizeof(readBuffer);
  queue->popItem(readBuffer, readSize);
  CHECK_EQUAL(sizeof(itemOne), readSize);
  MEMCMP_EQUAL(itemOne, readBuffer, readSize);

  readSize = sizeof(readBuffer);
  queue->popItem(readBuffer, readSize);
  CHECK_EQUAL(sizeof(itemTwo), readSize);
  MEMCMP_EQUAL(itemTwo, readBuffer, readSize);

  readSize = sizeof(readBuffer);
  queue->popItem(readBuffer, readSize);
  CHECK_EQUAL(sizeof(itemThree), readSize);
  MEMCMP_EQUAL(itemThree, readBuffer, readSize);
}

TEST(AsyncMessageQueue, testPeekReturnsTrueWhenMessageRead)
{
  uint8_t buffer[] = "Message";
  uint32_t size = sizeof(buffer) - 1U;

  queue->pushItem(buffer, size);
  CHECK_TRUE(queue->peekItem(buffer, size));
}

TEST(AsyncMessageQueue, testPeekReturnsFalseWhenSizeTooSmall)
{
  uint8_t buffer[] = "Message";
  uint32_t size = sizeof(buffer) - 1U;

  queue->pushItem(buffer, size);
  uint32_t smallSize = 1U;
  CHECK_FALSE(queue->peekItem(buffer, smallSize));
  CHECK_FALSE(queue->isEmpty());
}

TEST(AsyncMessageQueue, testPeekReturnsCorrectContentsWhenMessageRead)
{
  uint8_t writeBuffer[8U] = "Message";
  uint8_t readBuffer[7U] = {};

  uint32_t size = sizeof(writeBuffer) - 1U;
  queue->pushItem(writeBuffer, size);
  queue->peekItem(readBuffer, size);

  MEMCMP_EQUAL(readBuffer, writeBuffer, size);
}

TEST(AsyncMessageQueue, testPeekDoesNotRemoveItem)
{
  uint8_t writeBuffer[8U] = "Message";
  uint8_t readBuffer[7U] = {};

  uint32_t size = sizeof(writeBuffer) - 1U;
  CHECK_TRUE(queue->isEmpty());
  queue->pushItem(writeBuffer, size);
  CHECK_FALSE(queue->isEmpty());
  queue->peekItem(readBuffer, size);
  CHECK_FALSE(queue->isEmpty());
}

TEST(AsyncMessageQueue, testWrapping)
{
  uint8_t dummyData[] = { 1, 2, 3 };

  for (uint8_t i = 0U; i < Constants::CLOUD_ASYNC_MSG_QUEUE_SIZE; ++i)
  {
    queue->pushItem(dummyData, sizeof(dummyData));
  }

  CHECK_FALSE(queue->isEmpty());
  CHECK_TRUE(queue->isFull());

  // Read a message to get room for one more
  uint8_t readBuffer[32U];
  uint32_t readSize = sizeof(readBuffer);
  queue->popItem(readBuffer, readSize);

  // Add a new message to the now vacant spot
  uint8_t moreDummyData[] = { 3, 2, 1 };
  CHECK_TRUE(queue->pushItem(moreDummyData, sizeof(moreDummyData)));
  CHECK_TRUE(queue->isFull());
  CHECK_FALSE(queue->isEmpty());

  // Read until empty, the last message read should be 'newMessage'
  uint8_t messagesRead = 0U;

  while (!queue->isEmpty())
  {
    readSize = sizeof(readBuffer);
    CHECK_TRUE(queue->popItem(readBuffer, readSize));
    CHECK_FALSE(queue->isFull());
    ++messagesRead;

    if (messagesRead < Constants::CLOUD_ASYNC_MSG_QUEUE_SIZE)
    {
      MEMCMP_EQUAL(readBuffer, dummyData, readSize);
    }
    else
    {
      MEMCMP_EQUAL(readBuffer, moreDummyData, readSize);
    }
  }

  CHECK_EQUAL(Constants::CLOUD_ASYNC_MSG_QUEUE_SIZE, messagesRead);

  // At last, we can write and read again
  CHECK_TRUE(queue->pushItem(dummyData, sizeof(dummyData)));

  readSize = sizeof(readBuffer);
  CHECK_TRUE(queue->popItem(readBuffer, readSize));
  MEMCMP_EQUAL(readBuffer, dummyData, readSize);

  CHECK_TRUE(queue->isEmpty());
}

