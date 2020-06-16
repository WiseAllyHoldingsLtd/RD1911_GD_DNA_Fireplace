#include "CppUTest\TestHarness.h"
#include "CppUTestExt\MockSupport.h"

#include "Constants.h"
#include "ButtonDriver.h"
#include "QTouchDriverMock.h"
#include "TimerDriverMock.h"

const bool isButtonTouchedYes0[NUMBER_OF_BUTTONS] = { true, false, false};
const bool isButtonTouchedYes2[NUMBER_OF_BUTTONS] = { false, false, true };
const bool isButtonTouchedYes0And2[NUMBER_OF_BUTTONS] = { true, false, true };
const bool isButtonTouchedAll[NUMBER_OF_BUTTONS] = { true, true, true };
const bool isButtonTouchedNone[NUMBER_OF_BUTTONS] = { false, false, false };

QTouchDriverMock touchDriverMock;
ButtonDriver *buttonDriver;
TimerDriverDummy timerDummy(0, 10);

TEST_GROUP(buttonDriver)
{
  TEST_SETUP()
  {
    buttonDriver = new ButtonDriver(touchDriverMock, timerDummy);
  }

  TEST_TEARDOWN()
  {
    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();

    delete buttonDriver;
  }
};

TEST(buttonDriver, testEmptyButtonStatusInitially)
{
  ButtonStatus actualButtonStatus = buttonDriver->getButtonStatus();

  ButtonStatus expectedButtonStatus = { 0 };
  CHECK_TRUE(0 == memcmp(&expectedButtonStatus, &actualButtonStatus, sizeof(expectedButtonStatus)));
}


TEST(buttonDriver, testButton0PressedFirstSighting)
{
  touchDriverMock.setTouchReturnValue(isButtonTouchedYes0);
  buttonDriver->update();

  ButtonStatus actualButtonStatus = buttonDriver->getButtonStatus();

  ButtonStatus expectedButtonStatus = { 0 };
  CHECK_EQUAL(0, memcmp(&expectedButtonStatus, &actualButtonStatus, sizeof(expectedButtonStatus)));
}


TEST(buttonDriver, testButton0PressedAndNotReleasedYet)
{
  TimerDriverDummy timerDummy(0, 10);
  touchDriverMock.setTouchReturnValue(isButtonTouchedYes0);
  buttonDriver->update();
  buttonDriver->update();
  buttonDriver->update();
  ButtonStatus actualButtonStatus = buttonDriver->getButtonStatus();

  ButtonStatus expectedButtonStatus = { 0 };
  CHECK_EQUAL(0, memcmp(&expectedButtonStatus, &actualButtonStatus, sizeof(expectedButtonStatus)));
}

TEST(buttonDriver, testButton0PressedAndReleasedLowLimit)
{
  timerDummy.setNewIncrementValue(1);
  touchDriverMock.setTouchReturnValue(isButtonTouchedYes0);
  buttonDriver->update();
  touchDriverMock.setTouchReturnValue(isButtonTouchedNone);
  buttonDriver->update();
  ButtonStatus actualButtonStatus = buttonDriver->getButtonStatus();

  ButtonStatus expectedButtonStatus = { 0 };
  expectedButtonStatus.wasButtonPressedAndReleased[0] = true;
  CHECK_EQUAL(0, memcmp(&expectedButtonStatus, &actualButtonStatus, sizeof(expectedButtonStatus)));
}

TEST(buttonDriver, testButton0PressedAndReleased)
{
  timerDummy.setNewIncrementValue(100);
  touchDriverMock.setTouchReturnValue(isButtonTouchedYes0);
  buttonDriver->update();
  buttonDriver->update();
  touchDriverMock.setTouchReturnValue(isButtonTouchedNone);
  buttonDriver->update();
  ButtonStatus actualButtonStatus = buttonDriver->getButtonStatus();

  ButtonStatus expectedButtonStatus = { 0 };
  expectedButtonStatus.wasButtonPressedAndReleased[0] = true;
  CHECK_EQUAL(0, memcmp(&expectedButtonStatus, &actualButtonStatus, sizeof(expectedButtonStatus)));
}

