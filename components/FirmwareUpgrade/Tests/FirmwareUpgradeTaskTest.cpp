#include "assert.h"
#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include "FirmwareUpgradeTask.hpp"
#include "mock/FirmwareUpgradeControllerMock.h"


static FirmwareUpgradeControllerMock controller;
static FirmwareUpgradeTask *task;


TEST_GROUP(firmwareUpgradeInit)
{
  TEST_SETUP()
  {
    task = new FirmwareUpgradeTask(controller);
  }

  TEST_TEARDOWN()
  {
    delete task;
    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
  }
};


TEST_GROUP(firmwareUpgrade)
{
  const char providedUrl[11U] = "urlToNewFw";

  TEST_SETUP()
  {
    task = new FirmwareUpgradeTask(controller);
    task->startTask();
    mock().expectOneCall("FirmwareUpgradeControllerMock::isFirmwareUpgradeRequested").andReturnValue(true);
  }

  TEST_TEARDOWN()
  {
    delete task;
    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
  }
};


TEST(firmwareUpgradeInit, upgradeStateIsInitiallyIdle)
{
  mock().ignoreOtherCalls();
  CHECK_EQUAL(FirmwareUpgradeState::notRequested, task->getCurrentState());
}

TEST(firmwareUpgradeInit, taskReportsStartedIfStartedWhenUpgradeIsntRequested)
{
  task->startTask();

  mock().expectOneCall("FirmwareUpgradeControllerMock::isFirmwareUpgradeRequested").andReturnValue(false);
  task->run(false);

  CHECK_EQUAL(FirmwareUpgradeState::started, task->getCurrentState());
  CHECK_EQUAL(FirmwareUpgradeState::started, task->getCurrentState());
  CHECK_EQUAL(FirmwareUpgradeState::started, task->getCurrentState());
}

TEST(firmwareUpgrade, taskReportsFailureIfOTABeginFails)
{
  mock().expectOneCall("FirmwareUpgradeControllerMock::initOTA").andReturnValue(false);
  mock().expectNoCall("FirmwareUpgradeControllerMock::commitOTA");
  task->run(false);

  CHECK_EQUAL(FirmwareUpgradeState::failed, task->getCurrentState());
}

TEST(firmwareUpgrade, firmwareUpgradeFailsIfFileSizeCannotBeDecided)
{
  mock().expectOneCall("FirmwareUpgradeControllerMock::initOTA");
  mock().expectOneCall("FirmwareUpgradeControllerMock::getOTADataSize").andReturnValue(0U);
  mock().ignoreOtherCalls();

  task->run(false);
  CHECK_EQUAL(FirmwareUpgradeState::failed, task->getCurrentState());
}

TEST(firmwareUpgrade, firmwareUpgradeWithFileLessThanChunkSize)
{
  const uint32_t fileSize = 10U;
  assert(fileSize < Constants::FW_DOWNLOAD_CHUNK_SIZE);

  mock().expectOneCall("FirmwareUpgradeControllerMock::initOTA");
  mock().expectOneCall("FirmwareUpgradeControllerMock::getOTADataSize").andReturnValue(fileSize);

  mock().expectOneCall("FirmwareUpgradeControllerMock::downloadAndWriteOTAData")
      .withUnsignedIntParameter("startByteIndex", 0U)
      .withUnsignedIntParameter("endByteIndex", fileSize - 1U)  // Inclusive bounds => fileSize - 1U
      .withUnsignedIntParameter("numOfDownloadAttempts", Constants::FW_DOWNLOAD_ATTEMPTS_MAX);

  mock().ignoreOtherCalls();
  task->run(false);
}

TEST(firmwareUpgrade, firmwareUpgradeWithFileSizeEqualToChunkSize)
{
  const uint32_t fileSize = Constants::FW_DOWNLOAD_CHUNK_SIZE;

  mock().expectOneCall("FirmwareUpgradeControllerMock::initOTA");
  mock().expectOneCall("FirmwareUpgradeControllerMock::getOTADataSize").andReturnValue(fileSize);

  mock().expectOneCall("FirmwareUpgradeControllerMock::downloadAndWriteOTAData")
      .withUnsignedIntParameter("startByteIndex", 0U)
      .withUnsignedIntParameter("endByteIndex", fileSize - 1U)
      .withUnsignedIntParameter("numOfDownloadAttempts", Constants::FW_DOWNLOAD_ATTEMPTS_MAX);

  mock().ignoreOtherCalls();
  task->run(false);
}

