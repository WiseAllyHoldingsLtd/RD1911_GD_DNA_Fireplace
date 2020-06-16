#include "EspCpp.hpp"
#include "Constants.h"
#include "WeekProgram.h"
#include "HeatControllerTask.hpp"


namespace
{
  const char * LOG_TAG = "HeatController";
  const uint32_t CONTROLLER_LOOP_INTERVAL_MS = 599U;
  const uint32_t RESEND_CONFIG_INTERVAL_MS = (5U * 60000U); // every five minutes
}

HeatControllerTask::HeatControllerTask(RUInterface &regUnit, TimerDriverInterface &timer, SystemTimeDriverInterface &systemTime, SettingsInterface &settings,
                                        QueueInterface<bool> &isHeatElementOnQueue, TWIDriverInterface &twiDriver, SoftwareResetDriverInterface &resetDriver)

: Task(), m_regUnit(regUnit), m_timerDriver(timer), m_systemTime(systemTime), m_settings(settings),
  m_isRUHeatElementOnQueue(isHeatElementOnQueue), m_twiDriver(twiDriver), m_resetDriver(resetDriver),
  m_setPoint(0), m_ruCommFailures(0U), m_timestampLastRUHeatElementUpdate(0U), m_timestampLastConfigResend(0U)
{
}

void HeatControllerTask::run(bool eternalLoop)
{
  WeekProgram weekProgram(m_weekProgramNodes);

  do
  {
    /* Update RU setpoint if required */

    int32_t currentSetPoint = m_settings.getSetPoint();

    if ((m_setPoint != currentSetPoint) || (m_timerDriver.getTimeSinceTimestampMS(m_timestampLastConfigResend) >= RESEND_CONFIG_INTERVAL_MS))
    {
      if (updateRUSetPoint(currentSetPoint))
      {
        m_setPoint = currentSetPoint;
        m_ruCommFailures = 0U;
        m_timestampLastConfigResend = m_timerDriver.getTimeSinceBootMS();
      }
      else
      {
        m_ruCommFailures++;
        handleRUCommFailures();
      }
    }

    /* Check RU heat element state if required */

    if (m_timerDriver.getTimeSinceTimestampMS(m_timestampLastRUHeatElementUpdate) >= Constants::RU_HEAT_ELEMENT_UPDATE_PERIOD_MS)
    {
      bool isHeatElementActive = m_regUnit.isHeatElementActive();
      m_isRUHeatElementOnQueue.push(isHeatElementActive, 0U);
      m_timestampLastRUHeatElementUpdate = m_timerDriver.getTimeSinceBootMS();
    }

    /* Load new week program changes (if any) and update heating mode */

    DateTime currentDateTime;
    m_settings.getLocalTime(currentDateTime);

    m_weekProgramNodes.updateNodes(m_settings);
    bool shouldUpdateWeekProgramAsap = weekProgram.shouldUpdateASAP();

    if (shouldUpdateWeekProgramAsap || (currentDateTime.seconds == 0))
    {
      Override currentOverride;
      m_settings.getOverride(currentOverride);

      // Clear timed override if expired
      clearOverrideIfTimeExpired(currentOverride, currentDateTime);

      // Clear overrides 03:00:00 and 03:00:01 (just to be safe in case we get a 'time glitch')
      if (currentDateTime.hours == 3U && (currentDateTime.minutes == 0U || currentDateTime.minutes == 1U))
      {
        clearOverrideIfNow(currentOverride);
      }

      bool unusedOutput;
      HeatingMode::Enum heatingMode = weekProgram.getCurrentStatus(currentDateTime, 0U, unusedOutput);

      if (m_settings.getPrimaryHeatingMode() != heatingMode)
      {
        clearOverrideIfNow(currentOverride);
        m_settings.setPrimaryHeatingMode(heatingMode);
      }
    }

    /* Ahhh, relax until next round */

    delayMS(CONTROLLER_LOOP_INTERVAL_MS);

  } while (eternalLoop);
}


bool HeatControllerTask::updateRUSetPoint(int32_t setPoint)
{
  bool wasUpdated = false;

  if (setPoint <= Constants::HEATER_OFF_SET_POINT)
  {
    wasUpdated = m_regUnit.updateConfig(RURegulator::REGULATOR_OFF, Constants::USE_INTERNAL_TEMP, Constants::ENABLE_RU_POT, 0);

    if (wasUpdated)
    {
      ESP_LOGI(LOG_TAG, "Temperature set: OFF");
    }
  }
  else
  {
    wasUpdated = m_regUnit.updateConfig(RURegulator::REGULATOR_SETPOINT, Constants::USE_INTERNAL_TEMP, Constants::ENABLE_RU_POT, setPoint);

    if (wasUpdated)
    {
      ESP_LOGI(LOG_TAG, "Temperature set: %d", setPoint);
    }
  }

  return wasUpdated;
}


void HeatControllerTask::handleRUCommFailures(void)
{
  if (m_ruCommFailures >= Constants::NUM_OF_RU_TIMEOUT_RETRIES_BEFORE_RESTART)
  {
    ESP_LOGE(LOG_TAG, "Fatal RU communication error! Rebooting...");
    m_resetDriver.reset();
  }
  else if (m_ruCommFailures == Constants::NUM_OF_RU_TIMEOUT_RETRIES_BEFORE_REINIT)
  {
    ESP_LOGE(LOG_TAG, "Critical RU communication error! Resetting TWI bus driver.");
    m_twiDriver.setup(Constants::TWI_BUS_MAX_FREQUENCY);
  }
  else
  {
    // no action
  }
}

void HeatControllerTask::clearOverrideIfNow(const Override & override)
{
  if (override.getType() == OverrideType::now)
  {
    Override noOverride(OverrideType::none, HeatingMode::comfort);
    m_settings.setOverride(noOverride);
  }
}

void HeatControllerTask::clearOverrideIfTimeExpired(const Override & override, const DateTime &currentDateTime)
{
  if (override.getType() == OverrideType::timed)
  {
    DateTime endDateTime;
    override.getEndDateTime(endDateTime);
    bool isEndDateValid = endDateTime.isValid();
    bool isEndDateExpired = false;

    if (isEndDateValid)
    {
      uint32_t endUnixTime = m_systemTime.getUnixTimeFromDateTime(endDateTime);
      uint32_t currentUnixTime = m_systemTime.getUnixTimeFromDateTime(currentDateTime);
      isEndDateExpired = (endUnixTime <= currentUnixTime);
    }

    if ((!isEndDateValid) || isEndDateExpired)
    {
      Override noOverride(OverrideType::none, HeatingMode::comfort);
      m_settings.setOverride(noOverride);
    }
  }
}
