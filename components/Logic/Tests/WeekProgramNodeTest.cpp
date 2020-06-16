#include "CppUTest\TestHarness.h"
#include "CppUTestExt\MockSupport.h"

#include "WeekProgramNode.h"
#include "Constants.h"

static DateTime dt;
static WeekProgramNode node;


TEST_GROUP(WeekProgramNode)
{
  TEST_SETUP()
  {
  }

  TEST_TEARDOWN()
  {
    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
  }
};

TEST(WeekProgramNode, testBuildBitMap)
{
  node.minutes = 0u;
  node.hours = 0u;
  node.interval = Interval::INTERVAL_MONSUN; //0b011111111
  node.status = HeatingMode::comfort;

  uint8_t bitmap = node.buildBitMap();
  CHECK_EQUAL(127u, bitmap);

  node.interval = Interval::INTERVAL_SAT; //0b00100000
  bitmap = node.buildBitMap();
  CHECK_EQUAL(32u, bitmap);

  node.interval = Interval::INTERVAL_MONFRI; //0b00011111
  bitmap = node.buildBitMap();
  CHECK_EQUAL(31u, bitmap);

  node.interval = Interval::INTERVAL_SATSUN; //0b01100000
  bitmap = node.buildBitMap();
  CHECK_EQUAL(96u, bitmap);
}

