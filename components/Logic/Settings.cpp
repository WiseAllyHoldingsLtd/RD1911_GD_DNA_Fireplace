#include <cstring>
#include <algorithm>
#include "Settings.h"


Settings::Settings(SettingsStorageInterface & storage, RtcInterface &rtc, SystemTimeDriverInterface &sysTime, TimerDriverInterface &timer)
  : m_storage(storage), m_rtc(rtc), m_systemTime(sysTime), m_timerDriver(timer)
{
  setDefaultSettings();
  m_isModified = false;  // Override modified flag in constructor
}

void Settings::setDefaultSettings()
{
  m_runMode = Constants::DEFAULT_RUN_MODE;
  setDefaultSettingsExceptRunMode();
}

void Settings::setDefaultSettingsExceptRunMode()
{
  setWifiSSID(Constants::DEFAULT_WIFI_SSID);
  setWifiPassword(Constants::DEFAULT_WIFI_PASSWORD);

  m_staticIp = 0u;
  m_staticNetmask = 0u;
  m_staticGateway = 0u;
  m_staticDns1 = 0u;
  m_staticDns2 = 0u;

  m_comfortTemperatureSetPoint = Constants::DEFAULT_COMFORT_TEMPERATURE_SET_POINT;
  m_ecoTemperatureSetPoint = Constants::DEFAULT_ECO_TEMPERATURE_SET_POINT;
  m_primaryHeatingMode = HeatingMode::comfort;
  m_override = Override(OverrideType::none, HeatingMode::comfort);
  m_isButtonLockOn = Constants::DEFAULT_IS_BUTTON_LOCK_ON;
  m_isWeekProgramUpdated = false;
  m_utcOffset = Constants::DEFAULT_UTC_OFFSET;
  m_numOfWeekProgramNodes = 0u;
  m_isModified = true;
  m_lastTimeRead = 0u;
  m_lastTimeSet = 0;
  m_cachedTime = Constants::DEFAULT_DATETIME;
  m_fwUpgradeAttempts = 0U;
  m_fwVersionCode = 0U;
  memset(m_firmwareID, 0, sizeof(m_firmwareID));

  /* Set week program data for all days to invalid data (i.e. none set) */
  for (uint8_t i = 0U; i < NUM_OF_WEEKDAYS; i++)
  {
    m_arrayOfWeekProgramData[i].setData(Constants::DEFAULT_WEEK_PROGRAM_DATA);
    m_arrayOfWeekProgramExtData[i].setData(Constants::DEFAULT_WEEK_PROGRAM_EXT_DATA,
                                           sizeof(Constants::DEFAULT_WEEK_PROGRAM_EXT_DATA));
  }

  m_forgetMeState = ForgetMeState::noResetRequested;
}


void Settings::storeSettings(void)
{
  m_storeRetrieveLock.take();
  m_storage.storeRunMode(getRunMode());
  m_storage.storeWifiSSID(m_wifiSSID);
  m_storage.storeWifiPassword(m_wifiPassword);
  m_storage.storeStaticIp(m_staticIp);
  m_storage.storeStaticNetmask(m_staticNetmask);
  m_storage.storeStaticGateway(m_staticGateway);
  m_storage.storeStaticDns1(m_staticDns1);
  m_storage.storeStaticDns2(m_staticDns2);
  m_storage.storeUtcOffset(getUtcOffset());
  m_storage.storePrimaryHeatingMode(getPrimaryHeatingMode());
  m_storage.storeComfortTemperatureSetPoint(getComfortTemperatureSetPoint());
  m_storage.storeEcoTemperatureSetPoint(getEcoTemperatureSetPoint());
  m_storage.storeOverride(m_override);
  m_storage.storeIsButtonLockOn(isButtonLockOn());
  m_storage.storeFirmwareUpgradeAttempts(getFirmwareUpgradeCount());
  m_storage.storeFirmwareVersionCode(getFirmwareVersionCode());
  m_storage.storeFirmwareID(m_firmwareID);

  WeekProgramData weekProgramData;
  uint8_t bytes[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY];

  for (uint8_t i = 0u; i < NUM_OF_WEEKDAYS; ++i)
  {
    // "old" 4-node-limited week program data
    getWeekProgramDataForDay(static_cast<WeekDay::Enum>(i), weekProgramData);
    weekProgramData.getData(bytes);
    m_storage.storeWeekProgramDataForDay(static_cast<WeekDay::Enum>(i), bytes);

    // "new" extended week program data
    WeekProgramExtData weekProgramExtData;
    getWeekProgramExtDataForDay(static_cast<WeekDay::Enum>(i), weekProgramExtData);
    uint8_t weekProgramExtDataSize = std::min(weekProgramExtData.getCurrentSize(), Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY_EXT_MAX);
    uint8_t bytesExt[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY_EXT_MAX] = {};
    weekProgramExtData.getData(bytesExt, weekProgramExtDataSize);
    m_storage.storeWeekProgramExtDataForDay(static_cast<WeekDay::Enum>(i), bytesExt, weekProgramExtDataSize);
  }
  m_storage.storeForgetMeState(m_forgetMeState);

  m_isModified = false;
  m_storeRetrieveLock.give();
}


