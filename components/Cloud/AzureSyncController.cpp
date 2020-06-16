#include <cstdlib>
#include "EspCpp.hpp"
#include "AzureSyncController.h"
#include "ParameterID.h"
#include "MultipleFrameResponse.h"
#include "FrameType.h"


namespace
{
  const char LOG_TAG[] = "AzureSync";
  const uint8_t NOW_OVERRIDE_CODE = 1U;
  enum ApplianceLockType : uint8_t
  {
    NO_LOCK_ACTIVE = 0,
    CHILD_LOCK = 1,
    PIN_LOCK = 2,
    REMOTE_LOCK = 3
  };
}

AzureSyncController::AzureSyncController(SettingsInterface &settings,
                                         DeviceMetaInterface &deviceMeta,
                                         AsyncMessageQueueWriteInterface &asyncQueue)
  : m_settings(settings), m_deviceMeta(deviceMeta), m_asyncQueue(asyncQueue), m_forgetMeFrameSent(false)
{
  m_cachedComfortSetpoint = m_settings.getComfortTemperatureSetPoint();
  m_cachedEcoSetpoint = m_settings.getEcoTemperatureSetPoint();
  m_settings.getOverride(m_cachedOverride);
  m_cachedButtonLockEnabled = m_settings.isButtonLockOn();
}

void AzureSyncController::updateComfortTemperatureFromCloud(int32_t temperatureFP)
{
  m_settings.setComfortTemperatureSetPoint(temperatureFP);
  m_cachedComfortSetpoint = temperatureFP;
  ESP_LOGI(LOG_TAG, "Comfort temperature sync'ed from cloud, %d degC.", temperatureFP / 1000);
}

void AzureSyncController::updateEcoTemperatureFromCloud(int32_t temperatureFP)
{
  m_settings.setEcoTemperatureSetPoint(temperatureFP);
  m_cachedEcoSetpoint = temperatureFP;
  ESP_LOGI(LOG_TAG, "Eco temperature sync'ed from cloud, %d degC.", temperatureFP / 1000);
}

void AzureSyncController::updateOverrideFromCloud(const Override & override)
{
  m_settings.setOverride(override);
  m_cachedOverride = override;

  OverrideType::Enum overrideType = override.getType();
  DateTime dateTime;

  switch (overrideType)
  {
  case OverrideType::none:
    ESP_LOGI(LOG_TAG, "Override cleared from cloud - activating week program mode.");
    break;
  case OverrideType::now:
    ESP_LOGI(LOG_TAG, "Override::Now sync'ed from cloud, mode %d.", override.getModeAsCloudEnum());
    break;
  case OverrideType::constant:
    ESP_LOGI(LOG_TAG, "Override::Constant sync'ed from cloud, mode %d.", override.getModeAsCloudEnum());
    break;
  case OverrideType::timed:
    override.getEndDateTime(dateTime);
    ESP_LOGI(LOG_TAG, "Override::Timed sync'ed from cloud, mode %u. Ending: %04u-%02u-%02u %02u:%02u",
                        static_cast<uint32_t>(override.getModeAsCloudEnum()),
                        static_cast<uint32_t>(2000u + dateTime.year),
                        static_cast<uint32_t>(dateTime.month),
                        static_cast<uint32_t>(dateTime.days),
                        static_cast<uint32_t>(dateTime.hours),
                        static_cast<uint32_t>(dateTime.minutes));
    break;
  default:
    ESP_LOGW(LOG_TAG, "Override sync'ed from cloud, but ignored due to invalid type (%d)", overrideType);
    break;
  }
}

void AzureSyncController::updateButtonLockStatusFromCloud(bool buttonLockEnabled)
{
  m_settings.setIsButtonLockOn(buttonLockEnabled);
  m_cachedButtonLockEnabled = buttonLockEnabled;

  ESP_LOGI(LOG_TAG, "Button lock status sync'ed from cloud: %u",
      static_cast<uint32_t>(buttonLockEnabled));
}

