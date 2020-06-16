#include "CppUTest\TestHarness.h"
#include "CppUTestExt\MockSupport.h"

#include "WeekProgram.h"
#include "WeekProgramNodes.h"
#include "Constants.h"
#include "SettingsMock.h"
#include "WeekProgramExtDataCopier.h"

static WeekProgramNode dummyNode;
static SettingsMock settings;
static WeekProgramNodes* nodes;

static WeekProgramExtDataCopier weekProgramExtDataCopier;

// Creating a week program for test
static uint8_t weekdaysData[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY] = {
    CloudTimerHeatingMode::eco, 0, 0, 6, 0,
    CloudTimerHeatingMode::comfort, 6, 0, 7, 30,
    CloudTimerHeatingMode::eco, 7, 30, 15, 0,
    CloudTimerHeatingMode::comfort, 15, 0, 0, 0
};

static uint8_t saturdayData[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY] = {
    CloudTimerHeatingMode::eco, 0, 0, 8, 0,
    CloudTimerHeatingMode::comfort, 8, 0, 20, 0,
    CloudTimerHeatingMode::eco, 20, 0, 0, 0,
    Constants::CLOUD_PARAMETER_INVALID_VALUE, 0, 0, 0, 0
};

static uint8_t sundayData[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY] = {
    CloudTimerHeatingMode::shutdown, 0, 0, 7, 0,
    CloudTimerHeatingMode::away, 7, 0, 17, 0,
    CloudTimerHeatingMode::comfort, 17, 0, 21, 0,
    CloudTimerHeatingMode::eco, 21, 0, 0, 0
};


TEST_GROUP(WeekProgram)
{
  WeekProgramData weekdays, saturday, sunday;
  WeekProgramExtData invalidExtData;

  TEST_SETUP()
  {
    nodes = new WeekProgramNodes();
    weekdays.setData(weekdaysData);
    saturday.setData(saturdayData);
    sunday.setData(sundayData);
    mock().installCopier("WeekProgramData", weekProgramDataCopier);
    mock().installCopier("WeekProgramExtData", weekProgramExtDataCopier);
  }

  TEST_TEARDOWN()
  {
    delete nodes;
    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
  }
};

TEST(WeekProgram, test0NodesAtInit)
{
  WeekProgram wp(*nodes);
  // default program is all invalid slots => 0
  CHECK_EQUAL(0, nodes->getNumOfNodes());
  CHECK_TRUE(wp.shouldUpdateASAP());
}

static void setupExpectedCallsForOldWeekProgramTests(WeekProgramData &weekdays,
                                                     WeekProgramData &saturday,
                                                     WeekProgramData &sunday,
                                                     WeekProgramExtData &invalidExtData)
{
  mock().expectOneCall("isWeekProgramUpdated").andReturnValue(true);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::MONDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::MONDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &weekdays);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::TUESDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::TUESDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &weekdays);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::WEDNESDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::WEDNESDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &weekdays);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::THURSDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::THURSDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &weekdays);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::FRIDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::FRIDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &weekdays);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::SATURDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::SATURDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &saturday);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::SUNDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::SUNDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &sunday);

  nodes->updateNodes(settings);
}

TEST(WeekProgram, testWeekProgramReturnsEcoMondayAt_00_00)
{
  setupExpectedCallsForOldWeekProgramTests(weekdays, saturday, sunday, invalidExtData);

  WeekProgram wp(*nodes);
  DateTime dt = { 0, 0, 0, 0, 0, 0, WeekDay::MONDAY };

  bool outputBool;
  HeatingMode::Enum heatMode = wp.getCurrentStatus(dt, 0u, outputBool);

  CHECK_EQUAL(27, nodes->getNumOfNodes());
  CHECK_EQUAL(HeatingMode::eco, heatMode);
  CHECK_TRUE(wp.shouldUpdateASAP());

}

TEST(WeekProgram, testWeekProgramReturnsComfortMondayAt_07_29)
{
  setupExpectedCallsForOldWeekProgramTests(weekdays, saturday, sunday, invalidExtData);

  WeekProgram wp(*nodes);
  DateTime dt = { 0, 0, 0, 7, 29, 0, WeekDay::MONDAY };

  bool outputBool;
  HeatingMode::Enum heatMode = wp.getCurrentStatus(dt, 0u, outputBool);

  CHECK_EQUAL(HeatingMode::comfort, heatMode);
}

TEST(WeekProgram, testWeekProgramReturnsEcoTuesdayAt_07_30_And_12_00)
{
  setupExpectedCallsForOldWeekProgramTests(weekdays, saturday, sunday, invalidExtData);

  bool outputBool;
  WeekProgram wp(*nodes);

  DateTime dt = { 0, 0, 0, 7, 30, 0, WeekDay::TUESDAY };
  HeatingMode::Enum heatMode = wp.getCurrentStatus(dt, 0u, outputBool);
  CHECK_EQUAL(HeatingMode::eco, heatMode);

  dt = { 0, 0, 0, 12, 0, 0, WeekDay::TUESDAY };
  heatMode = wp.getCurrentStatus(dt, 0u, outputBool);
  CHECK_EQUAL(HeatingMode::eco, heatMode);
}

