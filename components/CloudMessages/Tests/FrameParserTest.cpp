#include "FrameParser.h"

#include "CppUTest/TestHarness.h"



static FrameParser * frame;

TEST_GROUP(basicFrameTests)
{
  TEST_SETUP()
  {
  }

  TEST_TEARDOWN()
  {
  }
};

TEST(basicFrameTests, testThatSizeIsSetCorrectly)
{
  uint8_t data[4U] = { 0x01, 0x02, 0x03, 0x04 };
  uint32_t size = sizeof(data);

  FrameParser frame(data, size);
  CHECK_EQUAL(size, frame.getSize());
}

TEST(basicFrameTests, testThatDefaultConstructorGivesZeroSize)
{
  FrameParser frame;
  CHECK_EQUAL(0U, frame.getSize());
}

TEST(basicFrameTests, testThatZeroSizeIsOK)
{
  uint8_t data[1U] = {};
  uint32_t size = 0U;

  FrameParser frame(data, size);
  CHECK_EQUAL(size, frame.getSize());
}

TEST(basicFrameTests, testThatSizeAboveLimitGivesZero)
{
  uint8_t data[FRAME_MAX_SIZE + 1U] = {};
  uint32_t size = sizeof(data);

  FrameParser frame(data, size);
  CHECK_EQUAL(0U, frame.getSize());
}


TEST_GROUP(frameDataTests)
{
  TEST_SETUP()
  {
    uint8_t data[8U] = { 0x01, 0x02, 0x03, 0x09, 0x08, 0x07, 0x0a, 0xff };
    uint32_t size = sizeof(data);
    frame = new FrameParser(data, size);
  }

  TEST_TEARDOWN()
  {
    delete frame;
  }
};

TEST(frameDataTests, testFrameIDIsTwoFirstBytesMSB)
{
  // 0x01 0x02 as MSB should give 0x0102
  CHECK_EQUAL(8U, frame->getSize());
  CHECK_EQUAL(258U, frame->getID());
}

TEST(frameDataTests, testFrameIDIsZeroIfSizeTooSmall)
{
  uint8_t data[1U] = { 0x02 };
  FrameParser theFrame(data, 1U);
  CHECK_EQUAL(0U, theFrame.getID());
}

TEST(frameDataTests, testGetUInt8ReturnsCorrectValue)
{
  CHECK_EQUAL(0x01, frame->getUInt8(0U));
  CHECK_EQUAL(0x09, frame->getUInt8(3U));
  CHECK_EQUAL(0xff, frame->getUInt8(7U));
}

TEST(frameDataTests, testGetUInt8ReturnsZeroIfOutsideBounds)
{
  CHECK_EQUAL(0, frame->getUInt8(frame->getSize()));
}

TEST(frameDataTests, testGetUInt16ReturnsCorrectValueLSB)
{
  CHECK_EQUAL(0x0201, frame->getUInt16AsLSB(0U));
  CHECK_EQUAL(0x0302, frame->getUInt16AsLSB(1U));
  CHECK_EQUAL(0x0708, frame->getUInt16AsLSB(4U));
  CHECK_EQUAL(0xff0a, frame->getUInt16AsLSB(6U));
}

TEST(frameDataTests, testGetUInt16ReturnsZeroIfOutsideBoundsLSB)
{
  CHECK_EQUAL(0, frame->getUInt16AsLSB(frame->getSize()));
  CHECK_EQUAL(0, frame->getUInt16AsLSB(frame->getSize() - 1U));
}

TEST(frameDataTests, testGetUInt16ReturnsCorrectValueMSB)
{
  CHECK_EQUAL(0x0102, frame->getUInt16AsMSB(0U));
  CHECK_EQUAL(0x0203, frame->getUInt16AsMSB(1U));
  CHECK_EQUAL(0x0807, frame->getUInt16AsMSB(4U));
  CHECK_EQUAL(0x0aff, frame->getUInt16AsMSB(6U));
}

TEST(frameDataTests, testGetUInt16ReturnsZeroIfOutsideBoundsMSB)
{
  CHECK_EQUAL(0, frame->getUInt16AsMSB(frame->getSize()));
  CHECK_EQUAL(0, frame->getUInt16AsMSB(frame->getSize() - 1U));
}

TEST(frameDataTests, testGetUInt32ReturnsCorrectValueLSB)
{
  CHECK_EQUAL(0x09030201, frame->getUInt32AsLSB(0U));
  CHECK_EQUAL(0x08090302, frame->getUInt32AsLSB(1U));
  CHECK_EQUAL(0xff0a0708, frame->getUInt32AsLSB(4U));
}

TEST(frameDataTests, testGetUInt32ReturnsZeroIfOutsideBoundsLSB)
{
  CHECK_EQUAL(0, frame->getUInt32AsLSB(frame->getSize()));
  CHECK_EQUAL(0, frame->getUInt32AsLSB(frame->getSize() - 1U));
  CHECK_EQUAL(0, frame->getUInt32AsLSB(frame->getSize() - 3U));
}

