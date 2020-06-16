#include "CppUTest\TestHarness.h"
#include "CppUTestExt\MockSupport.h"

#include "SettingsMock.h"
#include "DeviceMetaMock.h"
#include "AzureCloudSyncControllerMock.h"
#include "ParameterDataRequestMock.h"
#include "SystemTimeDriverMock.h"
#include "ParameterDataPayload.h"
#include "ParameterID.h"
#include "RtcMock.h"
#include "WeekProgramExtDataCopier.h"

static SettingsMock settings;
static DeviceMetaMock meta;
static AzureCloudSyncControllerMock cloudSync;
static ParameterDataRequestMock request;
static SystemTimeDriverMock sysTime;
static ParameterDataPayload * payload;
static WeekProgramExtDataCopier weekProgramExtDataCopier;


TEST_GROUP(ParameterDataPayloadProcessInvalidRequestOrNoParams)
{
  TEST_SETUP()
  {
    payload = new ParameterDataPayload(settings, meta, cloudSync, sysTime);
  }

  TEST_TEARDOWN()
  {
    delete payload;

    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
  }
};

TEST(ParameterDataPayloadProcessInvalidRequestOrNoParams, testProcessReturnsNoDataIfRequestIsInvalid)
{
  ParameterDataResponse response;

  mock().expectOneCall("ParameterDataRequestMock::isValid").andReturnValue(false);
  CHECK_EQUAL(0U, payload->processRequest(request, response));
}

TEST(ParameterDataPayloadProcessInvalidRequestOrNoParams, testProcessReturnsNoDataIfRequestIsEmpty)
{
  ParameterDataResponse response;

  mock().expectOneCall("ParameterDataRequestMock::isValid").andReturnValue(true);
  mock().expectOneCall("ParameterDataRequestMock::getNumOfParamDataItems").andReturnValue(0U);
  CHECK_EQUAL(0U, payload->processRequest(request, response));
}


TEST_GROUP(ParameterDataPayloadProcessSingleRequest)
{
  TEST_SETUP()
  {
    mock().installComparator("WeekProgramData", weekProgramDataComparator);
    mock().installCopier("WeekProgramData", weekProgramDataCopier);
    mock().installCopier("WeekProgramExtData", weekProgramExtDataCopier);
    mock().installComparator("DateTime", dateTimeComparator);
    mock().installCopier("DateTime", dateTimeCopier);
    mock().installCopier("Override", overrideCopier);
    mock().installComparator("Override", overrideComparator);

    payload = new ParameterDataPayload(settings, meta, cloudSync, sysTime);
    mock().expectOneCall("ParameterDataRequestMock::isValid").andReturnValue(true);
    mock().expectOneCall("ParameterDataRequestMock::getNumOfParamDataItems").andReturnValue(1U);
  }

  TEST_TEARDOWN()
  {
    delete payload;

    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
  }
};


TEST(ParameterDataPayloadProcessSingleRequest, processingOfDeviceData)
{
  ParameterDataResponse response;

  mock().expectOneCall("ParameterDataRequestMock::getParamDataID")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .andReturnValue(static_cast<uint16_t>(ParameterID::DEVICE_DETAILS));

  // On receive we expect 0 bytes (meaning read request)
  mock().expectOneCall("ParameterDataRequestMock::getParamDataSize")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .andReturnValue(static_cast<uint8_t>(0U));

  // Reading out GDID for the response
  uint8_t gdid[Constants::GDID_BCD_SIZE] = { 0x87, 0x89, 0x67, 0x45, 0x23, 0x01 };
  mock().expectOneCall("DeviceMetaMock::getGDIDAsBCD").withOutputParameterReturning("gdid", static_cast<void*>(gdid), Constants::GDID_BCD_SIZE);

  uint8_t returnedItems = payload->processRequest(request, response);
  CHECK_EQUAL(1U, returnedItems);
  CHECK_EQUAL(15U, response.getSize());

  // Verify bytes received in output
  uint8_t outputBytes[15U] = {};
  response.getBytes(outputBytes, sizeof(outputBytes));

  uint8_t expectedBytes[] = {
      0x21, 0x00,  // ApplianceFrameResonse FrameID
      0x1a, 0xfc, // DeviceDetails ParameterID
      0x08,       // Size
      Constants::COMPANY_CODE, Constants::COMPANY_SUBSET_CODE,
      0x87, 0x89, 0x67, 0x45, 0x23, 0x01,  // GDID as BCD
      0x00, 0x00  // EndOfFrame
  };

  MEMCMP_EQUAL(expectedBytes, outputBytes, sizeof(expectedBytes));
}

