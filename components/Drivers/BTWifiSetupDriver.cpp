#include <esp_log.h>
#include <sdkconfig.h>
#include <cstring>
#include <algorithm>

#include "BTWifiSetupDriver.h"
#include "Constants.h"

namespace
{
  const char LOG_TAG[] = "BTWifiSetupDriver";

  const uint16_t BLE_APP_ID = 0x55u;

  // These must be kept in sync with the calls to addCharacteristic in startGattService()
  enum AttributeIndices
  {
    SSID=2,
    PASSWORD=4,
    STATIC_IP=6,
    STATIC_NETMASK=8,
    STATIC_GATEWAY=10,
    STATIC_DNS1=12,
    STATIC_DNS2=14,
    PROXY_SERVER=16,
    PROXY_PORT=18,
    PROXY_USERNAME=20,
    PROXY_PASSWORD=22,
    CONNECTION_STATUS=24,
    CONNECTION_STATUS_STRING=27,
    CONNECTION_TYPE=30,
    DHCP_STATIC_IP=32,
    DHCP_MANUAL_PROXY=34,
	SW_VER=36,
    EOLT=38,
    TEST=40,
  };

  const size_t ADVERTISING_NAME_SIZE_MAX = 16;

  void generateAdvertisingName(char *advertisingName, const uint8_t *mac)
  {
    // Type
    advertisingName[0] = 'P'; // Panel
    advertisingName[1] = 'H'; // Heater

    // Address
    advertisingName[2] = ((mac[4]>>4) % 0xa) + '0';
    advertisingName[3] = ((mac[4]&0x0f) % 0xa) + '0';
    advertisingName[4] = ((mac[5]>>4) % 0xa) + '0';
    advertisingName[5] = ((mac[5]&0x0f) % 0xa) + '0';

    // Brand
    advertisingName[6]  = '<';
    advertisingName[7]  = 'D';
    advertisingName[8]  = 'i';
    advertisingName[9]  = 'm';
    advertisingName[10] = 'p';
    advertisingName[11] = 'l';
    advertisingName[12] = 'e';
    advertisingName[13] = 'x';
    advertisingName[14] = '>';
    advertisingName[15] = '\0';
  }

  bool validConnectionStatus(WifiConnectionStatus::Enum connectionStatus)
  {
    switch ( connectionStatus )
    {
    case WifiConnectionStatus::IN_PROGRESS:
    case WifiConnectionStatus::INTERNET_CONNECTION_ESTABLISHED:
    case WifiConnectionStatus::FULLY_FUNCTIONAL:
    case WifiConnectionStatus::DISCOVERY_AMQP_CONNECTION_REJECTED:
    case WifiConnectionStatus::DISCOVERY_AMQP_SERVICE_SERVER_INACCESSIBLE:
    case WifiConnectionStatus::DISCOVERY_SERVICE_DEVICE_REJECTED:
    case WifiConnectionStatus::FW_AUTH_SECURE_BOOT_FAILURE:
    case WifiConnectionStatus::HW_FAILURE_DEADLOCK:
    case WifiConnectionStatus::DHCP_FAILURE:
    case WifiConnectionStatus::DNS_HOST_RESOLUTION_FAILURE:
    case WifiConnectionStatus::ETHERNET_SETTINGS_INVALID:
    case WifiConnectionStatus::WIFI_NOT_DISCOVERED:
    case WifiConnectionStatus::WIFI_PASSWORD_INVALID:
      return true;
    default:
      return false;
    }
  }
}

BTWifiSetupDriver::BTWifiSetupDriver(BTDriverInterface &btDriver, uint32_t btPasskey, Settings &settings, SoftwareResetDriverInterface &swResetDriver):
  m_connectionType(0u),
  m_staticIp(0u),
  m_staticNetmask(0u),
  m_staticGateway(0u),
  m_staticDns1(0u),
  m_staticDns2(0u),
  m_proxyServer(0u),
  m_proxyPort(0u),
  m_connectionStatus(0u),
  m_dhcpStaticIp(false),
  m_dhcpManualProxy(false),
  m_btDriver(btDriver),
  m_btPasskey(btPasskey),
  m_settings(settings),
  m_swResetDriver(swResetDriver)
{
  reset();
}

