#include "CppUTest\TestHarness.h"

#include "Constants.h"
#include "Override.h"


TEST_GROUP(Override)
{
  TEST_SETUP()
  {
  }

  TEST_TEARDOWN()
  {
  }
};


TEST(Override, defaultConstructorCreatesOverrideNone)
{
  /*
   * OverrideNone returns mode Comfort as this is default. It should not be used in practice anyway.
   * OverrideNone returns false for getEndDateTime as it has no endDateTime defined.
   */
  DateTime dateTime;

  Override overrideNone;
  CHECK_EQUAL(static_cast<uint8_t>(OverrideType::none), static_cast<uint8_t>(overrideNone.getType()));
  CHECK_EQUAL(static_cast<uint8_t>(HeatingMode::comfort), static_cast<uint8_t>(overrideNone.getMode()));
  CHECK_FALSE(overrideNone.getEndDateTime(dateTime));
}

TEST(Override, constructConstantOverride)
{
  DateTime dateTime;

  Override override(OverrideType::constant, HeatingMode::eco);
  CHECK_EQUAL(static_cast<uint8_t>(OverrideType::constant), static_cast<uint8_t>(override.getType()));
  CHECK_EQUAL(static_cast<uint8_t>(HeatingMode::eco), static_cast<uint8_t>(override.getMode()));
  CHECK_FALSE(override.getEndDateTime(dateTime));
}

TEST(Override, constructNowOverride)
{
  DateTime dateTime;

  Override override(OverrideType::now, HeatingMode::off);
  CHECK_EQUAL(static_cast<uint8_t>(OverrideType::now), static_cast<uint8_t>(override.getType()));
  CHECK_EQUAL(static_cast<uint8_t>(HeatingMode::off), static_cast<uint8_t>(override.getMode()));
  CHECK_FALSE(override.getEndDateTime(dateTime));
}

TEST(Override, constructTimedOverride)
{
  DateTime dateTime = {
      18, 7, 5, // yy,mm,dd
      7, 56, 0, // hh,mm,ss
      WeekDay::THURSDAY
  };

  DateTime outputDateTime;

  Override override(OverrideType::timed, HeatingMode::comfort, dateTime);

  CHECK_EQUAL(static_cast<uint8_t>(OverrideType::timed), static_cast<uint8_t>(override.getType()));
  CHECK_EQUAL(static_cast<uint8_t>(HeatingMode::comfort), static_cast<uint8_t>(override.getMode()));
  CHECK_TRUE(override.getEndDateTime(outputDateTime));
  CHECK_EQUAL(dateTime.year, outputDateTime.year);
  CHECK_EQUAL(dateTime.month, outputDateTime.month);
  CHECK_EQUAL(dateTime.days, outputDateTime.days);
  CHECK_EQUAL(dateTime.hours, outputDateTime.hours);
  CHECK_EQUAL(dateTime.minutes, outputDateTime.minutes);
  CHECK_EQUAL(dateTime.seconds, outputDateTime.seconds);
  CHECK_EQUAL(dateTime.weekDay, outputDateTime.weekDay);
}

TEST(Override, constructTimedOverrideWithoutEndTimeSet)
{
  DateTime dateTime = {
      18, 7, 5, // yy,mm,dd
      7, 56, 0, // hh,mm,ss
      WeekDay::THURSDAY
  };

  DateTime expectedDateTime = Constants::DEFAULT_DATETIME;

  // Defines timed with constructor without DateTime argument - returnes default DateTime i.e.
  // end time in the past. In practice that would end the override as soon as it is created.
  // That should be OK as it has been defined incorrectly
  Override override(OverrideType::timed, HeatingMode::antiFrost);

  CHECK_EQUAL(static_cast<uint8_t>(OverrideType::timed), static_cast<uint8_t>(override.getType()));
  CHECK_EQUAL(static_cast<uint8_t>(HeatingMode::antiFrost), static_cast<uint8_t>(override.getMode()));
  CHECK_TRUE(override.getEndDateTime(dateTime));
  CHECK_EQUAL(expectedDateTime.year, dateTime.year);
  CHECK_EQUAL(expectedDateTime.month, dateTime.month);
  CHECK_EQUAL(expectedDateTime.days, dateTime.days);
  CHECK_EQUAL(expectedDateTime.hours, dateTime.hours);
  CHECK_EQUAL(expectedDateTime.minutes, dateTime.minutes);
  CHECK_EQUAL(expectedDateTime.seconds, dateTime.seconds);
  CHECK_EQUAL(expectedDateTime.weekDay, dateTime.weekDay);
}

TEST(Override, getCloudTimerHeatingModeComfort)
{
  Override override(OverrideType::now, HeatingMode::comfort);
  CHECK_EQUAL(static_cast<uint8_t>(CloudTimerHeatingMode::comfort), static_cast<uint8_t>(override.getModeAsCloudEnum()));
}

TEST(Override, getCloudTimerHeatingModeEco)
{
  Override override(OverrideType::now, HeatingMode::eco);
  CHECK_EQUAL(static_cast<uint8_t>(CloudTimerHeatingMode::eco), static_cast<uint8_t>(override.getModeAsCloudEnum()));
}

TEST(Override, getCloudTimerHeatingModeAway)
{
  Override override(OverrideType::now, HeatingMode::antiFrost);
  CHECK_EQUAL(static_cast<uint8_t>(CloudTimerHeatingMode::away), static_cast<uint8_t>(override.getModeAsCloudEnum()));
}

TEST(Override, getCloudTimerHeatingModeShutdown)
{
  Override override(OverrideType::now, HeatingMode::off);
  CHECK_EQUAL(static_cast<uint8_t>(CloudTimerHeatingMode::shutdown), static_cast<uint8_t>(override.getModeAsCloudEnum()));
}

TEST(Override, testAssignmentAndEquailtyOperatorsAndCopyConstructor)
{
  DateTime dateTime = { 18, 7, 5, 7, 56, 0, WeekDay::THURSDAY };
  Override override(OverrideType::timed, HeatingMode::comfort, dateTime);
  Override newOverride = Override(override);

  CHECK_TRUE(override == newOverride);
  CHECK_FALSE(override != newOverride);

  Override anotherOverride(OverrideType::constant, HeatingMode::comfort);
  CHECK_FALSE(override == anotherOverride);
  CHECK_TRUE(override != anotherOverride);
}


