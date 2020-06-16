

#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include "RebootScheduler.h"

#include "SystemTimeDriverMock.h"
#include "SoftwareResetDriverMock.h"
#include "RtcMock.h"

namespace
{
  RebootScheduler *rebootScheduler = nullptr;
  SoftwareResetDriverMock *softwareResetDriver = nullptr;
  SystemTimeDriverMock *systemTimeDriver = nullptr;

  void commonTestSetup()
  {
    systemTimeDriver = new SystemTimeDriverMock();
    softwareResetDriver = new SoftwareResetDriverMock();

    rebootScheduler = new RebootScheduler(*softwareResetDriver,
                                          *systemTimeDriver);

    mock().installComparator("DateTime", dateTimeComparator);
  }

  void commonTestTearDown()
  {
    delete rebootScheduler;
    rebootScheduler = nullptr;

    delete softwareResetDriver;
    delete systemTimeDriver;

    softwareResetDriver = nullptr;
    systemTimeDriver = nullptr;

    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
  }

  bool dateTimesEqual(const DateTime &dt1, const DateTime &dt2)
  {
    return dateTimeComparator.isEqual(&dt1, &dt2);
  }

  void runResetAndInitAndVerifyCalculatedRebootTime(
      const uint8_t (&gdid)[Constants::GDID_BCD_SIZE],
      uint32_t bootTimeUnix,
      uint32_t firstCandidateRebootTimeUnix,
      uint32_t preferredRebootTimeSpanLengthS,
      uint32_t maxUpTimeS,
      uint32_t expectedRebootTime)
  {
    // These two are not fully tested here
    const DateTime bootTimeDummy = Constants::DEFAULT_DATETIME;
    const DateTime preferredRebootTimeSpanStartDummy = Constants::DEFAULT_DATETIME;

    mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime")
        .ignoreOtherParameters()
        .andReturnValue(bootTimeUnix);
    mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime")
        .ignoreOtherParameters()
        .andReturnValue(firstCandidateRebootTimeUnix);
    mock().expectOneCall("SystemTimeDriverMock::getDateTimeFromUnixTime")
        .ignoreOtherParameters(); // only for pretty-printing next reboot time to the log
    bool initOk = rebootScheduler->resetAndInit(gdid,
                                                bootTimeDummy,
                                                preferredRebootTimeSpanStartDummy,
                                                preferredRebootTimeSpanLengthS,
                                                maxUpTimeS);
    CHECK_TRUE_TEXT(initOk, "Failed to init reboot scheduler");

    uint32_t nextRebootTime = rebootScheduler->getNextRebootTimeUnixLocal();

    CHECK_EQUAL_TEXT(expectedRebootTime, nextRebootTime,
        "Calculated reboot time doesn't match expected");
  }

}

//////////////////////////////////////////////////////////////////////////////

TEST_GROUP(rebootScheduler)
{
  TEST_SETUP()
  {
    commonTestSetup();
  }

  TEST_TEARDOWN()
  {
    commonTestTearDown();
  }
};

TEST(rebootScheduler, findsLatestPossibleRebootTimeWithLongMaxUptime)
{

  const uint8_t gdid[] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
  const uint32_t preferredRebootTimeSpanLengthS = 1u;
  const uint32_t maxUpTimeS = 60u*60u*24u*7u;
  const uint32_t bootTimeUnix = 500u;
  const uint32_t firstCandidateRebootTimeUnix = 1000u;
  const uint32_t expectedRebootTime = 1000u+60u*60u*24u*6u;

  runResetAndInitAndVerifyCalculatedRebootTime(
      gdid,
      bootTimeUnix,
      firstCandidateRebootTimeUnix,
      preferredRebootTimeSpanLengthS,
      maxUpTimeS,
      expectedRebootTime);
}

TEST(rebootScheduler, findsLatestPossibleRebootTimeWith2DaysMaxUptime)
{

  const uint8_t gdid[] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
  const uint32_t preferredRebootTimeSpanLengthS = 1u;
  const uint32_t maxUpTimeS = 60u*60u*24u*2u;
  const uint32_t bootTimeUnix = 500u;
  const uint32_t firstCandidateRebootTimeUnix = 1000u;
  const uint32_t expectedRebootTime = 1000u+60u*60u*24u*1u;

  runResetAndInitAndVerifyCalculatedRebootTime(
      gdid,
      bootTimeUnix,
      firstCandidateRebootTimeUnix,
      preferredRebootTimeSpanLengthS,
      maxUpTimeS,
      expectedRebootTime);
}

