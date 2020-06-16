#include "SettingsStorage.h"


SettingsStorage::SettingsStorage(PersistentStorageDriverInterface & driver)
: m_driver(driver)
{
}


bool SettingsStorage::retrieveRunMode(RunMode::Enum & runMode) const
{
  uint32_t data;
  bool result = m_driver.readValue(SettingUID::RUN_MODE, &data);

  if (result)
  {
    runMode = static_cast<RunMode::Enum>(data);
  }

  return result;
}


bool SettingsStorage::storeRunMode(RunMode::Enum runMode)
{
  uint32_t data = static_cast<uint32_t>(runMode);
  return m_driver.writeValue(SettingUID::RUN_MODE, data);
}


bool SettingsStorage::retrieveWifiSSID(char * ssid, uint32_t size) const
{
  return m_driver.readString(SettingUID::WIFI_SSID, ssid, size);
}


bool SettingsStorage::storeWifiSSID(const char * ssid)
{
  return m_driver.writeString(SettingUID::WIFI_SSID, ssid);
}


bool SettingsStorage::retrieveWifiPassword(char * password, uint32_t size) const
{
  return m_driver.readString(SettingUID::WIFI_PASS, password, size);
}


bool SettingsStorage::storeWifiPassword(const char * password)
{
  return m_driver.writeString(SettingUID::WIFI_PASS, password);
}

bool SettingsStorage::retrieveStaticIp(uint32_t &staticIp) const
{
  uint32_t data;
  bool result = m_driver.readValue(SettingUID::STATIC_IP, &data);

  if (result)
  {
    staticIp = data;
  }

  return result;
}

bool SettingsStorage::storeStaticIp(uint32_t staticIp)
{
  return m_driver.writeValue(SettingUID::STATIC_IP, staticIp);
}

bool SettingsStorage::retrieveStaticNetmask(uint32_t &staticNetmask) const
{
  uint32_t data;
  bool result = m_driver.readValue(SettingUID::STATIC_NETMASK, &data);

  if (result)
  {
    staticNetmask = data;
  }

  return result;
}

bool SettingsStorage::storeStaticNetmask(uint32_t staticNetmask)
{
  return m_driver.writeValue(SettingUID::STATIC_NETMASK, staticNetmask);
}

bool SettingsStorage::retrieveStaticGateway(uint32_t &staticGateway) const
{
  uint32_t data;
  bool result = m_driver.readValue(SettingUID::STATIC_GATEWAY, &data);

  if (result)
  {
    staticGateway = data;
  }

  return result;
}

bool SettingsStorage::storeStaticGateway(uint32_t staticGateway)
{
  return m_driver.writeValue(SettingUID::STATIC_GATEWAY, staticGateway);
}

bool SettingsStorage::retrieveStaticDns1(uint32_t &staticDns1) const
{
  uint32_t data;
  bool result = m_driver.readValue(SettingUID::STATIC_DNS1, &data);

  if (result)
  {
    staticDns1 = data;
  }

  return result;
}

bool SettingsStorage::storeStaticDns1(uint32_t staticDns1)
{
  return m_driver.writeValue(SettingUID::STATIC_DNS1, staticDns1);
}

bool SettingsStorage::retrieveStaticDns2(uint32_t &staticDns2) const
{
  uint32_t data;
  bool result = m_driver.readValue(SettingUID::STATIC_DNS2, &data);

  if (result)
  {
    staticDns2 = data;
  }

  return result;
}

bool SettingsStorage::storeStaticDns2(uint32_t staticDns2)
{
  return m_driver.writeValue(SettingUID::STATIC_DNS2, staticDns2);
}

bool SettingsStorage::retrieveUtcOffset(int32_t & utcOffsetSeconds) const
{
  uint32_t data;
  bool result = m_driver.readValue(SettingUID::UTC_OFFSET, &data);

  if (result)
  {
    utcOffsetSeconds = static_cast<int32_t>(data);
  }

  return result;
}


bool SettingsStorage::storeUtcOffset(int32_t utcOffsetSeconds)
{
  uint32_t data = static_cast<uint32_t>(utcOffsetSeconds);
  return m_driver.writeValue(SettingUID::UTC_OFFSET, data);
}


bool SettingsStorage::retrieveIsTimeSet(bool &isTimeSet) const
{
  uint32_t data;
  bool result = m_driver.readValue(SettingUID::IS_TIME_SET, &data);

  if (result)
  {
    isTimeSet = static_cast<bool>(data);
  }

  return result;
}


