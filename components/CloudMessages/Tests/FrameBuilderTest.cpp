#include "CppUTest/TestHarness.h"
#include "FrameBuilder.h"


static FrameBuilder * frame;

TEST_GROUP(FrameBuilder)
{
  TEST_SETUP()
  {
    frame = new FrameBuilder();
  }

  TEST_TEARDOWN()
  {
    delete frame;
  }
};

TEST(FrameBuilder, testThatSizeIsInitiallyZero)
{
  CHECK_EQUAL(0U, frame->getSize());
}

TEST(FrameBuilder, testReadReturnsZeroBytesWhenNothingToReturn)
{
  uint8_t buffer[1] = {};
  uint32_t bytesReturned = frame->getBytes(buffer, sizeof(buffer));
  CHECK_EQUAL(0U, bytesReturned);
}

TEST(FrameBuilder, testAddUInt16AsMSB)
{
  bool result = frame->addUInt16AsMSB(1243U); // Hex: 0x04DB. MSB is 0x04.
  CHECK_TRUE(result);

  uint8_t expectedData[] = { 0x04, 0xDB };
  uint8_t buffer[2] = {};
  uint32_t bytesReturned = frame->getBytes(buffer, sizeof(buffer));

  CHECK_EQUAL(2U, bytesReturned);
  MEMCMP_EQUAL(expectedData, buffer, 2U);
}

TEST(FrameBuilder, testBytesReturnedEqualsAvailableBytesIfBufferIsLarger)
{
  bool result = frame->addUInt16AsMSB(1243U); // Hex: 0x04DB. MSB is 0x04.
  CHECK_TRUE(result);

  uint8_t expectedData[] = { 0x04, 0xDB };

  uint8_t buffer[3] = {};
  uint32_t bytesReturned = frame->getBytes(buffer, sizeof(buffer));

  CHECK_EQUAL(2U, bytesReturned);
  MEMCMP_EQUAL(expectedData, buffer, 2U);
}

TEST(FrameBuilder, testBytesReturnedEqualsBufferSizeIfAvailableBytesIsLarger)
{
  bool result = frame->addUInt16AsMSB(1243U); // Hex: 0x04DB. MSB is 0x04.
  CHECK_TRUE(result);

  uint8_t expectedData[] = { 0x04 };

  uint8_t buffer[1] = {};
  uint32_t bytesReturned = frame->getBytes(buffer, sizeof(buffer));

  CHECK_EQUAL(1U, bytesReturned);
  MEMCMP_EQUAL(expectedData, buffer, 1U);
}

TEST(FrameBuilder, testAddUInt16AsLSB)
{
  bool result = frame->addUInt16AsLSB(1243U); // Hex: 0x04DB. MSB is 0x04.
  CHECK_TRUE(result);

  uint8_t expectedData[] = { 0xDB, 0x04 };
  uint8_t buffer[2] = {};
  uint32_t bytesReturned = frame->getBytes(buffer, sizeof(buffer));

  CHECK_EQUAL(2U, bytesReturned);
  MEMCMP_EQUAL(expectedData, buffer, 2U);
}

TEST(FrameBuilder, testAddUInt8)
{
  bool result = frame->addUInt8(123U); // Hex: 0x7B.
  CHECK_TRUE(result);

  uint8_t expectedData[] = { 0x7B };
  uint8_t buffer[1] = {};
  uint32_t bytesReturned = frame->getBytes(buffer, sizeof(buffer));

  CHECK_EQUAL(1U, bytesReturned);
  MEMCMP_EQUAL(expectedData, buffer, 1U);
}

TEST(FrameBuilder, testAddBytes)
{
  uint8_t bytes[] = { 5U, 4U, 3U, 2U, 1U };
  bool result = frame->addBytes(bytes, sizeof(bytes));
  CHECK_TRUE(result);

  uint8_t readBuffer[5] = {};
  uint32_t bytesReturned = frame->getBytes(readBuffer, sizeof(readBuffer));

  CHECK_EQUAL(5U, bytesReturned);
  MEMCMP_EQUAL(bytes, readBuffer, bytesReturned);
}

TEST(FrameBuilder, testAddFailsIfMaxSizeExceeeded)
{
  uint8_t bytes[Constants::CLOUD_RESPONSE_MSG_MAX_SIZE] = {};
  CHECK_TRUE(frame->addBytes(bytes, sizeof(bytes)));
  CHECK_FALSE(frame->addUInt8(1U));
}


TEST_GROUP(FrameBuilderModify)
{
  TEST_SETUP()
  {
    frame = new FrameBuilder();
    frame->addUInt8(1U);
    frame->addUInt8(2U);
    frame->addUInt8(3U);
    frame->addUInt8(4U);
    frame->addUInt8(5U);
    CHECK_EQUAL(5U, frame->getSize());
  }

  TEST_TEARDOWN()
  {
    delete frame;
  }
};

TEST(FrameBuilderModify, testModifyUInt8Index0)
{
  bool result = frame->modifyUInt8(0U, 11U);
  CHECK_TRUE(result);

  uint8_t expectedData[] = { 11U, 2U, 3U, 4U, 5U };
  uint8_t buffer[5];
  frame->getBytes(buffer, sizeof(buffer));
  MEMCMP_EQUAL(expectedData, buffer, sizeof(buffer))
}

