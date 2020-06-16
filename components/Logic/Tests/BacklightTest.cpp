#include "CppUTest\TestHarness.h"
#include "CppUTestExt\MockSupport.h"

#include "Constants.h"
#include "Backlight.h"

class BacklightDriverMock: public BacklightDriverInterface
{
public:
  void setBacklightLevel(uint8_t level)
  {
    mock().actualCall("setBacklightLevel").withIntParameter("level", level);
  }
};

static BacklightDriverMock backlightDriverMock;
static Backlight *backlight;

TEST_GROUP(backlight)
{
  TEST_SETUP()
  {
    backlight = new Backlight(0U, backlightDriverMock);
  }

  TEST_TEARDOWN()
  {
    delete backlight;
    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
  }
};

TEST(backlight, testThatScreenIsFullyLitAtStartup)
{
  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_MAX_VALUE);
  bool isScreenDimmed = backlight->dim(false, 0U, BacklightOptions::Normal);
  CHECK_FALSE(isScreenDimmed);
  CHECK_FALSE(backlight->isScreenFullyDimmed());
}

TEST(backlight, testThatScreenIsFullyDimmedAfterInactivity)
{
  
  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_MAX_VALUE);
  bool isScreenDimmed = backlight->dim(false, 1U, BacklightOptions::Normal);
  CHECK_FALSE(isScreenDimmed);
  CHECK_FALSE(backlight->isScreenFullyDimmed());

  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_NORMAL_VALUE);
  isScreenDimmed = backlight->dim(false, Constants::DIM_TIMEOUT_MS + Constants::DIM_FADE_OUT_TIME_MS + 1U, BacklightOptions::Normal);
  CHECK_TRUE(isScreenDimmed);
  CHECK_TRUE(backlight->isScreenFullyDimmed());
}


TEST(backlight, testThatScreenIsFullyLitAfterAWhileWhenActive)
{
  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_MAX_VALUE);
  bool isScreenDimmed = backlight->dim(false, 1U, BacklightOptions::Normal);
  CHECK_FALSE(isScreenDimmed);

  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_MAX_VALUE);
  isScreenDimmed = backlight->dim(true, Constants::DIM_TIMEOUT_MS + Constants::DIM_FADE_OUT_TIME_MS + 1U, BacklightOptions::Normal);
  CHECK_FALSE(isScreenDimmed);
}


TEST(backlight, test50PercentFadeOutWhenItIsCurrentlyDimming)
{
  
  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_MAX_VALUE);
  bool isScreenDimmed = backlight->dim(false, 1U, BacklightOptions::Normal);
  CHECK_FALSE(isScreenDimmed);

  mock().expectOneCall("setBacklightLevel").withIntParameter("level", (Constants::DIM_MAX_VALUE - Constants::DIM_NORMAL_VALUE) / 2 + Constants::DIM_NORMAL_VALUE);
  isScreenDimmed = backlight->dim(false, Constants::DIM_TIMEOUT_MS + Constants::DIM_FADE_OUT_TIME_MS / 2U + 1, BacklightOptions::Normal);
  CHECK_FALSE(isScreenDimmed);
}

TEST(backlight, testFullFadeOutPastFadeOutTime)
{
  
  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_MAX_VALUE);
  bool isScreenDimmed = backlight->dim(false, 1U, BacklightOptions::Normal);
  CHECK_FALSE(isScreenDimmed);

  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_NORMAL_VALUE);
  isScreenDimmed = backlight->dim(false, Constants::DIM_TIMEOUT_MS + Constants::DIM_FADE_OUT_TIME_MS + 2000U, BacklightOptions::Normal);
  CHECK_TRUE(isScreenDimmed);
}

TEST(backlight, testFullFadeOutPastFadeOutTimeAndFullLitAgainWhenActive)
{
  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_MAX_VALUE);
  bool isScreenDimmed = backlight->dim(false, 1U, BacklightOptions::Normal);
  CHECK_FALSE(isScreenDimmed);

  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_NORMAL_VALUE);
  isScreenDimmed = backlight->dim(false, Constants::DIM_TIMEOUT_MS + Constants::DIM_FADE_OUT_TIME_MS + 2001U, BacklightOptions::Normal);
  CHECK_TRUE(isScreenDimmed);

  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_NORMAL_VALUE);
  isScreenDimmed = backlight->dim(true, Constants::DIM_TIMEOUT_MS + Constants::DIM_FADE_OUT_TIME_MS + 12001U, BacklightOptions::Normal);
  CHECK_TRUE(isScreenDimmed);

  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_MAX_VALUE);
  isScreenDimmed = backlight->dim(true, Constants::DIM_TIMEOUT_MS + Constants::DIM_FADE_OUT_TIME_MS + 12001U + Constants::DIM_FADE_IN_TIME_MS, BacklightOptions::Normal);
  CHECK_FALSE(isScreenDimmed);
}