bool BTWifiSetupDriver::startGattService()
{
  bool success = false;
  const bool encrypted = true;

  success = m_btDriver.resetAndInit();
  if ( !success )
  {
    ESP_LOGE(LOG_TAG, "Failed to resetAndInit bt driver");
    return false;
  }

  success = m_btDriver.resetGattDb();
  if ( !success )
  {
    return false;
  }

  success = m_btDriver.addService(Constants::WIFI_SETUP_UUID, sizeof(Constants::WIFI_SETUP_UUID));
  if ( !success )
  {
    return false;
  }

  success = m_btDriver.addCharacteristic(reinterpret_cast<uint8_t *>(m_ssid),
                                         sizeof(m_ssid),
                                         Constants::SSID_UUID,
                                         sizeof(Constants::SSID_UUID),
                                         true, true, false, nullptr, nullptr, encrypted);
  if ( !success )
  {
    return false;
  }

  success = m_btDriver.addCharacteristic(reinterpret_cast<uint8_t *>(m_password),
                                         sizeof(m_password),
                                         Constants::PASSWORD_UUID,
                                         sizeof(Constants::PASSWORD_UUID),
                                         false, true, false, nullptr, nullptr, encrypted);
  if ( !success )
  {
    return false;
  }

  success = m_btDriver.addCharacteristic(reinterpret_cast<uint8_t *>(&m_staticIp),
                                         sizeof(m_staticIp),
                                         Constants::STATIC_IP_UUID,
                                         sizeof(Constants::STATIC_IP_UUID),
                                         true, true, false, nullptr, nullptr, encrypted);
  if ( !success )
  {
    return false;
  }

  success = m_btDriver.addCharacteristic(reinterpret_cast<uint8_t *>(&m_staticNetmask),
                                         sizeof(m_staticNetmask),
                                         Constants::STATIC_NETMASK_UUID,
                                         sizeof(Constants::STATIC_NETMASK_UUID),
                                         true, true, false, nullptr, nullptr, encrypted);
  if ( !success )
  {
    return false;
  }

  success = m_btDriver.addCharacteristic(reinterpret_cast<uint8_t *>(&m_staticGateway),
                                         sizeof(m_staticGateway),
                                         Constants::STATIC_GATEWAY_UUID,
                                         sizeof(Constants::STATIC_GATEWAY_UUID),
                                         true, true, false, nullptr, nullptr, encrypted);
  if ( !success )
  {
    return false;
  }

  success = m_btDriver.addCharacteristic(reinterpret_cast<uint8_t *>(&m_staticDns1),
                                         sizeof(m_staticDns1),
                                         Constants::STATIC_DNS1_UUID,
                                         sizeof(Constants::STATIC_DNS1_UUID),
                                         true, true, false, nullptr, nullptr, encrypted);
  if ( !success )
  {
    return false;
  }

  success = m_btDriver.addCharacteristic(reinterpret_cast<uint8_t *>(&m_staticDns2),
                                         sizeof(m_staticDns2),
                                         Constants::STATIC_DNS2_UUID,
                                         sizeof(Constants::STATIC_DNS2_UUID),
                                         true, true, false, nullptr, nullptr, encrypted);
  if ( !success )
  {
    return false;
  }

  success = m_btDriver.addCharacteristic(reinterpret_cast<uint8_t *>(&m_proxyServer),
                                         sizeof(m_proxyServer),
                                         Constants::PROXY_SERVER_UUID,
                                         sizeof(Constants::PROXY_SERVER_UUID),
                                         true, true, false, nullptr, nullptr, encrypted);
  if ( !success )
  {
    return false;
  }

  success = m_btDriver.addCharacteristic(reinterpret_cast<uint8_t *>(&m_proxyPort),
                                         sizeof(m_proxyPort),
                                         Constants::PROXY_PORT_UUID,
                                         sizeof(Constants::PROXY_PORT_UUID),
                                         true, true, false, nullptr, nullptr, encrypted);
  if ( !success )
  {
    return false;
  }

  success = m_btDriver.addCharacteristic(reinterpret_cast<uint8_t *>(m_proxyUsername),
                                         sizeof(m_proxyUsername),
                                         Constants::PROXY_USERNAME_UUID,
                                         sizeof(Constants::PROXY_USERNAME_UUID),
                                         true, true, false, nullptr, nullptr, encrypted);
  if ( !success )
  {
    return false;
  }

  success = m_btDriver.addCharacteristic(reinterpret_cast<uint8_t *>(m_proxyPassword),
                                         sizeof(m_proxyPassword),
                                         Constants::PROXY_PASSWORD_UUID,
                                         sizeof(Constants::PROXY_PASSWORD_UUID),
                                         false, true, false, nullptr, nullptr, encrypted);
  if ( !success )
  {
    return false;
  }

  m_connectionStatusCc = 0;
  success = m_btDriver.addCharacteristic(reinterpret_cast<uint8_t *>(&m_connectionStatus),
                                         sizeof(m_connectionStatus),
                                         Constants::CONNECTION_STATUS_UUID,
                                         sizeof(Constants::CONNECTION_STATUS_UUID),
                                         true, true, true, &m_connectionStatusCc, nullptr, encrypted);
  if ( !success )
  {
    return false;
  }

  m_connectionStatusStringCc = 0;
  success = m_btDriver.addCharacteristic(reinterpret_cast<uint8_t *>(m_connectionStatusString),
                                         sizeof(m_connectionStatusString),
                                         Constants::CONNECTION_STATUS_STRING_UUID,
                                         sizeof(Constants::CONNECTION_STATUS_STRING_UUID),
                                         true, true, true, &m_connectionStatusStringCc, nullptr, encrypted);
  if ( !success )
  {
    return false;
  }

  success = m_btDriver.addCharacteristic(reinterpret_cast<uint8_t *>(&m_connectionType),
                                         sizeof(m_connectionType),
                                         Constants::CONNECTION_TYPE_UUID,
                                         sizeof(Constants::CONNECTION_TYPE_UUID),
                                         true, true, false, nullptr, nullptr, encrypted);
  if ( !success )
  {
    return false;
  }

  success = m_btDriver.addCharacteristic(reinterpret_cast<uint8_t *>(&m_dhcpStaticIp),
                                         sizeof(m_dhcpStaticIp),
                                         Constants::DHCP_STATIC_IP_UUID,
                                         sizeof(Constants::DHCP_STATIC_IP_UUID),
                                         true, false, false, nullptr, nullptr, encrypted);
  if ( !success )
  {
    return false;
  }

  GattAttributeIndex indexOfDhcpManualProxyCharacteristic;
  success = m_btDriver.addCharacteristic(reinterpret_cast<uint8_t *>(&m_dhcpManualProxy),
                                         sizeof(m_dhcpManualProxy),
                                         Constants::DHCP_MANUAL_PROXY_UUID,
                                         sizeof(Constants::DHCP_MANUAL_PROXY_UUID),
                                         true, false, false, nullptr, &indexOfDhcpManualProxyCharacteristic, encrypted);
  if ( indexOfDhcpManualProxyCharacteristic != AttributeIndices::DHCP_MANUAL_PROXY )
  {
    ESP_LOGE(LOG_TAG, "Predefined characteristic index mismatch");
    return false;
  }
  if ( !success )
  {
    return false;
  }

  // Method of returning to the EoLT mode
  // Multiple services does not work
  /*
  success = m_btDriver.addService(Constants::FGT_CONFIG_UUID, sizeof(Constants::FGT_CONFIG_UUID), false);
  if ( !success )
  {
    ESP_LOGE(LOG_TAG, "Failed adding service");
    return false;
  }
   */

  // SW Version
  success = m_btDriver.addCharacteristic(reinterpret_cast<uint8_t *>(m_btValSWVer),
										 sizeof(m_btValSWVer),
										 Constants::SW_VER_UUID,
										 sizeof(Constants::SW_VER_UUID),
										 true, true, false, nullptr, nullptr, encrypted);
  if ( !success )
  {
    return false;
  }

  // EoLT State
  success = m_btDriver.addCharacteristic(reinterpret_cast<uint8_t *>(&m_btValEOLT),
                                          sizeof(m_btValEOLT),
                                          Constants::EOLT_UUID,
                                          sizeof(Constants::EOLT_UUID),
                                          true, true, false, nullptr, nullptr, encrypted);
  if ( !success )
  {
    return false;
  }

  GattAttributeIndex indexOfTestModeCharacteristic;
  // Test Mode
  success = m_btDriver.addCharacteristic(reinterpret_cast<uint8_t *>(&m_btValTEST),
                                          sizeof(m_btValTEST),
                                          Constants::TEST_MODE_UUID,
                                          sizeof(Constants::TEST_MODE_UUID),
                                          true, true, false, nullptr, &indexOfTestModeCharacteristic, encrypted);
  if ( indexOfTestModeCharacteristic != AttributeIndices::TEST )
  {
    ESP_LOGE(LOG_TAG, "Predefined FGT characteristic index mismatch");
    return false;
  }

  if ( !success )
  {
    return false;
  }

  char advertisingName[ADVERTISING_NAME_SIZE_MAX] = {};
  uint8_t mac[6] = {};
  if ( !m_btDriver.getBleMac(mac) )
  {
    ESP_LOGE(LOG_TAG, "Failed to generate advertising name (MAC)");
    return false;
  }

  generateAdvertisingName(advertisingName, mac);
  ESP_LOGI(LOG_TAG, "Using BLE MAC: %02x:%02x:%02x:%02x:%02x:%02x and advertising name: %s\n",
    static_cast<uint32_t>(mac[0]),
    static_cast<uint32_t>(mac[1]),
    static_cast<uint32_t>(mac[2]),
    static_cast<uint32_t>(mac[3]),
    static_cast<uint32_t>(mac[4]),
    static_cast<uint32_t>(mac[5]),
    advertisingName);

  success = m_btDriver.startBleApp(BLE_APP_ID, advertisingName, this, m_btPasskey);
  if ( !success )
  {
    ESP_LOGE(LOG_TAG, "Failed to start GATT service");
  }

  return success;
}

