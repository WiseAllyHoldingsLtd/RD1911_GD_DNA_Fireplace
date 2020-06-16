#include "CppUTest\TestHarness.h"
#include "CppUTestExt\MockSupport.h"

#include "SystemTestCommunicator.h"
#include "ViewModelMock.h"
#include "SpiDriverMock.h"
#include "Constants.h"
#include "FixedArrayComparator.h"


FixedArrayComparator sendBufferComparator(Constants::SYSTEM_TEST_EXPECTED_LED_VIEW_MESSAGE_LENGTH);

const uint8_t BUTTON_MSG_LENGTH = Constants::SYSTEM_TEST_EXPECTED_BUTTON_MESSAGE_LENGTH - Constants::SYSTEM_TEST_REPLY_OFFSET;

TEST_GROUP(systemTestCommunicator)
{
  TEST_SETUP()
  {
    mock().installComparator("sendBuffer", sendBufferComparator);
  }

  TEST_TEARDOWN()
  {
    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
  }
};


TEST(systemTestCommunicator, testConstructorInitsSpi)
{
  SpiDriverMock spiDriver;
  mock().expectOneCall("resetAndInit");
  SystemTestCommunicator sysTestCom(spiDriver);
}


TEST(systemTestCommunicator, testSendViewModelInfoCallsSpi)
{
  SpiDriverMock spiDriver;
  mock().expectOneCall("resetAndInit");
  SystemTestCommunicator sysTestCom(spiDriver);

  ViewModelMock viewModel;
  uint8_t receiveBuffer[Constants::SPI_SYSTEM_TEST_BUFFER_LENGTH] = {};

  mock().expectOneCall("sendReceive")
      .withParameter("target", SpiTarget::SystemTest)
      .withOutputParameterReturning("receiveBuffer", receiveBuffer, sizeof(receiveBuffer))
      .ignoreOtherParameters();
  mock().ignoreOtherCalls();

  sysTestCom.sendViewModelInfo(viewModel);
}


TEST(systemTestCommunicator, testGetButtonStatusFalseWhenNoSpiReceived)
{
  SpiDriverMock spiDriver;
  mock().expectOneCall("resetAndInit");
  SystemTestCommunicator sysTestCom(spiDriver);

  ViewModelMock viewModel;
  uint8_t receiveBuffer[Constants::SPI_SYSTEM_TEST_BUFFER_LENGTH] = {};

  mock().expectOneCall("sendReceive")
      .withParameter("target", SpiTarget::SystemTest)
      .withOutputParameterReturning("receiveBuffer", receiveBuffer, sizeof(receiveBuffer))
      .ignoreOtherParameters();
  mock().ignoreOtherCalls();
  sysTestCom.sendViewModelInfo(viewModel);

  ButtonStatus dummyButtonStatus = {};

  CHECK_FALSE(sysTestCom.getButtonStatus(dummyButtonStatus));
}


TEST(systemTestCommunicator, testGetButtonStatusTrueWhenSpiReceived)
{
  SpiDriverMock spiDriver;
  mock().expectOneCall("resetAndInit");
  SystemTestCommunicator sysTestCom(spiDriver);

  ViewModelMock viewModel;
  uint8_t receiveBuffer[Constants::SPI_SYSTEM_TEST_BUFFER_LENGTH] = {};
  receiveBuffer[2] = Constants::SYSTEM_TEST_COMMAND_TYPE_BUTTON;
  receiveBuffer[3] = BUTTON_MSG_LENGTH;

  mock().expectOneCall("sendReceive")
      .withParameter("target", SpiTarget::SystemTest)
      .withOutputParameterReturning("receiveBuffer", receiveBuffer, sizeof(receiveBuffer))
      .ignoreOtherParameters();
  mock().ignoreOtherCalls();
  sysTestCom.sendViewModelInfo(viewModel);

  ButtonStatus dummyButtonStatus = {};

  CHECK_TRUE(sysTestCom.getButtonStatus(dummyButtonStatus));
}


