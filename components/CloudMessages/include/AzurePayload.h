#pragma once
#include <cstdint>
#include "SettingsInterface.h"
#include "DeviceMetaInterface.h"
#include "AzureCloudSyncControllerInterface.h"
#include "FirmwareUpgradeControllerInterface.h"
#include "FrameParser.h"
#include "FrameBuilderUser.h"
#include "TransferFrameResponse.h"
#include "TimeSyncInfo.h"
#include "FirmwareUpdateReady.h"
#include "MultipleFrameRequest.h"
#include "OperationInfo.h"
#include "ParameterDataResponse.h"
#include "SystemTimeDriverInterface.h"


class AzurePayload
{
public:
  AzurePayload(SettingsInterface &settings, DeviceMetaInterface &meta, AzureCloudSyncControllerInterface &syncController,
                FirmwareUpgradeControllerInterface &fwUpdateController, SystemTimeDriverInterface &sysTime);

  bool parseTransferFrame(const uint8_t *jsonBytes, uint32_t jsonSize, uint32_t &outputTimeout, FrameParser &outputFrameReader) const;
  bool createTransferFrame(const FrameBuilderUser &frame, TransferFrameResponse &outputResponseFrame) const;

  bool processTimeSyncInfo(const TimeSyncInfo &timeInfo);
  bool processFirmwareUpdateRequest(const FirmwareUpdateReady &request);
  bool processMultipleFrame(const MultipleFrameRequest &request, TransferFrameResponse &outputResponseFrame);
  bool processOperationInfo(const OperationInfo &request);


private:
  uint8_t processApplianceParamsRequest(const MultipleFrameRequest &request, uint8_t applianceParamIndex, ParameterDataResponse &outputParamData);


  SettingsInterface &m_settings;
  DeviceMetaInterface &m_deviceMeta;
  AzureCloudSyncControllerInterface &m_syncController;
  FirmwareUpgradeControllerInterface &m_fwUpgradeController;
  SystemTimeDriverInterface &m_sysTime;
};