TEST(WeekProgram, testWeekProgramReturnsComfortWednesdayAt_16_00)
{
  setupExpectedCallsForOldWeekProgramTests(weekdays, saturday, sunday, invalidExtData);

  WeekProgram wp(*nodes);
  DateTime dt = { 0, 0, 0, 16, 0, 0, WeekDay::WEDNESDAY };

  bool outputBool;
  HeatingMode::Enum heatMode = wp.getCurrentStatus(dt, 0u, outputBool);

  CHECK_EQUAL(HeatingMode::comfort, heatMode);
}

TEST(WeekProgram, testWeekProgramReturnsComfortThursdayAt_23_59)
{
  setupExpectedCallsForOldWeekProgramTests(weekdays, saturday, sunday, invalidExtData);

  WeekProgram wp(*nodes);
  DateTime dt = { 0, 0, 0, 23, 59, 0, WeekDay::THURSDAY };

  bool outputBool;
  HeatingMode::Enum heatMode = wp.getCurrentStatus(dt, 0u, outputBool);

  CHECK_EQUAL(HeatingMode::comfort, heatMode);
}

TEST(WeekProgram, testWeekProgramReturnsEcoSaturdayAt_20_01)
{
  setupExpectedCallsForOldWeekProgramTests(weekdays, saturday, sunday, invalidExtData);

  WeekProgram wp(*nodes);
  DateTime dt = { 0, 0, 0, 20, 1, 0, WeekDay::SATURDAY };

  bool outputBool;
  HeatingMode::Enum heatMode = wp.getCurrentStatus(dt, 0u, outputBool);

  CHECK_EQUAL(HeatingMode::eco, heatMode);
}

TEST(WeekProgram, testWeekProgramReturnsAntiFrostSundayAt_8_00)
{
  setupExpectedCallsForOldWeekProgramTests(weekdays, saturday, sunday, invalidExtData);

  WeekProgram wp(*nodes);
  DateTime dt = { 0, 0, 0, 8, 0, 0, WeekDay::SUNDAY };

  bool outputBool;
  HeatingMode::Enum heatMode = wp.getCurrentStatus(dt, 0u, outputBool);

  CHECK_EQUAL(HeatingMode::antiFrost, heatMode);
}

TEST(WeekProgram, testWeekProgramReturnsOffSundayAt_0_00)
{
  setupExpectedCallsForOldWeekProgramTests(weekdays, saturday, sunday, invalidExtData);

  WeekProgram wp(*nodes);
  DateTime dt = { 0, 0, 0, 0, 0, 0, WeekDay::SUNDAY };

  bool outputBool;
  HeatingMode::Enum heatMode = wp.getCurrentStatus(dt, 0u, outputBool);

  CHECK_EQUAL(HeatingMode::off, heatMode);
}

TEST(WeekProgram, testWeekProgramReturnsCorrectStatusAtMidnightEvenIfFirstNodeThatDayStartsLater)
{
  static uint8_t mondayData[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY] = {
      CloudTimerHeatingMode::away, 2, 0, 0, 0,
      Constants::CLOUD_PARAMETER_INVALID_VALUE, 6, 0, 7, 30,
      Constants::CLOUD_PARAMETER_INVALID_VALUE, 7, 30, 15, 0,
      Constants::CLOUD_PARAMETER_INVALID_VALUE, 15, 0, 0, 0
  };

  WeekProgramData monday;
  monday.setData(mondayData);

  mock().expectOneCall("isWeekProgramUpdated").andReturnValue(true);
  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::MONDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::MONDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &monday);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::TUESDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::TUESDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &weekdays);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::WEDNESDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::WEDNESDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &weekdays);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::THURSDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::THURSDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &weekdays);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::FRIDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::FRIDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &weekdays);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::SATURDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::SATURDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &saturday);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::SUNDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::SUNDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &sunday);
  nodes->updateNodes(settings);

  WeekProgram wp(*nodes);
  DateTime sundayNight = { 0, 0, 0, 23, 59, 0, WeekDay::SUNDAY };
  DateTime mondayMorning = { 0, 0, 0, 0, 0, 0, WeekDay::MONDAY };

  bool outputBool;

  // Sunday ends with Eco
  HeatingMode::Enum heatMode = wp.getCurrentStatus(sundayNight, 0u, outputBool);
  CHECK_EQUAL(HeatingMode::eco, heatMode);

  // Monday starts without node at 0:00, so should continue with Eco
  heatMode = wp.getCurrentStatus(mondayMorning, 0u, outputBool);
  CHECK_EQUAL(HeatingMode::eco, heatMode);

  // Monday has Away node starting 2:00
  mondayMorning.hours = 2;
  heatMode = wp.getCurrentStatus(mondayMorning, 0u, outputBool);
  CHECK_EQUAL(HeatingMode::antiFrost, heatMode);
}

