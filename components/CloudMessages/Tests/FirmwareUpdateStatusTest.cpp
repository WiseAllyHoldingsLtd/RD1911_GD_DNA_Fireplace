#include "CppUTest/TestHarness.h"
#include "FirmwareUpdateStatus.h"


FirmwareUpdateStatus * fwUpdateStatus;


TEST_GROUP(FirmwareUpdateStatus)
{
  TEST_SETUP()
  {
    fwUpdateStatus = new FirmwareUpdateStatus();
  }

  TEST_TEARDOWN()
  {
    delete fwUpdateStatus;
  }
};

TEST(FirmwareUpdateStatus, testSizeAfterInit)
{
  CHECK_EQUAL(2U, fwUpdateStatus->getSize());
}

TEST(FirmwareUpdateStatus, testFrameIDAfterInit)
{
  uint8_t buffer[2];
  fwUpdateStatus->getBytes(buffer, sizeof(buffer)); // FrameType = 0x0002, MSB = 0x00

  CHECK_EQUAL(0x00, buffer[0]);
  CHECK_EQUAL(0x02, buffer[1]);
}

TEST(FirmwareUpdateStatus, testDataSuccessfullySet)
{
  CHECK_TRUE(fwUpdateStatus->addData(FirmwareUpdateResult::success, 1U, 2U, 3U, 4U));
}

TEST(FirmwareUpdateStatus, testSizeAfterDataHasBeenSet)
{
  fwUpdateStatus->addData(FirmwareUpdateResult::success, 1U, 2U, 3U, 4U);
  CHECK_EQUAL(8U, fwUpdateStatus->getSize());
}

TEST(FirmwareUpdateStatus, testDataHasBeenSetCorrectly)
{
  fwUpdateStatus->addData(FirmwareUpdateResult::success, 1U, 2U, 3U, 4U);
  uint8_t expectedData[] = { 0x00, 0x02, 0x00, 0x01, 0x02, 0x03, 0x00, 0x04 };

  uint8_t buffer[8];
  fwUpdateStatus->getBytes(buffer, sizeof(buffer));

  MEMCMP_EQUAL(expectedData, buffer, 8U);
}
