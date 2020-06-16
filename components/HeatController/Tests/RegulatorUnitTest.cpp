#include <cstring>
#include "CppUTest\TestHarness.h"
#include "CppUTestExt\MockSupport.h"

#include "RegulatorUnit.h"
#include "TWIDriverMock.h"
#include "FixedArrayComparator.h"


class FixedArrayValueCopier : public MockNamedValueCopier
{
public:
  FixedArrayValueCopier(uint8_t arraySize) : m_arraySize(arraySize) {}
  virtual void copy(void* out, const void* in)
  {
    memcpy(out, in, m_arraySize);
  }
private:
  uint8_t m_arraySize;
};


static TWIMock twi;
static RegulatorUnit *ru;


TEST_GROUP(regulatorunit)
{
  TEST_SETUP()
  {
    ru = new RegulatorUnit(twi);
  }

  TEST_TEARDOWN()
  {
    delete ru;
    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
  }
};

TEST(regulatorunit, testThatSelfTestSendsAPollCommand)
{
  FixedArrayComparator comparator(1u);
  mock().installComparator("SendBuffer", comparator);
  FixedArrayValueCopier copier(1u);
  mock().installCopier("ReceiveBuffer", copier);
  uint8_t sendBuffer[1] = {5u};
  uint8_t receiveBuffer[1] = {0u};

  mock().expectOneCall("TWIMock::sendReceive")
      .withParameter("slaveAddress", RegulatorUnit::RU_SLAVE_ADDRESS)
      .withParameterOfType("SendBuffer", "buffer", sendBuffer)
      .withOutputParameterOfTypeReturning("ReceiveBuffer", "buffer", receiveBuffer)
      .withParameter("sendLength", 1u)
      .withParameter("receiveLength", 1u)
      .andReturnValue(true);
  ru->performSelfTest();
}

TEST(regulatorunit, testThatSelfTestRetursTrueWhenCommunicationIsOK)
{
  FixedArrayComparator comparator(1u);
  mock().installComparator("SendBuffer", comparator);
  FixedArrayValueCopier copier(1u);
  mock().installCopier("ReceiveBuffer", copier);

  mock().expectOneCall("TWIMock::sendReceive")
      .ignoreOtherParameters()
      .andReturnValue(true);
  bool result = ru->performSelfTest();
  CHECK_TRUE(result);
}

TEST(regulatorunit, testThatSelfTestRetursFalseWhenCommunicationFails)
{
  FixedArrayComparator comparator(1u);
  mock().installComparator("SendBuffer", comparator);
  FixedArrayValueCopier copier(1u);
  mock().installCopier("ReceiveBuffer", copier);

  mock().expectOneCall("TWIMock::sendReceive")
      .ignoreOtherParameters()
      .andReturnValue(false);
  bool result = ru->performSelfTest();
  CHECK_FALSE(result);
}

TEST(regulatorunit, testThatSWVersionIsInvalidBeforeUpdateConfigCalled)
{
  CHECK_EQUAL(-1, ru->getSWVersion());
}

TEST(regulatorunit, testThatSWVersionIsInvalidIfUpdateConfigFails)
{
  FixedArrayComparator comparator(7u);
  mock().installComparator("SendBuffer", comparator);
  FixedArrayValueCopier copier(1u);
  mock().installCopier("ReceiveBuffer", copier);
  uint8_t receiveBuffer[1] = {7};

  mock().expectOneCall("TWIMock::sendReceive")
      .withOutputParameterOfTypeReturning("ReceiveBuffer", "buffer", receiveBuffer)
      .withParameter("receiveLength", 1u)
      .ignoreOtherParameters()
      .andReturnValue(false);
  ru->updateConfig(RURegulator::REGULATOR_SETPOINT, true, true, 0);
  CHECK_EQUAL(-1, ru->getSWVersion());
}

TEST(regulatorunit, testThatSendConfigSendsTheCorrectCommand)
{
  FixedArrayComparator comparator(7u);
  mock().installComparator("SendBuffer", comparator);
  FixedArrayValueCopier copier(1u);
  mock().installCopier("ReceiveBuffer", copier);
  uint8_t sendBuffer[7] = {1, 11, 88, 27, 0, 16, 112};
  uint8_t receiveBuffer[1] = {7};

  mock().expectOneCall("TWIMock::sendReceive")
      .withParameter("slaveAddress", RegulatorUnit::RU_SLAVE_ADDRESS)
      .withParameterOfType("SendBuffer", "buffer", sendBuffer)
      .withOutputParameterOfTypeReturning("ReceiveBuffer", "buffer", receiveBuffer)
      .withParameter("sendLength", 7u)
      .withParameter("receiveLength", 1u)
      .andReturnValue(true);
  ru->updateConfig(RURegulator::REGULATOR_OFF, true, false, 7000);
}