void BTWifiSetupDriver::stopGattService()
{
  m_btDriver.stopBleApp();
}

bool BTWifiSetupDriver::isGattServiceRunning()
{
  // NOTE: doesn't really verify that the WifiSetup Gatt service is running,
  // only that _some_ gatt service is running...
  return m_btDriver.isServiceRunning();
}

bool BTWifiSetupDriver::setConnectionStatus(WifiConnectionStatus::Enum connectionStatus,
                                            const char *connectionStatusString)
{
  bool success = false;

  if ( !validConnectionStatus(connectionStatus) )
  {
    ESP_LOGE(LOG_TAG, "Trying to set invalid connection status");
    return false;
  }

  if ( !validConnectionStatusString(connectionStatusString) )
  {
    ESP_LOGE(LOG_TAG, "Trying to set invalid connection status string");
    return false;
  }

  m_connectionStatus = connectionStatus;

  strncpy(m_connectionStatusString,
          connectionStatusString,
          Constants::CONNECTION_STATUS_STRING_SIZE_MAX-1);
  m_connectionStatusString[Constants::CONNECTION_STATUS_STRING_SIZE_MAX-1] = 0;

  if ( m_btDriver.isConnected() )
  {
    success = m_btDriver.writeCharacteristicValue(AttributeIndices::CONNECTION_STATUS,
                                                  reinterpret_cast<uint8_t *>(&connectionStatus),
                                                  sizeof(connectionStatus));
    if ( !success )
    {
      return false;
    }

    success = m_btDriver.sendCharacteristicNotification(AttributeIndices::CONNECTION_STATUS,
                                                        reinterpret_cast<uint8_t *>(&connectionStatus),
                                                        sizeof(connectionStatus));
    if ( !success )
    {
      ESP_LOGE(LOG_TAG, "Failed to send connectionStatus notification");
      // (don't abort operation)
    }

    success = m_btDriver.sendCharacteristicNotification(AttributeIndices::CONNECTION_STATUS_STRING,
                                                        reinterpret_cast<const uint8_t *>(m_connectionStatusString),
                                                        sizeof(m_connectionStatusString));
    if ( !success )
    {
      ESP_LOGE(LOG_TAG, "Failed to send connectionStatus notification");
      // (don't abort operation)
    }

    success = m_btDriver.writeCharacteristicValue(AttributeIndices::CONNECTION_STATUS_STRING,
                                                  reinterpret_cast<const uint8_t *>(m_connectionStatusString),
                                                  sizeof(m_connectionStatusString));
    if ( !success )
    {
      return false;
    }
  }

  return success;
}

