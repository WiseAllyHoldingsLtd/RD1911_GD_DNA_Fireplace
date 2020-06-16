#include <cstring>
#include <algorithm>
#include "EspCpp.hpp"
#include "Constants.h"
#include "ParameterDataPayload.h"
#include "WeekProgramExtData.h"

namespace
{
  const char LOG_TAG[] = "ParameterData";
  const uint8_t CONSTANT_OVERRIDE_CODE = 0U;

  #pragma pack(push, 1)
  struct TimedOverrideData
  {
    uint8_t mode;
    uint8_t activate;
    uint8_t day;
    uint8_t month;
    uint8_t year;
    uint8_t weekDay;
    int32_t utcOffset;
    uint8_t hours;
    uint8_t minutes;
  };
  #pragma pack(pop)
}


ParameterDataPayload::ParameterDataPayload(SettingsInterface &settings, DeviceMetaInterface &meta, AzureCloudSyncControllerInterface &syncController, SystemTimeDriverInterface &sysTime)
  : m_settings(settings), m_deviceMeta(meta), m_settingsSync(syncController), m_sysTime(sysTime)
{
}

uint8_t ParameterDataPayload::processRequest(const ParameterDataRequestInterface & request, ParameterDataResponse & outputResponse)
{
  uint8_t numOfResponseItems = 0U;

  if (request.isValid())
  {
    uint8_t numOfParams = request.getNumOfParamDataItems();

    for (uint8_t i = 0U; i < numOfParams; ++i)
    {
      uint16_t paramID = request.getParamDataID(i);
      uint8_t paramSize = request.getParamDataSize(i);
      uint8_t * paramData = nullptr;

      if (paramSize > 0U)
      {
        paramData = new uint8_t[paramSize];
        request.getParamData(i, paramData, paramSize);
      }

      switch (paramID)
      {
      case ParameterID::DEVICE_DETAILS:
        numOfResponseItems += processDeviceData(paramData, paramSize, outputResponse);
        break;
      case ParameterID::DEPRECATED_ACTIVATE_WEEK_PROGRAM_MODE: // fall through
      case ParameterID::ACTIVATE_WEEK_PROGRAM_MODE:
        numOfResponseItems += processActivateWeekProgram(paramData, paramSize);
        break;
      case ParameterID::DEPRECATED_ACTIVATE_CONSTANT_NOW_OVERRIDE: // fall through
      case ParameterID::ACTIVATE_CONSTANT_NOW_OVERRIDE:
        numOfResponseItems += processActivateNowOrConstantOverride(paramData, paramSize);
        break;
      case ParameterID::DEPRECATED_ACTIVATE_TIMED_OVERRIDE: // fall through
      case ParameterID::ACTIVATE_TIMED_OVERRIDE:
        numOfResponseItems += processActivateTimedOverride(paramData, paramSize);
        break;
      case ParameterID::DEPRECATED_WEEK_PROGRAM_MON_TUE: // fall through
      case ParameterID::WEEK_PROGRAM_MON_TUE:
        numOfResponseItems += processWeekProgramData(paramData, paramSize, WeekDay::MONDAY, WeekDay::TUESDAY);
        break;
      case ParameterID::DEPRECATED_WEEK_PROGRAM_WED_THU: // fall through
      case ParameterID::WEEK_PROGRAM_WED_THU:
        numOfResponseItems += processWeekProgramData(paramData, paramSize, WeekDay::WEDNESDAY, WeekDay::THURSDAY);
        break;
      case ParameterID::DEPRECATED_WEEK_PROGRAM_FRI_SAT: // fall through
      case ParameterID::WEEK_PROGRAM_FRI_SAT:
        numOfResponseItems += processWeekProgramData(paramData, paramSize, WeekDay::FRIDAY, WeekDay::SATURDAY);
        break;
      case ParameterID::DEPRECATED_WEEK_PROGRAM_SUN_MON: // fall through
      case ParameterID::WEEK_PROGRAM_SUN_MON:
        numOfResponseItems += processWeekProgramData(paramData, paramSize, WeekDay::SUNDAY, WeekDay::MONDAY);
        break;
      case ParameterID::DEPRECATED_COMFORT_TEMPERATURE: // fall through
      case ParameterID::COMFORT_TEMPERATURE:
        numOfResponseItems += processTemperatureSetPoint(paramData, paramSize, HeatingMode::comfort, outputResponse, ParameterID::COMFORT_TEMPERATURE);
        break;
      case ParameterID::DEPRECATED_ECO_TEMPERATURE: // fall through
      case ParameterID::ECO_TEMPERATURE:
        numOfResponseItems += processTemperatureSetPoint(paramData, paramSize, HeatingMode::eco, outputResponse, ParameterID::ECO_TEMPERATURE);
        break;
      case ParameterID::ACTIVE_SETPOINT_TEMPERATURE:
        numOfResponseItems += processTemperatureSetPoint(paramData, paramSize, m_settings.getPrimaryHeatingMode(), outputResponse, ParameterID::ACTIVE_SETPOINT_TEMPERATURE);
        break;
      case ParameterID::WEEK_PROGRAM_EXT_MON:
        numOfResponseItems += processWeekProgramDataExtended(paramData, paramSize, WeekDay::MONDAY);
        break;
      case ParameterID::WEEK_PROGRAM_EXT_TUE:
        numOfResponseItems += processWeekProgramDataExtended(paramData, paramSize, WeekDay::TUESDAY);
        break;
      case ParameterID::WEEK_PROGRAM_EXT_WED:
        numOfResponseItems += processWeekProgramDataExtended(paramData, paramSize, WeekDay::WEDNESDAY);
        break;
      case ParameterID::WEEK_PROGRAM_EXT_THU:
        numOfResponseItems += processWeekProgramDataExtended(paramData, paramSize, WeekDay::THURSDAY);
        break;
      case ParameterID::WEEK_PROGRAM_EXT_FRI:
        numOfResponseItems += processWeekProgramDataExtended(paramData, paramSize, WeekDay::FRIDAY);
        break;
      case ParameterID::WEEK_PROGRAM_EXT_SAT:
        numOfResponseItems += processWeekProgramDataExtended(paramData, paramSize, WeekDay::SATURDAY);
        break;
      case ParameterID::WEEK_PROGRAM_EXT_SUN:
        numOfResponseItems += processWeekProgramDataExtended(paramData, paramSize, WeekDay::SUNDAY);
        break;
      case ParameterID::LOCK_STATUS:
        numOfResponseItems += processLockStatus(paramData, paramSize);
        break;
      default:
        ESP_LOGW(LOG_TAG, "Ignoring parameterID - unknown ID (0x%04x)", paramID);
        break;
      }

      if (paramSize > 0U)
      {
        delete [] paramData;
      }
    }

    /* Finished looping through all params - finalize output */
    outputResponse.finalize();
  }
  else
  {
    ESP_LOGW(LOG_TAG, "Ignoring request - invalid data.");
  }

  return numOfResponseItems;
}