TEST(systemTestCommunicator, testGetButtonStatusTrueTwoTimesInRow)
{
  SpiDriverMock spiDriver;
  mock().expectOneCall("resetAndInit");
  SystemTestCommunicator sysTestCom(spiDriver);

  ViewModelMock viewModel;
  uint8_t receiveBuffer[Constants::SPI_SYSTEM_TEST_BUFFER_LENGTH] = {};
  receiveBuffer[2] = Constants::SYSTEM_TEST_COMMAND_TYPE_BUTTON;
  receiveBuffer[3] = BUTTON_MSG_LENGTH;

  mock().expectOneCall("sendReceive")
      .withParameter("target", SpiTarget::SystemTest)
      .withOutputParameterReturning("receiveBuffer", receiveBuffer, sizeof(receiveBuffer))
      .ignoreOtherParameters();
  mock().ignoreOtherCalls();
  sysTestCom.sendViewModelInfo(viewModel);

  ButtonStatus dummyButtonStatus = {};

  CHECK_TRUE(sysTestCom.getButtonStatus(dummyButtonStatus));
  CHECK_TRUE(sysTestCom.getButtonStatus(dummyButtonStatus));
}


TEST(systemTestCommunicator, testGetButtonStatusFalseAfterReset)
{
  SpiDriverMock spiDriver;
  mock().expectOneCall("resetAndInit");
  SystemTestCommunicator sysTestCom(spiDriver);

  ViewModelMock viewModel;
  uint8_t receiveBuffer[Constants::SPI_SYSTEM_TEST_BUFFER_LENGTH] = {};
  receiveBuffer[2] = Constants::SYSTEM_TEST_COMMAND_TYPE_BUTTON;
  receiveBuffer[3] = BUTTON_MSG_LENGTH;

  mock().expectOneCall("sendReceive")
      .withParameter("target", SpiTarget::SystemTest)
      .withOutputParameterReturning("receiveBuffer", receiveBuffer, sizeof(receiveBuffer))
      .ignoreOtherParameters();
  mock().ignoreOtherCalls();
  sysTestCom.sendViewModelInfo(viewModel);

  ButtonStatus dummyButtonStatus = {};

  CHECK_TRUE(sysTestCom.getButtonStatus(dummyButtonStatus));

  sysTestCom.resetButtonStatus();

  CHECK_FALSE(sysTestCom.getButtonStatus(dummyButtonStatus));
}


TEST(systemTestCommunicator, testParsedButtonPress0And2)
{
  SpiDriverMock spiDriver;
  mock().expectOneCall("resetAndInit");
  SystemTestCommunicator sysTestCom(spiDriver);

  ViewModelMock viewModel;
  uint8_t receiveBuffer[Constants::SPI_SYSTEM_TEST_BUFFER_LENGTH] = {};
  receiveBuffer[2] = Constants::SYSTEM_TEST_COMMAND_TYPE_BUTTON;
  receiveBuffer[3] = BUTTON_MSG_LENGTH;
  receiveBuffer[4] = 1;
  receiveBuffer[6] = 1;

  mock().expectOneCall("sendReceive")
      .withParameter("target", SpiTarget::SystemTest)
      .withOutputParameterReturning("receiveBuffer", receiveBuffer, sizeof(receiveBuffer))
      .ignoreOtherParameters();
  mock().ignoreOtherCalls();
  sysTestCom.sendViewModelInfo(viewModel);

  ButtonStatus simulatedButtonStatus = {};

  sysTestCom.getButtonStatus(simulatedButtonStatus);

  CHECK_EQUAL(true,  simulatedButtonStatus.wasButtonPressedAndReleased[0]);
  CHECK_EQUAL(false, simulatedButtonStatus.wasButtonPressedAndReleased[1]);
  CHECK_EQUAL(true,  simulatedButtonStatus.wasButtonPressedAndReleased[2]);
}


TEST(systemTestCommunicator, testParsedButtonPress1And2)
{
  SpiDriverMock spiDriver;
  mock().expectOneCall("resetAndInit");
  SystemTestCommunicator sysTestCom(spiDriver);

  ViewModelMock viewModel;
  uint8_t receiveBuffer[Constants::SPI_SYSTEM_TEST_BUFFER_LENGTH] = {};
  receiveBuffer[2] = Constants::SYSTEM_TEST_COMMAND_TYPE_BUTTON;
  receiveBuffer[3] = BUTTON_MSG_LENGTH;
  receiveBuffer[5] = 1;
  receiveBuffer[6] = 1;

  mock().expectOneCall("sendReceive")
      .withParameter("target", SpiTarget::SystemTest)
      .withOutputParameterReturning("receiveBuffer", receiveBuffer, sizeof(receiveBuffer))
      .ignoreOtherParameters();
  mock().ignoreOtherCalls();
  sysTestCom.sendViewModelInfo(viewModel);

  ButtonStatus simulatedButtonStatus = {};

  sysTestCom.getButtonStatus(simulatedButtonStatus);

  CHECK_EQUAL(false, simulatedButtonStatus.wasButtonPressedAndReleased[0]);
  CHECK_EQUAL(true, simulatedButtonStatus.wasButtonPressedAndReleased[1]);
  CHECK_EQUAL(true, simulatedButtonStatus.wasButtonPressedAndReleased[2]);
}


