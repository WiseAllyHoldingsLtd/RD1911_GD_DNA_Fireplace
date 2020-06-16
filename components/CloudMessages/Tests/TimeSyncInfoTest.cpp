#include "CppUTest/TestHarness.h"

#include "TimeSyncInfo.h"


static FrameParser * frame;
static TimeSyncInfo * timeSync;

TEST_GROUP(timeSyncInfoFrame)
{
  TEST_SETUP()
  {
    uint8_t data[11U] = { 0x00, 0x08, 0xbb, 0xc5, 0x1b, 0x5b, 0x05, 0x20, 0x1c, 0x00, 0x00 };
    uint32_t size = sizeof(data);
    frame = new FrameParser(data, size);
    timeSync = new TimeSyncInfo(*frame);
  }

  TEST_TEARDOWN()
  {
    delete timeSync;
    delete frame;
  }
};

TEST(timeSyncInfoFrame, testIsValid)
{
  CHECK_TRUE(timeSync->isValid());
}

TEST(timeSyncInfoFrame, testValidDataReturnedForUnixTime)
{
  uint32_t expectedData = 0x5b1bc5bb; // Sat, 09 Jun 2018 12:19:07 +0000
  CHECK_EQUAL(expectedData, timeSync->getUnixTime());
}

TEST(timeSyncInfoFrame, testValidDataReturnedForDayOfWeek)
{
  uint8_t expectedData = 0x05; // Saturday
  CHECK_EQUAL(expectedData, timeSync->getDayOfWeek());
}

TEST(timeSyncInfoFrame, testValidDataReturnedForUtcOffset)
{
  int32_t expectedData = 0x1c20; // 7200, two hours
  CHECK_EQUAL(expectedData, timeSync->getUtcOffset());
}

TEST(timeSyncInfoFrame, testValidDataReturnedForNegativeUtcOffset)
{
  uint8_t theData[] = { 0x00, 0x08, 0xbb, 0xc5, 0x1b, 0x5b, 0x05, 0xf0, 0xf1, 0xff, 0xff };
  FrameParser theFrame(theData, sizeof(theData));
  TimeSyncInfo theTimeSync(theFrame);

  int32_t expectedData = 0xfffff1f0; // -3600, two hours
  CHECK_EQUAL(expectedData, theTimeSync.getUtcOffset());
}


TEST_GROUP(notTimeSyncInfoFrame)
{
  TEST_SETUP()
  {
    uint8_t data[6U] = { 0x00, 0x07, 0x03, 0x09, 0x08, 0x07 };
    uint32_t size = sizeof(data);
    frame = new FrameParser(data, size);
    timeSync = new TimeSyncInfo(*frame);
  }

  TEST_TEARDOWN()
  {
    delete timeSync;
    delete frame;
  }
};

TEST(notTimeSyncInfoFrame, testIsValidReturnsFalse)
{
  CHECK_FALSE(timeSync->isValid());
}


