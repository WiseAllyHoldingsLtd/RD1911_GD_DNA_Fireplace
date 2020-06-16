#include "CppUTest\TestHarness.h"
#include "CppUTestExt\MockSupport.h"

#include "Settings.h"
#include "Constants.h"
#include "DateTime.h"
#include "RtcMock.h"
#include "SystemTimeDriverMock.h"
#include "PersistentStorageDriverMock.h"
#include "SettingsStorage.h"
#include "mock\TimerDriverMock.h"


static Settings* settings;
static RtcMock rtc;
static SystemTimeDriverMock sysTime;
static PersistentStorageDriverDummy * storageDriverDummy;
static SettingsStorage * storage;
static TimerDriverDummy timer(0, 1);

TEST_GROUP(settings)
{
  TEST_SETUP()
  {
    mock().installComparator("DateTime", dateTimeComparator);
    mock().installCopier("DateTime", dateTimeCopier);
    storageDriverDummy = new PersistentStorageDriverDummy();
    storage = new SettingsStorage(*storageDriverDummy);
    settings = new Settings(*storage , rtc, sysTime, timer);
  }

  TEST_TEARDOWN()
  {
    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
    delete settings;
    delete storage;
    delete storageDriverDummy;
  }
};

TEST(settings, testGetAndSetRunMode)
{
  settings->setRunMode(RunMode::normal);
  CHECK_EQUAL(RunMode::normal, settings->getRunMode());

  settings->setRunMode(RunMode::test);
  CHECK_EQUAL(RunMode::test, settings->getRunMode());
}

TEST(settings, testIsTestMode)
{
  settings->setRunMode(RunMode::normal);
  CHECK_FALSE(settings->isTestMode());

  settings->setRunMode(RunMode::test);
  CHECK_TRUE(settings->isTestMode());
}

TEST(settings, testSetInvalidRunModeInFactSetsDefault)
{
  settings->setRunMode(static_cast<RunMode::Enum>(9));
  CHECK_EQUAL(Constants::DEFAULT_RUN_MODE, settings->getRunMode());
}

TEST(settings, testGetAndSetWifiSSID)
{
  char ssid[Constants::WIFI_SSID_SIZE_MAX] = {};

  settings->setWifiSSID("willYouBeMyWifi");
  settings->getWifiSSID(ssid, sizeof(ssid));
  STRNCMP_EQUAL("willYouBeMyWifi", ssid, sizeof(ssid));

  settings->setWifiSSID("shorter");
  settings->getWifiSSID(ssid, sizeof(ssid));
  STRNCMP_EQUAL("shorter", ssid, sizeof(ssid));


  settings->setWifiSSID("evenLongerThanBefore");
  settings->getWifiSSID(ssid, sizeof(ssid));
  STRNCMP_EQUAL("evenLongerThanBefore", ssid, sizeof(ssid));
}

TEST(settings, testGetWifiSSIDWithTooShortBufferProvided)
{
  char ssid[2U];

  settings->setWifiSSID("myWifi");
  settings->getWifiSSID(ssid, sizeof(ssid));
  STRNCMP_EQUAL("m", ssid, sizeof(ssid));
  CHECK_EQUAL(1U, strnlen(ssid, sizeof(ssid)));
}

TEST(settings, testSetWifiSSIDLongerThanSupported)
{
  char ssid[Constants::WIFI_SSID_SIZE_MAX + 2U];

  for (uint8_t i = 0; i < sizeof(ssid); i++)
  {
    ssid[i] = 's';
  }

  settings->setWifiSSID(ssid); /* ssid is too large and even has no zero term. */
  settings->getWifiSSID(ssid, sizeof(ssid));

  CHECK_EQUAL(Constants::WIFI_SSID_SIZE_MAX - 1U, strnlen(ssid, sizeof(ssid)));
  CHECK_EQUAL('s', ssid[0U]);
  CHECK_EQUAL('s', ssid[Constants::WIFI_SSID_SIZE_MAX - 2U]);
}

TEST(settings, testGetAndSetWifiPassword)
{
  char password[Constants::WIFI_PASSWORD_SIZE_MAX] = {};

  settings->setWifiPassword("myPrecious");
  settings->getWifiPassword(password, sizeof(password));
  STRNCMP_EQUAL("myPrecious", password, sizeof(password));

  settings->setWifiPassword("secret");
  settings->getWifiPassword(password, sizeof(password));
  STRNCMP_EQUAL("secret", password, sizeof(password));


  settings->setWifiPassword("myPasswordIsSecret");
  settings->getWifiPassword(password, sizeof(password));
  STRNCMP_EQUAL("myPasswordIsSecret", password, sizeof(password));
}

TEST(settings, testGetWifiPasswordWithTooShortBufferProvided)
{
  char password[2U];

  settings->setWifiPassword("myWifi");
  settings->getWifiPassword(password, sizeof(password));
  STRNCMP_EQUAL("m", password, sizeof(password));
  CHECK_EQUAL(1U, strnlen(password, sizeof(password)));
}

TEST(settings, testSetWifiPasswordLongerThanSupported)
{
  char password[Constants::WIFI_PASSWORD_SIZE_MAX + 2U];

  for (uint8_t i = 0; i < sizeof(password); i++)
  {
    password[i] = 'p';
  }

  settings->setWifiPassword(password); /* password is too large and even has no zero term. */
  settings->getWifiPassword(password, sizeof(password));

  CHECK_EQUAL(Constants::WIFI_PASSWORD_SIZE_MAX - 1U, strnlen(password, sizeof(password)));
  CHECK_EQUAL('p', password[0U]);
  CHECK_EQUAL('p', password[Constants::WIFI_PASSWORD_SIZE_MAX - 2U]);
}

TEST(settings, testGetAndSetComfortSetPoint)
{
  settings->setComfortTemperatureSetPoint(17000);
  CHECK_EQUAL(17000, settings->getComfortTemperatureSetPoint());
  settings->setComfortTemperatureSetPoint(16000);
  CHECK_EQUAL(16000, settings->getComfortTemperatureSetPoint());
}

TEST(settings, testSetComfortSetPointTooLow)
{
  settings->setComfortTemperatureSetPoint(4999);
  CHECK_EQUAL(21000, settings->getComfortTemperatureSetPoint());
}

TEST(settings, testSetComfortSetPointTooHigh)
{
  settings->setComfortTemperatureSetPoint(70001);
  CHECK_EQUAL(21000, settings->getComfortTemperatureSetPoint());
}

