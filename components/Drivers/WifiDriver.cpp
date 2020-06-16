#include <cstring>

#include <esp_event_loop.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_wifi_types.h>
#include <sdkconfig.h>
#include <tcpip_adapter.h>
#include <lwip/api.h>
#include <lwip/sockets.h>
#include <lwip/dns.h>
#include <lwip/ip_addr.h>

#include "Constants.h"
#include "WifiDriver.h"


static const char* LOG_TAG = "WifiDriver";  /* TODO: Remove after testing? */


WifiDriver::WifiDriver(void)
  : m_isInitialized(false), m_isEventLoopStarted(false), m_isWlanConnected(false)
{
  /* Must set this initially as no connecting event is ongoing */
  m_eventFlags.set(CONNECTING_EVENT_FINISHED);
}


bool WifiDriver::resetAndInit()
{
  reset();
  return init();
}


void WifiDriver::reset()
{
  if (m_isInitialized)
  {
    disconnectFromWlan();
    esp_wifi_stop();
    esp_wifi_deinit();
    m_isInitialized = false;
  }
}


bool WifiDriver::init(void)
{
  if (!m_isInitialized)
  {
    esp_err_t commandResult;

    if (!m_isEventLoopStarted)
    {
      commandResult = esp_event_loop_init(WifiDriver::eventHandler, this);
    }
    else
    {
      esp_event_loop_set_cb(WifiDriver::eventHandler, this);
      commandResult = ESP_OK;
    }

    if (commandResult != ESP_OK)
    {
      ESP_LOGE(LOG_TAG, "Failed to initialize Wifi event loop (error %d)", commandResult);
    }
    else
    {
      tcpip_adapter_init();
      m_isEventLoopStarted = true;

      wifi_init_config_t wifiConfig = WIFI_INIT_CONFIG_DEFAULT();
      commandResult = esp_wifi_init(&wifiConfig);

      if (commandResult != ESP_OK)
      {
        ESP_LOGE(LOG_TAG, "Failed to initialize Wifi environment (error %d)", commandResult);
      }
      else
      {
        esp_wifi_set_storage(WIFI_STORAGE_RAM);
        commandResult = esp_wifi_set_mode(WIFI_MODE_STA);

        if (commandResult != ESP_OK)
        {
          ESP_LOGE(LOG_TAG, "Failed to set Wifi mode(error %d)", commandResult);
        }
        else
        {
          m_isInitialized = true;
        }
      }
    }
  }

  return m_isInitialized;
}


bool WifiDriver::configureConnection(const char ssID[], const char password[]) const
{
  bool configResult = false;

  if (m_isInitialized)
  {
    ESP_LOGI(LOG_TAG, "Configure wifiConn - SSID from settings is '%s', password is %u chars", ssID, strlen(password));
    wifi_config_t wlanConfig;
    memset(&wlanConfig, 0, sizeof(wlanConfig));
    wlanConfig.sta.bssid_set = false;
    memcpy(wlanConfig.sta.ssid, ssID, strnlen(ssID, sizeof(wlanConfig.sta.ssid)));
    memcpy(wlanConfig.sta.password, password, strnlen(password, sizeof(wlanConfig.sta.password)));

    ESP_LOGI(LOG_TAG, "Configure wifiConn - SSID on configStruct is '%s', password is %u chars",
        reinterpret_cast<char*>(wlanConfig.sta.ssid),
        strnlen(reinterpret_cast<char*>(wlanConfig.sta.password), sizeof(wlanConfig.sta.password)));

    esp_err_t returnCode = esp_wifi_set_config(ESP_IF_WIFI_STA, &wlanConfig);

    if (returnCode == ESP_OK)
    {
      esp_wifi_stop(); // Ensures restart if we are planning a reconnect. Might be required to avoid some 201 errors
      returnCode = esp_wifi_start();

      if (returnCode == ESP_OK)
      {
        configResult = true;
      }
      else
      {
        ESP_LOGE(LOG_TAG, "Failed to start Wifi config (error %d)", returnCode);
      }
    }
    else
    {
      ESP_LOGE(LOG_TAG, "Failed to set Wifi config (error %d)", returnCode);
    }
  }

  return configResult;
}

bool WifiDriver::configureDynamicIp(void)
{
  esp_err_t espResult;
  bool result = false;

  tcpip_adapter_dhcp_status_t status;
  espResult = tcpip_adapter_dhcpc_get_status(TCPIP_ADAPTER_IF_STA, &status);

  if ( status != TCPIP_ADAPTER_DHCP_STARTED )
  {
    espResult = tcpip_adapter_dhcpc_start(TCPIP_ADAPTER_IF_STA);
    if ( espResult == ESP_OK )
    {
      result = true;
    }
  }
  else
  {
    // say we succeeded if dhcp client was already started
    result = true;
    ESP_LOGW(LOG_TAG, "Dhcp client was already started");
  }

  return result;
}

