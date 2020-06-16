#include "CppUTest\TestHarness.h"

#include "Constants.h"
#include "DateTime.h"
#include "DisplayMock.h"
#include "RtcMock.h"
#include "SystemTimeDriverMock.h"
#include "RUMock.h"
#include "CryptoDriverMock.h"
#include "BTDriverMock.h"
#include "WifiDriverMock.h"
#include "TimerDriverMock.h"
#include "PartitionDriverMock.h"
#include "PersistentStorageDriverMock.h"
#include "SettingsStorage.h"
#include "DeviceMetaStorage.h"
#include "PowerOnSelfTest.h"


static PowerOnSelfTest selfTest;
static DisplayMock displayMock;
static RtcMock rtcMock;
static SystemTimeDriverMock systemTimeMock;
static RUMock ruMock;
static CryptoDriverMock cryptoMock;
static BTDriverMock btMock;
static WifiDriverMock wifiMock;
static TimerDriverDummy timerDummy(0U, 1000U);
static PartitionDriverMock partitionMock;
static PersistentStorageDriverMock settingsStorageMock;
static PersistentStorageDriverMock metaStorageMock;


TEST_GROUP(powerOnSelfTest)
{
  TEST_SETUP()
  {
    mock().installComparator("DateTime", dateTimeComparator);
    mock().installComparator("RURegulator", ruRegulatorComparator);
    mock().installCopier("DateTime", dateTimeCopier);
  }

  TEST_TEARDOWN()
  {
    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
  }
};

TEST(powerOnSelfTest, testSettingsStoreInitFailsAndReinitAlsoFails)
{
  // Ignore other calls, i.e. calls for partition info
  mock().ignoreOtherCalls();

  // Display setup
  mock().expectOneCall("setup");

  // Storage setup (fails => mode assumed normal mode, isTimeSet assumed false)
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::SETTINGS_PARTITION_NAME).withBoolParameter("readOnly", false).andReturnValue(false);
  mock().expectOneCall("PersistentStorageDriverMock::erase").withStringParameter("partition", Constants::SETTINGS_PARTITION_NAME).andReturnValue(false);

  // Meta setup (GDID set)
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::META_PARTITION_NAME).ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readString").withStringParameter("id", MetaUID::GDID).withOutputParameterReturning("str", "123456", 6).withUnsignedIntParameter("size", Constants::GDID_SIZE_MAX);

  // Rtc and SysTime setup (setting clock for the first time)
  DateTime dateTime = Constants::DEFAULT_DATETIME;
  mock().expectOneCall("RtcMock::setup").andReturnValue(true);
  mock().expectOneCall("RtcMock::setCurrentTime").withParameterOfType("DateTime", "dateTime", &dateTime);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime").ignoreOtherParameters();
  mock().expectOneCall("SystemTimeDriverMock::setUnixTime").ignoreOtherParameters();
  mock().expectOneCall("PersistentStorageDriverMock::writeValue").withStringParameter("id", SettingUID::IS_TIME_SET).withParameter("value", static_cast<uint32_t>(true));

  // SystemTime tick test (successful)
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(1U);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(2U);

  // RU selfTest and configuration check
  mock().expectOneCall("RUMock::performSelfTest").andReturnValue(true);
  mock().expectOneCall("RUMock::updateConfig").ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("RUMock::isHighPowerVersion").andReturnValue(true);

  // CryptoDriver setup
  mock().expectOneCall("CryptoDriverMock::resetAndInit").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::isProvisioned").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readCertificates").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readSerialNumber").ignoreOtherParameters();

  // Wifi init
  mock().expectOneCall("WifiDriverMock::resetAndInit").andReturnValue(true);

  // BT init
  mock().expectOneCall("BTDriverMock::resetAndInit").andReturnValue(true);

  // Run selfTest and check results
  bool result = selfTest.run(partitionMock, timerDummy, displayMock, rtcMock, systemTimeMock, ruMock, cryptoMock, btMock, wifiMock, settingsStorageMock, metaStorageMock);
  CHECK_FALSE(result);
  CHECK_EQUAL(8, selfTest.getErrorCode());
  CHECK_FALSE(selfTest.isFatalError());
}


TEST(powerOnSelfTest, testSettingsStoreInitFailsButReinitIsOK)
{
  // Ignore other calls, i.e. calls for partition info
  mock().ignoreOtherCalls();

  // Display setup
  mock().expectOneCall("setup");

  // Storage setup (fails => mode assumed normal mode, isTimeSet assumed false)
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::SETTINGS_PARTITION_NAME).withBoolParameter("readOnly", false).andReturnValue(false);
  mock().expectOneCall("PersistentStorageDriverMock::erase").withStringParameter("partition", Constants::SETTINGS_PARTITION_NAME).andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::SETTINGS_PARTITION_NAME).withBoolParameter("readOnly", false).andReturnValue(true);

  // Meta setup (GDID set)
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::META_PARTITION_NAME).ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readString").withStringParameter("id", MetaUID::GDID).withOutputParameterReturning("str", "123456", 6).withUnsignedIntParameter("size", Constants::GDID_SIZE_MAX);

  // Rtc and SysTime setup (setting clock for the first time)
  DateTime dateTime = Constants::DEFAULT_DATETIME;
  mock().expectOneCall("RtcMock::setup").andReturnValue(true);
  mock().expectOneCall("RtcMock::setCurrentTime").withParameterOfType("DateTime", "dateTime", &dateTime);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime").ignoreOtherParameters();
  mock().expectOneCall("SystemTimeDriverMock::setUnixTime").ignoreOtherParameters();
  mock().expectOneCall("PersistentStorageDriverMock::writeValue").withStringParameter("id", SettingUID::IS_TIME_SET).withParameter("value", static_cast<uint32_t>(true));

  // SystemTime tick test (successful)
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(1U);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(2U);

  // RU selfTest and configuration check
  mock().expectOneCall("RUMock::performSelfTest").andReturnValue(true);
  mock().expectOneCall("RUMock::updateConfig").ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("RUMock::isHighPowerVersion").andReturnValue(true);

  // CryptoDriver setup
  mock().expectOneCall("CryptoDriverMock::resetAndInit").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::isProvisioned").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readCertificates").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readSerialNumber").ignoreOtherParameters();

  // Wifi init
  mock().expectOneCall("WifiDriverMock::resetAndInit").andReturnValue(true);

  // BT init
  mock().expectOneCall("BTDriverMock::resetAndInit").andReturnValue(true);

  // Run selfTest and check results
  bool result = selfTest.run(partitionMock, timerDummy, displayMock, rtcMock, systemTimeMock, ruMock, cryptoMock, btMock, wifiMock, settingsStorageMock, metaStorageMock);
  CHECK_FALSE(result);
  CHECK_EQUAL(8, selfTest.getErrorCode());
  CHECK_FALSE(selfTest.isFatalError());
}