bool SettingsStorage::storeIsTimeSet(bool isTimeSet)
{
  uint32_t data = static_cast<uint32_t>(isTimeSet);
  return m_driver.writeValue(SettingUID::IS_TIME_SET, data);
}


bool SettingsStorage::retrievePrimaryHeatingMode(HeatingMode::Enum &heatingMode) const
{
  uint32_t data;
  bool result = m_driver.readValue(SettingUID::PRIMARY_HEATING_MODE, &data);

  if (result)
  {
    heatingMode = static_cast<HeatingMode::Enum>(data);
  }

  return result;
}


bool SettingsStorage::storePrimaryHeatingMode(HeatingMode::Enum heatingMode)
{
  uint32_t data = static_cast<uint32_t>(heatingMode);
  return m_driver.writeValue(SettingUID::PRIMARY_HEATING_MODE, data);
}


bool SettingsStorage::retrieveComfortTemperatureSetPoint(int32_t &setPoint) const
{
  uint32_t data;
  bool result = m_driver.readValue(SettingUID::COMFORT_TEMP, &data);

  if (result)
  {
    setPoint = static_cast<int32_t>(data);
  }

  return result;
}


bool SettingsStorage::storeComfortTemperatureSetPoint(int32_t setPoint)
{
  uint32_t data = static_cast<uint32_t>(setPoint);
  return m_driver.writeValue(SettingUID::COMFORT_TEMP, data);
}



bool SettingsStorage::retrieveEcoTemperatureSetPoint(int32_t &setPoint) const
{
  uint32_t data;
  bool result = m_driver.readValue(SettingUID::ECO_TEMP, &data);

  if (result)
  {
    setPoint = static_cast<int32_t>(data);
  }

  return result;
}


bool SettingsStorage::storeEcoTemperatureSetPoint(int32_t setPoint)
{
  uint32_t data = static_cast<uint32_t>(setPoint);
  return m_driver.writeValue(SettingUID::ECO_TEMP, data);
}

bool SettingsStorage::retrieveOverride(Override & override) const
{
  /* See storeOverride(...) for description of storage */
  uint32_t dataTypeMode = 0U;
  uint32_t dataDate = 0U;
  uint32_t dataTime = 0U;

  bool resultOne = m_driver.readValue(SettingUID::OVERRIDE_TYPE_MODE, &dataTypeMode);
  bool resultTwo = m_driver.readValue(SettingUID::OVERRIDE_DATE, &dataDate);
  bool resultThree = m_driver.readValue(SettingUID::OVERRIDE_TIME, &dataTime);

  if (resultOne && resultTwo && resultThree)
  {
    OverrideType::Enum type = static_cast<OverrideType::Enum>((dataTypeMode >> 8U) & 0xff);
    HeatingMode::Enum mode = static_cast<HeatingMode::Enum>(dataTypeMode & 0xff);

    DateTime dateTime;
    dateTime.year = ((dataDate >> 24U) & 0xff);
    dateTime.month = ((dataDate >> 16U) & 0xff);
    dateTime.days = ((dataDate >> 8U) & 0xff);
    dateTime.weekDay = static_cast<WeekDay::Enum>(dataDate & 0xff);

    dateTime.hours = ((dataTime >> 16U) & 0xff);
    dateTime.minutes = ((dataTime >> 8U) & 0xff);;
    dateTime.seconds = (dataTime & 0xff);

    override = Override(type, mode, dateTime);
  }


  return (resultOne && resultTwo && resultThree);
}

bool SettingsStorage::storeOverride(const Override & override)
{
  /* Store data as three uint32s (msb, , , lsb):
   * Bytes in VALUE1: <unused>, <unused>, type, mode
   * Bytes in VALUE2: yy, mm, dd, weekday
   * Bytes in VALUE3: <unused>, hh, mm, ss
   */
  DateTime dateTime;
  override.getEndDateTime(dateTime);

  uint32_t valueTypeMode = static_cast<uint8_t>(override.getType());
  valueTypeMode = ((valueTypeMode << 8U) | (static_cast<uint8_t>(override.getMode())));

  uint32_t valueDate = dateTime.year;
  valueDate = ((valueDate << 8U) | dateTime.month);
  valueDate = ((valueDate << 8U) | dateTime.days);
  valueDate = ((valueDate << 8U) | static_cast<uint8_t>(dateTime.weekDay));

  uint32_t valueTime = dateTime.hours;
  valueTime = ((valueTime << 8U) | dateTime.minutes);
  valueTime = ((valueTime << 8U) | dateTime.seconds);

  return m_driver.writeValue(SettingUID::OVERRIDE_TYPE_MODE, valueTypeMode)
          && m_driver.writeValue(SettingUID::OVERRIDE_DATE, valueDate)
          && m_driver.writeValue(SettingUID::OVERRIDE_TIME, valueTime);
}