TEST(WeekProgramNode, testgetBestFittingMinutesIntoWeek)
{
  dt =
  {
    0,
    0,
    0,
    13,
    15,
    40,
    WeekDay::FRIDAY
  };
  node.minutes = 15u;
  node.hours = 13u;
  node.interval = Interval::INTERVAL_SATSUN;
  node.status = HeatingMode::comfort;

  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // Will return the num of minutes till sunday 13:15.
  CHECK_EQUAL(6 * 60 * 24 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::SUNDAY;
  dt.hours = 14;
  minutes = node.getBestFittingMinutesIntoWeek(dt); // Will return num of min. till sunday 13:15, because datetime has passed the last point in time
  CHECK_EQUAL(9435, minutes);
}

TEST_GROUP(WeekProgramNodeSingleDayTest)
{
  TEST_SETUP()
  {
    dt =
    {
    0,
    0,
    0,
    0,
    0,
    40,
    WeekDay::MONDAY
  };
  node.minutes = 15u;
  node.hours = 13u;
  node.interval = Interval::INTERVAL_MON;
  node.status = HeatingMode::comfort;
  }

  TEST_TEARDOWN()
  {
    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
  }

};

TEST(WeekProgramNodeSingleDayTest, testgetBestFittingMinutesIntoWeekMonday)
{
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // Will return the num of minutes from monday 00:00 till the node's starting time.
  CHECK_EQUAL(24 * 0 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeSingleDayTest, testgetBestFittingMinutesIntoWeekTuesday)
{
  node.interval = Interval::INTERVAL_TUE;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // Will return the num of minutes from monday 00:00 till the node's starting time.
  CHECK_EQUAL(24 * 1 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeSingleDayTest, testgetBestFittingMinutesIntoWeekWednesday)
{
  node.interval = Interval::INTERVAL_WED;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // Will return the num of minutes from monday 00:00 till the node's starting time.
  CHECK_EQUAL(24 * 2 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeSingleDayTest, testgetBestFittingMinutesIntoWeekThursday)
{
  node.interval = Interval::INTERVAL_THU;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // Will return the num of minutes from monday 00:00 till the node's starting time.
  CHECK_EQUAL(24 * 3 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeSingleDayTest, testgetBestFittingMinutesIntoWeekFriday)
{
  node.interval = Interval::INTERVAL_FRI;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // Will return the num of minutes from monday 00:00 till the node's starting time.
  CHECK_EQUAL(24 * 4 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeSingleDayTest, testgetBestFittingMinutesIntoWeekSaturday)
{
  node.interval = Interval::INTERVAL_SAT;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // Will return the num of minutes from monday 00:00 till the node's starting time.
  CHECK_EQUAL(24 * 5 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeSingleDayTest, testgetBestFittingMinutesIntoWeekSunday)
{
  node.interval = Interval::INTERVAL_SUN;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // Will return the num of minutes from monday 00:00 till the node's starting time.
  CHECK_EQUAL(24 * 6 * 60 + 13 * 60 + 15, minutes);
}

TEST_GROUP(WeekProgramNodeSingleDayTestPastNodeTime)
{
  TEST_SETUP()
  {
    dt =
    {
    0,
    0,
    0,
    14,
    15,
    40,
    WeekDay::MONDAY
  };
  node.minutes = 15u;
  node.hours = 13u;
  node.interval = Interval::INTERVAL_MON;
  node.status = HeatingMode::comfort;


  }

  TEST_TEARDOWN()
  {
    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
  }

};



TEST(WeekProgramNodeSingleDayTestPastNodeTime, testgetBestFittingMinutesIntoWeekSingleDaysTimePastNodeTimeMonday)
{
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // Will return the num of minutes from monday 00:00 till the node's starting time.
  CHECK_EQUAL(24 * 0 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeSingleDayTestPastNodeTime, testgetBestFittingMinutesIntoWeekSingleDaysTimePastNodeTimeTuesday)
{
  node.interval = Interval::INTERVAL_TUE;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // Will return the num of minutes from monday 00:00 till the node's starting time.
  CHECK_EQUAL(24 * 1 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeSingleDayTestPastNodeTime, testgetBestFittingMinutesIntoWeekSingleDaysTimePastNodeTimeWednesday)
{
  node.interval = Interval::INTERVAL_WED;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // Will return the num of minutes from monday 00:00 till the node's starting time.
  CHECK_EQUAL(24 * 2 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeSingleDayTestPastNodeTime, testgetBestFittingMinutesIntoWeekSingleDaysTimePastNodeTimeThursday)
{
  node.interval = Interval::INTERVAL_THU;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // Will return the num of minutes from monday 00:00 till the node's starting time.
  CHECK_EQUAL(24 * 3 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeSingleDayTestPastNodeTime, testgetBestFittingMinutesIntoWeekSingleDaysTimePastNodeTimeFriday)
{
  node.interval = Interval::INTERVAL_FRI;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // Will return the num of minutes from monday 00:00 till the node's starting time.
  CHECK_EQUAL(24 * 4 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeSingleDayTestPastNodeTime, testgetBestFittingMinutesIntoWeekSingleDaysTimePastNodeTimeSaturday)
{
  node.interval = Interval::INTERVAL_SAT;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // Will return the num of minutes from monday 00:00 till the node's starting time.
  CHECK_EQUAL(24 * 5 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeSingleDayTestPastNodeTime, testgetBestFittingMinutesIntoWeekSingleDaysTimePastNodeTimeSunday)
{
  node.interval = Interval::INTERVAL_SUN;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // Will return the num of minutes from monday 00:00 till the node's starting time.
  CHECK_EQUAL(24 * 6 * 60 + 13 * 60 + 15, minutes);
}

TEST_GROUP(WeekProgramNodeSatSunTestBeforeNodeTime)
{
  TEST_SETUP()
  {
    dt =
    {
    0,
    0,
    0,
    0,
    0,
    40,
    WeekDay::MONDAY
  };
  node.minutes = 15u;
  node.hours = 13u;
  node.interval = Interval::INTERVAL_SATSUN;
  node.status = HeatingMode::comfort;


  }

  TEST_TEARDOWN()
  {
    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
  }

};


TEST(WeekProgramNodeSatSunTestBeforeNodeTime, testgetBestFittingMinutesIntoWeekSatSunIntervalCurrentTimeBeforeNodeTimeMon)
{
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min till
  CHECK_EQUAL(24 * 6 * 60 + 13 * 60 + 15, minutes);
}


TEST(WeekProgramNodeSatSunTestBeforeNodeTime, testgetBestFittingMinutesIntoWeekSatSunIntervalCurrentTimeBeforeNodeTimeTue)
{
  dt.weekDay = WeekDay::TUESDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min till
  CHECK_EQUAL(24 * 6 * 60 + 13 * 60 + 15, minutes);
}


TEST(WeekProgramNodeSatSunTestBeforeNodeTime, testgetBestFittingMinutesIntoWeekSatSunIntervalCurrentTimeBeforeNodeTimeWed)
{
  dt.weekDay = WeekDay::WEDNESDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min till
  CHECK_EQUAL(24 * 6 * 60 + 13 * 60 + 15, minutes);
}


TEST(WeekProgramNodeSatSunTestBeforeNodeTime, testgetBestFittingMinutesIntoWeekSatSunIntervalCurrentTimeBeforeNodeTimeThu)
{
  dt.weekDay = WeekDay::THURSDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min till
  CHECK_EQUAL(24 * 6 * 60 + 13 * 60 + 15, minutes);
}


TEST(WeekProgramNodeSatSunTestBeforeNodeTime, testgetBestFittingMinutesIntoWeekSatSunIntervalCurrentTimeBeforeNodeTimeFri)
{
  dt.weekDay = WeekDay::FRIDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min till
  CHECK_EQUAL(24 * 6 * 60 + 13 * 60 + 15, minutes);
}


TEST(WeekProgramNodeSatSunTestBeforeNodeTime, testgetBestFittingMinutesIntoWeekSatSunIntervalCurrentTimeBeforeNodeTimeSat)
{
  dt.weekDay = WeekDay::SATURDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min till
  CHECK_EQUAL(24 * 6 * 60 + 13 * 60 + 15, minutes);
}


TEST(WeekProgramNodeSatSunTestBeforeNodeTime, testgetBestFittingMinutesIntoWeekSatSunIntervalCurrentTimeBeforeNodeTimeSun)
{
  dt.weekDay = WeekDay::SUNDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min till
  CHECK_EQUAL(24 * 5 * 60 + 13 * 60 + 15, minutes);
}
TEST_GROUP(WeekProgramNodeSatSunTestPastNodeTime)
{
  TEST_SETUP()
  {
    dt =
    {
    0,
    0,
    0,
    14,
    5,
    40,
    WeekDay::MONDAY
  };
  node.minutes = 15u;
  node.hours = 13u;
  node.interval = Interval::INTERVAL_SATSUN;
  node.status = HeatingMode::comfort;
  }

  TEST_TEARDOWN()
  {
    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
  }

};

TEST(WeekProgramNodeSatSunTestPastNodeTime, testgetBestFittingMinutesIntoWeekSatSunIntervalCurrentTimeAfterNodeTimeMon)
{
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min
  CHECK_EQUAL(24 * 6 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeSatSunTestPastNodeTime, testgetBestFittingMinutesIntoWeekSatSunIntervalCurrentTimeAfterNodeTimeTue)
{
  dt.weekDay = WeekDay::TUESDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min
  CHECK_EQUAL(24 * 6 * 60 + 13 * 60 + 15, minutes);
}


TEST(WeekProgramNodeSatSunTestPastNodeTime, testgetBestFittingMinutesIntoWeekSatSunIntervalCurrentTimeAfterNodeTimeWed)
{
  dt.weekDay = WeekDay::WEDNESDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min
  CHECK_EQUAL(24 * 6 * 60 + 13 * 60 + 15, minutes);
}


TEST(WeekProgramNodeSatSunTestPastNodeTime, testgetBestFittingMinutesIntoWeekSatSunIntervalCurrentTimeAfterNodeTimeThu)
{
  dt.weekDay = WeekDay::THURSDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min
  CHECK_EQUAL(24 * 6 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeSatSunTestPastNodeTime, testgetBestFittingMinutesIntoWeekSatSunIntervalCurrentTimeAfterNodeTimeFri)
{
  dt.weekDay = WeekDay::FRIDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min
  CHECK_EQUAL(24 * 6 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeSatSunTestPastNodeTime, testgetBestFittingMinutesIntoWeekSatSunIntervalCurrentTimeAfterNodeTimeSat)
{
  dt.weekDay = WeekDay::SATURDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min
  CHECK_EQUAL(24 * 5 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeSatSunTestPastNodeTime, testgetBestFittingMinutesIntoWeekSatSunIntervalCurrentTimeAfterNodeTimeSun)
{
  dt.weekDay = WeekDay::SUNDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min
  CHECK_EQUAL(24 * 6 * 60 + 13 * 60 + 15, minutes);
}

TEST_GROUP(WeekProgramNodeMonFriTestPastNodeTime)
{
  TEST_SETUP()
{
    dt =
  {
    0,
    0,
    0,
    14,
    5,
    40,
    WeekDay::MONDAY
  };
  node.minutes = 15u;
  node.hours = 13u;
  node.interval = Interval::INTERVAL_MONFRI;
  node.status = HeatingMode::comfort;
  }

  TEST_TEARDOWN()
  {
    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
  }

};

TEST(WeekProgramNodeMonFriTestPastNodeTime, testgetBestFittingMinutesIntoWeekMonFriIntervalCurrentTimeAfterNodeTimeMon)
{
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min till monday
  CHECK_EQUAL(24 * 0 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeMonFriTestPastNodeTime, testgetBestFittingMinutesIntoWeekMonFriIntervalCurrentTimeAfterNodeTimeTue)
{
  dt.weekDay = WeekDay::TUESDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min till tuesday
  CHECK_EQUAL(24 * 1 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeMonFriTestPastNodeTime, testgetBestFittingMinutesIntoWeekMonFriIntervalCurrentTimeAfterNodeTimeWed)
{
  dt.weekDay = WeekDay::WEDNESDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min till wednesday
  CHECK_EQUAL(24 * 2 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeMonFriTestPastNodeTime, testgetBestFittingMinutesIntoWeekMonFriIntervalCurrentTimeAfterNodeTimeThu)
{
  dt.weekDay = WeekDay::THURSDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min till thursday
  CHECK_EQUAL(24 * 3 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeMonFriTestPastNodeTime, testgetBestFittingMinutesIntoWeekMonFriIntervalCurrentTimeAfterNodeTimeFri)
{
  dt.weekDay = WeekDay::FRIDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min till friday
  CHECK_EQUAL(24 * 4 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeMonFriTestPastNodeTime, testgetBestFittingMinutesIntoWeekMonFriIntervalCurrentTimeAfterNodeTimeSat)
{
  dt.weekDay = WeekDay::SATURDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min till friday
  CHECK_EQUAL(24 * 4 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeMonFriTestPastNodeTime, testgetBestFittingMinutesIntoWeekMonFriIntervalCurrentTimeAfterNodeTimeSun)
{
  dt.weekDay = WeekDay::SUNDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min till friday
  CHECK_EQUAL(24 * 4 * 60 + 13 * 60 + 15, minutes);
}

TEST_GROUP(WeekProgramNodeMonFriTestBeforeNodeTime)
{
  TEST_SETUP()
{
    dt =
  {
    0,
    0,
    0,
    0,
    5,
    40,
    WeekDay::MONDAY
  };
  node.minutes = 15u;
  node.hours = 13u;
  node.interval = Interval::INTERVAL_MONFRI;
  node.status = HeatingMode::comfort;
  }

  TEST_TEARDOWN()
  {
    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
  }

};


TEST(WeekProgramNodeMonFriTestBeforeNodeTime, testgetBestFittingMinutesIntoWeekMonFriIntervalCurrentTimeBeforeNodeTimeMon)
{
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min till friday
  CHECK_EQUAL(24 * 4 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeMonFriTestBeforeNodeTime, testgetBestFittingMinutesIntoWeekMonFriIntervalCurrentTimeBeforeNodeTimeTue)
{
  dt.weekDay = WeekDay::TUESDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min till monday
  CHECK_EQUAL(24 * 0 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeMonFriTestBeforeNodeTime, testgetBestFittingMinutesIntoWeekMonFriIntervalCurrentTimeBeforeNodeTimeWed)
{
  dt.weekDay = WeekDay::WEDNESDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min till tuesday
  CHECK_EQUAL(24 * 1 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeMonFriTestBeforeNodeTime, testgetBestFittingMinutesIntoWeekMonFriIntervalCurrentTimeBeforeNodeTimeThu)
{
  dt.weekDay = WeekDay::THURSDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min till wednesday
  CHECK_EQUAL(24 * 2 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeMonFriTestBeforeNodeTime, testgetBestFittingMinutesIntoWeekMonFriIntervalCurrentTimeBeforeNodeTimeFri)
{
  dt.weekDay = WeekDay::FRIDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min till thursday
  CHECK_EQUAL(24 * 3 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeMonFriTestBeforeNodeTime, testgetBestFittingMinutesIntoWeekMonFriIntervalCurrentTimeBeforeNodeTimeSat)
{
  dt.weekDay = WeekDay::SATURDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min till friday
  CHECK_EQUAL(24 * 4 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeMonFriTestBeforeNodeTime, testgetBestFittingMinutesIntoWeekMonFriIntervalCurrentTimeBeforeNodeTimeSun)
{
  dt.weekDay = WeekDay::SUNDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min till friday
  CHECK_EQUAL(24 * 4 * 60 + 13 * 60 + 15, minutes);
}

TEST_GROUP(WeekProgramNodeMonSunTestAfterNodeTime)
{
  TEST_SETUP()
{
    dt =
  {
    0,
    0,
    0,
    14,
    5,
    40,
    WeekDay::MONDAY
  };
  node.minutes = 15u;
  node.hours = 13u;
  node.interval = Interval::INTERVAL_MONSUN;
  node.status = HeatingMode::comfort;
  }

  TEST_TEARDOWN()
  {
    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
  }

};

TEST(WeekProgramNodeMonSunTestAfterNodeTime, testgetBestFittingMinutesIntoWeekMonSunIntervalCurrentTimeAfterNodeTimeMon)
{
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min till monday
  CHECK_EQUAL(24 * 0 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeMonSunTestAfterNodeTime, testgetBestFittingMinutesIntoWeekMonSunIntervalCurrentTimeAfterNodeTimeTue)
{
  dt.weekDay = WeekDay::TUESDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt);// returns num of min till tuesday
  CHECK_EQUAL(24 * 1 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeMonSunTestAfterNodeTime, testgetBestFittingMinutesIntoWeekMonSunIntervalCurrentTimeAfterNodeTimeWed)
{
  dt.weekDay = WeekDay::WEDNESDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min till wednesday
  CHECK_EQUAL(24 * 2 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeMonSunTestAfterNodeTime, testgetBestFittingMinutesIntoWeekMonSunIntervalCurrentTimeAfterNodeTimeThu)
{
  dt.weekDay = WeekDay::THURSDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt);// returns num of min till thursday
  CHECK_EQUAL(24 * 3 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeMonSunTestAfterNodeTime, testgetBestFittingMinutesIntoWeekMonSunIntervalCurrentTimeAfterNodeTimeFri)
{
  dt.weekDay = WeekDay::FRIDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt);// returns num of min till friday
  CHECK_EQUAL(24 * 4 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeMonSunTestAfterNodeTime, testgetBestFittingMinutesIntoWeekMonSunIntervalCurrentTimeAfterNodeTimeSat)
{
  dt.weekDay = WeekDay::SATURDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt);// returns num of min till saturday
  CHECK_EQUAL(24 * 5 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeMonSunTestAfterNodeTime, testgetBestFittingMinutesIntoWeekMonSunIntervalCurrentTimeAfterNodeTimeSun)
{
  dt.weekDay = WeekDay::SUNDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt);// returns num of min till sunday
  CHECK_EQUAL(24 * 6 * 60 + 13 * 60 + 15, minutes);
}

TEST_GROUP(WeekProgramNodeMonSunTestBeforeNodeTime)
{
  TEST_SETUP()
{
    dt =
  {
    0,
    0,
    0,
    0,
    5,
    40,
    WeekDay::MONDAY
  };
  node.minutes = 15u;
  node.hours = 13u;
  node.interval = Interval::INTERVAL_MONSUN;
  node.status = HeatingMode::comfort;
  }

  TEST_TEARDOWN()
  {
    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
  }

};

TEST(WeekProgramNodeMonSunTestBeforeNodeTime, testgetBestFittingMinutesIntoWeekMonSunIntervalCurrentTimeBeforeNodeTimeMon)
{
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); //returns num of min till sunday
  CHECK_EQUAL(24 * 6 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeMonSunTestBeforeNodeTime, testgetBestFittingMinutesIntoWeekMonSunIntervalCurrentTimeBeforeNodeTimeTue)
{
  dt.weekDay = WeekDay::TUESDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min till monday
  CHECK_EQUAL(24 * 0 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeMonSunTestBeforeNodeTime, testgetBestFittingMinutesIntoWeekMonSunIntervalCurrentTimeBeforeNodeTimeWed)
{
  dt.weekDay = WeekDay::WEDNESDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); //returns num of min till tuesday
  CHECK_EQUAL(24 * 1 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeMonSunTestBeforeNodeTime, testgetBestFittingMinutesIntoWeekMonSunIntervalCurrentTimeBeforeNodeTimeThu)
{
  dt.weekDay = WeekDay::THURSDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min till wednesday
  CHECK_EQUAL(24 * 2 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeMonSunTestBeforeNodeTime, testgetBestFittingMinutesIntoWeekMonSunIntervalCurrentTimeBeforeNodeTimeFri)
{
  dt.weekDay = WeekDay::FRIDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min till thursday
  CHECK_EQUAL(24 * 3 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeMonSunTestBeforeNodeTime, testgetBestFittingMinutesIntoWeekMonSunIntervalCurrentTimeBeforeNodeTimeSat)
{
  dt.weekDay = WeekDay::SATURDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min till friday
  CHECK_EQUAL(24 * 4 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNodeMonSunTestBeforeNodeTime, testgetBestFittingMinutesIntoWeekMonSunIntervalCurrentTimeBeforeNodeTimeSun)
{
  dt.weekDay = WeekDay::SUNDAY;
  uint16_t minutes = node.getBestFittingMinutesIntoWeek(dt); // returns num of min till saturday
  CHECK_EQUAL(24 * 5 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNode, testgetNumOfMinIntoWeekForThisNodeSingleDaysTimeBeforeNodeTime)
{
  DateTime dt =
  {
    0,
    0,
    0,
    0,
    0,
    40,
    WeekDay::MONDAY
  };
  WeekProgramNode node;
  node.minutes = 15u;
  node.hours = 13u;
  node.interval = Interval::INTERVAL_MON;
  node.status = HeatingMode::comfort;

  uint16_t minutes = node.getNumOfMinIntoWeekForThisNode(dt); // Will return the num of minutes from monday 00:00 till the node's starting time.
    CHECK_EQUAL(24 * 0 + 13 * 60 + 15, minutes);

  node.interval = Interval::INTERVAL_TUE;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // Will return the num of minutes from monday 00:00 till the node's starting time.
    CHECK_EQUAL(24 * 1 * 60 + 13 * 60 + 15, minutes);

  node.interval = Interval::INTERVAL_WED;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // Will return the num of minutes from monday 00:00 till the node's starting time.
    CHECK_EQUAL(24 * 2 * 60 + 13 * 60 + 15, minutes);

  node.interval = Interval::INTERVAL_THU;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // Will return the num of minutes from monday 00:00 till the node's starting time.
    CHECK_EQUAL(24 * 3 * 60 + 13 * 60 + 15, minutes);

  node.interval = Interval::INTERVAL_FRI;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // Will return the num of minutes from monday 00:00 till the node's starting time.
    CHECK_EQUAL(24 * 4 * 60 + 13 * 60 + 15, minutes);

  node.interval = Interval::INTERVAL_SAT;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // Will return the num of minutes from monday 00:00 till the node's starting time.
    CHECK_EQUAL(24 * 5 * 60 + 13 * 60 + 15, minutes);

  node.interval = Interval::INTERVAL_SUN;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // Will return the num of minutes from monday 00:00 till the node's starting time.
    CHECK_EQUAL(24 * 6 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNode, testgetNumOfMinIntoWeekForThisNodeSingleDaysTimePastNodeTime)
{
  // Even if the time has passed the node's starting time, we will still get num of min from mon 00:00 till node's starting time
  DateTime dt =
  {
    0,
    0,
    0,
    14,
    15,
    40,
    WeekDay::MONDAY
  };
  WeekProgramNode node;
  node.minutes = 15u;
  node.hours = 13u;
  node.interval = Interval::INTERVAL_MON;
  node.status = HeatingMode::comfort;

  uint16_t minutes = node.getNumOfMinIntoWeekForThisNode(dt); // Will return the num of minutes from monday 00:00 till the node's starting time.
  CHECK_EQUAL(24 * 0 + 13 * 60 + 15, minutes);

  node.interval = Interval::INTERVAL_TUE;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // Will return the num of minutes from monday 00:00 till the node's starting time.
  CHECK_EQUAL(24 * 1 * 60 + 13 * 60 + 15, minutes);

  node.interval = Interval::INTERVAL_WED;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // Will return the num of minutes from monday 00:00 till the node's starting time.
  CHECK_EQUAL(24 * 2 * 60 + 13 * 60 + 15, minutes);

  node.interval = Interval::INTERVAL_THU;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // Will return the num of minutes from monday 00:00 till the node's starting time.
  CHECK_EQUAL(24 * 3 * 60 + 13 * 60 + 15, minutes);

  node.interval = Interval::INTERVAL_FRI;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // Will return the num of minutes from monday 00:00 till the node's starting time.
  CHECK_EQUAL(24 * 4 * 60 + 13 * 60 + 15, minutes);

  node.interval = Interval::INTERVAL_SAT;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // Will return the num of minutes from monday 00:00 till the node's starting time.
  CHECK_EQUAL(24 * 5 * 60 + 13 * 60 + 15, minutes);

  node.interval = Interval::INTERVAL_SUN;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // Will return the num of minutes from monday 00:00 till the node's starting time.
  CHECK_EQUAL(24 * 6 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNode, testgetNumOfMinIntoWeekForThisSatSunIntervalCurrentTimeBeforeNodeTime)
{
  //hours and minutes are earlier than node's starting time, days will vary
  DateTime dt =
  {
    0,
    0,
    0,
    0,
    0,
    40,
    WeekDay::MONDAY
  };
  WeekProgramNode node;
  node.minutes = 15u;
  node.hours = 13u;
  node.interval = Interval::INTERVAL_SATSUN;
  node.status = HeatingMode::comfort;

  uint16_t minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till saturday
    CHECK_EQUAL(24 * 5 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::TUESDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till saturday
    CHECK_EQUAL(24 * 5 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::WEDNESDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till saturday
    CHECK_EQUAL(24 * 5 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::THURSDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till saturday
    CHECK_EQUAL(24 * 5 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::FRIDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till saturday
    CHECK_EQUAL(24 * 5 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::SATURDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till saturday
    CHECK_EQUAL(24 * 5 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::SUNDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till sunday
    CHECK_EQUAL(24 * 6 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNode, testgetNumOfMinIntoWeekForThisSatSunIntervalCurrentTimeAfterNodeTime)
{
  //hours and minutes are after node's starting time, days will vary
  DateTime dt =
  {
    0,
    0,
    0,
    14,
    5,
    40,
    WeekDay::MONDAY
  };
  WeekProgramNode node;
  node.minutes = 15u;
  node.hours = 13u;
  node.interval = Interval::INTERVAL_SATSUN;
  node.status = HeatingMode::comfort;

  uint16_t minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till saturday
  CHECK_EQUAL(24 * 5 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::TUESDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till saturday
  CHECK_EQUAL(24 * 5 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::WEDNESDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till saturday
  CHECK_EQUAL(24 * 5 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::THURSDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till saturday
  CHECK_EQUAL(24 * 5 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::FRIDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till saturday
  CHECK_EQUAL(24 * 5 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::SATURDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till sunday
  CHECK_EQUAL(24 * 6 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::SUNDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till saturday
  CHECK_EQUAL(24 * 5 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNode, testgetNumOfMinIntoWeekForThisMonFriIntervalCurrentTimeAfterNodeTime)
{
  //hours and minutes are after node's starting time, days will vary
  DateTime dt =
  {
    0,
    0,
    0,
    14,
    5,
    40,
    WeekDay::MONDAY
  };
  WeekProgramNode node;
  node.minutes = 15u;
  node.hours = 13u;
  node.interval = Interval::INTERVAL_MONFRI;
  node.status = HeatingMode::comfort;

  uint16_t minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till tuesday
  CHECK_EQUAL(24 * 1 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::TUESDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till wednesday
  CHECK_EQUAL(24 * 2 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::WEDNESDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till thursday
  CHECK_EQUAL(24 * 3 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::THURSDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till friday
  CHECK_EQUAL(24 * 4 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::FRIDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till monday
  CHECK_EQUAL(24 * 0 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::SATURDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till monday
  CHECK_EQUAL(24 * 0 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::SUNDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till monday
  CHECK_EQUAL(24 * 0 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNode, testgetNumOfMinIntoWeekForThisMonFriIntervalCurrentTimeBeforeNodeTime)
{
  //hours and minutes are before node's starting time, days will vary
  DateTime dt =
  {
    0,
    0,
    0,
    0,
    5,
    40,
    WeekDay::MONDAY
  };
  WeekProgramNode node;
  node.minutes = 15u;
  node.hours = 13u;
  node.interval = Interval::INTERVAL_MONFRI;
  node.status = HeatingMode::comfort;

  uint16_t minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till monday
  CHECK_EQUAL(24 * 0 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::TUESDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till tuesday
  CHECK_EQUAL(24 * 1 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::WEDNESDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till wednesday
  CHECK_EQUAL(24 * 2 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::THURSDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till thursday
  CHECK_EQUAL(24 * 3 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::FRIDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till friday
  CHECK_EQUAL(24 * 4 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::SATURDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till monday
  CHECK_EQUAL(24 * 0 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::SUNDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till monday
  CHECK_EQUAL(24 * 0 * 60 + 13 * 60 + 15, minutes);
}


TEST(WeekProgramNode, testgetNumOfMinIntoWeekForThisMonSunIntervalCurrentTimeAfterNodeTime)
{
  //hours and minutes are after node's starting time, days will vary
  DateTime dt =
  {
    0,
    0,
    0,
    14,
    5,
    40,
    WeekDay::MONDAY
  };
  WeekProgramNode node;
  node.minutes = 15u;
  node.hours = 13u;
  node.interval = Interval::INTERVAL_MONSUN;
  node.status = HeatingMode::comfort;

  uint16_t minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till tuesday
  CHECK_EQUAL(24 * 1 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::TUESDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till wednesday
  CHECK_EQUAL(24 * 2 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::WEDNESDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt);// returns num of min till thursday
  CHECK_EQUAL(24 * 3 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::THURSDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt);// returns num of min till friday
  CHECK_EQUAL(24 * 4 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::FRIDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt);// returns num of min till monday
  CHECK_EQUAL(24 * 5 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::SATURDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt);// returns num of min till monday
  CHECK_EQUAL(24 * 6 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::SUNDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt);// returns num of min till monday
  CHECK_EQUAL(24 * 0 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNode, testgetNumOfMinIntoWeekForThisMonSunIntervalCurrentTimeBeforeNodeTime)
{
  //hours and minutes are before node's starting time, days will vary
  DateTime dt =
  {
    0,
    0,
    0,
    0,
    5,
    40,
    WeekDay::MONDAY
  };
  WeekProgramNode node;
  node.minutes = 15u;
  node.hours = 13u;
  node.interval = Interval::INTERVAL_MONSUN;
  node.status = HeatingMode::comfort;

  uint16_t minutes = node.getNumOfMinIntoWeekForThisNode(dt); //returns num of min till monday
  CHECK_EQUAL(24 * 0 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::TUESDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till tuesday
  CHECK_EQUAL(24 * 1 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::WEDNESDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); //returns num of min till wednesday
  CHECK_EQUAL(24 * 2 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::THURSDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till thursday
  CHECK_EQUAL(24 * 3 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::FRIDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till friday
  CHECK_EQUAL(24 * 4 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::SATURDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till saturday
  CHECK_EQUAL(24 * 5 * 60 + 13 * 60 + 15, minutes);

  dt.weekDay = WeekDay::SUNDAY;
  minutes = node.getNumOfMinIntoWeekForThisNode(dt); // returns num of min till sunday
  CHECK_EQUAL(24 * 6 * 60 + 13 * 60 + 15, minutes);
}

TEST(WeekProgramNode, testOperatorNotEqual)
{
  WeekProgramNode a = { Interval::INTERVAL_MONSUN, HeatingMode::comfort, 1, 4 };
  WeekProgramNode c = { Interval::INTERVAL_MONSUN, HeatingMode::comfort, 1, 4 };
  WeekProgramNode b = { Interval::INTERVAL_MONSUN, HeatingMode::eco, 1, 4 };
  CHECK_TRUE(a != b);
  CHECK_TRUE(c != b);
  CHECK_FALSE(a != c);
}
