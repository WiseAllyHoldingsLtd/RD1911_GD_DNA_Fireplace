#include "CppUTest/TestHarness.h"

#include "ParameterDataRequest.h"


static FrameParser * frame;
static ParameterDataRequest * paramData;

TEST_GROUP(ParameterDataRequestInvalidData)
{
};

TEST(ParameterDataRequestInvalidData, testFrameIDMustMatch)
{
  uint8_t data[] = {
      0x00, 0xef, // invalid frameID
      0x01, 0x00, // paramID = 1
      0x01, 0x00, // paramSize = 1, paramData = 0
      0x00, 0x00  // endOfFrame
  };

  FrameParser parser(data, sizeof(data));
  ParameterDataRequest pFrame(parser);

  CHECK_FALSE(pFrame.isValid());
}

TEST(ParameterDataRequestInvalidData, testFrameMinimumLength)
{
  uint8_t data[] = {
      0x20, 0x00, // frameID
      0x01, 0x00,  // paramID = 1
                  // no param data
      0x00, 0x00  // endOfFrame
  };

  FrameParser parser(data, sizeof(data));
  ParameterDataRequest pFrame(parser);

  CHECK_FALSE(pFrame.isValid());
}

TEST(ParameterDataRequestInvalidData, testFrameWithIncorrectEndOfFrame)
{
  uint8_t data[] = {
      0x20, 0x00, // frameID
      0x01, 0x00, // paramID = 1
      0x01, 0x00, // paramSize = 1, paramData = 0
      0x02, 0x00  // endOfFrame invalid
  };

  FrameParser parser(data, sizeof(data));
  ParameterDataRequest pFrame(parser);

  CHECK_FALSE(pFrame.isValid());
}


TEST_GROUP(ParameterDataRequestValidSingleParameter)
{
  TEST_SETUP()
  {
    uint8_t data[] = {
        0x20, 0x00, // frameID (LSB)
        0x1b, 0x01, // paramID = 011B,
        0x12,       // paramSize 0x12 = 18
        0x1a, 0x2b, 0x3c, 0x4d, 0x5e,
        0x12, 0x34, 0x56, 0x78, 0x90,
        0x00, 0x22, 0x44, 0x66, 0x88,
        0xff, 0xef, 0xdf,
        0x00, 0x00  // endOfFrame
    };

    frame = new FrameParser(data, sizeof(data));
    paramData = new ParameterDataRequest(*frame);
  }

  TEST_TEARDOWN()
  {
    delete paramData;
    delete frame;
  }
};

TEST(ParameterDataRequestValidSingleParameter, testValidation)
{
  CHECK_TRUE(paramData->isValid());
}

TEST(ParameterDataRequestValidSingleParameter, testNumberOfParamData)
{
  CHECK_EQUAL(1U, paramData->getNumOfParamDataItems());
}

TEST(ParameterDataRequestValidSingleParameter, testParamDataID)
{
  CHECK_EQUAL(0x011b, paramData->getParamDataID(0U));
}

TEST(ParameterDataRequestValidSingleParameter, testParamDataContents)
{
  uint8_t expectedBytes[] = { 0x1a, 0x2b, 0x3c, 0x4d, 0x5e, 0x12, 0x34, 0x56, 0x78, 0x90, 0x00, 0x22, 0x44, 0x66, 0x88, 0xff, 0xef, 0xdf };

  uint8_t paramSize = paramData->getParamDataSize(0U);
  uint8_t * paramBytes = new uint8_t[paramSize];
  uint8_t returnedBytes = paramData->getParamData(0U, paramBytes, paramSize);

  CHECK_EQUAL(18U, paramSize);
  CHECK_EQUAL(18U, returnedBytes);
  MEMCMP_EQUAL(expectedBytes, paramBytes, 18U);

  delete [] paramBytes;
}


TEST_GROUP(ParameterDataRequestValidSeveralParameters)
{
  TEST_SETUP()
  {
    uint8_t data[] = {
        0x20, 0x00, // frameID (LSB)
        0x1c, 0x01, // paramID#0
        0x0f,       // paramSize#0
        0x1a, 0x2b, 0x3c, 0x4d, 0x5e, 0x12, 0x34, 0x56, 0x78, 0x90, 0x00, 0x22, 0x44, 0x66, 0x88,
        0x04, 0x00, // paramID#1
        0x03,  // paramSize#1
        0xef, 0xa1, 0x00,
        0x04, 0x21, // paramID#2
        0x08,  // paramSize#2
        0x14, 0x01, 0xa0, 0x04, 0x11, 0xa0, 0x33, 0x34,
        0x00, 0x00  // endOfFrame
    };

    frame = new FrameParser(data, sizeof(data));
    paramData = new ParameterDataRequest(*frame);
  }

  TEST_TEARDOWN()
  {
    delete paramData;
    delete frame;
  }
};