TEST(powerOnSelfTest, testMetaStoreInitFails)
{
  // Ignore other calls, i.e. calls for partition info
  mock().ignoreOtherCalls();

  // Display setup
  mock().expectOneCall("setup");

  // Storage setup - normal mode, isTimeSet assumed false
  uint32_t storedRunMode = static_cast<uint32_t>(RunMode::normal);
  uint32_t storedTimeSet = false;
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::SETTINGS_PARTITION_NAME).withBoolParameter("readOnly", false).andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::RUN_MODE).withOutputParameterReturning("value", &storedRunMode, sizeof(uint32_t));
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::IS_TIME_SET).withOutputParameterReturning("value", &storedTimeSet, sizeof(uint32_t));

  // Meta setup
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::META_PARTITION_NAME).ignoreOtherParameters().andReturnValue(false);

  // Rtc and SysTime setup (setting clock for the first time)
  DateTime dateTime = Constants::DEFAULT_DATETIME;
  mock().expectOneCall("RtcMock::setup").andReturnValue(true);
  mock().expectOneCall("RtcMock::setCurrentTime").withParameterOfType("DateTime", "dateTime", &dateTime);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime").ignoreOtherParameters();
  mock().expectOneCall("SystemTimeDriverMock::setUnixTime").ignoreOtherParameters();
  mock().expectOneCall("PersistentStorageDriverMock::writeValue").withStringParameter("id", SettingUID::IS_TIME_SET).withParameter("value", static_cast<uint32_t>(true));

  // SystemTime tick test (successful)
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(1U);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(2U);

  // RU selfTest and configuration check
  mock().expectOneCall("RUMock::performSelfTest").andReturnValue(true);
  mock().expectOneCall("RUMock::updateConfig").ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("RUMock::isHighPowerVersion").andReturnValue(true);


  // CryptoDriver setup
  mock().expectOneCall("CryptoDriverMock::resetAndInit").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::isProvisioned").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readCertificates").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readSerialNumber").ignoreOtherParameters();

  // Wifi init
  mock().expectOneCall("WifiDriverMock::resetAndInit").andReturnValue(true);

  // BT init
  mock().expectOneCall("BTDriverMock::resetAndInit").andReturnValue(true);

  // Run selfTest and check results
  bool result = selfTest.run(partitionMock, timerDummy, displayMock, rtcMock, systemTimeMock, ruMock, cryptoMock, btMock, wifiMock, settingsStorageMock, metaStorageMock);
  CHECK_FALSE(result);
  CHECK_EQUAL(64, selfTest.getErrorCode());
  CHECK_FALSE(selfTest.isFatalError());
}

TEST(powerOnSelfTest, testMetaStoreInitFailsGDIDNotSetWhenNotInTestMode)
{
  // Ignore other calls, i.e. calls for partition info
  mock().ignoreOtherCalls();

  // Display setup
  mock().expectOneCall("setup");

  // Storage setup - normal mode, isTimeSet assumed false
  uint32_t storedRunMode = static_cast<uint32_t>(RunMode::normal);
  uint32_t storedTimeSet = false;
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::SETTINGS_PARTITION_NAME).withBoolParameter("readOnly", false).andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::RUN_MODE).withOutputParameterReturning("value", &storedRunMode, sizeof(uint32_t));
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::IS_TIME_SET).withOutputParameterReturning("value", &storedTimeSet, sizeof(uint32_t));

  // Meta setup (GDID not set)
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::META_PARTITION_NAME).ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readString").
      withStringParameter("id", MetaUID::GDID)
      .withOutputParameterReturning("str", "somebogus", 9)
      .withUnsignedIntParameter("size", Constants::GDID_SIZE_MAX)
      .andReturnValue(false);

  // Rtc and SysTime setup (setting clock for the first time)
  DateTime dateTime = Constants::DEFAULT_DATETIME;
  mock().expectOneCall("RtcMock::setup").andReturnValue(true);
  mock().expectOneCall("RtcMock::setCurrentTime").withParameterOfType("DateTime", "dateTime", &dateTime);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime").ignoreOtherParameters();
  mock().expectOneCall("SystemTimeDriverMock::setUnixTime").ignoreOtherParameters();
  mock().expectOneCall("PersistentStorageDriverMock::writeValue").withStringParameter("id", SettingUID::IS_TIME_SET).withParameter("value", static_cast<uint32_t>(true));

  // SystemTime tick test (successful)
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(1U);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(2U);

  // RU selfTest and configuration check
  mock().expectOneCall("RUMock::performSelfTest").andReturnValue(true);
  mock().expectOneCall("RUMock::updateConfig").ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("RUMock::isHighPowerVersion").andReturnValue(true);


  // CryptoDriver setup
  mock().expectOneCall("CryptoDriverMock::resetAndInit").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::isProvisioned").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readCertificates").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readSerialNumber").ignoreOtherParameters();

  // Wifi init
  mock().expectOneCall("WifiDriverMock::resetAndInit").andReturnValue(true);

  // BT init
  mock().expectOneCall("BTDriverMock::resetAndInit").andReturnValue(true);

  // Run selfTest and check results
  bool result = selfTest.run(partitionMock, timerDummy, displayMock, rtcMock, systemTimeMock, ruMock, cryptoMock, btMock, wifiMock, settingsStorageMock, metaStorageMock);
  CHECK_FALSE(result);
  CHECK_EQUAL(64, selfTest.getErrorCode());
  CHECK_FALSE(selfTest.isFatalError());
}


TEST(powerOnSelfTest, testRtcSetupFails)
{
  // Ignore other calls, i.e. calls for partition info
  mock().ignoreOtherCalls();

  // Display setup
  mock().expectOneCall("setup");

  // Storage setup - normal mode, isTimeSet assumed false
  uint32_t storedRunMode = static_cast<uint32_t>(RunMode::normal);
  uint32_t storedTimeSet = false;
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::SETTINGS_PARTITION_NAME).withBoolParameter("readOnly", false).andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::RUN_MODE).withOutputParameterReturning("value", &storedRunMode, sizeof(uint32_t));
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::IS_TIME_SET).withOutputParameterReturning("value", &storedTimeSet, sizeof(uint32_t));

  // Meta setup (GDID set)
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::META_PARTITION_NAME).ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readString").withStringParameter("id", MetaUID::GDID).withOutputParameterReturning("str", "123456", 6).withUnsignedIntParameter("size", Constants::GDID_SIZE_MAX);

  // Rtc and SysTime setup (setup fails, starting with default time, do not store 'isTimeSet' as true)
  mock().expectOneCall("RtcMock::setup").andReturnValue(false);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime").ignoreOtherParameters();
  mock().expectOneCall("SystemTimeDriverMock::setUnixTime").ignoreOtherParameters();

  // SystemTime tick test (successful)
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(1U);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(2U);

  // RU selfTest and configuration check
  mock().expectOneCall("RUMock::performSelfTest").andReturnValue(true);
  mock().expectOneCall("RUMock::updateConfig").ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("RUMock::isHighPowerVersion").andReturnValue(true);


  // CryptoDriver setup
  mock().expectOneCall("CryptoDriverMock::resetAndInit").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::isProvisioned").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readCertificates").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readSerialNumber").ignoreOtherParameters();

  // Wifi init
  mock().expectOneCall("WifiDriverMock::resetAndInit").andReturnValue(true);

  // BT init
  mock().expectOneCall("BTDriverMock::resetAndInit").andReturnValue(true);

  // Run selfTest and check results
  bool result = selfTest.run(partitionMock, timerDummy, displayMock, rtcMock, systemTimeMock, ruMock, cryptoMock, btMock, wifiMock, settingsStorageMock, metaStorageMock);
  CHECK_FALSE(result);
  CHECK_EQUAL(1, selfTest.getErrorCode());
  CHECK_FALSE(selfTest.isFatalError());
}


