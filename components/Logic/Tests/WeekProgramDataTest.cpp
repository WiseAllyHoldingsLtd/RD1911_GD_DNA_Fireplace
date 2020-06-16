#include <cstring>
#include "CppUTest\TestHarness.h"
#include "WeekProgramData.h"


TEST_GROUP(weekProgramData)
{
  WeekProgramData * weekProgram;

  TEST_SETUP()
  {
    weekProgram = new WeekProgramData();
  }

  TEST_TEARDOWN()
  {
    delete weekProgram;
  }
};

TEST(weekProgramData, testSetAndGetData)
{
  uint8_t inputValues[20U] = {12, 34, 2345, 35, 567, 34, 2, 4, 234, 234, 0, 76, 12, 92, 21, 54, 2, 123, 86, 34 };
  uint8_t outputValues[20U];

  weekProgram->setData(inputValues);
  weekProgram->getData(outputValues);
  MEMCMP_EQUAL(inputValues, outputValues, sizeof(inputValues));

  inputValues[4U]++;
  weekProgram->setData(inputValues);
  weekProgram->getData(outputValues);
  MEMCMP_EQUAL(inputValues, outputValues, sizeof(inputValues));
}

TEST(weekProgramData, testWeekProgramDataIsInitiallyInvalid)
{
  uint8_t invalidValues[20U];
  memset(invalidValues, 0xEE, sizeof(invalidValues));

  uint8_t initialData[20U] = {};
  weekProgram->getData(initialData);
  MEMCMP_EQUAL(invalidValues, initialData, sizeof(initialData));
}


TEST(weekProgramData, testGetNumberOfSlotsIsFour)
{
  CHECK_EQUAL(4U, weekProgram->getNumberOfSlots());
}

TEST(weekProgramData, testGetWeekProgramNodeFailsWhenDataIsInvalid)
{
  WeekProgramNode node;
  CHECK_FALSE(weekProgram->getWeekProgramNodeFromSlot(1U, node));
  CHECK_FALSE(weekProgram->getWeekProgramNodeFromSlot(2U, node));
  CHECK_FALSE(weekProgram->getWeekProgramNodeFromSlot(3U, node));
  CHECK_FALSE(weekProgram->getWeekProgramNodeFromSlot(4U, node));
}

TEST_GROUP(weekProgramDataWithValidData)
{
  WeekProgramData * weekProgram;

  TEST_SETUP()
  {
    uint8_t inputValues[20U] = {
        static_cast<uint8_t>(CloudTimerHeatingMode::away), 12, 23, 13, 24, // slot 1
        static_cast<uint8_t>(CloudTimerHeatingMode::eco), 14, 25, 15, 26, // slot 2
        static_cast<uint8_t>(CloudTimerHeatingMode::comfort), 16, 27, 17, 28, // slot 3
        static_cast<uint8_t>(CloudTimerHeatingMode::shutdown), 18, 29, 19, 30  // slot 4
    };

    weekProgram = new WeekProgramData();
    weekProgram->setData(inputValues);
  }

  TEST_TEARDOWN()
  {
    delete weekProgram;
  }
};

TEST(weekProgramDataWithValidData, testGetWeekProgramNodesWhenAllSlotsAreValid)
{
  WeekProgramNode node;
  CHECK_TRUE(weekProgram->getWeekProgramNodeFromSlot(1U, node));
  CHECK_TRUE(weekProgram->getWeekProgramNodeFromSlot(2U, node));
  CHECK_TRUE(weekProgram->getWeekProgramNodeFromSlot(3U, node));
  CHECK_TRUE(weekProgram->getWeekProgramNodeFromSlot(4U, node));
}

TEST(weekProgramDataWithValidData, testGetWeekProgramNodesFailsForInvalidSlotNos)
{
  WeekProgramNode node;
  CHECK_FALSE(weekProgram->getWeekProgramNodeFromSlot(0U, node));
  CHECK_FALSE(weekProgram->getWeekProgramNodeFromSlot(5U, node));
}

TEST(weekProgramDataWithValidData, testWeekProgramNodeValuesAreSetCorrectlySlot1)
{
  WeekProgramNode node;
  weekProgram->getWeekProgramNodeFromSlot(1U, node);
  CHECK_EQUAL(HeatingMode::antiFrost, node.status);
  CHECK_EQUAL(12, node.hours);
  CHECK_EQUAL(23, node.minutes);
}

