#include "CppUTest\TestHarness.h"
#include "CppUTestExt\MockSupport.h"

#include "Viewmodel.h"
#include "Constants.h"
//#include "TemperatureValuesMock.h"
#include "SettingsMock.h"
#include "DeviceMetaMock.h"
//#include "Address.h"
#include <string.h>

static SettingsMock settings;
static DeviceMetaMock deviceMeta;
//static DateTime dateTime = { 0, 0, 0, 0, 0, 0, WeekDay::MONDAY };
ViewModel *viewModel = nullptr;

TEST_GROUP(ViewModelTest)
{
  TEST_SETUP()
  {
     
  }

  TEST_TEARDOWN()
  {
    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
    delete viewModel;
  }
};

TEST(ViewModelTest, testStringSaved)
{
  const char string[] = "TS";
  viewModel = new ViewModel(string);

  STRNCMP_EQUAL(string, viewModel->getTextString(), 10);
}

TEST(ViewModelTest, testToolongStringTruncated)
{
  const char string[] = "omglolwtfbbq";
  viewModel = new ViewModel(string);

  STRNCMP_EQUAL("om", viewModel->getTextString(), 10);
}

TEST(ViewModelTest, testShortString)
{
  const char string[] = "A";
  viewModel = new ViewModel(string);

  STRNCMP_EQUAL(string, viewModel->getTextString(), 10);
}

TEST(ViewModelTest, testStringWithSpaces)
{
  const char string[] = " 3";
  viewModel = new ViewModel(string);

  STRNCMP_EQUAL(string, viewModel->getTextString(), 10);
}

TEST(ViewModelTest, testConstructorGeneratesMainScreen)
{
  mock().expectOneCall("getSetPoint").andReturnValue(22000);
  mock().expectOneCall("getHeatingMode").andReturnValue(HeatingMode::comfort);
  mock().expectOneCall("isOverrideActive").andReturnValue(false);
  UIState uiState = { Screen::mainScreen};
  viewModel = new ViewModel(uiState,  settings, deviceMeta, false, false);
  char * expectedText = "22";

  STRCMP_EQUAL(expectedText , viewModel->getTextString());
}

TEST(ViewModelTest, testMainScreenHeatIconAlwaysOn)
{
  mock().expectNCalls(2, "getSetPoint").andReturnValue(25000);
  mock().ignoreOtherCalls();
  UIState uiState = { Screen::mainScreen };
  uiState.currentScreenEnterTimestampMs = 0u;
  uiState.timeSinceBootMs = 0u;

  viewModel = new ViewModel(uiState, settings, deviceMeta, true, false);
  CHECK_TRUE(viewModel->isYellowLEDActive());
  delete viewModel;
  uiState.timeSinceBootMs += 1000u;

  viewModel = new ViewModel(uiState, settings, deviceMeta, true, false);
  CHECK_TRUE(viewModel->isYellowLEDActive());
}

TEST(ViewModelTest, testMainScreenHeatIconLitFullyDimmed)
{
  mock().expectNCalls(1, "getSetPoint").andReturnValue(25000);
  mock().expectOneCall("getHeatingMode").andReturnValue(HeatingMode::comfort);
  mock().expectOneCall("isOverrideActive").andReturnValue(false);
  UIState uiState = { Screen::mainScreen };
  uiState.currentScreenEnterTimestampMs = 0u;
  uiState.timeSinceBootMs = 30000u;
  uiState.timestampLastActiveMs = 10000u;

  // icon on
  viewModel = new ViewModel(uiState, settings, deviceMeta, true, true);
  CHECK_TRUE(viewModel->isYellowLEDActive());
}

TEST(ViewModelTest, testMainEcoMode)
{
  mock().expectOneCall("getSetPoint").andReturnValue(22000);
  mock().expectOneCall("getHeatingMode").andReturnValue(HeatingMode::eco);
  mock().expectOneCall("isOverrideActive").andReturnValue(false);
  UIState uiState = { Screen::mainScreen };

  viewModel = new ViewModel(uiState, settings, deviceMeta, false, false);
  CHECK_TRUE(viewModel->isRGBGreenActive());

}