uint8_t ParameterDataPayload::processDeviceData(const uint8_t * data, uint8_t size, ParameterDataResponse & outputResponse)
{
  uint8_t gdidBCD[Constants::GDID_BCD_SIZE];
  m_deviceMeta.getGDIDAsBCD(gdidBCD);

  uint8_t deviceData[Constants::GDID_BCD_SIZE + 2U];
  deviceData[0U] = Constants::COMPANY_CODE;
  deviceData[1U] = Constants::COMPANY_SUBSET_CODE;
  memcpy(&deviceData[2U], gdidBCD, Constants::GDID_BCD_SIZE);


  uint8_t returnNum = 0U;

  if (outputResponse.addParameterData(ParameterID::DEVICE_DETAILS, deviceData, sizeof(deviceData)))
  {
    ESP_LOGI(LOG_TAG, "DeviceDetails OK.");
    returnNum = 1U;
  }
  else
  {
    ESP_LOGW(LOG_TAG, "DeviceDetails failed.");
  }

  return returnNum;
}

uint8_t ParameterDataPayload::processActivateWeekProgram(const uint8_t * data, uint8_t size)
{
  if (size == 1U)
  {
    // One byte containing a flag (active/inactive). We only take action is override is flag is set,
    // as it is unclear what actions we are supposed to take for 'inactive'.
    bool shouldClearOverride = (data[0U] > 0U);

    if (shouldClearOverride)
    {
      Override noOverride(OverrideType::none, HeatingMode::comfort);
      m_settingsSync.updateOverrideFromCloud(noOverride);
    }
    else
    {
      ESP_LOGI(LOG_TAG, "ActivateTimerMode request ignored. Status is set to 'inactive'.");
    }
  }
  else
  {
    ESP_LOGW(LOG_TAG, "ActivateTimerMode request ignored. Unexpected size (%u).", size);
  }

  return 0U;
}

