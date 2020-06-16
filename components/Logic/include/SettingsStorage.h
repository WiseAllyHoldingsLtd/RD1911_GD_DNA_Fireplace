#pragma once

#include "SettingsStorageInterface.h"
#include "PersistentStorageDriverInterface.h"


class SettingsStorage : public SettingsStorageInterface
{
public:
  SettingsStorage(PersistentStorageDriverInterface &driver);

  virtual bool retrieveRunMode(RunMode::Enum & runMode) const;
  virtual bool storeRunMode(RunMode::Enum runMode);

  virtual bool retrieveWifiSSID(char * ssid, uint32_t size) const;
  virtual bool storeWifiSSID(const char * ssid);

  virtual bool retrieveWifiPassword(char * password, uint32_t size) const;
  virtual bool storeWifiPassword(const char * password);

  virtual bool retrieveStaticIp(uint32_t &staticIp) const;
  virtual bool storeStaticIp(uint32_t staticIp);

  virtual bool retrieveStaticNetmask(uint32_t &staticNetmask) const;
  virtual bool storeStaticNetmask(uint32_t staticNetmask);

  virtual bool retrieveStaticGateway(uint32_t &staticGateway) const;
  virtual bool storeStaticGateway(uint32_t staticGateway);

  virtual bool retrieveStaticDns1(uint32_t &staticDns1) const;
  virtual bool storeStaticDns1(uint32_t staticDns1);

  virtual bool retrieveStaticDns2(uint32_t &staticDns2) const;
  virtual bool storeStaticDns2(uint32_t staticDns2);

  virtual bool retrieveUtcOffset(int32_t & utcOffsetSeconds) const;
  virtual bool storeUtcOffset(int32_t utcOffsetSeconds);

  virtual bool retrieveIsTimeSet(bool &isTimeSet) const;
  virtual bool storeIsTimeSet(bool isTimeSet);

  virtual bool retrievePrimaryHeatingMode(HeatingMode::Enum & heatingMode) const;
  virtual bool storePrimaryHeatingMode(HeatingMode::Enum heatingMode);

  virtual bool retrieveComfortTemperatureSetPoint(int32_t & setPoint) const;
  virtual bool storeComfortTemperatureSetPoint(int32_t setPoint);

  virtual bool retrieveEcoTemperatureSetPoint(int32_t & setPoint) const;
  virtual bool storeEcoTemperatureSetPoint(int32_t setPoint);

  virtual bool retrieveOverride(Override & override) const;
  virtual bool storeOverride(const Override & override);

  virtual bool retrieveIsButtonLockOn(bool &lockEnabled) const;
  virtual bool storeIsButtonLockOn(bool lockEnabled);

  virtual bool retrieveWeekProgramDataForDay(WeekDay::Enum weekDay, uint8_t (&slots)[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY]) const;
  virtual bool storeWeekProgramDataForDay(WeekDay::Enum weekDay, uint8_t (&slots)[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY]);

  virtual bool retrieveWeekProgramExtDataForDay(WeekDay::Enum weekDay,
                                                uint8_t *slotData,
                                                uint32_t &slotDataSize) const;
  virtual bool storeWeekProgramExtDataForDay(WeekDay::Enum weekDay,
                                             const uint8_t *slotData,
                                             uint8_t slotDataSize);

  virtual bool retrieveFirmwareUpgradeAttempts(uint8_t &attempts) const;
  virtual bool storeFirmwareUpgradeAttempts(uint8_t attempts);

  virtual bool retrieveFirmwareID(char * firmwareID, uint32_t size) const;
  virtual bool storeFirmwareID(const char * firmwareID);

  virtual bool retrieveFirmwareVersionCode(uint32_t &code) const;
  virtual bool storeFirmwareVersionCode(uint32_t code);

  virtual bool retrieveForgetMeState(ForgetMeState::Enum &forgetMeState) const;
  virtual bool storeForgetMeState(ForgetMeState::Enum forgetMeState);

private:
  PersistentStorageDriverInterface & m_driver;
};