bool WifiDriver::configureStaticIp(uint32_t staticIp,
                                   uint32_t staticNetmask,
                                   uint32_t staticGateway,
                                   uint32_t staticDns1,
                                   uint32_t staticDns2)
{


  tcpip_adapter_ip_info_t ipInfo;

  // First part of dotted quad (a.b.c.d) is assumed to be at the lowest memory address
  // I guess we could call that big endian.
  ipInfo.ip.addr = staticIp;
  ipInfo.gw.addr = staticGateway;
  ipInfo.netmask.addr = staticNetmask;

  ESP_LOGI(LOG_TAG, "Static ip config: ip %hu.%hu.%hu.%hu"
      " gw %hu.%hu.%hu.%hu"
      " netmask %hu.%hu.%hu.%hu", IP2STR(&ipInfo.ip), IP2STR(&ipInfo.gw), IP2STR(&ipInfo.netmask));

  esp_err_t returnCode = tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA);
  if ( returnCode == ESP_OK || returnCode == ESP_ERR_TCPIP_ADAPTER_DHCP_ALREADY_STOPPED )
  {
    returnCode = tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);
    if ( returnCode == ESP_OK )
    {
      ip_addr_t dnsIp;
      dnsIp.type = IPADDR_TYPE_V4;
      dnsIp.u_addr.ip4.addr = staticDns1;
      dns_setserver(0, &dnsIp);

      dnsIp.u_addr.ip4.addr = staticDns2;
      dns_setserver(1, &dnsIp);
    }
    else
    {
      ESP_LOGE(LOG_TAG, "Failed to set static ip, netmask and gateway");
    }
  }
  else
  {
    ESP_LOGE(LOG_TAG, "Fail to stop dhcp client");
  }

  return true;
}

bool WifiDriver::connectToWlan()
{
  if (m_isInitialized && !m_isWlanConnected)
  {
    m_eventFlags.clear(CONNECTING_EVENT_FINISHED);
    esp_err_t connectCode = esp_wifi_connect();

    if (connectCode == ESP_OK)
    {
      /* Wait for connection - m_isWlanConnected is updated in event handler */
      ESP_LOGI(LOG_TAG, "Connecting to Wifi, awaiting response ...");
      bool connectTimedOut = !(m_eventFlags.waitForSet(CONNECTING_EVENT_FINISHED, Constants::WIFI_CONNECTING_TIMEOUT_MS));

      if (connectTimedOut)
      {
        ESP_LOGW(LOG_TAG, "Attempt to connect to Wifi timed out.");
        m_eventFlags.set(CONNECTING_EVENT_FINISHED);
      }
    }
    else
    {
      ESP_LOGE(LOG_TAG, "Attempt to connect to Wifi failed, errorCode=%d", connectCode);
      m_eventFlags.set(CONNECTING_EVENT_FINISHED);
    }
  }
  else
  {
    ESP_LOGE(LOG_TAG, "Failed to connect to WLAN, Wifi has not been initialized or Wlan is already connected!");
  }

  return m_isWlanConnected;
}

void WifiDriver::disconnectFromWlan(void)
{
  if (m_isWlanConnected)
  {
    m_eventFlags.clear(CONNECTING_EVENT_FINISHED);
    esp_err_t disconnectResult = esp_wifi_disconnect();

    if (disconnectResult == ESP_OK)
    {
      /* Wait for disconnection - m_isWlanConnected updated in eventHandler */
      bool disconnectTimedOut = !(m_eventFlags.waitForSet(CONNECTING_EVENT_FINISHED, Constants::WIFI_CONNECTING_TIMEOUT_MS));

      if (disconnectTimedOut)
      {
        ESP_LOGI(LOG_TAG, "Attempt to disconnect from Wifi timed out.");
        m_eventFlags.set(CONNECTING_EVENT_FINISHED);
      }

    }
    else
    {
      /* Disconnection failed, i.e. we weren't connected at all for some reason */
      m_isWlanConnected = false;
      m_eventFlags.set(CONNECTING_EVENT_FINISHED);
    }
  }
}


bool WifiDriver::isConnectedToWlan(void) const
{
  return m_isWlanConnected;
}


esp_err_t WifiDriver::eventHandler(void * context, system_event_t * event)
{
  esp_err_t returnCode = ESP_OK;
  WifiDriver *wifiDriverPtr = static_cast<WifiDriver *>(context);

  if (wifiDriverPtr != nullptr)
  {
    switch (event->event_id)
    {
    case SYSTEM_EVENT_STA_CONNECTED:
      /* Connected to AP. Do nothing here - wait until we get IP. */
      ESP_LOGI(LOG_TAG, "Connected to AP. AuthMode: %d", event->event_info.connected.authmode);
      break;

    case SYSTEM_EVENT_STA_GOT_IP:
      /* Received IP address. Good to go. */
      ESP_LOGI(LOG_TAG, "Connected to WLAN. IP: %d.%d.%d.%d", IP2STR(&event->event_info.got_ip.ip_info.ip));
      wifiDriverPtr->m_isWlanConnected = true;
      wifiDriverPtr->m_eventFlags.set(CONNECTING_EVENT_FINISHED);
      break;

    case SYSTEM_EVENT_STA_DISCONNECTED:
      /* Typically happens is we lose a couple of AP beacons in a row. Connections is marked invalid at this point. */
      /* TODO: Consider signalling other components when disconnected from AP, as all connections must be reset. */
      ESP_LOGW(LOG_TAG, "Disconnected from AP. Reason: %d", event->event_info.disconnected.reason);
      wifiDriverPtr->m_isWlanConnected = false;
      wifiDriverPtr->m_eventFlags.set(CONNECTING_EVENT_FINISHED);
      break;

    case SYSTEM_EVENT_STA_LOST_IP:
      /* Typically happens after being 'DISCONNECTED' for a couple of minutes. */
      ESP_LOGW(LOG_TAG, "Disconnected from WLAN (lost IP)");
      wifiDriverPtr->m_isWlanConnected = false;
      break;

    case SYSTEM_EVENT_STA_START:
    case SYSTEM_EVENT_STA_STOP:
      /* Ignore */
      break;

    default:
      ESP_LOGW(LOG_TAG, "WifiDriver eventHandler called, unhandled event ID %d", event->event_id);
      break;
    }
  }
  else
  {
    ESP_LOGE(LOG_TAG, "WifiDriver::eventHandler context nullptr");
    returnCode = ESP_FAIL;
  }

  return returnCode;
}