TEST(systemTestCommunicator, testParsedButtonHold0And2)
{
  SpiDriverMock spiDriver;
  mock().expectOneCall("resetAndInit");
  SystemTestCommunicator sysTestCom(spiDriver);

  ViewModelMock viewModel;
  uint8_t receiveBuffer[Constants::SPI_SYSTEM_TEST_BUFFER_LENGTH] = {};
  receiveBuffer[2] = Constants::SYSTEM_TEST_COMMAND_TYPE_BUTTON;
  receiveBuffer[3] = BUTTON_MSG_LENGTH;
  receiveBuffer[7] = 1;
  receiveBuffer[9] = 1;

  mock().expectOneCall("sendReceive")
      .withParameter("target", SpiTarget::SystemTest)
      .withOutputParameterReturning("receiveBuffer", receiveBuffer, sizeof(receiveBuffer))
      .ignoreOtherParameters();
  mock().ignoreOtherCalls();
  sysTestCom.sendViewModelInfo(viewModel);

  ButtonStatus simulatedButtonStatus = {};

  sysTestCom.getButtonStatus(simulatedButtonStatus);

  CHECK_EQUAL(true, simulatedButtonStatus.isButtonHeld[0]);
  CHECK_EQUAL(false, simulatedButtonStatus.isButtonHeld[1]);
  CHECK_EQUAL(true, simulatedButtonStatus.isButtonHeld[2]);
}


TEST(systemTestCommunicator, testParsedButtonHold1And2)
{
  SpiDriverMock spiDriver;
  mock().expectOneCall("resetAndInit");
  SystemTestCommunicator sysTestCom(spiDriver);

  ViewModelMock viewModel;
  uint8_t receiveBuffer[Constants::SPI_SYSTEM_TEST_BUFFER_LENGTH] = {};
  receiveBuffer[2] = Constants::SYSTEM_TEST_COMMAND_TYPE_BUTTON;
  receiveBuffer[3] = BUTTON_MSG_LENGTH;
  receiveBuffer[8] = 1;
  receiveBuffer[9] = 1;

  mock().expectOneCall("sendReceive")
      .withParameter("target", SpiTarget::SystemTest)
      .withOutputParameterReturning("receiveBuffer", receiveBuffer, sizeof(receiveBuffer))
      .ignoreOtherParameters();
  mock().ignoreOtherCalls();
  sysTestCom.sendViewModelInfo(viewModel);

  ButtonStatus simulatedButtonStatus = {};

  sysTestCom.getButtonStatus(simulatedButtonStatus);

  CHECK_EQUAL(false, simulatedButtonStatus.isButtonHeld[0]);
  CHECK_EQUAL(true, simulatedButtonStatus.isButtonHeld[1]);
  CHECK_EQUAL(true, simulatedButtonStatus.isButtonHeld[2]);
}



TEST(systemTestCommunicator, testParsedButtonHoldTimes)
{
  SpiDriverMock spiDriver;
  mock().expectOneCall("resetAndInit");
  SystemTestCommunicator sysTestCom(spiDriver);

  ViewModelMock viewModel;
  uint8_t receiveBuffer[Constants::SPI_SYSTEM_TEST_BUFFER_LENGTH] = {};
  receiveBuffer[2] = Constants::SYSTEM_TEST_COMMAND_TYPE_BUTTON;
  receiveBuffer[3] = BUTTON_MSG_LENGTH;

  receiveBuffer[10 + 0] = 0x05;
  receiveBuffer[10 + 1] = 0x04;
  receiveBuffer[10 + 2] = 0x03;
  receiveBuffer[10 + 3] = 0x02;
  receiveBuffer[14 + 1] = 0x60;
  receiveBuffer[18 + 2] = 0x70;

  mock().expectOneCall("sendReceive")
      .withParameter("target", SpiTarget::SystemTest)
      .withOutputParameterReturning("receiveBuffer", receiveBuffer, sizeof(receiveBuffer))
      .ignoreOtherParameters();
  mock().ignoreOtherCalls();
  sysTestCom.sendViewModelInfo(viewModel);

  ButtonStatus simulatedButtonStatus = {};

  sysTestCom.getButtonStatus(simulatedButtonStatus);

  CHECK_EQUAL(0x02030405, simulatedButtonStatus.buttonHeldDurationMs[0]);
  CHECK_EQUAL(0x6000, simulatedButtonStatus.buttonHeldDurationMs[1]);
  CHECK_EQUAL(0x700000, simulatedButtonStatus.buttonHeldDurationMs[2]);
}


