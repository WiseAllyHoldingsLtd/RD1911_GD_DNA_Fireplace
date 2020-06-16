#pragma once

#include <cstring>
#include "CppUTestExt\MockSupport.h"
#include "SettingsInterface.h"
#include "WeekProgramDataComparatorAndCopier.h"
#include "OverrideComparatorAndCopier.h"


class SettingsMock : public SettingsInterface
{
public:
  virtual RunMode::Enum getRunMode(void)
  {
    return static_cast<RunMode::Enum>(mock().actualCall("getRunMode").returnIntValue());
  }

  virtual void setRunMode(RunMode::Enum runMode)
  {
    mock().actualCall("setRunMode").withIntParameter("runMode", runMode);
  }

  virtual bool isTestMode(void)
  {
    return mock().actualCall("isTestMode").returnBoolValue();
  }

  virtual void getWifiSSID(char * ssid, uint32_t size)
  {
    mock().actualCall("getWifiSSID")
        .withOutputParameter("ssid", static_cast<void*>(ssid))
        .withUnsignedIntParameter("size", size);
  }

  virtual void setWifiSSID(const char * ssid)
  {
    mock().actualCall("setWifiSSID").withStringParameter("ssid", ssid);
  }

  virtual void getWifiPassword(char * password, uint32_t size)
  {
    mock().actualCall("getWifiPassword")
        .withOutputParameter("password", static_cast<void*>(password))
        .withUnsignedIntParameter("size", size);
  }

  virtual void setWifiPassword(const char * password)
  {
    mock().actualCall("setWifiPassword").withStringParameter("password", password);
  }

  virtual void setStaticIp(uint32_t staticIp)
  {
    mock().actualCall("setStaticIp")
            .withUnsignedIntParameter("staticIp", staticIp);
  }

  virtual uint32_t getStaticIp()
  {
    return mock().actualCall("getStaticIp")
        .returnUnsignedIntValueOrDefault(0u);
  }

  void setStaticNetmask(uint32_t staticNetmask)
  {
    mock().actualCall("setStaticNetmask")
        .withUnsignedIntParameter("staticNetmask", staticNetmask);
  }

  uint32_t getStaticNetmask()
  {
    return mock().actualCall("getStaticNetmask")
        .returnUnsignedIntValueOrDefault(0u);
  }

  void setStaticGateway(uint32_t staticGateway)
  {
    mock().actualCall("setStaticGateway")
        .withUnsignedIntParameter("staticGateway", staticGateway);
  }

  uint32_t getStaticGateway()
  {
    return mock().actualCall("getStaticGateway")
        .returnUnsignedIntValueOrDefault(0u);
  }

  void setStaticDns1(uint32_t staticDns1)
  {
    mock().actualCall("setStaticDns1")
        .withUnsignedIntParameter("staticDns1", staticDns1);
  }

  uint32_t getStaticDns1()
  {
    return mock().actualCall("getStaticDns1")
        .returnUnsignedIntValueOrDefault(0u);
  }

  void setStaticDns2(uint32_t staticDns2)
  {
    mock().actualCall("setStaticDns2")
        .withUnsignedIntParameter("staticDns2", staticDns2);
  }

  uint32_t getStaticDns2()
  {
    return mock().actualCall("getStaticDns2")
        .returnUnsignedIntValueOrDefault(0u);
  }

  virtual int32_t getComfortTemperatureSetPoint(void)
  {
    return mock().actualCall("getComfortTemperatureSetPoint").returnLongIntValue();
  }

  virtual void setComfortTemperatureSetPoint(int32_t setPoint)
  {
    mock().actualCall("setComfortTemperatureSetPoint").withLongIntParameter("setPoint", setPoint);
  }

  virtual int32_t getEcoTemperatureSetPoint(void)
  {
    return mock().actualCall("getEcoTemperatureSetPoint").returnLongIntValue();
  }

  virtual void setEcoTemperatureSetPoint(int32_t setPoint)
  {
    mock().actualCall("setEcoTemperatureSetPoint").withLongIntParameter("setPoint", setPoint);
  }

  virtual int32_t getSetPoint()
  {
    return mock().actualCall("getSetPoint").returnLongIntValue();
  }

  virtual void setSetPoint(int32_t setPoint)
  {
    mock().actualCall("setSetPoint").withIntParameter("setPoint", setPoint);
  }

  virtual HeatingMode::Enum getHeatingMode(void)
  {
    return static_cast<HeatingMode::Enum>(mock().actualCall("getHeatingMode").returnIntValue());
  }

  virtual HeatingMode::Enum getPrimaryHeatingMode(void)
  {
    return static_cast<HeatingMode::Enum>(mock().actualCall("getPrimaryHeatingMode").returnIntValue());
  }

  virtual void setOverride(const Override & override)
  {
    mock().actualCall("setOverride").withParameterOfType("Override", "override", &override);
  }

  virtual void getOverride(Override & override)
  {
    mock().actualCall("getOverride").withOutputParameterOfType("Override", "override", &override);
  }

  virtual void toggleOverride(void)
  {
    mock().actualCall("toggleOverride");
  }

  virtual HeatingMode::Enum getOverrideHeatingMode(void)
  {
    return static_cast<HeatingMode::Enum>(mock().actualCall("getOverrideHeatingMode").returnIntValue());
  }

  virtual bool isOverrideActive(void)
  {
    return mock().actualCall("isOverrideActive").returnBoolValue();
  }

  virtual void setPrimaryHeatingMode(HeatingMode::Enum heatingMode)
  {
    mock().actualCall("setPrimaryHeatingMode").withIntParameter("heatingMode", heatingMode);
  }

  virtual void increaseSetPoint()
  {
    mock().actualCall("increaseSetPoint");
  }

