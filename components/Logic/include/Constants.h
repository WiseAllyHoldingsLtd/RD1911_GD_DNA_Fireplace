#pragma once

#include "UIState.h"
#include "DateTime.h"
#include "HeatingMode.h"
#include "CloudTimerHeatingMode.h"
#include "OverrideType.h"
#include "RunMode.h"
#include "EOLStatus.h"

#include <stdint.h>
#include <cstddef>

#define NUM_OF_WEEKDAYS 7U
#define NUM_OF_UNIQUE_INTERVALS 10U

#define IS_PRODUCTION_VERSION 0


class Constants
{
public:
  /* Official SW version is given by SW_VERSION, when releasing new version the SW_TEST_NO should be zero.
   * SW_TEST_NO is increased by one to "force" upgrade from Cloud during testing. When testing is complete,
   * SW_VERSION should be increased by one (i.e. increase "minor") or possibly rounded up to closest 10 to
   * set new "major" version. At the same time SW_TEST_NO should be set to zero again.
   */
  static const uint8_t SW_VERSION = 18U; // Fixedpoint 1 decimal (10 = 1.0, max. supported value is 255=25.5)
  static const uint8_t SW_TEST_NO = 11U; // Max supported value is 99

  /* Urls for discovery and health check */
  static const char DISCOVERY_URL[];
  static const char HEALTHCHECK_URL[];

  /* Urls for NTP */
  static const char NTP_URL1[];
  static const char NTP_URL2[];

  static const uint8_t NTP_RUN_ONCE_TIMEOUT_SECONDS = 15u;

  /* CompanyCode, CompanySubsetCode acc. to Defined in Parameter Definition Wif Appliance_v1.3 */
  static const uint8_t COMPANY_CODE = 2U;
  static const uint8_t COMPANY_SUBSET_CODE = 2U;

  /* Default eeprom values (factory reset): */
  static const HeatingMode::Enum DEFAULT_HEATING_MODE = HeatingMode::comfort;
  static const int32_t DEFAULT_COMFORT_TEMPERATURE_SET_POINT = 21000;
  static const int32_t DEFAULT_ECO_TEMPERATURE_SET_POINT = 18000;
  static const int32_t HEATER_OFF_SET_POINT = 0;

  static const OverrideType::Enum DEFAULT_OVERRIDE_TYPE = OverrideType::none;

  static const bool DEFAULT_IS_LOCAL_OVERRIDE_ACTIVE = false;

  static const bool DEFAULT_IS_BUTTON_LOCK_ON = false;
  static const uint32_t DEFAULT_SET_TIME_LIMITATION_MS = 60 * 60000U;

  static const RunMode::Enum DEFAULT_RUN_MODE = RunMode::test;
  static const char DEFAULT_WIFI_SSID[];
  static const char DEFAULT_WIFI_PASSWORD[];
  static const char DEFAULT_GDID[];
  static const uint32_t DEFAULT_BT_PASSKEY = 110188U; /* Default value used during provisioning acc. to William Watterson */
  static const EOLStatus::Enum DEFAULT_EOL_STATUS = EOLStatus::notCompleted;
  static const int32_t DEFAULT_UTC_OFFSET = 0;

  static const uint8_t CLOUD_PARAMETER_INVALID_VALUE = 0xEE;
  static const uint16_t CLOUD_PARAMETER_END_OF_FRAME = 0U;
  static const uint8_t CLOUD_ASYNC_MSG_QUEUE_SIZE = 5U;
  static const uint16_t CLOUD_RESPONSE_MSG_MAX_SIZE = 1024U;
  static const uint32_t CLOUD_DEFAULT_PROCESSING_TIMEOUT_MS = 10000U; // Same as gateway!
  static const uint32_t CLOUD_AZURE_DOWORK_DEFAULT_INTERVAL_MS = 5U;
  static const uint32_t CLOUD_AZURE_DOWORK_SHORT_INTERVAL_MS = 1U;



  static const uint8_t GDID_SIZE_MAX = 13U;  /* 12 digits + zeroterm */
  static const uint8_t GDID_BCD_SIZE = 6U;
  static const uint8_t BT_PASSKEY_STRING_SIZE_MAX = 7u; /* 6 digits + null terminator */