TEST(ViewModelTest, testMainCorrectTextWhenHeaterOff)
{
  mock().expectOneCall("getSetPoint").andReturnValue(0);
  mock().expectOneCall("getHeatingMode").andReturnValue(HeatingMode::eco);
  mock().expectOneCall("isOverrideActive").andReturnValue(false);
  UIState uiState = { Screen::mainScreen };
  uiState.currentScreenEnterTimestampMs = 0u;
  uiState.timeSinceBootMs = 0u;

  viewModel = new ViewModel(uiState, settings, deviceMeta, false, false);
  STRNCMP_EQUAL("--", viewModel->getTextString(), 10);
}

TEST(ViewModelTest, testMainScreenFullyDimmedShowsOnlyHeatingMode)
{
  mock().expectOneCall("getSetPoint").andReturnValue(22000);
  mock().expectOneCall("getHeatingMode").andReturnValue(HeatingMode::eco);
  mock().expectOneCall("isOverrideActive").andReturnValue(false);
  UIState uiState = { Screen::mainScreen };

  viewModel = new ViewModel(uiState, settings, deviceMeta, false, true);
  STRCMP_EQUAL("  ", viewModel->getTextString());
  CHECK_TRUE(viewModel->isRGBGreenActive());
}

TEST(ViewModelTest, testMainScreenOverrideModeIconBlinking)
{
  mock().expectNCalls(21, "getSetPoint").andReturnValue(25000);
  mock().expectNCalls(21, "getHeatingMode").andReturnValue(HeatingMode::eco);
  mock().expectNCalls(21, "isOverrideActive").andReturnValue(true);
  UIState uiState = { Screen::mainScreen };
  uiState.currentScreenEnterTimestampMs = 0u;
  uiState.timeSinceBootMs = 0u;

  for (int i = 0; i < 10; i++)
  {
    // icon on
    viewModel = new ViewModel(uiState, settings, deviceMeta, true, false);
    CHECK_TRUE(viewModel->isRGBGreenActive());
    delete viewModel;
    uiState.timeSinceBootMs += 1000u;

    // icon off
    viewModel = new ViewModel(uiState, settings, deviceMeta, true, false);
    CHECK_FALSE(viewModel->isRGBGreenActive());
    delete viewModel;
    uiState.timeSinceBootMs += 1000u;
  }
  viewModel = new ViewModel(uiState, settings, deviceMeta, false, false); // because the teardown deletes an instance of viewmodel

}

TEST(ViewModelTest, testMainScreenOverrideModeIconBlinkingFullyDimmed)
{
  mock().expectNCalls(21, "getSetPoint").andReturnValue(25000);
  mock().expectNCalls(21, "getHeatingMode").andReturnValue(HeatingMode::antiFrost);
  mock().expectNCalls(21, "isOverrideActive").andReturnValue(true);
  UIState uiState = { Screen::mainScreen };
  uiState.currentScreenEnterTimestampMs = 0u;
  uiState.timeSinceBootMs = 30000u;
  uiState.timestampLastActiveMs = 10000u;

  for (int i = 0; i < 10; i++)
  {
    // icon on
    viewModel = new ViewModel(uiState, settings, deviceMeta, true, true);
    CHECK_TRUE(viewModel->isRGBBlueActive());
    delete viewModel;
    uiState.timeSinceBootMs += 1000u;

    // icon off
    viewModel = new ViewModel(uiState, settings, deviceMeta, true, true);
    CHECK_FALSE(viewModel->isRGBBlueActive());
    delete viewModel;
    uiState.timeSinceBootMs += 1000u;
  }
  viewModel = new ViewModel(uiState, settings, deviceMeta, false, false); // because the teardown deletes an instance of viewmodel
}

TEST(ViewModelTest, testButtonTestScreenShowsCorrectTextAndNoLEDs)
{
  UIState uiState = { Screen::buttonTestScreen };

  viewModel = new ViewModel(uiState, settings, deviceMeta, false, true);
  STRNCMP_EQUAL("Su", viewModel->getTextString(), 10);
  CHECK_FALSE(viewModel->isRGBRedActive());
  CHECK_FALSE(viewModel->isRGBGreenActive());
  CHECK_FALSE(viewModel->isRGBBlueActive());
  CHECK_FALSE(viewModel->isYellowLEDActive());
}