TEST(buttonDriver, testButton2PressedAndReleased)
{
  timerDummy.setNewIncrementValue(100);
  touchDriverMock.setTouchReturnValue(isButtonTouchedYes2);
  buttonDriver->update();
  buttonDriver->update();
  touchDriverMock.setTouchReturnValue(isButtonTouchedNone);
  buttonDriver->update();
  ButtonStatus actualButtonStatus = buttonDriver->getButtonStatus();

  ButtonStatus expectedButtonStatus = { 0 };
  expectedButtonStatus.wasButtonPressedAndReleased[2] = true;
  CHECK_EQUAL(0, memcmp(&expectedButtonStatus, &actualButtonStatus, sizeof(expectedButtonStatus)));
}


TEST(buttonDriver, testAllButtonsPressedAndReleased)
{
  timerDummy.setNewIncrementValue(100);
  touchDriverMock.setTouchReturnValue(isButtonTouchedAll);
  buttonDriver->update();
  touchDriverMock.setTouchReturnValue(isButtonTouchedNone);
  buttonDriver->update();
  ButtonStatus actualButtonStatus = buttonDriver->getButtonStatus();

  ButtonStatus expectedButtonStatus = { 0 };
  expectedButtonStatus.wasButtonPressedAndReleased[0] = true;
  expectedButtonStatus.wasButtonPressedAndReleased[1] = true;
  expectedButtonStatus.wasButtonPressedAndReleased[2] = true;
  CHECK_EQUAL(0, memcmp(&expectedButtonStatus, &actualButtonStatus, sizeof(expectedButtonStatus)));
}


TEST(buttonDriver, testButton0PressedThen2Pressed)
{
  touchDriverMock.setTouchReturnValue(isButtonTouchedYes0);
  buttonDriver->update();
  touchDriverMock.setTouchReturnValue(isButtonTouchedYes0And2);
  buttonDriver->update();
  touchDriverMock.setTouchReturnValue(isButtonTouchedYes2);
  buttonDriver->update();

  ButtonStatus actualButtonStatus = buttonDriver->getButtonStatus();

  ButtonStatus expectedButtonStatus = { 0 };
  expectedButtonStatus.wasButtonPressedAndReleased[0] = true;
  CHECK_EQUAL(0, memcmp(&expectedButtonStatus, &actualButtonStatus, sizeof(expectedButtonStatus)));

  touchDriverMock.setTouchReturnValue(isButtonTouchedNone);
  buttonDriver->update();
  actualButtonStatus = buttonDriver->getButtonStatus();

  expectedButtonStatus.wasButtonPressedAndReleased[2] = true;
  CHECK_EQUAL(0, memcmp(&expectedButtonStatus, &actualButtonStatus, sizeof(expectedButtonStatus)));
}


TEST(buttonDriver, testAllButtonsPressedThenReset)
{
  touchDriverMock.setTouchReturnValue(isButtonTouchedAll);
  buttonDriver->update();
  touchDriverMock.setTouchReturnValue(isButtonTouchedNone);
  buttonDriver->update();
  ButtonStatus actualButtonStatus = buttonDriver->getButtonStatus();

  ButtonStatus expectedButtonStatus = { 0 };
  expectedButtonStatus.wasButtonPressedAndReleased[0] = true;
  expectedButtonStatus.wasButtonPressedAndReleased[1] = true;
  expectedButtonStatus.wasButtonPressedAndReleased[2] = true;
  CHECK_EQUAL(0, memcmp(&expectedButtonStatus, &actualButtonStatus, sizeof(expectedButtonStatus)));

  buttonDriver->resetButtonStatus();
  actualButtonStatus = buttonDriver->getButtonStatus();

  expectedButtonStatus.wasButtonPressedAndReleased[0] = false;
  expectedButtonStatus.wasButtonPressedAndReleased[1] = false;
  expectedButtonStatus.wasButtonPressedAndReleased[2] = false;
  CHECK_EQUAL(0, memcmp(&expectedButtonStatus, &actualButtonStatus, sizeof(expectedButtonStatus)));
}


