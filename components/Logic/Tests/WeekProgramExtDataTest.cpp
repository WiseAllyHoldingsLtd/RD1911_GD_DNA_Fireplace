

#include <cstring>
#include "CppUTest\TestHarness.h"
#include "WeekProgramExtData.h"


TEST_GROUP(weekProgramExtData)
{
  WeekProgramExtData *weekProgram;

  TEST_SETUP()
  {
    weekProgram = new WeekProgramExtData();
  }

  TEST_TEARDOWN()
  {
    delete weekProgram;
  }
};

TEST(weekProgramExtData, testSizeAndNumberOfNodes)
{
  uint8_t inputValues[20U] = {12, 34, 2345, 35, 567, 34, 2, 4, 234, 234, 0, 76, 12, 92, 21, 54, 2, 123, 86, 34 };

  weekProgram->setData(inputValues, sizeof(inputValues));
  CHECK_EQUAL(sizeof(inputValues), weekProgram->getCurrentSize());
  CHECK_EQUAL(4u, weekProgram->getNumberOfSlots());
}

TEST(weekProgramExtData, testIncreasingNumberOfSlots)
{
  uint8_t inputValues[20U] = {12, 34, 2345, 35, 567, 34, 2, 4, 234, 234, 0, 76, 12, 92, 21, 54, 2, 123, 86, 34 };

  weekProgram->setData(inputValues, sizeof(inputValues)/2u);
  CHECK_EQUAL(10u, weekProgram->getCurrentSize());
  CHECK_EQUAL(2u, weekProgram->getNumberOfSlots());

  weekProgram->setData(inputValues, sizeof(inputValues));
  CHECK_EQUAL(20u, weekProgram->getCurrentSize());
  CHECK_EQUAL(4u, weekProgram->getNumberOfSlots());
}

TEST(weekProgramExtData, testDecresingNumberOfSlots)
{
  uint8_t inputValues[20U] = {12, 34, 2345, 35, 567, 34, 2, 4, 234, 234, 0, 76, 12, 92, 21, 54, 2, 123, 86, 34 };

  weekProgram->setData(inputValues, sizeof(inputValues));
  CHECK_EQUAL(sizeof(inputValues), weekProgram->getCurrentSize());
  CHECK_EQUAL(4u, weekProgram->getNumberOfSlots());

  weekProgram->setData(inputValues, 10u);
  CHECK_EQUAL(10u, weekProgram->getCurrentSize());
  CHECK_EQUAL(2u, weekProgram->getNumberOfSlots());
}

TEST(weekProgramExtData, discardIncompleteNodes)
{
  uint8_t inputValues[20U] = {12, 34, 2345, 35, 567, 34, 2, 4, 234, 234, 0, 76, 12, 92, 21, 54, 2, 123, 86, 34 };

  weekProgram->setData(inputValues, 13u);
  CHECK_EQUAL(13u, weekProgram->getCurrentSize());
  CHECK_EQUAL(2u, weekProgram->getNumberOfSlots());
}

TEST(weekProgramExtData, testSetAndGetData)
{
  uint8_t inputValues[20U] = { 12, 34, 2345, 35, 567, 34, 2, 4, 234, 234, 0, 76, 12, 92, 21, 54, 2, 123, 86, 34 };
  uint8_t outputValues[20U];

  weekProgram->setData(inputValues, sizeof(inputValues));
  weekProgram->getData(outputValues, sizeof(outputValues));
  MEMCMP_EQUAL(inputValues, outputValues, sizeof(inputValues));

  inputValues[4U]++;
  weekProgram->setData(inputValues, sizeof(inputValues));
  weekProgram->getData(outputValues, sizeof(outputValues));
  MEMCMP_EQUAL(inputValues, outputValues, sizeof(inputValues));
}

TEST(weekProgramExtData, getTooMuchData)
{
  uint8_t inputValues[10U] = { 12u, 34u, 45u, 35u, 67u, 34u, 2u, 4u, 234u, 234u };
  uint8_t outputValues[15U] = {};
  uint8_t expectedValues[15u] = { 12u, 34u, 45u, 35u, 67u, 34u, 2u, 4u, 234u, 234u,
                                  0u, 0u, 0u, 0u, 0u };

  weekProgram->setData(inputValues, sizeof(inputValues));
  weekProgram->getData(outputValues, sizeof(outputValues)); // request more data than available
  MEMCMP_EQUAL(expectedValues, outputValues, sizeof(expectedValues));
}