  virtual void decreaseSetPoint()
  {
    mock().actualCall("decreaseSetPoint");
  }

  virtual void unlockButtonLock(void)
  {
    mock().actualCall("unlockButtonLock");
  }

  virtual void toggleButtonLock(void)
  {
    mock().actualCall("toggleButtonLock");
  }

  virtual bool isButtonLockOn(void)
  {
    return mock().actualCall("isButtonLockOn").returnBoolValue();
  }

  virtual void setIsButtonLockOn(bool isButtonLockOn)
  {
    mock().actualCall("setIsButtonLockOn").withIntParameter("isButtonLockOn", isButtonLockOn);
  }

  virtual void factoryReset(void)
  {
    mock().actualCall("factoryResetS");
  }

  virtual void setStandbyMode(bool isStandbyMode)
  {
    mock().actualCall("setStandbyMode").withParameter("isStandbyMode", isStandbyMode);
  }

  virtual bool isStandbyMode(void)
  {
    return mock().actualCall("isStandbyMode").returnBoolValue();
  }

  virtual void getWeekProgramDataForDay(WeekDay::Enum weekDay, WeekProgramData & weekProgramData)
  {
    mock().actualCall("getWeekProgramDataForDay")
        .withIntParameter("weekDay", weekDay)
        .withOutputParameterOfType("WeekProgramData", "weekProgramData", &weekProgramData);
  }

  virtual void setWeekProgramDataForDay(WeekDay::Enum weekDay, const WeekProgramData & weekProgramData)
  {
    mock().actualCall("setWeekProgramDataForDay")
        .withIntParameter("weekDay", weekDay)
        .withParameterOfType("WeekProgramData", "weekProgramData", &weekProgramData);
  }

  virtual bool isWeekProgramUpdated()
  {
    return mock().actualCall("isWeekProgramUpdated").returnBoolValue();
  }

  virtual void getWeekProgramExtDataForDay(WeekDay::Enum weekDay, WeekProgramExtData &weekProgramData)
  {
    mock().actualCall("getWeekProgramExtDataForDay")
        .withIntParameter("weekDay", weekDay)
        .withOutputParameterOfType("WeekProgramExtData", "weekProgramData", &weekProgramData);
  }

  virtual void setWeekProgramExtDataForDay(WeekDay::Enum weekDay, const WeekProgramExtData &weekProgramData)
  {
    mock().actualCall("setWeekProgramDataForDay")
            .withIntParameter("weekDay", weekDay)
            .withParameterOfType("WeekProgramExtData", "weekProgramData", &weekProgramData);
  }

  virtual void resetWeekProgramExtDataForDay(WeekDay::Enum weekDay)
  {
    mock().actualCall("resetWeekProgramExtDataForDay")
        .withIntParameter("weekDay", weekDay);
  }

  virtual void setTimeFromUnixTime(uint32_t unixTime)
  {
    mock().actualCall("setTimeFromUnixTime").withUnsignedIntParameter("unixTime", unixTime);
  }

  virtual void setTimeFromDateTime(const DateTime &dateTime)
  {
    mock().actualCall("setTimeFromDateTime").withParameterOfType("DateTime", "dateTime", (void*)&dateTime);
  }

  virtual void getUnixTime(DateTime &dateTime)
  {
    mock().actualCall("getUnixTime").withOutputParameterOfType("DateTime", "dateTime", (void*)&dateTime);
  }

  virtual void getLocalTime(DateTime &dateTime)
  {
    mock().actualCall("getLocalTime").withOutputParameterOfType("DateTime", "dateTime", (void*)&dateTime);
  }

  virtual void setUtcOffset(int32_t utcOffsetSeconds)
  {
    mock().actualCall("setUtcOffset").withIntParameter("utcOffsetSeconds", utcOffsetSeconds);
  }

  virtual int32_t getUtcOffset(void)
  {
    return mock().actualCall("getUtcOffset").returnIntValue();
  }

  virtual void setNumOfWeekProgramNodes(uint8_t numOfNode)
  {
    mock().actualCall("setNumOfWeekProgramNodes");
  }

  virtual uint8_t getNumOfWeekProgramNodes(void)
  {
    return mock().actualCall("getNumOfWeekProgramNodes").returnIntValue();
  }

  virtual uint8_t getFirmwareUpgradeCount(void)
  {
    return mock().actualCall("getFirmwareUpgradeCount").returnUnsignedIntValue();
  }

  virtual void setFirmwareUpgradeCount(uint8_t value)
  {
    mock().actualCall("setFirmwareUpgradeCount").withUnsignedIntParameter("value", value);
  }

  virtual uint32_t getFirmwareVersionCode(void)
  {
    return mock().actualCall("getFirmwareVersionCode").returnUnsignedIntValue();
  }

  virtual void setFirmwareVersionCode(uint32_t value)
  {
    mock().actualCall("setFirmwareVersionCode").withUnsignedIntParameter("value", value);
  }

  virtual void getFirmwareID(char * id, uint32_t size)
  {
    mock().actualCall("getFirmwareID")
        .withOutputParameter("id", static_cast<void*>(id))
        .withUnsignedIntParameter("size", size);
  }

  virtual void setFirmwareID(const char * id)
  {
    mock().actualCall("setFirmwareID").withStringParameter("id", id);
  }

  virtual ForgetMeState::Enum getForgetMeState()
  {
    return static_cast<ForgetMeState::Enum>(mock()
        .actualCall("getForgetMeState")
        .returnUnsignedIntValue());
  }

  virtual void setForgetMeState(ForgetMeState::Enum forgetMeState)
  {
    mock().actualCall("setForgetMeState")
        .withUnsignedIntParameter("forgetMeState", static_cast<ForgetMeState::Enum>(forgetMeState));
  }
};
