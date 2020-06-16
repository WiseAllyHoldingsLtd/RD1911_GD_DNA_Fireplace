#include "CppUTest/TestHarness.h"

#include "OperationInfo.h"

TEST_GROUP(OperationInfo)
{
  TEST_SETUP()
  {

  }

  TEST_TEARDOWN()
  {

  }
};

TEST(OperationInfo, valid)
{
  uint8_t bytes[4] = {
      0x0u, 0x14u,
      0x0u, // forget me
      0x0u // success
  };
  uint32_t size = static_cast<uint32_t>(sizeof(bytes));
  FrameParser fp(bytes, size);
  OperationInfo oi(fp);
  bool isValid = oi.isValid();
  CHECK_TRUE(isValid);
}

TEST(OperationInfo, validAlsoOnForgetMeError)
{
  uint8_t bytes[4] = {
      0x0u, 0x14u,
      0x0u, // forget me
      0x1u // forget me error
  };
  uint32_t size = static_cast<uint32_t>(sizeof(bytes));
  FrameParser fp(bytes, size);
  OperationInfo oi(fp);
  bool isValid = oi.isValid();
  CHECK_TRUE(isValid);
}

TEST(OperationInfo, invalidId)
{
  uint8_t bytes[4] = {
      0x0u, 0x13u,
      0x0u, // forget me
      0x0u // success
  };
  uint32_t size = static_cast<uint32_t>(sizeof(bytes));
  FrameParser fp(bytes, size);
  OperationInfo oi(fp);
  bool isValid = oi.isValid();
  CHECK_FALSE(isValid);
}

TEST(OperationInfo, invalidOperation)
{
  uint8_t bytes[4] = {
      0x0u, 0x14u,
      0x1u, // unknown operation
      0x0u // success
  };
  uint32_t size = static_cast<uint32_t>(sizeof(bytes));
  FrameParser fp(bytes, size);
  OperationInfo oi(fp);
  bool isValid = oi.isValid();
  CHECK_FALSE(isValid);
}

TEST(OperationInfo, invalidStatus)
{
  uint8_t bytes[4] = {
      0x0u, 0x14u,
      0x0u, // forget me
      0x2u // unknown status
  };
  uint32_t size = static_cast<uint32_t>(sizeof(bytes));
  FrameParser fp(bytes, size);
  OperationInfo oi(fp);
  bool isValid = oi.isValid();
  CHECK_FALSE(isValid);
}