TEST(ViewModelTest, testSoftwareVersionScreenShowsConnectionStatusOffline)
{
  UIState uiState = { Screen::swVersionScreen };
  uiState.isWifiConnected = false;
  uiState.isAzureConnected = false;

  for (int i = 0; i < 5; i++)
  {
    // icon on (red)
    viewModel = new ViewModel(uiState, settings, deviceMeta, false, false);
    CHECK_TRUE(viewModel->isRGBRedActive());
    CHECK_FALSE(viewModel->isRGBGreenActive());
    CHECK_FALSE(viewModel->isRGBBlueActive());
    delete viewModel;
    uiState.timeSinceBootMs += 1000u;

    // icon off
    viewModel = new ViewModel(uiState, settings, deviceMeta, true, true);
    CHECK_FALSE(viewModel->isRGBRedActive());
    CHECK_FALSE(viewModel->isRGBGreenActive());
    CHECK_FALSE(viewModel->isRGBBlueActive());
    delete viewModel;
    uiState.timeSinceBootMs += 1000u;
  }

  viewModel = new ViewModel(uiState, settings, deviceMeta, false, false); // because the teardown deletes an instance of viewmodel
}

TEST(ViewModelTest, testSoftwareVersionScreenShowsConnectionStatusWifiConnectedButNotAzure)
{
  UIState uiState = { Screen::swVersionScreen };
  uiState.isWifiConnected = true;
  uiState.isAzureConnected = false;

  for (int i = 0; i < 5; i++)
  {
    // icon on (yellow)
    viewModel = new ViewModel(uiState, settings, deviceMeta, false, false);
    CHECK_TRUE(viewModel->isRGBRedActive());
    CHECK_TRUE(viewModel->isRGBGreenActive());
    CHECK_FALSE(viewModel->isRGBBlueActive());
    delete viewModel;
    uiState.timeSinceBootMs += 1000u;

    // icon off
    viewModel = new ViewModel(uiState, settings, deviceMeta, true, true);
    CHECK_FALSE(viewModel->isRGBRedActive());
    CHECK_FALSE(viewModel->isRGBGreenActive());
    CHECK_FALSE(viewModel->isRGBBlueActive());
    delete viewModel;
    uiState.timeSinceBootMs += 1000u;
  }

  viewModel = new ViewModel(uiState, settings, deviceMeta, false, false); // because the teardown deletes an instance of viewmodel
}

TEST(ViewModelTest, testSoftwareVersionScreenShowsConnectionStatusAzureOnline)
{
  UIState uiState = { Screen::swVersionScreen };
  uiState.isAzureConnected = true;

  for (int i = 0; i < 5; i++)
  {
    // icon on (green)
    viewModel = new ViewModel(uiState, settings, deviceMeta, false, false);
    CHECK_FALSE(viewModel->isRGBRedActive());
    CHECK_TRUE(viewModel->isRGBGreenActive());
    CHECK_FALSE(viewModel->isRGBBlueActive());
    delete viewModel;
    uiState.timeSinceBootMs += 1000u;

    // icon off
    viewModel = new ViewModel(uiState, settings, deviceMeta, true, true);
    CHECK_FALSE(viewModel->isRGBRedActive());
    CHECK_FALSE(viewModel->isRGBGreenActive());
    CHECK_FALSE(viewModel->isRGBBlueActive());
    delete viewModel;
    uiState.timeSinceBootMs += 1000u;
  }

  viewModel = new ViewModel(uiState, settings, deviceMeta, false, false); // because the teardown deletes an instance of viewmodel
}

