#include "CppUTest/TestHarness.h"
#include "ParameterDataResponse.h"


ParameterDataResponse * paramData;


TEST_GROUP(ParameterDataResponse)
{
  TEST_SETUP()
  {
    paramData = new ParameterDataResponse();
  }

  TEST_TEARDOWN()
  {
    delete paramData;
  }
};

TEST(ParameterDataResponse, testSizeAfterInit)
{
  CHECK_EQUAL(2U, paramData->getSize());
}

TEST(ParameterDataResponse, testSizeAfterInitAndFinalizing)
{
  CHECK_TRUE(paramData->finalize());
  CHECK_EQUAL(4U, paramData->getSize());
}

TEST(ParameterDataResponse, finalizingTwiceDoesNothing)
{
  CHECK_TRUE(paramData->finalize());
  CHECK_TRUE(paramData->finalize()); // does nothing, already finalized
  CHECK_EQUAL(4U, paramData->getSize());
}

TEST(ParameterDataResponse, testFrameIDAfterInit)
{
  uint8_t buffer[2];
  paramData->getBytes(buffer, sizeof(buffer));

  uint8_t expectedBytes[] = { 0x21, 0x00 };
  MEMCMP_EQUAL(expectedBytes, buffer, sizeof(buffer));
}

TEST(ParameterDataResponse, finalizingAddsEndOfFrameBytesAtTheEnd)
{
  paramData->finalize();

  uint8_t buffer[4];
  paramData->getBytes(buffer, sizeof(buffer));

  uint8_t expectedBytes[] = { 0x21, 0x00, 0x00, 0x00 };
  MEMCMP_EQUAL(expectedBytes, buffer, sizeof(buffer));
}

TEST(ParameterDataResponse, addSingleParameterDataItem)
{
  uint8_t dataBuffer[] = { 0x01, 0x02, 0x03 };
  uint16_t dataID = 0x2a31;
  bool result = paramData->addParameterData(dataID, dataBuffer, sizeof(dataBuffer));
  CHECK_TRUE(result);

  uint8_t outputData[8]; // frameID(2) + paramID(2) + size(1) + data(3)
  paramData->getBytes(outputData, sizeof(outputData));

  uint8_t expectedBytes[] = { 0x21, 0x00, 0x31, 0x2a, 3U, 0x01, 0x02, 0x03 };
  MEMCMP_EQUAL(expectedBytes, outputData, sizeof(outputData));
}

TEST(ParameterDataResponse, addParameterAfterFinalizingFails)
{
  paramData->finalize();
  uint8_t buffer[] = { 1U };

  bool result = paramData->addParameterData(0U, buffer, sizeof(buffer));
  CHECK_FALSE(result);
}

TEST(ParameterDataResponse, addTwoParametersThenFinalize)
{
  uint16_t paramOneID = 0x1213;
  uint8_t paramOne[] = { 0xab, 0xbc, 0xcd };

  uint16_t paramTwoID = 0x1122;
  uint8_t paramTwo[] = { 0x11, 0x22, 0x33, 0x44 };

  paramData->addParameterData(paramOneID, paramOne, sizeof(paramOne));
  paramData->addParameterData(paramTwoID, paramTwo, sizeof(paramTwo));
  paramData->finalize();

  uint8_t outputData[17]; // frameID(2) + 2*paramID(4) + 2*size(2) + data(3+4) + endOfFrame(2)
  paramData->getBytes(outputData, sizeof(outputData));

  uint8_t expectedBytes[] = { 0x21, 0x00, 0x13, 0x12, 3U, 0xab, 0xbc, 0xcd, 0x22, 0x11, 4U, 0x11, 0x22, 0x33, 0x44, 0U, 0U };
  MEMCMP_EQUAL(expectedBytes, outputData, sizeof(outputData));
}

TEST(ParameterDataResponse, addParameterOfSizeZeroIsOK)
{
  bool result = paramData->addParameterData(0U, nullptr, 0U);
  CHECK_TRUE(result);

  uint8_t outputData[5]; // frameID(2) + paramID(2) + size(1)
  paramData->getBytes(outputData, sizeof(outputData));

  uint8_t expectedBytes[] = { 0x21, 0x00, 0x00, 0x00, 0U };
  MEMCMP_EQUAL(expectedBytes, outputData, sizeof(outputData));
}