void Settings::retrieveSettings()
{
  m_storeRetrieveLock.take();
  RunMode::Enum runMode;
  HeatingMode::Enum heatingMode;
  int32_t setPoint, utcOffset;
  Override override;
  uint32_t versionCode;
  uint8_t attempts;
  bool enabled;
  char ssid[Constants::WIFI_SSID_SIZE_MAX];
  char password[Constants::WIFI_PASSWORD_SIZE_MAX];
  uint32_t staticIp;
  uint32_t staticNetmask;
  uint32_t staticGateway;
  uint32_t staticDns1;
  uint32_t staticDns2;
  char fwID[Constants::FW_DOWNLOAD_ID_MAX_LENGTH];
  ForgetMeState::Enum forgetMeState;

  if (m_storage.retrieveRunMode(runMode))
  {
    setRunMode(runMode);
  }

  if (m_storage.retrieveWifiSSID(ssid, sizeof(ssid)))
  {
    setWifiSSID(ssid);
  }

  if (m_storage.retrieveWifiPassword(password, sizeof(password)))
  {
    setWifiPassword(password);
  }

  if (m_storage.retrieveStaticIp(staticIp))
  {
    setStaticIp(staticIp);
  }

  if (m_storage.retrieveStaticNetmask(staticNetmask))
  {
    setStaticNetmask(staticNetmask);
  }

  if (m_storage.retrieveStaticGateway(staticGateway))
  {
    setStaticGateway(staticGateway);
  }

  if (m_storage.retrieveStaticDns1(staticDns1))
  {
    setStaticDns1(staticDns1);
  }

  if (m_storage.retrieveStaticDns2(staticDns2))
  {
    setStaticDns2(staticDns2);
  }

  if (m_storage.retrieveUtcOffset(utcOffset))
  {
    setUtcOffset(utcOffset);
  }

  if (m_storage.retrievePrimaryHeatingMode(heatingMode)) {
    setPrimaryHeatingMode(heatingMode);
  }

  if (m_storage.retrieveComfortTemperatureSetPoint(setPoint)) {
    setComfortTemperatureSetPoint(setPoint);
  }

  if (m_storage.retrieveEcoTemperatureSetPoint(setPoint)) {
    setEcoTemperatureSetPoint(setPoint);
  }

  if (m_storage.retrieveOverride(override))
  {
    setOverride(override);
  }

  if (m_storage.retrieveIsButtonLockOn(enabled)) {
    setIsButtonLockOn(enabled);
  }

  if (m_storage.retrieveFirmwareUpgradeAttempts(attempts))
  {
    setFirmwareUpgradeCount(attempts);
  }

  if (m_storage.retrieveFirmwareVersionCode(versionCode))
  {
    setFirmwareVersionCode(versionCode);
  }

  if (m_storage.retrieveFirmwareID(fwID, sizeof(fwID)))
  {
    setFirmwareID(fwID);
  }

  uint8_t bytes[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY];
  WeekProgramData weekProgramData;

  for (uint8_t i = 0u; i < NUM_OF_WEEKDAYS; ++i)
  {
    if (m_storage.retrieveWeekProgramDataForDay(static_cast<WeekDay::Enum>(i), bytes))
    {
      weekProgramData.setData(bytes);
      setWeekProgramDataForDay(static_cast<WeekDay::Enum>(i), weekProgramData);
    }

    uint8_t bytesExt[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY_EXT_MAX] = {};
    uint32_t bytesExtSize = sizeof(bytesExt);
    if ( m_storage.retrieveWeekProgramExtDataForDay(static_cast<WeekDay::Enum>(i), bytesExt, bytesExtSize) )
    {
      WeekProgramExtData weekProgramExtData;
      weekProgramExtData.setData(bytesExt, bytesExtSize);
      setWeekProgramExtDataForDay(static_cast<WeekDay::Enum>(i), weekProgramExtData);
    }
  }

  if ( m_storage.retrieveForgetMeState(forgetMeState) )
  {
    setForgetMeState(forgetMeState);
  }


  m_isModified = false;
  m_storeRetrieveLock.give();
}