TEST(buttonDriver, testButton0PressedResetPressedAgain)
{
  touchDriverMock.setTouchReturnValue(isButtonTouchedYes0);
  buttonDriver->update();
  touchDriverMock.setTouchReturnValue(isButtonTouchedNone);
  buttonDriver->update();
  ButtonStatus actualButtonStatus = buttonDriver->getButtonStatus();

  ButtonStatus expectedButtonStatus = { 0 };
  expectedButtonStatus.wasButtonPressedAndReleased[0] = true;
  CHECK_EQUAL(0, memcmp(&expectedButtonStatus, &actualButtonStatus, sizeof(expectedButtonStatus)));

  buttonDriver->resetButtonStatus();
  actualButtonStatus = buttonDriver->getButtonStatus();

  expectedButtonStatus.wasButtonPressedAndReleased[0] = false;
  CHECK_EQUAL(0, memcmp(&expectedButtonStatus, &actualButtonStatus, sizeof(expectedButtonStatus)));

  touchDriverMock.setTouchReturnValue(isButtonTouchedNone);
  buttonDriver->update();
  touchDriverMock.setTouchReturnValue(isButtonTouchedYes0);
  buttonDriver->update();
  touchDriverMock.setTouchReturnValue(isButtonTouchedNone);
  buttonDriver->update();
  actualButtonStatus = buttonDriver->getButtonStatus();

  expectedButtonStatus.wasButtonPressedAndReleased[0] = true;
  CHECK_EQUAL(0, memcmp(&expectedButtonStatus, &actualButtonStatus, sizeof(expectedButtonStatus)));
}


TEST(buttonDriver, testButton0PressedAndReleasedHighLimit)
{
  timerDummy.setNewIncrementValue(0);
  touchDriverMock.setTouchReturnValue(isButtonTouchedYes0);
  buttonDriver->update();
  timerDummy.incrementTime(499);
  buttonDriver->update();
  touchDriverMock.setTouchReturnValue(isButtonTouchedNone);
  timerDummy.incrementTime(10);
  buttonDriver->update();
  ButtonStatus actualButtonStatus = buttonDriver->getButtonStatus();

  ButtonStatus expectedButtonStatus = { 0 };
  expectedButtonStatus.wasButtonPressedAndReleased[0] = true;
  CHECK_EQUAL(0, memcmp(&expectedButtonStatus, &actualButtonStatus, sizeof(expectedButtonStatus)));
}


TEST(buttonDriver, testButton0NotPressedAndReleasedWhenHeldLimitReached)
{
  timerDummy.setNewIncrementValue(0);
  touchDriverMock.setTouchReturnValue(isButtonTouchedYes0);
  buttonDriver->update();
  timerDummy.incrementTime(Constants::BUTTON_HELD_THRESHOLD_MS + 1U);
  buttonDriver->update();
  touchDriverMock.setTouchReturnValue(isButtonTouchedNone);
  timerDummy.incrementTime(10);
  buttonDriver->update();
  ButtonStatus actualButtonStatus = buttonDriver->getButtonStatus();

  ButtonStatus expectedButtonStatus = { 0 };
  expectedButtonStatus.wasButtonPressedAndReleased[0] = false;
  expectedButtonStatus.isButtonHeld[0] = true;
  expectedButtonStatus.buttonHeldDurationMs[0] = Constants::BUTTON_HELD_THRESHOLD_MS + 1U;
  CHECK_EQUAL(0, memcmp(&expectedButtonStatus, &actualButtonStatus, sizeof(expectedButtonStatus)));
}


TEST(buttonDriver, testButton0HeldLowerLimit)
{
  timerDummy.setNewIncrementValue(0);
  touchDriverMock.setTouchReturnValue(isButtonTouchedYes0);
  buttonDriver->update();
  timerDummy.incrementTime(Constants::BUTTON_HELD_THRESHOLD_MS + 1U);
  buttonDriver->update();
  ButtonStatus actualButtonStatus = buttonDriver->getButtonStatus();

  ButtonStatus expectedButtonStatus = { 0 };
  expectedButtonStatus.isButtonHeld[0] = true;
  expectedButtonStatus.isButtonStartedBeingHeld[0] = true;
  expectedButtonStatus.buttonHeldDurationMs[0] = Constants::BUTTON_HELD_THRESHOLD_MS + 1U;
  CHECK_EQUAL(0, memcmp(&expectedButtonStatus, &actualButtonStatus, sizeof(expectedButtonStatus)));
}