bool SettingsStorage::retrieveIsButtonLockOn(bool &lockEnabled) const
{
  uint32_t data;
  bool result = m_driver.readValue(SettingUID::IS_BUTTON_LOCK_ON, &data);

  if (result)
  {
    lockEnabled = static_cast<bool>(data);
  }

  return result;
}


bool SettingsStorage::storeIsButtonLockOn(bool lockEnabled)
{
  uint32_t data = static_cast<uint32_t>(lockEnabled);
  return m_driver.writeValue(SettingUID::IS_BUTTON_LOCK_ON, data);
}


bool SettingsStorage::retrieveWeekProgramDataForDay(WeekDay::Enum weekDay, uint8_t (&slots)[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY]) const
{
  bool result = false;
  uint32_t size = sizeof(slots);

  switch (weekDay)
  {
  case WeekDay::MONDAY:
    result = m_driver.readBlob(SettingUID::WEEK_PROGRAM_MONDAY, slots, size);
    break;
  case WeekDay::TUESDAY:
    result = m_driver.readBlob(SettingUID::WEEK_PROGRAM_TUESDAY, slots, size);
    break;
  case WeekDay::WEDNESDAY:
    result = m_driver.readBlob(SettingUID::WEEK_PROGRAM_WEDNESDAY, slots, size);
    break;
  case WeekDay::THURSDAY:
    result = m_driver.readBlob(SettingUID::WEEK_PROGRAM_THURSDAY, slots, size);
    break;
  case WeekDay::FRIDAY:
    result = m_driver.readBlob(SettingUID::WEEK_PROGRAM_FRIDAY, slots, size);
    break;
  case WeekDay::SATURDAY:
    result = m_driver.readBlob(SettingUID::WEEK_PROGRAM_SATURDAY, slots, size);
    break;
  case WeekDay::SUNDAY:
    result = m_driver.readBlob(SettingUID::WEEK_PROGRAM_SUNDAY, slots, size);
    break;
  default:
    break;
  }

  return (result && (size == sizeof(slots))); /* Size is updated with actual num of bytes retrieved. */
}


bool SettingsStorage::storeWeekProgramDataForDay(WeekDay::Enum weekDay, uint8_t (&slots)[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY])
{
  bool result = false;
  uint32_t size = sizeof(slots);

  switch (weekDay)
  {
  case WeekDay::MONDAY:
    result = m_driver.writeBlob(SettingUID::WEEK_PROGRAM_MONDAY, slots, size);
    break;
  case WeekDay::TUESDAY:
    result = m_driver.writeBlob(SettingUID::WEEK_PROGRAM_TUESDAY, slots, size);
    break;
  case WeekDay::WEDNESDAY:
    result = m_driver.writeBlob(SettingUID::WEEK_PROGRAM_WEDNESDAY, slots, size);
    break;
  case WeekDay::THURSDAY:
    result = m_driver.writeBlob(SettingUID::WEEK_PROGRAM_THURSDAY, slots, size);
    break;
  case WeekDay::FRIDAY:
    result = m_driver.writeBlob(SettingUID::WEEK_PROGRAM_FRIDAY, slots, size);
    break;
  case WeekDay::SATURDAY:
    result = m_driver.writeBlob(SettingUID::WEEK_PROGRAM_SATURDAY, slots, size);
    break;
  case WeekDay::SUNDAY:
    result = m_driver.writeBlob(SettingUID::WEEK_PROGRAM_SUNDAY, slots, size);
    break;
  default:
    break;
  }

  return result;
}