RunMode::Enum Settings::getRunMode(void)
{
  return m_runMode;
}


void Settings::setRunMode(RunMode::Enum runMode)
{
  switch (runMode)
  {
  case RunMode::normal:
  case RunMode::test:
    break;
  default:
    runMode = Constants::DEFAULT_RUN_MODE;
    break;
  }

  m_runMode = runMode;
  m_isModified = true;
}


bool Settings::isTestMode(void)
{
  return (getRunMode() == RunMode::test);
}


void Settings::getWifiSSID(char * ssid, uint32_t size)
{
  m_accessorLock.take();
  strncpy(ssid, m_wifiSSID, size);
  ssid[size - 1U] = '\0';
  m_accessorLock.give();
}


void Settings::setWifiSSID(const char * ssid)
{
  m_accessorLock.take();
  strncpy(m_wifiSSID, ssid, sizeof(m_wifiSSID));
  m_wifiSSID[sizeof(m_wifiSSID) - 1U] = '\0';

  m_isModified = true;
  m_accessorLock.give();
}


void Settings::getWifiPassword(char * password, uint32_t size)
{
  m_accessorLock.take();
  strncpy(password, m_wifiPassword, size);
  password[size - 1U] = '\0';
  m_accessorLock.give();
}


void Settings::setWifiPassword(const char * password)
{
  m_accessorLock.take();
  strncpy(m_wifiPassword, password, sizeof(m_wifiPassword));
  m_wifiPassword[sizeof(m_wifiPassword) - 1U] = '\0';

  m_isModified = true;
  m_accessorLock.give();
}

void Settings::setStaticIp(uint32_t staticIp)
{
  m_staticIp = staticIp;
  m_isModified = true;
}

uint32_t Settings::getStaticIp()
{
  return m_staticIp;
}

void Settings::setStaticNetmask(uint32_t staticNetmask)
{
  m_staticNetmask = staticNetmask;
  m_isModified = true;
}

uint32_t Settings::getStaticNetmask()
{
  return m_staticNetmask;
}

void Settings::setStaticGateway(uint32_t staticGateway)
{
  m_staticGateway = staticGateway;
  m_isModified = true;
}

uint32_t Settings::getStaticGateway()
{
  return m_staticGateway;
}

void Settings::setStaticDns1(uint32_t staticDns1)
{
  m_staticDns1 = staticDns1;
  m_isModified = true;
}

uint32_t Settings::getStaticDns1()
{
  return m_staticDns1;
}

void Settings::setStaticDns2(uint32_t staticDns2)
{
  m_staticDns2 = staticDns2;
  m_isModified = true;
}

uint32_t Settings::getStaticDns2()
{
  return m_staticDns2;
}

int32_t Settings::getComfortTemperatureSetPoint(void)
{
  return m_comfortTemperatureSetPoint;
}

void Settings::setComfortTemperatureSetPoint(int32_t setPoint)
{
  /* Input validation: Use default value if outside valid range. */
  if ((setPoint > Constants::MAX_SETP_TEMPERATURE) || (setPoint < Constants::MIN_SETP_TEMPERATURE))
  {
    setPoint = Constants::DEFAULT_COMFORT_TEMPERATURE_SET_POINT;
  }

  m_comfortTemperatureSetPoint = setPoint;
  m_isModified = true;
}

int32_t Settings::getEcoTemperatureSetPoint(void)
{
  return m_ecoTemperatureSetPoint;
}

void Settings::setEcoTemperatureSetPoint(int32_t setPoint)
{
  /* Input validation: Use default value if outside valid range. */
  if ((setPoint != Constants::HEATER_OFF_SET_POINT) && ((setPoint > Constants::MAX_SETP_TEMPERATURE) || (setPoint < Constants::MIN_SETP_TEMPERATURE)))
  {
    setPoint = Constants::DEFAULT_ECO_TEMPERATURE_SET_POINT;
  }

  m_ecoTemperatureSetPoint = setPoint;
  m_isModified = true;
}


