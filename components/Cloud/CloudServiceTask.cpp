#include "EspCpp.hpp"

#include "DiscoveryService.hpp"
#include "HealthCheckService.hpp"
#include "SntpDriverInterface.h"

#include "CloudServiceTask.hpp"

namespace {
  const char LOG_TAG[] = "CloudServiceTask";
}


CloudServiceTask::CloudServiceTask()
: m_cloudDriver(nullptr),
  m_deviceMeta(nullptr),
  m_settings(nullptr),
  m_connectionStringQueue(nullptr),
  m_discoveryResultQueue(nullptr),
  m_healthCheckEventFlags(nullptr),
  m_running(false),
  m_paused(true),
  m_inited(false),
  m_currentHealth(0u),
  m_commandQueue(nullptr),
  m_sntpDriver(nullptr),
  m_wasDiscoverySuccessfulSinceLastRestart(false)
{
}

void CloudServiceTask::init(const CloudDriverInterface *cloud,
                            DeviceMetaInterface *deviceMeta,
                            SettingsInterface *settings,
                            QueueInterface<ConnectionStrings_t> *queue,
                            QueueInterface<WifiConnectionStatus::Enum> *discoveryResultQueue,
                            EventFlagsInterface *healthCheckEventFlags,
                            QueueInterface<CloudServiceCommand> *commandQueue,
                            SntpDriverInterface *sntpDriver)
{
  m_cloudDriver = cloud;
  m_deviceMeta = deviceMeta;
  m_settings = settings;
  m_connectionStringQueue = queue;
  m_discoveryResultQueue = discoveryResultQueue;
  m_healthCheckEventFlags = healthCheckEventFlags;
  m_commandQueue = commandQueue;
  m_sntpDriver = sntpDriver;
  m_inited = true;
}

void CloudServiceTask::run(bool eternalLoop)
{
  if (m_inited) {
    m_running = true;

    HealthCheckService healthCheckService(*m_cloudDriver,
                                          *m_deviceMeta,
                                          *m_settings,
                                          *m_healthCheckEventFlags);

    do
    {
      ESP_LOGI(LOG_TAG, "Starting from the top of run()...");
      setWasDiscoverySuccessfulSinceLastRestart(false);
      m_currentHealth = 0u;
      m_healthCheckEventFlags->clear(Constants::HEALTH_CHECK_FLAG_OK);
      if ( m_paused )
      {
        ESP_LOGI(LOG_TAG, "Paused, waiting for unpause");
        while ( !isUnpauseRequested(Constants::CLOUD_SERVICE_UNPAUSE_SLEEP_TIME) )
        {
          ESP_LOGI(LOG_TAG, "Still waiting for unpause");
        }
        m_paused = false;
        ESP_LOGI(LOG_TAG, "Unpaused, resuming work");
      }

      do {
        /* Get time using SNTP so that cert validation will work */
        uint64_t sysTimeUnix = 0u;
        m_sntpDriver->runSntpOnce(Constants::NTP_URL1,
                                  Constants::NTP_URL2,
                                  sysTimeUnix);
        // Always call set time to avoid weird situations,
        // ignoring return value of runSntpOnce
        m_settings->setTimeFromUnixTime(sysTimeUnix);

        DiscoveryService discoveryService(*m_cloudDriver,
                                          *m_deviceMeta,
                                          *m_connectionStringQueue,
                                          *m_discoveryResultQueue);
        if (discoveryService.registerDevice()) {
          setWasDiscoverySuccessfulSinceLastRestart(true);
          ESP_LOGI(LOG_TAG, "Registered OK!");
        }
        else {
          m_paused=isPauseRequested(Constants::DISCOVERY_RETRY_INTERVAL_MS);
        }
      } while ( !wasDiscoverySuccessfulSinceLastRestart()
          && !isPaused() );

      if (wasDiscoverySuccessfulSinceLastRestart()) {
        do
        {
          checkHealth(healthCheckService);
        } while ( eternalLoop && shouldRunRegularHealthChecks() );
      }
    } while ( eternalLoop );
  }
  else {
    ESP_LOGE(LOG_TAG, "Test task not initialized!");
  }
  m_running = false;
  ESP_LOGI(LOG_TAG, "CloudService task done");
}

bool CloudServiceTask::isRunning() const
{
  return m_running;
}

uint8_t CloudServiceTask::getCurrentHealth() const
{
  return m_currentHealth;
}

bool CloudServiceTask::requestPause()
{
  bool requestSentOrAlreadyPaused = isPaused();

  if ( !requestSentOrAlreadyPaused )
  {
    m_commandQueue->reset();

    CloudServiceCommand cmd;
    cmd.type = CloudServiceCommand::PAUSE;
    requestSentOrAlreadyPaused = m_commandQueue->push(cmd);
  }
  else
  {
    ESP_LOGW(LOG_TAG, "Pause being requested when already paused");
  }
  return requestSentOrAlreadyPaused;
}

