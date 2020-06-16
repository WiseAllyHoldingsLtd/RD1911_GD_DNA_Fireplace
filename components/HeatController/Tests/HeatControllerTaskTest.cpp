#include "assert.h"
#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include "RUMock.h"
#include "RtcMock.h"
#include "TimerDriverMock.h"
#include "SystemTimeDriverMock.h"
#include "SettingsMock.h"
#include "QueueMock.hpp"
#include "TWIDriverMock.h"
#include "SoftwareResetDriverMock.h"
#include "HeatControllerTask.hpp"
#include "DateTime.h"


static RUMock ruMock;
static TimerDriverMock timerMock;
static SystemTimeDriverMock sysTimeMock;
static SettingsMock settingsMock;
static QueueMock<bool> heatElementStateQueue;
static TWIMock twiMock;
static HeatControllerTask *heatController;
static SoftwareResetDriverMock swResetMock;


TEST_GROUP(heatController)
{
  TEST_SETUP()
  {
    mock().installComparator("RURegulator", ruRegulatorComparator);
    mock().installCopier("DateTime", dateTimeCopier);
    mock().installComparator("DateTime", dateTimeComparator);
    mock().installCopier("Override", overrideCopier);
    mock().installComparator("Override", overrideComparator);

    heatController = new HeatControllerTask(ruMock, timerMock, sysTimeMock, settingsMock, heatElementStateQueue, twiMock, swResetMock);
  }

  TEST_TEARDOWN()
  {
    delete heatController;

    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
  }
};


TEST(heatController, testThatRUIsUpdatedWithNewSetPointWhenChanged)
{
  RURegulator ruMode = RURegulator::REGULATOR_SETPOINT;

  mock().expectOneCall("getSetPoint").andReturnValue(21000);
  mock().expectOneCall("RUMock::updateConfig")
      .withParameterOfType("RURegulator", "regulator", &ruMode)
      .withBoolParameter("useInternalTemp", true)
      .withBoolParameter("enableRUPot", false)
      .withIntParameter("newSetpointFP", 21000)
      .andReturnValue(true);

  mock().ignoreOtherCalls();

  heatController->run(false);
}

TEST(heatController, testThatRUIsUpdatedWithNewSetPointWhenChangedAgain)
{
  RURegulator ruMode = RURegulator::REGULATOR_SETPOINT;

  mock().strictOrder();
  mock().expectOneCall("getSetPoint").andReturnValue(21000);
  mock().expectOneCall("RUMock::updateConfig")
      .withParameterOfType("RURegulator", "regulator", &ruMode)
      .withBoolParameter("useInternalTemp", true)
      .withBoolParameter("enableRUPot", false)
      .withIntParameter("newSetpointFP", 21000)
      .andReturnValue(true);

  mock().expectOneCall("getSetPoint").andReturnValue(20000);
  mock().expectOneCall("RUMock::updateConfig")
      .withParameterOfType("RURegulator", "regulator", &ruMode)
      .withBoolParameter("useInternalTemp", true)
      .withBoolParameter("enableRUPot", false)
      .withIntParameter("newSetpointFP", 20000)
      .andReturnValue(true);

  mock().ignoreOtherCalls();

  heatController->run(false);
  heatController->run(false);
}

TEST(heatController, testThatRUIsNotUpdatedIfSetPointIsUnchanged)
{
  RURegulator ruMode = RURegulator::REGULATOR_SETPOINT;

  mock().expectOneCall("getSetPoint").andReturnValue(21000);
  mock().expectOneCall("RUMock::updateConfig").ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("getSetPoint").andReturnValue(21000);
  mock().ignoreOtherCalls();

  heatController->run(false);
  heatController->run(false);
}

