#include <cstring>
#include "EspCpp.hpp"
#include "Base64Converter.h"
#include "TransferFrameRequest.h"
#include "ParameterDataRequest.h"
#include "ParameterDataPayload.h"
#include "MultipleFrameResponse.h"
#include "AzurePayload.h"


namespace
{
  const char LOG_TAG[] = "AzurePayload";
}

AzurePayload::AzurePayload(SettingsInterface &settings, DeviceMetaInterface &meta, AzureCloudSyncControllerInterface &syncController,
                            FirmwareUpgradeControllerInterface &fwUpdateController, SystemTimeDriverInterface &sysTime)
  : m_settings(settings), m_deviceMeta(meta), m_syncController(syncController),
    m_fwUpgradeController(fwUpdateController), m_sysTime(sysTime)
{
}


bool AzurePayload::parseTransferFrame(const uint8_t *jsonBytes, uint32_t jsonSize, uint32_t &outputTimeout, FrameParser &outputFrameReader) const
{
  bool wasParsed = false;

  if ((jsonBytes != nullptr) && (jsonSize > 0U))
  {
    char * payloadStr = new char[jsonSize + 1U];
    memcpy(payloadStr, jsonBytes, jsonSize);
    payloadStr[jsonSize] = '\0';

    TransferFrameRequest request;
    wasParsed = request.parseJSONString(payloadStr);
    delete [] payloadStr;

    if (wasParsed)
    {
      outputTimeout = request.getTimeoutMS();

      Base64 base64;
      uint32_t requiredSize = base64.getDecodedSize(request.getFrame());

      if (requiredSize > 0U)
      {
        uint8_t * decodedBytes = new uint8_t[requiredSize];

        if (base64.decode(request.getFrame(), decodedBytes, &requiredSize))
        {
          outputFrameReader = FrameParser(decodedBytes, requiredSize);
          wasParsed = true;
        }
        else
        {
          ESP_LOGW(LOG_TAG, "Parse failed - base64-decode failed.");
        }

        delete [] decodedBytes;
      }
      else
      {
        ESP_LOGW(LOG_TAG, "Parse failed - base64-decode failed (invalid size).");
      }
    }
    else
    {
      ESP_LOGW(LOG_TAG, "Parse failed - invalid JSON");
    }
  }
  else
  {
    ESP_LOGW(LOG_TAG, "Parse failed - no input data.");
  }

  return wasParsed;
}


bool AzurePayload::createTransferFrame(const FrameBuilderUser &frame, TransferFrameResponse &outputResponseFrame) const
{
  bool wasCreated = false;
  uint32_t frameSize = frame.getSize();

  if (frameSize > 0U)
  {
    uint8_t *frameBytes = new uint8_t[frameSize];

    if (frame.getBytes(frameBytes, frameSize) == frameSize)
    {
      Base64 base64;
      uint32_t base64Size = base64.getEncodedSize(frameBytes, frameSize);

      if (base64Size > 0U)
      {
        char *base64Bytes = new char[base64Size];

        if (base64.encode(frameBytes, frameSize, base64Bytes, base64Size))
        {
          outputResponseFrame.setFrame(base64Bytes);
          wasCreated = true;
        }
        else
        {
          ESP_LOGW(LOG_TAG, "Create failed - base64-encode failed.");
        }

        delete [] base64Bytes;
      }
      else
      {
        ESP_LOGW(LOG_TAG, "Create failed - base64-encode failed (invalid size).");
      }
    }
    else
    {
      ESP_LOGW(LOG_TAG, "Create failed - read input data failed.");
    }

    delete [] frameBytes;
  }
  else
  {
    ESP_LOGW(LOG_TAG, "Create failed - no input data.");
  }

  return wasCreated;
}


bool AzurePayload::processTimeSyncInfo(const TimeSyncInfo &timeInfo)
{
  bool wasProcessed = false;

  if (timeInfo.isValid())
  {
    uint32_t unixTime = timeInfo.getUnixTime();
    int32_t utcOffset = timeInfo.getUtcOffset();

    m_settings.setTimeFromUnixTime(unixTime);
    ESP_LOGI(LOG_TAG, "SetTime called: %u.", unixTime);

    if (m_settings.getUtcOffset() != utcOffset)
    {
      m_settings.setUtcOffset(utcOffset);
      ESP_LOGI(LOG_TAG, "SetUTCOffset: (%i).", utcOffset);
    }

    wasProcessed = true;
  }

  return wasProcessed;
}


bool AzurePayload::processFirmwareUpdateRequest(const FirmwareUpdateReady &request)
{
  bool wasProcessed = false;

  if (request.isValid())
  {
    uint16_t urlLengthWithZeroTerm = request.getUrlLength() + 1U;

    if ((urlLengthWithZeroTerm > 1U) && (urlLengthWithZeroTerm <= Constants::FW_DOWNLOAD_URL_MAX_LENGTH))
    {
      char urlBuffer[Constants::FW_DOWNLOAD_URL_MAX_LENGTH];
      request.getUrlString(urlBuffer, urlLengthWithZeroTerm);
      ESP_LOGI(LOG_TAG, "FirmwareURL: %s", urlBuffer);

      m_fwUpgradeController.setFirmwareUrl(urlBuffer);
      bool wasSignalSent = m_fwUpgradeController.signalFirmwareUpgradeRequest();

      if (wasSignalSent)
      {
        ESP_LOGI(LOG_TAG, "FirmwareUpdate 'start' signal has been sent.");
      }
      else
      {
        ESP_LOGW(LOG_TAG, "FirmwareUpdate request ignored - probably failed too many times.");
      }

      wasProcessed = true;
    }
    else
    {
      ESP_LOGE(LOG_TAG, "FirmwareUpdate request - Invalid URL length (%u).", urlLengthWithZeroTerm - 1U);
    }
  }

  return wasProcessed;
}


