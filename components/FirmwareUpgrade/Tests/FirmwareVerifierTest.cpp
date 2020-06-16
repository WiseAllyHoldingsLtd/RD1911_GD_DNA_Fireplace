#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include "FirmwareVerifier.h"
#include "SettingsMock.h"



uint32_t getExpectedVersionCode(void)
{
  uint32_t versionCode = 0U;
  versionCode += (Constants::SW_VERSION * 256U);
  versionCode += Constants::SW_TEST_NO;
  return versionCode;
}


static SettingsMock settingsMock;
static FirmwareVerifier *verifier;


TEST_GROUP(FirmwareVerifier)
{
  TEST_SETUP()
  {
    verifier = new FirmwareVerifier(settingsMock);
  }

  TEST_TEARDOWN()
  {
    delete verifier;
    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
  }
};

TEST(FirmwareVerifier, initialValuesIndicateNoUpgrade)
{
  STRCMP_EQUAL("", verifier->getFirmwareUpgradeID());
  CHECK_EQUAL(0U, verifier->getFirmwareVersionCode());
  CHECK_EQUAL(0U, verifier->getFirmwareUpgradeCount());
}

TEST(FirmwareVerifier, initReadsValuesFromSettings)
{
  mock().expectOneCall("getFirmwareUpgradeCount").andReturnValue(3U);
  mock().expectOneCall("getFirmwareVersionCode").andReturnValue(323532U);
  mock().expectOneCall("getFirmwareID").withOutputParameterReturning("id", "firmware.bin", 13U).withUnsignedIntParameter("size", Constants::FW_DOWNLOAD_ID_MAX_LENGTH);

  verifier->init();

  CHECK_EQUAL(3U, verifier->getFirmwareUpgradeCount());
  CHECK_EQUAL(323532U, verifier->getFirmwareVersionCode());
  STRCMP_EQUAL("firmware.bin", verifier->getFirmwareUpgradeID());
}

TEST(FirmwareVerifier, noUpgradeInAction)
{
  mock().expectOneCall("getFirmwareID").withOutputParameterReturning("id", "", 1U).withUnsignedIntParameter("size", Constants::FW_DOWNLOAD_ID_MAX_LENGTH);
  mock().ignoreOtherCalls();
  verifier->init();

  CHECK_FALSE(verifier->isUpgrading());
}

TEST(FirmwareVerifier, upgradeInAction)
{
  mock().expectOneCall("getFirmwareID").withOutputParameterReturning("id", "not-empty", 10U).withUnsignedIntParameter("size", Constants::FW_DOWNLOAD_ID_MAX_LENGTH);
  mock().ignoreOtherCalls();
  verifier->init();

  CHECK_TRUE(verifier->isUpgrading());
}

TEST(FirmwareVerifier, upgradeBlockOnMaxAttempts)
{
  mock().expectOneCall("getFirmwareID").withOutputParameterReturning("id", "not-empty", 10U).withUnsignedIntParameter("size", Constants::FW_DOWNLOAD_ID_MAX_LENGTH);
  mock().expectOneCall("getFirmwareUpgradeCount").andReturnValue(Constants::FW_UPGRADE_MAX_ATTEMPTS);
  mock().ignoreOtherCalls();
  verifier->init();

  CHECK_TRUE(verifier->isUpgradeBlocked());
}

TEST(FirmwareVerifier, upgradeNotBlockedBelowMaxAttempts)
{
  mock().expectOneCall("getFirmwareID").withOutputParameterReturning("id", "not-empty", 10U).withUnsignedIntParameter("size", Constants::FW_DOWNLOAD_ID_MAX_LENGTH);
  mock().expectOneCall("getFirmwareUpgradeCount").andReturnValue(Constants::FW_UPGRADE_MAX_ATTEMPTS - 1U);
  mock().ignoreOtherCalls();
  verifier->init();

  CHECK_FALSE(verifier->isUpgradeBlocked());
}

TEST(FirmwareVerifier, registerUpgradeAttemptFailsOnInvalidInput)
{
  mock().ignoreOtherCalls();
  verifier->init();

  CHECK_FALSE(verifier->registerUpgradeAttempt(nullptr));
  CHECK_FALSE(verifier->registerUpgradeAttempt(""));
  CHECK_FALSE(verifier->registerUpgradeAttempt("ThisIsNoUrl"));
}

