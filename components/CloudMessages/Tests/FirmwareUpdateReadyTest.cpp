#include "CppUTest/TestHarness.h"
#include <cstring>
#include "FirmwareUpdateReady.h"


static FrameParser * frame;
static FirmwareUpdateReady * fwUpdate;

TEST_GROUP(fwUpdateReadyFrame)
{
  TEST_SETUP()
  {
    uint8_t data[] = {
        0x00, 0x01, // frame type (MSB)
        0x00, 0x02, // retry count (MSB)
        0x00, 0x0f, // url length (MSB)
        'h', 't', 't', 'p', 's', ':', '/', '/', 'u', 'r', 'l', '.', 'c', 'o', 'm'
    };

    uint32_t size = sizeof(data);
    frame = new FrameParser(data, size);
    fwUpdate = new FirmwareUpdateReady(*frame);
  }

  TEST_TEARDOWN()
  {
    delete fwUpdate;
    delete frame;
  }
};

TEST(fwUpdateReadyFrame, testIsValidReturnsTrue)
{
  CHECK_TRUE(fwUpdate->isValid());
}

TEST(fwUpdateReadyFrame, testValidDataReturnedForRetryCount)
{
  uint16_t expectedData = 0x0002; // Two retries
  CHECK_EQUAL(expectedData, fwUpdate->getRetryCount());
}

TEST(fwUpdateReadyFrame, testValidDataReturnedForUrlLength)
{
  uint8_t expectedData = 15; // note: does not include required '\0'
  CHECK_EQUAL(expectedData, fwUpdate->getUrlLength());
}

TEST(fwUpdateReadyFrame, testValidDataReturnedForUrl)
{
  char urlString[16U];
  memset(urlString, '1', 16); // ensures that no zero-term char is present before copy

  uint16_t urlLength = fwUpdate->getUrlString(urlString, 16U);

  CHECK_EQUAL(15U, urlLength);
  CHECK_EQUAL(15U, strlen(urlString));
  STRCMP_EQUAL("https://url.com", urlString);
}


TEST_GROUP(notFwUpdateReadyFrame)
{
  TEST_SETUP()
  {
    uint8_t data[] = { 0x00, 0x07, 0x03, 0x09, 0x08, 0x07 };
    uint32_t size = sizeof(data);
    frame = new FrameParser(data, size);
    fwUpdate = new FirmwareUpdateReady(*frame);
  }

  TEST_TEARDOWN()
  {
    delete fwUpdate;
    delete frame;
  }
};

TEST(notFwUpdateReadyFrame, testIsValidReturnsFalse)
{
  CHECK_FALSE(fwUpdate->isValid());
}

TEST(notFwUpdateReadyFrame, testNoUrlIsReturned)
{
  char urlString[16U];
  memset(urlString, '1', 16); // ensures that no zero-term char is present before copy

  uint16_t urlLength = fwUpdate->getUrlString(urlString, 16U);
  CHECK_EQUAL(0U, urlLength);
}