TEST(regulatorunit, testThatSendConfigSendsTheCorrectCommand2)
{
  FixedArrayComparator comparator(7u);
  mock().installComparator("SendBuffer", comparator);
  FixedArrayValueCopier copier(1u);
  mock().installCopier("ReceiveBuffer", copier);
  uint8_t sendBuffer[7] = {1, 4, 216, 89, 0, 118, 82};
  uint8_t receiveBuffer[1] = {7};

  mock().expectOneCall("TWIMock::sendReceive")
      .withParameter("slaveAddress", RegulatorUnit::RU_SLAVE_ADDRESS)
      .withParameterOfType("SendBuffer", "buffer", sendBuffer)
      .withOutputParameterOfTypeReturning("ReceiveBuffer", "buffer", receiveBuffer)
      .withParameter("sendLength", 7u)
      .withParameter("receiveLength", 1u)
      .andReturnValue(true);
  ru->updateConfig(RURegulator::REGULATOR_SETPOINT, false, true, 23000);
}

TEST(regulatorunit, testThatSendConfigReturnsTrueWhenCommunicationIsOK)
{
  FixedArrayComparator comparator(7u);
  mock().installComparator("SendBuffer", comparator);
  FixedArrayValueCopier copier(1u);
  mock().installCopier("ReceiveBuffer", copier);
  uint8_t receiveBuffer[1] = {7};

  mock().expectOneCall("TWIMock::sendReceive")
      .withOutputParameterOfTypeReturning("ReceiveBuffer", "buffer", receiveBuffer)
      .withParameter("receiveLength", 1u)
      .ignoreOtherParameters()
      .andReturnValue(true);
  bool result = ru->updateConfig(RURegulator::REGULATOR_SETPOINT, true, true, 0);
  CHECK_TRUE(result);
}

TEST(regulatorunit, testThatSendConfigReturnsFalseIfCommunicationFails)
{
  FixedArrayComparator comparator(7u);
  mock().installComparator("SendBuffer", comparator);
  FixedArrayValueCopier copier(1u);
  mock().installCopier("ReceiveBuffer", copier);
  uint8_t receiveBuffer[1] = {1};

  mock().expectOneCall("TWIMock::sendReceive")
      .withOutputParameterOfTypeReturning("ReceiveBuffer", "buffer", receiveBuffer)
      .withParameter("receiveLength", 1u)
      .ignoreOtherParameters()
      .andReturnValue(true);
  bool result = ru->updateConfig(RURegulator::REGULATOR_SETPOINT, true, true, 0);
  CHECK_FALSE(result);
}

TEST(regulatorunit, testThatSendConfigSetsSWVersionWhenOK)
{
  FixedArrayComparator comparator(7u);
  mock().installComparator("SendBuffer", comparator);
  FixedArrayValueCopier copier(1u);
  mock().installCopier("ReceiveBuffer", copier);
  uint8_t receiveBuffer[1] = {111}; // 7 + (13 << 3)

  mock().expectOneCall("TWIMock::sendReceive")
      .withOutputParameterOfTypeReturning("ReceiveBuffer", "buffer", receiveBuffer)
      .withParameter("receiveLength", 1u)
      .ignoreOtherParameters()
      .andReturnValue(true);
  ru->updateConfig(RURegulator::REGULATOR_SETPOINT, true, true, 0);
  CHECK_EQUAL(13, ru->getSWVersion());
}

TEST(regulatorunit, testThatGetEstimatedRoomTemperatureSendsTheCorrectCommand)
{
  FixedArrayComparator comparator(1u);
  mock().installComparator("SendBuffer", comparator);
  FixedArrayValueCopier copier(6u);
  mock().installCopier("ReceiveBuffer", copier);
  uint8_t sendBuffer[1] = {3};

  mock().expectOneCall("TWIMock::sendReceive")
      .withParameter("slaveAddress", RegulatorUnit::RU_SLAVE_ADDRESS)
      .withParameterOfType("SendBuffer", "buffer", sendBuffer)
      .withParameter("sendLength", 1u)
      .ignoreOtherParameters()
      .andReturnValue(true);
  ru->getEstimatedRoomTemperatureFP();
}

TEST(regulatorunit, testThatGetEstimatedRoomTemperatureReturnsInvalidValueWhenCommunicationFails)
{
  FixedArrayComparator comparator(1u);
  mock().installComparator("SendBuffer", comparator);
  FixedArrayValueCopier copier(6u);
  mock().installCopier("ReceiveBuffer", copier);

  mock().expectOneCall("TWIMock::sendReceive")
      .ignoreOtherParameters()
      .andReturnValue(false);
  int32_t temp = ru->getEstimatedRoomTemperatureFP();
  CHECK_EQUAL(INT32_MIN, temp);
}