TEST(weekProgramExtData, weekProgramDataIsInitiallyOfSizeZero)
{
  uint8_t expectedData[20u] = {1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u,
                               11u, 12u, 13u, 14u, 15u, 16u, 17u, 18u, 19u, 20u };;
  uint8_t initialData[20U] = {1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u,
                              11u, 12u, 13u, 14u, 15u, 16u, 17u, 18u, 19u, 20u };
  weekProgram->getData(initialData, sizeof(initialData));
  MEMCMP_EQUAL(expectedData, initialData, sizeof(initialData));
  CHECK_EQUAL(0u, weekProgram->getCurrentSize());
  CHECK_EQUAL(0u, weekProgram->getNumberOfSlots());

  // note: tests that initialData array isn't modified by the getData call
}

TEST(weekProgramExtData, testGetWeekProgramNodeFailsWhenDataIsInvalid)
{
  WeekProgramNode node;
  CHECK_FALSE(weekProgram->getWeekProgramNodeFromSlot(1U, node));
  CHECK_FALSE(weekProgram->getWeekProgramNodeFromSlot(2U, node));
  CHECK_FALSE(weekProgram->getWeekProgramNodeFromSlot(3U, node));
  CHECK_FALSE(weekProgram->getWeekProgramNodeFromSlot(4U, node));
}

TEST_GROUP(weekProgramExtDataWithValidData)
{
  WeekProgramExtData *weekProgram;

  TEST_SETUP()
  {
    uint8_t inputValues[] = {
        12, 23, 13, 24, static_cast<uint8_t>(CloudTimerHeatingMode::away), // slot 1
        14, 25, 15, 26, static_cast<uint8_t>(CloudTimerHeatingMode::eco), // slot 2
        16, 27, 17, 28, static_cast<uint8_t>(CloudTimerHeatingMode::comfort), // slot 3
        18, 29, 19, 30, static_cast<uint8_t>(CloudTimerHeatingMode::shutdown), // slot 4
        19, 30, 19, 40, static_cast<uint8_t>(CloudTimerHeatingMode::away), // slot 5
        19, 40, 19, 45, static_cast<uint8_t>(CloudTimerHeatingMode::comfort), // slot 6
        19, 45, 19, 50, static_cast<uint8_t>(CloudTimerHeatingMode::eco), // slot 7
        19, 50, 23, 50, static_cast<uint8_t>(CloudTimerHeatingMode::shutdown), // slot 8
    };

    weekProgram = new WeekProgramExtData();
    weekProgram->setData(inputValues, sizeof(inputValues));
  }

  TEST_TEARDOWN()
  {
    delete weekProgram;
  }
};

TEST(weekProgramExtDataWithValidData, testGetWeekProgramNodesWhenAllSlotsAreValid)
{
  WeekProgramNode node;
  CHECK_TRUE(weekProgram->getWeekProgramNodeFromSlot(1U, node));
  CHECK_TRUE(weekProgram->getWeekProgramNodeFromSlot(2U, node));
  CHECK_TRUE(weekProgram->getWeekProgramNodeFromSlot(3U, node));
  CHECK_TRUE(weekProgram->getWeekProgramNodeFromSlot(4U, node));
  CHECK_TRUE(weekProgram->getWeekProgramNodeFromSlot(5U, node));
  CHECK_TRUE(weekProgram->getWeekProgramNodeFromSlot(6U, node));
  CHECK_TRUE(weekProgram->getWeekProgramNodeFromSlot(7U, node));
  CHECK_TRUE(weekProgram->getWeekProgramNodeFromSlot(8U, node));
}

TEST(weekProgramExtDataWithValidData, testGetWeekProgramNodesFailsForInvalidSlotNos)
{
  WeekProgramNode node;
  CHECK_FALSE(weekProgram->getWeekProgramNodeFromSlot(0U, node));
  CHECK_FALSE(weekProgram->getWeekProgramNodeFromSlot(9U, node));
}

TEST(weekProgramExtDataWithValidData, testWeekProgramNodeValuesAreSetCorrectlySlot1)
{
  WeekProgramNode node;
  weekProgram->getWeekProgramNodeFromSlot(1U, node);
  CHECK_EQUAL(HeatingMode::antiFrost, node.status);
  CHECK_EQUAL(12, node.hours);
  CHECK_EQUAL(23, node.minutes);
}

TEST(weekProgramExtDataWithValidData, testWeekProgramNodeValuesAreSetCorrectlySlot2)
{
  WeekProgramNode node;
  weekProgram->getWeekProgramNodeFromSlot(2U, node);
  CHECK_EQUAL(HeatingMode::eco, node.status);
  CHECK_EQUAL(14, node.hours);
  CHECK_EQUAL(25, node.minutes);
}

