#include "CppUTest/TestHarness.h"

#include "AzurePayload.h"

#include "SettingsMock.h"
#include "DeviceMetaMock.h"
#include "AzureCloudSyncControllerMock.h"
#include "FirmwareUpgradeControllerMock.h"
#include "SystemTimeDriverMock.h"

namespace
{

SettingsMock *settings = nullptr;
DeviceMetaMock *deviceMeta = nullptr;
AzureCloudSyncControllerMock *syncController = nullptr;
FirmwareUpgradeControllerMock *fwUpdateController = nullptr;
SystemTimeDriverMock *systemTime = nullptr;

AzurePayload *azurePayload;

void commonSetup()
{
  settings = new SettingsMock();
  deviceMeta = new DeviceMetaMock();
  syncController = new AzureCloudSyncControllerMock();
  fwUpdateController = new FirmwareUpgradeControllerMock();
  systemTime = new SystemTimeDriverMock();

  azurePayload = new AzurePayload(*settings,
                                  *deviceMeta,
                                  *syncController,
                                  *fwUpdateController,
                                  *systemTime);
}

void commonTeardown()
{
  delete azurePayload; azurePayload = nullptr;

  delete settings; settings = nullptr;
  delete deviceMeta; deviceMeta = nullptr;
  delete syncController; syncController = nullptr;
  delete fwUpdateController; fwUpdateController = nullptr;
  delete systemTime; systemTime = nullptr;
}

}

// mock decode - not supposed to be invoked during test
void mbedtls_base64_decode(unsigned char*, unsigned int, unsigned int*, unsigned char const*, unsigned int)
{

}

// mock encode - not supposed to be invoked during test
void mbedtls_base64_encode(unsigned char*, unsigned int, unsigned int*, unsigned char const*, unsigned int)
{

}

TEST_GROUP(AzurePayload)
{
  TEST_SETUP()
  {
    commonSetup();
  }

  TEST_TEARDOWN()
  {
    commonTeardown();

    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
  }
};

TEST(AzurePayload, processOperationInfo)
{
  uint8_t bytes[4] = {
      0x0u, 0x14u,
      0x0u, // forget me
      0x0u // success
  };
  uint32_t size = static_cast<uint32_t>(sizeof(bytes));
  FrameParser fp(bytes, size);
  OperationInfo oi(fp);

  mock().expectOneCall("getForgetMeState")
      .andReturnValue(static_cast<unsigned int>(ForgetMeState::resetRequested));
  mock().expectOneCall("setForgetMeState")
      .withUnsignedIntParameter("forgetMeState", static_cast<unsigned int>(ForgetMeState::resetConfirmed));
  bool wasProcessed = azurePayload->processOperationInfo(oi);
  CHECK_TRUE(wasProcessed);
}

TEST(AzurePayload, forgetMeNotRequestedFromAppliance)
{
  uint8_t bytes[4] = {
      0x0u, 0x14u,
      0x0u, // forget me
      0x0u // success
  };
  uint32_t size = static_cast<uint32_t>(sizeof(bytes));
  FrameParser fp(bytes, size);
  OperationInfo oi(fp);

  mock().expectOneCall("getForgetMeState")
      .andReturnValue(static_cast<unsigned int>(ForgetMeState::noResetRequested));
  mock().expectNoCall("setForgetMeState");
  bool wasProcessed = azurePayload->processOperationInfo(oi);
  CHECK_TRUE(wasProcessed);
}

TEST(AzurePayload, forgetMeAlreadyConfirmed)
{
  uint8_t bytes[4] = {
      0x0u, 0x14u,
      0x0u, // forget me
      0x0u // success
  };
  uint32_t size = static_cast<uint32_t>(sizeof(bytes));
  FrameParser fp(bytes, size);
  OperationInfo oi(fp);

  mock().expectOneCall("getForgetMeState")
      .andReturnValue(static_cast<unsigned int>(ForgetMeState::resetConfirmed));
  mock().expectNoCall("setForgetMeState");
  bool wasProcessed = azurePayload->processOperationInfo(oi);
  CHECK_TRUE(wasProcessed);
}

TEST(AzurePayload, forgetMeFailed)
{
  uint8_t bytes[4] = {
      0x0u, 0x14u,
      0x0u, // forget me
      0x1u // failure
  };
  uint32_t size = static_cast<uint32_t>(sizeof(bytes));
  FrameParser fp(bytes, size);
  OperationInfo oi(fp);

  mock().expectNoCall("setForgetMeState");
  bool wasProcessed = azurePayload->processOperationInfo(oi);
  CHECK_TRUE(wasProcessed);
}

TEST(AzurePayload, forgetMeUnknownOperation)
{
  uint8_t bytes[4] = {
      0x0u, 0x14u,
      0x1u, // forget me
      0x0u // failure
  };
  uint32_t size = static_cast<uint32_t>(sizeof(bytes));
  FrameParser fp(bytes, size);
  OperationInfo oi(fp);

  mock().expectNoCall("setForgetMeState");
  bool wasProcessed = azurePayload->processOperationInfo(oi);
  CHECK_FALSE(wasProcessed);
}