TEST(heatController, testThatRUIsUpdatedWithRegulatorOffWhenSetPointIsZero)
{
  RURegulator ruMode = RURegulator::REGULATOR_OFF;

  // Ensure REGULATOR_SETPOINT first
  mock().expectOneCall("getSetPoint").andReturnValue(21000);
  mock().expectOneCall("RUMock::updateConfig").ignoreOtherParameters().andReturnValue(true);

  // Force RU off
  mock().expectOneCall("getSetPoint").andReturnValue(Constants::HEATER_OFF_SET_POINT);
  mock().expectOneCall("RUMock::updateConfig")
      .withParameterOfType("RURegulator", "regulator", &ruMode)
      .withBoolParameter("useInternalTemp", true)
      .withBoolParameter("enableRUPot", false)
      .withIntParameter("newSetpointFP", Constants::HEATER_OFF_SET_POINT)
      .andReturnValue(true);

  mock().ignoreOtherCalls();

  heatController->run(false);
  heatController->run(false);
}

TEST(heatController, testThatTwiDriverIsReinitiatedIfRUCommFailsConsecutivelyTooManyTimes)
{
  uint8_t twiReinitLimit = Constants::NUM_OF_RU_TIMEOUT_RETRIES_BEFORE_REINIT;

  for (uint8_t i = 0U; i < twiReinitLimit - 1U; ++i)
  {
    mock().expectOneCall("getSetPoint").andReturnValue(21000);
    mock().expectOneCall("RUMock::updateConfig").ignoreOtherParameters().andReturnValue(false);
    mock().ignoreOtherCalls();
    heatController->run(false);
  }

  mock().expectOneCall("getSetPoint").andReturnValue(21000);
  mock().expectOneCall("RUMock::updateConfig").ignoreOtherParameters().andReturnValue(false);
  mock().expectOneCall("TWIMock::setup").withUnsignedIntParameter("busFrequency", Constants::TWI_BUS_MAX_FREQUENCY);
  mock().ignoreOtherCalls();
  heatController->run(false);
}

TEST(heatController, testThatSWResetIsPerformedIfRUCommFailsConsecutivelyEvenMoreTimes)
{
  uint8_t restartLimit = Constants::NUM_OF_RU_TIMEOUT_RETRIES_BEFORE_RESTART;

  for (uint8_t i = 0U; i < restartLimit - 1U; ++i)
  {
    mock().expectOneCall("getSetPoint").andReturnValue(21000);
    mock().expectOneCall("RUMock::updateConfig").ignoreOtherParameters().andReturnValue(false);
    mock().ignoreOtherCalls();
    heatController->run(false);
  }

  mock().expectOneCall("getSetPoint").andReturnValue(21000);
  mock().expectOneCall("RUMock::updateConfig").ignoreOtherParameters().andReturnValue(false);
  mock().expectOneCall("SoftwareResetDriverMock::reset");
  mock().ignoreOtherCalls();
  heatController->run(false);
}

TEST(heatController, testThatTwiDriverIsNotReinitiatedIfRUCommFailsConsecutivelyManyTimesThenRecovers)
{
  uint8_t twiReinitLimit = Constants::NUM_OF_RU_TIMEOUT_RETRIES_BEFORE_REINIT;

  for (uint8_t i = 0U; i < twiReinitLimit - 1U; ++i)
  {
    mock().expectOneCall("getSetPoint").andReturnValue(21000);
    mock().expectOneCall("RUMock::updateConfig").ignoreOtherParameters().andReturnValue(false);
    mock().ignoreOtherCalls();
    heatController->run(false);
  }

  mock().expectOneCall("getSetPoint").andReturnValue(21000);
  mock().expectOneCall("RUMock::updateConfig").ignoreOtherParameters().andReturnValue(true);
  mock().expectNoCall("TWIMock::setup");
  mock().ignoreOtherCalls();
  heatController->run(false);
}