TEST(ParameterDataRequestValidSeveralParameters, testTwoParametersFirstOfZeroSize)
{
  // special case 1: empty paramData with non-empty after
  uint8_t data[] = {
      0x20, 0x00,
      0x12, 0x34, // paramID#0
      0x00,       // paramSize#0
      0x56, 0x78, // paramID#1
      0x01,       // paramSize#1
      0x02,       // paramData#1
      0x00, 0x00  // endOfFrame
  };

  FrameParser theParser(data, sizeof(data));
  ParameterDataRequest theRequest(theParser);

  CHECK_TRUE(theRequest.isValid());
  CHECK_EQUAL(2U, theRequest.getNumOfParamDataItems());
}

TEST(ParameterDataRequestValidSeveralParameters, testTwoParametersLastOfZeroSize)
{
  // special case 2: non-empty paramData with empty after
  uint8_t data[] = {
      0x20, 0x00,
      0x12, 0x34, // paramID#0
      0x02,       // paramSize#0
      0x01, 0x02, // paramData#0
      0x56, 0x78, // paramID#1
      0x00,       // paramSize#1
      0x00, 0x00  // endOfFrame
  };

  FrameParser theParser(data, sizeof(data));
  ParameterDataRequest theRequest(theParser);

  CHECK_TRUE(theRequest.isValid());
  CHECK_EQUAL(2U, theRequest.getNumOfParamDataItems());
}

TEST(ParameterDataRequestValidSeveralParameters, testValidation)
{
  CHECK_TRUE(paramData->isValid());
}

TEST(ParameterDataRequestValidSeveralParameters, testNumberOfPayloadFrames)
{
  CHECK_EQUAL(3U, paramData->getNumOfParamDataItems());
}

TEST(ParameterDataRequestValidSeveralParameters, testPayloadFrameData)
{
  uint8_t expectedBytes0[] = { 0x1a, 0x2b, 0x3c, 0x4d, 0x5e, 0x12, 0x34, 0x56, 0x78, 0x90, 0x00, 0x22, 0x44, 0x66, 0x88 };
  uint8_t expectedBytes1[] = { 0xef, 0xa1, 0x00 };
  uint8_t expectedBytes2[] = { 0x14, 0x01, 0xa0, 0x04, 0x11, 0xa0, 0x33, 0x34 };

  // Param #0
  uint8_t paramNo = 0U;
  uint16_t paramID = paramData->getParamDataID(paramNo);
  uint8_t paramSize = paramData->getParamDataSize(paramNo);
  uint8_t *paramBytes = new uint8_t[paramSize];
  uint8_t returnedBytes = paramData->getParamData(paramNo, paramBytes, paramSize);
  CHECK_EQUAL(0x011c, paramID);
  CHECK_EQUAL(15U, paramSize);
  CHECK_EQUAL(15U, returnedBytes);
  MEMCMP_EQUAL(expectedBytes0, paramBytes, 15U);
  delete [] paramBytes;

  // Param #1
  paramNo = 1U;
  paramID = paramData->getParamDataID(paramNo);
  paramSize = paramData->getParamDataSize(paramNo);
  paramBytes = new uint8_t[paramSize];
  returnedBytes = paramData->getParamData(paramNo, paramBytes, paramSize);
  CHECK_EQUAL(0x0004, paramID);
  CHECK_EQUAL(3U, paramSize);
  CHECK_EQUAL(3U, returnedBytes);
  MEMCMP_EQUAL(expectedBytes1, paramBytes, 3U);
  delete [] paramBytes;

  // Param #2
  paramNo = 2U;
  paramID = paramData->getParamDataID(paramNo);
  paramSize = paramData->getParamDataSize(paramNo);
  paramBytes = new uint8_t[paramSize];
  returnedBytes = paramData->getParamData(paramNo, paramBytes, paramSize);
  CHECK_EQUAL(0x2104, paramID);
  CHECK_EQUAL(8U, paramSize);
  CHECK_EQUAL(8U, returnedBytes);
  MEMCMP_EQUAL(expectedBytes2, paramBytes, 8U);
  delete [] paramBytes;
}