void AzureSyncController::updateFromSettings(void)
{
  // Get new values from settings
  int32_t comfortTemp = m_settings.getComfortTemperatureSetPoint();
  int32_t ecoTemp = m_settings.getEcoTemperatureSetPoint();

  Override override;
  m_settings.getOverride(override);

  bool buttonLockEnabled = m_settings.isButtonLockOn();

  // Prepare Azure payload and add parameters for any changed values
  ParameterDataResponse paramData;
  uint8_t numOfAddedParams = 0U;

  if (comfortTemp != m_cachedComfortSetpoint)
  {
    uint8_t newTemp = static_cast<uint8_t>(comfortTemp / 1000);
    uint8_t data[1] = { newTemp };

    if (paramData.addParameterData(ParameterID::COMFORT_TEMPERATURE, data, sizeof(data)))
    {
      ++numOfAddedParams;
      ESP_LOGI(LOG_TAG, "Registered sync request for comfort temperature. New temp: %u", newTemp);
    }
    else
    {
      ESP_LOGW(LOG_TAG, "Failed to register sync request for comfort temperature.");
    }
  }

  if (ecoTemp != m_cachedEcoSetpoint)
  {
    uint8_t newTemp = static_cast<uint8_t>(ecoTemp / 1000);
    uint8_t data[1] = { newTemp };

    if (paramData.addParameterData(ParameterID::ECO_TEMPERATURE, data, sizeof(data)))
    {
      ++numOfAddedParams;
      ESP_LOGI(LOG_TAG, "Registered sync request for eco temperature. New temp: %u", newTemp);
    }
    else
    {
      ESP_LOGW(LOG_TAG, "Failed to register sync request for eco temperature.");
    }
  }

  if (override != m_cachedOverride)
  {
    OverrideType::Enum overrideType = override.getType();
    bool wasAdded = false;

    switch (overrideType)
    {
    case OverrideType::none:
      wasAdded = addDiscardOverrideParameter(paramData);
      break;
    case OverrideType::now:
      wasAdded = addNowOverrideParameter(override.getModeAsCloudEnum(), paramData);
      break;
    case OverrideType::constant:
      ESP_LOGW(LOG_TAG, "Ignored override sync request - Constant override cannot be initiated on appliance.");
      break;
    case OverrideType::timed:
      ESP_LOGW(LOG_TAG, "Ignored override sync request - Timed override cannot be initiated on appliance.");
      break;
    default:
      ESP_LOGW(LOG_TAG, "Failed to register override sync request - invalid override type (%d).", overrideType);
      break;
    }

    if (wasAdded)
    {
      ++numOfAddedParams;
    }
  }

  if ( buttonLockEnabled != m_cachedButtonLockEnabled )
  {
    uint8_t applianceLockType = buttonLockEnabled ?
        ApplianceLockType::CHILD_LOCK : ApplianceLockType::NO_LOCK_ACTIVE;
    bool wasAdded = paramData.addParameterData(ParameterID::LOCK_STATUS,
                                               &applianceLockType,
                                               sizeof(applianceLockType));

    if ( wasAdded )
    {
      ++numOfAddedParams;
      ESP_LOGI(LOG_TAG, "Registered sync request for button lock.");
    }
    else
    {
      ESP_LOGW(LOG_TAG, "Failed to register sync request for button lock.");
    }

  }

  if (numOfAddedParams > 0U)
  {
    paramData.finalize();
    bool shouldUpdateCache = addPayloadToAsyncQueue(paramData); // should not update if queue is full, because then we'll be able to send the update on next 'round'

    if (shouldUpdateCache)
    {
      // Update cache now - as we have sent update to cloud.
      m_cachedComfortSetpoint = comfortTemp;
      m_cachedEcoSetpoint = ecoTemp;
      m_cachedOverride = override;
      m_cachedButtonLockEnabled = buttonLockEnabled;
    }
  }
  else
  {
    // no changes - do nothing
  }

  ForgetMeState::Enum forgetMeState = m_settings.getForgetMeState();
  if ( forgetMeState == ForgetMeState::resetRequested )
  {
    if ( !m_forgetMeFrameSent )
    {
      m_forgetMeFrameSent = addForgetMeFrameToAsyncQueue();
    }
  }
  else
  {
    m_forgetMeFrameSent = false;
  }
}