bool SettingsStorage::retrieveWeekProgramExtDataForDay(WeekDay::Enum weekDay,
                                                       uint8_t *slotData,
                                                       uint32_t &slotDataSize) const
{
  bool result = false;

  switch (weekDay)
  {
  case WeekDay::MONDAY:
    result = m_driver.readBlob(SettingUID::WEEK_PROGRAM_EXT_MONDAY, slotData, slotDataSize);
    break;
  case WeekDay::TUESDAY:
    result = m_driver.readBlob(SettingUID::WEEK_PROGRAM_EXT_TUESDAY, slotData, slotDataSize);
    break;
  case WeekDay::WEDNESDAY:
    result = m_driver.readBlob(SettingUID::WEEK_PROGRAM_EXT_WEDNESDAY, slotData, slotDataSize);
    break;
  case WeekDay::THURSDAY:
    result = m_driver.readBlob(SettingUID::WEEK_PROGRAM_EXT_THURSDAY, slotData, slotDataSize);
    break;
  case WeekDay::FRIDAY:
    result = m_driver.readBlob(SettingUID::WEEK_PROGRAM_EXT_FRIDAY, slotData, slotDataSize);
    break;
  case WeekDay::SATURDAY:
    result = m_driver.readBlob(SettingUID::WEEK_PROGRAM_EXT_SATURDAY, slotData, slotDataSize);
    break;
  case WeekDay::SUNDAY:
    result = m_driver.readBlob(SettingUID::WEEK_PROGRAM_EXT_SUNDAY, slotData, slotDataSize);
    break;
  default:
    break;
  }

  return (result);
}

bool SettingsStorage::storeWeekProgramExtDataForDay(WeekDay::Enum weekDay,
                                                    const uint8_t *slotData,
                                                    uint8_t slotDataSize)
{
  bool result = false;

  switch (weekDay)
  {
  case WeekDay::MONDAY:
    result = m_driver.writeBlob(SettingUID::WEEK_PROGRAM_EXT_MONDAY, slotData, slotDataSize);
    break;
  case WeekDay::TUESDAY:
    result = m_driver.writeBlob(SettingUID::WEEK_PROGRAM_EXT_TUESDAY, slotData, slotDataSize);
    break;
  case WeekDay::WEDNESDAY:
    result = m_driver.writeBlob(SettingUID::WEEK_PROGRAM_EXT_WEDNESDAY, slotData, slotDataSize);
    break;
  case WeekDay::THURSDAY:
    result = m_driver.writeBlob(SettingUID::WEEK_PROGRAM_EXT_THURSDAY, slotData, slotDataSize);
    break;
  case WeekDay::FRIDAY:
    result = m_driver.writeBlob(SettingUID::WEEK_PROGRAM_EXT_FRIDAY, slotData, slotDataSize);
    break;
  case WeekDay::SATURDAY:
    result = m_driver.writeBlob(SettingUID::WEEK_PROGRAM_EXT_SATURDAY, slotData, slotDataSize);
    break;
  case WeekDay::SUNDAY:
    result = m_driver.writeBlob(SettingUID::WEEK_PROGRAM_EXT_SUNDAY, slotData, slotDataSize);
    break;
  default:
    break;
  }

  return result;
}


bool SettingsStorage::retrieveFirmwareUpgradeAttempts(uint8_t &attempts) const
{
  uint32_t data;
  bool result = m_driver.readValue(SettingUID::FW_ATTEMPTS, &data);

  if (result)
  {
    attempts = static_cast<uint8_t>(data);
  }

  return result;
}


bool SettingsStorage::storeFirmwareUpgradeAttempts(uint8_t attempts)
{
  uint32_t data = static_cast<uint32_t>(attempts);
  return m_driver.writeValue(SettingUID::FW_ATTEMPTS, data);
}


bool SettingsStorage::retrieveFirmwareID(char * firmwareID, uint32_t size) const
{
  return m_driver.readString(SettingUID::FW_ID, firmwareID, size);
}

bool SettingsStorage::storeFirmwareID(const char * firmwareID)
{
  return m_driver.writeString(SettingUID::FW_ID, firmwareID);
}

bool SettingsStorage::retrieveFirmwareVersionCode(uint32_t &code) const
{
  uint32_t data;
  bool result = m_driver.readValue(SettingUID::FW_VERSION_CODE, &data);

  if (result)
  {
    code = data;
  }

  return result;
}


bool SettingsStorage::storeFirmwareVersionCode(uint32_t code)
{
  return m_driver.writeValue(SettingUID::FW_VERSION_CODE, code);
}

bool SettingsStorage::retrieveForgetMeState(ForgetMeState::Enum &forgetMeState) const
{
  uint32_t data;
  bool result = m_driver.readValue(SettingUID::FORGET_ME_STATE, &data);

  if ( result )
  {
    forgetMeState = static_cast<ForgetMeState::Enum>(data);
  }

  return result;
}

bool SettingsStorage::storeForgetMeState(ForgetMeState::Enum forgetMeState)
{
  return m_driver.writeValue(SettingUID::FORGET_ME_STATE, forgetMeState);
}