TEST(backlight, testFadeInHalfAfterInactivity)
{
  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_MAX_VALUE);
  bool isScreenDimmed = backlight->dim(false, 1U, BacklightOptions::Normal);
  CHECK_FALSE(isScreenDimmed);

  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_NORMAL_VALUE);
  isScreenDimmed = backlight->dim(false, Constants::DIM_TIMEOUT_MS + Constants::DIM_FADE_OUT_TIME_MS + 1U, BacklightOptions::Normal);
  CHECK_TRUE(isScreenDimmed);

  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_NORMAL_VALUE);
  isScreenDimmed = backlight->dim(true, Constants::DIM_TIMEOUT_MS + Constants::DIM_FADE_OUT_TIME_MS + 2001U, BacklightOptions::Normal);
  CHECK_TRUE(isScreenDimmed);

  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_NORMAL_VALUE + (Constants::DIM_MAX_VALUE-Constants::DIM_NORMAL_VALUE)/2U);
  isScreenDimmed = backlight->dim(false, Constants::DIM_TIMEOUT_MS + Constants::DIM_FADE_OUT_TIME_MS + 2001U + Constants::DIM_FADE_IN_TIME_MS / 2U, BacklightOptions::Normal);
  CHECK_FALSE(isScreenDimmed);
}


TEST(backlight, testFadeInHalfAfterInactivityAndButtonPressedAgain)
{
  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_MAX_VALUE);
  bool isScreenDimmed = backlight->dim(false, 1U, BacklightOptions::Normal);
  CHECK_FALSE(isScreenDimmed);

  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_NORMAL_VALUE);
  isScreenDimmed = backlight->dim(false, Constants::DIM_TIMEOUT_MS + Constants::DIM_FADE_OUT_TIME_MS + 1U, BacklightOptions::Normal);
  CHECK_TRUE(isScreenDimmed);

  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_NORMAL_VALUE);
  isScreenDimmed = backlight->dim(true, Constants::DIM_TIMEOUT_MS + Constants::DIM_FADE_OUT_TIME_MS + 2001U, BacklightOptions::Normal);
  CHECK_TRUE(isScreenDimmed);

  mock().expectOneCall("setBacklightLevel").withIntParameter("level", (Constants::DIM_MAX_VALUE - Constants::DIM_NORMAL_VALUE) / 2U + Constants::DIM_NORMAL_VALUE);
  isScreenDimmed = backlight->dim(true, Constants::DIM_TIMEOUT_MS + Constants::DIM_FADE_OUT_TIME_MS + 2001U + Constants::DIM_FADE_IN_TIME_MS / 2U, BacklightOptions::Normal);
  CHECK_FALSE(isScreenDimmed);
}

TEST(backlight,testFullyLitBeforeDimInactivityHappens)
{
  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_MAX_VALUE);
  bool isScreenDimmed = backlight->dim(false, 1U, BacklightOptions::Normal);
  CHECK_FALSE(isScreenDimmed);

  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_MAX_VALUE);
  isScreenDimmed = backlight->dim(false, Constants::DIM_TIMEOUT_MS + 1U, BacklightOptions::Normal);
  CHECK_FALSE(isScreenDimmed);
}