void BTWifiSetupDriver::reset()
{
  memset(m_ssid, 0, sizeof(m_ssid));
  memset(m_password, 0, sizeof(m_password));
  m_staticIp = 0;
  m_staticNetmask = 0;
  m_staticGateway = 0;
  m_staticDns1 = 0;
  m_staticDns2 = 0;
  m_proxyServer = 0;
  m_proxyPort = 0;
  memset(m_proxyUsername, 0, sizeof(m_proxyUsername));
  memset(m_proxyPassword, 0, sizeof(m_proxyPassword));
  m_connectionStatus = WifiConnectionStatus::IN_PROGRESS;
  memset(m_connectionStatusString, 0, sizeof(m_connectionStatusString));
  m_connectionType = 0;
  m_dhcpStaticIp = false;
  m_dhcpManualProxy = false;
  m_btValTEST = 0;
  m_btValEOLT = m_settings.getRunMode() == RunMode::test ? 0x00 : Constants::FGT_EOLT_COMPLETE;
  m_btValSWVer[0] = 1;
  m_btValSWVer[1] = Constants::SW_VERSION / 10u;
  m_btValSWVer[2] = Constants::SW_VERSION % 10u;
}

bool BTWifiSetupDriver::validConnectionStatusString(const char *str) const
{
  uint32_t i;

  for ( i=0; str[i] != 0 && i < sizeof(m_connectionStatusString); i++ );

  if ( i >= sizeof(m_connectionStatusString) )
  {
    return false;
  }

  /* potentially check against preset status strings - not sure how this attribute will be used yet */

  return true;
}