uint8_t ParameterDataPayload::processActivateNowOrConstantOverride(const uint8_t * data, uint8_t size)
{
  // TODO: Note that requests with mode 'unknown' should cause us to report actual data back. This is intentionally
  // ignored for now as expected behaviour is unclear. Messages with 'status=inactive' is also ignored for now.
  if (size == 3U)
  {
    bool shouldSetOverride = (data[2] > 0U); // inactive/active flag

    if (shouldSetOverride)
    {
      CloudTimerHeatingMode::Enum mode = getModeEnumFromByte(data[0]);

      if (mode == CloudTimerHeatingMode::shutdown)
      {
        ESP_LOGI(LOG_TAG, "ActivateNowOrConstantOverride request ignored. Shutdown mode not allowed in overrides.");
      }
      else if (mode == CloudTimerHeatingMode::unknown) // read request
      {
        ESP_LOGI(LOG_TAG, "ActivateNowOrConstantOverride read request ignored. Not supported.");
      }
      else
      {
        OverrideType::Enum type = (data[1] == CONSTANT_OVERRIDE_CODE ? OverrideType::constant : OverrideType::now);
        Override override(type, convertCloudModeToHeatingMode(mode));
        m_settingsSync.updateOverrideFromCloud(override);
      }
    }
    else
    {
      ESP_LOGI(LOG_TAG, "ActivateNowOrConstantOverride request ignored. Status is set to 'inactive'.");
    }
  }
  else
  {
    ESP_LOGW(LOG_TAG, "ActivateNowOrConstantOverride request ignored. Unexpected size (%u).", size);
  }

  return 0U;
}

uint8_t ParameterDataPayload::processActivateTimedOverride(const uint8_t * data, uint8_t size)
{
  if (size == sizeof(TimedOverrideData))
  {
    TimedOverrideData overrideData;
    memcpy(&overrideData, data, size);

    if (overrideData.activate)
    {
      CloudTimerHeatingMode::Enum mode = getModeEnumFromByte(overrideData.mode);

      if (mode == CloudTimerHeatingMode::shutdown)
      {
        ESP_LOGI(LOG_TAG, "ActivateTimedOverride request ignored. Shutdown mode not allowed in overrides.");
      }
      else if (mode == CloudTimerHeatingMode::unknown) // read request
      {
        ESP_LOGI(LOG_TAG, "ActivateTimedOverride read request ignored. Not supported.");
      }
      else
      {
        DateTime dateTime = {
            overrideData.year, overrideData.month, overrideData.day,
            overrideData.hours, overrideData.minutes, 0U,
            getWeekDayFromCloudByte(overrideData.weekDay)
        };

        if (m_settings.getUtcOffset() != overrideData.utcOffset)
        {
          ESP_LOGW(LOG_TAG, "Override UTC offset (%d) does not match UTC offset in settings (%d).", overrideData.utcOffset, m_settings.getUtcOffset());
        }

        uint32_t unixTimeUtc0 = m_sysTime.getUnixTimeFromDateTime(dateTime);
        m_sysTime.getDateTimeFromUnixTime(unixTimeUtc0 + overrideData.utcOffset, dateTime);

        Override override(OverrideType::timed, convertCloudModeToHeatingMode(mode), dateTime);
        m_settingsSync.updateOverrideFromCloud(override);
      }
    }
    else
    {
      ESP_LOGI(LOG_TAG, "ActivateNowOrConstantOverride request ignored. Status is set to 'inactive'.");
    }
  }
  else
  {
    ESP_LOGW(LOG_TAG, "ActivateTimedOverride request ignored. Expected size %u, got %u.", sizeof(TimedOverrideData), size);
  }

  return 0U;
}

