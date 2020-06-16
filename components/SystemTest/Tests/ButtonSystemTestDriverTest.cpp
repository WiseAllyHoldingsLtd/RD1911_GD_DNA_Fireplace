#include <cstring>

#include "CppUTest\TestHarness.h"
#include "CppUTestExt\MockSupport.h"

#include "ButtonSystemTestDriver.h"
#include "SystemTestCommunicatorMock.h"
#include "ButtonDriverMock.h"


TEST_GROUP(buttonSystemTestDriver)
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


TEST(buttonSystemTestDriver, testGetButtonStatusFromButtonDriver)
{
  SystemTestCommunicatorMock sysTestComMock;
  ButtonDriverMock buttonDriverMock;
  ButtonSystemTestDriver buttonSystemTestDriver(sysTestComMock, buttonDriverMock);

  ButtonStatus generatedButtonStatus = {};
  generatedButtonStatus.isButtonHeld[2] = true;

  ButtonStatus dummyButtonStatus = {};

  mock().expectOneCall("getButtonStatusS").withOutputParameterReturning("buttonStatus", (void*)&dummyButtonStatus, sizeof(dummyButtonStatus)).andReturnValue(false);
  mock().expectOneCall("getButtonStatusB").andReturnValue((void*)&generatedButtonStatus);
  ButtonStatus returnedButtonStatus = buttonSystemTestDriver.getButtonStatus();

  CHECK_EQUAL(0, memcmp(&generatedButtonStatus, &returnedButtonStatus, sizeof(generatedButtonStatus)));
}


TEST(buttonSystemTestDriver, testGetButtonStatusFromSystemTest)
{
  SystemTestCommunicatorMock sysTestComMock;
  ButtonDriverMock buttonDriverMock;
  ButtonSystemTestDriver buttonSystemTestDriver(sysTestComMock, buttonDriverMock);

  ButtonStatus generatedButtonStatus = {};
  generatedButtonStatus.isButtonHeld[2] = true;

  mock().expectOneCall("getButtonStatusS").withOutputParameterReturning("buttonStatus", (void*)&generatedButtonStatus, sizeof(generatedButtonStatus)).andReturnValue(true);
  ButtonStatus returnedButtonStatus = buttonSystemTestDriver.getButtonStatus();

  CHECK_EQUAL(0, memcmp(&generatedButtonStatus, &returnedButtonStatus, sizeof(generatedButtonStatus)));
}


TEST(buttonSystemTestDriver, testResetCallsResetOnCommunicator)
{
  SystemTestCommunicatorMock sysTestComMock;
  ButtonDriverMock buttonDriverMock;
  ButtonSystemTestDriver buttonSystemTestDriver(sysTestComMock, buttonDriverMock);
  mock().expectOneCall("resetButtonStatusS");
  mock().ignoreOtherCalls();
  buttonSystemTestDriver.resetButtonStatus();
}

TEST(buttonSystemTestDriver, testResetCallsResetOnRealButtonDriver)
{
  SystemTestCommunicatorMock sysTestComMock;
  ButtonDriverMock buttonDriverMock;
  ButtonSystemTestDriver buttonSystemTestDriver(sysTestComMock, buttonDriverMock);
  mock().expectOneCall("resetButtonStatusB");
  mock().ignoreOtherCalls();
  buttonSystemTestDriver.resetButtonStatus();
}

