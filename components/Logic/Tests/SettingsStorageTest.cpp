#include "CppUTest\TestHarness.h"

#include "Constants.h"
#include "PersistentStorageDriverMock.h"
#include "SettingsStorage.h"


static PersistentStorageDriverDummy *driverDummy;
static SettingsStorage *storage;


TEST_GROUP(settingsStorage)
{
  TEST_SETUP()
  {
    driverDummy = new PersistentStorageDriverDummy();
    storage = new SettingsStorage(*driverDummy);
  }

  TEST_TEARDOWN()
  {
    delete storage;
    delete driverDummy;
  }
};


TEST(settingsStorage, verifyCorrectBehaviourForDriverDummyValues)
{
  uint32_t value = 0U;

  bool readNonExistingValueSucceeded = driverDummy->readValue("myValue", &value);
  CHECK_EQUAL(false, readNonExistingValueSucceeded);

  bool writeNewValueSucceeded = driverDummy->writeValue("newValue", 10U);
  CHECK_TRUE(writeNewValueSucceeded);

  bool readExistingValueSucceeded = driverDummy->readValue("newValue", &value);
  CHECK_TRUE(readExistingValueSucceeded);
  CHECK_EQUAL(10U, value);

  bool updateExistingValueSucceeded = driverDummy->writeValue("newValue", 20U);
  CHECK_TRUE(updateExistingValueSucceeded);

  readExistingValueSucceeded = driverDummy->readValue("newValue", &value);
  CHECK_TRUE(readExistingValueSucceeded);
  CHECK_EQUAL(20U, value);

  char strVal[5];
  bool readValueAsStringSucceeded = driverDummy->readString("newValue", strVal, 5);
  CHECK_EQUAL(false, readValueAsStringSucceeded);
}

TEST(settingsStorage, verifyCorrectBehaviourForDriverDummyStrings)
{
  char str[128U];

  bool readNonExistingStringSucceeded = driverDummy->readString("myString", str, sizeof(str));
  CHECK_EQUAL(false, readNonExistingStringSucceeded);

  bool writeNewStringSucceeded = driverDummy->writeString("newString", "contents");
  CHECK_TRUE(writeNewStringSucceeded);

  bool readExistingStringSucceeded = driverDummy->readString("newString", str, sizeof(str));
  CHECK_TRUE(readExistingStringSucceeded);
  STRNCMP_EQUAL("contents", str, sizeof(str));

  bool updateExistingStringSucceeded = driverDummy->writeString("newValue", "other");
  CHECK_TRUE(updateExistingStringSucceeded);

  readExistingStringSucceeded = driverDummy->readString("newValue", str, sizeof(str));
  CHECK_TRUE(readExistingStringSucceeded);
  STRNCMP_EQUAL("other", str, sizeof(str));

  bool readExistingStringWithInsufficientBufferSucceeded = driverDummy->readString("newValue", str, 5); // one too short due to '\0'
  CHECK_EQUAL(false, readExistingStringWithInsufficientBufferSucceeded);

  uint32_t value = 0U;
  bool readStringAsValueSucceeded = driverDummy->readValue("newValue", &value);
  CHECK_EQUAL(false, readStringAsValueSucceeded);
}

