#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include "RUMock.h"
#include "TriacDriver.h"
#include "Constants.h"


static RUMock ruMock;
static TriacDriver * triacDriver;


TEST_GROUP(TriacDriver)
{
  TEST_SETUP()
  {
    triacDriver = new TriacDriver(ruMock);
    mock().installComparator("RURegulator", ruRegulatorComparator);
  }

  TEST_TEARDOWN()
  {
    delete triacDriver;

    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
  }
};


TEST(TriacDriver, turnOnUseLowExternallyMeasuredTempAndMaxSetpointToForceOn)
{
  RURegulator ruMode = RURegulator::REGULATOR_SETPOINT;
  mock().expectOneCall("RUMock::updateConfig")
    .withParameterOfType("RURegulator", "regulator", &ruMode)
    .withBoolParameter("useInternalTemp", false)
    .withBoolParameter("enableRUPot", false)
    .withIntParameter("newSetpointFP", Constants::MAX_SETP_TEMPERATURE)
    .andReturnValue(true);

  mock().expectOneCall("RUMock::updateExternalTemperatureFP").withIntParameter("temperatureFP", Constants::MIN_SETP_TEMPERATURE).andReturnValue(true);
  mock().ignoreOtherCalls();

  bool result = triacDriver->turnOn();
  CHECK_TRUE(result);
}

TEST(TriacDriver, turnOnReturnsFalseIfUpdateExternalTemperatureFails)
{
  mock().expectOneCall("RUMock::updateConfig").ignoreOtherParameters().andReturnValue(true);
  mock().expectOneCall("RUMock::updateExternalTemperatureFP").ignoreOtherParameters().andReturnValue(false);
  mock().ignoreOtherCalls();

  bool result = triacDriver->turnOn();
  CHECK_FALSE(result);
}

TEST(TriacDriver, turnOnDoesNotUpdateExternalTemperatureIfUpdateConfigFails)
{
  mock().expectOneCall("RUMock::updateConfig").ignoreOtherParameters().andReturnValue(false);
  mock().expectNoCall("RUMock::updateExternalTemperatureFP");
  mock().ignoreOtherCalls();

  bool result = triacDriver->turnOn();
  CHECK_FALSE(result);
}

TEST(TriacDriver, turnOffUseRegulatorOffToForceOff)
{
  RURegulator ruMode = RURegulator::REGULATOR_OFF;
  mock().expectOneCall("RUMock::updateConfig")
    .withParameterOfType("RURegulator", "regulator", &ruMode)
    .ignoreOtherParameters()
    .andReturnValue(true);

  mock().ignoreOtherCalls();

  bool result = triacDriver->turnOff();
  CHECK_TRUE(result);
}

TEST(TriacDriver, turnOffReturnsFalseIfUpdateConfigFails)
{
  mock().expectOneCall("RUMock::updateConfig").ignoreOtherParameters().andReturnValue(false);

  bool result = triacDriver->turnOff();
  CHECK_FALSE(result);
}
