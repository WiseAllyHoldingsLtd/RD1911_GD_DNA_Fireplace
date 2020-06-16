#pragma once

#include <stdint.h>

#include "HeatingMode.h"
#include "DateTime.h"
#include "Override.h"
#include "RunMode.h"
#include "WeekProgramData.h"
#include "WeekProgramExtData.h"
#include "ForgetMeState.h"

class SettingsInterface
{
public:

  virtual ~SettingsInterface(){};

  virtual RunMode::Enum getRunMode(void) = 0;
  virtual void setRunMode(RunMode::Enum runMode) = 0;
  virtual bool isTestMode(void) = 0;

  virtual void getWifiSSID(char * ssid, uint32_t size) = 0;
  virtual void setWifiSSID(const char * ssid) = 0;

  virtual void getWifiPassword(char * password, uint32_t size) = 0;
  virtual void setWifiPassword(const char * password) = 0;

  virtual void setStaticIp(uint32_t staticIp) = 0;
  virtual uint32_t getStaticIp() = 0;

  virtual void setStaticNetmask(uint32_t staticNetmask) = 0;
  virtual uint32_t getStaticNetmask() = 0;

  virtual void setStaticGateway(uint32_t staticGateway) = 0;
  virtual uint32_t getStaticGateway() = 0;

  virtual void setStaticDns1(uint32_t staticDns1) = 0;
  virtual uint32_t getStaticDns1() = 0;

  virtual void setStaticDns2(uint32_t staticDns2) = 0;
  virtual uint32_t getStaticDns2() = 0;

  virtual int32_t getComfortTemperatureSetPoint(void) = 0;
  virtual void setComfortTemperatureSetPoint(int32_t setPoint) = 0;
  virtual int32_t getEcoTemperatureSetPoint(void) = 0;
  virtual void setEcoTemperatureSetPoint(int32_t setPoint) = 0;

  virtual void setSetPoint(int32_t setPoint) = 0;
  virtual int32_t getSetPoint(void) = 0;

  virtual HeatingMode::Enum getHeatingMode(void) = 0; /** The HeatingMode actually displayed on the display to the user */

  virtual void setOverride(const Override & override) = 0;
  virtual void getOverride(Override & override) = 0;
  virtual void toggleOverride(void) = 0;

  virtual HeatingMode::Enum getOverrideHeatingMode(void) = 0;
  virtual bool isOverrideActive(void) = 0;

  virtual HeatingMode::Enum getPrimaryHeatingMode(void) = 0; /** The HeatingMode if you don't account for overrides */
  virtual void setPrimaryHeatingMode(HeatingMode::Enum heatingMode) = 0; /* In addition, there will be OverrideHeatingMode */

  virtual void increaseSetPoint() = 0; /** Increases by one degree (1000) or 10 percent. */
  virtual void decreaseSetPoint() = 0; /** Decreases by one degree (1000) or 10 percent. */
  virtual void setIsButtonLockOn(bool isButtonLockOn) = 0;
  virtual bool isButtonLockOn(void) = 0;
  virtual void unlockButtonLock(void) = 0;
  virtual void toggleButtonLock(void) = 0;

  virtual void factoryReset(void) = 0;
  virtual void setForgetMeState(ForgetMeState::Enum forgetMeState) = 0;
  virtual ForgetMeState::Enum getForgetMeState(void) = 0;

  virtual void setTimeFromUnixTime(uint32_t unixTime) = 0;
  virtual void setTimeFromDateTime(const DateTime &dateTime) = 0;
  virtual void getUnixTime(DateTime &dateTime) = 0;
  virtual void getLocalTime(DateTime &dateTime) = 0;

  virtual void setUtcOffset(int32_t utcOffsetSeconds) = 0;
  virtual int32_t getUtcOffset(void) = 0;

  virtual void getWeekProgramDataForDay(WeekDay::Enum weekDay, WeekProgramData &weekProgramData) = 0;
  virtual void setWeekProgramDataForDay(WeekDay::Enum weekDay, const WeekProgramData &weekProgramData) = 0;
  virtual bool isWeekProgramUpdated() = 0;
  virtual void getWeekProgramExtDataForDay(WeekDay::Enum weekDay, WeekProgramExtData &weekProgramData) = 0;
  virtual void setWeekProgramExtDataForDay(WeekDay::Enum weekDay, const WeekProgramExtData &weekProgramData) = 0;
  virtual void resetWeekProgramExtDataForDay(WeekDay::Enum weekDay) = 0;

  virtual uint8_t getFirmwareUpgradeCount(void) = 0; /* Attempts to install current firmware update */
  virtual void setFirmwareUpgradeCount(uint8_t value) = 0;
  virtual uint32_t getFirmwareVersionCode(void) = 0;
  virtual void setFirmwareVersionCode(uint32_t value) = 0;
  virtual void getFirmwareID(char * id, uint32_t size) = 0;
  virtual void setFirmwareID(const char * id) = 0;

};
