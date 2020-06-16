#include <cstring>
#include "EspCpp.hpp"
#include "Constants.h"
#include "SettingsStorage.h"
#include "DeviceMetaStorage.h"
#include "PowerOnSelfTest.h"


namespace
{
  const char LOG_TAG[] = "POST";

  enum ErrorCode  // Actual error code is a bitmask based on the following values (256 is max allowable bit, as we can only display 999)
  {
    SUCCESS = 0,
    RTC_SELFTEST_FAILED = 1,      // RTC failed, time reset to default (but would be corrected by healthCheck)
    SYSTIME_SELFTEST_FAILED = 2,  // sysTime not working... time probably won't run at all
    CRYPTO_SELFTEST_FAILED = 4,   // CryptoChip init failed
    STORAGE_SELFTEST_FAILED = 8,  // settings cleared
    WIFI_SELFTEST_FAILED = 16,    // Wifi init failed
    BT_SELFTEST_FAILED = 32,      // BT init failed
    META_SELFTEST_FAILED = 64,    // Device meta data unavailable (such as GDID)
    RU_SELFTEST_FAILED = 128,     // RU selfTest or config failed (FATAL error as SW version is unknown)
    RU_NOT_COMPATIBLE = 256       // RU is probably 'normal' type, not high power. FATAL error
  };

  const uint16_t FATAL_LIMIT = 128;
}


PowerOnSelfTest::PowerOnSelfTest(void) : m_errorCode(ErrorCode::SUCCESS)
{
}