uint8_t ParameterDataPayload::processWeekProgramData(const uint8_t * data, uint8_t size, WeekDay::Enum weekDayOne, WeekDay::Enum weekDayTwo)
{
  if (size == (2 * Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY)) // due to 2* we know we have an even number
  {
    uint8_t buffer[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY];
    WeekProgramData weekProgData;

    /* Day one */
    memcpy(buffer, &data[0], sizeof(buffer));
    weekProgData.setData(buffer);
    m_settings.setWeekProgramDataForDay(weekDayOne, weekProgData);
    ESP_LOGI(LOG_TAG, "SetWeekProgram OK. Day: %d", static_cast<int32_t>(weekDayOne));

    // If we ever receive an "old" type week program (limited to four nodes),
    // we write an invalid extended week program to make sure
    // that the old one is used.
    m_settings.resetWeekProgramExtDataForDay(weekDayOne);

    /* Day two */
    if (!((weekDayOne == WeekDay::SUNDAY) && (weekDayTwo == WeekDay::MONDAY))) // Monday is sent twice. This ugly hack ignores the second.
    {
      memcpy(buffer, &data[sizeof(buffer)], sizeof(buffer));
      weekProgData.setData(buffer);
      m_settings.setWeekProgramDataForDay(weekDayTwo, weekProgData);
      ESP_LOGI(LOG_TAG, "SetWeekProgram OK. Day: %d", static_cast<int32_t>(weekDayTwo));
      m_settings.resetWeekProgramExtDataForDay(weekDayTwo);
    }
  }
  else
  {
    ESP_LOGW(LOG_TAG, "SetWeekProgram request ignored. Unexpected size (%u).", static_cast<uint32_t>(size));
  }

  return 0U;
}

uint8_t ParameterDataPayload::processWeekProgramDataExtended(const uint8_t *data, uint8_t size, WeekDay::Enum weekDay)
{
  ESP_LOGI(LOG_TAG, "processWeekProgramDataExtended called for day Day: %u", static_cast<uint32_t>(weekDay));

  if ( size % Constants::WEEKPROGRAM_SLOT_SIZE == 0u )
  {
    // Limit size to make it consistent with max size in storage (just discard any excess nodes)
    size = std::min(size, Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY_EXT_MAX);

    WeekProgramExtData weekProgramExtData;
    weekProgramExtData.setData(data, size);
    m_settings.setWeekProgramExtDataForDay(weekDay, weekProgramExtData);
    ESP_LOGI(LOG_TAG, "SetWeekProgram extended OK. Day: %u", static_cast<uint32_t>(weekDay));
  }
  else
  {
    ESP_LOGW(LOG_TAG, "SetWeekProgram extended request ignored. Unexpected size (%u).", static_cast<uint32_t>(size));
  }
  return 0u;
}