TEST(heatController, testThatSWResetIsNotPerformedIfRUCommFailsConsecutivelyEvenAfterTwiReinitButThenRecovers)
{
  uint8_t restartLimit = Constants::NUM_OF_RU_TIMEOUT_RETRIES_BEFORE_RESTART;

  for (uint8_t i = 0U; i < restartLimit - 1U; ++i)
  {
    mock().expectOneCall("getSetPoint").andReturnValue(21000);
    mock().expectOneCall("RUMock::updateConfig").ignoreOtherParameters().andReturnValue(false);
    mock().ignoreOtherCalls();
    heatController->run(false);
  }

  mock().expectOneCall("getSetPoint").andReturnValue(21000);
  mock().expectOneCall("RUMock::updateConfig").ignoreOtherParameters().andReturnValue(true);
  mock().expectNoCall("SoftwareResetDriverMock::reset");
  mock().ignoreOtherCalls();
  heatController->run(false);
}

TEST(heatController, testThatRUHeatElementStatusIsReadAndReportedEveryOneSec)
{
  mock().expectNCalls(2, "getTimeSinceTimestampMS").withUnsignedLongIntParameter("timestamp", 0U).andReturnValue(999U);
  mock().expectNoCall("RUMock::isHeatElementActive");
  mock().expectNoCall("QueueMock::push");
  mock().ignoreOtherCalls();
  heatController->run(false);

  mock().expectNCalls(2, "getTimeSinceTimestampMS").withUnsignedLongIntParameter("timestamp", 0U).andReturnValue(1000U);
  mock().expectOneCall("RUMock::isHeatElementActive").andReturnValue(false);
  mock().expectOneCall("QueueMock::push").ignoreOtherParameters();
  mock().expectOneCall("getTimeSinceBootMS").andReturnValue(1001U);
  mock().ignoreOtherCalls();
  heatController->run(false);

  mock().expectOneCall("getTimeSinceTimestampMS").withUnsignedLongIntParameter("timestamp", 0U).andReturnValue(2000U); // resendConfig-check
  mock().expectOneCall("getTimeSinceTimestampMS").withUnsignedLongIntParameter("timestamp", 1001U).andReturnValue(2000U); // checkElementStatus-check
  mock().expectOneCall("RUMock::isHeatElementActive").andReturnValue(false);
  mock().expectOneCall("QueueMock::push").ignoreOtherParameters();
  mock().expectOneCall("getTimeSinceBootMS").andReturnValue(2001U);
  mock().ignoreOtherCalls();
  heatController->run(false);
}

TEST(heatController, testThatCorrectHeatElementStatusIsReportedWhenOff)
{
  mock().expectNCalls(2, "getTimeSinceTimestampMS").withUnsignedLongIntParameter("timestamp", 0U).andReturnValue(1000U);
  mock().expectOneCall("RUMock::isHeatElementActive").andReturnValue(false);
  mock().expectOneCall("QueueMock::push").withBoolParameter("item", false).withUnsignedIntParameter("timeout", 0);
  mock().ignoreOtherCalls();
  heatController->run(false);
}

TEST(heatController, testThatCorrectHeatElementStatusIsReportedWhenOn)
{
  mock().expectNCalls(2, "getTimeSinceTimestampMS").withUnsignedLongIntParameter("timestamp", 0U).andReturnValue(1000U);
  mock().expectOneCall("RUMock::isHeatElementActive").andReturnValue(true);
  mock().expectOneCall("QueueMock::push").withBoolParameter("item", true).withUnsignedIntParameter("timeout", 0);
  mock().ignoreOtherCalls();
  heatController->run(false);
}

TEST(heatController, testThatSetPrimaryHeatingModeIsntCalledIfPrimaryHeatingModeIsUnchanged)
{
  DateTime dateTime = { 0, 0, 0, 0, 0, 0, WeekDay::MONDAY };

  mock().expectOneCall("isWeekProgramUpdated").andReturnValue(false);
  mock().expectOneCall("getLocalTime").withOutputParameterOfTypeReturning("DateTime", "dateTime", &dateTime);
  mock().expectOneCall("getPrimaryHeatingMode").andReturnValue(HeatingMode::comfort); // we should already have Comfort as that is default
  mock().expectNoCall("setPrimaryHeatingMode");
  mock().ignoreOtherCalls();

  heatController->run(false);
}