TEST(ViewModelTest, testSoftwareVersionScreenShowsSoftwareVersionInThreeScreens)
{
  // This test relies on Constants::SW_VERSION and Constants::SW_TEST_NO as
  // ViewModel uses them directly (we can't inject any other value).
  // The test is therefore a bit weak since the test code may make the same
  // invalid assumptions as the code under test and fail to discover some problems.

  UIState uiState = { Screen::swVersionScreen };
  uiState.isAzureConnected = true;

  char expectedMajor[Constants::TEXT_MAX_LENGTH] {};
  char expectedMinor[Constants::TEXT_MAX_LENGTH] {};
  char expectedTest[Constants::TEXT_MAX_LENGTH] {};

  snprintf(expectedMajor,
           static_cast<uint8_t>(Constants::TEXT_MAX_LENGTH),
           "%02u",
           Constants::SW_VERSION / 10u);

  snprintf(expectedMinor,
           static_cast<uint8_t>(Constants::TEXT_MAX_LENGTH),
           "%02u",
           Constants::SW_VERSION % 10u);

  snprintf(expectedTest,
           static_cast<uint8_t>(Constants::TEXT_MAX_LENGTH),
           "%02u",
           Constants::SW_TEST_NO);

  char *dummyPtr = nullptr;

  // major
  viewModel = new ViewModel(uiState, settings, deviceMeta, false, false);
  STRNCMP_EQUAL(expectedMajor, viewModel->getTextString(), sizeof(expectedMajor));
  uint32_t parsedMajor = std::strtoul(viewModel->getTextString(), &dummyPtr, 10u);
  delete viewModel;
  uiState.timeSinceBootMs += 1667u;

  // minor
  viewModel = new ViewModel(uiState, settings, deviceMeta, true, true);
  STRNCMP_EQUAL(expectedMinor, viewModel->getTextString(), sizeof(expectedMinor));
  uint32_t parsedMinor = std::strtoul(viewModel->getTextString(), &dummyPtr, 10u);
  delete viewModel;
  uiState.timeSinceBootMs += 1667u;

  // test no
  viewModel = new ViewModel(uiState, settings, deviceMeta, true, true);
  STRNCMP_EQUAL(expectedTest, viewModel->getTextString(), sizeof(expectedTest));
  uint32_t parsedTest = std::strtoul(viewModel->getTextString(), &dummyPtr, 10u);
  delete viewModel;
  uiState.timeSinceBootMs += 1667u;

  // Test wrap-around, although the ui controller should exit this screen
  // before this happens
  viewModel = new ViewModel(uiState, settings, deviceMeta, true, true);
  STRNCMP_EQUAL(expectedMajor, viewModel->getTextString(), sizeof(expectedMajor));
  delete viewModel;

  uint8_t reconstructedSwVersion = static_cast<uint8_t>(parsedMajor)*10u + static_cast<uint8_t>(parsedMinor);
  CHECK_EQUAL_TEXT(Constants::SW_VERSION, reconstructedSwVersion, "Parsed and reconstructed sw version is incorrect");
  CHECK_EQUAL_TEXT(Constants::SW_TEST_NO, parsedTest, "Parsed sw test number is incorrect");

  viewModel = new ViewModel(uiState, settings, deviceMeta, false, false); // because the teardown deletes an instance of viewmodel
}

TEST(ViewModelTest, testFactoryResetConfirmScreen)
{
  mock().ignoreOtherCalls();
  UIState uiState = { Screen::confirmFactoryResetScreen};

  viewModel = new ViewModel(uiState, settings, deviceMeta, false, false);
  char * expectedText = "rE";

  STRNCMP_EQUAL(expectedText, viewModel->getTextString(), 10);
}

TEST(ViewModelTest, testBluetoothSetupScreenShowsCorrectTextAndBlueLED)
{
  mock().ignoreOtherCalls();
  UIState uiState = { Screen::bluetoothSetupScreen };

  viewModel = new ViewModel(uiState, settings, deviceMeta, false, false);
  char * expectedText = "BT";

  STRNCMP_EQUAL(expectedText, viewModel->getTextString(), 10);
  CHECK_FALSE(viewModel->isRGBRedActive());
  CHECK_FALSE(viewModel->isRGBGreenActive());
  CHECK_TRUE(viewModel->isRGBBlueActive());
  CHECK_FALSE(viewModel->isYellowLEDActive());
}