void Settings::setSetPoint(int32_t setPoint)
{
  HeatingMode::Enum currentHeatingMode = getHeatingMode();

  switch (currentHeatingMode)
  {
  case HeatingMode::comfort:
    setComfortTemperatureSetPoint(setPoint);
    break;
  case HeatingMode::eco:
    setEcoTemperatureSetPoint(setPoint);
    break;
  case HeatingMode::off:
  case HeatingMode::antiFrost:
    /* Do nothing, we are not supposed to change these temps */
  default:
    break;
  }
}

int32_t Settings::getSetPoint()
{
  int32_t setPoint = 0;
  HeatingMode::Enum currentHeatingMode = getHeatingMode();
  switch (currentHeatingMode)
  {
  case HeatingMode::comfort:
    setPoint = getComfortTemperatureSetPoint();
    break;
  case HeatingMode::eco:
    setPoint = getEcoTemperatureSetPoint();
    break;
  case HeatingMode::antiFrost:
    setPoint = Constants::ANTI_FROST_SET_POINT;
    break;
  case HeatingMode::off:
  default:
    setPoint = INT32_MIN; /* Value should not be used, but need to set a value. */
    break;
  }
  return setPoint;
}

HeatingMode::Enum Settings::getHeatingMode(void)
{
  HeatingMode::Enum currentHeatingMode = m_primaryHeatingMode;
  if (isOverrideActive())
  {
    currentHeatingMode = m_override.getMode();
  }

  return currentHeatingMode;
}


void Settings::setOverride(const Override & override)
{
  m_accessorLock.take();
  m_override = override;
  m_isModified = true;
  m_accessorLock.give();
}

void Settings::getOverride(Override & override)
{
  m_accessorLock.take();
  override = m_override;
  m_accessorLock.give();
}

void Settings::toggleOverride(void)
{
  m_accessorLock.take();

  if (isOverrideActive())
  {
    // was active, deactivate
    m_override = Override(OverrideType::none, HeatingMode::comfort);
  }
  else
  {
    // must activate. If mode now is comfort go to eco, else go to Comfort
    if (m_primaryHeatingMode == HeatingMode::comfort)
    {
      m_override = Override(OverrideType::now, HeatingMode::eco);
    }
    else
    {
      m_override = Override(OverrideType::now, HeatingMode::comfort);
    }
  }

  m_isModified = true;
  m_accessorLock.give();
}

HeatingMode::Enum Settings::getOverrideHeatingMode(void)
{
  return m_override.getMode();
}

bool Settings::isOverrideActive(void)
{
  return m_override.getType() != OverrideType::none;
}

HeatingMode::Enum Settings::getPrimaryHeatingMode(void)
{
  return m_primaryHeatingMode;
}

void Settings::setPrimaryHeatingMode(HeatingMode::Enum heatingMode)
{
  /* Input validation: Use default value if outside valid range. */
  switch (heatingMode)
  {
  case HeatingMode::comfort:
  case HeatingMode::eco:
  case HeatingMode::antiFrost:
  case HeatingMode::off:
    /* All OK values */
    break;
  default:
    /* Invalid value - reset to default: */
    heatingMode = Constants::DEFAULT_HEATING_MODE;
    break;
  }
 
  m_primaryHeatingMode = heatingMode;
  m_isModified = true;
}

void Settings::increaseSetPoint()
{
  if (isSetPointManuallyUserAdjustable())
  {
    HeatingMode::Enum currentHeatingMode;
    currentHeatingMode = getHeatingMode();

    int32_t maxTemp = Constants::MAX_SETP_TEMPERATURE;

    m_accessorLock.take();

    if (currentHeatingMode == static_cast<int16_t>(HeatingMode::comfort))
    {
      m_comfortTemperatureSetPoint += Constants::SETP_TEMPERATURE_STEPS;
      if (m_comfortTemperatureSetPoint > maxTemp)
      {
        m_comfortTemperatureSetPoint = maxTemp;
      }
      m_isModified = true;
    }
    else if (currentHeatingMode == static_cast<int16_t>(HeatingMode::eco))
    {
      if (m_ecoTemperatureSetPoint == Constants::HEATER_OFF_SET_POINT) {
        m_ecoTemperatureSetPoint = Constants::MIN_SETP_TEMPERATURE;
      }
      else {
        m_ecoTemperatureSetPoint += Constants::SETP_TEMPERATURE_STEPS;
        if (m_ecoTemperatureSetPoint > maxTemp)
        {
          m_ecoTemperatureSetPoint = maxTemp;
        }
      }
      m_isModified = true;
    }
    else
    {
      /* Do nothing. */
    }

    m_accessorLock.give();
  }
}