TEST(settingsStorage, verifyCorrectBehaviourForDriverDummyBlobs)
{
  const uint8_t blobWriteData[16U] = { 1U, 3U, 5U, 7U, 9U, 11U, 13U, 15U, 2U, 4U, 6U, 8U, 10U, 12U, 14U, 16U };
  uint8_t blobReadData[16U] = {};

  uint32_t size = sizeof(blobReadData);
  bool readNonExistingBlobSucceeded = driverDummy->readBlob("myBlob", blobReadData, size);
  CHECK_EQUAL(false, readNonExistingBlobSucceeded);
  CHECK_EQUAL(0U, size);

  size = sizeof(blobWriteData);
  bool writeNewBlobSucceeded = driverDummy->writeBlob("newBlob", blobWriteData, size);
  CHECK_TRUE(writeNewBlobSucceeded);

  size = sizeof(blobReadData);
  bool readExistingBlobSucceeded = driverDummy->readBlob("newBlob", blobReadData, size);
  CHECK_TRUE(readExistingBlobSucceeded);
  MEMCMP_EQUAL(blobWriteData, blobReadData, size);
  CHECK_EQUAL(16U, size);

  size = 10U;
  bool updateExistingStringSucceeded = driverDummy->writeBlob("newBlob", blobWriteData, size);
  CHECK_TRUE(updateExistingStringSucceeded);

  size = sizeof(blobReadData);
  readExistingBlobSucceeded = driverDummy->readBlob("newBlob", blobReadData, size);
  CHECK_TRUE(readExistingBlobSucceeded);
  MEMCMP_EQUAL(blobWriteData, blobReadData, size);
  CHECK_EQUAL(10U, size);

  size = 5U;
  bool readExistingStringWithInsufficientBufferSucceeded = driverDummy->readBlob("newBlob", blobReadData, size);
  CHECK_EQUAL(false, readExistingStringWithInsufficientBufferSucceeded);
  CHECK_EQUAL(0U, size);
}

TEST(settingsStorage, testWhenValueNotSetRetrieveShouldReturnFalse)
{
  RunMode::Enum runMode;
  CHECK_FALSE(storage->retrieveRunMode(runMode));
}

TEST(settingsStorage, testSettingAndRetrievingRunMode)
{
  RunMode::Enum runMode = RunMode::test;

  storage->storeRunMode(RunMode::normal);
  CHECK_TRUE(storage->retrieveRunMode(runMode));
  CHECK_EQUAL(RunMode::normal, runMode);

  storage->storeRunMode(RunMode::test);
  CHECK_TRUE(storage->retrieveRunMode(runMode));
  CHECK_EQUAL(RunMode::test, runMode);
}

TEST(settingsStorage, testSettingAndRetrievingWifiSSID)
{
  char wifiSSID[Constants::WIFI_SSID_SIZE_MAX] = {};

  storage->storeWifiSSID("my_wifi_ssid");
  CHECK_TRUE(storage->retrieveWifiSSID(wifiSSID, sizeof(wifiSSID)));
  STRNCMP_EQUAL("my_wifi_ssid", wifiSSID, sizeof(wifiSSID));
}

TEST(settingsStorage, testSettingAndRetrievingWifiPass)
{
  char wifiPass[Constants::WIFI_PASSWORD_SIZE_MAX] = {};

  storage->storeWifiPassword("my_wifi_password");
  CHECK_TRUE(storage->retrieveWifiPassword(wifiPass, sizeof(wifiPass)));
  STRNCMP_EQUAL("my_wifi_password", wifiPass, sizeof(wifiPass));
}

TEST(settingsStorage, testSettingAndRetrievingUtcOffset)
{
  int32_t utcOffset = 0;

  storage->storeUtcOffset(43200); // +12 hrs
  CHECK_TRUE(storage->retrieveUtcOffset(utcOffset));
  CHECK_EQUAL(43200, utcOffset);

  storage->storeUtcOffset(-43200); // -12 hrs
  CHECK_TRUE(storage->retrieveUtcOffset(utcOffset));
  CHECK_EQUAL(-43200, utcOffset);

  storage->storeUtcOffset(0);
  CHECK_TRUE(storage->retrieveUtcOffset(utcOffset));
  CHECK_EQUAL(0, utcOffset);
}

TEST(settingsStorage, testSettingAndRetrievingIsTimeSet)
{
  bool timeSet;

  storage->storeIsTimeSet(false);
  CHECK_TRUE(storage->retrieveIsTimeSet(timeSet));
  CHECK_EQUAL(false, timeSet);

  storage->storeIsTimeSet(true);
  CHECK_TRUE(storage->retrieveIsTimeSet(timeSet));
  CHECK_EQUAL(true, timeSet);
}