TEST(heatController, testThatSetPrimaryHeatingModeIsCalledIfPrimaryHeatingModeHasChanged)
{
  DateTime dateTime = { 0, 0, 0, 0, 0, 0, WeekDay::MONDAY };

  mock().expectOneCall("isWeekProgramUpdated").andReturnValue(false);
  mock().expectOneCall("getLocalTime").withOutputParameterOfTypeReturning("DateTime", "dateTime", &dateTime);
  mock().expectOneCall("getPrimaryHeatingMode").andReturnValue(HeatingMode::off);
  mock().expectOneCall("setPrimaryHeatingMode").withIntParameter("heatingMode", HeatingMode::comfort);
  mock().ignoreOtherCalls();

  heatController->run(false);
}

TEST(heatController, testThatWeekProgramConstantComfortIsUsedByDefault)
{
  DateTime dateTime = { 0, 0, 0, 0, 0, 0, WeekDay::MONDAY };

  mock().expectNCalls(7*24, "isWeekProgramUpdated").andReturnValue(false);
  mock().expectNCalls(7*24, "getPrimaryHeatingMode").andReturnValue(HeatingMode::eco);
  mock().expectNCalls(7*24, "setPrimaryHeatingMode").withIntParameter("heatingMode", HeatingMode::comfort);
  mock().ignoreOtherCalls();

  for (uint8_t day = 0; day < 7; ++day)
  {
    dateTime.days = static_cast<WeekDay::Enum>(day);

    for (uint8_t hour = 0; hour < 24; ++hour)
    {
      dateTime.hours = hour;
      mock().expectOneCall("getLocalTime").withOutputParameterOfTypeReturning("DateTime", "dateTime", &dateTime);
      heatController->run(false);
    }
  }
}

TEST(heatController, testThatHeatingModeChangesIsCheckedOnEveryChangeOfMinuteIfWeekProgramHasntChanged)
{
  DateTime dateTime = { 0, 0, 0, 0, 0, 0, WeekDay::MONDAY };

  mock().ignoreOtherCalls();
  mock().expectNCalls(5, "isWeekProgramUpdated").andReturnValue(false);

  mock().expectOneCall("getLocalTime").withOutputParameterOfTypeReturning("DateTime", "dateTime", &dateTime);
  mock().expectOneCall("getPrimaryHeatingMode").andReturnValue(HeatingMode::comfort);
  heatController->run(false);

  dateTime.seconds = 1;
  mock().expectOneCall("getLocalTime").withOutputParameterOfTypeReturning("DateTime", "dateTime", &dateTime);
  heatController->run(false);

  dateTime.seconds = 59;
  mock().expectOneCall("getLocalTime").withOutputParameterOfTypeReturning("DateTime", "dateTime", &dateTime);
  heatController->run(false);

  dateTime.seconds = 0;
  dateTime.minutes = 1;
  mock().expectOneCall("getLocalTime").withOutputParameterOfTypeReturning("DateTime", "dateTime", &dateTime);
  mock().expectOneCall("getPrimaryHeatingMode").andReturnValue(HeatingMode::comfort);
  heatController->run(false);

  dateTime.minutes = 2;
  mock().expectOneCall("getLocalTime").withOutputParameterOfTypeReturning("DateTime", "dateTime", &dateTime);
  mock().expectOneCall("getPrimaryHeatingMode").andReturnValue(HeatingMode::comfort);
  heatController->run(false);
}