TEST(weekProgramDataWithValidData, testWeekProgramNodeValuesAreSetCorrectlySlot2)
{
  WeekProgramNode node;
  weekProgram->getWeekProgramNodeFromSlot(2U, node);
  CHECK_EQUAL(HeatingMode::eco, node.status);
  CHECK_EQUAL(14, node.hours);
  CHECK_EQUAL(25, node.minutes);
}

TEST(weekProgramDataWithValidData, testWeekProgramNodeValuesAreSetCorrectlySlot3)
{
  WeekProgramNode node;
  weekProgram->getWeekProgramNodeFromSlot(3U, node);
  CHECK_EQUAL(HeatingMode::comfort, node.status);
  CHECK_EQUAL(16, node.hours);
  CHECK_EQUAL(27, node.minutes);
}

TEST(weekProgramDataWithValidData, testWeekProgramNodeValuesAreSetCorrectlySlot4)
{
  WeekProgramNode node;
  weekProgram->getWeekProgramNodeFromSlot(4U, node);
  CHECK_EQUAL(HeatingMode::off, node.status);
  CHECK_EQUAL(18, node.hours);
  CHECK_EQUAL(29, node.minutes);
}

TEST(weekProgramDataWithValidData, testProvidedWeekProgramNodeIntervalIsLeftUnchanged)
{
  WeekProgramNode node;

  node.interval = Interval::INTERVAL_MONFRI;
  weekProgram->getWeekProgramNodeFromSlot(1U, node);
  CHECK_EQUAL(Interval::INTERVAL_MONFRI, node.interval);

  node.interval = Interval::INTERVAL_MON;
  weekProgram->getWeekProgramNodeFromSlot(2U, node);
  CHECK_EQUAL(Interval::INTERVAL_MON, node.interval);

  node.interval = Interval::INTERVAL_FRI;
  weekProgram->getWeekProgramNodeFromSlot(3U, node);
  CHECK_EQUAL(Interval::INTERVAL_FRI, node.interval);

  node.interval = Interval::INTERVAL_SUN;
  weekProgram->getWeekProgramNodeFromSlot(4U, node);
  CHECK_EQUAL(Interval::INTERVAL_SUN, node.interval);
}


TEST_GROUP(weekProgramDataWithStrangeData)
{
  WeekProgramData * weekProgram;

  TEST_SETUP()
  {
    uint8_t inputValues[20U] = {
        5, 25, 123, 0, 0, // slot 1
        0, 0xEE, 0, 0, 0, // slot 2
        0, 0, 0xEE, 0, 0, // slot 3
        0, 0, 0, 0xEE, 0, // slot 4
    };

    weekProgram = new WeekProgramData();
    weekProgram->setData(inputValues);
  }

  TEST_TEARDOWN()
  {
    delete weekProgram;
  }
};

TEST(weekProgramDataWithStrangeData, testHeatingModeDefaultsToComfort)
{
  WeekProgramNode node;
  weekProgram->getWeekProgramNodeFromSlot(1U, node);
  CHECK_EQUAL(HeatingMode::comfort, node.status);
}

TEST(weekProgramDataWithStrangeData, testTooLargeHoursValueWrapsCorrectly)
{
  WeekProgramNode node;
  weekProgram->getWeekProgramNodeFromSlot(1U, node);
  CHECK_EQUAL(1U, node.hours);
}

TEST(weekProgramDataWithStrangeData, testTooLargeMinutesValueWrapsCorrectly)
{
  WeekProgramNode node;
  weekProgram->getWeekProgramNodeFromSlot(1U, node);
  CHECK_EQUAL(3U, node.minutes);
}

TEST(weekProgramDataWithStrangeData, testInvalidHoursValueResultsInInvalidSlot)
{
  WeekProgramNode node;
  CHECK_FALSE(weekProgram->getWeekProgramNodeFromSlot(2U, node));
}

TEST(weekProgramDataWithStrangeData, testInvalidMinutesValueResultsInInvalidSlot)
{
  WeekProgramNode node;
  CHECK_FALSE(weekProgram->getWeekProgramNodeFromSlot(3U, node));
}

TEST(weekProgramDataWithStrangeData, testInvalidEndHoursValueResultsInInvalidSlot)
{
  WeekProgramNode node;
  CHECK_FALSE(weekProgram->getWeekProgramNodeFromSlot(4U, node));
}