TEST(powerOnSelfTest, testRtcTimeSetToDefaultIfItHasBeenSetButIsInvalid)
{
  // Ignore other calls, i.e. calls for partition info
  mock().ignoreOtherCalls();

  // Display setup
  mock().expectOneCall("setup");

  // Storage setup - normal mode, isTimeSet assumed true
  uint32_t storedRunMode = static_cast<uint32_t>(RunMode::normal);
  uint32_t storedTimeSet = true;
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::SETTINGS_PARTITION_NAME).withBoolParameter("readOnly", false).andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::RUN_MODE).withOutputParameterReturning("value", &storedRunMode, sizeof(uint32_t));
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::IS_TIME_SET).withOutputParameterReturning("value", &storedTimeSet, sizeof(uint32_t));

  // Meta setup (GDID set)
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::META_PARTITION_NAME).ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readString").withStringParameter("id", MetaUID::GDID).withOutputParameterReturning("str", "123456", 6).withUnsignedIntParameter("size", Constants::GDID_SIZE_MAX);

  // Rtc and SysTime setup (resetting time to default time)
  DateTime dateTime = Constants::DEFAULT_DATETIME;
  mock().expectOneCall("RtcMock::setup").andReturnValue(true);
  mock().expectOneCall("RtcMock::getCurrentTime").ignoreOtherParameters().andReturnValue(TimeStatus::INTEGRITY_LOST);
  mock().expectOneCall("RtcMock::setCurrentTime").withParameterOfType("DateTime", "dateTime", &dateTime);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime").ignoreOtherParameters();
  mock().expectOneCall("SystemTimeDriverMock::setUnixTime").ignoreOtherParameters();

  // SystemTime tick test (successful)
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(1U);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(2U);

  // RU selfTest and configuration check
  mock().expectOneCall("RUMock::performSelfTest").andReturnValue(true);
  mock().expectOneCall("RUMock::updateConfig").ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("RUMock::isHighPowerVersion").andReturnValue(true);


  // CryptoDriver setup
  mock().expectOneCall("CryptoDriverMock::resetAndInit").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::isProvisioned").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readCertificates").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readSerialNumber").ignoreOtherParameters();

  // Wifi init
  mock().expectOneCall("WifiDriverMock::resetAndInit").andReturnValue(true);

  // BT init
  mock().expectOneCall("BTDriverMock::resetAndInit").andReturnValue(true);

  // Run selfTest and check results
  bool result = selfTest.run(partitionMock, timerDummy, displayMock, rtcMock, systemTimeMock, ruMock, cryptoMock, btMock, wifiMock, settingsStorageMock, metaStorageMock);
  CHECK_FALSE(result);
  CHECK_EQUAL(1, selfTest.getErrorCode());
  CHECK_FALSE(selfTest.isFatalError());
}


TEST(powerOnSelfTest, testSystemTimeNotRunning)
{
  // Ignore other calls, i.e. calls for partition info
  mock().ignoreOtherCalls();

  // Display setup
  mock().expectOneCall("setup");

  // Storage setup - normal mode, isTimeSet assumed false
  uint32_t storedRunMode = static_cast<uint32_t>(RunMode::normal);
  uint32_t storedTimeSet = false;
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::SETTINGS_PARTITION_NAME).withBoolParameter("readOnly", false).andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::RUN_MODE).withOutputParameterReturning("value", &storedRunMode, sizeof(uint32_t));
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::IS_TIME_SET).withOutputParameterReturning("value", &storedTimeSet, sizeof(uint32_t));

  // Meta setup (GDID set)
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::META_PARTITION_NAME).ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readString").withStringParameter("id", MetaUID::GDID).withOutputParameterReturning("str", "123456", 6).withUnsignedIntParameter("size", Constants::GDID_SIZE_MAX);

  // Rtc and SysTime setup (setting clock for the first time)
  DateTime dateTime = Constants::DEFAULT_DATETIME;
  mock().expectOneCall("RtcMock::setup").andReturnValue(true);
  mock().expectOneCall("RtcMock::setCurrentTime").withParameterOfType("DateTime", "dateTime", &dateTime);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime").ignoreOtherParameters();
  mock().expectOneCall("SystemTimeDriverMock::setUnixTime").ignoreOtherParameters();
  mock().expectOneCall("PersistentStorageDriverMock::writeValue").withStringParameter("id", SettingUID::IS_TIME_SET).withParameter("value", static_cast<uint32_t>(true));

  // SystemTime tick test (error)
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(1U);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(1U);

  // RU selfTest and configuration check
  mock().expectOneCall("RUMock::performSelfTest").andReturnValue(true);
  mock().expectOneCall("RUMock::updateConfig").ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("RUMock::isHighPowerVersion").andReturnValue(true);


  // CryptoDriver setup
  mock().expectOneCall("CryptoDriverMock::resetAndInit").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::isProvisioned").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readCertificates").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readSerialNumber").ignoreOtherParameters();

  // Wifi init
  mock().expectOneCall("WifiDriverMock::resetAndInit").andReturnValue(true);

  // BT init
  mock().expectOneCall("BTDriverMock::resetAndInit").andReturnValue(true);

  // Run selfTest and check results
  bool result = selfTest.run(partitionMock, timerDummy, displayMock, rtcMock, systemTimeMock, ruMock, cryptoMock, btMock, wifiMock, settingsStorageMock, metaStorageMock);
  CHECK_FALSE(result);
  CHECK_EQUAL(2, selfTest.getErrorCode());
  CHECK_FALSE(selfTest.isFatalError());
}


