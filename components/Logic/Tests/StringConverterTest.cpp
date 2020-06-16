#include "CppUTest\TestHarness.h"

#include "StringConverter.h"



TEST_GROUP(StringConverterTest)
{
  TEST_SETUP()
  {
  }

  TEST_TEARDOWN()
  {
  }
};


TEST(StringConverterTest, testConvertNumber)
{
  char string[3] = "01";
  uint8_t buffer[4] = {0, 0, 0, 0};

  int result = StringConverter::convert(string, buffer, 3);

  CHECK_EQUAL(2, result);
  CHECK_EQUAL(0x3Fu, buffer[0]);
  CHECK_EQUAL(0x06u, buffer[1]);
  CHECK_EQUAL(0u, buffer[2]);
}


TEST(StringConverterTest, testEmptyString)
{
  char string[2] = "";
  uint8_t buffer[4] = {0, 0, 0, 0};

  int result = StringConverter::convert(string, buffer, 4);

  CHECK_EQUAL(0, result);
  CHECK_EQUAL(0u, buffer[0]);
}


TEST(StringConverterTest, testConvertStringLongerThanMaxOutputLength)
{
  char string[8] = "0123456";
  uint8_t buffer[2] = {0, 0};

  int result = StringConverter::convert(string, buffer, 2);

  CHECK_EQUAL(2, result);
  CHECK_EQUAL(0x3Fu, buffer[0]);
  CHECK_EQUAL(0x06u, buffer[1]);
}


TEST(StringConverterTest, testConvertSpace)
{
  char string[2] = " ";
  uint8_t buffer[1] = {0};

  int result = StringConverter::convert(string, buffer, 1);

  CHECK_EQUAL(1, result);
  CHECK_EQUAL(0u, buffer[0]);
}

TEST(StringConverterTest, testConvertUnknownSymbols)
{
  char string[4] = "@.Q";
  uint8_t buffer[3] = {0, 0, 0};

  int result = StringConverter::convert(string, buffer, 3);

  CHECK_EQUAL(3, result);
  CHECK_EQUAL(0u, buffer[0]);
  CHECK_EQUAL(0u, buffer[1]);
  CHECK_EQUAL(0u, buffer[2]);
}

TEST(StringConverterTest, testConvertZeroMaxOutputLength)
{
  char string[3] = "01";
  uint8_t buffer[4] = {0, 0, 0, 0};

  int result = StringConverter::convert(string, buffer, 0);

  CHECK_EQUAL(0, result);
  CHECK_EQUAL(0u, buffer[0]);
}