uint8_t ParameterDataPayload::processTemperatureSetPoint(const uint8_t * data, uint8_t size, HeatingMode::Enum heatingMode, ParameterDataResponse & outputResponse, ParameterID::Enum requestID)
{
  const uint8_t READ_REQUEST_VALUE = 0xFF;
  uint8_t returnedItems = 0U;

  if (size == 1U)
  {
    // One byte containing the temperature, or a read request byte
    // If we set temperature, we also report the new value
    uint8_t temperature = data[0U];


    if (temperature != READ_REQUEST_VALUE)
    {
      // Set Eco or Comfort
      int temperatureFP = (1000 * static_cast<int>(temperature));

      switch (heatingMode)
      {
      case HeatingMode::comfort:
        ESP_LOGD(LOG_TAG, "SetComfortTemperature. Temperature: %u", temperature);
        m_settingsSync.updateComfortTemperatureFromCloud(temperatureFP);
        break;
      case HeatingMode::eco:
        ESP_LOGD(LOG_TAG, "SetEcoTemperature. Temperature: %u", temperature);
        m_settingsSync.updateEcoTemperatureFromCloud(temperatureFP);
        break;
      default:
        // not possible to set temperature for other modes.
        ESP_LOGW(LOG_TAG, "SetTemperature ignored. HeatingMode: %d, temperature: %u", heatingMode, temperature);
        break;
      }
    }
    else
    {
      uint8_t temperatureReading = 0U;

      switch (heatingMode)
      {
      case HeatingMode::comfort:
        temperatureReading = static_cast<uint8_t>(m_settings.getComfortTemperatureSetPoint() / 1000);
        ESP_LOGI(LOG_TAG, "ReadComfortTemperature. Temperature: %u", temperatureReading);
        break;
      case HeatingMode::eco:
        temperatureReading = static_cast<uint8_t>(m_settings.getEcoTemperatureSetPoint() / 1000);
        ESP_LOGI(LOG_TAG, "ReadEcoTemperature. Temperature: %u", temperatureReading);
        break;
      case HeatingMode::antiFrost:
        temperatureReading = static_cast<uint8_t>(Constants::ANTI_FROST_SET_POINT / 1000);
        ESP_LOGI(LOG_TAG, "ReadAntiFrostTemperature. Temperature: %u", temperatureReading);
        break;
      case HeatingMode::off:
        ESP_LOGI(LOG_TAG, "ReadStandbyTemperature. Temperature: %u", temperatureReading);
        break;
      default:
        ESP_LOGW(LOG_TAG, "ReadInvalidModeTemperature. HeatingMode: %d, temperature: %u", heatingMode, temperatureReading);
        break;
      }

      uint8_t data[1U] = { temperatureReading };

      if (outputResponse.addParameterData(requestID, data, sizeof(data)))
      {
        ESP_LOGI(LOG_TAG, "ReadTemperatureRequest OK.");
        returnedItems = 1U;
      }
      else
      {
        ESP_LOGW(LOG_TAG, "ReadTemperatureRequest OK.");
      }
    }
  }
  else
  {
    ESP_LOGW(LOG_TAG, "SetTemperature request ignored. Unexpected size (%u).", size);
  }

  return returnedItems;
}

uint8_t ParameterDataPayload::processLockStatus(const uint8_t *data, uint8_t size)
{
  if ( size == 1u )
  {
    switch ( data[0] )
    {
    case 0:
      m_settingsSync.updateButtonLockStatusFromCloud(false);
      break;
    case 1:
      m_settingsSync.updateButtonLockStatusFromCloud(true);
      break;
    default:
      ESP_LOGW(LOG_TAG, "LockStatus request ignored. Unsupported lock type (%u).",
          static_cast<uint32_t>(data[0]));
      break;
    }
  }
  else
  {
    ESP_LOGW(LOG_TAG, "LockStatus request from cloud ignored. Unexpected size(%u)",
        static_cast<uint32_t>(size));
  }

  return 0;
}

CloudTimerHeatingMode::Enum ParameterDataPayload::getModeEnumFromByte(uint8_t code) const
{
  CloudTimerHeatingMode::Enum mode = static_cast<CloudTimerHeatingMode::Enum>(code);

  switch (mode)
  {
  case CloudTimerHeatingMode::comfort:
  case CloudTimerHeatingMode::eco:
  case CloudTimerHeatingMode::away:
  case CloudTimerHeatingMode::shutdown:
    break;
  default:
    mode = CloudTimerHeatingMode::unknown;
    break;
  }

  return mode;
}

HeatingMode::Enum ParameterDataPayload::convertCloudModeToHeatingMode(CloudTimerHeatingMode::Enum cloudMode) const
{
  HeatingMode::Enum heatingMode = HeatingMode::comfort;

  switch (cloudMode)
  {
  case CloudTimerHeatingMode::eco:
    heatingMode = HeatingMode::eco;
    break;
  case CloudTimerHeatingMode::away:
    heatingMode = HeatingMode::antiFrost;
    break;
  case CloudTimerHeatingMode::shutdown:
    heatingMode = HeatingMode::off;
    break;
  default:
    // comfort
    break;
  }

  return heatingMode;
}

WeekDay::Enum ParameterDataPayload::getWeekDayFromCloudByte(uint8_t cloudByte) const
{
  // WeekDay enum has Sunday = 0, Monday = 1, ..., Saturday = 6
  // Cloud def: Monday = 0, Tuesday = 1, ..., Sunday = 6
  uint8_t weekDay = ((cloudByte + 1U) % 7U);
  return static_cast<WeekDay::Enum>(weekDay);
}