TEST(buttonDriver, testButton0ButtonHeldIsRememberedUntilGetButtonStatusIsCalled)
{
  timerDummy.setNewIncrementValue(1);
  touchDriverMock.setTouchReturnValue(isButtonTouchedYes0);
  buttonDriver->update();
  timerDummy.incrementTime(Constants::BUTTON_HELD_THRESHOLD_MS - 1U); // 1 + (X-1) = X
  buttonDriver->update();

  ButtonStatus actualButtonStatus = buttonDriver->getButtonStatus(); //Button is held but less than button hold threshold

  ButtonStatus expectedButtonStatus = { 0 };
  expectedButtonStatus.isButtonStartedBeingHeld[0] = true;
  CHECK_EQUAL(0, memcmp(&expectedButtonStatus, &actualButtonStatus, sizeof(expectedButtonStatus)));

  buttonDriver->update(); //Now we pass the button hold threshold

  touchDriverMock.setTouchReturnValue(isButtonTouchedNone); //Release button

  buttonDriver->update(); //Button is no longer held, but it must be remembered/cached until getButtonStatus() is called once or else this info will be lost.

  actualButtonStatus = buttonDriver->getButtonStatus();

  expectedButtonStatus.isButtonHeld[0] = true;
  expectedButtonStatus.isButtonStartedBeingHeld[0] = false;
  expectedButtonStatus.buttonHeldDurationMs[0] = Constants::BUTTON_HELD_THRESHOLD_MS + 1U;
  CHECK_EQUAL(0, memcmp(&expectedButtonStatus, &actualButtonStatus, sizeof(expectedButtonStatus)));
}


TEST(buttonDriver, testButtonHeldDuration0BeforeHeld)
{
  timerDummy.setNewIncrementValue(50);
  touchDriverMock.setTouchReturnValue(isButtonTouchedYes0);
  buttonDriver->update();
  buttonDriver->update();
  ButtonStatus actualButtonStatus = buttonDriver->getButtonStatus();

  CHECK_EQUAL(0, actualButtonStatus.buttonHeldDurationMs[0]);
}


TEST(buttonDriver, testButtonPressAndReleaseIgnoredWhileOtherButtonIsHeld)
{
  timerDummy.setNewIncrementValue(0);
  touchDriverMock.setTouchReturnValue(isButtonTouchedYes0);
  buttonDriver->update();
  timerDummy.incrementTime(Constants::BUTTON_HELD_THRESHOLD_MS + 1U);
  buttonDriver->update();
  ButtonStatus actualButtonStatus = buttonDriver->getButtonStatus();

  ButtonStatus expectedButtonStatus = { 0 };
  expectedButtonStatus.isButtonHeld[0] = true;
  expectedButtonStatus.isButtonStartedBeingHeld[0] = true;
  expectedButtonStatus.buttonHeldDurationMs[0] = Constants::BUTTON_HELD_THRESHOLD_MS + 1U;
  CHECK_EQUAL(0, memcmp(&expectedButtonStatus, &actualButtonStatus, sizeof(expectedButtonStatus)));

  touchDriverMock.setTouchReturnValue(isButtonTouchedYes0And2);
  timerDummy.incrementTime(1);
  buttonDriver->update();
  touchDriverMock.setTouchReturnValue(isButtonTouchedYes0);
  timerDummy.incrementTime(1);
  buttonDriver->update();
  actualButtonStatus = buttonDriver->getButtonStatus();

  expectedButtonStatus.buttonHeldDurationMs[0] = Constants::BUTTON_HELD_THRESHOLD_MS + 3U;
  CHECK_EQUAL(0, memcmp(&expectedButtonStatus, &actualButtonStatus, sizeof(expectedButtonStatus)));
}