TEST(heatController, testThatHeatingModeChangesIsCheckedImmediatelyIfWeekProgramHasChanged)
{
  DateTime dateTime = { 0, 0, 0, 0, 0, 30, WeekDay::MONDAY };

  mock().expectOneCall("isWeekProgramUpdated").andReturnValue(true);
  mock().expectOneCall("getLocalTime").withOutputParameterOfTypeReturning("DateTime", "dateTime", &dateTime);
  mock().expectOneCall("getPrimaryHeatingMode").andReturnValue(HeatingMode::off);
  mock().expectOneCall("setPrimaryHeatingMode").withIntParameter("heatingMode", HeatingMode::comfort);
  mock().ignoreOtherCalls();

  heatController->run(false);
}


TEST(heatController, testThatNowOverrideIsResetAt_3_00)
{
  DateTime dateTime = { 0, 0, 0, 3, 0, 0, WeekDay::WEDNESDAY };
  Override nowOverride(OverrideType::now, HeatingMode::eco);
  Override disableOverride(OverrideType::none, HeatingMode::comfort);

  mock().expectOneCall("isWeekProgramUpdated").andReturnValue(false);
  mock().expectOneCall("getLocalTime").withOutputParameterOfTypeReturning("DateTime", "dateTime", &dateTime);
  mock().expectOneCall("getOverride").withOutputParameterOfTypeReturning("Override", "override", &nowOverride);
  mock().expectOneCall("setOverride").withParameterOfType("Override", "override", &disableOverride);
  mock().expectOneCall("getPrimaryHeatingMode").andReturnValue(HeatingMode::comfort);
  mock().ignoreOtherCalls();

  heatController->run(false);
}

TEST(heatController, testThatNowOverrideIsResetAt_3_01)
{
  DateTime dateTime = { 0, 0, 0, 3, 1, 0, WeekDay::WEDNESDAY };
  Override nowOverride(OverrideType::now, HeatingMode::antiFrost);
  Override disableOverride(OverrideType::none, HeatingMode::comfort);

  mock().expectOneCall("isWeekProgramUpdated").andReturnValue(false);
  mock().expectOneCall("getLocalTime").withOutputParameterOfTypeReturning("DateTime", "dateTime", &dateTime);
  mock().expectOneCall("getOverride").withOutputParameterOfTypeReturning("Override", "override", &nowOverride);
  mock().expectOneCall("setOverride").withParameterOfType("Override", "override", &disableOverride);
  mock().expectOneCall("getPrimaryHeatingMode").andReturnValue(HeatingMode::comfort);
  mock().ignoreOtherCalls();

  heatController->run(false);
}

TEST(heatController, testThatConstantOverrideIsNotReset)
{
  DateTime dateTime = { 0, 0, 0, 3, 0, 0, WeekDay::WEDNESDAY };
  Override constantOverride(OverrideType::constant, HeatingMode::eco);

  mock().expectOneCall("isWeekProgramUpdated").andReturnValue(false);
  mock().expectOneCall("getLocalTime").withOutputParameterOfTypeReturning("DateTime", "dateTime", &dateTime);
  mock().expectOneCall("getOverride").withOutputParameterOfTypeReturning("Override", "override", &constantOverride);
  mock().expectNoCall("setOverride");
  mock().expectOneCall("getPrimaryHeatingMode").andReturnValue(HeatingMode::comfort);
  mock().ignoreOtherCalls();

  heatController->run(false);
}

TEST(heatController, testThatTimedOverrideIsNotResetIfNotExpired)
{
  DateTime now = { 18, 7, 6, 9, 50, 0, WeekDay::FRIDAY };
  DateTime overrideEnd = { 18, 7, 6, 9, 51, 0, WeekDay::FRIDAY };

  Override timedOverride(OverrideType::timed, HeatingMode::eco, overrideEnd);

  mock().expectOneCall("isWeekProgramUpdated").andReturnValue(false);
  mock().expectOneCall("getLocalTime").withOutputParameterOfTypeReturning("DateTime", "dateTime", &now);
  mock().expectOneCall("getOverride").withOutputParameterOfTypeReturning("Override", "override", &timedOverride);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime").withParameterOfType("DateTime", "dateTime", &overrideEnd).andReturnValue(2000U);
  mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime").withParameterOfType("DateTime", "dateTime", &now).andReturnValue(1000U);
  mock().expectNoCall("setOverride");
  mock().expectOneCall("getPrimaryHeatingMode").andReturnValue(HeatingMode::comfort);
  mock().ignoreOtherCalls();

  heatController->run(false);
}