TEST(systemTestCommunicator, testSpiDriverGetsCorrectBufferLengths)
{
  SpiDriverMock spiDriver;
  mock().expectOneCall("resetAndInit");
  SystemTestCommunicator sysTestCom(spiDriver);

  ViewModelMock viewModel;
  mock().ignoreOtherCalls();
  uint8_t receiveBuffer[Constants::SPI_SYSTEM_TEST_BUFFER_LENGTH] = {};

  mock().expectOneCall("sendReceive")
      .withParameter("target", SpiTarget::SystemTest)
      .withOutputParameterReturning("receiveBuffer", receiveBuffer, sizeof(receiveBuffer))
      .withIntParameter("bufferLength", Constants::SPI_SYSTEM_TEST_BUFFER_LENGTH)
      .ignoreOtherParameters();

  sysTestCom.sendViewModelInfo(viewModel);
}


TEST(systemTestCommunicator, testViewModelSerialisingEmptyViewModel)
{
  SpiDriverMock spiDriver;
  mock().expectOneCall("resetAndInit");
  SystemTestCommunicator sysTestCom(spiDriver);

  ViewModelMock viewModel;
  mock().ignoreOtherCalls();
  uint8_t receiveBuffer[Constants::SPI_SYSTEM_TEST_BUFFER_LENGTH] = {};
  uint8_t sendBuffer[Constants::SPI_SYSTEM_TEST_BUFFER_LENGTH] = {0};

  sendBuffer[0] = Constants::SYSTEM_TEST_COMMAND_TYPE_LED_VIEW;
  sendBuffer[1] = Constants::SYSTEM_TEST_EXPECTED_LED_VIEW_MESSAGE_LENGTH;

  mock().expectOneCall("sendReceive")
      .withParameter("target", SpiTarget::SystemTest)
      .withParameterOfType("sendBuffer", "sendBuffer", sendBuffer)
      .withOutputParameterReturning("receiveBuffer", receiveBuffer, sizeof(receiveBuffer))
      .ignoreOtherParameters();

  sysTestCom.sendViewModelInfo(viewModel);
}


TEST(systemTestCommunicator, testViewModelSerialisingMainScreenInfoComfortHeatOn)
{
  SpiDriverMock spiDriver;
  mock().expectOneCall("resetAndInit");
  SystemTestCommunicator sysTestCom(spiDriver);

  ViewModelDummy viewModel;
  strcpy(viewModel.m_textString, "22");
  viewModel.m_isRGBRedActive = true;
  viewModel.m_isYellowLEDActive = true;

  uint8_t sendBuffer[Constants::SYSTEM_TEST_EXPECTED_LED_VIEW_MESSAGE_LENGTH] = {
      4, 9, 50, 50, 0, 1, 0, 0, 1,
  };

  uint8_t receiveBuffer[Constants::SYSTEM_TEST_EXPECTED_LED_VIEW_MESSAGE_LENGTH] = { };
  mock().expectOneCall("sendReceive")
      .withParameter("target", SpiTarget::SystemTest)
      .withParameterOfType("sendBuffer", "sendBuffer", sendBuffer)
      .withOutputParameterReturning("receiveBuffer", receiveBuffer, sizeof(receiveBuffer))
      .ignoreOtherParameters();

  sysTestCom.sendViewModelInfo(viewModel);
}