TEST(backlight,testFadeInWhileFadingOut)
{
  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_MAX_VALUE);
  bool isScreenDimmed = backlight->dim(false, 1U, BacklightOptions::Normal);
  CHECK_FALSE(isScreenDimmed);

  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_MAX_VALUE);
  isScreenDimmed = backlight->dim(false, Constants::DIM_TIMEOUT_MS + 1U, BacklightOptions::Normal);
  CHECK_FALSE(isScreenDimmed);
  
  mock().expectOneCall("setBacklightLevel").withIntParameter("level", (Constants::DIM_MAX_VALUE - Constants::DIM_NORMAL_VALUE) / 2U + Constants::DIM_NORMAL_VALUE);
  isScreenDimmed = backlight->dim(false, Constants::DIM_TIMEOUT_MS + Constants::DIM_FADE_OUT_TIME_MS / 2U + 1U, BacklightOptions::Normal);
  CHECK_FALSE(isScreenDimmed);
  
  mock().expectOneCall("setBacklightLevel").withIntParameter("level", (Constants::DIM_MAX_VALUE - Constants::DIM_NORMAL_VALUE) / 2U + Constants::DIM_NORMAL_VALUE);
  isScreenDimmed = backlight->dim(true, Constants::DIM_TIMEOUT_MS + Constants::DIM_FADE_OUT_TIME_MS / 2U + 1U, BacklightOptions::Normal);
  CHECK_FALSE(isScreenDimmed);
  
  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_MAX_VALUE);
  isScreenDimmed = backlight->dim(true, Constants::DIM_TIMEOUT_MS + Constants::DIM_FADE_OUT_TIME_MS / 2U + 1U + Constants::DIM_FADE_IN_TIME_MS, BacklightOptions::Normal);
  CHECK_FALSE(isScreenDimmed);
}

TEST(backlight, testFadeToDark)
{
  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_MAX_VALUE);
  bool isScreenDimmed = backlight->dim(false, 1U, BacklightOptions::Dark);
  CHECK_FALSE(isScreenDimmed);

  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_MAX_VALUE);
  isScreenDimmed = backlight->dim(false, Constants::DIM_TIMEOUT_MS + 1U, BacklightOptions::Dark);
  CHECK_FALSE(isScreenDimmed);
  
  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_DARK_VALUE);
  isScreenDimmed = backlight->dim(false, Constants::DIM_TIMEOUT_MS + Constants::DIM_FADE_OUT_TIME_MS + 1U, BacklightOptions::Dark);
  CHECK_TRUE(isScreenDimmed);
 
}

TEST(backlight, testFadeToBright)
{
  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_MAX_VALUE);
  bool isScreenDimmed = backlight->dim(false, 1U, BacklightOptions::Bright);
  CHECK_FALSE(isScreenDimmed);

  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_MAX_VALUE);
  isScreenDimmed = backlight->dim(false, Constants::DIM_TIMEOUT_MS + 1U, BacklightOptions::Bright);
  CHECK_FALSE(isScreenDimmed);

  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_BRIGHT_VALUE);
  isScreenDimmed = backlight->dim(false, Constants::DIM_TIMEOUT_MS + Constants::DIM_FADE_OUT_TIME_MS + 1U, BacklightOptions::Bright);
  CHECK_TRUE(isScreenDimmed);

}

TEST(backlight, testFadeToBrightThenToDark)
{
  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_MAX_VALUE);
  bool isScreenDimmed = backlight->dim(false, 1U, BacklightOptions::Bright);
  CHECK_FALSE(isScreenDimmed);

  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_MAX_VALUE);
  isScreenDimmed = backlight->dim(false, Constants::DIM_TIMEOUT_MS + 1U, BacklightOptions::Bright);
  CHECK_FALSE(isScreenDimmed);

  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_BRIGHT_VALUE);
  isScreenDimmed = backlight->dim(false, Constants::DIM_TIMEOUT_MS + Constants::DIM_FADE_OUT_TIME_MS + 1U, BacklightOptions::Bright);
  CHECK_TRUE(isScreenDimmed);

  // If the brightness is somehow changed when the display is already dimmed, then the change will happen instantly.
  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_DARK_VALUE);
  isScreenDimmed = backlight->dim(false, Constants::DIM_TIMEOUT_MS + Constants::DIM_FADE_OUT_TIME_MS + 4U, BacklightOptions::Dark);
  CHECK_TRUE(isScreenDimmed);
}

TEST(backlight, testReturnFalseWhenMaxBrightness)
{
  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_MAX_VALUE);
  bool isScreenDimmed = backlight->dim(false, 1U, BacklightOptions::Max);
  CHECK_FALSE(isScreenDimmed);

  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_MAX_VALUE);
  isScreenDimmed = backlight->dim(false, Constants::DIM_TIMEOUT_MS + 1U, BacklightOptions::Max);
  CHECK_FALSE(isScreenDimmed);

  mock().expectOneCall("setBacklightLevel").withIntParameter("level", Constants::DIM_MAX_VALUE);
  isScreenDimmed = backlight->dim(false, Constants::DIM_TIMEOUT_MS + Constants::DIM_FADE_OUT_TIME_MS + 1U, BacklightOptions::Max);
  CHECK_FALSE(isScreenDimmed); 

}