TEST(heatController, testThatOverrideIsNotResetAtOtherTimes)
{
  mock().ignoreOtherCalls();
  mock().expectNCalls(4, "isWeekProgramUpdated").andReturnValue(false);
  mock().expectNoCall("setOverride");

  Override nowOverride(OverrideType::now, HeatingMode::antiFrost);
  DateTime dateTime = { 0, 0, 0, 2, 59, 58, WeekDay::WEDNESDAY };
  mock().expectOneCall("getLocalTime").withOutputParameterOfTypeReturning("DateTime", "dateTime", &dateTime);
  mock().expectOneCall("getOverride").withOutputParameterOfTypeReturning("Override", "override", &nowOverride);
  mock().expectOneCall("getPrimaryHeatingMode").andReturnValue(HeatingMode::comfort);
  heatController->run(false);

  dateTime = { 0, 0, 0, 2, 59, 59, WeekDay::WEDNESDAY };
  mock().expectOneCall("getLocalTime").withOutputParameterOfTypeReturning("DateTime", "dateTime", &dateTime);
  heatController->run(false);

  dateTime = { 0, 0, 0, 3, 0, 1, WeekDay::WEDNESDAY };
  mock().expectOneCall("getLocalTime").withOutputParameterOfTypeReturning("DateTime", "dateTime", &dateTime);
  heatController->run(false);

  dateTime = { 0, 0, 0, 3, 1, 1, WeekDay::WEDNESDAY };
  mock().expectOneCall("getLocalTime").withOutputParameterOfTypeReturning("DateTime", "dateTime", &dateTime);
  heatController->run(false);
}

TEST(heatController, configResentEvery5MinutesSecsEvenIfSetpointIsUnchanged)
{
  mock().ignoreOtherCalls();

  mock().expectOneCall("getTimeSinceTimestampMS").withUnsignedLongIntParameter("timestamp", 0U).andReturnValue(299999U);
  mock().expectOneCall("getTimeSinceTimestampMS").withUnsignedLongIntParameter("timestamp", 0U).andReturnValue(0U); // dummy call for heatElementOnCheck
  heatController->run(false);

  mock().ignoreOtherCalls();
  mock().expectOneCall("getTimeSinceTimestampMS").withUnsignedLongIntParameter("timestamp", 0U).andReturnValue(300000U);
  mock().expectOneCall("RUMock::updateConfig").ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("getTimeSinceBootMS").andReturnValue(300000U);
  mock().expectOneCall("getTimeSinceTimestampMS").withUnsignedLongIntParameter("timestamp", 0U).andReturnValue(0U); // dummy call for heatElementOnCheck
  heatController->run(false);

  mock().expectOneCall("getTimeSinceTimestampMS").withUnsignedLongIntParameter("timestamp", 300000U).andReturnValue(299999U);
  mock().expectOneCall("getTimeSinceTimestampMS").withUnsignedLongIntParameter("timestamp", 0U).andReturnValue(0U); // dummy call for heatElementOnCheck
  heatController->run(false);

  mock().expectOneCall("getTimeSinceTimestampMS").withUnsignedLongIntParameter("timestamp", 300000U).andReturnValue(300000U);
  mock().expectOneCall("RUMock::updateConfig").ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("getTimeSinceBootMS").andReturnValue(600000U);
  mock().expectOneCall("getTimeSinceTimestampMS").withUnsignedLongIntParameter("timestamp", 0U).andReturnValue(0U); // dummy call for heatElementOnCheck
  heatController->run(false);
}