TEST(settings, testGetAndSetEcoSetPoint)
{
  settings->setEcoTemperatureSetPoint(17000);
  CHECK_EQUAL(17000, settings->getEcoTemperatureSetPoint());
  settings->setEcoTemperatureSetPoint(16000);
  CHECK_EQUAL(16000, settings->getEcoTemperatureSetPoint());
}

TEST(settings, testSetEcoSetPointTooLow)
{
  settings->setEcoTemperatureSetPoint(4999);
  CHECK_EQUAL(18000, settings->getEcoTemperatureSetPoint());
}

TEST(settings, testSetEcoSetPointTooHigh)
{
  settings->setEcoTemperatureSetPoint(70001);
  CHECK_EQUAL(18000, settings->getEcoTemperatureSetPoint());
}

TEST(settings, testGetSetPointWhenInComfortMode)
{
  settings->setComfortTemperatureSetPoint(20000);
  settings->setEcoTemperatureSetPoint(17000);
  settings->setPrimaryHeatingMode(HeatingMode::comfort);
  settings->setOverride(Override(OverrideType::none, HeatingMode::comfort));
  CHECK_EQUAL(20000, settings->getSetPoint());
}

TEST(settings, testGetSetPointWhenInEcoMode)
{
  settings->setComfortTemperatureSetPoint(20000);
  settings->setEcoTemperatureSetPoint(17000);
  settings->setPrimaryHeatingMode(HeatingMode::eco);
  settings->setOverride(Override(OverrideType::none, HeatingMode::comfort));
  CHECK_EQUAL(17000, settings->getSetPoint());
}

TEST(settings, testGetSetPointWhenInAntiFrostMode)
{
  settings->setComfortTemperatureSetPoint(20000);
  settings->setEcoTemperatureSetPoint(17000);
  settings->setPrimaryHeatingMode(HeatingMode::antiFrost);
  settings->setOverride(Override(OverrideType::none, HeatingMode::comfort));
  CHECK_EQUAL(7000, settings->getSetPoint());
}

TEST(settings, testGetSetPointWhenInOffMode)
{
  settings->setComfortTemperatureSetPoint(20000);
  settings->setEcoTemperatureSetPoint(17000);
  settings->setPrimaryHeatingMode(HeatingMode::off);
  settings->setOverride(Override(OverrideType::none, HeatingMode::comfort));
  CHECK_EQUAL(INT32_MIN, settings->getSetPoint());
}

TEST(settings, testGetAndSetHeatingMode)
{
  settings->setPrimaryHeatingMode(HeatingMode::antiFrost);
  CHECK_EQUAL(HeatingMode::antiFrost, settings->getHeatingMode());
  settings->setPrimaryHeatingMode(HeatingMode::comfort);
  CHECK_EQUAL(HeatingMode::comfort, settings->getHeatingMode());
}

TEST(settings, testIncreaseComfortSetPoint)
{
  settings->setPrimaryHeatingMode(HeatingMode::comfort);
  settings->setComfortTemperatureSetPoint(21000);
  settings->increaseSetPoint();
  CHECK_EQUAL(22000, settings->getComfortTemperatureSetPoint());
}

TEST(settings, testIncreaseEcoSetPoint)
{
  settings->setPrimaryHeatingMode(HeatingMode::eco);
  settings->setEcoTemperatureSetPoint(21000);
  settings->increaseSetPoint();
  CHECK_EQUAL(22000, settings->getEcoTemperatureSetPoint());
}

TEST(settings, testIncreaseSetPointToLimit)
{
  settings->setPrimaryHeatingMode(HeatingMode::eco);
  settings->setEcoTemperatureSetPoint(29000);
  settings->increaseSetPoint();
  CHECK_EQUAL(30000, settings->getEcoTemperatureSetPoint());
}

TEST(settings, testIncreaseSetPointAtLimit)
{
  settings->setPrimaryHeatingMode(HeatingMode::comfort);
  settings->setComfortTemperatureSetPoint(30000);
  settings->increaseSetPoint();
  CHECK_EQUAL(30000, settings->getComfortTemperatureSetPoint());
}

TEST(settings, testSetAndIncreaseSetPointAboveLimit)
{
  settings->setPrimaryHeatingMode(HeatingMode::comfort);
  settings->setComfortTemperatureSetPoint(41000);
  settings->increaseSetPoint();
  CHECK_EQUAL(22000, settings->getComfortTemperatureSetPoint());
}

TEST(settings, testDecreaseComfortSetPoint)
{
  settings->setPrimaryHeatingMode(HeatingMode::comfort);
  settings->setComfortTemperatureSetPoint(21000);
  settings->decreaseSetPoint();
  CHECK_EQUAL(20000, settings->getComfortTemperatureSetPoint());
}

TEST(settings, testDecreaseEcoSetPoint)
{
  settings->setPrimaryHeatingMode(HeatingMode::eco);
  settings->setEcoTemperatureSetPoint(21000);
  settings->decreaseSetPoint();
  CHECK_EQUAL(20000, settings->getEcoTemperatureSetPoint());
}

TEST(settings, testDecreaseSetPointToLimit)
{
  settings->setPrimaryHeatingMode(HeatingMode::eco);
  settings->setEcoTemperatureSetPoint(8000);
  settings->decreaseSetPoint();
  CHECK_EQUAL(7000, settings->getEcoTemperatureSetPoint());
}

TEST(settings, testDecreaseSetPointAtLimit)
{
  settings->setPrimaryHeatingMode(HeatingMode::comfort);
  settings->setComfortTemperatureSetPoint(7000);
  settings->decreaseSetPoint();
  CHECK_EQUAL(7000, settings->getComfortTemperatureSetPoint());
}

TEST(settings, testDecreaseSetPointBelowLimit)
{
  settings->setPrimaryHeatingMode(HeatingMode::comfort);
  settings->setComfortTemperatureSetPoint(7000);
  settings->decreaseSetPoint();
  CHECK_EQUAL(7000, settings->getComfortTemperatureSetPoint());
}

TEST(settings, testDontDecreaseComfortSetPointWhenInFrost)
{
  settings->setPrimaryHeatingMode(HeatingMode::antiFrost);
  settings->setComfortTemperatureSetPoint(10000);
  settings->decreaseSetPoint();
  CHECK_EQUAL(10000, settings->getComfortTemperatureSetPoint());
}

TEST(settings, testDontDecreaseEcoSetPointWhenInFrost)
{
  settings->setPrimaryHeatingMode(HeatingMode::antiFrost);
  settings->setEcoTemperatureSetPoint(10000);
  settings->decreaseSetPoint();
  CHECK_EQUAL(10000, settings->getEcoTemperatureSetPoint());
}