bool AzurePayload::processMultipleFrame(const MultipleFrameRequest &request, TransferFrameResponse &outputResponseFrame)
{
  /*
   * Recap:
   *   MultipleFrameRequest is a single MULTIPLE_FRAME_REQUEST which contains a number of APPLIANCE_PARAMS_REQUEST frames (typically just one).
   *   The APPLIANCE_PARAMS_REQUEST frames in turn contains a number of ParameterDataRequest "frames".
   *   Most of the ParameterDataRequest frames are commands, i.e. we receive them and store data. Some requires us to answer. For each APPLIANCE_PARAMS_REQUEST
   *   that contains at least one ParameterDataRequest which requires answer, we create an APPLIANCE_PARAMS_RESPONSE in which we put the
   *   ParameterDataResponse.
   */
  uint16_t wasResponseCreated = false;

  if (request.isValid())
  {
    uint16_t numOfFrames = request.getNumOfPayloadFrames();

    if (numOfFrames > 0U)
    {
      MultipleFrameResponse response;

      for (uint16_t i = 0U; i < numOfFrames; ++i)
      {
        ParameterDataResponse paramData;
        uint8_t numOfResponseItems = processApplianceParamsRequest(request, i, paramData);

        if (numOfResponseItems > 0U)
        {
          bool wasAdded = response.addParameterDataResponse(paramData);

          if (wasAdded)
          {
            ESP_LOGI(LOG_TAG, "Added ApplianceParamResponse with %u payload items to MultipleFrameResponse.", numOfResponseItems);
          }
          else
          {
            ESP_LOGI(LOG_TAG, "Failed to add ApplianceParamResponse with %u payload items to MultipleFrameResponse.", numOfResponseItems);
          }
        }
      }

      // Check to see if we have returned any response Frames - if yes, fill TransferFrameResponse.
      if (response.getNumberOfFrames() > 0U)
      {
        wasResponseCreated = createTransferFrame(response, outputResponseFrame);
      }
      else
      {
        ESP_LOGD(LOG_TAG, "MultipleRequest requires no response.");
        wasResponseCreated = true;
      }
    }
  }

  return wasResponseCreated;
}

bool AzurePayload::processOperationInfo(const OperationInfo &request)
{
  bool wasProcessed = false;

  if (request.getType() == OperationType::forgetMe)
  {
    if (request.getStatus() == OperationStatus::success)
    {
      if ( m_settings.getForgetMeState() == ForgetMeState::resetRequested )
      {
        m_settings.setForgetMeState(ForgetMeState::resetConfirmed);
        ESP_LOGI(LOG_TAG, "OperationInfo ForgetMe::OK' received. Forget me request confirmed.");
      }
      else
      {
        ESP_LOGW(LOG_TAG, "OperationInfo ForgetMe::OK' received. Forget me request confirmed, but not initiated from appliance, ignoring");
      }
    }
    else
    {
      ESP_LOGW(LOG_TAG, "OperationInfo ForgetMe::Failed' received. Forget me request not confirmed, retry scheduled for next reboot.");
    }

    wasProcessed = true;
  }
  else
  {
    ESP_LOGW(LOG_TAG, "OperationInfo specified unsupported operation (%u).", request.getType());
  }

  return wasProcessed;
}

uint8_t AzurePayload::processApplianceParamsRequest(const MultipleFrameRequest &request, uint8_t applianceParamIndex, ParameterDataResponse &outputParamData)
{
  uint8_t numberOfResponseItems = 0U;
  uint8_t frameSize = request.getPayloadFrameSize(applianceParamIndex);

  if (frameSize > 0U)
  {
    uint8_t * frameBytes = new uint8_t[frameSize];
    request.getPayloadFrame(applianceParamIndex, frameBytes, frameSize);
    FrameParser frameReader(frameBytes, frameSize);
    delete [] frameBytes;

    ParameterDataRequest dataRequest(frameReader);

    if (dataRequest.isValid())
    {
      ESP_LOGD(LOG_TAG, "Processing ApplianceParamsRequest with %u ParamData frames in payload.", dataRequest.getNumOfParamDataItems());
      ParameterDataPayload requestProcessor(m_settings, m_deviceMeta, m_syncController, m_sysTime);

      uint8_t returnedItems = requestProcessor.processRequest(dataRequest, outputParamData);
      ESP_LOGD(LOG_TAG, "Processing ApplianceParamsRequest returned %u response item(s).", returnedItems);
      numberOfResponseItems += returnedItems;
    }
    else
    {
      ESP_LOGW(LOG_TAG, "ApplianceParamsRequest ignored. Invalid payload.");
    }
  }
  else
  {
    ESP_LOGW(LOG_TAG, "ApplianceParamsRequest ignored. Size is 0.");
  }

  return numberOfResponseItems;
}