TEST(frameDataTests, testGetUInt32ReturnsCorrectValueMSB)
{
  CHECK_EQUAL(0x01020309, frame->getUInt32AsMSB(0U));
  CHECK_EQUAL(0x02030908, frame->getUInt32AsMSB(1U));
  CHECK_EQUAL(0x08070aff, frame->getUInt32AsMSB(4U));
}

TEST(frameDataTests, testGetUInt32ReturnsZeroIfOutsideBoundsMSB)
{
  CHECK_EQUAL(0, frame->getUInt32AsMSB(frame->getSize()));
  CHECK_EQUAL(0, frame->getUInt32AsMSB(frame->getSize() - 1U));
  CHECK_EQUAL(0, frame->getUInt32AsMSB(frame->getSize() - 3U));
}

TEST(frameDataTests, testGetInt32ReturnsCorrectValueLSB)
{
  CHECK_EQUAL(0x09030201, frame->getInt32AsLSB(0U));
  CHECK_EQUAL(0x08090302, frame->getInt32AsLSB(1U));
  CHECK_EQUAL(0xff0a0708, frame->getInt32AsLSB(4U));

  // Just to be extra certain
  uint8_t data[8U] = { 0xff, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff };
  FrameParser theFrame(data, 8U);
  CHECK_EQUAL(-1, theFrame.getInt32AsLSB(0U));
  CHECK_EQUAL(-2, theFrame.getInt32AsLSB(4U));
}

TEST(frameDataTests, testGetInt32ReturnsZeroIfOutsideBoundsLSB)
{
  CHECK_EQUAL(0, frame->getInt32AsLSB(frame->getSize()));
  CHECK_EQUAL(0, frame->getInt32AsLSB(frame->getSize() - 1U));
  CHECK_EQUAL(0, frame->getInt32AsLSB(frame->getSize() - 3U));
}

TEST(frameDataTests, testGetInt32ReturnsCorrectValueMSB)
{
  CHECK_EQUAL(0x01020309, frame->getInt32AsMSB(0U));
  CHECK_EQUAL(0x02030908, frame->getInt32AsMSB(1U));
  CHECK_EQUAL(0x08070aff, frame->getInt32AsMSB(4U));

  // Just to be extra certain
  uint8_t data[8U] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe };
  FrameParser theFrame(data, 8U);
  CHECK_EQUAL(-1, theFrame.getInt32AsMSB(0U));
  CHECK_EQUAL(-2, theFrame.getInt32AsMSB(4U));
}

TEST(frameDataTests, testGetInt32ReturnsZeroIfOutsideBoundsMSB)
{
  CHECK_EQUAL(0, frame->getInt32AsMSB(frame->getSize()));
  CHECK_EQUAL(0, frame->getInt32AsMSB(frame->getSize() - 1U));
  CHECK_EQUAL(0, frame->getInt32AsMSB(frame->getSize() - 3U));
}

TEST(frameDataTests, testGetBytesReturnsCorrectValues)
{
  uint8_t frameData[8U] = { 23U, 45U, 255U, 34U, 1U, 0U, 7U, 88U };
  FrameParser theFrame(frameData, 8U);

  uint8_t buffer[8U] = {};
  uint16_t returnedBytes = 0U;

  returnedBytes = theFrame.getBytes(0U, buffer, 3U);
  CHECK_EQUAL(3U, returnedBytes);
  MEMCMP_EQUAL(frameData, buffer, 3U);

  returnedBytes = theFrame.getBytes(2U, buffer, 6U);
  CHECK_EQUAL(6U, returnedBytes);
  MEMCMP_EQUAL(&frameData[2], buffer, 6U);

  returnedBytes = theFrame.getBytes(0U, buffer, 8U);
  CHECK_EQUAL(8U, returnedBytes);
  MEMCMP_EQUAL(frameData, buffer, 8U);
}

TEST(frameDataTests, testGetBytesReturnsZeroWhenOutsideBounds)
{
  uint8_t buffer[9U] = {};
  uint16_t returnedBytes = 0U;

  returnedBytes = frame->getBytes(0U, buffer, 9U);
  CHECK_EQUAL(0U, returnedBytes);

  returnedBytes = frame->getBytes(8U, buffer, 2U);
  CHECK_EQUAL(0U, returnedBytes);
}


TEST(frameDataTests, testAssignmentOperatorAndCopyConstructor)
{
  FrameParser newFrame;
  newFrame = FrameParser(*frame);

  CHECK_EQUAL(frame->getSize(), newFrame.getSize());
  CHECK_EQUAL(frame->getID(), newFrame.getID());
  CHECK_EQUAL(frame->getUInt32AsLSB(0U), newFrame.getUInt32AsLSB(0U));
  CHECK_EQUAL(frame->getUInt32AsLSB(4U), newFrame.getUInt32AsLSB(4U));
}