TEST(ViewModelTest, testBluetoothSetupScreenShowsGdidAndBluetoothPasskey)
{
  mock().ignoreOtherCalls();
  UIState uiState = { Screen::bluetoothSetupScreen };

  const char expectedText1[] = "BT";

  const char expectedText2[] = "1d";
  const char expectedText3[] = "01";
  const char expectedText4[] = "23";
  const char expectedText5[] = "45";
  const char expectedText6[] = "67";
  const char expectedText7[] = "89";
  const char expectedText8[] = "ab";

  const char expectedText9[] = "Pn";
  const char expectedText10[] = "10";
  const char expectedText11[] = "34";
  const char expectedText12[] = "56";

  const char mockGdid[13] = "0123456789ab";
  const uint32_t mockPasskey = 103456u;

  mock().expectNCalls(13, "DeviceMetaMock::getGDID")
      .withOutputParameterReturning("gdid", mockGdid, sizeof(mockGdid))
      .withUnsignedIntParameter("size", 13u);
  mock().expectNCalls(13, "DeviceMetaMock::getBluetoothPasskey")
      .andReturnValue(mockPasskey);

  viewModel = new ViewModel(uiState, settings, deviceMeta, false, false);
  STRNCMP_EQUAL(expectedText1, viewModel->getTextString(), 10);
  delete viewModel;
  uiState.timeSinceBootMs += Constants::BLUETOOTH_SCROLL_PAGE_DURATION_MS;

  viewModel = new ViewModel(uiState, settings, deviceMeta, false, false);
  STRNCMP_EQUAL(expectedText2, viewModel->getTextString(), 10);
  delete viewModel;
  uiState.timeSinceBootMs += Constants::BLUETOOTH_SCROLL_PAGE_DURATION_MS;

  viewModel = new ViewModel(uiState, settings, deviceMeta, false, false);
  STRNCMP_EQUAL(expectedText3, viewModel->getTextString(), 10);
  delete viewModel;
  uiState.timeSinceBootMs += Constants::BLUETOOTH_SCROLL_PAGE_DURATION_MS;

  viewModel = new ViewModel(uiState, settings, deviceMeta, false, false);
  STRNCMP_EQUAL(expectedText4, viewModel->getTextString(), 10);
  delete viewModel;
  uiState.timeSinceBootMs += Constants::BLUETOOTH_SCROLL_PAGE_DURATION_MS;

  viewModel = new ViewModel(uiState, settings, deviceMeta, false, false);
  STRNCMP_EQUAL(expectedText5, viewModel->getTextString(), 10);
  delete viewModel;
  uiState.timeSinceBootMs += Constants::BLUETOOTH_SCROLL_PAGE_DURATION_MS;

  viewModel = new ViewModel(uiState, settings, deviceMeta, false, false);
  STRNCMP_EQUAL(expectedText6, viewModel->getTextString(), 10);
  delete viewModel;
  uiState.timeSinceBootMs += Constants::BLUETOOTH_SCROLL_PAGE_DURATION_MS;

  viewModel = new ViewModel(uiState, settings, deviceMeta, false, false);
  STRNCMP_EQUAL(expectedText7, viewModel->getTextString(), 10);
  delete viewModel;
  uiState.timeSinceBootMs += Constants::BLUETOOTH_SCROLL_PAGE_DURATION_MS;

  viewModel = new ViewModel(uiState, settings, deviceMeta, false, false);
  STRNCMP_EQUAL(expectedText8, viewModel->getTextString(), 10);
  delete viewModel;
  uiState.timeSinceBootMs += Constants::BLUETOOTH_SCROLL_PAGE_DURATION_MS;

  viewModel = new ViewModel(uiState, settings, deviceMeta, false, false);
  STRNCMP_EQUAL(expectedText9, viewModel->getTextString(), 10);
  delete viewModel;
  uiState.timeSinceBootMs += Constants::BLUETOOTH_SCROLL_PAGE_DURATION_MS;

  viewModel = new ViewModel(uiState, settings, deviceMeta, false, false);
  STRNCMP_EQUAL(expectedText10, viewModel->getTextString(), 10);
  delete viewModel;
  uiState.timeSinceBootMs += Constants::BLUETOOTH_SCROLL_PAGE_DURATION_MS;

  viewModel = new ViewModel(uiState, settings, deviceMeta, false, false);
  STRNCMP_EQUAL(expectedText11, viewModel->getTextString(), 10);
  delete viewModel;
  uiState.timeSinceBootMs += Constants::BLUETOOTH_SCROLL_PAGE_DURATION_MS;

  viewModel = new ViewModel(uiState, settings, deviceMeta, false, false);
  STRNCMP_EQUAL(expectedText12, viewModel->getTextString(), 10);
  delete viewModel;
  uiState.timeSinceBootMs += Constants::BLUETOOTH_SCROLL_PAGE_DURATION_MS;

  viewModel = new ViewModel(uiState, settings, deviceMeta, false, false);
  STRNCMP_EQUAL(expectedText1, viewModel->getTextString(), 10);
}