TEST(settings, testDontIncreaseComfortSetPointWhenInFrost)
{
  settings->setPrimaryHeatingMode(HeatingMode::antiFrost);
  settings->setComfortTemperatureSetPoint(21000);
  settings->increaseSetPoint();
  CHECK_EQUAL(21000, settings->getComfortTemperatureSetPoint());
}

TEST(settings, testDontIncreaseEcoSetPointWhenInFrost)
{
  settings->setPrimaryHeatingMode(HeatingMode::antiFrost);
  settings->setEcoTemperatureSetPoint(21000);
  settings->increaseSetPoint();
  CHECK_EQUAL(21000, settings->getEcoTemperatureSetPoint());
}

TEST(settings, testDontDecreaseComfortSetPointWhenOff)
{
  settings->setPrimaryHeatingMode(HeatingMode::off);
  settings->setComfortTemperatureSetPoint(10000);
  settings->decreaseSetPoint();
  CHECK_EQUAL(10000, settings->getComfortTemperatureSetPoint());
}

TEST(settings, testDontDecreaseEcoSetPointWhenOff)
{
  settings->setPrimaryHeatingMode(HeatingMode::off);
  settings->setEcoTemperatureSetPoint(10000);
  settings->decreaseSetPoint();
  CHECK_EQUAL(10000, settings->getEcoTemperatureSetPoint());
}

TEST(settings, testDontIncreaseComfortSetPointWhenOff)
{
  settings->setPrimaryHeatingMode(HeatingMode::off);
  settings->setComfortTemperatureSetPoint(21000);
  settings->increaseSetPoint();
  CHECK_EQUAL(21000, settings->getComfortTemperatureSetPoint());
}

TEST(settings, testDontIncreaseEcoSetPointWhenOff)
{
  settings->setPrimaryHeatingMode(HeatingMode::off);
  settings->setEcoTemperatureSetPoint(21000);
  settings->increaseSetPoint();
  CHECK_EQUAL(21000, settings->getEcoTemperatureSetPoint());
}

TEST(settings, testGetPrimaryHeatingMode)
{
  settings->setPrimaryHeatingMode(HeatingMode::antiFrost);
  CHECK_EQUAL(HeatingMode::antiFrost, settings->getPrimaryHeatingMode());
  settings->setPrimaryHeatingMode(HeatingMode::comfort);
  CHECK_EQUAL(HeatingMode::comfort, settings->getPrimaryHeatingMode());
}

TEST(settings, testSetPrimaryHeatingModeToInvalidValue)
{
  settings->setPrimaryHeatingMode(static_cast<HeatingMode::Enum>(200));
  CHECK_EQUAL(HeatingMode::comfort, settings->getPrimaryHeatingMode());
}

TEST(settings, testGetAndSetOverride)
{
  DateTime dateTime = { 17, 12, 11, 12, 13, 14, WeekDay::MONDAY };
  Override override(OverrideType::timed, HeatingMode::eco, dateTime);
  Override outputOverride;

  settings->setOverride(override);
  settings->getOverride(outputOverride);
  CHECK_TRUE(override == outputOverride);

  override = Override(OverrideType::none, HeatingMode::antiFrost);
  settings->setOverride(override);
  settings->getOverride(outputOverride);
  CHECK_TRUE(override == outputOverride);
}

TEST(settings, testGetOverrideHeatingMode)
{
  Override override(OverrideType::none, HeatingMode::comfort);
  settings->setOverride(override);
  CHECK_EQUAL(HeatingMode::comfort, settings->getOverrideHeatingMode());

  override = Override(OverrideType::none, HeatingMode::eco);
  settings->setOverride(override);
  CHECK_EQUAL(HeatingMode::eco, settings->getOverrideHeatingMode());

  override = Override(OverrideType::none, HeatingMode::antiFrost);
  settings->setOverride(override);
  CHECK_EQUAL(HeatingMode::antiFrost, settings->getOverrideHeatingMode());

  override = Override(OverrideType::none, HeatingMode::off);
  settings->setOverride(override);
  CHECK_EQUAL(HeatingMode::off, settings->getOverrideHeatingMode());
}

TEST(settings, testIsOverrideActive)
{
  Override override(OverrideType::none, HeatingMode::comfort);
  settings->setOverride(override);
  CHECK_FALSE(settings->isOverrideActive());

  override = Override(OverrideType::constant, HeatingMode::comfort);
  settings->setOverride(override);
  CHECK_TRUE(settings->isOverrideActive());

  override = Override(OverrideType::now, HeatingMode::comfort);
  settings->setOverride(override);
  CHECK_TRUE(settings->isOverrideActive());

  override = Override(OverrideType::timed, HeatingMode::comfort);
  settings->setOverride(override);
  CHECK_TRUE(settings->isOverrideActive());
}

TEST(settings, testToggleOverrideFromOnToOff)
{
  Override override(OverrideType::timed, HeatingMode::eco);
  settings->setOverride(override);
  settings->toggleOverride();
  CHECK_FALSE(settings->isOverrideActive());
}

TEST(settings, testToggleOverrideFromOffToOnWhenInComfort)
{
  settings->setPrimaryHeatingMode(HeatingMode::comfort);
  CHECK_FALSE(settings->isOverrideActive());

  settings->toggleOverride();
  CHECK_TRUE(settings->isOverrideActive());
  CHECK_EQUAL(HeatingMode::eco, settings->getOverrideHeatingMode());
}

TEST(settings, testToggleOverrideFromOffToOnWhenInEco)
{
  settings->setPrimaryHeatingMode(HeatingMode::eco);
  CHECK_FALSE(settings->isOverrideActive());

  settings->toggleOverride();
  CHECK_TRUE(settings->isOverrideActive());
  CHECK_EQUAL(HeatingMode::comfort, settings->getOverrideHeatingMode());
}

TEST(settings, testToggleOverrideFromOffToOnWhenInAntiFrost)
{
  settings->setPrimaryHeatingMode(HeatingMode::antiFrost);
  CHECK_FALSE(settings->isOverrideActive());

  settings->toggleOverride();
  CHECK_TRUE(settings->isOverrideActive());
  CHECK_EQUAL(HeatingMode::comfort, settings->getOverrideHeatingMode());
}

