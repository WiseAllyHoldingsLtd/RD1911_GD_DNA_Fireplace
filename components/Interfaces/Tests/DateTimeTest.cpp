#include "CppUTest\TestHarness.h"

#include "DateTime.h"
#include "Constants.h"

DateTime dateTime;

TEST_GROUP(DateTimeTest)
{
  TEST_SETUP()
  {
    dateTime = 
    { 16, /* year */
      2, /* month */
      28, /* day */
      23, /* hour */
      59, /* minutes */
      59, /* seconds */
      WeekDay::SATURDAY /* incorrect on purpose */
    };
  }

  TEST_TEARDOWN()
  {
  }
};

TEST(DateTimeTest, testIncreaseMinutes)
{
  dateTime.increaseMinute(1u);
  CHECK_EQUAL(0u, dateTime.minutes);
  dateTime.increaseMinute(33u);
  CHECK_EQUAL(33u, dateTime.minutes);
  dateTime.increaseMinute(33u);
  CHECK_EQUAL(6u, dateTime.minutes);
}

TEST(DateTimeTest, testDecreaseMinutes)
{
  dateTime.decreaseMinute(1u);
  CHECK_EQUAL(58u, dateTime.minutes);
  dateTime.decreaseMinute(33u);
  CHECK_EQUAL(25u, dateTime.minutes);
  dateTime.decreaseMinute(26u);
  CHECK_EQUAL(59u, dateTime.minutes);
}

TEST(DateTimeTest, testIncreaseHours)
{
  dateTime.increaseHour(1u);
  CHECK_EQUAL(0u, dateTime.hours);
  dateTime.increaseHour(23u);
  CHECK_EQUAL(23u, dateTime.hours);
  dateTime.increaseHour(23u);
  CHECK_EQUAL(22u, dateTime.hours);
}

TEST(DateTimeTest, testDecreaseHours)
{
  dateTime.decreaseHour(1u);
  CHECK_EQUAL(22u, dateTime.hours);
  dateTime.decreaseHour(25u);
  CHECK_EQUAL(21u, dateTime.hours);
  dateTime.decreaseHour(15u);
  CHECK_EQUAL(6u, dateTime.hours);
}

TEST(DateTimeTest, testNextWeekDay)
{
  dateTime.nextDay();
  CHECK_EQUAL(WeekDay::SUNDAY, dateTime.weekDay);
  dateTime.nextDay();
  CHECK_EQUAL(WeekDay::MONDAY, dateTime.weekDay);
  dateTime.nextDay();
  CHECK_EQUAL(WeekDay::TUESDAY, dateTime.weekDay);
  dateTime.nextDay();
  CHECK_EQUAL(WeekDay::WEDNESDAY, dateTime.weekDay);
  dateTime.nextDay();
  CHECK_EQUAL(WeekDay::THURSDAY, dateTime.weekDay);
  dateTime.nextDay();
  CHECK_EQUAL(WeekDay::FRIDAY, dateTime.weekDay);
  dateTime.nextDay();
  CHECK_EQUAL(WeekDay::SATURDAY, dateTime.weekDay);
}

TEST(DateTimeTest, testPreviousWeekDay)
{
  dateTime.previousDay();
  CHECK_EQUAL(WeekDay::FRIDAY, dateTime.weekDay);
  dateTime.previousDay();
  CHECK_EQUAL(WeekDay::THURSDAY, dateTime.weekDay);
  dateTime.previousDay();
  CHECK_EQUAL(WeekDay::WEDNESDAY, dateTime.weekDay);
  dateTime.previousDay();
  CHECK_EQUAL(WeekDay::TUESDAY, dateTime.weekDay);
  dateTime.previousDay();
  CHECK_EQUAL(WeekDay::MONDAY, dateTime.weekDay);
  dateTime.previousDay();
  CHECK_EQUAL(WeekDay::SUNDAY, dateTime.weekDay);
  dateTime.previousDay();
  CHECK_EQUAL(WeekDay::SATURDAY, dateTime.weekDay);
}

TEST(DateTimeTest, testValidationOfDateTime)
{
  DateTime validDate = { 18, 5, 19, 10, 45, 31, WeekDay::SATURDAY };
  CHECK_TRUE(validDate.isValid());

  validDate = Constants::DEFAULT_DATETIME;
  CHECK_TRUE(validDate.isValid());

  validDate = { 18, 2, 28, 23, 59, 59, WeekDay::MONDAY };
  CHECK_TRUE(validDate.isValid());

  validDate = { 20, 2, 29, 23, 59, 59, WeekDay::MONDAY };
  CHECK_TRUE(validDate.isValid());

  validDate = { 20, 4, 30, 1, 1, 1, WeekDay::MONDAY };
  CHECK_TRUE(validDate.isValid());

  validDate = { 99, 12, 31, 0, 0, 0, WeekDay::SUNDAY };
  CHECK_TRUE(validDate.isValid());

  DateTime invalidDate = { 18, 5, 32, 10, 45, 31, WeekDay::SATURDAY };
  CHECK_FALSE(invalidDate.isValid());

  invalidDate = { 100, 5, 19, 10, 45, 31, WeekDay::SATURDAY };
  CHECK_FALSE(invalidDate.isValid());

  invalidDate = { 18, 2, 29, 23, 59, 59, WeekDay::MONDAY };
  CHECK_FALSE(invalidDate.isValid());

  invalidDate = { 20, 4, 31, 1, 1, 1, WeekDay::MONDAY };
  CHECK_FALSE(invalidDate.isValid());

  invalidDate = { 18, 5, 1, 24, 45, 31, WeekDay::SATURDAY };
  CHECK_FALSE(invalidDate.isValid());

  invalidDate = { 18, 5, 1, 10, 60, 31, WeekDay::SATURDAY };
  CHECK_FALSE(invalidDate.isValid());

  invalidDate = { 18, 5, 1, 10, 45, 60, WeekDay::SATURDAY };
  CHECK_FALSE(invalidDate.isValid());
}