TEST(powerOnSelfTest, testSystemTimeRunningTooFast)
{
  // Ignore other calls, i.e. calls for partition info
  mock().ignoreOtherCalls();

  // Display setup
  mock().expectOneCall("setup");

  // Storage setup - normal mode, isTimeSet assumed false
  uint32_t storedRunMode = static_cast<uint32_t>(RunMode::normal);
  uint32_t storedTimeSet = false;
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::SETTINGS_PARTITION_NAME).withBoolParameter("readOnly", false).andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::RUN_MODE).withOutputParameterReturning("value", &storedRunMode, sizeof(uint32_t));
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::IS_TIME_SET).withOutputParameterReturning("value", &storedTimeSet, sizeof(uint32_t));

  // Meta setup (GDID set)
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::META_PARTITION_NAME).ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readString").withStringParameter("id", MetaUID::GDID).withOutputParameterReturning("str", "123456", 6).withUnsignedIntParameter("size", Constants::GDID_SIZE_MAX);

  // Rtc and SysTime setup (setting clock for the first time)
  DateTime dateTime = Constants::DEFAULT_DATETIME;
  mock().expectOneCall("RtcMock::setup").andReturnValue(true);
  mock().expectOneCall("RtcMock::setCurrentTime").withParameterOfType("DateTime", "dateTime", &dateTime);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime").ignoreOtherParameters();
  mock().expectOneCall("SystemTimeDriverMock::setUnixTime").ignoreOtherParameters();
  mock().expectOneCall("PersistentStorageDriverMock::writeValue").withStringParameter("id", SettingUID::IS_TIME_SET).withParameter("value", static_cast<uint32_t>(true));

  // SystemTime tick test (error)
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(1U);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(4U);

  // RU selfTest and configuration check
  mock().expectOneCall("RUMock::performSelfTest").andReturnValue(true);
  mock().expectOneCall("RUMock::updateConfig").ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("RUMock::isHighPowerVersion").andReturnValue(true);


  // CryptoDriver setup
  mock().expectOneCall("CryptoDriverMock::resetAndInit").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::isProvisioned").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readCertificates").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readSerialNumber").ignoreOtherParameters();

  // Wifi init
  mock().expectOneCall("WifiDriverMock::resetAndInit").andReturnValue(true);

  // BT init
  mock().expectOneCall("BTDriverMock::resetAndInit").andReturnValue(true);

  // Run selfTest and check results
  bool result = selfTest.run(partitionMock, timerDummy, displayMock, rtcMock, systemTimeMock, ruMock, cryptoMock, btMock, wifiMock, settingsStorageMock, metaStorageMock);
  CHECK_FALSE(result);
  CHECK_EQUAL(2, selfTest.getErrorCode());
  CHECK_FALSE(selfTest.isFatalError());
}

TEST(powerOnSelfTest, testRUSelfTestFailsAndWifiBTInitSkippedAsAConsequence)
{
  // Ignore other calls, i.e. calls for partition info
  mock().ignoreOtherCalls();

  // Display setup
  mock().expectOneCall("setup");

  // Storage setup - normal mode, isTimeSet assumed false
  uint32_t storedRunMode = static_cast<uint32_t>(RunMode::normal);
  uint32_t storedTimeSet = true;
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::SETTINGS_PARTITION_NAME).withBoolParameter("readOnly", false).andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::RUN_MODE).withOutputParameterReturning("value", &storedRunMode, sizeof(uint32_t));
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::IS_TIME_SET).withOutputParameterReturning("value", &storedTimeSet, sizeof(uint32_t));

  // Meta setup (GDID set)
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::META_PARTITION_NAME).ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readString").withStringParameter("id", MetaUID::GDID).withOutputParameterReturning("str", "123456", 6).withUnsignedIntParameter("size", Constants::GDID_SIZE_MAX);

  // Rtc and SysTime setup (setting clock for the first time)
  DateTime dateTime = { 18, 5, 17, 9, 10, 11, WeekDay::THURSDAY };
  mock().expectOneCall("RtcMock::setup").andReturnValue(true);
  mock().expectOneCall("RtcMock::getCurrentTime").withOutputParameterOfTypeReturning("DateTime", "dateTime", &dateTime);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime").withParameterOfType("DateTime", "dateTime", &dateTime).andReturnValue(123456U);
  mock().expectOneCall("SystemTimeDriverMock::setUnixTime").withUnsignedIntParameter("unixTime", 123456U);

  // SystemTime tick test (successful)
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(1U);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(2U);

  // RU selfTest and configuration check
  mock().expectOneCall("RUMock::performSelfTest").andReturnValue(false);

  // CryptoDriver setup
  mock().expectOneCall("CryptoDriverMock::resetAndInit").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::isProvisioned").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readCertificates").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readSerialNumber").ignoreOtherParameters();

  // Wifi init
  mock().expectNoCall("WifiDriverMock::resetAndInit");

  // BT init
  mock().expectNoCall("BTDriverMock::resetAndInit");

  // Run selfTest and check results
  bool result = selfTest.run(partitionMock, timerDummy, displayMock, rtcMock, systemTimeMock, ruMock, cryptoMock, btMock, wifiMock, settingsStorageMock, metaStorageMock);
  CHECK_FALSE(result);
  CHECK_EQUAL(128, selfTest.getErrorCode());
  CHECK_TRUE(selfTest.isFatalError());
}

TEST(powerOnSelfTest, testRUUpdateConfigFailsAndWifiBTInitSkippedAsAConsequence)
{
  // Ignore other calls, i.e. calls for partition info
  mock().ignoreOtherCalls();

  // Display setup
  mock().expectOneCall("setup");

  // Storage setup - normal mode, isTimeSet assumed false
  uint32_t storedRunMode = static_cast<uint32_t>(RunMode::normal);
  uint32_t storedTimeSet = true;
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::SETTINGS_PARTITION_NAME).withBoolParameter("readOnly", false).andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::RUN_MODE).withOutputParameterReturning("value", &storedRunMode, sizeof(uint32_t));
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::IS_TIME_SET).withOutputParameterReturning("value", &storedTimeSet, sizeof(uint32_t));

  // Meta setup (GDID set)
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::META_PARTITION_NAME).ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readString").withStringParameter("id", MetaUID::GDID).withOutputParameterReturning("str", "123456", 6).withUnsignedIntParameter("size", Constants::GDID_SIZE_MAX);

  // Rtc and SysTime setup (setting clock for the first time)
  DateTime dateTime = { 18, 5, 17, 9, 10, 11, WeekDay::THURSDAY };
  mock().expectOneCall("RtcMock::setup").andReturnValue(true);
  mock().expectOneCall("RtcMock::getCurrentTime").withOutputParameterOfTypeReturning("DateTime", "dateTime", &dateTime);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime").withParameterOfType("DateTime", "dateTime", &dateTime).andReturnValue(123456U);
  mock().expectOneCall("SystemTimeDriverMock::setUnixTime").withUnsignedIntParameter("unixTime", 123456U);

  // SystemTime tick test (successful)
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(1U);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(2U);

  // RU selfTest and configuration check
  mock().expectOneCall("RUMock::performSelfTest").andReturnValue(true);
  mock().expectOneCall("RUMock::updateConfig").ignoreOtherParameters().andReturnValue(false);

  // CryptoDriver setup
  mock().expectOneCall("CryptoDriverMock::resetAndInit").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::isProvisioned").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readCertificates").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readSerialNumber").ignoreOtherParameters();

  // Wifi init
  mock().expectNoCall("WifiDriverMock::resetAndInit");

  // BT init
  mock().expectNoCall("BTDriverMock::resetAndInit");

  // Run selfTest and check results
  bool result = selfTest.run(partitionMock, timerDummy, displayMock, rtcMock, systemTimeMock, ruMock, cryptoMock, btMock, wifiMock, settingsStorageMock, metaStorageMock);
  CHECK_FALSE(result);
  CHECK_EQUAL(128, selfTest.getErrorCode());
  CHECK_TRUE(selfTest.isFatalError());
}