TEST(ParameterDataPayloadProcessSingleRequest, processOfSetComfortTemperature)
{
  ParameterDataResponse response;

  mock().expectOneCall("ParameterDataRequestMock::getParamDataID")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .andReturnValue(static_cast<uint16_t>(ParameterID::COMFORT_TEMPERATURE));

  // On receive we expect 1 bytes with requested temp.
  mock().expectOneCall("ParameterDataRequestMock::getParamDataSize")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .andReturnValue(static_cast<uint8_t>(1U));

  uint8_t paramDataBytes[1U] = { 23U }; // 23 degrees
  mock().expectOneCall("ParameterDataRequestMock::getParamData")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .withOutputParameterReturning("paramDataBytes", paramDataBytes, sizeof(paramDataBytes))
      .withUnsignedIntParameter("size", sizeof(paramDataBytes))
      .andReturnValue(static_cast<uint8_t>(sizeof(paramDataBytes)));

  // Set actual temperature
  mock().expectOneCall("AzureCloudSyncControllerMock::updateComfortTemperatureFromCloud").withIntParameter("temperatureFP", 23000);

  // No response required
  uint8_t returnedItems = payload->processRequest(request, response);
  CHECK_EQUAL(0U, returnedItems);
}

TEST(ParameterDataPayloadProcessSingleRequest, processOfSetEcoTemperature)
{
  ParameterDataResponse response;

  mock().expectOneCall("ParameterDataRequestMock::getParamDataID")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .andReturnValue(static_cast<uint16_t>(ParameterID::ECO_TEMPERATURE));

  // On receive we expect 1 bytes with requested temp.
  mock().expectOneCall("ParameterDataRequestMock::getParamDataSize")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .andReturnValue(static_cast<uint8_t>(1U));

  uint8_t paramDataBytes[1U] = { 17U }; // 17 degrees
  mock().expectOneCall("ParameterDataRequestMock::getParamData")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .withOutputParameterReturning("paramDataBytes", paramDataBytes, sizeof(paramDataBytes))
      .withUnsignedIntParameter("size", sizeof(paramDataBytes))
      .andReturnValue(static_cast<uint8_t>(sizeof(paramDataBytes)));

  // Set actual temperature
  mock().expectOneCall("AzureCloudSyncControllerMock::updateEcoTemperatureFromCloud").withIntParameter("temperatureFP", 17000);

  // No response required
  uint8_t returnedItems = payload->processRequest(request, response);
  CHECK_EQUAL(0U, returnedItems);
}

TEST(ParameterDataPayloadProcessSingleRequest, processOfReadEcoTemperature)
{
  ParameterDataResponse response;

  mock().expectOneCall("ParameterDataRequestMock::getParamDataID")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .andReturnValue(static_cast<uint16_t>(ParameterID::ECO_TEMPERATURE));

  // On receive we expect 1 bytes with requested temp.
  mock().expectOneCall("ParameterDataRequestMock::getParamDataSize")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .andReturnValue(static_cast<uint8_t>(1U));

  uint8_t paramDataBytes[1U] = { 0xff }; // I.e. read request
  mock().expectOneCall("ParameterDataRequestMock::getParamData")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .withOutputParameterReturning("paramDataBytes", paramDataBytes, sizeof(paramDataBytes))
      .withUnsignedIntParameter("size", sizeof(paramDataBytes))
      .andReturnValue(static_cast<uint8_t>(sizeof(paramDataBytes)));

  // Get actual temperature
  mock().expectOneCall("getEcoTemperatureSetPoint").andReturnValue(16000); // temp. reading = 16 degrees

  // Check response size
  uint8_t returnedItems = payload->processRequest(request, response);
  CHECK_EQUAL(1U, returnedItems);

  CHECK_EQUAL(8U, response.getSize()); // FrameID(2), ParamID(2), Size(1), Payload(1), EndOfFrame(2)

  // Verify bytes received in output
  uint8_t outputBytes[8U] = {};
  response.getBytes(outputBytes, sizeof(outputBytes));

  uint8_t expectedBytes[] = {
      0x21, 0x00,  // ApplianceFrameResonse FrameID
      0x40, 0x01, // EcoTemp ParameterID
      0x01, 16U,  // Size, Value
      0x00, 0x00  // EndOfFrame
  };

  MEMCMP_EQUAL(expectedBytes, outputBytes, sizeof(expectedBytes));
}