TEST(regulatorunit, testThatGetEstimatedRoomTemperatureReturnsInvalidValueWhenCheckSumFails)
{
  FixedArrayComparator comparator(1u);
  mock().installComparator("SendBuffer", comparator);
  FixedArrayValueCopier copier(6u);
  mock().installCopier("ReceiveBuffer", copier);
  uint8_t receiveBuffer[6] = {24, 121, 0, 0, 0, 0};

  mock().expectOneCall("TWIMock::sendReceive")
      .withOutputParameterOfTypeReturning("ReceiveBuffer", "buffer", receiveBuffer)
      .withParameter("receiveLength", 6u)
      .ignoreOtherParameters()
      .andReturnValue(true);
  int32_t temp = ru->getEstimatedRoomTemperatureFP();
  CHECK_EQUAL(INT32_MIN, temp);
}

TEST(regulatorunit, testThatGetEstimatedRoomTemperatureParsesTemperature)
{
  FixedArrayComparator comparator(1u);
  mock().installComparator("SendBuffer", comparator);
  FixedArrayValueCopier copier(6u);
  mock().installCopier("ReceiveBuffer", copier);
  uint8_t receiveBuffer[6] = {24, 121, 0, 0, 161, 204};

// Can calcualte checksum like this (make methods public first)
//  uint16_t checksum = ru->calculateChecksum(receiveBuffer, 4);
//  ru->getCheckBytesFromChecksum(checksum, &receiveBuffer[4]);

  mock().expectOneCall("TWIMock::sendReceive")
      .withParameter("slaveAddress", RegulatorUnit::RU_SLAVE_ADDRESS)
      .withOutputParameterOfTypeReturning("ReceiveBuffer", "buffer", receiveBuffer)
      .withParameter("receiveLength", 6u)
      .ignoreOtherParameters()
      .andReturnValue(true);
  int32_t temp = ru->getEstimatedRoomTemperatureFP();
  CHECK_EQUAL(31000, temp);
}


TEST(regulatorunit, testThatGetRegulatorPotMeterSendsTheCorrectCommand)
{
  FixedArrayComparator comparator(1u);
  mock().installComparator("SendBuffer", comparator);
  FixedArrayValueCopier copier(6u);
  mock().installCopier("ReceiveBuffer", copier);
  uint8_t sendBuffer[1] = {4};

  mock().expectOneCall("TWIMock::sendReceive")
      .withParameter("slaveAddress", RegulatorUnit::RU_SLAVE_ADDRESS)
      .withParameterOfType("SendBuffer", "buffer", sendBuffer)
      .withParameter("sendLength", 1u)
      .ignoreOtherParameters()
      .andReturnValue(true);
  ru->getRegulatorPotMeterFP();
}

TEST(regulatorunit, testThatGetRegulatorPotMeterReturnsInvalidValueWhenCommunicationFails)
{
  FixedArrayComparator comparator(1u);
  mock().installComparator("SendBuffer", comparator);
  FixedArrayValueCopier copier(6u);
  mock().installCopier("ReceiveBuffer", copier);

  mock().expectOneCall("TWIMock::sendReceive")
      .ignoreOtherParameters()
      .andReturnValue(false);
  int32_t temp = ru->getRegulatorPotMeterFP();
  CHECK_EQUAL(INT32_MIN, temp);
}

TEST(regulatorunit, testThatGetRegulatorPotMeterReturnsInvalidValueWhenCheckSumFails)
{
  FixedArrayComparator comparator(1u);
  mock().installComparator("SendBuffer", comparator);
  FixedArrayValueCopier copier(6u);
  mock().installCopier("ReceiveBuffer", copier);
  uint8_t receiveBuffer[6] = {24, 121, 0, 0, 0, 0};

  mock().expectOneCall("TWIMock::sendReceive")
      .withOutputParameterOfTypeReturning("ReceiveBuffer", "buffer", receiveBuffer)
      .withParameter("receiveLength", 6u)
      .ignoreOtherParameters()
      .andReturnValue(true);
  int32_t temp = ru->getRegulatorPotMeterFP();
  CHECK_EQUAL(INT32_MIN, temp);
}

TEST(regulatorunit, testThatGetRegulatorPotMeterParsesTemperature)
{
  FixedArrayComparator comparator(1u);
  mock().installComparator("SendBuffer", comparator);
  FixedArrayValueCopier copier(6u);
  mock().installCopier("ReceiveBuffer", copier);
  uint8_t receiveBuffer[6] = {24, 121, 0, 0, 161, 204};

  mock().expectOneCall("TWIMock::sendReceive")
      .withOutputParameterOfTypeReturning("ReceiveBuffer", "buffer", receiveBuffer)
      .withParameter("receiveLength", 6u)
      .ignoreOtherParameters()
      .andReturnValue(true);
  int32_t temp = ru->getRegulatorPotMeterFP();
  CHECK_EQUAL(31000, temp);
}


