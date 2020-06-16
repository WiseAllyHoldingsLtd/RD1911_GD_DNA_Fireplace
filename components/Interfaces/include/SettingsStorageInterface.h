#pragma once

#include <stdint.h>
#include "RunMode.h"
#include "HeatingMode.h"
#include "DateTime.h"
#include "Override.h"
#include "Constants.h"
#include "ForgetMeState.h"


/* Unique keys used for storing settings in NonVolatile storage */
class SettingUID
{
public:
  static const char RUN_MODE[];
  static const char WIFI_SSID[];
  static const char WIFI_PASS[];
  static const char STATIC_IP[];
  static const char STATIC_NETMASK[];
  static const char STATIC_GATEWAY[];
  static const char STATIC_DNS1[];
  static const char STATIC_DNS2[];
  static const char PRIMARY_HEATING_MODE[];
  static const char COMFORT_TEMP[];
  static const char ECO_TEMP[];
  static const char OVERRIDE_TYPE_MODE[];
  static const char OVERRIDE_DATE[];
  static const char OVERRIDE_TIME[];
  static const char IS_BUTTON_LOCK_ON[];
  static const char UTC_OFFSET[];
  static const char IS_TIME_SET[];
  static const char WEEK_PROGRAM_MONDAY[];
  static const char WEEK_PROGRAM_TUESDAY[];
  static const char WEEK_PROGRAM_WEDNESDAY[];
  static const char WEEK_PROGRAM_THURSDAY[];
  static const char WEEK_PROGRAM_FRIDAY[];
  static const char WEEK_PROGRAM_SATURDAY[];
  static const char WEEK_PROGRAM_SUNDAY[];
  static const char FW_ATTEMPTS[];
  static const char FW_ID[];
  static const char FW_VERSION_CODE[];
  static const char FORGET_ME_STATE[];
  static const char WEEK_PROGRAM_EXT_MONDAY[];
  static const char WEEK_PROGRAM_EXT_TUESDAY[];
  static const char WEEK_PROGRAM_EXT_WEDNESDAY[];
  static const char WEEK_PROGRAM_EXT_THURSDAY[];
  static const char WEEK_PROGRAM_EXT_FRIDAY[];
  static const char WEEK_PROGRAM_EXT_SATURDAY[];
  static const char WEEK_PROGRAM_EXT_SUNDAY[];
};


class SettingsStorageInterface
{
public:
  virtual ~SettingsStorageInterface(void) {}

  virtual bool retrieveRunMode(RunMode::Enum & runMode) const = 0;
  virtual bool storeRunMode(RunMode::Enum runMode) = 0;

  virtual bool retrieveWifiSSID(char * ssid, uint32_t size) const = 0;
  virtual bool storeWifiSSID(const char * ssid) = 0;

  virtual bool retrieveWifiPassword(char * password, uint32_t size) const = 0;
  virtual bool storeWifiPassword(const char * password) = 0;

  virtual bool retrieveStaticIp(uint32_t &staticIp) const = 0;
  virtual bool storeStaticIp(uint32_t staticIp) = 0;

  virtual bool retrieveStaticNetmask(uint32_t &staticNetmask) const = 0;
  virtual bool storeStaticNetmask(uint32_t staticNetmask) = 0;

  virtual bool retrieveStaticGateway(uint32_t &staticGateway) const = 0;
  virtual bool storeStaticGateway(uint32_t staticGateway) = 0;

  virtual bool retrieveStaticDns1(uint32_t &staticDns1) const = 0;
  virtual bool storeStaticDns1(uint32_t staticDns1) = 0;

  virtual bool retrieveStaticDns2(uint32_t &staticDns2) const = 0;
  virtual bool storeStaticDns2(uint32_t staticDns2) = 0;

  virtual bool retrieveUtcOffset(int32_t & utcOffsetSeconds) const = 0;
  virtual bool storeUtcOffset(int32_t utcOffsetSeconds) = 0;

  virtual bool retrieveIsTimeSet(bool &isTimeSet) const = 0;
  virtual bool storeIsTimeSet(bool isTimeSet) = 0;

  virtual bool retrievePrimaryHeatingMode(HeatingMode::Enum &heatingMode) const = 0;
  virtual bool storePrimaryHeatingMode(HeatingMode::Enum heatingMode) = 0;

  virtual bool retrieveComfortTemperatureSetPoint(int32_t &setPoint) const = 0;
  virtual bool storeComfortTemperatureSetPoint(int32_t setPoint) = 0;

  virtual bool retrieveEcoTemperatureSetPoint(int32_t &setPoint) const = 0;
  virtual bool storeEcoTemperatureSetPoint(int32_t setPoint) = 0;

  virtual bool retrieveOverride(Override & override) const = 0;
  virtual bool storeOverride(const Override & override) = 0;

  virtual bool retrieveIsButtonLockOn(bool &lockEnabled) const = 0;
  virtual bool storeIsButtonLockOn(bool lockEnabled) = 0;

  virtual bool retrieveWeekProgramDataForDay(WeekDay::Enum weekDay, uint8_t (&slots)[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY]) const = 0;
  virtual bool storeWeekProgramDataForDay(WeekDay::Enum weekDay, uint8_t (&slots)[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY]) = 0;

  virtual bool retrieveWeekProgramExtDataForDay(WeekDay::Enum weekDay,
                                                uint8_t *slotData,
                                                uint32_t &slotDataSize) const = 0;
  virtual bool storeWeekProgramExtDataForDay(WeekDay::Enum weekDay,
                                             const uint8_t *slotData,
                                             uint8_t slotDataSize) = 0;

  virtual bool retrieveFirmwareUpgradeAttempts(uint8_t &attempts) const = 0;
  virtual bool storeFirmwareUpgradeAttempts(uint8_t attempts) = 0;

  virtual bool retrieveFirmwareID(char * firmwareID, uint32_t size) const = 0;
  virtual bool storeFirmwareID(const char * firmwareID) = 0;

  virtual bool retrieveFirmwareVersionCode(uint32_t &code) const = 0;
  virtual bool storeFirmwareVersionCode(uint32_t code) = 0;

  virtual bool retrieveForgetMeState(ForgetMeState::Enum &forgetMeState) const = 0;
  virtual bool storeForgetMeState(ForgetMeState::Enum forgetMeState) = 0;
};