void Settings::decreaseSetPoint()
{
  if (isSetPointManuallyUserAdjustable())
  {
    HeatingMode::Enum currentHeatingMode;
    currentHeatingMode = getHeatingMode();

    m_accessorLock.take();

    if (currentHeatingMode == static_cast<int16_t>(HeatingMode::comfort))
    {
      m_comfortTemperatureSetPoint -= Constants::SETP_TEMPERATURE_STEPS;
      if (m_comfortTemperatureSetPoint < Constants::MIN_SETP_TEMPERATURE)
      {
        m_comfortTemperatureSetPoint = Constants::MIN_SETP_TEMPERATURE;
      }
      m_isModified = true;
    }
    else if (currentHeatingMode == static_cast<int16_t>(HeatingMode::eco))
    {
      m_ecoTemperatureSetPoint -= Constants::SETP_TEMPERATURE_STEPS;
      if (m_ecoTemperatureSetPoint < Constants::MIN_SETP_TEMPERATURE)
      {
        m_ecoTemperatureSetPoint = Constants::HEATER_OFF_SET_POINT;
      }
      m_isModified = true;
    }
    else
    {
      /* Do nothing. */
    }

    m_accessorLock.give();
  }
}


bool Settings::isSetPointManuallyUserAdjustable(void)
{
  bool isAdjustable = false;
  HeatingMode::Enum currentHeatingMode;
  currentHeatingMode = getHeatingMode();
  if ((currentHeatingMode == static_cast<int16_t>(HeatingMode::comfort)) || (currentHeatingMode == static_cast<int16_t>(HeatingMode::eco)))
  {
    isAdjustable = true;
  }
  return isAdjustable;
}

void Settings::setIsButtonLockOn(bool isButtonLockOn)
{
  m_isButtonLockOn = isButtonLockOn;
  m_isModified = true;
}

bool Settings::isButtonLockOn(void)
{
  return m_isButtonLockOn;
}

void Settings::unlockButtonLock(void)
{
  m_isButtonLockOn = false;
  m_isModified = true;
}

void Settings::toggleButtonLock(void)
{
  m_isButtonLockOn = !m_isButtonLockOn;
  m_isModified = true;
}


void Settings::factoryReset(void)
{
  setTimeFromDateTime(Constants::DEFAULT_DATETIME);

  // We want to keep the runMode to what it was, unless users doing a forgetMe ends up an appliance in testMode
  setDefaultSettingsExceptRunMode();
  storeSettings();
}

void Settings::setForgetMeState(ForgetMeState::Enum forgetMeState)
{
  m_forgetMeState = forgetMeState;
  m_isModified = true;
}

ForgetMeState::Enum Settings::getForgetMeState(void)
{
  return m_forgetMeState;
}

void Settings::setTimeFromUnixTime(uint32_t unixTime)
{
  m_accessorLock.take();

  if ((m_lastTimeSet == 0U) || (m_timerDriver.getTimeSinceTimestampMS(m_lastTimeSet) >= static_cast<uint64_t>(Constants::DEFAULT_SET_TIME_LIMITATION_MS)))
  {
    m_systemTime.setUnixTime(unixTime);
    m_lastTimeSet = m_timerDriver.getTimeSinceBootMS();

    DateTime dateTime;
    m_systemTime.getDateTimeFromUnixTime(unixTime, dateTime);
    m_rtc.setCurrentTime(dateTime);
    m_lastTimeRead = 0u;
  }

  m_accessorLock.give();
}


void Settings::setTimeFromDateTime(const DateTime &dateTime)
{
  m_accessorLock.take();

  if ((m_lastTimeSet == 0U) || (m_timerDriver.getTimeSinceTimestampMS(m_lastTimeSet) >= static_cast<uint64_t>(Constants::DEFAULT_SET_TIME_LIMITATION_MS)))
  {
    uint32_t unixTime = m_systemTime.getUnixTimeFromDateTime(dateTime);
    m_systemTime.setUnixTime(unixTime);
    m_lastTimeSet = m_timerDriver.getTimeSinceBootMS();

    m_rtc.setCurrentTime(dateTime);
    m_lastTimeRead = 0u;
  }

  m_accessorLock.give();
}