TEST(systemTestCommunicator, testViewModelSerialisingMainScreenInfoEconomyHeatOff)
{
  SpiDriverMock spiDriver;
  mock().expectOneCall("resetAndInit");
  SystemTestCommunicator sysTestCom(spiDriver);

  ViewModelDummy viewModel;
  strcpy(viewModel.m_textString, " 7");
  viewModel.m_isRGBGreenActive = true;

  uint8_t sendBuffer[Constants::SYSTEM_TEST_EXPECTED_LED_VIEW_MESSAGE_LENGTH] = {
      4, 9, 32, 55, 0, 0, 1, 0, 0,
  };

  uint8_t receiveBuffer[Constants::SYSTEM_TEST_EXPECTED_LED_VIEW_MESSAGE_LENGTH] = { };
  mock().expectOneCall("sendReceive")
      .withParameter("target", SpiTarget::SystemTest)
      .withParameterOfType("sendBuffer", "sendBuffer", sendBuffer)
      .withOutputParameterReturning("receiveBuffer", receiveBuffer, sizeof(receiveBuffer))
      .ignoreOtherParameters();
  sysTestCom.sendViewModelInfo(viewModel);
}

TEST(systemTestCommunicator, testViewModelSerialisingAllLEDsOn)
{
  SpiDriverMock spiDriver;
  mock().expectOneCall("resetAndInit");
  SystemTestCommunicator sysTestCom(spiDriver);

  ViewModelDummy viewModel;
   strcpy(viewModel.m_textString, "88");
   viewModel.m_isRGBRedActive = true;
   viewModel.m_isRGBGreenActive = true;
   viewModel.m_isRGBBlueActive = true;
   viewModel.m_isYellowLEDActive = true;

   uint8_t sendBuffer[Constants::SYSTEM_TEST_EXPECTED_LED_VIEW_MESSAGE_LENGTH] = {
      4, 9, 56, 56, 0, 1, 1, 1, 1,
   };

  uint8_t receiveBuffer[Constants::SYSTEM_TEST_EXPECTED_LED_VIEW_MESSAGE_LENGTH] = {};
  mock().expectOneCall("sendReceive")
      .withParameter("target", SpiTarget::SystemTest)
      .withParameterOfType("sendBuffer", "sendBuffer", sendBuffer)
      .withOutputParameterReturning("receiveBuffer", receiveBuffer, sizeof(receiveBuffer))
      .ignoreOtherParameters();
  sysTestCom.sendViewModelInfo(viewModel);
}

TEST(systemTestCommunicator, testClearSettingsTrueWhenSpiReceived)
{
  SpiDriverMock spiDriver;
  mock().expectOneCall("resetAndInit");
  SystemTestCommunicator sysTestCom(spiDriver);

  ViewModelMock viewModel;
  uint8_t receiveBuffer[Constants::SPI_SYSTEM_TEST_BUFFER_LENGTH] = {};
  receiveBuffer[2] = 3;
  receiveBuffer[3] = 8;
  receiveBuffer[4] = 0xE2;
  receiveBuffer[5] = 0xEC;
  receiveBuffer[6] = 0x4F;
  receiveBuffer[7] = 0xC2;
  receiveBuffer[8] = 0x5C;
  receiveBuffer[9] = 0xED;
  receiveBuffer[10] = 0x34;
  receiveBuffer[11] = 0xB3;

  mock().expectOneCall("sendReceive")
      .withParameter("target", SpiTarget::SystemTest)
      .withOutputParameterReturning("receiveBuffer", receiveBuffer, sizeof(receiveBuffer))
      .ignoreOtherParameters();

  mock().ignoreOtherCalls();
  sysTestCom.sendViewModelInfo(viewModel);

  ButtonStatus dummyButtonStatus = {};

  CHECK_TRUE(sysTestCom.shouldClearSettings());
}

TEST(systemTestCommunicator, testClearSettingsFalseWhenMagicIsWrong)
{
  SpiDriverMock spiDriver;
  mock().expectOneCall("resetAndInit");
  SystemTestCommunicator sysTestCom(spiDriver);

  ViewModelMock viewModel;
  uint8_t receiveBuffer[Constants::SPI_SYSTEM_TEST_BUFFER_LENGTH] = {};
  receiveBuffer[2] = 3;
  receiveBuffer[3] = 8;
  receiveBuffer[4] = 0xE2;
  receiveBuffer[5] = 0xEC;
  receiveBuffer[6] = 0x4F;
  receiveBuffer[7] = 0xC3;
  receiveBuffer[8] = 0x5C;
  receiveBuffer[9] = 0xED;
  receiveBuffer[10] = 0x34;
  receiveBuffer[11] = 0xB3;

  mock().expectOneCall("sendReceive")
      .withParameter("target", SpiTarget::SystemTest)
      .withOutputParameterReturning("receiveBuffer", receiveBuffer, sizeof(receiveBuffer))
      .ignoreOtherParameters();
  mock().ignoreOtherCalls();
  sysTestCom.sendViewModelInfo(viewModel);

  ButtonStatus dummyButtonStatus = {};

  CHECK_FALSE(sysTestCom.shouldClearSettings());
}