TEST(weekProgramExtDataWithValidData, testWeekProgramNodeValuesAreSetCorrectlySlot3)
{
  WeekProgramNode node;
  weekProgram->getWeekProgramNodeFromSlot(3U, node);
  CHECK_EQUAL(HeatingMode::comfort, node.status);
  CHECK_EQUAL(16, node.hours);
  CHECK_EQUAL(27, node.minutes);
}

TEST(weekProgramExtDataWithValidData, testWeekProgramNodeValuesAreSetCorrectlySlot4)
{
  WeekProgramNode node;
  weekProgram->getWeekProgramNodeFromSlot(4U, node);
  CHECK_EQUAL(HeatingMode::off, node.status);
  CHECK_EQUAL(18, node.hours);
  CHECK_EQUAL(29, node.minutes);
}

TEST(weekProgramExtDataWithValidData, testWeekProgramNodeValuesAreSetCorrectlySlot5)
{
  WeekProgramNode node;
  weekProgram->getWeekProgramNodeFromSlot(5U, node);
  CHECK_EQUAL(HeatingMode::antiFrost, node.status);
  CHECK_EQUAL(19, node.hours);
  CHECK_EQUAL(30, node.minutes);
}

TEST(weekProgramExtDataWithValidData, testWeekProgramNodeValuesAreSetCorrectlySlot6)
{
  WeekProgramNode node;
  weekProgram->getWeekProgramNodeFromSlot(6U, node);
  CHECK_EQUAL(HeatingMode::comfort, node.status);
  CHECK_EQUAL(19, node.hours);
  CHECK_EQUAL(40, node.minutes);
}

TEST(weekProgramExtDataWithValidData, testWeekProgramNodeValuesAreSetCorrectlySlot7)
{
  WeekProgramNode node;
  weekProgram->getWeekProgramNodeFromSlot(7U, node);
  CHECK_EQUAL(HeatingMode::eco, node.status);
  CHECK_EQUAL(19, node.hours);
  CHECK_EQUAL(45, node.minutes);
}

TEST(weekProgramExtDataWithValidData, testWeekProgramNodeValuesAreSetCorrectlySlot8)
{
  WeekProgramNode node;
  weekProgram->getWeekProgramNodeFromSlot(8U, node);
  CHECK_EQUAL(HeatingMode::off, node.status);
  CHECK_EQUAL(19, node.hours);
  CHECK_EQUAL(50, node.minutes);
}


TEST(weekProgramExtDataWithValidData, testProvidedWeekProgramNodeIntervalIsLeftUnchanged)
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


TEST_GROUP(weekProgramExtDataWithStrangeData)
{
  WeekProgramExtData *weekProgram;

  TEST_SETUP()
  {
    uint8_t inputValues[20U] = {
        25, 123, 0, 0, 5, // slot 1
        0xEE, 0, 0, 0, 0, // slot 2
        0, 0xEE, 0, 0, 0, // slot 3
        0, 0, 0xEE, 0, 0, // slot 4
    };

    weekProgram = new WeekProgramExtData();
    weekProgram->setData(inputValues, sizeof(inputValues));
  }

  TEST_TEARDOWN()
  {
    delete weekProgram;
  }
};

TEST(weekProgramExtDataWithStrangeData, testHeatingModeDefaultsToComfort)
{
  WeekProgramNode node;
  weekProgram->getWeekProgramNodeFromSlot(1U, node);
  CHECK_EQUAL(HeatingMode::comfort, node.status);
}

TEST(weekProgramExtDataWithStrangeData, testTooLargeHoursValueWrapsCorrectly)
{
  WeekProgramNode node;
  weekProgram->getWeekProgramNodeFromSlot(1U, node);
  CHECK_EQUAL(1U, node.hours);
}

TEST(weekProgramExtDataWithStrangeData, testTooLargeMinutesValueWrapsCorrectly)
{
  WeekProgramNode node;
  weekProgram->getWeekProgramNodeFromSlot(1U, node);
  CHECK_EQUAL(3U, node.minutes);
}

TEST(weekProgramExtDataWithStrangeData, testInvalidHoursValueResultsInInvalidSlot)
{
  WeekProgramNode node;
  CHECK_FALSE(weekProgram->getWeekProgramNodeFromSlot(2U, node));
}

TEST(weekProgramExtDataWithStrangeData, testInvalidMinutesValueResultsInInvalidSlot)
{
  WeekProgramNode node;
  CHECK_FALSE(weekProgram->getWeekProgramNodeFromSlot(3U, node));
}

TEST(weekProgramExtDataWithStrangeData, testInvalidEndHoursValueResultsInInvalidSlot)
{
  WeekProgramNode node;
  CHECK_FALSE(weekProgram->getWeekProgramNodeFromSlot(4U, node));
}

