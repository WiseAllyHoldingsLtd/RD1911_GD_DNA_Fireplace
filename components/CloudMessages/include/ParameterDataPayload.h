#pragma once

#include "SettingsInterface.h"
#include "DeviceMetaInterface.h"
#include "AzureCloudSyncControllerInterface.h"
#include "ParameterDataRequestInterface.h"
#include "ParameterDataResponse.h"
#include "DateTime.h"
#include "SystemTimeDriverInterface.h"
#include "HeatingMode.h"
#include "ParameterID.h"


class ParameterDataPayload
{
public:
  ParameterDataPayload(SettingsInterface &settings, DeviceMetaInterface & meta, AzureCloudSyncControllerInterface &syncController, SystemTimeDriverInterface &sysTime);
  uint8_t processRequest(const ParameterDataRequestInterface & request, ParameterDataResponse & outputResponse);

private:
  uint8_t processDeviceData(const uint8_t * data, uint8_t size, ParameterDataResponse & outputResponse);
  uint8_t processActivateWeekProgram(const uint8_t * data, uint8_t size);
  uint8_t processActivateNowOrConstantOverride(const uint8_t * data, uint8_t size);
  uint8_t processActivateTimedOverride(const uint8_t * data, uint8_t size);
  uint8_t processWeekProgramData(const uint8_t * data, uint8_t size, WeekDay::Enum weekDayOne, WeekDay::Enum weekDayTwo);
  uint8_t processWeekProgramDataExtended(const uint8_t *data, uint8_t size, WeekDay::Enum weekDay);
  uint8_t processTemperatureSetPoint(const uint8_t * data, uint8_t size, HeatingMode::Enum heatingMode, ParameterDataResponse & outputResponse, ParameterID::Enum requestID);
  uint8_t processLockStatus(const uint8_t *data, uint8_t size);
  CloudTimerHeatingMode::Enum getModeEnumFromByte(uint8_t code) const;
  HeatingMode::Enum convertCloudModeToHeatingMode(CloudTimerHeatingMode::Enum cloudMode) const;
  WeekDay::Enum getWeekDayFromCloudByte(uint8_t cloudByte) const;

  SettingsInterface &m_settings;
  DeviceMetaInterface &m_deviceMeta;
  AzureCloudSyncControllerInterface &m_settingsSync;
  SystemTimeDriverInterface &m_sysTime;
};
