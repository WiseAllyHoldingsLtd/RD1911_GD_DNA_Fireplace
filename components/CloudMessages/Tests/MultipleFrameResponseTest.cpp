#include "CppUTest/TestHarness.h"
#include "ParameterDataResponse.h"
#include "MultipleFrameResponse.h"


MultipleFrameResponse * multipleFrame;


TEST_GROUP(MultipleFrameResponse)
{
  TEST_SETUP()
  {
    multipleFrame = new MultipleFrameResponse();
  }

  TEST_TEARDOWN()
  {
    delete multipleFrame;
  }
};

TEST(MultipleFrameResponse, testSizeAndNumberOfFramesAfterInit)
{
  CHECK_EQUAL(4U, multipleFrame->getSize());
  CHECK_EQUAL(0U, multipleFrame->getNumberOfFrames());
}


TEST(MultipleFrameResponse, testFrameIDAndNumOfFramesAfterInit)
{
  uint8_t buffer[4];
  multipleFrame->getBytes(buffer, sizeof(buffer));

  uint8_t expectedBytes[] = {
      0x00, 0x23, // FrameType = 0x0023
      0x00, 0x00  // NumOfFrames = 0, none has been added
  };

  MEMCMP_EQUAL(expectedBytes, buffer, sizeof(buffer));
}

TEST(MultipleFrameResponse, testAddParameterData)
{
  uint8_t paramDataBytes[] = { 0xab, 0x00, 0xcd, 0x11 };

  ParameterDataResponse paramData;
  paramData.addParameterData(0x2143, paramDataBytes, sizeof(paramDataBytes)); // total size: FrameID, ParamID,size,data = 9. EndOfBytes=2. => Total 11.
  paramData.finalize();

  bool result = multipleFrame->addParameterDataResponse(paramData);
  CHECK_TRUE(result);
  CHECK_EQUAL(1U, multipleFrame->getNumberOfFrames());

  const uint32_t expectedSize = 16U; // initSize: 4, paramDataSize: 12.
  uint32_t responseSize = multipleFrame->getSize();
  CHECK_EQUAL(expectedSize, responseSize);

  uint8_t buffer[expectedSize];
  multipleFrame->getBytes(buffer, expectedSize);

  uint8_t expectedBytes[expectedSize] = {
      0x00, 0x23, // FrameID
      0x01, 0x00, // NumFrames, one has been added
      0x0b,       // Frame Size
      0x21, 0x00, // FrameID, paramDataFrame
      0x43, 0x21, // paramDataID
      0x04, 0xab, 0x00, 0xcd, 0x11, // Size + Data, paramData
      0x00, 0x00
  };

  MEMCMP_EQUAL(expectedBytes, buffer, expectedSize);
}

TEST(MultipleFrameResponse, testAddTwoParameterDataFrames)
{
  /* Add first parameterFrame */
  uint8_t paramDataBytesOne[] = { 0xab, 0x00, 0xcd, 0x11 };
  ParameterDataResponse paramDataOne;
  paramDataOne.addParameterData(0x2143, paramDataBytesOne, sizeof(paramDataBytesOne)); // total size: FrameID, ParamID,size,data = 9. EndOfBytes=2. => Total 11.
  paramDataOne.finalize();

  multipleFrame->addParameterDataResponse(paramDataOne);
  CHECK_EQUAL(1U, multipleFrame->getNumberOfFrames());

  /* Add second parameterFrame */
  uint8_t paramDataBytesTwo[] = { 0xcd, 0xff, 0xab, 0x11, 0x22 };
  ParameterDataResponse paramDataTwo;
  paramDataTwo.addParameterData(0x2244, paramDataBytesTwo, sizeof(paramDataBytesTwo)); // total size: FrameID, ParamID,size,data = 10. EndOfBytes=2. => Total 12.
  paramDataTwo.finalize();

  multipleFrame->addParameterDataResponse(paramDataTwo);
  CHECK_EQUAL(2U, multipleFrame->getNumberOfFrames());


  const uint32_t expectedSize = 29U; // initSize: 4, paramDataSizeOne: 11 + 1 size byte, paramDataSizeTwo: 12 + 1 size byte.
  uint32_t responseSize = multipleFrame->getSize();
  CHECK_EQUAL(expectedSize, responseSize);

  uint8_t buffer[expectedSize];
  multipleFrame->getBytes(buffer, expectedSize);

  uint8_t expectedBytes[expectedSize] = {
      0x00, 0x23, // FrameID
      0x02, 0x00, // NumFrames, one has been added
      0x0b,       // Frame Size # 1
      0x21, 0x00, // Frame # 1
      0x43, 0x21,
      0x04, 0xab, 0x00, 0xcd, 0x11,
      0x00, 0x00,
      0x0c,       // Frame Size #2
      0x21, 0x00,
      0x44, 0x22,
      0x05, 0xcd, 0xff, 0xab, 0x11, 0x22,
      0x00, 0x00
  };

  MEMCMP_EQUAL(expectedBytes, buffer, expectedSize);
}