TEST(ParameterDataPayloadProcessSingleRequest, processOfSetActiveTemperatureComfort)
{
  ParameterDataResponse response;

  mock().expectOneCall("ParameterDataRequestMock::getParamDataID")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .andReturnValue(static_cast<uint16_t>(ParameterID::ACTIVE_SETPOINT_TEMPERATURE));

  mock().expectOneCall("getPrimaryHeatingMode").andReturnValue(HeatingMode::comfort);

  // On receive we expect 1 bytes with requested temp.
  mock().expectOneCall("ParameterDataRequestMock::getParamDataSize")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .andReturnValue(static_cast<uint8_t>(1U));

  uint8_t paramDataBytes[1U] = { 21U };
  mock().expectOneCall("ParameterDataRequestMock::getParamData")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .withOutputParameterReturning("paramDataBytes", paramDataBytes, sizeof(paramDataBytes))
      .withUnsignedIntParameter("size", sizeof(paramDataBytes))
      .andReturnValue(static_cast<uint8_t>(sizeof(paramDataBytes)));

  // Set actual temperature
  mock().expectOneCall("AzureCloudSyncControllerMock::updateComfortTemperatureFromCloud").withIntParameter("temperatureFP", 21000);

  // No response required
  uint8_t returnedItems = payload->processRequest(request, response);
  CHECK_EQUAL(0U, returnedItems);
}

TEST(ParameterDataPayloadProcessSingleRequest, processOfReadAntiFrostTemperatureComfort)
{
  ParameterDataResponse response;

  mock().expectOneCall("ParameterDataRequestMock::getParamDataID")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .andReturnValue(static_cast<uint16_t>(ParameterID::ACTIVE_SETPOINT_TEMPERATURE));

  mock().expectOneCall("getPrimaryHeatingMode").andReturnValue(HeatingMode::antiFrost);

  // On receive we expect 1 bytes with requested temp.
  mock().expectOneCall("ParameterDataRequestMock::getParamDataSize")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .andReturnValue(static_cast<uint8_t>(1U));

  uint8_t paramDataBytes[1U] = { 0xffU };
  mock().expectOneCall("ParameterDataRequestMock::getParamData")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .withOutputParameterReturning("paramDataBytes", paramDataBytes, sizeof(paramDataBytes))
      .withUnsignedIntParameter("size", sizeof(paramDataBytes))
      .andReturnValue(static_cast<uint8_t>(sizeof(paramDataBytes)));

  // Actual temperature isn't read from settings for AntiFrost, so no mocks for that

  // Check response size
  uint8_t returnedItems = payload->processRequest(request, response);
  CHECK_EQUAL(1U, returnedItems);

  CHECK_EQUAL(8U, response.getSize()); // FrameID(2), ParamID(2), Size(1), Payload(1), EndOfFrame(2)

  // Verify bytes received in output
  uint8_t outputBytes[8U] = {};
  response.getBytes(outputBytes, sizeof(outputBytes));

  uint8_t expectedBytes[] = {
      0x21, 0x00, // ApplianceFrameResonse FrameID
      0xdb, 0x00, // EcoTemp ParameterID
      0x01, 7U,  // Size, Value
      0x00, 0x00  // EndOfFrame
  };

  MEMCMP_EQUAL(expectedBytes, outputBytes, sizeof(expectedBytes));
}

