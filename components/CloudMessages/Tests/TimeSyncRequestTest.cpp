#include "CppUTest/TestHarness.h"
#include "TimeSyncRequest.h"


TEST_GROUP(TimeSyncRequest)
{
  TimeSyncRequest timeSyncRequest;

  TEST_SETUP()
  {
  }

  TEST_TEARDOWN()
  {
  }
};

TEST(TimeSyncRequest, testSize)
{
  CHECK_EQUAL(2U, timeSyncRequest.getSize());
}

TEST(TimeSyncRequest, testData)
{
  uint8_t buffer[2];
  timeSyncRequest.getBytes(buffer, sizeof(buffer)); // FrameType = 0x0007, MSB = 0x00

  CHECK_EQUAL(0x00, buffer[0]);
  CHECK_EQUAL(0x07, buffer[1]);
}