TEST(firmwareUpgrade, firmwareUpgradeWithFileSizeOneLargerThanChunkSize)
{
  const uint32_t fileSize = Constants::FW_DOWNLOAD_CHUNK_SIZE + 1U;

  mock().expectOneCall("FirmwareUpgradeControllerMock::initOTA");
  mock().expectOneCall("FirmwareUpgradeControllerMock::getOTADataSize").andReturnValue(fileSize);

  mock().expectOneCall("FirmwareUpgradeControllerMock::downloadAndWriteOTAData")
      .withUnsignedIntParameter("startByteIndex", 0U)
      .withUnsignedIntParameter("endByteIndex", Constants::FW_DOWNLOAD_CHUNK_SIZE - 1U)
      .withUnsignedIntParameter("numOfDownloadAttempts", Constants::FW_DOWNLOAD_ATTEMPTS_MAX);

  mock().expectOneCall("FirmwareUpgradeControllerMock::downloadAndWriteOTAData")
      .withUnsignedIntParameter("startByteIndex", Constants::FW_DOWNLOAD_CHUNK_SIZE)
      .withUnsignedIntParameter("endByteIndex", Constants::FW_DOWNLOAD_CHUNK_SIZE)
      .withUnsignedIntParameter("numOfDownloadAttempts", Constants::FW_DOWNLOAD_ATTEMPTS_MAX);

  mock().ignoreOtherCalls();
  task->run(false);
}

TEST(firmwareUpgrade, firmwareUpgradeWithFileSizeSpanningMultipleChunks)
{
  const uint32_t fileSize = (3 * Constants::FW_DOWNLOAD_CHUNK_SIZE) + (Constants::FW_DOWNLOAD_CHUNK_SIZE / 2U);

  mock().expectOneCall("FirmwareUpgradeControllerMock::initOTA");
  mock().expectOneCall("FirmwareUpgradeControllerMock::getOTADataSize").andReturnValue(fileSize);

  mock().expectOneCall("FirmwareUpgradeControllerMock::downloadAndWriteOTAData")
      .withUnsignedIntParameter("startByteIndex", 0U)
      .withUnsignedIntParameter("endByteIndex", Constants::FW_DOWNLOAD_CHUNK_SIZE - 1U)
      .withUnsignedIntParameter("numOfDownloadAttempts", Constants::FW_DOWNLOAD_ATTEMPTS_MAX);

  mock().expectOneCall("FirmwareUpgradeControllerMock::downloadAndWriteOTAData")
      .withUnsignedIntParameter("startByteIndex", Constants::FW_DOWNLOAD_CHUNK_SIZE)
      .withUnsignedIntParameter("endByteIndex", (2 * Constants::FW_DOWNLOAD_CHUNK_SIZE) - 1U)
      .withUnsignedIntParameter("numOfDownloadAttempts", Constants::FW_DOWNLOAD_ATTEMPTS_MAX);

  mock().expectOneCall("FirmwareUpgradeControllerMock::downloadAndWriteOTAData")
      .withUnsignedIntParameter("startByteIndex", 2 * Constants::FW_DOWNLOAD_CHUNK_SIZE)
      .withUnsignedIntParameter("endByteIndex", (3 * Constants::FW_DOWNLOAD_CHUNK_SIZE) - 1U)
      .withUnsignedIntParameter("numOfDownloadAttempts", Constants::FW_DOWNLOAD_ATTEMPTS_MAX);

  mock().expectOneCall("FirmwareUpgradeControllerMock::downloadAndWriteOTAData")
      .withUnsignedIntParameter("startByteIndex", 3 * Constants::FW_DOWNLOAD_CHUNK_SIZE)
      .withUnsignedIntParameter("endByteIndex", (3 * Constants::FW_DOWNLOAD_CHUNK_SIZE) + (Constants::FW_DOWNLOAD_CHUNK_SIZE / 2U) - 1U)
      .withUnsignedIntParameter("numOfDownloadAttempts", Constants::FW_DOWNLOAD_ATTEMPTS_MAX);

  mock().ignoreOtherCalls();
  task->run(false);
}

TEST(firmwareUpgrade, firmwareUpgradeFailsWhenDownloadAndWriteFailsOnFirstChunk)
{
  const uint32_t fileSize = 10U;
  assert(fileSize < Constants::FW_DOWNLOAD_CHUNK_SIZE);

  mock().expectOneCall("FirmwareUpgradeControllerMock::initOTA");
  mock().expectOneCall("FirmwareUpgradeControllerMock::getOTADataSize").andReturnValue(fileSize);

  mock().expectOneCall("FirmwareUpgradeControllerMock::downloadAndWriteOTAData")
    .ignoreOtherParameters()
    .andReturnValue(false);

  mock().ignoreOtherCalls();
  mock().expectNoCall("FirmwareUpgradeControllerMock::commitOTA");

  task->run(false);
  CHECK_EQUAL(FirmwareUpgradeState::failed, task->getCurrentState());
}