TEST(settings, testToggleOverrideFromOffToOnWhenInOff)
{
  settings->setPrimaryHeatingMode(HeatingMode::off);
  CHECK_FALSE(settings->isOverrideActive());

  settings->toggleOverride();
  CHECK_TRUE(settings->isOverrideActive());
  CHECK_EQUAL(HeatingMode::comfort, settings->getOverrideHeatingMode());
}

TEST(settings, testButtonLockIsInitialisedToFalse)
{
  CHECK_EQUAL(false, settings->isButtonLockOn());
}

TEST(settings, testToggleAndGetButtonLock)
{
  CHECK_EQUAL(false, settings->isButtonLockOn());
  settings->toggleButtonLock();
  CHECK_EQUAL(true, settings->isButtonLockOn());
}


TEST(settings, testUnlockButtonLockSetsButtonLockToFalseWhenTrue)
{
   //default is button lock = off
  CHECK_EQUAL(false, settings->isButtonLockOn());
  settings->toggleButtonLock();
  CHECK_EQUAL(true, settings->isButtonLockOn());
  settings->unlockButtonLock();
  CHECK_EQUAL(false, settings->isButtonLockOn());
}

TEST(settings, testUnlockButtonLockSetsButtonLockToFalseWhenFalse)
{
   //default is button lock = off
  CHECK_EQUAL(false, settings->isButtonLockOn());
  settings->unlockButtonLock();
  CHECK_EQUAL(false, settings->isButtonLockOn());
}

TEST(settings, testSetIsButtonLockOn)
{
  settings->setIsButtonLockOn(true);
  CHECK_TRUE(settings->isButtonLockOn());
  settings->setIsButtonLockOn(false);
  CHECK_FALSE(settings->isButtonLockOn());

  // Test that it doesn't matter if you're setting the lock from true to true.
  settings->toggleButtonLock();
  CHECK_TRUE(settings->isButtonLockOn());
  settings->setIsButtonLockOn(true);
  CHECK_TRUE(settings->isButtonLockOn());
}

TEST(settings, testToggleButtonSetsButtonLockToTrueAfterItWasUnlockedByUnlockFunction)
{
   //default is button lock = off
  CHECK_EQUAL(false, settings->isButtonLockOn());
  settings->unlockButtonLock();
  CHECK_EQUAL(false, settings->isButtonLockOn());
  settings->toggleButtonLock();
  CHECK_EQUAL(true, settings->isButtonLockOn());
}

TEST(settings, testSetAndGetTemperatureSetpoint)
{
  settings->setPrimaryHeatingMode(HeatingMode::comfort); // We need to set this, because the rest of the test is dependant on a heatingmode.
  settings->setSetPoint(22000);
  CHECK_EQUAL(22000, settings->getSetPoint());
}

TEST(settings, testSetTimeFromDateTimeThenGet)
{
  DateTime dateTime =
    { 16, /* year */
      2, /* month */
      28, /* day */
      23, /* hour */
      59, /* minutes */
      59, /* seconds */
      WeekDay::MONDAY /* incorrect on purpose */
    };

  mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime")
      .withParameterOfType("DateTime", "dateTime", &dateTime)
      .andReturnValue(1456703999U);

  mock().expectOneCall("SystemTimeDriverMock::setUnixTime").withUnsignedIntParameter("unixTime", 1456703999U);
  mock().expectOneCall("RtcMock::setCurrentTime").withParameterOfType("DateTime", "dateTime", &dateTime);
  settings->setTimeFromDateTime(dateTime);

  timer.incrementTime(1000);  // Avoid caching of time

  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(1456704999U);
  mock().expectOneCall("SystemTimeDriverMock::getDateTimeFromUnixTime").ignoreOtherParameters();
  settings->getUnixTime(dateTime);

  //Rtc is mocked, so there's no point in checking whether the values from getter is correct.
}

TEST(settings, testSetTimeFromDateTimeActuallySetsTimeAtLimitedIntervals)
{
  DateTime dateTime = { 18, 6, 22, 1, 2, 3, WeekDay::FRIDAY };

  mock().ignoreOtherCalls();
  mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime").withParameterOfType("DateTime", "dateTime", &dateTime).andReturnValue(1U);
  mock().expectOneCall("SystemTimeDriverMock::setUnixTime").withUnsignedIntParameter("unixTime", 1U);
  mock().expectOneCall("RtcMock::setCurrentTime").withParameterOfType("DateTime", "dateTime", &dateTime);
  settings->setTimeFromDateTime(dateTime); // initial set

  timer.incrementTime(Constants::DEFAULT_SET_TIME_LIMITATION_MS - 1000U);
  settings->setTimeFromDateTime(dateTime); // not set due to too little time elapsed

  timer.incrementTime(1000U);
  dateTime.hours += 1U;
  mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime").withParameterOfType("DateTime", "dateTime", &dateTime).andReturnValue(2U);
  mock().expectOneCall("SystemTimeDriverMock::setUnixTime").withUnsignedIntParameter("unixTime", 2U);
  mock().expectOneCall("RtcMock::setCurrentTime").withParameterOfType("DateTime", "dateTime", &dateTime);
  settings->setTimeFromDateTime(dateTime); // set again

  // Try yet again to verify that variable keeping track of 'last time set' was updated correctly
  timer.incrementTime(Constants::DEFAULT_SET_TIME_LIMITATION_MS - 1000U);
  settings->setTimeFromDateTime(dateTime);

  timer.incrementTime(1000U);
  dateTime.hours += 1U;
  mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime").withParameterOfType("DateTime", "dateTime", &dateTime).andReturnValue(3U);
  mock().expectOneCall("SystemTimeDriverMock::setUnixTime").withUnsignedIntParameter("unixTime", 3U);
  mock().expectOneCall("RtcMock::setCurrentTime").withParameterOfType("DateTime", "dateTime", &dateTime);
  settings->setTimeFromDateTime(dateTime);
}


TEST(settings, testSetTimeFromUnixTimeThenGet)
{
  uint32_t unixTime = 1526300654U; // https://www.unixtimestamp.com/ => 05/14/2018 @ 12:24:14pm (UTC)
  DateTime dateTime = { 18, 5, 14, 12, 24, 14, WeekDay::MONDAY };

  mock().expectOneCall("SystemTimeDriverMock::setUnixTime").withUnsignedIntParameter("unixTime", unixTime);
  mock().expectOneCall("SystemTimeDriverMock::getDateTimeFromUnixTime")
      .withUnsignedIntParameter("unixTime", unixTime)
      .withOutputParameterOfTypeReturning("DateTime", "dateTime", &dateTime);

  mock().expectOneCall("RtcMock::setCurrentTime").withParameterOfType("DateTime", "dateTime", &dateTime);
  settings->setTimeFromUnixTime(unixTime);

  timer.incrementTime(1000);  // Avoid caching of time

  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(1526301654U);
  mock().expectOneCall("SystemTimeDriverMock::getDateTimeFromUnixTime").ignoreOtherParameters();
  settings->getUnixTime(dateTime);
}