bool PowerOnSelfTest::run(PartitionDriverInterface & partitions, TimerDriverInterface &timer, DisplayInterface & display,
                          RtcInterface &rtc, SystemTimeDriverInterface &sysTime,
                          RUInterface &ru, CryptoDriverInterface &crypto,
                          BTDriverInterface &bluetooth, WifiDriverInterface &wifi,
                          PersistentStorageDriverInterface &settingsStorage,
                          PersistentStorageDriverInterface &metaStorage)
{
  ESP_LOGI(LOG_TAG, "Starting Power On Self Test.");
  m_errorCode = ErrorCode::SUCCESS;

  /* Print information partition info */
  ESP_LOGI(LOG_TAG, "Booting from '%s' at offset 0x%08x.", partitions.getBootPartitionName(), partitions.getBootPartitionAddress());

  if (partitions.getBootPartitionAddress() != partitions.getRunningPartitionAddress())
  {
    ESP_LOGW(LOG_TAG, "Configured boot partition '%s' differs from actual running partition '%s'! Possible corrupt OTA data partition or OTA app partition.",
        partitions.getBootPartitionName(),
        partitions.getRunningPartitionName());
  }

  ESP_LOGI(LOG_TAG, "Next OTA update partition is '%s' at offset 0x%08x.", partitions.getUpdatePartitionName(), partitions.getUpdatePartitionAddress());


  /* Setup display, then wait to allow the "all LEDs on" to be visible for user */
  display.setup();
  timer.waitMS(1000U);


  /* Initialize storage */

  bool isTestMode = true;
  bool isTimeSet = false;

  if (settingsStorage.init(Constants::SETTINGS_PARTITION_NAME, false))
  {
    ESP_LOGI(LOG_TAG, "Settings storage initialization OK.");
    SettingsStorage storage(settingsStorage);

    RunMode::Enum runMode = RunMode::test;
    storage.retrieveRunMode(runMode);
    isTestMode = (runMode == RunMode::test);

    storage.retrieveIsTimeSet(isTimeSet);
  }
  else
  {
    ESP_LOGE(LOG_TAG, "Settings storage initialization FAILED. Attempting erase and reinit.");
    m_errorCode += ErrorCode::STORAGE_SELFTEST_FAILED;

    if (settingsStorage.erase(Constants::SETTINGS_PARTITION_NAME))
    {
      if (settingsStorage.init(Constants::SETTINGS_PARTITION_NAME, false))
      {
        ESP_LOGE(LOG_TAG, "Settings storage reinitialization OK. Contents has been erased.");
      }
      else
      {
        ESP_LOGE(LOG_TAG, "Settings storage reinitialization FAILED. Contents has been erased.");
      }
    }
    else
    {
      ESP_LOGE(LOG_TAG, "Settings storage reinitialization FAILED.");
    }
  }


  /* Initialize device meta storage */

  if (metaStorage.init(Constants::META_PARTITION_NAME, !isTestMode))
  {
    ESP_LOGD(LOG_TAG, "Device meta data storage initialization OK (%s)", isTestMode ? "read/write" : "read-only");
    DeviceMetaStorage storage(metaStorage);

    char deviceGDID[Constants::GDID_SIZE_MAX];
    bool isGDIDSet = storage.retrieveGDID(deviceGDID, sizeof(deviceGDID));

    if (isGDIDSet)
    {
      ESP_LOGI(LOG_TAG, "Device GDID is %s", deviceGDID);
    }
    else
    {
      if (!isTestMode)
      {
        // Not test mode, and GDID not set - Fatal error
        ESP_LOGE(LOG_TAG, "Device meta data storage initialization FAILED (no GDID) - Fatal error");
        m_errorCode += ErrorCode::META_SELFTEST_FAILED;
      }
    }
  }
  else
  {
    ESP_LOGE(LOG_TAG, "Device meta data storage initialization FAILED - Fatal error");
    m_errorCode += ErrorCode::META_SELFTEST_FAILED;
  }


  /* RTC and SystemTime */

  if (rtc.setup())
  {
    if (!isTimeSet)
    {
      DateTime dateTime = Constants::DEFAULT_DATETIME;
      rtc.setCurrentTime(dateTime);

      uint32_t unixTime = sysTime.getUnixTimeFromDateTime(dateTime);
      sysTime.setUnixTime(unixTime);

      SettingsStorage storage(settingsStorage);
      storage.storeIsTimeSet(true);

      ESP_LOGD(LOG_TAG, "RTC initial configuration set. Using default time.");
    }
    else
    {
      DateTime dateTime;
      TimeStatus::Enum timeStatus = rtc.getCurrentTime(dateTime);

      if (timeStatus != TimeStatus::OK)
      {
        dateTime = Constants::DEFAULT_DATETIME;
        rtc.setCurrentTime(dateTime);

        ESP_LOGW(LOG_TAG, "RTC initialization FAILED. Using default time.");
        m_errorCode += ErrorCode::RTC_SELFTEST_FAILED;
      }

      uint32_t unixTime = sysTime.getUnixTimeFromDateTime(dateTime);
      sysTime.setUnixTime(unixTime);

      ESP_LOGD(LOG_TAG, "RTC initialization OK. SystemTime set.");
    }
  }
  else
  {
    ESP_LOGW(LOG_TAG, "RTC initialization FAILED. Using default time.");
    m_errorCode += ErrorCode::RTC_SELFTEST_FAILED;

    DateTime dateTime = Constants::DEFAULT_DATETIME;
    uint32_t unixTime = sysTime.getUnixTimeFromDateTime(dateTime);
    sysTime.setUnixTime(unixTime);
  }

  // System time has now been set, perform "selfTest" to see it ticking
  uint32_t startTime = sysTime.getUnixTime();
  timer.waitMS(1500U);
  uint32_t endTime = sysTime.getUnixTime();

  if ((endTime == (startTime + 1U)) || (endTime== (startTime + 2U)))
  {
    ESP_LOGD(LOG_TAG, "SystemTime initialization OK.");
  }
  else
  {
    ESP_LOGE(LOG_TAG, "SystemTime initialization FAILED (clock not ticking).");
    m_errorCode += ErrorCode::SYSTIME_SELFTEST_FAILED;
  }


  /* RU init. and config, and SW version check
   * Note:
   * In case of RU error we need to stop the Wifi/BT tests, as a normal
   * RU does not provide enough power to perform those tests.
   *
   * */

  bool isRUErrorDetected = false;

  if (ru.performSelfTest())
  {
    if (ru.updateConfig(RURegulator::REGULATOR_OFF, true, false, 22000))
    {
      if (!ru.isHighPowerVersion())
      {
        ESP_LOGE(LOG_TAG, "RegUnit configuration check (SW) FAILED - Fatal error.");
        m_errorCode += ErrorCode::RU_NOT_COMPATIBLE;
        isRUErrorDetected = true;
      }
      else
      {
        ESP_LOGD(LOG_TAG, "RegUnit initialization OK.");
      }
    }
    else
    {
      ESP_LOGE(LOG_TAG, "RegUnit initialization FAILED (config failed).");
      m_errorCode += ErrorCode::RU_SELFTEST_FAILED;
      isRUErrorDetected = true;
    }
  }
  else
  {
    ESP_LOGE(LOG_TAG, "RegUnit initialization FAILED (selftest failed).");
    m_errorCode += ErrorCode::RU_SELFTEST_FAILED;
    isRUErrorDetected = true;
  }


  /* CryptoDriver init and check */

  if (crypto.resetAndInit())
  {
    bool isCryptoOK = (crypto.isProvisioned() && crypto.readCertificates());

    if (isCryptoOK)
    {
      ESP_LOGD(LOG_TAG, "CryptoChip initialization OK.");

      uint8_t buffer[9] = {};

      if (crypto.readSerialNumber(buffer))
      {
        ESP_LOGI(LOG_TAG, "CryptoChip serial: 0x%02x %02x %02x %02x %02x %02x %02x %02x %02x",
            buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7], buffer[8]);
      }
    }
    else
    {
      ESP_LOGE(LOG_TAG, "CryptoChip initialization FAILED (not provisioned).");
      m_errorCode += ErrorCode::CRYPTO_SELFTEST_FAILED;
    }
  }
  else
  {
    ESP_LOGE(LOG_TAG, "CryptoChip initialization FAILED.");
    m_errorCode += ErrorCode::CRYPTO_SELFTEST_FAILED;
  }

  if (!isRUErrorDetected)
  {
    /* BTDriver init */
    if (bluetooth.resetAndInit())
    {
      ESP_LOGD(LOG_TAG, "Bluetooth initialization OK.");
    }
    else
    {
      ESP_LOGE(LOG_TAG, "Bluetooth initialization FAILED.");
      m_errorCode += ErrorCode::BT_SELFTEST_FAILED;
    }


    /* WifiDriver init */
    if (wifi.resetAndInit())
    {
      ESP_LOGD(LOG_TAG, "Wifi initialization OK.");
    }
    else
    {
      ESP_LOGE(LOG_TAG, "Wifi initialization FAILED.");
      m_errorCode += ErrorCode::WIFI_SELFTEST_FAILED;
    }
  }

  ESP_LOGI(LOG_TAG, "Firmware version %02d.%02d.%02d", Constants::SW_VERSION / 10u, Constants::SW_VERSION % 10u, Constants::SW_TEST_NO);
  ESP_LOGI(LOG_TAG, "Power On Self Test executed with result code %d", m_errorCode);
  return (m_errorCode == ErrorCode::SUCCESS);
}

int32_t PowerOnSelfTest::getErrorCode() const
{
  return m_errorCode;
}

bool PowerOnSelfTest::isFatalError(void) const
{
  return static_cast<uint16_t>(m_errorCode) >= FATAL_LIMIT;
}
