#include "CppUTest\TestHarness.h"
#include "CppUTestExt\MockSupport.h"

#include "LinearInterpolator.h"

TEST_GROUP(LinearInterpolator)
{
  TEST_TEARDOWN()
  {
    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
  }
};

TEST(LinearInterpolator, testAtLowerBound)
{
  int32_t xValues[] = { 1, 13 };
  int32_t yValues[] = { 10, 130 };
  uint8_t numPairs = sizeof(xValues) / sizeof(xValues[0]);

  LinearInterpolator<int32_t, int32_t> li = LinearInterpolator<int32_t, int32_t>(xValues, yValues, numPairs);

  int32_t y = 0;
  
  bool result = li.interpolate(1, y);

  CHECK_TRUE(result);
  CHECK_EQUAL(10, y);
}

TEST(LinearInterpolator, testBelowLowerBound)
{
  int32_t xValues[] = { 1, 13 };
  int32_t yValues[] = { 10, 130 };
  uint8_t numPairs = sizeof(xValues) / sizeof(xValues[0]);

  LinearInterpolator<int32_t, int32_t> li = LinearInterpolator<int32_t, int32_t>(xValues, yValues, numPairs);

  int32_t y = 0;

  bool result = li.interpolate(0, y);

  CHECK_FALSE(result);
  CHECK_EQUAL(10, y);
}

TEST(LinearInterpolator, testAtUpperBound)
{
  int32_t xValues[] = { 1, 13 };
  int32_t yValues[] = { 10, 130 };
  uint8_t numPairs = sizeof(xValues) / sizeof(xValues[0]);

  LinearInterpolator<int32_t, int32_t> li = LinearInterpolator<int32_t, int32_t>(xValues, yValues, numPairs);

  int32_t y = 0;

  bool result = li.interpolate(13, y);

  CHECK_TRUE(result);
  CHECK_EQUAL(130, y);
}

TEST(LinearInterpolator, testAboveUpperBound)
{
  int32_t xValues[] = { 1, 13 };
  int32_t yValues[] = { 10, 130 };
  uint8_t numPairs = sizeof(xValues) / sizeof(xValues[0]);

  LinearInterpolator<int32_t, int32_t> li = LinearInterpolator<int32_t, int32_t>(xValues, yValues, numPairs);

  int32_t y = 0;

  bool result = li.interpolate(14, y);

  CHECK_FALSE(result);
  CHECK_EQUAL(130, y);
}

TEST(LinearInterpolator, testAtMidPoint)
{
  int32_t xValues[] = { 1, 13 };
  int32_t yValues[] = { 10, 130 };
  uint8_t numPairs = sizeof(xValues) / sizeof(xValues[0]);

  LinearInterpolator<int32_t, int32_t> li = LinearInterpolator<int32_t, int32_t>(xValues, yValues, numPairs);

  int32_t y = 0;

  bool result = li.interpolate(7, y);

  CHECK_TRUE(result);
  CHECK_EQUAL(70, y);
}

TEST(LinearInterpolator, testAtTwoThirds)
{
  int32_t xValues[] = { 1, 4 };
  int32_t yValues[] = { 10, 40 };
  uint8_t numPairs = sizeof(xValues) / sizeof(xValues[0]);

  LinearInterpolator<int32_t, int32_t> li = LinearInterpolator<int32_t, int32_t>(xValues, yValues, numPairs);

  int32_t y = 0;

  bool result = li.interpolate(3, y);

  CHECK_TRUE(result);
  CHECK_EQUAL(30, y);
}

TEST(LinearInterpolator, testAtMidPointExact)
{
  int32_t xValues[] = { 1, 7, 13 };
  int32_t yValues[] = { 10, 135, 260 };
  uint8_t numPairs = sizeof(xValues) / sizeof(xValues[0]);

  LinearInterpolator<int32_t, int32_t> li = LinearInterpolator<int32_t, int32_t>(xValues, yValues, numPairs);

  int32_t y = 0;

  bool result = li.interpolate(7, y);

  CHECK_TRUE(result);
  CHECK_EQUAL(135, y);
}