TEST(rebootScheduler, bootingOneSecondAfterRebootTimeSpanWith2DaysMaxUptime)
{

  const uint8_t gdid[] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
  const uint32_t preferredRebootTimeSpanLengthS = 1u;
  const uint32_t maxUpTimeS = 60u*60u*24u*2u;
  const uint32_t bootTimeUnix = 1001u;
  const uint32_t firstCandidateRebootTimeUnix = 1000u;
  const uint32_t expectedRebootTime = 1000u+60u*60u*24u*2u;

  runResetAndInitAndVerifyCalculatedRebootTime(
      gdid,
      bootTimeUnix,
      firstCandidateRebootTimeUnix,
      preferredRebootTimeSpanLengthS,
      maxUpTimeS,
      expectedRebootTime);
}

TEST(rebootScheduler, bootingOneSecondIntoPreferredRebootTimeSpan)
{
  const uint8_t gdid[] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
  const uint32_t preferredRebootTimeSpanLengthS = 1u;
  const uint32_t maxUpTimeS = 60u*60u*24u;
  const uint32_t bootTimeUnix = 1001u;
  const uint32_t firstCandidateRebootTimeUnix = 1000u;
  const uint32_t expectedRebootTime = 1000u+60u*60u*24u;

  runResetAndInitAndVerifyCalculatedRebootTime(
      gdid,
      bootTimeUnix,
      firstCandidateRebootTimeUnix,
      preferredRebootTimeSpanLengthS,
      maxUpTimeS,
      expectedRebootTime);
}

TEST(rebootScheduler, bootingAtStartOfPreferredRebootTimeSpan)
{
  const uint8_t gdid[] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
  const uint32_t preferredRebootTimeSpanLengthS = 1u;
  const uint32_t maxUpTimeS = 60u*60u*24u;
  const uint32_t bootTimeUnix = 1000u;
  const uint32_t firstCandidateRebootTimeUnix = 1000u;
  const uint32_t expectedRebootTime = 1000u+60u*60u*24u;

  runResetAndInitAndVerifyCalculatedRebootTime(
      gdid,
      bootTimeUnix,
      firstCandidateRebootTimeUnix,
      preferredRebootTimeSpanLengthS,
      maxUpTimeS,
      expectedRebootTime);
}

TEST(rebootScheduler, gdidDerivedUniqueRebootTime1)
{
  const uint8_t gdid[] = { 0x55u, 0xeeu, 0xbbu, 0xccu, 0xaau, 0xffu };
  const uint32_t preferredRebootTimeSpanLengthS = 1u;
  const uint32_t maxUpTimeS = 60u*60u*24u;
  const uint32_t bootTimeUnix = 1000u;
  const uint32_t firstCandidateRebootTimeUnix = 1000u;
  const uint32_t expectedRebootTime = 1000u+60u*60u*24u;

  runResetAndInitAndVerifyCalculatedRebootTime(
      gdid,
      bootTimeUnix,
      firstCandidateRebootTimeUnix,
      preferredRebootTimeSpanLengthS,
      maxUpTimeS,
      expectedRebootTime);
}

TEST(rebootScheduler, gdidDerivedUniqueRebootTime2)
{
  const uint8_t gdid[] = { 0x0u, 0x1u, 0x0u, 0x0u, 0x0u, 0x0u };
  const uint32_t preferredRebootTimeSpanLengthS = 60u*60u*2u;
  const uint32_t maxUpTimeS = 60u*60u*24u;
  const uint32_t bootTimeUnix = 1000u;
  const uint32_t firstCandidateRebootTimeUnix = 1000u;
  const uint32_t expectedRebootTime = 1000u+60u*60u*24u + (0x100u*199u) % (60u*60u*2u);

  runResetAndInitAndVerifyCalculatedRebootTime(
      gdid,
      bootTimeUnix,
      firstCandidateRebootTimeUnix,
      preferredRebootTimeSpanLengthS,
      maxUpTimeS,
      expectedRebootTime);
}

TEST(rebootScheduler, gdidDerivedUniqueRebootTime3)
{
  const uint8_t gdid[] = { 0x0u, 0x1u, 0x1u, 0x0u, 0x0u, 0x0u };
  const uint32_t preferredRebootTimeSpanLengthS = 60u*60u*2u;
  const uint32_t maxUpTimeS = 60u*60u*24u;
  const uint32_t bootTimeUnix = 1000u;
  const uint32_t firstCandidateRebootTimeUnix = 1000u;
  const uint32_t expectedRebootTime = 1000u+60u*60u*24u + (0x100u*199u+13u) % (60u*60u*2u);

  runResetAndInitAndVerifyCalculatedRebootTime(
      gdid,
      bootTimeUnix,
      firstCandidateRebootTimeUnix,
      preferredRebootTimeSpanLengthS,
      maxUpTimeS,
      expectedRebootTime);
}

