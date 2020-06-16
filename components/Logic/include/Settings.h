#pragma once

#include "Mutex.hpp"
#include "SettingsInterface.h"
#include "Constants.h"
#include "SettingsStorageInterface.h"
#include "RtcInterface.h"
#include "SystemTimeDriverInterface.h"
#include "TimerDriverInterface.h"


/** Used by controllers to change global settings throughout the system.
  * Most of the variables here will be read/written to eeprom for consistency.
  *
  */
class Settings : public SettingsInterface
{
public:
  Settings(SettingsStorageInterface & storage, RtcInterface &rtc, SystemTimeDriverInterface &sysTime, TimerDriverInterface &timer);


  void storeSettings(void);
  void retrieveSettings();

  virtual RunMode::Enum getRunMode(void);
  virtual void setRunMode(RunMode::Enum runMode);
  virtual bool isTestMode(void);

  virtual void getWifiSSID(char * ssid, uint32_t size);
  virtual void setWifiSSID(const char * ssid);

  virtual void getWifiPassword(char * password, uint32_t size);
  virtual void setWifiPassword(const char * password);

  virtual void setStaticIp(uint32_t staticIp);
  virtual uint32_t getStaticIp();

  virtual void setStaticNetmask(uint32_t staticNetmask);
  virtual uint32_t getStaticNetmask();

  virtual void setStaticGateway(uint32_t staticGateway);
  virtual uint32_t getStaticGateway();

  virtual void setStaticDns1(uint32_t staticDns1);
  virtual uint32_t getStaticDns1();

  virtual void setStaticDns2(uint32_t staticDns2);
  virtual uint32_t getStaticDns2();

  virtual int32_t getComfortTemperatureSetPoint(void);
  virtual void setComfortTemperatureSetPoint(int32_t setPoint);
  virtual int32_t getEcoTemperatureSetPoint(void);
  virtual void setEcoTemperatureSetPoint(int32_t setPoint);

  virtual void setSetPoint(int32_t setPoint);
  virtual int32_t getSetPoint(void);

  virtual HeatingMode::Enum getHeatingMode(void);
  virtual HeatingMode::Enum getPrimaryHeatingMode(void);
  virtual void setPrimaryHeatingMode(HeatingMode::Enum heatingMode);

  virtual void setOverride(const Override & override);
  virtual void getOverride(Override & override);
  virtual void toggleOverride(void);
  virtual HeatingMode::Enum getOverrideHeatingMode(void);
  virtual bool isOverrideActive(void);

  virtual void increaseSetPoint();
  virtual void decreaseSetPoint();
  virtual void setIsButtonLockOn(bool isButtonLockOn);
  virtual bool isButtonLockOn(void);
  virtual void toggleButtonLock(void);
  virtual void unlockButtonLock(void);

  virtual void factoryReset(void);
  virtual void setForgetMeState(ForgetMeState::Enum forgetMeState);
  virtual ForgetMeState::Enum getForgetMeState(void);

  virtual void setTimeFromUnixTime(uint32_t unixTime);
  virtual void setTimeFromDateTime(const DateTime &dateTime);
  virtual void getUnixTime(DateTime &dateTime);
  virtual void getLocalTime(DateTime &dateTime);

  virtual void setUtcOffset(int32_t utcOffsetSeconds);
  virtual int32_t getUtcOffset(void);

  virtual void getWeekProgramDataForDay(WeekDay::Enum weekDay, WeekProgramData & weekProgramData);
  virtual void setWeekProgramDataForDay(WeekDay::Enum weekDay, const WeekProgramData & weekProgramData);
  virtual bool isWeekProgramUpdated();
  virtual void getWeekProgramExtDataForDay(WeekDay::Enum weekDay, WeekProgramExtData &weekProgramData);
  virtual void setWeekProgramExtDataForDay(WeekDay::Enum weekDay, const WeekProgramExtData &weekProgramData);
  virtual void resetWeekProgramExtDataForDay(WeekDay::Enum weekDay);

  virtual uint8_t getFirmwareUpgradeCount(void);
  virtual void setFirmwareUpgradeCount(uint8_t value);

  virtual uint32_t getFirmwareVersionCode(void);
  virtual void setFirmwareVersionCode(uint32_t value);

  virtual void getFirmwareID(char * id, uint32_t size);
  virtual void setFirmwareID(const char * id);

  bool isModified();

private:
  SettingsStorageInterface &m_storage;
  RtcInterface &m_rtc;
  SystemTimeDriverInterface &m_systemTime;
  TimerDriverInterface &m_timerDriver;

  Mutex m_accessorLock;
  Mutex m_storeRetrieveLock;

  RunMode::Enum m_runMode;
  char m_wifiSSID[Constants::WIFI_SSID_SIZE_MAX];
  char m_wifiPassword[Constants::WIFI_PASSWORD_SIZE_MAX];

  uint32_t m_staticIp;
  uint32_t m_staticNetmask;
  uint32_t m_staticGateway;
  uint32_t m_staticDns1;
  uint32_t m_staticDns2;

  int32_t m_comfortTemperatureSetPoint;
  int32_t m_ecoTemperatureSetPoint;
  HeatingMode::Enum m_primaryHeatingMode;
  Override m_override;

  bool m_isButtonLockOn;
  bool m_isWeekProgramUpdated;
  WeekProgramData m_arrayOfWeekProgramData[NUM_OF_WEEKDAYS];
  WeekProgramExtData m_arrayOfWeekProgramExtData[NUM_OF_WEEKDAYS];
  uint8_t m_fwUpgradeAttempts;
  uint32_t m_fwVersionCode;
  char m_firmwareID[Constants::FW_DOWNLOAD_ID_MAX_LENGTH];
  
  bool isSetPointManuallyUserAdjustable(void);
  void setDefaultSettings();
  void setDefaultSettingsExceptRunMode();
  int32_t m_utcOffset;

  uint8_t m_numOfWeekProgramNodes;
  bool m_isModified;
  uint64_t m_lastTimeRead;
  uint64_t m_lastTimeSet;
  DateTime m_cachedTime;
  bool m_isSystemTimeSet;

  ForgetMeState::Enum m_forgetMeState;
};