TEST(settings, testSetTimeFromUnixTimeActuallySetsTimeAtLimitedIntervals)
{
  uint32_t unixTime = 1526300654U;
  uint32_t secondUnixTime = unixTime + Constants::DEFAULT_SET_TIME_LIMITATION_MS;
  uint32_t thirdUnixTime = secondUnixTime + Constants::DEFAULT_SET_TIME_LIMITATION_MS;

  mock().expectOneCall("SystemTimeDriverMock::setUnixTime").withUnsignedIntParameter("unixTime", unixTime);
  mock().expectOneCall("SystemTimeDriverMock::setUnixTime").withUnsignedIntParameter("unixTime", secondUnixTime);
  mock().expectOneCall("SystemTimeDriverMock::setUnixTime").withUnsignedIntParameter("unixTime", thirdUnixTime);
  mock().ignoreOtherCalls();

  settings->setTimeFromUnixTime(unixTime); // initial set

  timer.incrementTime(Constants::DEFAULT_SET_TIME_LIMITATION_MS - 1000U);
  settings->setTimeFromUnixTime(unixTime); // not set due to too little time elapsed

  timer.incrementTime(1000U);
  settings->setTimeFromUnixTime(secondUnixTime); // set again

  // Try yet again to verify that variable keeping track of 'last time set' was updated correctly
  timer.incrementTime(Constants::DEFAULT_SET_TIME_LIMITATION_MS - 1000U);
  settings->setTimeFromUnixTime(unixTime);

  timer.incrementTime(1000U);
  settings->setTimeFromUnixTime(thirdUnixTime);
}

TEST(settings, testTimeIsCached)
{
  DateTime dateTime = Constants::DEFAULT_DATETIME;

  mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime").ignoreOtherParameters().andReturnValue(1U);
  mock().expectOneCall("SystemTimeDriverMock::setUnixTime").ignoreOtherParameters();
  mock().expectOneCall("RtcMock::setCurrentTime").ignoreOtherParameters();
  settings->setTimeFromDateTime(dateTime);
  timer.incrementTime(1000);  // Avoid caching of time

  mock().expectOneCall("SystemTimeDriverMock::getUnixTime").andReturnValue(1U);
  mock().expectOneCall("SystemTimeDriverMock::getDateTimeFromUnixTime").ignoreOtherParameters();
  settings->getUnixTime(dateTime);
  timer.incrementTime(100);
  settings->getUnixTime(dateTime);
}


TEST(settings, testSetAndGetUtcOffset)
{
  settings->setUtcOffset(10000);
  CHECK_EQUAL(10000, settings->getUtcOffset());
  settings->setUtcOffset(-10000);
  CHECK_EQUAL(-10000, settings->getUtcOffset());
}
TEST(settings, testSetAndGetWeekProgramDataOnMonday)
{
  WeekProgramData weekProgramInput, weekProgramOutput;
  weekProgramInput.setData({ 1U, 2U, 3U, 4U, 5U, 5U, 4U, 3U, 2U, 1U, 1U, 2U, 3U, 4U, 5U, 5U, 4U, 3U, 2U, 1U });

  settings->setWeekProgramDataForDay(WeekDay::MONDAY, weekProgramInput);
  settings->getWeekProgramDataForDay(WeekDay::MONDAY, weekProgramOutput);
  CHECK_TRUE(weekProgramInput == weekProgramOutput);
}

TEST(settings, testSetAndGetWeekProgramDataOnWednesday)
{
  WeekProgramData weekProgramInput, weekProgramOutput;
  weekProgramInput.setData({ 5U, 4U, 3U, 2U, 1U, 1U, 2U, 3U, 4U, 5U, 5U, 4U, 3U, 2U, 1U, 1U, 2U, 3U, 4U, 5U });

  settings->setWeekProgramDataForDay(WeekDay::WEDNESDAY, weekProgramInput);
  settings->getWeekProgramDataForDay(WeekDay::WEDNESDAY, weekProgramOutput);
  CHECK_TRUE(weekProgramInput == weekProgramOutput);
}

TEST(settings, testSetAndGetWeekProgramDataOnSunday)
{
  WeekProgramData weekProgramInput, weekProgramOutput;
  weekProgramInput.setData({ 2U, 1U, 1U, 2U, 3U, 4U, 5U, 5U, 4U, 3U, 2U, 1U, 1U, 2U, 3U, 4U, 5U, 5U, 4U, 3U });

  settings->setWeekProgramDataForDay(WeekDay::SUNDAY, weekProgramInput);
  settings->getWeekProgramDataForDay(WeekDay::SUNDAY, weekProgramOutput);
  CHECK_TRUE(weekProgramInput == weekProgramOutput);
}

TEST(settings, testSettingWeekProgramDataDoesntAffectOtherDays)
{
  WeekProgramData weekProgramInputOrig, weekProgramInputNew, weekProgramOutput;
  weekProgramInputOrig.setData({ 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U });
  weekProgramInputNew.setData({ 2U, 2U,2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U });
  settings->setWeekProgramDataForDay(WeekDay::MONDAY, weekProgramInputOrig);
  settings->setWeekProgramDataForDay(WeekDay::TUESDAY, weekProgramInputOrig);
  settings->setWeekProgramDataForDay(WeekDay::WEDNESDAY, weekProgramInputOrig);

  settings->setWeekProgramDataForDay(WeekDay::TUESDAY, weekProgramInputNew);

  settings->getWeekProgramDataForDay(WeekDay::MONDAY, weekProgramOutput);
  CHECK_TRUE(weekProgramInputOrig == weekProgramOutput);

  settings->getWeekProgramDataForDay(WeekDay::WEDNESDAY, weekProgramOutput);
  CHECK_TRUE(weekProgramInputOrig == weekProgramOutput);

  settings->getWeekProgramDataForDay(WeekDay::TUESDAY, weekProgramOutput);
  CHECK_TRUE(weekProgramInputNew == weekProgramOutput);
}