TEST(powerOnSelfTest, testRUNotCompatibleAndWifiBTInitSkippedAsAConsequence)
{
  // Ignore other calls, i.e. calls for partition info
  mock().ignoreOtherCalls();

  // Display setup
  mock().expectOneCall("setup");

  // Storage setup - normal mode, isTimeSet assumed false
  uint32_t storedRunMode = static_cast<uint32_t>(RunMode::normal);
  uint32_t storedTimeSet = true;
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::SETTINGS_PARTITION_NAME).withBoolParameter("readOnly", false).andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::RUN_MODE).withOutputParameterReturning("value", &storedRunMode, sizeof(uint32_t));
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::IS_TIME_SET).withOutputParameterReturning("value", &storedTimeSet, sizeof(uint32_t));

  // Meta setup (GDID set)
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::META_PARTITION_NAME).ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readString").withStringParameter("id", MetaUID::GDID).withOutputParameterReturning("str", "123456", 6).withUnsignedIntParameter("size", Constants::GDID_SIZE_MAX);

  // Rtc and SysTime setup (setting clock for the first time)
  DateTime dateTime = { 18, 5, 17, 9, 10, 11, WeekDay::THURSDAY };
  mock().expectOneCall("RtcMock::setup").andReturnValue(true);
  mock().expectOneCall("RtcMock::getCurrentTime").withOutputParameterOfTypeReturning("DateTime", "dateTime", &dateTime);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime").withParameterOfType("DateTime", "dateTime", &dateTime).andReturnValue(123456U);
  mock().expectOneCall("SystemTimeDriverMock::setUnixTime").withUnsignedIntParameter("unixTime", 123456U);

  // SystemTime tick test (successful)
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(1U);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(2U);

  // RU selfTest and configuration check
  mock().expectOneCall("RUMock::performSelfTest").andReturnValue(true);
  mock().expectOneCall("RUMock::updateConfig").ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("RUMock::isHighPowerVersion").andReturnValue(false);

  // CryptoDriver setup
  mock().expectOneCall("CryptoDriverMock::resetAndInit").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::isProvisioned").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readCertificates").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readSerialNumber").ignoreOtherParameters();

  // Wifi init
  mock().expectNoCall("WifiDriverMock::resetAndInit");

  // BT init
  mock().expectNoCall("BTDriverMock::resetAndInit");

  // Run selfTest and check results
  bool result = selfTest.run(partitionMock, timerDummy, displayMock, rtcMock, systemTimeMock, ruMock, cryptoMock, btMock, wifiMock, settingsStorageMock, metaStorageMock);
  CHECK_FALSE(result);
  CHECK_EQUAL(256, selfTest.getErrorCode());
  CHECK_TRUE(selfTest.isFatalError());
}

TEST(powerOnSelfTest, testCryptoDriverInitFails)
{
  // Ignore other calls, i.e. calls for partition info
  mock().ignoreOtherCalls();

  // Display setup
  mock().expectOneCall("setup");

  // Storage setup - normal mode, isTimeSet assumed false
  uint32_t storedRunMode = static_cast<uint32_t>(RunMode::normal);
  uint32_t storedTimeSet = false;
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::SETTINGS_PARTITION_NAME).withBoolParameter("readOnly", false).andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::RUN_MODE).withOutputParameterReturning("value", &storedRunMode, sizeof(uint32_t));
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::IS_TIME_SET).withOutputParameterReturning("value", &storedTimeSet, sizeof(uint32_t));

  // Meta setup (GDID set)
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::META_PARTITION_NAME).ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readString").withStringParameter("id", MetaUID::GDID).withOutputParameterReturning("str", "123456", 6).withUnsignedIntParameter("size", Constants::GDID_SIZE_MAX);

  // Rtc and SysTime setup (setting clock for the first time)
  DateTime dateTime = Constants::DEFAULT_DATETIME;
  mock().expectOneCall("RtcMock::setup").andReturnValue(true);
  mock().expectOneCall("RtcMock::setCurrentTime").withParameterOfType("DateTime", "dateTime", &dateTime);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime").ignoreOtherParameters();
  mock().expectOneCall("SystemTimeDriverMock::setUnixTime").ignoreOtherParameters();
  mock().expectOneCall("PersistentStorageDriverMock::writeValue").withStringParameter("id", SettingUID::IS_TIME_SET).withParameter("value", static_cast<uint32_t>(true));

  // SystemTime tick test (successful)
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(1U);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(3U);

  // RU selfTest and configuration check
  mock().expectOneCall("RUMock::performSelfTest").andReturnValue(true);
  mock().expectOneCall("RUMock::updateConfig").ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("RUMock::isHighPowerVersion").andReturnValue(true);


  // CryptoDriver setup
  mock().expectOneCall("CryptoDriverMock::resetAndInit").andReturnValue(false);

  // Wifi init
  mock().expectOneCall("WifiDriverMock::resetAndInit").andReturnValue(true);

  // BT init
  mock().expectOneCall("BTDriverMock::resetAndInit").andReturnValue(true);

  // Run selfTest and check results
  bool result = selfTest.run(partitionMock, timerDummy, displayMock, rtcMock, systemTimeMock, ruMock, cryptoMock, btMock, wifiMock, settingsStorageMock, metaStorageMock);
  CHECK_FALSE(result);
  CHECK_EQUAL(4, selfTest.getErrorCode());
  CHECK_FALSE(selfTest.isFatalError());
}

