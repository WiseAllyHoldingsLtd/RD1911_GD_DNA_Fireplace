#include "CppUTest\TestHarness.h"
#include "CppUTestExt\MockSupport.h"

#include "WeekProgramNodes.h"
#include "Constants.h"
#include "SettingsMock.h"
#include "WeekProgramExtDataCopier.h"

WeekProgramNode dummyNode;
static SettingsMock settings;
static WeekProgramExtDataCopier weekProgramExtDataCopier;

TEST_GROUP(WeekProgramNodes)
{
  TEST_SETUP()
  {
    mock().installComparator("WeekProgramData", weekProgramDataComparator);
    mock().installCopier("WeekProgramData", weekProgramDataCopier);
    mock().installCopier("WeekProgramExtData", weekProgramExtDataCopier);
  }

  TEST_TEARDOWN()
  {
    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
  }
};

TEST(WeekProgramNodes, testCompareWeekProgramNodes)
{
  WeekProgramNodes nodes;

  WeekProgramNode early;
  WeekProgramNode late;
  early.status = HeatingMode::eco;
  late.status = HeatingMode::eco;
  early.interval = Interval::INTERVAL_MON;
  late.interval = Interval::INTERVAL_MON;
  early.hours = 5;
  late.hours = 19;
  early.minutes = 50;
  late.minutes = 50;

  int8_t compare = nodes.weekProgramNodeCompare(early, late);
  CHECK_EQUAL(-1, compare);
  compare = nodes.weekProgramNodeCompare(late, early);
  CHECK_EQUAL(1, compare);
  compare = nodes.weekProgramNodeCompare(late, late);
  CHECK_EQUAL(0, compare);

  CHECK_TRUE(nodes.weekProgramNodeEqual(late, late));
  CHECK_FALSE(nodes.weekProgramNodeEqual(early, late));
}


TEST(WeekProgramNodes, testWeekProgramNodeEqual)
{
  WeekProgramNodes nodes;

  WeekProgramNode first;
  WeekProgramNode second;
  first.status = HeatingMode::eco;
  second.status = HeatingMode::eco;
  first.interval = Interval::INTERVAL_MON;
  second.interval = Interval::INTERVAL_MON;
  first.hours = 5;
  second.hours = 19;
  first.minutes = 50;
  second.minutes = 50;


  bool isEqual = nodes.weekProgramNodeEqual(first, second);
  CHECK_FALSE(isEqual);
  second.hours = 5;
  isEqual = nodes.weekProgramNodeEqual(first, second);
  CHECK_TRUE(isEqual);
}

TEST(WeekProgramNodes, testgetBestFittingWeekProgramNode)
{
  WeekProgramNodes nodes;

  WeekProgramNode first;
  WeekProgramNode second;

  DateTime dt =
  {
    0, // year
    0, // month
    0, // date
    6, // hours
    15, // minutes
    40, // seconds
    WeekDay::MONDAY // day of the week
  };

  first.interval = Interval::INTERVAL_MON;
  second.interval = Interval::INTERVAL_MON;
  first.hours = 5;
  second.hours = 6;
  first.minutes = 50;
  second.minutes = 50;

  // Time is before second, but after first. Therefore first is returned
  WeekProgramNode node = nodes.getBestFittingWeekProgramNode(first, second, dt);
  CHECK_TRUE(nodes.weekProgramNodeEqual(first, node));

  dt.hours = 2;
  dt.minutes = 49;

  // Time is before first and second (looped around). Therefore second is returned because it is the one that is "right" before the current time.
  node = nodes.getBestFittingWeekProgramNode(first, second, dt);
  CHECK_TRUE(nodes.weekProgramNodeEqual(second, node));
}

TEST(WeekProgramNodes, nodesUpdatedAtInit)
{
  WeekProgramNodes nodes;
  CHECK_EQUAL(0u, nodes.getNumOfNodes()); // num of nodes is 0 default (only invalid nodes)

  mock().expectOneCall("isWeekProgramUpdated").andReturnValue(false);
  nodes.updateNodes(settings);
  CHECK_TRUE(nodes.hasNodesBeenUpdated()); //true the first time
}

TEST(WeekProgramNodes, nodesNotUpdatedNthTimeBecauseSettingsWeekProgramNotUpdated)
{
  WeekProgramNodes nodes;
  CHECK_EQUAL(0u, nodes.getNumOfNodes()); // num of nodes is 0 default (only invalid nodes)

  mock().expectOneCall("isWeekProgramUpdated").andReturnValue(false);
  nodes.updateNodes(settings);
  nodes.hasNodesBeenUpdated(); //true the first time, but we're not really checking this 'cause it's not part of this test
  CHECK_FALSE(nodes.hasNodesBeenUpdated()); //false the nth time
  CHECK_EQUAL(0u, nodes.getNumOfNodes());
}