TEST(settings, testSetAndGetWeekProgramExtDataOnMonday)
{
  WeekProgramExtData weekProgramInput, weekProgramOutput;
  uint8_t weekProgramInputData[] = { 1U, 2U, 3U, 4U, 5U, 5U, 4U, 3U, 2U, 1U, 1U, 2U, 3U, 4U, 5U, 5U, 4U, 3U, 2U, 1U };
  weekProgramInput.setData(weekProgramInputData, sizeof(weekProgramInputData));

  settings->setWeekProgramExtDataForDay(WeekDay::MONDAY, weekProgramInput);
  settings->getWeekProgramExtDataForDay(WeekDay::MONDAY, weekProgramOutput);
  CHECK_TRUE(weekProgramInput == weekProgramOutput);
}

TEST(settings, testSetAndGetWeekProgramExtData8Nodes)
{
  WeekProgramExtData weekProgramInput, weekProgramOutput;
  uint8_t weekProgramInputData[] = {
      1U, 2U, 3U, 4U, CloudTimerHeatingMode::away,
      5U, 4U, 3U, 2U, CloudTimerHeatingMode::comfort,
      1U, 2U, 3U, 4U, CloudTimerHeatingMode::eco,
      5U, 4U, 3U, 2U, CloudTimerHeatingMode::comfort,
      1U, 2U, 3U, 4U, CloudTimerHeatingMode::eco,
      5U, 4U, 3U, 2U, CloudTimerHeatingMode::comfort,
      1U, 2U, 3U, 4U, CloudTimerHeatingMode::eco,
      5U, 4U, 3U, 2U, CloudTimerHeatingMode::comfort
  };
  weekProgramInput.setData(weekProgramInputData, sizeof(weekProgramInputData));

  settings->setWeekProgramExtDataForDay(WeekDay::MONDAY, weekProgramInput);
  settings->getWeekProgramExtDataForDay(WeekDay::MONDAY, weekProgramOutput);
  CHECK_TRUE(weekProgramInput == weekProgramOutput);
  CHECK_EQUAL(8u, weekProgramOutput.getNumberOfSlots());
  CHECK_EQUAL(sizeof(weekProgramInputData), weekProgramOutput.getCurrentSize());
}

TEST(settings, testSetAndGetWeekProgramExtData12Nodes)
{
  WeekProgramExtData weekProgramInput, weekProgramOutput;
  uint8_t weekProgramInputData[] = {
      1U, 2U, 3U, 4U, CloudTimerHeatingMode::away,
      5U, 4U, 3U, 2U, CloudTimerHeatingMode::comfort,
      1U, 2U, 3U, 4U, CloudTimerHeatingMode::eco,
      5U, 4U, 3U, 2U, CloudTimerHeatingMode::comfort,
      1U, 2U, 3U, 4U, CloudTimerHeatingMode::eco,
      5U, 4U, 3U, 2U, CloudTimerHeatingMode::comfort,
      1U, 2U, 3U, 4U, CloudTimerHeatingMode::eco,
      5U, 4U, 3U, 2U, CloudTimerHeatingMode::comfort,
      1U, 2U, 3U, 4U, CloudTimerHeatingMode::eco,
      5U, 4U, 3U, 2U, CloudTimerHeatingMode::comfort,
      1U, 2U, 3U, 4U, CloudTimerHeatingMode::eco,
      5U, 4U, 3U, 2U, CloudTimerHeatingMode::comfort
  };
  weekProgramInput.setData(weekProgramInputData, sizeof(weekProgramInputData));

  settings->setWeekProgramExtDataForDay(WeekDay::MONDAY, weekProgramInput);
  settings->getWeekProgramExtDataForDay(WeekDay::MONDAY, weekProgramOutput);
  CHECK_TRUE(weekProgramInput == weekProgramOutput);
  CHECK_EQUAL(12u, weekProgramOutput.getNumberOfSlots());
  CHECK_EQUAL(sizeof(weekProgramInputData), weekProgramOutput.getCurrentSize());
}

IGNORE_TEST(settings, testSetAndGetWeekProgramExtData13NodesDiscardsLastNode)
{
  WeekProgramExtData weekProgramInput, weekProgramOutput;
  uint8_t weekProgramInputData[] = {
      1U, 2U, 3U, 4U, CloudTimerHeatingMode::away,
      5U, 4U, 3U, 2U, CloudTimerHeatingMode::comfort,
      1U, 2U, 3U, 4U, CloudTimerHeatingMode::eco,
      5U, 4U, 3U, 2U, CloudTimerHeatingMode::comfort,
      1U, 2U, 3U, 4U, CloudTimerHeatingMode::eco,
      5U, 4U, 3U, 2U, CloudTimerHeatingMode::comfort,
      1U, 2U, 3U, 4U, CloudTimerHeatingMode::eco,
      5U, 4U, 3U, 2U, CloudTimerHeatingMode::comfort,
      1U, 2U, 3U, 4U, CloudTimerHeatingMode::eco,
      5U, 4U, 3U, 2U, CloudTimerHeatingMode::comfort,
      1U, 2U, 3U, 4U, CloudTimerHeatingMode::eco,
      5U, 4U, 3U, 2U, CloudTimerHeatingMode::comfort,
      1U, 2U, 3U, 4U, CloudTimerHeatingMode::eco
  };
  weekProgramInput.setData(weekProgramInputData, sizeof(weekProgramInputData));

  settings->setWeekProgramExtDataForDay(WeekDay::MONDAY, weekProgramInput);
  settings->getWeekProgramExtDataForDay(WeekDay::MONDAY, weekProgramOutput);
  CHECK_TRUE(weekProgramInput == weekProgramOutput);
  CHECK_EQUAL(12u, weekProgramOutput.getNumberOfSlots());
  CHECK_EQUAL((sizeof(weekProgramInputData)-Constants::WEEKPROGRAM_SLOT_SIZE), weekProgramOutput.getCurrentSize());
}

TEST(settings, testSetAndGetWeekProgramExtDataOnWednesday)
{
  WeekProgramExtData weekProgramInput, weekProgramOutput;
  uint8_t weekProgramInputData[] = { 5U, 4U, 3U, 2U, 1U, 1U, 2U, 3U, 4U, 5U, 5U, 4U, 3U, 2U, 1U, 1U, 2U, 3U, 4U, 5U };
  weekProgramInput.setData(weekProgramInputData, sizeof(weekProgramInputData));

  settings->setWeekProgramExtDataForDay(WeekDay::WEDNESDAY, weekProgramInput);
  settings->getWeekProgramExtDataForDay(WeekDay::WEDNESDAY, weekProgramOutput);
  CHECK_TRUE(weekProgramInput == weekProgramOutput);
}