void BTWifiSetupDriver::handleCharacteristicValueChanged(GattAttributeIndex attributeIndex,
                                                         uint16_t size, // TODO, might have to include offset param as well
                                                         uint8_t *value)
{
  // TODO write/mirror all values in member variables accessible via getters
  char dbgValue[Constants::WIFI_PASSWORD_SIZE_MAX] = {};
  std::size_t memberMaxSize = 0U;

  switch ( attributeIndex )
  {
  case AttributeIndices::SSID:
    memberMaxSize = Constants::WIFI_SSID_SIZE_MAX;
    memset(m_ssid, 0, memberMaxSize);
    memcpy(m_ssid, value, std::min(static_cast<std::size_t>(size), memberMaxSize));
    m_ssid[Constants::WIFI_SSID_SIZE_MAX-1] = 0;

    ESP_LOGI(LOG_TAG, "SSID value with size %u received: '%s'", static_cast<uint32_t>(size), m_ssid);
    m_eventFlags.set(SSID_RECEIVED_EVENT_ID);
    break;

  case AttributeIndices::PASSWORD:
    memberMaxSize = Constants::WIFI_PASSWORD_SIZE_MAX;
    memset(m_password, 0, memberMaxSize);
    memcpy(m_password, value, std::min(static_cast<std::size_t>(size), memberMaxSize));
    m_password[Constants::WIFI_PASSWORD_SIZE_MAX-1] = 0;
    memset(dbgValue, '*', strlen(m_password));

    ESP_LOGI(LOG_TAG, "Password value with size %u received: '%s'", static_cast<uint32_t>(size), dbgValue);
    m_eventFlags.set(PASSWORD_RECEIVED_EVENT_ID);
    break;

  case AttributeIndices::STATIC_IP:
    memberMaxSize = sizeof(m_staticIp);

    // Assumption: static ip is received as "big endian first"
    // i.e. first byte of dotted quad has the lowest address

    if ( size == memberMaxSize )
    {
      memcpy(&m_staticIp, value, size);

      ESP_LOGI(LOG_TAG, "Static ip received: (0x%.8x) %u.%u.%u.%u",
        m_staticIp,
        static_cast<uint32_t>(m_staticIp & 0x000000ff),
        static_cast<uint32_t>((m_staticIp & 0x0000ff00) >> 8),
        static_cast<uint32_t>((m_staticIp & 0x00ff0000) >> 16),
        static_cast<uint32_t>(m_staticIp >> 24));
    }
    else
    {
      ESP_LOGW(LOG_TAG, "Received %hu bytes for static ip, ignoring", size);
    }
    break;



  case AttributeIndices::STATIC_NETMASK:
    memberMaxSize = sizeof(m_staticNetmask);

    if ( size == memberMaxSize )
    {
      memcpy(&m_staticNetmask, value, size);

      ESP_LOGI(LOG_TAG, "Netmask received: (0x%.8x) %u.%u.%u.%u",
        m_staticNetmask,
        static_cast<uint32_t>(m_staticNetmask & 0x000000ff),
        static_cast<uint32_t>((m_staticNetmask & 0x0000ff00) >> 8),
        static_cast<uint32_t>((m_staticNetmask & 0x00ff0000) >> 16),
        static_cast<uint32_t>(m_staticNetmask >> 24));
    }
    else
    {
      ESP_LOGW(LOG_TAG, "Received %hu bytes for netmask, ignoring", size);
    }


    break;

  case AttributeIndices::STATIC_GATEWAY:
    memberMaxSize = sizeof(m_staticGateway);

    if ( size == memberMaxSize )
    {
      memcpy(&m_staticGateway, value, size);

      ESP_LOGI(LOG_TAG, "Gateway ip received: (0x%.8x) %u.%u.%u.%u",
        m_staticGateway,
        static_cast<uint32_t>(m_staticGateway & 0x000000ff),
        static_cast<uint32_t>((m_staticGateway & 0x0000ff00) >> 8),
        static_cast<uint32_t>((m_staticGateway & 0x00ff0000) >> 16),
        static_cast<uint32_t>(m_staticGateway >> 24));
    }
    else
    {
      ESP_LOGW(LOG_TAG, "Received %hu bytes for gateway ip, ignoring", size);
    }


    break;

  case STATIC_DNS1:
    memberMaxSize = sizeof(m_staticDns1);

    if ( size == memberMaxSize )
    {
      memcpy(&m_staticDns1, value, size);

      ESP_LOGI(LOG_TAG, "DNS1 ip received: (0x%.8x) %u.%u.%u.%u",
          m_staticDns1,
        static_cast<uint32_t>(m_staticDns1 & 0x000000ff),
        static_cast<uint32_t>((m_staticDns1 & 0x0000ff00) >> 8),
        static_cast<uint32_t>((m_staticDns1 & 0x00ff0000) >> 16),
        static_cast<uint32_t>(m_staticDns1 >> 24));
    }
    else
    {
      ESP_LOGW(LOG_TAG, "Received %hu bytes for DNS1, ignoring", size);
    }

    break;
  case AttributeIndices::STATIC_DNS2:
    memberMaxSize = sizeof(m_staticDns2);

    if ( size == memberMaxSize )
    {
      memcpy(&m_staticDns2, value, size);

      ESP_LOGI(LOG_TAG, "DNS2 ip received: (0x%.8x) %u.%u.%u.%u",
          m_staticDns2,
        static_cast<uint32_t>(m_staticDns2 & 0x000000ff),
        static_cast<uint32_t>((m_staticDns2 & 0x0000ff00) >> 8),
        static_cast<uint32_t>((m_staticDns2 & 0x00ff0000) >> 16),
        static_cast<uint32_t>(m_staticDns2 >> 24));
    }
    else
    {
      ESP_LOGW(LOG_TAG, "Received %hu bytes for DNS2, ignoring", size);
    }

    break;

  case AttributeIndices::CONNECTION_TYPE:
    memberMaxSize = sizeof(m_connectionType);
    memcpy(&m_connectionType, value, std::min(static_cast<std::size_t>(size), memberMaxSize));

    ESP_LOGI(LOG_TAG, "Connection type value received!");
    m_eventFlags.set(CONNECTION_TYPE_RECEIVED_EVENT_ID);
    break;

  case AttributeIndices::DHCP_STATIC_IP:
    memberMaxSize = sizeof(m_dhcpStaticIp);

    if ( size == memberMaxSize )
    {
      memcpy(&m_dhcpStaticIp, value, memberMaxSize);
      ESP_LOGI(LOG_TAG, "dhcpStaticIp boolean received: %u",
          static_cast<uint32_t>(m_dhcpStaticIp));
    }
    else
    {
      ESP_LOGE(LOG_TAG, "Received %hu bytes for DHCP-STATIC-IP, ignoring", size);
    }
    break;


  case AttributeIndices::CONNECTION_STATUS:
    memberMaxSize = sizeof(m_connectionStatus);
    memcpy(&m_connectionStatus, value, std::min(static_cast<std::size_t>(size), memberMaxSize));
    if ( m_connectionStatus == WifiConnectionStatus::CONNECTION_REQUEST )
    {
      m_eventFlags.set(APP_RECONNECT_CMD_RECEIVED_EVENT_ID);
    }
    break;

  case AttributeIndices::SW_VER:
	  m_btDriver.writeCharacteristicValue(AttributeIndices::SW_VER,     reinterpret_cast<uint8_t *>(m_btValSWVer),      Constants::SW_VER_BT_SIZE_MAX);
      break;

  case AttributeIndices::EOLT:
   if (m_btValTEST)
   {
     // Exit test mode?
     if(*value == Constants::FGT_EOLT_COMPLETE)
     {
       m_settings.setRunMode(RunMode::normal);
     }
     else if(*value == Constants::FGT_EOLT_ENTER)
     {
       m_settings.setRunMode(RunMode::test);
     }

     if (m_settings.isModified())
     {
       m_settings.storeSettings();
     }
   }
   m_btValEOLT = m_settings.getRunMode() == RunMode::test ? 0x00 : Constants::FGT_EOLT_COMPLETE;
   m_btDriver.writeCharacteristicValue(AttributeIndices::EOLT,       reinterpret_cast<uint8_t *>(&m_btValEOLT),      Constants::EOLT_BT_SIZE_MAX);
   break;

  case AttributeIndices::TEST:
   // Enable Test mode - "Write enable"
   if(*value)
   {
     m_btValTEST = 1;
     m_btDriver.writeCharacteristicValue(AttributeIndices::TEST,       reinterpret_cast<uint8_t *>(&m_btValTEST),      Constants::TEST_BT_SIZE_MAX);
   }
   // If we exit Test mode - Reset the processor
   else if(m_btValTEST)
   {
     m_swResetDriver.reset();
   }
   break;

  default:
    ESP_LOGI(LOG_TAG, "Unhandled characteristic value changed: %u",
             static_cast<uint32_t>(attributeIndex));
    break;
  }

  // TODO forward a single event like "all attributes set" or something and
  // include all settings as parameters (or have them queried using getters)
}