TEST(firmwareUpgrade, firmwareUpgradeFailsWhenDownloadAndWriteFailsOnSecondChunk)
{
  const uint32_t fileSize = Constants::FW_DOWNLOAD_CHUNK_SIZE + 1U;

  mock().expectOneCall("FirmwareUpgradeControllerMock::initOTA");
  mock().expectOneCall("FirmwareUpgradeControllerMock::getOTADataSize").andReturnValue(fileSize);

  mock().expectOneCall("FirmwareUpgradeControllerMock::downloadAndWriteOTAData")
      .ignoreOtherParameters()
      .andReturnValue(true);

  mock().expectOneCall("FirmwareUpgradeControllerMock::downloadAndWriteOTAData")
          .ignoreOtherParameters()
          .andReturnValue(false);

  mock().ignoreOtherCalls();
  mock().expectNoCall("FirmwareUpgradeControllerMock::commitOTA");

  task->run(false);
  CHECK_EQUAL(FirmwareUpgradeState::failed, task->getCurrentState());
}

TEST(firmwareUpgrade, firmwareUpgradeIsCommittedWhenDownloadAndWriteOfSmallFileSucceeds)
{
  const uint32_t fileSize = 10U;
  assert(fileSize < Constants::FW_DOWNLOAD_CHUNK_SIZE);

  mock().expectOneCall("FirmwareUpgradeControllerMock::initOTA");
  mock().expectOneCall("FirmwareUpgradeControllerMock::getOTADataSize").andReturnValue(fileSize);

  mock().expectOneCall("FirmwareUpgradeControllerMock::downloadAndWriteOTAData")
    .ignoreOtherParameters()
    .andReturnValue(true);

  mock().expectOneCall("FirmwareUpgradeControllerMock::commitOTA");
  mock().ignoreOtherCalls();

  task->run(false);
}

TEST(firmwareUpgrade, firmwareUpgradeIsCommittedWhenDownloadAndWriteOfLargeFileSucceeds)
{
  const uint32_t fileSize = 100 * Constants::FW_DOWNLOAD_CHUNK_SIZE;

  mock().expectOneCall("FirmwareUpgradeControllerMock::initOTA");
  mock().expectOneCall("FirmwareUpgradeControllerMock::getOTADataSize").andReturnValue(fileSize);

  mock().expectNCalls(100, "FirmwareUpgradeControllerMock::downloadAndWriteOTAData")
    .ignoreOtherParameters()
    .andReturnValue(true);

  mock().expectOneCall("FirmwareUpgradeControllerMock::commitOTA");
  mock().ignoreOtherCalls();

  task->run(false);
}

TEST(firmwareUpgrade, firmwareUpgradeSucceedsIfFileIsCommitted)
{
  mock().expectOneCall("FirmwareUpgradeControllerMock::initOTA");
  mock().expectOneCall("FirmwareUpgradeControllerMock::getOTADataSize").andReturnValue(1U);
  mock().expectOneCall("FirmwareUpgradeControllerMock::downloadAndWriteOTAData")
        .ignoreOtherParameters()
        .andReturnValue(true);

  mock().expectOneCall("FirmwareUpgradeControllerMock::commitOTA").andReturnValue(true);
  mock().ignoreOtherCalls();

  task->run(false);
  CHECK_EQUAL(FirmwareUpgradeState::completed, task->getCurrentState());
}

TEST(firmwareUpgrade, firmwareUpgradeSucceedsIfFileCommitFails)
{
  mock().expectOneCall("FirmwareUpgradeControllerMock::initOTA");
  mock().expectOneCall("FirmwareUpgradeControllerMock::getOTADataSize").andReturnValue(1U);
  mock().expectOneCall("FirmwareUpgradeControllerMock::downloadAndWriteOTAData")
        .ignoreOtherParameters()
        .andReturnValue(true);

  mock().expectOneCall("FirmwareUpgradeControllerMock::commitOTA").andReturnValue(false);
  mock().ignoreOtherCalls();

  task->run(false);
  CHECK_EQUAL(FirmwareUpgradeState::failed, task->getCurrentState());
}