TEST(buttonDriver, testResetButtonStatusWaitsForNoButtonsDown)
{
  timerDummy.setNewIncrementValue(1);
  touchDriverMock.setTouchReturnValue(isButtonTouchedYes0);
  buttonDriver->update();
  touchDriverMock.setTouchReturnValue(isButtonTouchedYes2);
  buttonDriver->update();
  ButtonStatus actualButtonStatus = buttonDriver->getButtonStatus();

  ButtonStatus expectedButtonStatus = { 0 };
  expectedButtonStatus.wasButtonPressedAndReleased[0] = true;
  CHECK_EQUAL(0, memcmp(&expectedButtonStatus, &actualButtonStatus, sizeof(expectedButtonStatus)));

  buttonDriver->resetButtonStatus();

  touchDriverMock.setTouchReturnValue(isButtonTouchedYes2);
  buttonDriver->update();
  touchDriverMock.setTouchReturnValue(isButtonTouchedNone);
  buttonDriver->update();
  actualButtonStatus = buttonDriver->getButtonStatus();

  expectedButtonStatus.wasButtonPressedAndReleased[0] = false;
  expectedButtonStatus.wasButtonPressedAndReleased[2] = false;
  CHECK_EQUAL(0, memcmp(&expectedButtonStatus, &actualButtonStatus, sizeof(expectedButtonStatus)));
}


TEST(buttonDriver, testButtonStatusNotChangedUntilReleased)
{
  timerDummy.setNewIncrementValue(1);
  ButtonStatus expectedButtonStatus = { 0 };

  ButtonStatus actualButtonStatus = buttonDriver->getButtonStatus();
  CHECK_EQUAL(0, memcmp(&expectedButtonStatus, &actualButtonStatus, sizeof(expectedButtonStatus)));

  touchDriverMock.setTouchReturnValue(isButtonTouchedYes0);
  buttonDriver->update();

  actualButtonStatus = buttonDriver->getButtonStatus();
  CHECK_EQUAL(0, memcmp(&expectedButtonStatus, &actualButtonStatus, sizeof(expectedButtonStatus)));

  buttonDriver->update();

  actualButtonStatus = buttonDriver->getButtonStatus();
  CHECK_EQUAL(0, memcmp(&expectedButtonStatus, &actualButtonStatus, sizeof(expectedButtonStatus)));

  buttonDriver->update();

  actualButtonStatus = buttonDriver->getButtonStatus();
  CHECK_EQUAL(0, memcmp(&expectedButtonStatus, &actualButtonStatus, sizeof(expectedButtonStatus)));

  buttonDriver->update();

  actualButtonStatus = buttonDriver->getButtonStatus();
  CHECK_EQUAL(0, memcmp(&expectedButtonStatus, &actualButtonStatus, sizeof(expectedButtonStatus)));

  touchDriverMock.setTouchReturnValue(isButtonTouchedNone);
  buttonDriver->update();

  actualButtonStatus = buttonDriver->getButtonStatus();
  expectedButtonStatus.wasButtonPressedAndReleased[0] = true;
  CHECK_EQUAL(0, memcmp(&expectedButtonStatus, &actualButtonStatus, sizeof(expectedButtonStatus)));
}


TEST(buttonDriver, testButton0and3Held)
{
  timerDummy.setNewIncrementValue(0);
  touchDriverMock.setTouchReturnValue(isButtonTouchedYes0And2);
  buttonDriver->update();
  timerDummy.incrementTime(Constants::BUTTON_HELD_THRESHOLD_MS + 1U);
  buttonDriver->update();
  ButtonStatus actualButtonStatus = buttonDriver->getButtonStatus();

  ButtonStatus expectedButtonStatus = { 0 };
  expectedButtonStatus.isButtonHeld[0] = true;
  expectedButtonStatus.isButtonStartedBeingHeld[0] = true;
  expectedButtonStatus.buttonHeldDurationMs[0] = Constants::BUTTON_HELD_THRESHOLD_MS + 1U;
  expectedButtonStatus.isButtonHeld[2] = true;
  expectedButtonStatus.isButtonStartedBeingHeld[2] = true;
  expectedButtonStatus.buttonHeldDurationMs[2] = Constants::BUTTON_HELD_THRESHOLD_MS + 1U;
  CHECK_EQUAL(0, memcmp(&expectedButtonStatus, &actualButtonStatus, sizeof(expectedButtonStatus)));
}