bool BTWifiSetupDriver::waitForReconnectCmdReceived(uint32_t maxTime) const
{
  bool wasReceived = m_eventFlags.waitForSet(APP_RECONNECT_CMD_RECEIVED_EVENT_ID, maxTime);

  return wasReceived;
}


void BTWifiSetupDriver::registerReconnectCmdConsumed(void)
{
  /* Clear flags for all relevant WifiSettings (see "waitForSet..." in waitForWifiSettingsReceived */
  m_eventFlags.clear(APP_RECONNECT_CMD_RECEIVED_EVENT_ID);
}


void BTWifiSetupDriver::getWifiSSID(char * ssid, uint32_t size)
{
  strncpy(ssid, m_ssid, size);
  ssid[size - 1U] = '\0';
}


void BTWifiSetupDriver::getWifiPassword(char * password, uint32_t size)
{
  strncpy(password, m_password, size);
  password[size - 1U] = '\0';
}


uint8_t BTWifiSetupDriver::getWifiConnectionType(void)
{
  return m_connectionType;
}

uint32_t BTWifiSetupDriver::getStaticIp() const
{
  return m_staticIp;
}

uint32_t BTWifiSetupDriver::getStaticNetmask() const
{
  return m_staticNetmask;
}

uint32_t BTWifiSetupDriver::getStaticGateway() const
{
  return m_staticGateway;
}