TEST(powerOnSelfTest, testCryptoDriverNotProvisioned)
{
  // Ignore other calls, i.e. calls for partition info
  mock().ignoreOtherCalls();

  // Display setup
  mock().expectOneCall("setup");

  // Storage setup - normal mode, isTimeSet assumed false
  uint32_t storedRunMode = static_cast<uint32_t>(RunMode::normal);
  uint32_t storedTimeSet = false;
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::SETTINGS_PARTITION_NAME).withBoolParameter("readOnly", false).andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::RUN_MODE).withOutputParameterReturning("value", &storedRunMode, sizeof(uint32_t));
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::IS_TIME_SET).withOutputParameterReturning("value", &storedTimeSet, sizeof(uint32_t));

  // Meta setup (GDID set)
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::META_PARTITION_NAME).ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readString").withStringParameter("id", MetaUID::GDID).withOutputParameterReturning("str", "123456", 6).withUnsignedIntParameter("size", Constants::GDID_SIZE_MAX);

  // Rtc and SysTime setup (setting clock for the first time)
  DateTime dateTime = Constants::DEFAULT_DATETIME;
  mock().expectOneCall("RtcMock::setup").andReturnValue(true);
  mock().expectOneCall("RtcMock::setCurrentTime").withParameterOfType("DateTime", "dateTime", &dateTime);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime").ignoreOtherParameters();
  mock().expectOneCall("SystemTimeDriverMock::setUnixTime").ignoreOtherParameters();
  mock().expectOneCall("PersistentStorageDriverMock::writeValue").withStringParameter("id", SettingUID::IS_TIME_SET).withParameter("value", static_cast<uint32_t>(true));

  // SystemTime tick test (successful)
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(1U);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(3U);

  // RU selfTest and configuration check
  mock().expectOneCall("RUMock::performSelfTest").andReturnValue(true);
  mock().expectOneCall("RUMock::updateConfig").ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("RUMock::isHighPowerVersion").andReturnValue(true);


  // CryptoDriver setup
  mock().expectOneCall("CryptoDriverMock::resetAndInit").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::isProvisioned").andReturnValue(false);

  // Wifi init
  mock().expectOneCall("WifiDriverMock::resetAndInit").andReturnValue(true);

  // BT init
  mock().expectOneCall("BTDriverMock::resetAndInit").andReturnValue(true);

  // Run selfTest and check results
  bool result = selfTest.run(partitionMock, timerDummy, displayMock, rtcMock, systemTimeMock, ruMock, cryptoMock, btMock, wifiMock, settingsStorageMock, metaStorageMock);
  CHECK_FALSE(result);
  CHECK_EQUAL(4, selfTest.getErrorCode());
  CHECK_FALSE(selfTest.isFatalError());
}

TEST(powerOnSelfTest, testCryptoDriverCertificateReadFails)
{
  // Ignore other calls, i.e. calls for partition info
  mock().ignoreOtherCalls();

  // Display setup
  mock().expectOneCall("setup");

  // Storage setup - normal mode, isTimeSet assumed false
  uint32_t storedRunMode = static_cast<uint32_t>(RunMode::normal);
  uint32_t storedTimeSet = false;
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::SETTINGS_PARTITION_NAME).withBoolParameter("readOnly", false).andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::RUN_MODE).withOutputParameterReturning("value", &storedRunMode, sizeof(uint32_t));
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::IS_TIME_SET).withOutputParameterReturning("value", &storedTimeSet, sizeof(uint32_t));

  // Meta setup (GDID set)
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::META_PARTITION_NAME).ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readString").withStringParameter("id", MetaUID::GDID).withOutputParameterReturning("str", "123456", 6).withUnsignedIntParameter("size", Constants::GDID_SIZE_MAX);

  // Rtc and SysTime setup (setting clock for the first time)
  DateTime dateTime = Constants::DEFAULT_DATETIME;
  mock().expectOneCall("RtcMock::setup").andReturnValue(true);
  mock().expectOneCall("RtcMock::setCurrentTime").withParameterOfType("DateTime", "dateTime", &dateTime);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime").ignoreOtherParameters();
  mock().expectOneCall("SystemTimeDriverMock::setUnixTime").ignoreOtherParameters();
  mock().expectOneCall("PersistentStorageDriverMock::writeValue").withStringParameter("id", SettingUID::IS_TIME_SET).withParameter("value", static_cast<uint32_t>(true));

  // SystemTime tick test (successful)
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(1U);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(3U);

  // RU selfTest and configuration check
  mock().expectOneCall("RUMock::performSelfTest").andReturnValue(true);
  mock().expectOneCall("RUMock::updateConfig").ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("RUMock::isHighPowerVersion").andReturnValue(true);


  // CryptoDriver setup
  mock().expectOneCall("CryptoDriverMock::resetAndInit").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::isProvisioned").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readCertificates").andReturnValue(false);

  // Wifi init
  mock().expectOneCall("WifiDriverMock::resetAndInit").andReturnValue(true);

  // BT init
  mock().expectOneCall("BTDriverMock::resetAndInit").andReturnValue(true);

  // Run selfTest and check results
  bool result = selfTest.run(partitionMock, timerDummy, displayMock, rtcMock, systemTimeMock, ruMock, cryptoMock, btMock, wifiMock, settingsStorageMock, metaStorageMock);
  CHECK_FALSE(result);
  CHECK_EQUAL(4, selfTest.getErrorCode());
  CHECK_FALSE(selfTest.isFatalError());
}

TEST(powerOnSelfTest, testWifiInitFails)
{
  // Ignore other calls, i.e. calls for partition info
  mock().ignoreOtherCalls();

  // Display setup
  mock().expectOneCall("setup");

  // Storage setup - normal mode, isTimeSet assumed false
  uint32_t storedRunMode = static_cast<uint32_t>(RunMode::normal);
  uint32_t storedTimeSet = false;
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::SETTINGS_PARTITION_NAME).withBoolParameter("readOnly", false).andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::RUN_MODE).withOutputParameterReturning("value", &storedRunMode, sizeof(uint32_t));
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::IS_TIME_SET).withOutputParameterReturning("value", &storedTimeSet, sizeof(uint32_t));

  // Meta setup (GDID set)
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::META_PARTITION_NAME).ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readString").withStringParameter("id", MetaUID::GDID).withOutputParameterReturning("str", "123456", 6).withUnsignedIntParameter("size", Constants::GDID_SIZE_MAX);

  // Rtc and SysTime setup (setting clock for the first time)
  DateTime dateTime = Constants::DEFAULT_DATETIME;
  mock().expectOneCall("RtcMock::setup").andReturnValue(true);
  mock().expectOneCall("RtcMock::setCurrentTime").withParameterOfType("DateTime", "dateTime", &dateTime);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime").ignoreOtherParameters();
  mock().expectOneCall("SystemTimeDriverMock::setUnixTime").ignoreOtherParameters();
  mock().expectOneCall("PersistentStorageDriverMock::writeValue").withStringParameter("id", SettingUID::IS_TIME_SET).withParameter("value", static_cast<uint32_t>(true));

  // SystemTime tick test (successful)
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(1U);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(3U);

  // RU selfTest and configuration check
  mock().expectOneCall("RUMock::performSelfTest").andReturnValue(true);
  mock().expectOneCall("RUMock::updateConfig").ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("RUMock::isHighPowerVersion").andReturnValue(true);


  // CryptoDriver setup
  mock().expectOneCall("CryptoDriverMock::resetAndInit").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::isProvisioned").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readCertificates").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readSerialNumber").ignoreOtherParameters();

  // Wifi init
  mock().expectOneCall("WifiDriverMock::resetAndInit").andReturnValue(false);

  // BT init
  mock().expectOneCall("BTDriverMock::resetAndInit").andReturnValue(true);

  // Run selfTest and check results
  bool result = selfTest.run(partitionMock, timerDummy, displayMock, rtcMock, systemTimeMock, ruMock, cryptoMock, btMock, wifiMock, settingsStorageMock, metaStorageMock);
  CHECK_FALSE(result);
  CHECK_EQUAL(16, selfTest.getErrorCode());
  CHECK_FALSE(selfTest.isFatalError());
}