  static const uint32_t DISCOVERY_RETRY_INTERVAL_MS = 120000U;
  static const uint32_t HEALTHCHECK_INTERVAL_MS = 120000U;
  static const uint32_t HEALTHCHECK_RETRY_INTERVAL_MS = 30000U;
  static const uint32_t HEALTHCHECK_BACKOFF_INTERVAL_MS = 60000U;
  static const uint8_t HEALTHCHECK_FULL_HEALTH = 100u;
  static const uint8_t HEALTHCHECK_NUM_TRIES_WITH_BACKOFF = 4u;
  static const uint8_t HEALTHCHECK_NUM_TRIES_WITHOUT_BACKOFF = 3u;

  static const uint8_t MAX_NUM_OF_MEASURES_FOR_ADAPTIVE_HEATING = 10U;

  static const DateTime DEFAULT_DATETIME;
  static const uint8_t MAX_TIMER_INTERRUPT_HANDLERS = 5U;


  /* System test constants: */
  static const uint8_t SYSTEM_TEST_COMMAND_TYPE_OFFSET = 0U;
  static const uint8_t SYSTEM_TEST_COMMAND_LENGTH_OFFSET = 1U;
  static const uint8_t SYSTEM_TEST_COMMAND_HEADER_LENGTH = SYSTEM_TEST_COMMAND_LENGTH_OFFSET + 1U;
  static const uint8_t SYSTEM_TEST_REPLY_OFFSET = 2U; /* This is because the first payload byte from SPI slave is sent after second byte from master */
  static const uint8_t SPI_CHUNK_SIZE = 110U; /* Must be the same on unit, IOExpander and PC*/
  static const uint8_t SPI_SYSTEM_TEST_BUFFER_LENGTH = SYSTEM_TEST_REPLY_OFFSET + SPI_CHUNK_SIZE;
  static const uint8_t SYSTEM_TEST_COMMAND_TYPE_VIEW   = 1U;
  static const uint8_t SYSTEM_TEST_COMMAND_TYPE_BUTTON = 2U;
  static const uint8_t SYSTEM_TEST_COMMAND_TYPE_CLEAR_SETTINGS = 3U;
  static const uint8_t SYSTEM_TEST_COMMAND_TYPE_LED_VIEW = 4U;
  static const uint8_t SYSTEM_TEST_EXPECTED_LED_VIEW_MESSAGE_LENGTH  = 9U;
  static const uint8_t SYSTEM_TEST_EXPECTED_BUTTON_MESSAGE_LENGTH = 21U;
  static const uint8_t SYSTEM_TEST_EXPECTED_CLEAR_SETTINGS_MESSAGE_LENGTH = 10U;


  /* Physical button constants: */
  static const uint16_t BUTTON_UPDATE_INTERVAL_MS = 15U;
  static const uint16_t BUTTON_DOWN_COUNTER_LIMIT = 7U;
  static const uint32_t BUTTON_STARTED_BEING_HELD_THRESHOLD_MS = 300U; //To support several buttons being held but not at the exact same time.
  static const uint32_t BUTTON_HELD_THRESHOLD_MS = 600U;
  static const uint32_t BUTTON_HOLD_FIRST_FIRE_MS = BUTTON_HELD_THRESHOLD_MS; /* The same as button held threshold to avoid
                                                                              glitch where if a button is pressed longer than BUTTON_HELD_THRESHOLD_MS but
                                                                              shorter than  BUTTON_HOLD_FIRST_FIRE_MS then the GUI will ignore it. */
  static const uint32_t BUTTON_HOLD_FIRE_RATE_MS = 400U;
  static const uint32_t BUTTON_HOLD_TIME_SCREEN_FIRE_RATE_MS = 200U;

  static const uint8_t BACK_BUTTON_INDEX = 0U;
  static const uint8_t PLUS_BUTTON_INDEX = 1U;
  static const uint8_t MINUS_BUTTON_INDEX = 2U;
  static const uint8_t UP_BUTTON_INDEX = PLUS_BUTTON_INDEX;
  static const uint8_t DOWN_BUTTON_INDEX = MINUS_BUTTON_INDEX;


  /* Business logic constants: */
  static const Screen::Enum STARTUP_SCREEN = Screen::factoryResetScreen;
  static const uint8_t TEXT_MAX_LENGTH = 3u;


  static const int32_t MAX_SETP_TEMPERATURE = 30000;
  static const int32_t MIN_SETP_TEMPERATURE = 7000;
  static const int32_t SETP_TEMPERATURE_STEPS = 1000; /** When user increases/decreases set point. */
  static const int32_t ANTI_FROST_SET_POINT = 7000;