TEST(regulatorunit, testThatIsHeatElementActiveSendsTheRightCommand)
{
  FixedArrayComparator comparator(1u);
  mock().installComparator("SendBuffer", comparator);
  FixedArrayValueCopier copier(6u);
  mock().installCopier("ReceiveBuffer", copier);
  uint8_t sendBuffer[1] = {5};

  mock().expectOneCall("TWIMock::sendReceive")
      .withParameter("slaveAddress", RegulatorUnit::RU_SLAVE_ADDRESS)
      .withParameterOfType("SendBuffer", "buffer", sendBuffer)
      .withParameter("sendLength", 1u)
      .ignoreOtherParameters()
      .andReturnValue(true);
  ru->isHeatElementActive();
}

TEST(regulatorunit, testThatIsHeatElementActiveRetursFalseWhenCommunicationFails)
{
  FixedArrayComparator comparator(1u);
  mock().installComparator("SendBuffer", comparator);
  FixedArrayValueCopier copier(1u);
  mock().installCopier("ReceiveBuffer", copier);

  mock().expectOneCall("TWIMock::sendReceive")
      .ignoreOtherParameters()
      .andReturnValue(false);
  bool result = ru->isHeatElementActive();
  CHECK_FALSE(result);
}

TEST(regulatorunit, testThatIsHeatElementActiveParsesOffOK)
{
  FixedArrayComparator comparator(1u);
  mock().installComparator("SendBuffer", comparator);
  FixedArrayValueCopier copier(1u);
  mock().installCopier("ReceiveBuffer", copier);
  uint8_t receiveBuffer[1] = {0};

  mock().expectOneCall("TWIMock::sendReceive")
      .withOutputParameterOfTypeReturning("ReceiveBuffer", "buffer", receiveBuffer)
      .withParameter("receiveLength", 1u)
      .ignoreOtherParameters()
      .andReturnValue(true);
  bool result = ru->isHeatElementActive();
  CHECK_FALSE(result);
}

TEST(regulatorunit, testThatIsHeatElementActiveParsesOnOK)
{
  FixedArrayComparator comparator(1u);
  mock().installComparator("SendBuffer", comparator);
  FixedArrayValueCopier copier(1u);
  mock().installCopier("ReceiveBuffer", copier);
  uint8_t receiveBuffer[1] = {1};

  mock().expectOneCall("TWIMock::sendReceive")
      .withOutputParameterOfTypeReturning("ReceiveBuffer", "buffer", receiveBuffer)
      .withParameter("receiveLength", 1u)
      .ignoreOtherParameters()
      .andReturnValue(true);
  bool result = ru->isHeatElementActive();
  CHECK_TRUE(result);
}

TEST(regulatorunit, testThatDefaultProductVersionIsNotHighPower)
{
  CHECK_FALSE(ru->isHighPowerVersion());
}

TEST(regulatorunit, testThatProductVersionIsNotHighPowerForVersion15)
{
  FixedArrayComparator comparator(7u);
  mock().installComparator("SendBuffer", comparator);
  FixedArrayValueCopier copier(1u);
  mock().installCopier("ReceiveBuffer", copier);
  uint8_t receiveBuffer[1] = {127}; // 7 + (15 << 3)

  mock().expectOneCall("TWIMock::sendReceive")
      .withOutputParameterOfTypeReturning("ReceiveBuffer", "buffer", receiveBuffer)
      .withParameter("receiveLength", 1u)
      .ignoreOtherParameters()
      .andReturnValue(true);
  ru->updateConfig(RURegulator::REGULATOR_SETPOINT, true, true, 0);

  CHECK_EQUAL(15, ru->getSWVersion());
  CHECK_FALSE(ru->isHighPowerVersion());
}

TEST(regulatorunit, testThatDefaultProductVersionIsHighPowerForVersion16)
{
  FixedArrayComparator comparator(7u);
  mock().installComparator("SendBuffer", comparator);
  FixedArrayValueCopier copier(1u);
  mock().installCopier("ReceiveBuffer", copier);
  uint8_t receiveBuffer[1] = {135}; // 7 + (16 << 3)

  mock().expectOneCall("TWIMock::sendReceive")
      .withOutputParameterOfTypeReturning("ReceiveBuffer", "buffer", receiveBuffer)
      .withParameter("receiveLength", 1u)
      .ignoreOtherParameters()
      .andReturnValue(true);
  ru->updateConfig(RURegulator::REGULATOR_SETPOINT, true, true, 0);

  CHECK_EQUAL(16, ru->getSWVersion());
  CHECK_TRUE(ru->isHighPowerVersion());
}