TEST(powerOnSelfTest, testBluetoothInitFails)
{
  // Ignore other calls, i.e. calls for partition info
  mock().ignoreOtherCalls();

  // Display setup
  mock().expectOneCall("setup");

  // Storage setup - normal mode, isTimeSet assumed false
  uint32_t storedRunMode = static_cast<uint32_t>(RunMode::normal);
  uint32_t storedTimeSet = false;
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::SETTINGS_PARTITION_NAME).withBoolParameter("readOnly", false).andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::RUN_MODE).withOutputParameterReturning("value", &storedRunMode, sizeof(uint32_t));
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::IS_TIME_SET).withOutputParameterReturning("value", &storedTimeSet, sizeof(uint32_t));

  // Meta setup (GDID set)
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::META_PARTITION_NAME).ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readString").withStringParameter("id", MetaUID::GDID).withOutputParameterReturning("str", "123456", 6).withUnsignedIntParameter("size", Constants::GDID_SIZE_MAX);

  // Rtc and SysTime setup (setting clock for the first time)
  DateTime dateTime = Constants::DEFAULT_DATETIME;
  mock().expectOneCall("RtcMock::setup").andReturnValue(true);
  mock().expectOneCall("RtcMock::setCurrentTime").withParameterOfType("DateTime", "dateTime", &dateTime);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime").ignoreOtherParameters();
  mock().expectOneCall("SystemTimeDriverMock::setUnixTime").ignoreOtherParameters();
  mock().expectOneCall("PersistentStorageDriverMock::writeValue").withStringParameter("id", SettingUID::IS_TIME_SET).withParameter("value", static_cast<uint32_t>(true));

  // SystemTime tick test (successful)
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(1U);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(3U);

  // RU selfTest and configuration check
  mock().expectOneCall("RUMock::performSelfTest").andReturnValue(true);
  mock().expectOneCall("RUMock::updateConfig").ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("RUMock::isHighPowerVersion").andReturnValue(true);


  // CryptoDriver setup
  mock().expectOneCall("CryptoDriverMock::resetAndInit").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::isProvisioned").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readCertificates").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readSerialNumber").ignoreOtherParameters();

  // Wifi init
  mock().expectOneCall("WifiDriverMock::resetAndInit").andReturnValue(true);

  // BT init
  mock().expectOneCall("BTDriverMock::resetAndInit").andReturnValue(false);

  // Run selfTest and check results
  bool result = selfTest.run(partitionMock, timerDummy, displayMock, rtcMock, systemTimeMock, ruMock, cryptoMock, btMock, wifiMock, settingsStorageMock, metaStorageMock);
  CHECK_FALSE(result);
  CHECK_EQUAL(32, selfTest.getErrorCode());
  CHECK_FALSE(selfTest.isFatalError());
}

TEST(powerOnSelfTest, testSelfTestSucceedsWithTimeBeingSetForTheFirstTime)
{
  // Ignore other calls, i.e. calls for partition info
  mock().ignoreOtherCalls();

  // Display setup
  mock().expectOneCall("setup");

  // Storage setup - normal mode, isTimeSet assumed false
  uint32_t storedRunMode = static_cast<uint32_t>(RunMode::normal);
  uint32_t storedTimeSet = false;
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::SETTINGS_PARTITION_NAME).withBoolParameter("readOnly", false).andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::RUN_MODE).withOutputParameterReturning("value", &storedRunMode, sizeof(uint32_t));
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::IS_TIME_SET).withOutputParameterReturning("value", &storedTimeSet, sizeof(uint32_t));

  // Meta setup (GDID set)
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::META_PARTITION_NAME).ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readString").withStringParameter("id", MetaUID::GDID).withOutputParameterReturning("str", "123456", 6).withUnsignedIntParameter("size", Constants::GDID_SIZE_MAX);

  // Rtc and SysTime setup (setting clock for the first time)
  DateTime dateTime = Constants::DEFAULT_DATETIME;
  mock().expectOneCall("RtcMock::setup").andReturnValue(true);
  mock().expectOneCall("RtcMock::setCurrentTime").withParameterOfType("DateTime", "dateTime", &dateTime);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime").ignoreOtherParameters();
  mock().expectOneCall("SystemTimeDriverMock::setUnixTime").ignoreOtherParameters();
  mock().expectOneCall("PersistentStorageDriverMock::writeValue").withStringParameter("id", SettingUID::IS_TIME_SET).withParameter("value", static_cast<uint32_t>(true));

  // SystemTime tick test (successful)
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(1U);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(3U);

  // RU selfTest and configuration check
  mock().expectOneCall("RUMock::performSelfTest").andReturnValue(true);
  mock().expectOneCall("RUMock::updateConfig").ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("RUMock::isHighPowerVersion").andReturnValue(true);


  // CryptoDriver setup
  mock().expectOneCall("CryptoDriverMock::resetAndInit").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::isProvisioned").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readCertificates").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readSerialNumber").ignoreOtherParameters();

  // Wifi init
  mock().expectOneCall("WifiDriverMock::resetAndInit").andReturnValue(true);

  // BT init
  mock().expectOneCall("BTDriverMock::resetAndInit").andReturnValue(true);

  // Run selfTest and check results
  bool result = selfTest.run(partitionMock, timerDummy, displayMock, rtcMock, systemTimeMock, ruMock, cryptoMock, btMock, wifiMock, settingsStorageMock, metaStorageMock);
  CHECK_TRUE(result);
  CHECK_EQUAL(0, selfTest.getErrorCode());
  CHECK_FALSE(selfTest.isFatalError());
}


TEST(powerOnSelfTest, testSelfTestSucceedsWithDefaultGDIDWhenInTestMode)
{
  // Ignore other calls, i.e. calls for partition info
  mock().ignoreOtherCalls();

  // Display setup
  mock().expectOneCall("setup");

  // Storage setup - normal mode, isTimeSet assumed false
  uint32_t storedRunMode = static_cast<uint32_t>(RunMode::test);
  uint32_t storedTimeSet = false;
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::SETTINGS_PARTITION_NAME).withBoolParameter("readOnly", false).andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::RUN_MODE).withOutputParameterReturning("value", &storedRunMode, sizeof(uint32_t));
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::IS_TIME_SET).withOutputParameterReturning("value", &storedTimeSet, sizeof(uint32_t));

  // Meta setup (GDID not set)
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::META_PARTITION_NAME).ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readString")
      .withStringParameter("id", MetaUID::GDID)
      .withOutputParameterReturning("str", "somebogus", 9)
      .withUnsignedIntParameter("size", Constants::GDID_SIZE_MAX)
      .andReturnValue(false);

  // Rtc and SysTime setup (setting clock for the first time)
  DateTime dateTime = Constants::DEFAULT_DATETIME;
  mock().expectOneCall("RtcMock::setup").andReturnValue(true);
  mock().expectOneCall("RtcMock::setCurrentTime").withParameterOfType("DateTime", "dateTime", &dateTime);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime").ignoreOtherParameters();
  mock().expectOneCall("SystemTimeDriverMock::setUnixTime").ignoreOtherParameters();
  mock().expectOneCall("PersistentStorageDriverMock::writeValue").withStringParameter("id", SettingUID::IS_TIME_SET).withParameter("value", static_cast<uint32_t>(true));

  // SystemTime tick test (successful)
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(1U);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(2U);

  // RU selfTest and configuration check
  mock().expectOneCall("RUMock::performSelfTest").andReturnValue(true);
  mock().expectOneCall("RUMock::updateConfig").ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("RUMock::isHighPowerVersion").andReturnValue(true);


  // CryptoDriver setup
  mock().expectOneCall("CryptoDriverMock::resetAndInit").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::isProvisioned").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readCertificates").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readSerialNumber").ignoreOtherParameters();

  // Wifi init
  mock().expectOneCall("WifiDriverMock::resetAndInit").andReturnValue(true);

  // BT init
  mock().expectOneCall("BTDriverMock::resetAndInit").andReturnValue(true);

  // Run selfTest and check results
  bool result = selfTest.run(partitionMock, timerDummy, displayMock, rtcMock, systemTimeMock, ruMock, cryptoMock, btMock, wifiMock, settingsStorageMock, metaStorageMock);
  CHECK_TRUE(result);
  CHECK_EQUAL(0, selfTest.getErrorCode());
  CHECK_FALSE(selfTest.isFatalError());
}