TEST(settingsStorage, testSettingAndRetrievingComfortTemperature)
{
  int32_t setPoint;
  storage->storeComfortTemperatureSetPoint(20000);

  bool result = storage->retrieveComfortTemperatureSetPoint(setPoint);
  CHECK_TRUE(result);
  CHECK_EQUAL(20000, setPoint);
}

TEST(settingsStorage, testSettingAndRetrievingEcoTemperature)
{
  int32_t setPoint;
  storage->storeEcoTemperatureSetPoint(10000);

  CHECK_TRUE(storage->retrieveEcoTemperatureSetPoint(setPoint));
  CHECK_EQUAL(10000, setPoint);
}

TEST(settingsStorage, testSettingComfortAndRetrievingEcoTemperatureShouldFail)
{
  int32_t setPoint;
  storage->storeComfortTemperatureSetPoint(20000);

  CHECK_FALSE(storage->retrieveEcoTemperatureSetPoint(setPoint));
}

TEST(settingsStorage, testSettingAndRetrievingPrimaryHeatingMode)
{
  HeatingMode::Enum heatingMode;
  storage->storePrimaryHeatingMode(HeatingMode::comfort);

  CHECK_TRUE(storage->retrievePrimaryHeatingMode(heatingMode));
  CHECK_EQUAL(HeatingMode::comfort, heatingMode);
}

TEST(settingsStorage, testSettingAndRetrievingOverrideNone)
{
  Override override(OverrideType::none, HeatingMode::comfort);
  storage->storeOverride(override);

  DateTime dateTime = { 18, 7, 5, 18, 38, 17, WeekDay::THURSDAY };
  Override outputOverride(OverrideType::timed, HeatingMode::eco, dateTime);
  CHECK_TRUE(storage->retrieveOverride(outputOverride));
  CHECK_TRUE(override == outputOverride);
}

TEST(settingsStorage, testSettingAndRetrievingOverrideNow)
{
  Override override(OverrideType::now, HeatingMode::eco);
  storage->storeOverride(override);

  DateTime dateTime = { 18, 7, 5, 18, 38, 17, WeekDay::THURSDAY };
  Override outputOverride(OverrideType::none, HeatingMode::comfort, dateTime);
  CHECK_TRUE(storage->retrieveOverride(outputOverride));
  CHECK_TRUE(override == outputOverride);
}

TEST(settingsStorage, testSettingAndRetrievingOverrideConstant)
{
  Override override(OverrideType::constant, HeatingMode::antiFrost);
  storage->storeOverride(override);

  DateTime dateTime = { 18, 7, 5, 18, 38, 17, WeekDay::THURSDAY };
  Override outputOverride(OverrideType::none, HeatingMode::comfort, dateTime);
  CHECK_TRUE(storage->retrieveOverride(outputOverride));
  CHECK_TRUE(override == outputOverride);
}

TEST(settingsStorage, testSettingAndRetrievingOverrideTimed)
{
  DateTime dateTime = { 18, 7, 5, 18, 38, 17, WeekDay::THURSDAY };
  Override override(OverrideType::timed, HeatingMode::off, dateTime);
  storage->storeOverride(override);

  Override outputOverride(OverrideType::none, HeatingMode::comfort);
  CHECK_TRUE(storage->retrieveOverride(outputOverride));
  CHECK_TRUE(override == outputOverride);
}

TEST(settingsStorage, testSettingAndRetrievingIsButtonLockOn)
{
  bool enabled;

  storage->storeIsButtonLockOn(false);
  CHECK_TRUE(storage->retrieveIsButtonLockOn(enabled));
  CHECK_EQUAL(false, enabled);

  storage->storeIsButtonLockOn(true);
  CHECK_TRUE(storage->retrieveIsButtonLockOn(enabled));
  CHECK_EQUAL(true, enabled);
}