  static const uint64_t NO_JITTER_PERIOD_MS = 1000U;
  static const uint16_t BUTTON_COMBO_HOLD_TIME_MS = 1000U;
  static const uint16_t BUTTON_TEST_HOLD_TIME_MS = 3000U;
  static const uint16_t RESET_HOLD_TIME_MS = 3000U;

  static const bool USE_INTERNAL_TEMP = true;
  static const bool ENABLE_RU_POT = false;

  static const uint8_t DISPLAY_REDRAW_INTERVAL = 25U;  // milliseconds * 10

  static const uint32_t TWI_BUS_MAX_FREQUENCY = 50000u;
  static const uint8_t TWI_TRANSFER_TIMEOUT_MS = 13u; // Maximum allowable in I2C driver is approx. 13.1 ms
  static const uint8_t NUM_OF_RU_TIMEOUT_RETRIES_BEFORE_REINIT = 100u;
  static const uint8_t NUM_OF_RU_TIMEOUT_RETRIES_BEFORE_RESTART = 200u;
  static const uint8_t NUM_OF_RTC_TIMEOUT_RETRIES_BEFORE_RESTART = 20u;
  static const uint16_t RTC_UPDATE_PERIOD_MS = 750u;
  static const uint16_t RU_HEAT_ELEMENT_UPDATE_PERIOD_MS = 1000u;

  /* Display dim constants: */
  static const uint16_t DIM_TIMEOUT_MS = 15000U;
  static const uint8_t DIM_DARK_VALUE = 5U;
  static const uint8_t DIM_NORMAL_VALUE = 10U;
  static const uint8_t DIM_BRIGHT_VALUE = 20U;
  static const uint8_t DIM_MAX_VALUE = 200U;
  static const uint16_t DIM_FADE_OUT_TIME_MS = 2000U;
  static const uint16_t DIM_FADE_IN_TIME_MS = 500U;
  // TODO do we need more than one dim value? can the user set this manually?

  static const uint32_t BLUETOOTH_LED_PERIOD_NO_CONNECTION_MS = 2000U;
  static const uint32_t BLUETOOTH_LED_PERIOD_CONNECTED_MS = 500U;
  static const uint32_t BLUETOOTH_SCROLL_PAGE_DURATION_MS = 3000u;
  static const uint32_t BLUETOOTH_SCROLL_PAGE_PAUSE_MS = 200u;

  static const uint32_t BLUETOOTH_SCREEN_DEFAULT_CONN_WAIT_TIME_MS = 72000U;  /* return to main if not connected */
  static const uint16_t BLUETOOTH_SCREEN_SHORT_CONN_WAIT_TIME_MS = 30000U;    /* return to main if not connected */
  static const uint16_t BLUETOOTH_SCREEN_MAX_VISIBLE_TIME_S = 600U;           /* return to main even is connected */
  static const uint16_t SW_VERSION_SCREEN_VISIBLE_TIME_S = 5U;
  static const uint16_t BUTTON_LOCK_SCREEN_VISIBLE_TIME_S = 3U;
  static const uint16_t CONFIRM_SCREEN_VISIBLE_TIME_S = 3U;
  static const uint16_t NEXT_SCREEN_DELAY_NEVER = 0U;

  static const uint8_t WEEKPROGRAM_SLOTS_PER_WEEKDAY = 4U;
  static const uint8_t WEEKPROGRAM_SLOTS_PER_WEEKDAY_EXT_MAX = 12u;
  static const uint8_t WEEKPROGRAM_SLOT_SIZE = 5U;
  static const uint8_t WEEKPROGRAM_STORAGE_SIZE_PER_DAY = (WEEKPROGRAM_SLOTS_PER_WEEKDAY * WEEKPROGRAM_SLOT_SIZE);
  static const uint8_t WEEKPROGRAM_STORAGE_SIZE_PER_DAY_EXT_MAX;
  static const uint8_t DEFAULT_WEEK_PROGRAM_DATA[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY];
  static const uint8_t DEFAULT_WEEK_PROGRAM_EXT_DATA[Constants::WEEKPROGRAM_SLOT_SIZE];
  static const CloudTimerHeatingMode::Enum DEFAULT_WEEK_PROGRAM_HEATING_MODE = CloudTimerHeatingMode::comfort;