TEST(powerOnSelfTest, testSelfTestSucceedsWithTimeBeingLoadedFromRtc)
{
  // Ignore other calls, i.e. calls for partition info
  mock().ignoreOtherCalls();

  // Display setup
  mock().expectOneCall("setup");

  // Storage setup - normal mode, isTimeSet assumed false
  uint32_t storedRunMode = static_cast<uint32_t>(RunMode::normal);
  uint32_t storedTimeSet = true;
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::SETTINGS_PARTITION_NAME).withBoolParameter("readOnly", false).andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::RUN_MODE).withOutputParameterReturning("value", &storedRunMode, sizeof(uint32_t));
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::IS_TIME_SET).withOutputParameterReturning("value", &storedTimeSet, sizeof(uint32_t));

  // Meta setup (GDID set)
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::META_PARTITION_NAME).ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readString").withStringParameter("id", MetaUID::GDID).withOutputParameterReturning("str", "123456", 6).withUnsignedIntParameter("size", Constants::GDID_SIZE_MAX);

  // Rtc and SysTime setup (setting clock for the first time)
  DateTime dateTime = { 18, 5, 17, 9, 10, 11, WeekDay::THURSDAY };
  mock().expectOneCall("RtcMock::setup").andReturnValue(true);
  mock().expectOneCall("RtcMock::getCurrentTime").withOutputParameterOfTypeReturning("DateTime", "dateTime", &dateTime);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime").withParameterOfType("DateTime", "dateTime", &dateTime).andReturnValue(123456U);
  mock().expectOneCall("SystemTimeDriverMock::setUnixTime").withUnsignedIntParameter("unixTime", 123456U);

  // SystemTime tick test (successful)
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(1U);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(2U);

  // RU selfTest and configuration check
  mock().expectOneCall("RUMock::performSelfTest").andReturnValue(true);
  mock().expectOneCall("RUMock::updateConfig").ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("RUMock::isHighPowerVersion").andReturnValue(true);

  // CryptoDriver setup
  mock().expectOneCall("CryptoDriverMock::resetAndInit").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::isProvisioned").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readCertificates").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readSerialNumber").ignoreOtherParameters();

  // Wifi init
  mock().expectOneCall("WifiDriverMock::resetAndInit").andReturnValue(true);

  // BT init
  mock().expectOneCall("BTDriverMock::resetAndInit").andReturnValue(true);

  // Run selfTest and check results
  bool result = selfTest.run(partitionMock, timerDummy, displayMock, rtcMock, systemTimeMock, ruMock, cryptoMock, btMock, wifiMock, settingsStorageMock, metaStorageMock);
  CHECK_TRUE(result);
  CHECK_EQUAL(0, selfTest.getErrorCode());
  CHECK_FALSE(selfTest.isFatalError());
}


TEST(powerOnSelfTest, testCombinationOfSystemTimeErrorAndMetaInitError)
{
  // Ignore other calls, i.e. calls for partition info
  mock().ignoreOtherCalls();

  // Display setup
  mock().expectOneCall("setup");

  // Storage setup - normal mode, isTimeSet assumed false
  uint32_t storedRunMode = static_cast<uint32_t>(RunMode::normal);
  uint32_t storedTimeSet = false;
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::SETTINGS_PARTITION_NAME).withBoolParameter("readOnly", false).andReturnValue(true);
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::RUN_MODE).withOutputParameterReturning("value", &storedRunMode, sizeof(uint32_t));
  mock().expectOneCall("PersistentStorageDriverMock::readValue").withStringParameter("id", SettingUID::IS_TIME_SET).withOutputParameterReturning("value", &storedTimeSet, sizeof(uint32_t));

  // Meta setup
  mock().expectOneCall("PersistentStorageDriverMock::init").withStringParameter("partition", Constants::META_PARTITION_NAME).ignoreOtherParameters().andReturnValue(false);

  // Rtc and SysTime setup (setting clock for the first time)
  DateTime dateTime = Constants::DEFAULT_DATETIME;
  mock().expectOneCall("RtcMock::setup").andReturnValue(true);
  mock().expectOneCall("RtcMock::setCurrentTime").withParameterOfType("DateTime", "dateTime", &dateTime);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime").ignoreOtherParameters();
  mock().expectOneCall("SystemTimeDriverMock::setUnixTime").ignoreOtherParameters();
  mock().expectOneCall("PersistentStorageDriverMock::writeValue").withStringParameter("id", SettingUID::IS_TIME_SET).withParameter("value", static_cast<uint32_t>(true));

  // SystemTime tick test (error)
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(1U);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(4U);

  // RU selfTest and configuration check
  mock().expectOneCall("RUMock::performSelfTest").andReturnValue(true);
  mock().expectOneCall("RUMock::updateConfig").ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("RUMock::isHighPowerVersion").andReturnValue(true);

  // CryptoDriver setup
  mock().expectOneCall("CryptoDriverMock::resetAndInit").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::isProvisioned").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readCertificates").andReturnValue(true);
  mock().expectOneCall("CryptoDriverMock::readSerialNumber").ignoreOtherParameters();

  // Wifi init
  mock().expectOneCall("WifiDriverMock::resetAndInit").andReturnValue(true);

  // BT init
  mock().expectOneCall("BTDriverMock::resetAndInit").andReturnValue(true);

  // Run selfTest and check results
  bool result = selfTest.run(partitionMock, timerDummy, displayMock, rtcMock, systemTimeMock, ruMock, cryptoMock, btMock, wifiMock, settingsStorageMock, metaStorageMock);
  CHECK_FALSE(result);
  CHECK_EQUAL(66, selfTest.getErrorCode());
  CHECK_FALSE(selfTest.isFatalError());
}