TEST(rebootScheduler, gdidDerivedUniqueRebootTime6hourTimeSpan1)
{
  const uint8_t gdid[] = { 0x1u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u };
  const uint32_t preferredRebootTimeSpanLengthS = 60u*60u*6u;
  const uint32_t maxUpTimeS = 60u*60u*24u;
  const uint32_t bootTimeUnix = 1000u;
  const uint32_t firstCandidateRebootTimeUnix = 1000u;
  const uint32_t expectedRebootTime = 1000u+60u*60u*24u + (199u%preferredRebootTimeSpanLengthS);

  runResetAndInitAndVerifyCalculatedRebootTime(
      gdid,
      bootTimeUnix,
      firstCandidateRebootTimeUnix,
      preferredRebootTimeSpanLengthS,
      maxUpTimeS,
      expectedRebootTime);
}

TEST(rebootScheduler, gdidDerivedUniqueRebootTime6hourTimeSpan2)
{
  const uint8_t gdid[] = { 0x3u, 0x1u, 0x0u, 0x0u, 0x0u, 0x0u };
  const uint32_t preferredRebootTimeSpanLengthS = 60u*60u*6u;
  const uint32_t maxUpTimeS = 60u*60u*24u;
  const uint32_t bootTimeUnix = 1000u;
  const uint32_t firstCandidateRebootTimeUnix = 1000u;
  const uint32_t expectedRebootTime = 1000u+60u*60u*24u + ((259u*199u)%preferredRebootTimeSpanLengthS);

  runResetAndInitAndVerifyCalculatedRebootTime(
      gdid,
      bootTimeUnix,
      firstCandidateRebootTimeUnix,
      preferredRebootTimeSpanLengthS,
      maxUpTimeS,
      expectedRebootTime);
}

TEST(rebootScheduler, gdidDerivedUniqueRebootTime6hourTimeSpan3)
{
  const uint8_t gdid[] = { 0x3u, 0x1u, 0x2u, 0x3u, 0x0u, 0x0u };
  const uint32_t preferredRebootTimeSpanLengthS = 60u*60u*6u;
  const uint32_t maxUpTimeS = 60u*60u*24u;
  const uint32_t bootTimeUnix = 1000u;
  const uint32_t firstCandidateRebootTimeUnix = 1000u;
  const uint32_t expectedRebootTime = 1000u+60u*60u*24u + 18351u;//((259u*199u+770u*13u)%preferredRebootTimeSpanLengthS);

  runResetAndInitAndVerifyCalculatedRebootTime(
      gdid,
      bootTimeUnix,
      firstCandidateRebootTimeUnix,
      preferredRebootTimeSpanLengthS,
      maxUpTimeS,
      expectedRebootTime);
}

TEST(rebootScheduler, gdidDerivedUniqueRebootTime6hourTimeSpan4)
{
  const uint8_t gdid[] = { 0xffu, 0xffu, 0xffu, 0xff, 0x0, 0x0 };
  const uint32_t preferredRebootTimeSpanLengthS = 60u*60u*6u;
  const uint32_t maxUpTimeS = 60u*60u*24u;
  const uint32_t bootTimeUnix = 1000u;
  const uint32_t firstCandidateRebootTimeUnix = 1000u;
  const uint32_t expectedRebootTime = 1000u+60u*60u*24u + (0xffffu*13u+0xffffu*199u) % (60u*60u*6u);

  runResetAndInitAndVerifyCalculatedRebootTime(
      gdid,
      bootTimeUnix,
      firstCandidateRebootTimeUnix,
      preferredRebootTimeSpanLengthS,
      maxUpTimeS,
      expectedRebootTime);
}

TEST(rebootScheduler, gdidDerivedUniqueRebootTime6hourTimeSpan5)
{
  const uint8_t gdid[] = { 0x00u, 0x03u, 0x1au, 0x4c, 0x090, 0x02 };
  const uint32_t preferredRebootTimeSpanLengthS = 60u*60u*6u;
  const uint32_t maxUpTimeS = 60u*60u*24u;
  const uint32_t bootTimeUnix = 1000u;
  const uint32_t firstCandidateRebootTimeUnix = 1000u;
  const uint32_t expectedRebootTime = 1000u+60u*60u*24u + 17298u;//(0x4c1au*13u+0x300u*199u) % (60u*60u*6u);

  runResetAndInitAndVerifyCalculatedRebootTime(
      gdid,
      bootTimeUnix,
      firstCandidateRebootTimeUnix,
      preferredRebootTimeSpanLengthS,
      maxUpTimeS,
      expectedRebootTime);
}