TEST(FrameBuilderModify, testModifyUInt8DoNotAlterSize)
{
  uint32_t origSize = frame->getSize();
  frame->modifyUInt8(0U, 11U);
  CHECK_EQUAL(origSize, frame->getSize());
}

TEST(FrameBuilderModify, testModifyUInt8Index4)
{
  bool result = frame->modifyUInt8(4U, 127U);
  CHECK_TRUE(result);

  uint8_t expectedData[] = { 1U, 2U, 3U, 4U, 127U };
  uint8_t buffer[5];
  frame->getBytes(buffer, sizeof(buffer));
  MEMCMP_EQUAL(expectedData, buffer, sizeof(buffer))
}

TEST(FrameBuilderModify, testModifyUInt8OutsideBoundsFails)
{
  bool result = frame->modifyUInt8(5U, 0U);
  CHECK_FALSE(result);
}

TEST(FrameBuilderModify, testAddAfterModifyUInt8)
{
  frame->modifyUInt8(0U, 11U);
  CHECK_TRUE(frame->addUInt8(10U));
  CHECK_EQUAL(6U, frame->getSize());

  uint8_t expectedData[] = { 11U, 2U, 3U, 4U, 5U, 10U };
  uint8_t buffer[6];
  frame->getBytes(buffer, sizeof(buffer));
  MEMCMP_EQUAL(expectedData, buffer, sizeof(buffer))
}

TEST(FrameBuilderModify, testModifyUInt16AsMSBIndex0)
{
  bool result = frame->modifyUInt16AsMSB(0U, 500U); // 0x01f4
  CHECK_TRUE(result);

  uint8_t expectedData[] = { 0x01U, 0xf4U, 3U, 4U, 5U };
  uint8_t buffer[5];
  frame->getBytes(buffer, sizeof(buffer));
  MEMCMP_EQUAL(expectedData, buffer, sizeof(buffer))
}

TEST(FrameBuilderModify, testModifyUInt16AsMSBDoNotAlterSize)
{
  uint32_t origSize = frame->getSize();
  frame->modifyUInt16AsMSB(0U, 500U);
  CHECK_EQUAL(origSize, frame->getSize());
}

TEST(FrameBuilderModify, testModifyUInt16AsMSBIndex3)
{
  bool result = frame->modifyUInt16AsMSB(3U, 10000U); // 0x2710
  CHECK_TRUE(result);

  uint8_t expectedData[] = { 1U, 2U, 3U, 0x27U, 0x10U };
  uint8_t buffer[5];
  frame->getBytes(buffer, sizeof(buffer));
  MEMCMP_EQUAL(expectedData, buffer, sizeof(buffer))
}

TEST(FrameBuilderModify, testModifyUInt16AsMSBOutsideBoundsFails)
{
  bool result = frame->modifyUInt16AsMSB(4U, 0U);
  CHECK_FALSE(result);
}

TEST(FrameBuilderModify, testAddAfterModifyUInt16AsMSB)
{
  frame->modifyUInt16AsMSB(0U, 500U);
  CHECK_TRUE(frame->addUInt8(10U));
  CHECK_EQUAL(6U, frame->getSize());

  uint8_t expectedData[] = { 0x01U, 0xf4U, 3U, 4U, 5U, 10U };
  uint8_t buffer[6];
  frame->getBytes(buffer, sizeof(buffer));
  MEMCMP_EQUAL(expectedData, buffer, sizeof(buffer))
}

TEST(FrameBuilderModify, testModifyUInt16AsLSBIndex0)
{
  bool result = frame->modifyUInt16AsLSB(0U, 500U); // 0x01f4
  CHECK_TRUE(result);

  uint8_t expectedData[] = { 0xf4U, 0x01U, 3U, 4U, 5U };
  uint8_t buffer[5];
  frame->getBytes(buffer, sizeof(buffer));
  MEMCMP_EQUAL(expectedData, buffer, sizeof(buffer))
}

TEST(FrameBuilderModify, testModifyUInt16AsLSBDoNotAlterSize)
{
  uint32_t origSize = frame->getSize();
  frame->modifyUInt16AsLSB(0U, 500U);
  CHECK_EQUAL(origSize, frame->getSize());
}

TEST(FrameBuilderModify, testModifyUInt16AsLSBIndex3)
{
  bool result = frame->modifyUInt16AsLSB(3U, 10000U); // 0x2710
  CHECK_TRUE(result);

  uint8_t expectedData[] = { 1U, 2U, 3U, 0x10U, 0x27U };
  uint8_t buffer[5];
  frame->getBytes(buffer, sizeof(buffer));
  MEMCMP_EQUAL(expectedData, buffer, sizeof(buffer))
}

TEST(FrameBuilderModify, testModifyUInt16AsLSBOutsideBoundsFails)
{
  bool result = frame->modifyUInt16AsLSB(40U, 0U);
  CHECK_FALSE(result);
}

TEST(FrameBuilderModify, testAddAfterModifyUInt16AsLSB)
{
  frame->modifyUInt16AsLSB(0U, 500U);
  CHECK_TRUE(frame->addUInt8(10U));
  CHECK_EQUAL(6U, frame->getSize());

  uint8_t expectedData[] = { 0xf4U, 0x01U, 3U, 4U, 5U, 10U };
  uint8_t buffer[6];
  frame->getBytes(buffer, sizeof(buffer));
  MEMCMP_EQUAL(expectedData, buffer, sizeof(buffer))
}