TEST(FirmwareVerifier, registerUpgradeAttemptReturnsTrueOnUrlInput)
{
  mock().ignoreOtherCalls();
  verifier->init();

  CHECK_TRUE(verifier->registerUpgradeAttempt("https://url/filename?queryString"));
  CHECK_TRUE(verifier->registerUpgradeAttempt("https://url/filename"));
  CHECK_TRUE(verifier->registerUpgradeAttempt("https://url/"));
  CHECK_TRUE(verifier->registerUpgradeAttempt("https://url"));
}

TEST(FirmwareVerifier, registerUpgradeSetsFirmwareID)
{
  mock().ignoreOtherCalls();
  verifier->init();

  verifier->registerUpgradeAttempt("https://url/filename?queryString");
  STRCMP_EQUAL("/filename", verifier->getFirmwareUpgradeID());

  verifier->registerUpgradeAttempt("https://url/filename");
  STRCMP_EQUAL("/filename", verifier->getFirmwareUpgradeID());

  verifier->registerUpgradeAttempt("https://url/");
  STRCMP_EQUAL("/", verifier->getFirmwareUpgradeID());

  verifier->registerUpgradeAttempt("https://url");
  STRCMP_EQUAL("/url", verifier->getFirmwareUpgradeID());
}


TEST_GROUP(FirmwareVerifierInitialized)
{
  TEST_SETUP()
  {
    mock().expectOneCall("getFirmwareUpgradeCount").andReturnValue(0U);
    mock().expectOneCall("getFirmwareVersionCode").andReturnValue(0U);
    mock().expectOneCall("getFirmwareID").withOutputParameterReturning("id", "", 1U).withUnsignedIntParameter("size", Constants::FW_DOWNLOAD_ID_MAX_LENGTH);

    verifier = new FirmwareVerifier(settingsMock);
    verifier->init();
  }

  TEST_TEARDOWN()
  {
    delete verifier;
    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
  }
};

TEST(FirmwareVerifierInitialized, noUpgradeInAction)
{
  CHECK_FALSE(verifier->isUpgrading());
  CHECK_FALSE(verifier->isUpgradeBlocked());
}

TEST(FirmwareVerifierInitialized, registerUpgradeSetsPropertiesCorrectly)
{
  // As this URL/ FirmwareID is new, is should be set and attempts = 1 and versionCode should be set acc. to current version
  verifier->registerUpgradeAttempt("https://url/to/file.bin?query=string&no=more");

  STRCMP_EQUAL("/file.bin", verifier->getFirmwareUpgradeID());
  CHECK_EQUAL(1U, verifier->getFirmwareUpgradeCount());
  CHECK_EQUAL(getExpectedVersionCode(), verifier->getFirmwareVersionCode());
  CHECK_TRUE(verifier->isUpgrading());
}

TEST(FirmwareVerifierInitialized, registerUpgradeAgainIncreaseAttemptNo)
{
  verifier->registerUpgradeAttempt("https://url/to/file.bin?query=string&no=more");
  verifier->registerUpgradeAttempt("https://url/to/file.bin?query=string&no=more");

  STRCMP_EQUAL("/file.bin", verifier->getFirmwareUpgradeID());
  CHECK_EQUAL(2U, verifier->getFirmwareUpgradeCount());
  CHECK_EQUAL(getExpectedVersionCode(), verifier->getFirmwareVersionCode());
}

TEST(FirmwareVerifierInitialized, registerUpgradeUntilBlocked)
{
  for (uint8_t i = 0U; i < Constants::FW_UPGRADE_MAX_ATTEMPTS; ++i)
  {
    verifier->registerUpgradeAttempt("https://url/to/file.bin?query=string&no=more");
  }

  CHECK_EQUAL(Constants::FW_UPGRADE_MAX_ATTEMPTS, verifier->getFirmwareUpgradeCount());
  CHECK_TRUE(verifier->isUpgradeBlocked());

  CHECK_FALSE(verifier->registerUpgradeAttempt("https://url/to/file.bin?query=string&no=more"));
  CHECK_EQUAL(Constants::FW_UPGRADE_MAX_ATTEMPTS, verifier->getFirmwareUpgradeCount());
}