TEST(settingsStorage, testSettingAndRetrievingWeekProgramDataForMonday)
{
  uint8_t weekProgramData[20U] = { 1U, 1U, 2U, 2U, 3U, 3U, 4U, 4U, 5U, 5U, 6U, 6U, 7U, 7U, 8U, 8U, 9U, 9U, 10U, 10U };
  uint8_t outputData[20U] = {};

  storage->storeWeekProgramDataForDay(WeekDay::MONDAY, weekProgramData);
  CHECK_TRUE(storage->retrieveWeekProgramDataForDay(WeekDay::MONDAY, outputData));
  MEMCMP_EQUAL(outputData, weekProgramData, sizeof(outputData));
}


TEST(settingsStorage, testSettingAndRetrievingWeekProgramDataForThursday)
{
  uint8_t weekProgramData[20U] = { 10U, 1U, 2U, 27U, 3U, 3U, 4U, 4U, 5U, 54U, 6U, 6U, 7U, 7U, 8U, 8U, 9U, 9U, 10U, 10U };
  uint8_t outputData[20U] = {};

  storage->storeWeekProgramDataForDay(WeekDay::THURSDAY, weekProgramData);
  CHECK_TRUE(storage->retrieveWeekProgramDataForDay(WeekDay::THURSDAY, outputData));
  MEMCMP_EQUAL(outputData, weekProgramData, sizeof(outputData));
}


TEST(settingsStorage, testSettingAndRetrievingWeekProgramDataForSunday)
{
  uint8_t weekProgramData[20U] = { 13U, 1U, 2U, 2U, 3U, 3U, 4U, 4U, 5U, 5U, 6U, 6U, 79U, 7U, 8U, 84U, 9U, 9U, 10U, 10U };
  uint8_t outputData[20U] = {};

  storage->storeWeekProgramDataForDay(WeekDay::SUNDAY, weekProgramData);
  CHECK_TRUE(storage->retrieveWeekProgramDataForDay(WeekDay::SUNDAY, outputData));
  MEMCMP_EQUAL(outputData, weekProgramData, sizeof(outputData));
}


TEST(settingsStorage, testSettingWeekProgramForOneDayDoesNotAffectOtherDays)
{
  uint8_t weekProgramData[20U] = { 1U, 1U, 2U, 2U, 3U, 3U, 4U, 4U, 5U, 5U, 6U, 6U, 7U, 7U, 8U, 8U, 9U, 9U, 10U, 10U };

  storage->storeWeekProgramDataForDay(WeekDay::MONDAY, weekProgramData);
  CHECK_FALSE(storage->retrieveWeekProgramDataForDay(WeekDay::TUESDAY, weekProgramData));
}

TEST(settingsStorage, testSettingAndRetrievingFirmwareUpgradeAttempts)
{
  uint8_t attempts = 0U;
  storage->storeFirmwareUpgradeAttempts(3U);

  CHECK_TRUE(storage->retrieveFirmwareUpgradeAttempts(attempts));
  CHECK_EQUAL(3U, attempts);
}

TEST(settingsStorage, testSettingAndRetrievingFirmwareID)
{
  char firmwareID[Constants::FW_DOWNLOAD_ID_MAX_LENGTH] = {};

  storage->storeFirmwareID("file_name_000102.bin");
  CHECK_TRUE(storage->retrieveFirmwareID(firmwareID, sizeof(firmwareID)));
  STRNCMP_EQUAL("file_name_000102.bin", firmwareID, sizeof(firmwareID));
}

TEST(settingsStorage, testSettingAndRetrievingFirmwareVersionCode)
{
  uint32_t code = 0U;
  storage->storeFirmwareVersionCode(232134U);

  CHECK_TRUE(storage->retrieveFirmwareVersionCode(code));
  CHECK_EQUAL(232134U, code);
}