  /* Wifi params */
  static const uint32_t WIFI_CONNECTING_TIMEOUT_MS = 15000U;


  /* BT Wifi setup service and characteristic 128-bit UUIDs in little-endian: */
  /* These 128bit UUIDs are based on source code from the Quantum app,
     The UUIDs in the specification from Firmwave are not correct. */
  static uint8_t WIFI_SETUP_UUID[16];
  // SSID characteristic UUID: 00005001-0000-1000-8000-00805F9B34FB
  static uint8_t SSID_UUID[16];
  static uint8_t PASSWORD_UUID[16];
  static uint8_t STATIC_IP_UUID[16];
  static uint8_t STATIC_NETMASK_UUID[16];
  static uint8_t STATIC_GATEWAY_UUID[16];
  static uint8_t STATIC_DNS1_UUID[16];
  static uint8_t STATIC_DNS2_UUID[16];
  static uint8_t PROXY_SERVER_UUID[16];
  static uint8_t PROXY_PORT_UUID[16];
  static uint8_t PROXY_USERNAME_UUID[16];
  static uint8_t PROXY_PASSWORD_UUID[16];
  static uint8_t CONNECTION_STATUS_UUID[16];
  static uint8_t CONNECTION_STATUS_STRING_UUID[16];
  static uint8_t CONNECTION_TYPE_UUID[16];
  static uint8_t DHCP_STATIC_IP_UUID[16];
  static uint8_t DHCP_MANUAL_PROXY_UUID[16];

  /* BT Finished Goods Tester service and characteristic 128-bit UUIDs in little-endian: */
  /* These UUIDs are based on the BLE_v0.40.xlsx document */
  // Information and EoLT Configuration UUID: 00000000-0003-1000-8000-00805F9B34FB
  static uint8_t FGT_CONFIG_UUID[16];
  static uint8_t LCD_UUID[2];
  static uint8_t TRIAC_UUID[2];
  static uint8_t NTC_UUID[2];
  static uint8_t EOLT_UUID[2];
  static uint8_t TEST_MODE_UUID[2];
  static uint8_t PASSKEY_UUID[2];
  static uint8_t GDID_UUID[2];
  static uint8_t FULL_INFO_UUID[2];
  static uint8_t SW_VER_UUID[2];
  static uint8_t OTA_EN_UUID[2];
  static uint8_t BOOTLOADER_COMMAND[16];
  static uint8_t WIFI_STATUS_UUID[2];

  /* Size, including null terminator, of BLE advertising name */
  static constexpr std::size_t BT_DEVICE_NAME_SIZE_MAX = 32u;

  /* Max number of GATT database entries supported. This limits the number of
   * GATT characteristics and attributes. */
  static constexpr std::size_t GATTDB_ENTRIES_MAX = 64u;
  static constexpr std::size_t GATT_UUIDS_MAX = GATTDB_ENTRIES_MAX/2u;

  /* Some max sizes for connection parameters/GATT characteristics */
  static constexpr std::size_t WIFI_SSID_SIZE_MAX=32u;
  static constexpr std::size_t WIFI_PASSWORD_SIZE_MAX=32u;
  static constexpr std::size_t PROXY_USERNAME_SIZE_MAX=32u;
  static constexpr std::size_t PROXY_PASSWORD_SIZE_MAX=32u;
  static constexpr std::size_t CONNECTION_STATUS_STRING_SIZE_MAX=32u;

  /* Some max sizes for FGT parameters/GATT characteristics */
  static constexpr std::size_t LCD_BT_SIZE_MAX	    = 4u;
  static constexpr std::size_t TRIAC_BT_SIZE_MAX    = 3u;
  static constexpr std::size_t NTC_BT_SIZE_MAX      = 3u;
  static constexpr std::size_t EOLT_BT_SIZE_MAX	    = 1u;
  static constexpr std::size_t TEST_BT_SIZE_MAX	    = 1u;
  static constexpr std::size_t PASSKEY_BT_SIZE_MAX  = 5u;
  static constexpr std::size_t GDID_BT_SIZE_MAX     = 6u;
  static constexpr std::size_t FULL_INFO_BT_SIZE_MAX= 60u;
  static constexpr std::size_t SW_VER_BT_SIZE_MAX   = 3u;
  static constexpr std::size_t WIFI_STATUS_BT_SIZE_MAX = 1u;
  static constexpr std::size_t OTA_EN_BT_SIZE_MAX   = 1u;
  static constexpr std::size_t OTA_CMD_BT_SIZE_MAX  = 512u;