TEST(FirmwareVerifierInitialized, newUrlSetsNewFirmwareIDAndResetsCounter)
{
  verifier->registerUpgradeAttempt("https://url/to/file.bin?query=string&no=more");
  verifier->registerUpgradeAttempt("https://url/to/file.bin?query=string&no=more");
  verifier->registerUpgradeAttempt("https://url/to/file.bin?query=string&no=more");
  CHECK_EQUAL(3U, verifier->getFirmwareUpgradeCount());

  verifier->registerUpgradeAttempt("https://url/new_file.bin");
  STRCMP_EQUAL("/new_file.bin", verifier->getFirmwareUpgradeID());
  CHECK_EQUAL(1U, verifier->getFirmwareUpgradeCount());
}

TEST(FirmwareVerifierInitialized, newUrlReleasesBlockingState)
{
  for (uint8_t i = 0U; i < Constants::FW_UPGRADE_MAX_ATTEMPTS; ++i)
  {
    verifier->registerUpgradeAttempt("https://url/to/file.bin?query=string&no=more");
  }

  CHECK_TRUE(verifier->isUpgradeBlocked());

  CHECK_TRUE(verifier->registerUpgradeAttempt("https://url/another.bin?"));
  CHECK_FALSE(verifier->isUpgradeBlocked());
  CHECK_EQUAL(1U, verifier->getFirmwareUpgradeCount());
}

TEST(FirmwareVerifierInitialized, storeSavesUpdatedValuesToSettings)
{
  verifier->registerUpgradeAttempt("https://url/to/file.bin?query=string&no=more");
  verifier->registerUpgradeAttempt("https://url/to/file.bin?query=string&no=more");

  mock().expectOneCall("setFirmwareUpgradeCount").withUnsignedIntParameter("value", 2U);
  mock().expectOneCall("setFirmwareVersionCode").withUnsignedIntParameter("value", getExpectedVersionCode());
  mock().expectOneCall("setFirmwareID").withStringParameter("id", "/file.bin");
  verifier->saveToSettings();
}

TEST(FirmwareVerifierInitialized, verifyFailsIfNotUpgrading)
{
  CHECK_FALSE(verifier->verifyUpgrade());
}


TEST_GROUP(FirmwareVerifierUpgrading)
{
  TEST_SETUP()
  {
    mock().expectOneCall("getFirmwareUpgradeCount").andReturnValue(2U);
    mock().expectOneCall("getFirmwareVersionCode").andReturnValue(3U); // lower than current version on purpose
    mock().expectOneCall("getFirmwareID").withOutputParameterReturning("id", "firmware.bin", 13U).withUnsignedIntParameter("size", Constants::FW_DOWNLOAD_ID_MAX_LENGTH);

    verifier = new FirmwareVerifier(settingsMock);
    verifier->init();
  }

  TEST_TEARDOWN()
  {
    delete verifier;
    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
  }
};

TEST(FirmwareVerifierUpgrading, upgradeInAction)
{
  CHECK_TRUE(verifier->isUpgrading());
  CHECK_FALSE(verifier->isUpgradeBlocked());
}

TEST(FirmwareVerifierUpgrading, verificationSucceedsAsCurrentVersionIsLargerThanRegisteredVersion)
{
  CHECK_TRUE(verifier->verifyUpgrade());
}

TEST(FirmwareVerifierUpgrading, verificationResetsRegisteredValuesOnSuccess)
{
  verifier->verifyUpgrade();
  CHECK_EQUAL(0U, verifier->getFirmwareUpgradeCount());
  CHECK_EQUAL(0U, verifier->getFirmwareVersionCode());
  STRCMP_EQUAL("", verifier->getFirmwareUpgradeID());
}

TEST(FirmwareVerifierUpgrading, verificationFailsIfRegisteredVersionEqualsCurrentVersion)
{
  verifier->registerUpgradeAttempt("http://new_file.bin");
  CHECK_FALSE(verifier->verifyUpgrade());
}

TEST(FirmwareVerifierUpgrading, verificationDoesNotResetRegisteredValuesOnFail)
{
  verifier->registerUpgradeAttempt("http://new_file.bin");
  verifier->verifyUpgrade();
  CHECK_EQUAL(1U, verifier->getFirmwareUpgradeCount());
  CHECK_EQUAL(getExpectedVersionCode(), verifier->getFirmwareVersionCode());
  STRCMP_EQUAL("/new_file.bin", verifier->getFirmwareUpgradeID());
}