TEST(WeekProgramNodes, testgetMinTillNextComfortMondayToTuesday)
{
  WeekProgramNodes nodes;

  /*
   * Configure nodes first. Default is one node per day (all comfort), tuesday we want program with three nodes
   * with Eco 0:00, Comf 6:00, Eco 23:00
   */
  uint8_t tuesdayData[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY] = {
      CloudTimerHeatingMode::eco, 0, 0, 6, 0,
      CloudTimerHeatingMode::comfort, 6, 0, 23, 0,
      CloudTimerHeatingMode::eco, 23, 0, 0, 0,
      Constants::CLOUD_PARAMETER_INVALID_VALUE, 0, 0, 0, 0
  };

  WeekProgramData defaultProg;
  defaultProg.setData(Constants::DEFAULT_WEEK_PROGRAM_DATA);

  WeekProgramData tuesdayProg;
  tuesdayProg.setData(tuesdayData);

  WeekProgramExtData invalidExtData;

  mock().expectOneCall("isWeekProgramUpdated").andReturnValue(true);
  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::MONDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::MONDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &defaultProg);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::TUESDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::TUESDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &tuesdayData);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::WEDNESDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::WEDNESDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &defaultProg);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::THURSDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::THURSDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &defaultProg);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::FRIDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::FRIDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &defaultProg);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::SATURDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::SATURDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &defaultProg);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::SUNDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::SUNDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &defaultProg);

  nodes.updateNodes(settings);
  // should be 0 valid nodes for each day, except tuesday which has 3 nodes = 3 + 0
  CHECK_EQUAL(3, nodes.getNumOfNodes());


  DateTime dt = { 0, 0, 0, 16, 30, 0, WeekDay::MONDAY };
    
  // Find min till next comfort first node
  uint16_t minTillNextComfort = nodes.getNumOfMinTillNextComfort(dt, dummyNode);
  // 7.5 hours to reach tuesday, first comfort is at 6:00
  CHECK_EQUAL(13*60 + 30, minTillNextComfort);
}


TEST(WeekProgramNodes, testgetMinTillNextComfortMondayToThursday)
{
  WeekProgramNodes nodes;

  /*
   * Configure nodes first. Monday is all comfort (default), tue,wed,fri-sun is only eco.
   * Thursday is Comfort 06:00-16:00, else Eco
   */

  uint8_t allEcoData[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY] = {
      CloudTimerHeatingMode::eco, 0, 0, 0, 0,
      Constants::CLOUD_PARAMETER_INVALID_VALUE, 0, 0, 6, 0,
      Constants::CLOUD_PARAMETER_INVALID_VALUE, 0, 0, 6, 0,
      Constants::CLOUD_PARAMETER_INVALID_VALUE, 0, 0, 6, 0
  };

  uint8_t thursdayData[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY] = {
      CloudTimerHeatingMode::eco, 0, 0, 6, 0,
      CloudTimerHeatingMode::comfort, 6, 0, 16, 0,
      CloudTimerHeatingMode::eco, 16, 0, 0, 0,
      Constants::CLOUD_PARAMETER_INVALID_VALUE, 0, 0, 0, 0
  };

  WeekProgramData defaultProg;
  defaultProg.setData(Constants::DEFAULT_WEEK_PROGRAM_DATA);

  WeekProgramData ecoProg;
  ecoProg.setData(allEcoData);

  WeekProgramData thursdayProg;
  thursdayProg.setData(thursdayData);

  WeekProgramExtData invalidExtData;

  mock().expectOneCall("isWeekProgramUpdated").andReturnValue(true);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::MONDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::MONDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &defaultProg);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::TUESDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::TUESDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &ecoProg);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::WEDNESDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::WEDNESDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &ecoProg);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::THURSDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::THURSDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &thursdayProg);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::FRIDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::FRIDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &ecoProg);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::SATURDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::SATURDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &ecoProg);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::SUNDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::SUNDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &ecoProg);

  nodes.updateNodes(settings);
  CHECK_EQUAL(8, nodes.getNumOfNodes());


  DateTime dt =
  {
    0, 0, 0,
    16, 30, 0, WeekDay::MONDAY
  };

  // Find min till next comfort first node
  uint16_t minTillNextComfort = nodes.getNumOfMinTillNextComfort(dt, dummyNode);
  // 7.5 hours to reach tuesday, then 24*2 hours to reach thursday , first comfort is at 6:00
  CHECK_EQUAL(7*60 + 30 + 24*2*60 + 6* 60, minTillNextComfort);
}