TEST(buttonDriver, testButton0and3HeldButNotSimultaneously)
{
  timerDummy.setNewIncrementValue(0);
  touchDriverMock.setTouchReturnValue(isButtonTouchedYes0);
  buttonDriver->update();
  timerDummy.incrementTime(100);
  buttonDriver->update();

  timerDummy.incrementTime(1);
  touchDriverMock.setTouchReturnValue(isButtonTouchedYes0And2);
  buttonDriver->update();

  timerDummy.incrementTime(Constants::BUTTON_HELD_THRESHOLD_MS + 1U);
  buttonDriver->update();

  ButtonStatus actualButtonStatus = buttonDriver->getButtonStatus();

  ButtonStatus expectedButtonStatus = { 0 };
  expectedButtonStatus.isButtonHeld[0] = true;
  expectedButtonStatus.isButtonStartedBeingHeld[0] = true;
  expectedButtonStatus.buttonHeldDurationMs[0] = Constants::BUTTON_HELD_THRESHOLD_MS + 102U;
  expectedButtonStatus.isButtonHeld[2] = true;
  expectedButtonStatus.isButtonStartedBeingHeld[2] = true;
  expectedButtonStatus.buttonHeldDurationMs[2] = Constants::BUTTON_HELD_THRESHOLD_MS + 1U;
  CHECK_EQUAL(0, memcmp(&expectedButtonStatus, &actualButtonStatus, sizeof(expectedButtonStatus)));
}


TEST(buttonDriver, testButton0StartedBeingHeldLowerLimit)
{
  timerDummy.setNewIncrementValue(0);
  touchDriverMock.setTouchReturnValue(isButtonTouchedYes0);
  buttonDriver->update();
  timerDummy.incrementTime(Constants::BUTTON_STARTED_BEING_HELD_THRESHOLD_MS + 1U);
  buttonDriver->update();

  ButtonStatus actualButtonStatus = buttonDriver->getButtonStatus();

  ButtonStatus expectedButtonStatus = { 0 };
  expectedButtonStatus.isButtonStartedBeingHeld[0] = true;
  expectedButtonStatus.buttonHeldDurationMs[0] = 0UL; //Because this number should remain 0 as long as the button is only started being held (and not "properly" held yet).
  CHECK_EQUAL(0, memcmp(&expectedButtonStatus, &actualButtonStatus, sizeof(expectedButtonStatus)));
}


TEST(buttonDriver, testButton0StartedBeingHeldIsChangedToButtonPressedAndReleased)
{
  timerDummy.setNewIncrementValue(0);
  touchDriverMock.setTouchReturnValue(isButtonTouchedYes0);
  buttonDriver->update();
  timerDummy.incrementTime(Constants::BUTTON_STARTED_BEING_HELD_THRESHOLD_MS + 1U);
  buttonDriver->update();
  ButtonStatus actualButtonStatus = buttonDriver->getButtonStatus();

  ButtonStatus expectedButtonStatus = { 0 };
  expectedButtonStatus.isButtonStartedBeingHeld[0] = true;
  expectedButtonStatus.buttonHeldDurationMs[0] = 0UL; //Because this number should remain 0 as long as the button is only started being held (and not "properly" held yet).
  CHECK_EQUAL(0, memcmp(&expectedButtonStatus, &actualButtonStatus, sizeof(expectedButtonStatus)));

  timerDummy.incrementTime(1);
  touchDriverMock.setTouchReturnValue(isButtonTouchedNone);
  buttonDriver->update();
  actualButtonStatus = buttonDriver->getButtonStatus();
  expectedButtonStatus.isButtonStartedBeingHeld[0] = false;
  expectedButtonStatus.wasButtonPressedAndReleased[0] = true; //Because it was held for less than the button hold threshold.
  CHECK_EQUAL(0, memcmp(&expectedButtonStatus, &actualButtonStatus, sizeof(expectedButtonStatus)));
}