bool AzureSyncController::addDiscardOverrideParameter(ParameterDataResponse & paramData) const
{
  uint8_t data[1] = { 1U }; // 1 = activate
  bool wasAdded = paramData.addParameterData(ParameterID::ACTIVATE_WEEK_PROGRAM_MODE, data, sizeof(data));

  if (wasAdded)
  {
    ESP_LOGI(LOG_TAG, "Registered sync request to discard override (i.e. activate week program mode).");
  }
  else
  {
    ESP_LOGW(LOG_TAG, "Failed to register sync request to discard override.");
  }

  return wasAdded;
}

bool AzureSyncController::addNowOverrideParameter(CloudTimerHeatingMode::Enum mode, ParameterDataResponse & paramData) const
{
  uint8_t data[3] = {
        static_cast<uint8_t>(mode),
        NOW_OVERRIDE_CODE,
        1U // activate
   };

  bool wasAdded = paramData.addParameterData(ParameterID::ACTIVATE_CONSTANT_NOW_OVERRIDE, data, sizeof(data));

  if (wasAdded)
  {
    ESP_LOGI(LOG_TAG, "Registered sync request to activate Now override, mode %d.", mode);
  }
  else
  {
    ESP_LOGW(LOG_TAG, "Failed to register sync request to activate Now override, mode %d.", mode);
  }

  return wasAdded;
}

bool AzureSyncController::addPayloadToAsyncQueue(const ParameterDataResponse & paramData)
{
  bool wasAdded = false;

  if (!m_asyncQueue.isFull())
  {
    MultipleFrameResponse response;

    if (response.addParameterDataResponse(paramData))
    {
      uint32_t responseSize = response.getSize(); // size guaranteed > 0 as we check that params were added
      uint8_t *responseBytes = new uint8_t[responseSize];

      if (response.getBytes(responseBytes, responseSize) == responseSize)
      {
        // We do not need to create base64 or JSON apparently, the Azure SDK does that for us...
        wasAdded = m_asyncQueue.pushItem(responseBytes, responseSize);
      }
      else
      {
        ESP_LOGW(LOG_TAG, "Create sync message failed - invalid data.");
      }

      delete [] responseBytes;
    }
  }

  return wasAdded;
}

bool AzureSyncController::addForgetMeFrameToAsyncQueue()
{
  bool wasAdded = false;

  if (!m_asyncQueue.isFull())
  {
    enum ForgetMeOperation : uint8_t { ForgetMe=0u };

    uint8_t gdid[Constants::GDID_BCD_SIZE] = { };

    m_deviceMeta.getGDIDAsBCD(gdid); // lsb first

    FrameBuilder frameBuilder;
    frameBuilder.addUInt16AsMSB(FrameType::APPLIANCE_FORGET_ME);
    frameBuilder.addUInt8(gdid[5]); // most significant byte first
    frameBuilder.addUInt8(gdid[4]);
    frameBuilder.addUInt8(gdid[3]);
    frameBuilder.addUInt8(gdid[2]);
    frameBuilder.addUInt8(gdid[1]);
    frameBuilder.addUInt8(gdid[0]);
    frameBuilder.addUInt8(ForgetMeOperation::ForgetMe);

    uint32_t frameSize = frameBuilder.getSize();
    uint8_t *frameBytes = new uint8_t[frameSize];

    frameBuilder.getBytes(frameBytes, frameSize);

    // We do not need to create base64 or JSON apparently, the Azure SDK does that for us...
    wasAdded = m_asyncQueue.pushItem(frameBytes, frameSize);

    delete[] frameBytes;

    if ( wasAdded )
    {
      ESP_LOGI(LOG_TAG, "ForgetMe frame added to outgoing async Azure queue");
    }
    else
    {
      ESP_LOGE(LOG_TAG, "ForgetMe frame _not_ added to outgoing async Azure queue");
    }
  }

  return wasAdded;
}