TEST(settings, testSetAndGetWeekProgramExtDataOnSunday)
{
  WeekProgramExtData weekProgramInput, weekProgramOutput;
  uint8_t weekProgramInputData[] = { 2U, 1U, 1U, 2U, 3U, 4U, 5U, 5U, 4U, 3U, 2U, 1U, 1U, 2U, 3U, 4U, 5U, 5U, 4U, 3U };
  weekProgramInput.setData(weekProgramInputData, sizeof(weekProgramInputData));

  settings->setWeekProgramExtDataForDay(WeekDay::SUNDAY, weekProgramInput);
  settings->getWeekProgramExtDataForDay(WeekDay::SUNDAY, weekProgramOutput);
  CHECK_TRUE(weekProgramInput == weekProgramOutput);
}

TEST(settings, testSettingWeekProgramExtDataDoesntAffectOtherDays)
{
  WeekProgramExtData weekProgramInputOrig, weekProgramInputNew, weekProgramOutput;
  uint8_t weekProgramInputOrigData[] = { 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U };
  uint8_t weekProgramInputNewData[] = { 2U, 2U,2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U };
  weekProgramInputOrig.setData(weekProgramInputOrigData, sizeof(weekProgramInputOrigData));
  weekProgramInputNew.setData(weekProgramInputNewData, sizeof(weekProgramInputNewData));
  settings->setWeekProgramExtDataForDay(WeekDay::MONDAY, weekProgramInputOrig);
  settings->setWeekProgramExtDataForDay(WeekDay::TUESDAY, weekProgramInputOrig);
  settings->setWeekProgramExtDataForDay(WeekDay::WEDNESDAY, weekProgramInputOrig);

  settings->setWeekProgramExtDataForDay(WeekDay::TUESDAY, weekProgramInputNew);

  settings->getWeekProgramExtDataForDay(WeekDay::MONDAY, weekProgramOutput);
  CHECK_TRUE(weekProgramInputOrig == weekProgramOutput);

  settings->getWeekProgramExtDataForDay(WeekDay::WEDNESDAY, weekProgramOutput);
  CHECK_TRUE(weekProgramInputOrig == weekProgramOutput);

  settings->getWeekProgramExtDataForDay(WeekDay::TUESDAY, weekProgramOutput);
  CHECK_TRUE(weekProgramInputNew == weekProgramOutput);
}

TEST(settings, testResettingWeekProgramExtDataMakesItInvalidAndDoesntAffectOtherDays)
{
  WeekProgramExtData weekProgramInputOrig, weekProgramOutput;
  uint8_t weekProgramInputOrigData[] = { 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U };
  weekProgramInputOrig.setData(weekProgramInputOrigData, sizeof(weekProgramInputOrigData));

  settings->setWeekProgramExtDataForDay(WeekDay::MONDAY, weekProgramInputOrig);
  settings->setWeekProgramExtDataForDay(WeekDay::TUESDAY, weekProgramInputOrig);
  settings->setWeekProgramExtDataForDay(WeekDay::WEDNESDAY, weekProgramInputOrig);

  settings->resetWeekProgramExtDataForDay(WeekDay::TUESDAY);

  settings->getWeekProgramExtDataForDay(WeekDay::MONDAY, weekProgramOutput);
  CHECK_TRUE(weekProgramInputOrig == weekProgramOutput);
  CHECK_TRUE(weekProgramOutput.isValid());

  settings->getWeekProgramExtDataForDay(WeekDay::WEDNESDAY, weekProgramOutput);
  CHECK_TRUE(weekProgramInputOrig == weekProgramOutput);
  CHECK_TRUE(weekProgramOutput.isValid());

  settings->getWeekProgramExtDataForDay(WeekDay::TUESDAY, weekProgramOutput);
  CHECK_FALSE(weekProgramOutput.isValid());
}

TEST(settings, testGetAndSetFwUpgradeAttempts)
{
  settings->setFirmwareUpgradeCount(3U);
  CHECK_EQUAL(3U, settings->getFirmwareUpgradeCount());
  settings->setFirmwareUpgradeCount(0U);
  CHECK_EQUAL(0U, settings->getFirmwareUpgradeCount());
}

TEST(settings, testGetAndSetFwVersionCode)
{
  settings->setFirmwareVersionCode(23526U);
  CHECK_EQUAL(23526U, settings->getFirmwareVersionCode());
  settings->setFirmwareVersionCode(0U);
  CHECK_EQUAL(0U, settings->getFirmwareVersionCode());
}

TEST(settings, testGetAndSetFirmwareID)
{
  char theID[Constants::FW_DOWNLOAD_ID_MAX_LENGTH] = {};

  settings->setFirmwareID("fw-01.02.03.bin");
  settings->getFirmwareID(theID, sizeof(theID));
  STRNCMP_EQUAL("fw-01.02.03.bin", theID, sizeof(theID));

  settings->setFirmwareID("fw-04.04.04.bin");
  settings->getFirmwareID(theID, sizeof(theID));
  STRNCMP_EQUAL("fw-04.04.04.bin", theID, sizeof(theID));

  settings->setFirmwareID("");
  settings->getFirmwareID(theID, sizeof(theID));
  STRNCMP_EQUAL("", theID, sizeof(theID));
}

TEST(settings, testGetFirmwareIDWithTooShortBufferProvided)
{
  char theID[2U];

  settings->setFirmwareID("myWifi");
  settings->getFirmwareID(theID, sizeof(theID));
  STRNCMP_EQUAL("m", theID, sizeof(theID));
  CHECK_EQUAL(1U, strnlen(theID, sizeof(theID)));
}

TEST(settings, testSetFirmwareIDLongerThanSupported)
{
  char theID[Constants::FW_DOWNLOAD_ID_MAX_LENGTH + 2U];

  for (uint8_t i = 0; i < sizeof(theID); i++)
  {
    theID[i] = 'p';
  }

  settings->setFirmwareID(theID);
  settings->getFirmwareID(theID, sizeof(theID));

  CHECK_EQUAL(Constants::FW_DOWNLOAD_ID_MAX_LENGTH - 1U, strnlen(theID, sizeof(theID)));
  CHECK_EQUAL('p', theID[0U]);
  CHECK_EQUAL('p', theID[Constants::FW_DOWNLOAD_ID_MAX_LENGTH - 2U]);
}

TEST(settings, testObjectIsNotModifiedByDefault)
{
  CHECK_FALSE(settings->isModified());
}