  static constexpr std::size_t MAC_ADDRESS_SIZE=6u;

  /* Constants related to Wifi /Azure */
  static const uint16_t FW_DOWNLOAD_URL_MAX_LENGTH = 1025U;  /* According to Architecture Components_Appliance Cloud interfaces, +1 for zero-term. */
  static const uint32_t FW_DOWNLOAD_CHUNK_SIZE = (8 * 1024U);
  static const uint8_t FW_DOWNLOAD_ATTEMPTS_MAX = 7U;       /* Number of attempts to download each chunk before giving up */
  static const uint64_t FW_UPGRADE_TIMEOUT = 60u*60u*1000u;
  static const uint32_t FW_DOWNLOAD_ID_MAX_LENGTH = 129U;
  static const uint8_t FW_UPGRADE_MAX_ATTEMPTS = 5U;
  static const uint32_t FW_UPGRADE_REBOOT_DELAY_MS = 3000u;

  static const uint16_t CLOUD_CONNECTION_STRING_MAX_LENGTH = 192U;
  static const uint64_t AZURE_MAX_SHUTDOWN_WAIT_TIME_MS = 60000U;
  static const uint64_t AZURE_SEND_CONFIRMATION_APP_MAX_WAIT_MS = 3u*60000u;
  static const uint32_t AZURE_MAX_CONSECUTIVE_SEND_TIMEOUTS = 3u;
  static const uint32_t AZURE_OPTION_MESSAGE_TIMEOUT_MS = 0u; // 0 - disabled

  static const uint16_t CRYPTO_CLIENT_CERT_SIZE_MAX = 1024u;
  static const uint16_t CRYPTO_DER_CERT_INIT_SIZE = 1024u;
  static const uint16_t CRYPTO_PEM_CERT_INIT_SIZE = 1024u;

  static const char SETTINGS_PARTITION_NAME[];
  static const char META_PARTITION_NAME[];
  
  /* Health check event flags */
  static const uint8_t HEALTH_CHECK_FLAG_OK = 0u;

  /* BLE Constants */
  static const uint8_t FGT_EOLT_COMPLETE = 0x47u;
  static const uint8_t FGT_EOLT_ENTER    = 0x64u;

  /* ConnectionController/connectivity constants */
  static const uint16_t CONNECTION_CONTROLLER_PERIOD = 1000u;
  static const uint64_t CLOUD_SERVICE_PAUSE_TIMEOUT = 60000u;
  static const uint64_t ACTIVATING_WIFI_SETUP_TIMEOUT_MS = 30000u;
  static const uint16_t CLOUD_SERVICE_WAIT_CHECK_PERIOD = 1000u;
  static const uint16_t CLOUD_SERVICE_UNPAUSE_SLEEP_TIME = 10000u;
  static const uint64_t IOTHUB_CONNECT_TIMEOUT_MS = 2u*60u*1000u;
  static const uint64_t CONNECTION_CONTROLLER_DISCONNECTING_TIMEOUT1_MS = 30000u;
  static const uint64_t CONNECTION_CONTROLLER_DISCONNECTING_TIMEOUT2_MS = 60000u;

  /* Constants for automatic, scheduled reboots */
  static const uint32_t REBOOT_SCHEDULER_MAX_UPTIME_S = 60u*60u*24u;

  static const uint32_t REBOOT_SCHEDULER_PREFERRED_TIME_SPAN_START_HOURS = 0u;
  static const uint32_t REBOOT_SCHEDULER_PREFERRED_TIME_SPAN_START_MINUTES = 0u;
  static const uint32_t REBOOT_SCHEDULER_PREFERRED_TIME_SPAN_START_SECONDS = 0u;
  static const uint32_t REBOOT_SCHEDULER_PREFERRED_TIME_SPAN_LENGTH_S = 60u*60u*6u;
  static const uint32_t REBOOT_SCHEDULER_SEQNUM_COEFFICIENT = 199u;
  static const uint32_t REBOOT_SCHEDULER_DATEVALUE_COEFFICIENT = 13u;
  static const uint32_t REBOOT_SCHEDULER_MINIMUM_UPTIME_MS = 900u * 1000u;

  /* FGT/End of line test constants */
  static const char FGT_WIFI_TEST_SSID[];
  static const char FGT_WIFI_TEST_PASSWORD[];
};