bool CloudServiceTask::requestUnpause()
{
  bool requestSentOrAlreadyUnpaused = !isPaused();

  if ( !requestSentOrAlreadyUnpaused )
  {
    m_commandQueue->reset();

    CloudServiceCommand cmd;
    cmd.type = CloudServiceCommand::UNPAUSE;
    requestSentOrAlreadyUnpaused = m_commandQueue->push(cmd);
  } else
  {
    ESP_LOGW(LOG_TAG, "Unpause being requested when already unpaused");
  }
  return requestSentOrAlreadyUnpaused;
}

bool CloudServiceTask::isPaused() const
{
  return m_paused;
}

void CloudServiceTask::waitUntilPausedOrUnpaused(bool waitForPaused)
{
  while ( waitForPaused != m_paused )
  {
    delayMS(Constants::CLOUD_SERVICE_WAIT_CHECK_PERIOD);
  }
}

bool CloudServiceTask::wasDiscoverySuccessfulSinceLastRestart() const
{
  return m_wasDiscoverySuccessfulSinceLastRestart;
}

bool CloudServiceTask::isControlCmdReceived(uint32_t timeout, CloudServiceCommand &command)
{
  CloudServiceCommand tempCmd;
  bool queueReadSuccessful = false;

  if ( (queueReadSuccessful=m_commandQueue->pop(tempCmd, timeout)) )
  {
    command = tempCmd;
  }
  return queueReadSuccessful;
}

/**
 * @remark
 * If the command at the front of the queue
 * is something other than a request to unpause,
 * it will be discarded. It's safer to use
 * isControlCmdReceived() directly in situations where
 * any command must be handled.
 */
bool CloudServiceTask::isUnpauseRequested(uint32_t timeout)
{
  CloudServiceCommand command;
  if ( !isControlCmdReceived(timeout, command) )
  {
    return false;
  }
  return (command.type == CloudServiceCommand::UNPAUSE);
}

/**
 * @remark
 * If the command at the front of the queue
 * is something other than a pause request,
 * that command will be discarded. Unless
 * CloudServiceTask is already in the paused state
 * (isPaused() returns true).
 */
bool CloudServiceTask::isPauseRequested(uint32_t timeout)
{
  bool result = false;

  if ( !isPaused() )
  {
    CloudServiceCommand command;
    if ( !isControlCmdReceived(timeout, command) )
    {
      result = false;
    }
    else
    {
      result = (command.type == CloudServiceCommand::PAUSE);
    }
  }
  else
  {
    result = true;
  }

  return result;
}

void CloudServiceTask::setWasDiscoverySuccessfulSinceLastRestart(bool wasSuccessful)
{
  m_wasDiscoverySuccessfulSinceLastRestart = wasSuccessful;
}

// Perform health check with multiple tries and back-off
// according to
// GD IoT Field node and Cloud interactions_v1.2.docx
void CloudServiceTask::checkHealth(HealthCheckService &healthCheckService)
{
  bool healthOk = false;

  uint8_t tempHealth = 0u;

  for ( uint8_t i=1;
        i<=Constants::HEALTHCHECK_NUM_TRIES_WITH_BACKOFF
        && !healthOk
        && !isPaused();
        ++i )
  {
    for ( uint8_t n=1;
        n<=Constants::HEALTHCHECK_NUM_TRIES_WITHOUT_BACKOFF
        && !healthOk
        && !isPaused();
        ++n )
    {
      if (!healthCheckService.checkHealth(tempHealth)) {
        ESP_LOGE(LOG_TAG, "Health check failed");
      }
      healthOk = tempHealth == Constants::HEALTHCHECK_FULL_HEALTH;
      m_currentHealth = tempHealth;

      if ( n+1 <= Constants::HEALTHCHECK_NUM_TRIES_WITHOUT_BACKOFF && !healthOk )
      {
        if ( (m_paused=isPauseRequested(Constants::HEALTHCHECK_RETRY_INTERVAL_MS)) )
        {
          ESP_LOGI(LOG_TAG, "Aborting health check retry due to pause request");
        }
      }
    }
    if ( i+1 <= Constants::HEALTHCHECK_NUM_TRIES_WITH_BACKOFF && !healthOk )
    {
      if ( (m_paused=isPauseRequested(Constants::HEALTHCHECK_BACKOFF_INTERVAL_MS)) )
      {
        ESP_LOGI(LOG_TAG, "Aborting health check retry with back-off time due to pause request");
      }
    }
  }

  // TODO if health check with back-off fails four times,
  // GD IoT Field node and Cloud interactions_v1.2.docx says reboot
}

bool CloudServiceTask::shouldRunRegularHealthChecks()
{
  return (m_currentHealth == Constants::HEALTHCHECK_FULL_HEALTH
      && !(m_paused=isPauseRequested(Constants::HEALTHCHECK_INTERVAL_MS)));
}