TEST(settings, testUsingGettersShouldNotTriggerModified)
{
  settings->getComfortTemperatureSetPoint();
  settings->getEcoTemperatureSetPoint();
  settings->isButtonLockOn();
  settings->getRunMode();
  settings->getFirmwareUpgradeCount();
  CHECK_FALSE(settings->isModified());
}

TEST(settings, testUsingSettersShouldTriggerModified)
{
  settings->setComfortTemperatureSetPoint(25000);
  CHECK_TRUE(settings->isModified());
}

TEST(settings, testUsingSettersForStringsShouldTriggerModified)
{
  settings->setWifiSSID("ssid");
  CHECK_TRUE(settings->isModified());
}


TEST(settings, testStoringSettingsSavesToPersistentStorage)
{
  WeekProgramData weekProgramInput;
  weekProgramInput.setData({ 5U, 4U, 3U, 2U, 1U, 1U, 2U, 3U, 4U, 5U, 5U, 4U, 3U, 2U, 1U, 1U, 2U, 3U, 4U, 5U });

  WeekProgramExtData weekProgramExtInput;
  uint8_t weekProgramExtInputData[] = { 5U, 4U, 3U, 2U, 1U, 1U, 2U, 3U, 4U, 5U, 5U, 4U, 3U, 2U, 1U, 1U, 2U, 3U, 4U, 5U,
                                        5U, 4U, 3U, 2U, 1U, 1U, 2U, 3U, 4U, 5U };
  weekProgramExtInput.setData(weekProgramExtInputData, sizeof(weekProgramExtInputData));

  settings->setRunMode(RunMode::normal);
  settings->setWifiPassword("password");
  settings->setFirmwareUpgradeCount(3U);
  settings->setFirmwareID("fwID");
  settings->setComfortTemperatureSetPoint(19000);
  settings->setWeekProgramDataForDay(WeekDay::THURSDAY, weekProgramInput);
  settings->setWeekProgramExtDataForDay(WeekDay::WEDNESDAY, weekProgramExtInput);
  settings->storeSettings();

  RunMode::Enum runMode = RunMode::test;
  CHECK_TRUE(storage->retrieveRunMode(runMode));
  CHECK_EQUAL(settings->getRunMode(), runMode);

  char storedPassword[Constants::WIFI_PASSWORD_SIZE_MAX] = {};
  CHECK_TRUE(storage->retrieveWifiPassword(storedPassword, sizeof(storedPassword)));
  STRCMP_EQUAL("password", storedPassword);

  char storedFwID[Constants::FW_DOWNLOAD_ID_MAX_LENGTH] = {};
  CHECK_TRUE(storage->retrieveFirmwareID(storedFwID, sizeof(storedFwID)));
  STRCMP_EQUAL("fwID", storedFwID);

  uint8_t attempts = 0;
  CHECK_TRUE(storage->retrieveFirmwareUpgradeAttempts(attempts));
  CHECK_EQUAL(3U, attempts);

  int32_t storedSetPoint = 0;
  CHECK_TRUE(storage->retrieveComfortTemperatureSetPoint(storedSetPoint));
  CHECK_EQUAL(19000, storedSetPoint);

  uint8_t storedWeekProgramData[20] = {};
  CHECK_TRUE(storage->retrieveWeekProgramDataForDay(WeekDay::THURSDAY, storedWeekProgramData));
  WeekProgramData weekProgramOutput;
  weekProgramOutput.setData(storedWeekProgramData);
  CHECK_TRUE(weekProgramInput == weekProgramOutput);

  uint8_t storedWeekProgramExtData[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY_EXT_MAX] = { };
  uint32_t extWeekProgramDataSize = sizeof(storedWeekProgramExtData);
  storage->retrieveWeekProgramExtDataForDay(WeekDay::WEDNESDAY, storedWeekProgramExtData, extWeekProgramDataSize);
  CHECK_EQUAL(extWeekProgramDataSize, sizeof(weekProgramExtInputData));
  CHECK_EQUAL(0, memcmp(weekProgramExtInputData, storedWeekProgramExtData, extWeekProgramDataSize));
}


TEST(settings, testRetrievingSettingsLoadsFromPersistentStorage)
{
  uint8_t weekProgramDataBytes[] = { 5U, 4U, 3U, 2U, 1U, 1U, 2U, 3U, 4U, 5U, 5U, 4U, 3U, 2U, 1U, 1U, 2U, 3U, 4U, 5U };

  storage->storeRunMode(RunMode::normal);
  storage->storeEcoTemperatureSetPoint(17000);
  storage->storeWifiSSID("ssidString");
  storage->storeWeekProgramDataForDay(WeekDay::SATURDAY, weekProgramDataBytes);
  storage->storeWeekProgramExtDataForDay(WeekDay::FRIDAY, weekProgramDataBytes, sizeof(weekProgramDataBytes));
  storage->storeFirmwareVersionCode(234126U);
  static_assert(RunMode::normal != Constants::DEFAULT_RUN_MODE, "Use different testing runMode");
  static_assert(17000 != Constants::DEFAULT_ECO_TEMPERATURE_SET_POINT, "Use different testing ecoSetpoint");
  CHECK_TRUE(strlen("ssidString") != strlen(Constants::DEFAULT_WIFI_SSID));

  settings->retrieveSettings();

  char retrievedSSID[Constants::WIFI_SSID_SIZE_MAX];
  settings->getWifiSSID(retrievedSSID, sizeof(retrievedSSID));

  WeekProgramData expectedWeekProgram;
  expectedWeekProgram.setData(weekProgramDataBytes);

  WeekProgramExtData expectedWeekProgramExt;
  expectedWeekProgramExt.setData(weekProgramDataBytes, sizeof(weekProgramDataBytes));

  WeekProgramData retrievedWeekProgram;
  settings->getWeekProgramDataForDay(WeekDay::SATURDAY, retrievedWeekProgram);

  WeekProgramExtData retrievedWeekProgramExt;
  settings->getWeekProgramExtDataForDay(WeekDay::FRIDAY, retrievedWeekProgramExt);

  CHECK_EQUAL(RunMode::normal, settings->getRunMode());
  CHECK_EQUAL(17000, settings->getEcoTemperatureSetPoint());
  CHECK_EQUAL(234126U, settings->getFirmwareVersionCode());
  STRCMP_EQUAL("ssidString", retrievedSSID);
  CHECK_TRUE(expectedWeekProgram == retrievedWeekProgram);
  CHECK_TRUE(expectedWeekProgramExt == retrievedWeekProgramExt);
}