TEST(ViewModelTest, testBluetoothSetupScreenBlueLEDBlinksIfBluetoothIsActive)
{
  mock().ignoreOtherCalls();
  UIState uiState = { Screen::bluetoothSetupScreen };
  uiState.isBluetoothActive = true;
  uiState.currentScreenEnterTimestampMs = 0u;
  uiState.timeSinceBootMs = 0u;

  for (int i = 0; i < 10; i++)
  {
    // icon on
    viewModel = new ViewModel(uiState, settings, deviceMeta, true, false);
    CHECK_TRUE(viewModel->isRGBBlueActive());
    delete viewModel;
    uiState.timeSinceBootMs += Constants::BLUETOOTH_LED_PERIOD_NO_CONNECTION_MS;

    // icon off
    viewModel = new ViewModel(uiState, settings, deviceMeta, true, false);
    CHECK_FALSE(viewModel->isRGBBlueActive());
    delete viewModel;
    uiState.timeSinceBootMs += Constants::BLUETOOTH_LED_PERIOD_NO_CONNECTION_MS;
  }

  viewModel = new ViewModel(uiState, settings, deviceMeta, false, false); // because the teardown deletes an instance of viewmodel
}

TEST(ViewModelTest, testBluetoothSetupScreenBlueLEDBlinksFastIfBluetoothIsConnected)
{
  mock().ignoreOtherCalls();
  UIState uiState = { Screen::bluetoothSetupScreen };
  uiState.isBluetoothActive = true;
  uiState.isBluetoothConnected = true;
  uiState.currentScreenEnterTimestampMs = 0u;
  uiState.timeSinceBootMs = 0u;

  for (int i = 0; i < 10; i++)
  {
    // icon on
    viewModel = new ViewModel(uiState, settings, deviceMeta, true, false);
    CHECK_TRUE(viewModel->isRGBBlueActive());
    delete viewModel;
    uiState.timeSinceBootMs += Constants::BLUETOOTH_LED_PERIOD_CONNECTED_MS;

    // icon off
    viewModel = new ViewModel(uiState, settings, deviceMeta, true, false);
    CHECK_FALSE(viewModel->isRGBBlueActive());
    delete viewModel;
    uiState.timeSinceBootMs += Constants::BLUETOOTH_LED_PERIOD_CONNECTED_MS;
  }

  viewModel = new ViewModel(uiState, settings, deviceMeta, false, false); // because the teardown deletes an instance of viewmodel
}

TEST(ViewModelTest, testFirmwareUpgradeScreenHasLEDsOff)
{
  mock().ignoreOtherCalls();
  UIState uiState = { Screen::firmwareUpgradeScreen };

  viewModel = new ViewModel(uiState, settings, deviceMeta, false, false);

  CHECK_FALSE(viewModel->isRGBRedActive());
  CHECK_FALSE(viewModel->isRGBGreenActive());
  CHECK_FALSE(viewModel->isRGBBlueActive());
  CHECK_FALSE(viewModel->isYellowLEDActive());
}

TEST(ViewModelTest, testFirmwareUpgradeScreenTogglesSecondDigitEverySecond)
{
  mock().ignoreOtherCalls();
  UIState uiState = { Screen::firmwareUpgradeScreen };

  char * expectedText1 = "F ";
  char * expectedText2 = "F_";
  char * expectedText3 = "F-";
  char * expectedText4 = "F^";

  viewModel = new ViewModel(uiState, settings, deviceMeta, false, false);
  STRNCMP_EQUAL(expectedText1, viewModel->getTextString(), 3U);
  delete viewModel;

  uiState.timeSinceBootMs += 1000U;
  viewModel = new ViewModel(uiState, settings, deviceMeta, false, false);
  STRNCMP_EQUAL(expectedText2, viewModel->getTextString(), 3U);
  delete viewModel;

  uiState.timeSinceBootMs += 1000U;
  viewModel = new ViewModel(uiState, settings, deviceMeta, false, false);
  STRNCMP_EQUAL(expectedText3, viewModel->getTextString(), 3U);
  delete viewModel;

  uiState.timeSinceBootMs += 1000U;
  viewModel = new ViewModel(uiState, settings, deviceMeta, false, false);
  STRNCMP_EQUAL(expectedText4, viewModel->getTextString(), 3U);
  delete viewModel;

  uiState.timeSinceBootMs += 1000U;
  viewModel = new ViewModel(uiState, settings, deviceMeta, false, false);
  STRNCMP_EQUAL(expectedText1, viewModel->getTextString(), 3U);
}