TEST(rebootScheduler, gdidDerivedUniqueRebootTimeLastTwoBytesDoesntMatter)
{
  const uint8_t gdid[] = { 0xffu, 0xffu, 0xffu, 0xff, 0xaf, 0x3df };
  const uint32_t preferredRebootTimeSpanLengthS = 60u*60u*6u;
  const uint32_t maxUpTimeS = 60u*60u*24u;
  const uint32_t bootTimeUnix = 1000u;
  const uint32_t firstCandidateRebootTimeUnix = 1000u;
  const uint32_t expectedRebootTime = 1000u+60u*60u*24u + (0xffffu*13u+0xffffu*199u) % (60u*60u*6u);
  // same offset as gdidDerivedUniqueRebootTime6hourTimeSpan4 above although gdid is different

  runResetAndInitAndVerifyCalculatedRebootTime(
      gdid,
      bootTimeUnix,
      firstCandidateRebootTimeUnix,
      preferredRebootTimeSpanLengthS,
      maxUpTimeS,
      expectedRebootTime);
}

//////////////////////////////////////////////////////////////////////////////

TEST_GROUP(shouldReboot)
{
  TEST_SETUP()
  {
    commonTestSetup();

    const uint8_t gdid[] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
    const DateTime bootTime = {
        .year = 0u, .month = 1u, .days = 0u,
        .hours = 15, .minutes = 0u, .seconds = 0u,
        .weekDay = WeekDay::MONDAY
    };

    const DateTime preferredRebootTimeSpanStart = {
        .year = 0u, .month = 1u, .days = 0u,
        .hours = 23, .minutes = 30u, .seconds = 0u,
        .weekDay = WeekDay::MONDAY
    };

    const uint32_t preferredRebootTimeSpanLengthS = 1u;
    const uint32_t maxUpTimeS = 60u*60u*24u;

    const DateTime expectedCandidate = {
        .year = bootTime.year, .month = bootTime.month, .days = bootTime.days,
        .hours = preferredRebootTimeSpanStart.hours,
        .minutes = preferredRebootTimeSpanStart.minutes,
        .seconds = preferredRebootTimeSpanStart.seconds,
        .weekDay = WeekDay::MONDAY
    };

    mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime")
        .withParameterOfType("DateTime", "dateTime", static_cast<const void *>(&bootTime))
        .andReturnValue(1000u);
    mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime")
        .withParameterOfType("DateTime", "dateTime", static_cast<const void *>(&expectedCandidate))
        .andReturnValue(1000u+60u*60u*8u+60u*30u);
    mock().expectOneCall("SystemTimeDriverMock::getDateTimeFromUnixTime")
        .ignoreOtherParameters(); // only for pretty-printing next reboot time to the log
    bool initOk = rebootScheduler->resetAndInit(gdid,
                                                bootTime,
                                                preferredRebootTimeSpanStart,
                                                preferredRebootTimeSpanLengthS,
                                                maxUpTimeS);
    CHECK_TRUE_TEXT(initOk, "Failed to init reboot scheduler");


    const uint32_t expectedRebootTime = 1000u+60u*60u*8u+60u*30u;

    uint32_t nextRebootTime = rebootScheduler->getNextRebootTimeUnixLocal();

    CHECK_EQUAL_TEXT(nextRebootTime, expectedRebootTime,
        "Calculated reboot time doesn't match expected");
  }

  TEST_TEARDOWN()
  {
    commonTestTearDown();
  }

};

TEST(shouldReboot, noTooEarly)
{
  const DateTime currentLocalTime = Constants::DEFAULT_DATETIME;
  mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime")
      .withParameterOfType("DateTime", "dateTime", static_cast<const void *>(&currentLocalTime))
      .andReturnValue(1000u+60u*60u*8u+60u*29u+59u);
  bool shouldReboot = rebootScheduler->shouldReboot(currentLocalTime);
  CHECK_FALSE_TEXT(shouldReboot, "Wanted to reboot one second too early");
}

TEST(shouldReboot, yesRightOnTime)
{
  const DateTime currentLocalTime = Constants::DEFAULT_DATETIME;
  mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime")
      .withParameterOfType("DateTime", "dateTime", static_cast<const void *>(&currentLocalTime))
      .andReturnValue(1000u+60u*60u*8u+60u*30u);
  bool shouldReboot = rebootScheduler->shouldReboot(currentLocalTime);
  CHECK_TRUE_TEXT(shouldReboot, "No reboot although current time equals next reboot time");
}

TEST(shouldReboot, yesPassedRebootTime)
{
  const DateTime currentLocalTime = Constants::DEFAULT_DATETIME;
  mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime")
      .withParameterOfType("DateTime", "dateTime", static_cast<const void *>(&currentLocalTime))
      .andReturnValue(1000u+60u*60u*8u+60u*30u+1u);
  bool shouldReboot = rebootScheduler->shouldReboot(currentLocalTime);
  CHECK_TRUE_TEXT(shouldReboot, "No reboot although current time has passed reboot time");
}

//////////////////////////////////////////////////////////////////////////////

