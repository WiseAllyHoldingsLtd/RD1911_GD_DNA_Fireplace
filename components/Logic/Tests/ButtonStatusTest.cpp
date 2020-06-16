#include "CppUTest\TestHarness.h"
#include "CppUTestExt\MockSupport.h"

#include "ButtonStatus.h"

TEST_GROUP(buttonStatus)
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

TEST(buttonStatus, testEmptyButtonStatusGivesNoActivity)
{
  ButtonStatus buttonStatus = { 0 };
  CHECK_FALSE(buttonStatus.isButtonActivity());
}

TEST(buttonStatus, testButton0PressedAndReleasedActivity)
{
  ButtonStatus buttonStatus = { 0 };
  buttonStatus.wasButtonPressedAndReleased[0] = true;
  CHECK_TRUE(buttonStatus.isButtonActivity());
}

TEST(buttonStatus, testButton2PressedAndReleasedActivity)
{
  ButtonStatus buttonStatus = { 0 };
  buttonStatus.wasButtonPressedAndReleased[2] = true;
  CHECK_TRUE(buttonStatus.isButtonActivity());
}

TEST(buttonStatus, testButton2PressedAndHeldActivity)
{
  ButtonStatus buttonStatus = { 0 };
  buttonStatus.isButtonHeld[2] = true;
  CHECK_TRUE(buttonStatus.isButtonActivity());
}

TEST(buttonStatus, testButton1PressedAndHeldActivity)
{
  ButtonStatus buttonStatus = { 0 };
  buttonStatus.isButtonHeld[1] = true;
  CHECK_TRUE(buttonStatus.isButtonActivity());
}

TEST(buttonStatus, testAnyButtonPressedAndReleasedTrueButton1)
{
  ButtonStatus buttonStatus = { 0 };
  buttonStatus.wasButtonPressedAndReleased[1] = true;
  CHECK_TRUE(buttonStatus.isAnyButtonPressedAndReleased());
}

TEST(buttonStatus, testAnyButtonPressedAndReleasedTrueButton2)
{
  ButtonStatus buttonStatus = { 0 };
  buttonStatus.wasButtonPressedAndReleased[2] = true;
  CHECK_TRUE(buttonStatus.isAnyButtonPressedAndReleased());
}


TEST(buttonStatus, testAnyButtonPressedAndReleasedFalse)
{
  ButtonStatus buttonStatus = { 0 };
  CHECK_FALSE(buttonStatus.isAnyButtonPressedAndReleased());
}


TEST(buttonStatus, testAnyButtonPressedAndReleasedFalseWhenButtonHeld)
{
  ButtonStatus buttonStatus = { 0 };
  buttonStatus.isButtonHeld[1] = true;
  CHECK_FALSE(buttonStatus.isAnyButtonPressedAndReleased());
}