TEST(WeekProgramNodes, testgetMinTillNextComfortSunday)
{
  WeekProgramNodes nodes;

  /*
   * Configure nodes first. Mon-Sat is all Eco.
   * Sunday is Eco until 15, then Comfort.
   */

  uint8_t allEcoData[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY] = {
      CloudTimerHeatingMode::eco, 0, 0, 0, 0,
      Constants::CLOUD_PARAMETER_INVALID_VALUE, 0, 0, 6, 0,
      Constants::CLOUD_PARAMETER_INVALID_VALUE, 0, 0, 6, 0,
      Constants::CLOUD_PARAMETER_INVALID_VALUE, 0, 0, 6, 0
  };

  uint8_t sundayData[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY] = {
      CloudTimerHeatingMode::eco, 0, 0, 15, 0,
      CloudTimerHeatingMode::comfort, 15, 0, 0, 0,
      Constants::CLOUD_PARAMETER_INVALID_VALUE, 0, 0, 0, 0,
      Constants::CLOUD_PARAMETER_INVALID_VALUE, 0, 0, 0, 0
  };


  WeekProgramData ecoProg;
  ecoProg.setData(allEcoData);

  WeekProgramData sundayProg;
  sundayProg.setData(sundayData);

  WeekProgramExtData invalidExtData;

  mock().expectOneCall("isWeekProgramUpdated").andReturnValue(true);
  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::MONDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::MONDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &ecoProg);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::TUESDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::TUESDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &ecoProg);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::WEDNESDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::WEDNESDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &ecoProg);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::THURSDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::THURSDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &ecoProg);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::FRIDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::FRIDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &ecoProg);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::SATURDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::SATURDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &ecoProg);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::SUNDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::SUNDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &sundayProg);

  nodes.updateNodes(settings);
  CHECK_EQUAL(8, nodes.getNumOfNodes()); // 6*1 + 2


  DateTime dt =
  {
    0, 0, 0,
    9, 0, 0, WeekDay::SUNDAY

  };

  // Find min till next comfort first node
  uint16_t minTillNextComfort = nodes.getNumOfMinTillNextComfort(dt, dummyNode);
  // 6 hours to reach 15:00
  CHECK_EQUAL(6*60, minTillNextComfort);
}

TEST(WeekProgramNodes, extendedWeekProgramsTakesPriorityWhenValid)
{
  WeekProgramNodes nodes;

  /*
   * Configure nodes first. Mon-Sat is all Eco.
   * Sunday is Eco until 15, then Comfort.
   */

  uint8_t allEcoData[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY] = {
      CloudTimerHeatingMode::eco, 0, 0, 0, 0,
      Constants::CLOUD_PARAMETER_INVALID_VALUE, 0, 0, 6, 0,
      Constants::CLOUD_PARAMETER_INVALID_VALUE, 0, 0, 6, 0,
      Constants::CLOUD_PARAMETER_INVALID_VALUE, 0, 0, 6, 0
  };

  uint8_t sundayData[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY] = {
      CloudTimerHeatingMode::eco, 0, 0, 15, 0,
      CloudTimerHeatingMode::comfort, 15, 0, 0, 0,
      Constants::CLOUD_PARAMETER_INVALID_VALUE, 0, 0, 0, 0,
      Constants::CLOUD_PARAMETER_INVALID_VALUE, 0, 0, 0, 0
  };

  uint8_t sundayExtData[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY] = {
      0, 0, 15, 0, CloudTimerHeatingMode::eco,
      16, 0, 0, 0, CloudTimerHeatingMode::comfort
  };

  WeekProgramData ecoProg;
  ecoProg.setData(allEcoData);

  WeekProgramData sundayProg;
  sundayProg.setData(sundayData);

  WeekProgramExtData sundayProgExt;
  sundayProgExt.setData(sundayExtData, sizeof(sundayExtData));

  WeekProgramExtData invalidExtData;

  mock().expectOneCall("isWeekProgramUpdated").andReturnValue(true);
  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::MONDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::MONDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &ecoProg);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::TUESDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::TUESDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &ecoProg);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::WEDNESDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::WEDNESDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &ecoProg);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::THURSDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::THURSDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &ecoProg);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::FRIDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::FRIDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &ecoProg);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::SATURDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &invalidExtData);
  mock().expectOneCall("getWeekProgramDataForDay").withIntParameter("weekDay", WeekDay::SATURDAY).withOutputParameterOfTypeReturning("WeekProgramData", "weekProgramData", &ecoProg);

  mock().expectOneCall("getWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::SUNDAY).withOutputParameterOfTypeReturning("WeekProgramExtData", "weekProgramData", &sundayProgExt);

  nodes.updateNodes(settings);
  CHECK_EQUAL(8, nodes.getNumOfNodes()); // 6*1 + 2


  DateTime dt =
  {
    0, 0, 0,
    9, 0, 0, WeekDay::SUNDAY
  };

  // Find min till next comfort first node
  uint16_t minTillNextComfort = nodes.getNumOfMinTillNextComfort(dt, dummyNode);
  // 7 hours to reach 16:00 in contrast to 6 hours to reach 15:00 as in the 4-node data (see previous test)
  CHECK_EQUAL(7*60, minTillNextComfort);
}