TEST(ParameterDataPayloadProcessSingleRequest, processOfWeekProgramMonTue)
{
  ParameterDataResponse response;

  mock().expectOneCall("ParameterDataRequestMock::getParamDataID")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .andReturnValue(static_cast<uint16_t>(ParameterID::WEEK_PROGRAM_MON_TUE));

  // On receive we expect 40 bytes with week program data.
  mock().expectOneCall("ParameterDataRequestMock::getParamDataSize")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .andReturnValue(static_cast<uint8_t>(40U));

  uint8_t paramDataBytes[40U] = {
      CloudTimerHeatingMode::eco, 0, 0, 6, 0, // mon slot1
      CloudTimerHeatingMode::comfort, 6, 0 , 8 , 0, // mon slot 2
      CloudTimerHeatingMode::eco, 8, 0, 15, 0, // mon slot 3
      CloudTimerHeatingMode::comfort, 15, 0, 23, 59, // mon slot 4
      CloudTimerHeatingMode::away, 0, 0, 5, 0, // tue slot1
      CloudTimerHeatingMode::shutdown, 5, 0, 9, 0, // tue slot2
      CloudTimerHeatingMode::away, 9, 0, 16, 30, // tue slot3
      CloudTimerHeatingMode::away, 16, 30, 23, 59  // tue slot4
  };

  mock().expectOneCall("ParameterDataRequestMock::getParamData")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .withOutputParameterReturning("paramDataBytes", paramDataBytes, sizeof(paramDataBytes))
      .withUnsignedIntParameter("size", sizeof(paramDataBytes))
      .andReturnValue(static_cast<uint8_t>(sizeof(paramDataBytes)));

  // Check that we set the correct week program data for day one
  uint8_t dayOneData[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY];
  memcpy(dayOneData, &paramDataBytes[0], Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY);
  WeekProgramData dayOneProgram;
  dayOneProgram.setData(dayOneData);

  mock().expectOneCall("setWeekProgramDataForDay")
      .withIntParameter("weekDay", WeekDay::MONDAY)
      .withParameterOfType("WeekProgramData", "weekProgramData", &dayOneProgram);
  mock().expectOneCall("resetWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::MONDAY);

  // Check day two
  uint8_t dayTwoData[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY];
  memcpy(dayTwoData, &paramDataBytes[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY], Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY);
  WeekProgramData dayTwoProgram;
  dayTwoProgram.setData(dayTwoData);

  mock().expectOneCall("setWeekProgramDataForDay")
      .withIntParameter("weekDay", WeekDay::TUESDAY)
      .withParameterOfType("WeekProgramData", "weekProgramData", &dayTwoProgram);
  mock().expectOneCall("resetWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::TUESDAY);

  // No response required
  uint8_t returnedItems = payload->processRequest(request, response);
  CHECK_EQUAL(0U, returnedItems);
}

TEST(ParameterDataPayloadProcessSingleRequest, processOfWeekProgramFriSat)
{
  ParameterDataResponse response;

  mock().expectOneCall("ParameterDataRequestMock::getParamDataID")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .andReturnValue(static_cast<uint16_t>(ParameterID::WEEK_PROGRAM_FRI_SAT));

  // On receive we expect 40 bytes with week program data.
  mock().expectOneCall("ParameterDataRequestMock::getParamDataSize")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .andReturnValue(static_cast<uint8_t>(40U));

  uint8_t invalid = Constants::CLOUD_PARAMETER_INVALID_VALUE;
  uint8_t paramDataBytes[40U] = {
      CloudTimerHeatingMode::eco, 0, 0, 6, 0,
      CloudTimerHeatingMode::comfort, 6, 0 , 8 , 0,
      CloudTimerHeatingMode::eco, 8, 0, 15, 0,
      invalid, invalid, invalid, invalid, invalid,
      CloudTimerHeatingMode::away, 0, 0, 5, 0,
      CloudTimerHeatingMode::shutdown, 5, 0, 9, 0,
      invalid, invalid, invalid, invalid, invalid,
      invalid, invalid, invalid, invalid, invalid
  };

  mock().expectOneCall("ParameterDataRequestMock::getParamData")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .withOutputParameterReturning("paramDataBytes", paramDataBytes, sizeof(paramDataBytes))
      .withUnsignedIntParameter("size", sizeof(paramDataBytes))
      .andReturnValue(static_cast<uint8_t>(sizeof(paramDataBytes)));

  // Check that we set the correct week program data for day one
  uint8_t dayOneData[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY];
  memcpy(dayOneData, &paramDataBytes[0], Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY);
  WeekProgramData dayOneProgram;
  dayOneProgram.setData(dayOneData);

  mock().expectOneCall("setWeekProgramDataForDay")
      .withIntParameter("weekDay", WeekDay::FRIDAY)
      .withParameterOfType("WeekProgramData", "weekProgramData", &dayOneProgram);
  mock().expectOneCall("resetWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::FRIDAY);


  // Check day two
  uint8_t dayTwoData[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY];
  memcpy(dayTwoData, &paramDataBytes[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY], Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY);
  WeekProgramData dayTwoProgram;
  dayTwoProgram.setData(dayTwoData);

  mock().expectOneCall("setWeekProgramDataForDay")
      .withIntParameter("weekDay", WeekDay::SATURDAY)
      .withParameterOfType("WeekProgramData", "weekProgramData", &dayTwoProgram);
  mock().expectOneCall("resetWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::SATURDAY);

  // No response required
  uint8_t returnedItems = payload->processRequest(request, response);
  CHECK_EQUAL(0U, returnedItems);
}

TEST(ParameterDataPayloadProcessSingleRequest, processOfWeekProgramSunMonIgnoresMon)
{
  ParameterDataResponse response;

  mock().expectOneCall("ParameterDataRequestMock::getParamDataID")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .andReturnValue(static_cast<uint16_t>(ParameterID::WEEK_PROGRAM_SUN_MON));

  // On receive we expect 40 bytes with week program data.
  mock().expectOneCall("ParameterDataRequestMock::getParamDataSize")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .andReturnValue(static_cast<uint8_t>(40U));

  uint8_t invalid = Constants::CLOUD_PARAMETER_INVALID_VALUE;
  uint8_t paramDataBytes[40U] = {
      CloudTimerHeatingMode::eco, 0, 0, 6, 0,
      CloudTimerHeatingMode::comfort, 6, 0 , 8 , 0,
      CloudTimerHeatingMode::eco, 8, 0, 15, 0,
      invalid, invalid, invalid, invalid, invalid,
      CloudTimerHeatingMode::away, 0, 0, 5, 0,
      CloudTimerHeatingMode::shutdown, 5, 0, 9, 0,
      invalid, invalid, invalid, invalid, invalid,
      invalid, invalid, invalid, invalid, invalid
  };

  mock().expectOneCall("ParameterDataRequestMock::getParamData")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .withOutputParameterReturning("paramDataBytes", paramDataBytes, sizeof(paramDataBytes))
      .withUnsignedIntParameter("size", sizeof(paramDataBytes))
      .andReturnValue(static_cast<uint8_t>(sizeof(paramDataBytes)));

  // Check that we set the correct week program data for day one
  uint8_t dayOneData[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY];
  memcpy(dayOneData, &paramDataBytes[0], Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY);
  WeekProgramData dayOneProgram;
  dayOneProgram.setData(dayOneData);

  mock().expectOneCall("setWeekProgramDataForDay")
      .withIntParameter("weekDay", WeekDay::SUNDAY)
      .withParameterOfType("WeekProgramData", "weekProgramData", &dayOneProgram);
  mock().expectOneCall("resetWeekProgramExtDataForDay").withIntParameter("weekDay", WeekDay::SUNDAY);

  // Day two should be ignored

  // No response required
  uint8_t returnedItems = payload->processRequest(request, response);
  CHECK_EQUAL(0U, returnedItems);
}

TEST(ParameterDataPayloadProcessSingleRequest, processOfWeekProgramActivation)
{
  ParameterDataResponse response;

  mock().expectOneCall("ParameterDataRequestMock::getParamDataID")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .andReturnValue(static_cast<uint16_t>(ParameterID::ACTIVATE_WEEK_PROGRAM_MODE));

  // On receive we expect 1 byte (active/inactive).
  mock().expectOneCall("ParameterDataRequestMock::getParamDataSize")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .andReturnValue(static_cast<uint8_t>(1U));

  uint8_t paramDataBytes[1] = { 1U }; // i.e. activate week program mode

  mock().expectOneCall("ParameterDataRequestMock::getParamData")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .withOutputParameterReturning("paramDataBytes", paramDataBytes, sizeof(paramDataBytes))
      .withUnsignedIntParameter("size", sizeof(paramDataBytes))
      .andReturnValue(static_cast<uint8_t>(sizeof(paramDataBytes)));

  // Set week program mode by creating a 'None' override
  Override noOverride(OverrideType::none, HeatingMode::comfort);
  mock().expectOneCall("AzureCloudSyncControllerMock::updateOverrideFromCloud").withParameterOfType("Override", "override", &noOverride);


  // No response required
  uint8_t returnedItems = payload->processRequest(request, response);
  CHECK_EQUAL(0U, returnedItems);
}

TEST(ParameterDataPayloadProcessSingleRequest, processOfNowOverrideActivation)
{
  ParameterDataResponse response;

  mock().expectOneCall("ParameterDataRequestMock::getParamDataID")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .andReturnValue(static_cast<uint16_t>(ParameterID::ACTIVATE_CONSTANT_NOW_OVERRIDE));

  // On receive we expect 3 bytes (mode, constant/now flag, inactive/active).
  mock().expectOneCall("ParameterDataRequestMock::getParamDataSize")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .andReturnValue(static_cast<uint8_t>(3U));

  uint8_t paramDataBytes[3] = {
      static_cast<uint8_t>(CloudTimerHeatingMode::eco),
      1U, // Now override
      1U // activate
  };

  mock().expectOneCall("ParameterDataRequestMock::getParamData")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .withOutputParameterReturning("paramDataBytes", paramDataBytes, sizeof(paramDataBytes))
      .withUnsignedIntParameter("size", sizeof(paramDataBytes))
      .andReturnValue(static_cast<uint8_t>(sizeof(paramDataBytes)));

  // Set override
  Override nowOverride(OverrideType::now, HeatingMode::eco);
  mock().expectOneCall("AzureCloudSyncControllerMock::updateOverrideFromCloud").withParameterOfType("Override", "override", &nowOverride);


  // No response required
  uint8_t returnedItems = payload->processRequest(request, response);
  CHECK_EQUAL(0U, returnedItems);
}

TEST(ParameterDataPayloadProcessSingleRequest, processOfConstantOverrideActivation)
{
  ParameterDataResponse response;

  mock().expectOneCall("ParameterDataRequestMock::getParamDataID")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .andReturnValue(static_cast<uint16_t>(ParameterID::ACTIVATE_CONSTANT_NOW_OVERRIDE));

  // On receive we expect 3 bytes (mode, constant/now flag, inactive/active).
  mock().expectOneCall("ParameterDataRequestMock::getParamDataSize")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .andReturnValue(static_cast<uint8_t>(3U));

  uint8_t paramDataBytes[3] = {
      static_cast<uint8_t>(CloudTimerHeatingMode::away),
      0U, // Constant override
      1U // activate
  };

  mock().expectOneCall("ParameterDataRequestMock::getParamData")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .withOutputParameterReturning("paramDataBytes", paramDataBytes, sizeof(paramDataBytes))
      .withUnsignedIntParameter("size", sizeof(paramDataBytes))
      .andReturnValue(static_cast<uint8_t>(sizeof(paramDataBytes)));

  // Set override
  Override constantOverride(OverrideType::constant, HeatingMode::antiFrost);
  mock().expectOneCall("AzureCloudSyncControllerMock::updateOverrideFromCloud").withParameterOfType("Override", "override", &constantOverride);


  // No response required
  uint8_t returnedItems = payload->processRequest(request, response);
  CHECK_EQUAL(0U, returnedItems);
}

TEST(ParameterDataPayloadProcessSingleRequest, processOfConstantOverrideActivationIgnoredForShutdown)
{
  ParameterDataResponse response;

  mock().expectOneCall("ParameterDataRequestMock::getParamDataID")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .andReturnValue(static_cast<uint16_t>(ParameterID::ACTIVATE_CONSTANT_NOW_OVERRIDE));

  // On receive we expect 3 bytes (mode, constant/now flag, inactive/active).
  mock().expectOneCall("ParameterDataRequestMock::getParamDataSize")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .andReturnValue(static_cast<uint8_t>(3U));

  uint8_t paramDataBytes[3] = {
      static_cast<uint8_t>(CloudTimerHeatingMode::shutdown),
      0U, // Constant override
      1U // activate
  };

  mock().expectOneCall("ParameterDataRequestMock::getParamData")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .withOutputParameterReturning("paramDataBytes", paramDataBytes, sizeof(paramDataBytes))
      .withUnsignedIntParameter("size", sizeof(paramDataBytes))
      .andReturnValue(static_cast<uint8_t>(sizeof(paramDataBytes)));

  // Set override ignored
  mock().expectNoCall("AzureCloudSyncControllerMock::updateOverrideFromCloud");


  // No response required
  uint8_t returnedItems = payload->processRequest(request, response);
  CHECK_EQUAL(0U, returnedItems);
}

TEST(ParameterDataPayloadProcessSingleRequest, processOfConstantOverrideActivationIgnoredForUnkown)
{
  ParameterDataResponse response;

  mock().expectOneCall("ParameterDataRequestMock::getParamDataID")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .andReturnValue(static_cast<uint16_t>(ParameterID::ACTIVATE_CONSTANT_NOW_OVERRIDE));

  // On receive we expect 3 bytes (mode, constant/now flag, inactive/active).
  mock().expectOneCall("ParameterDataRequestMock::getParamDataSize")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .andReturnValue(static_cast<uint8_t>(3U));

  uint8_t paramDataBytes[3] = {
      static_cast<uint8_t>(CloudTimerHeatingMode::unknown),
      0U, // Constant override
      1U // activate
  };

  mock().expectOneCall("ParameterDataRequestMock::getParamData")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .withOutputParameterReturning("paramDataBytes", paramDataBytes, sizeof(paramDataBytes))
      .withUnsignedIntParameter("size", sizeof(paramDataBytes))
      .andReturnValue(static_cast<uint8_t>(sizeof(paramDataBytes)));

  // Set override ignored
  mock().expectNoCall("AzureCloudSyncControllerMock::updateOverrideFromCloud");


  // No response required
  uint8_t returnedItems = payload->processRequest(request, response);
  CHECK_EQUAL(0U, returnedItems);
}


TEST(ParameterDataPayloadProcessSingleRequest, processOfTimedOverrideActivation)
{
  ParameterDataResponse response;

  mock().expectOneCall("ParameterDataRequestMock::getParamDataID")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .andReturnValue(static_cast<uint16_t>(ParameterID::ACTIVATE_TIMED_OVERRIDE));

  // On receive we expect 12 bytes
  mock().expectOneCall("ParameterDataRequestMock::getParamDataSize")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .andReturnValue(static_cast<uint8_t>(12U));

  uint8_t paramDataBytes[12] = {
      static_cast<uint8_t>(CloudTimerHeatingMode::eco),
      1U, // activate
      25U, 07U, 18U, // day, month, year
      01U, // Tuesday
      0x20, 0x1c, 0U, 0U, // utc offset: +7200
      10U, 20U  // hh:mm
  };

  mock().expectOneCall("ParameterDataRequestMock::getParamData")
      .withUnsignedIntParameter("paramDataIndex", 0U)
      .withOutputParameterReturning("paramDataBytes", paramDataBytes, sizeof(paramDataBytes))
      .withUnsignedIntParameter("size", sizeof(paramDataBytes))
      .andReturnValue(static_cast<uint8_t>(sizeof(paramDataBytes)));

  // Override has UTC offset, we check this vs settings
  mock().expectOneCall("getUtcOffset").andReturnValue(0); // we don't care, return value isn't used for anything but logging

  // Handle time, we must adjust for UTC setting manually based on payload data
  DateTime dateTime = { 18U, 07U, 25U, 10U, 20U, 00U, WeekDay::TUESDAY }; // unixTime = 1532514000
  mock().expectOneCall("SystemTimeDriverMock::getUnixTimeFromDateTime").withParameterOfType("DateTime", "dateTime", &dateTime).andReturnValue(1532514000U);

  DateTime dateTimeWithUtc = dateTime;
  dateTimeWithUtc.hours += 2U;
  mock().expectOneCall("SystemTimeDriverMock::getDateTimeFromUnixTime")
      .withUnsignedIntParameter("unixTime", 1532514000U + 7200U)
      .withOutputParameterOfTypeReturning("DateTime", "dateTime", &dateTimeWithUtc);

  // Set override
  Override timedOverride(OverrideType::timed, HeatingMode::eco, dateTimeWithUtc);
  mock().expectOneCall("AzureCloudSyncControllerMock::updateOverrideFromCloud").withParameterOfType("Override", "override", &timedOverride);


  // No response required
  uint8_t returnedItems = payload->processRequest(request, response);
  CHECK_EQUAL(0U, returnedItems);
}