TEST(systemTestCommunicator, testClearSettingsFalseWheMessageNotReceived)
{
  SpiDriverMock spiDriver;
  mock().expectOneCall("resetAndInit");
  SystemTestCommunicator sysTestCom(spiDriver);

  ViewModelMock viewModel;
  uint8_t receiveBuffer[Constants::SPI_SYSTEM_TEST_BUFFER_LENGTH] = {};
  receiveBuffer[2] = 50; /* Unknown message */
  receiveBuffer[3] = 8;
  receiveBuffer[4] = 0xE2;
  receiveBuffer[5] = 0xEC;
  receiveBuffer[6] = 0x4F;
  receiveBuffer[7] = 0xC2;
  receiveBuffer[8] = 0x5C;
  receiveBuffer[9] = 0xED;
  receiveBuffer[10] = 0x34;
  receiveBuffer[11] = 0xB3;

  mock().expectOneCall("sendReceive")
      .withParameter("target", SpiTarget::SystemTest)
      .withOutputParameterReturning("receiveBuffer", receiveBuffer, sizeof(receiveBuffer))
      .ignoreOtherParameters();
  mock().ignoreOtherCalls();
  sysTestCom.sendViewModelInfo(viewModel);

  ButtonStatus dummyButtonStatus = {};

  CHECK_FALSE(sysTestCom.shouldClearSettings());
}

TEST(systemTestCommunicator, testSimulatedStandbySwitchReturnsFalseWhenSimulatedSwitchIsFalse)
{
  SpiDriverMock spiDriver;
  mock().expectOneCall("resetAndInit");
  SystemTestCommunicator sysTestCom(spiDriver);

  ViewModelMock viewModel;
  uint8_t receiveBuffer[Constants::SPI_SYSTEM_TEST_BUFFER_LENGTH] = {};
  memset(receiveBuffer, 0, sizeof(receiveBuffer));
  receiveBuffer[2] = Constants::SYSTEM_TEST_COMMAND_TYPE_BUTTON;
  receiveBuffer[3] = BUTTON_MSG_LENGTH;
  receiveBuffer[22] = 0;

  mock().expectOneCall("sendReceive")
      .withParameter("target", SpiTarget::SystemTest)
      .withOutputParameterReturning("receiveBuffer", receiveBuffer, sizeof(receiveBuffer))
      .ignoreOtherParameters();
  mock().ignoreOtherCalls();
  sysTestCom.sendViewModelInfo(viewModel);

  CHECK_FALSE(sysTestCom.isSimulatedStandbySwitchActive());
}

TEST(systemTestCommunicator, testSimulatedStandbySwitchReturnsTrueWhenSimulatedSwitchIsTrue)
{
  SpiDriverMock spiDriver;
  mock().expectOneCall("resetAndInit");
  SystemTestCommunicator sysTestCom(spiDriver);

  ViewModelMock viewModel;
  uint8_t receiveBuffer[Constants::SPI_SYSTEM_TEST_BUFFER_LENGTH] = {};
  receiveBuffer[2] = Constants::SYSTEM_TEST_COMMAND_TYPE_BUTTON;
  receiveBuffer[3] = BUTTON_MSG_LENGTH;
  receiveBuffer[22] = 1;

  mock().expectOneCall("sendReceive")
      .withParameter("target", SpiTarget::SystemTest)
      .withOutputParameterReturning("receiveBuffer", receiveBuffer, sizeof(receiveBuffer))
      .ignoreOtherParameters();
  mock().ignoreOtherCalls();
  sysTestCom.sendViewModelInfo(viewModel);

  CHECK_TRUE(sysTestCom.isSimulatedStandbySwitchActive());
}