uint32_t BTWifiSetupDriver::getStaticDns1() const
{
  return m_staticDns1;
}

uint32_t BTWifiSetupDriver::getStaticDns2() const
{
  return m_staticDns2;
}

void BTWifiSetupDriver::setWifiSSID(char * ssid)
{
  strncpy(m_ssid, ssid, sizeof(m_ssid));
  m_ssid[(sizeof(m_ssid)-1u)] = '\0';
}

void BTWifiSetupDriver::setWifiPassword(char * password)
{
  strncpy(m_password, password, sizeof(m_password));
  m_ssid[(sizeof(m_password)-1u)] = '\0';
}

void BTWifiSetupDriver::setStaticIpInUse()
{
  m_dhcpStaticIp = true;
  updateDhcpStaticIp();
}

void BTWifiSetupDriver::setDynamicIpInUse()
{
  m_dhcpStaticIp = false;
  updateDhcpStaticIp();
}

void BTWifiSetupDriver::updateDhcpStaticIp()
{
  if ( m_btDriver.isConnected() )
  {
    bool success = false;

    success = m_btDriver.writeCharacteristicValue(AttributeIndices::DHCP_STATIC_IP,
                                                  reinterpret_cast<uint8_t *>(&m_dhcpStaticIp),
                                                  sizeof(m_dhcpStaticIp));
  }
}