void Settings::getUnixTime(DateTime &dateTime)
{
  m_accessorLock.take();

  // Check if we can re-use the cached time
  if (m_timerDriver.getTimeSinceTimestampMS(m_lastTimeRead) < Constants::RTC_UPDATE_PERIOD_MS)
  {
    dateTime = m_cachedTime;
  }
  else
  {
    // Or get updated time from system time
    uint32_t now = m_systemTime.getUnixTime();
    m_systemTime.getDateTimeFromUnixTime(now, dateTime);

    m_cachedTime = dateTime;
    m_lastTimeRead = m_timerDriver.getTimeSinceBootMS();
  }

  m_accessorLock.give();
}

void Settings::getLocalTime(DateTime &dateTime)
{
  DateTime nowDateTime;
  getUnixTime(nowDateTime);
  int32_t utcOffset = getUtcOffset();

  uint32_t localTime = m_systemTime.getUnixTimeFromDateTime(nowDateTime);
  localTime += utcOffset;

  m_systemTime.getDateTimeFromUnixTime(localTime, dateTime);
}


void Settings::setUtcOffset(int32_t utcOffsetSeconds)
{
  m_utcOffset = utcOffsetSeconds;
  m_isModified = true;
}


int32_t Settings::getUtcOffset(void)
{
  return m_utcOffset;
}

void Settings::getWeekProgramDataForDay(WeekDay::Enum weekDay, WeekProgramData & weekProgramData)
{
  weekProgramData = m_arrayOfWeekProgramData[static_cast<int16_t>(weekDay)];
}

void Settings::setWeekProgramDataForDay(WeekDay::Enum weekDay, const WeekProgramData & weekProgramData)
{
  if (m_arrayOfWeekProgramData[static_cast<int16_t>(weekDay)] != weekProgramData)
  {
    m_isWeekProgramUpdated = true;
    m_arrayOfWeekProgramData[static_cast<int16_t>(weekDay)] = weekProgramData;
  }

  m_isModified = true;
}

bool Settings::isWeekProgramUpdated()
{
  bool isUpdated = m_isWeekProgramUpdated;
  m_isWeekProgramUpdated = false;
  return isUpdated;
}

void Settings::getWeekProgramExtDataForDay(WeekDay::Enum weekDay,
                                           WeekProgramExtData &weekProgramData)
{
  weekProgramData = m_arrayOfWeekProgramExtData[static_cast<int16_t>(weekDay)];
}

void Settings::setWeekProgramExtDataForDay(WeekDay::Enum weekDay,
                                           const WeekProgramExtData &weekProgramData)
{
  if ( m_arrayOfWeekProgramExtData[static_cast<int16_t>(weekDay)] != weekProgramData )
  {
    m_isWeekProgramUpdated = true;
    m_arrayOfWeekProgramExtData[static_cast<int16_t>(weekDay)] = weekProgramData;
  }

  m_isModified = true;
}

void Settings::resetWeekProgramExtDataForDay(WeekDay::Enum weekDay)
{
  uint8_t invalid[] = {
      0u, 0u, 0u, 0u, Constants::CLOUD_PARAMETER_INVALID_VALUE,
  };

  WeekProgramExtData invalidWeekProgramExt;
  invalidWeekProgramExt.setData(invalid, sizeof(invalid));
  setWeekProgramExtDataForDay(weekDay, invalidWeekProgramExt);
}

uint8_t Settings::getFirmwareUpgradeCount(void)
{
  return m_fwUpgradeAttempts;
}

void Settings::setFirmwareUpgradeCount(uint8_t value)
{
  m_fwUpgradeAttempts = value;
  m_isModified = true;
}

uint32_t Settings::getFirmwareVersionCode(void)
{
  return m_fwVersionCode;
}

void Settings::setFirmwareVersionCode(uint32_t value)
{
  m_fwVersionCode = value;
  m_isModified = true;
}

void Settings::getFirmwareID(char * id, uint32_t size)
{
  m_accessorLock.take();
  strncpy(id, m_firmwareID, size);
  id[size - 1U] = '\0';
  m_accessorLock.give();
}

void Settings::setFirmwareID(const char * id)
{
  m_accessorLock.take();
  strncpy(m_firmwareID, id, sizeof(m_firmwareID));
  m_firmwareID[sizeof(m_firmwareID) - 1U] = '\0';

  m_isModified = true;
  m_accessorLock.give();
}

bool Settings::isModified()
{
  return m_isModified;
}
