
#include "FreeRTOS.hpp"

#include <esp_log.h>
#include <esp_system.h>
#include <esp_bt.h>
#include <esp_bt_device.h>

#include <esp_gap_ble_api.h>
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"

#include <cstring>
#include <limits>

#include "BTDriver.h"

enum
{
  ADVERTISING_NOT_CONFIGURED = 0,
  ADVERTISING_DATA_SET = (1<<0),
  ADVERTISING_RESPONSE_DATA_SET = (1<<1),
  ADVERTISING_ACTIVE = (1<<2)
};

enum
{
  CLIENT_CONFIG_SIZE=2
};

// Extensions to Bluedroid for static passkey

extern "C"
{
  static uint32_t bt_static_passkey;
  static uint8_t bt_static_passkey_enable;
  void bt_ext_enable_static_passkey(uint32_t static_passkey){
  	bt_static_passkey = static_passkey;
	bt_static_passkey_enable = 1;
  }
  void bt_ext_disable_static_passkey(void){
	bt_static_passkey_enable = 0;
  }
}
namespace
{
  const char LOG_TAG[] = "BTDriver";

  const size_t CHARACTERISTIC_DECLARATION_SIZE { sizeof(uint8_t) };

  BTDriver *s_driver = nullptr;

  // BLE security configuration (pretend these are declared const)
  esp_ble_auth_req_t BLE_SECURITY_AUTH_REQ_MODE = ESP_LE_AUTH_REQ_SC_MITM;
  esp_ble_io_cap_t BLE_SECURITY_IO_CAP = ESP_IO_CAP_OUT; // We have a "display" to show a randomly generated passcode. But we force a static one.
  uint8_t BLE_SECURITY_MAX_KEY_SIZE = 16;
  uint8_t BLE_SECURITY_INITIATOR_KEYS = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
  uint8_t BLE_SECURITY_RESPONDER_KEYS = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;


  esp_ble_adv_data_t s_advertisingConfig =
  {
      false, // configure advertising(false) or scan response(true)?
	  true, // include device name?
      true, // include_txpower
      0x20, // min interval
      0x40, // max interval
      0x0, // appearance..?
      0, // manufacturer_len
      nullptr, // p_manufacturer_data
      0, // service_data_len
      nullptr, // p_service_data
      0, // sevice_uuid_len
      nullptr, // p_service_uuid (this was not null in examples)
      ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT// flag
  };

  esp_ble_adv_data_t s_scanResponseConfig =
  {
      true, // configure advertising(false) or scan response(true)?
      true, // include device name?
      true, // include_txpower
      0x20, // min interval
      0x40, // max interval
      0x0, // appearance..?
      0, // manufacturer_len
      nullptr, // p_manufacturer_data
      0, // service_data_len
      nullptr, // p_service_data
      0, // sevice_uuid_len
      nullptr, // p_service_uuid (this was not null in examples)
      ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT// flag
  };

  esp_ble_adv_params_t s_advertisingParams;

  // These should be const, but underlying API (esp_bt) does not use const.
  uint16_t PRIMARY_SERVICE_UUID = ESP_GATT_UUID_PRI_SERVICE;
  uint16_t CHARACTERISTIC_DECLARATION_UUID = ESP_GATT_UUID_CHAR_DECLARE;
  uint16_t CHARACTERISTIC_CLIENT_CONFIG_UUID = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
  uint8_t CHARACTERISTIC_PROP_READ = ESP_GATT_CHAR_PROP_BIT_READ;
  uint8_t CHARACTERISTIC_PROP_WRITE = ESP_GATT_CHAR_PROP_BIT_WRITE;
  uint8_t CHARACTERISTIC_PROP_READ_WRITE = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE;
  uint8_t CHARACTERISTIC_PROP_READ_WRITE_NOTIFY = ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
  uint8_t CHARACTERISTIC_PROP_READ_NOTIFY = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;

  uint8_t *getPermission(bool readable, bool writable, bool notifiable)
  {
    if ( readable )
    {
      if ( writable )
      {
        if ( notifiable )
        {
          return &CHARACTERISTIC_PROP_READ_WRITE_NOTIFY;
        } else
        {
          return &CHARACTERISTIC_PROP_READ_WRITE;
        }
      } else if ( notifiable )
      {
        return &CHARACTERISTIC_PROP_READ_NOTIFY;
      } else
      {
        return &CHARACTERISTIC_PROP_READ;
      }
    } else
    {
      if ( writable )
      {
        return &CHARACTERISTIC_PROP_WRITE;
      } else
      {
        ESP_LOGE(LOG_TAG, "Failed to add characteristic: unsupported combination of permissions");
        return nullptr;
      }
    }
  }
}

BTDriver::BTDriver():
m_driverInitialized(false),
m_serviceRunning(false),
m_appRegistered(false),
m_dbRegistered(false),
m_numDbEntries(0u),
m_numUuids(0u),
m_eventReceiver(nullptr),
m_advertisingStatus(ADVERTISING_NOT_CONFIGURED),
m_gattInterface(0u),
m_connectionId(0u),
m_connected(false)
{
}

bool BTDriver::resetAndInit()
{
  reset();
  return init();
}

bool BTDriver::getBleMac(uint8_t (&dstMac)[Constants::MAC_ADDRESS_SIZE])
{
  if ( !isDriverInitialized() )
  {
    return false;
  }

  const uint8_t *mac = esp_bt_dev_get_address();
  if ( mac == nullptr )
  {
    return false;
  }
  std::memcpy(dstMac, mac, 6);
  return true;
}

bool BTDriver::startBleApp(uint16_t bleAppId,
                           const char *deviceName,
                           BTDriverEventReceiverInterface *eventReceiver,
                           uint32_t staticPasskey)
{
  esp_err_t ret;

  if ( !isDriverInitialized() )
  {
    ESP_LOGE(LOG_TAG, "Failed to start GATT service: BTDriver not initialized");
    return false;
  }

  if ( isServiceRunning() )
  {
    ESP_LOGE(LOG_TAG, "Failed to start GATT service: already running");
    return false;
  }

  if ( eventReceiver == nullptr )
  {
    ESP_LOGE(LOG_TAG, "No event receiver registered");
    return false;
  }

  strncpy(m_deviceName, deviceName, Constants::BT_DEVICE_NAME_SIZE_MAX-1);
  m_deviceName[Constants::BT_DEVICE_NAME_SIZE_MAX-1] = 0;

  m_connected = false;
  m_connectionId = 0;
  m_gattInterface = 0;

  m_eventReceiver = eventReceiver;

  bool success = setSecurityParameters();
  if ( !success )
  {
    ESP_LOGE(LOG_TAG, "Failed to set security parameters");
    return false;
  }

  if ( staticPasskey > 0 )
  {
    bt_ext_enable_static_passkey(staticPasskey);
  } else
  {
    bt_ext_disable_static_passkey();
  }

  ret = esp_ble_gatts_app_register(bleAppId);
  if ( ret != ESP_OK )
  {
      ESP_LOGE(LOG_TAG, "Failed to register GATT app: %d", ret);
      return false;
  }

  return true;
}

void BTDriver::stopBleApp()
{
  esp_err_t err;

  if ( isServiceRunning() )
  {
    err = esp_ble_gatts_stop_service(m_attributeHandles[0]);
    if ( err == ESP_OK )
    {
      setServiceRunning(false);
    } else
    {
      ESP_LOGE(LOG_TAG, "Failed to stop GATT service");
    }
  }

  if ( isAppRegistered() )
  {
    err = esp_ble_gatts_app_unregister(m_gattInterface);
    if ( err == ESP_OK )
    {
      setAppRegistered(false);
    } else
    {
      ESP_LOGE(LOG_TAG, "Failed to unregister GATT app");
    }
  }
}

bool BTDriver::isConnected()
{
  return m_connected;
}

bool BTDriver::isServiceRunning() const
{
  return m_serviceRunning;
}

void BTDriver::staticGapEventHandler(esp_gap_ble_cb_event_t event,
                                     esp_ble_gap_cb_param_t *param)
{
  if ( s_driver != nullptr )
  {
    s_driver->gapEventHandler(event, param);
  } else
  {
    ESP_LOGE(LOG_TAG, "Received GAP event with no driver instance to handle it");
  }
}

void BTDriver::gapEventHandler(esp_gap_ble_cb_event_t event,
                               esp_ble_gap_cb_param_t *param)
{
  switch ( event )
  {
  case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
    m_advertisingStatus |= ADVERTISING_DATA_SET;

    if ( m_advertisingStatus == (ADVERTISING_DATA_SET | ADVERTISING_RESPONSE_DATA_SET) )
    {
      startAdvertising();
    }
    break;
  case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
    m_advertisingStatus |= ADVERTISING_RESPONSE_DATA_SET;

    if ( m_advertisingStatus == (ADVERTISING_DATA_SET | ADVERTISING_RESPONSE_DATA_SET) )
    {
      startAdvertising();
    }
    break;
  case ESP_GAP_BLE_SEC_REQ_EVT:
	  ESP_LOGI(LOG_TAG, "ESP_GAP_BLE_SEC_REQ_EVT received");
	  // Reply to the Pairing request
	  esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
	  break;
  case ESP_GAP_BLE_KEY_EVT:
      ESP_LOGI(LOG_TAG, "ESP_GAP_BLE_KEY_EVT received");
      break;
  case ESP_GAP_BLE_AUTH_CMPL_EVT:
      ESP_LOGI(LOG_TAG, "ESP_GAP_BLE_AUTH_CMPL_EVT received, addr type: %d", param->ble_security.auth_cmpl.addr_type);
      if ( param->ble_security.auth_cmpl.success )
      {
        ESP_LOGI(LOG_TAG, "Authentication successful");
      } else
      {
        ESP_LOGI(LOG_TAG, "Authentication failed");
      }
      break;
  case ESP_GAP_BLE_PASSKEY_NOTIF_EVT:
    ESP_LOGI(LOG_TAG, "ESP_GAP_BLE_PASSKEY_NOTIF_EVT: %u", param->ble_security.key_notif.passkey);
    break;
  case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
    ESP_LOGV(LOG_TAG, "ESP_GAP_BLE_ADV_START_COMPLETE_EVT");
    break;
  default:
    ESP_LOGV(LOG_TAG, "Unhandled GAP event: %u", static_cast<uint32_t>(event));
    break;
  }
}

void BTDriver::staticGattEventHandler(esp_gatts_cb_event_t event,
                                      esp_gatt_if_t interface,
                                      esp_ble_gatts_cb_param_t *param)
{
  if ( s_driver != nullptr )
  {
    s_driver->gattEventHandler(event, interface, param);
  } else
  {
    ESP_LOGE(LOG_TAG, "Received GATT event with no driver instance to handle it");
  }
}

void BTDriver::gattEventHandler(esp_gatts_cb_event_t event,
                                esp_gatt_if_t interface,
                                esp_ble_gatts_cb_param_t *param)
{
  esp_err_t err;

  switch ( event )
  {
  case ESP_GATTS_REG_EVT:
    m_gattInterface = interface;

    if ( param->reg.status == ESP_GATT_OK )
    {
      ESP_LOGI(LOG_TAG, "Successfully registered gatts app");
      setAppRegistered(true);
    } else
    {
      ESP_LOGE(LOG_TAG, "Failed to register gatts app");
    }

    err = esp_ble_gap_set_device_name(m_deviceName);
    if ( err != ESP_OK )
    {
      ESP_LOGE(LOG_TAG, "Failed to set BLE device name");
    }

    // Register GATT profile and characteristics
    err = esp_ble_gatts_create_attr_tab(m_gattDb, interface, m_numDbEntries, 0);
    if ( err != ESP_OK )
    {
      ESP_LOGE(LOG_TAG, "Failed to register gatt service db");
    }

    // This triggers ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT event
    err = esp_ble_gap_config_adv_data(&s_advertisingConfig);
    if ( err != ESP_OK )
    {
      ESP_LOGE(LOG_TAG, "Failed to set BLE advertising data");
    }
    // This triggers ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT event
    err = esp_ble_gap_config_adv_data(&s_scanResponseConfig);
    if ( err != ESP_OK )
    {
      ESP_LOGE(LOG_TAG, "Failed to set BLE scan response data");
    }
    break;
  case ESP_GATTS_READ_EVT:
    ESP_LOGI(LOG_TAG, "GATT Read event for attribute: %hu", param->read.handle);
    break;
  case ESP_GATTS_WRITE_EVT:
    ESP_LOGD(LOG_TAG, "GATT Write event for attribute: %hu", param->write.handle);
    sendWriteEvent(*param);
    break;
  case ESP_GATTS_EXEC_WRITE_EVT:
    ESP_LOGI(LOG_TAG, "ESP_GATTS_EXEC_WRITE_EVT");
    break;
  case ESP_GATTS_CONNECT_EVT:
    ESP_LOGI(LOG_TAG, "Got connection event for device: %02x:%02x:%02x:%02x:%02x:%02x",
        param->connect.remote_bda[0],
        param->connect.remote_bda[1],
        param->connect.remote_bda[2],
        param->connect.remote_bda[3],
        param->connect.remote_bda[4],
        param->connect.remote_bda[5]);
    m_advertisingStatus &= ~ADVERTISING_ACTIVE;
    m_connectionId = param->connect.conn_id;
    m_connected = true;
    // Commented out - Do not send the pairing request from the peripheral. Wait for Central request
	//esp_ble_set_encryption(param->connect.remote_bda, ESP_BLE_SEC_ENCRYPT_MITM);

    // TODO investigate: BT: l2cble_start_conn_update, the last connection update command still pending.
    break;
  case ESP_GATTS_DISCONNECT_EVT:
    ESP_LOGI(LOG_TAG, "Got disconnect event");
    m_connected = false;
    if ( isServiceRunning() )
    {
    	//stop advertising after a disconnect is received
    	ESP_LOGI(LOG_TAG, "Stop advertising");
        setServiceRunning(false);
//      err = esp_ble_gap_start_advertising(&s_advertisingParams);
//      if ( err == ESP_OK )
//      {
//        m_advertisingStatus |= ADVERTISING_ACTIVE;
//        ESP_LOGI(LOG_TAG, "Start advertising");
//      } else
//      {
//        ESP_LOGE(LOG_TAG, "Failed to start advertising");
//      }
    }
    break;
  case ESP_GATTS_CREAT_ATTR_TAB_EVT:
    if ( param->add_attr_tab.status == ESP_GATT_OK )
    {
      ESP_LOGI(LOG_TAG, "GATT attribute table created successfully");
      setDbRegistered(true, param->add_attr_tab.handles);

      err = esp_ble_gatts_start_service(m_attributeHandles[0]); // Assume that the first attribute is the service...
      if ( err != ESP_OK )
      {
        ESP_LOGE(LOG_TAG, "Failed to start GATT service");
      } else
      {
        ESP_LOGI(LOG_TAG, "GATT service started");
        setServiceRunning(true);
      }
    } else
    {
      ESP_LOGE(LOG_TAG, "Failed to create GATT attribute table: %u",
          param->add_attr_tab.status);
    }
    break;
  case ESP_GATTS_SET_ATTR_VAL_EVT:
    ESP_LOGV(LOG_TAG, "Characteristic value updated: 0x%02X", param->set_attr_val.attr_handle);
    break;
  case ESP_GATTS_START_EVT:
    ESP_LOGV(LOG_TAG, "GATT service started: 0x%02X", param->start.service_handle);
    break;
  case ESP_GATTS_STOP_EVT:
    ESP_LOGV(LOG_TAG, "GATT service stopped: 0x%02X", param->stop.service_handle);
    break;
  case ESP_GATTS_UNREG_EVT:
    ESP_LOGV(LOG_TAG, "GATT unregistered event");
    break;
  case ESP_GATTS_MTU_EVT:
    ESP_LOGI(LOG_TAG, "GATT MTU exchange event, new MTU size is: %u bytes", param->mtu.mtu);
    break;
  case ESP_GATTS_CONF_EVT:
    //ESP_LOGI(LOG_TAG, "ESP_GATTS_CONF_EVT");
    break;
  default:
    ESP_LOGI(LOG_TAG, "Unhandled GATT event: %u", static_cast<uint32_t>(event));
    break;
  }
}

void BTDriver::reset()
{
  stopBleApp();

  if (isDriverInitialized())
  {
    esp_bluedroid_disable();
    esp_bluedroid_deinit();

    esp_bt_controller_disable();
    esp_bt_controller_deinit();
  }

  // Note: this cannot be reversed
  esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);

  resetGattDb();

  m_advertisingStatus = ADVERTISING_NOT_CONFIGURED;
  m_eventReceiver = nullptr;
  setAppRegistered(false);
  setDbRegistered(false, nullptr);
  setServiceRunning(false);
  setDriverInitialized(false);

  m_gattInterface = 0u;
  m_connectionId = 0u;
  m_connected = false;

  memset(m_deviceName, 0, sizeof(m_deviceName));

  s_driver = nullptr;
}

bool BTDriver::resetGattDb()
{
  if ( isServiceRunning() )
  {
    ESP_LOGE(LOG_TAG, "Cannot reset db elements when service is running");
    return false;
  }

  // Reset db
  m_numDbEntries = 0;
  m_numUuids = 0;
  memset(m_gattDb, 0, sizeof(m_gattDb));
  memset(m_attributeHandles, 0, sizeof(m_attributeHandles));
  memset(m_uuids, 0, sizeof(m_uuids));
  setDbRegistered(false, nullptr);

  return true;
}

bool BTDriver::addService(const uint8_t *uuid, size_t uuidSize,  bool encrypted)
{
  if ( m_numDbEntries > 0 ) // Only support one service, and force it to the first db entry
  {
    return false;
  }

  uint8_t *uuidBuf = m_uuids+m_numUuids*UUID_SIZE_128;
  memcpy(uuidBuf, uuid, uuidSize);

  esp_gatts_attr_db_t &entry = m_gattDb[m_numDbEntries];
  entry.attr_control.auto_rsp = ESP_GATT_AUTO_RSP;
  entry.att_desc.uuid_length = ESP_UUID_LEN_16;
  entry.att_desc.uuid_p = reinterpret_cast<uint8_t *>(&PRIMARY_SERVICE_UUID);
  entry.att_desc.perm = ESP_GATT_PERM_READ_ENCRYPTED;
  entry.att_desc.max_length = uuidSize;
  entry.att_desc.length = uuidSize;
  entry.att_desc.value = uuidBuf;

  m_numDbEntries += 1;
  m_numUuids += 1;

  return true;
}

bool BTDriver::addCharacteristic(uint8_t *value,
                                 size_t valueSize,
                                 const uint8_t *uuid,
                                 size_t uuidSize,
                                 bool readable,
                                 bool writable,
                                 bool supportNotifications,
                                 uint16_t *clientConfig,
                                 GattAttributeIndex *valueIndex,
								 bool encrypted)
{
  if ( m_numDbEntries+2 >= Constants::GATTDB_ENTRIES_MAX )
  {
    return false;
  }

  uint8_t *uuidBuf = nullptr;
  // ESP_GATT_PERM_READ_ENC_MITM and ESP_GATT_PERM_WRITE_ENC_MITM seem broken in IDF v3 with regards
  // to bonding, so using just "encrypted".
  const uint16_t permissionRead = (encrypted ? ESP_GATT_PERM_READ_ENCRYPTED : ESP_GATT_PERM_READ);
  const uint16_t permissionWrite = (encrypted ? ESP_GATT_PERM_WRITE_ENCRYPTED : ESP_GATT_PERM_WRITE);


  { // Characteristic declaration
    esp_gatts_attr_db_t &entry = m_gattDb[m_numDbEntries];
    entry.attr_control.auto_rsp = ESP_GATT_AUTO_RSP;
    entry.att_desc.uuid_length = ESP_UUID_LEN_16;
    entry.att_desc.uuid_p = reinterpret_cast<uint8_t *>(const_cast<uint16_t *>(&CHARACTERISTIC_DECLARATION_UUID));
    entry.att_desc.perm = permissionRead;
    entry.att_desc.max_length = CHARACTERISTIC_DECLARATION_SIZE;
    entry.att_desc.length = CHARACTERISTIC_DECLARATION_SIZE;
    entry.att_desc.value = getPermission(readable, writable, supportNotifications);
    if ( entry.att_desc.value == nullptr )
    {
      return false;
    }
  }
  m_numDbEntries += 1;

  { // Characteristic value
    uuidBuf = m_uuids+m_numUuids*UUID_SIZE_128;
    memcpy(uuidBuf, uuid, uuidSize);

    esp_gatts_attr_db_t &entry = m_gattDb[m_numDbEntries];
    entry.attr_control.auto_rsp = ESP_GATT_AUTO_RSP;
    entry.att_desc.uuid_length = (uuidSize == 16 ? ESP_UUID_LEN_128 : ESP_UUID_LEN_16);
    entry.att_desc.uuid_p = uuidBuf;
    entry.att_desc.perm = 0;
    if ( readable )
    {
      entry.att_desc.perm = permissionRead;
    }
    if ( writable )
    {
      entry.att_desc.perm |= permissionWrite;
    }
    entry.att_desc.max_length = valueSize;
    entry.att_desc.length = valueSize;
    entry.att_desc.value = value;

    if ( valueIndex != nullptr )
    {
      *valueIndex = m_numDbEntries;
    }
  }
  m_numDbEntries += 1;
  m_numUuids += 1;

  if ( supportNotifications )
  {
    if ( m_numDbEntries >= Constants::GATTDB_ENTRIES_MAX )
    {
      return false;
    }
    if ( clientConfig == nullptr )
    {
      ESP_LOGE(LOG_TAG, "clientConfig set to null with notifications enabled");
      return false;
    }
    esp_gatts_attr_db_t &entry = m_gattDb[m_numDbEntries];
    entry.attr_control.auto_rsp = ESP_GATT_AUTO_RSP;
    entry.att_desc.uuid_length = ESP_UUID_LEN_16;
    entry.att_desc.uuid_p = reinterpret_cast<uint8_t *>(const_cast<uint16_t *>(&CHARACTERISTIC_CLIENT_CONFIG_UUID));
    entry.att_desc.perm = permissionRead | permissionWrite;
    entry.att_desc.max_length = CLIENT_CONFIG_SIZE;
    entry.att_desc.length = CLIENT_CONFIG_SIZE;
    entry.att_desc.value = reinterpret_cast<uint8_t *>(clientConfig);

    m_numDbEntries += 1;
  }

  return true;
}

bool BTDriver::writeCharacteristicValue(GattAttributeIndex valueIndex,
                                        const uint8_t *value,
                                        size_t valueSize)
{
  if ( valueIndex >= m_numDbEntries || !isDbRegistered() )
  {
    return false;
  }

  esp_err_t res;
  res = esp_ble_gatts_set_attr_value(
          m_attributeHandles[valueIndex],
          valueSize,
          value);
  if ( res != ESP_OK )
  {
    ESP_LOGE(LOG_TAG, "Failed to set value for GATT attribute: %u", static_cast<uint32_t>(valueIndex));
    return false;
  }
  return true;
}

bool BTDriver::sendCharacteristicNotification(GattAttributeIndex valueIndex,
                                              const uint8_t *value,
                                              size_t valueSize)
{
  if ( !m_connected )
  {
    ESP_LOGE(LOG_TAG, "Cannot send GATT notifications when not connected to a BLE master device");
    return false;
  }

  /* TODO "BT: attribute value too long, to be truncated to 20" <-- try changing MTU size */
  esp_err_t err = esp_ble_gatts_send_indicate(m_gattInterface,
                                              m_connectionId,
                                              m_attributeHandles[valueIndex],
                                              valueSize,
                                              const_cast<uint8_t *>(value),
                                              false);
  if ( err != ESP_OK )
  {
    ESP_LOGE(LOG_TAG, "Failed to send GATT notification for attribute: %zu", valueIndex);
    return false;
  }
  return true;
}

bool BTDriver::init()
{
  if ( s_driver != nullptr )
  {
    ESP_LOGE(LOG_TAG, "Multiple BTDriver instances not supported");
    return false;
  }

  esp_err_t ret;

  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  ret = esp_bt_controller_init(&bt_cfg);
  if ( ret ) {
    ESP_LOGE(LOG_TAG, "Failed to init Bluetooth controller");
    return false;
  }
  ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
  if ( ret ) {
    ESP_LOGE(LOG_TAG, "Failed to enable Bluetooth controller");
    return false;
  }

  ret = esp_bluedroid_init();
  if ( ret ) {
    ESP_LOGE(LOG_TAG, "Failed to init bluedroid");
    return false;
  }
  ret = esp_bluedroid_enable();
  if ( ret ) {
    ESP_LOGE(LOG_TAG, "Failed to enable bluedroid");
    return false;
  }

  s_driver = this;

  // Event handlers
  esp_ble_gatts_register_callback(staticGattEventHandler);
  esp_ble_gap_register_callback(staticGapEventHandler);

  setDriverInitialized(true);

  return true;
}

bool BTDriver::setSecurityParameters()
{
  esp_err_t ret;

  ret = esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &BLE_SECURITY_AUTH_REQ_MODE, sizeof(BLE_SECURITY_AUTH_REQ_MODE));
  if ( ret != ESP_OK )
  {
      ESP_LOGE(LOG_TAG, "Failed to set BLE authentication mode: %d", ret);
      return false;
  }

  ret = esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &BLE_SECURITY_MAX_KEY_SIZE, sizeof(BLE_SECURITY_MAX_KEY_SIZE));
  if ( ret != ESP_OK )
  {
      ESP_LOGE(LOG_TAG, "Failed to set BLE key size: %d", ret);
      return false;
  }

  ret = esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &BLE_SECURITY_IO_CAP, sizeof(BLE_SECURITY_IO_CAP));
  if ( ret != ESP_OK )
  {
      ESP_LOGE(LOG_TAG, "Failed to set BLE IO capabilities: %d", ret);
      return false;
  }

  ret = esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &BLE_SECURITY_INITIATOR_KEYS, sizeof(BLE_SECURITY_INITIATOR_KEYS));
  if ( ret != ESP_OK )
  {
      ESP_LOGE(LOG_TAG, "Failed to set BLE initiator keys: %d", ret);
      return false;
  }

  ret = esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &BLE_SECURITY_RESPONDER_KEYS, sizeof(BLE_SECURITY_RESPONDER_KEYS));
  if ( ret != ESP_OK )
  {
      ESP_LOGE(LOG_TAG, "Failed to set BLE responder keys: %d", ret);
      return false;
  }
  return true;
}

void BTDriver::setDbRegistered(bool dbRegistered, const uint16_t *handles)
{
  // TODO mutex

  if ( dbRegistered )
  {
    memcpy(m_attributeHandles,
           handles,
           sizeof(m_attributeHandles[0]) * m_numDbEntries);
  } else
  {
    memset(m_attributeHandles, 0, sizeof(m_attributeHandles));
  }
  m_dbRegistered = dbRegistered;
}

GattAttributeIndex BTDriver::handleToIndex(uint16_t handle)
{
  for ( GattAttributeIndex i=0; i<m_numDbEntries; i++ )
  {
    if ( m_attributeHandles[i] == handle )
    {
      return i;
    }
  }
  return std::numeric_limits<uint16_t>::max();
}

void BTDriver::sendWriteEvent(const esp_ble_gatts_cb_param_t &param)
{
  if ( m_eventReceiver != nullptr )
  {
    GattAttributeIndex index = handleToIndex(param.write.handle);
    if ( index != std::numeric_limits<uint16_t>::max() )
    {
      m_eventReceiver->handleCharacteristicValueChanged(index,
                                                        param.write.len,
                                                        param.write.value);
    }
  } else
  {
    ESP_LOGE(LOG_TAG, "Failed to send write event: no event receiver registered");
  }
}

bool BTDriver::startAdvertising()
{
  esp_err_t err;

  s_advertisingParams.adv_int_min = 0x80;
  s_advertisingParams.adv_int_max = 0x100;
  s_advertisingParams.adv_type = ADV_TYPE_IND;
  s_advertisingParams.own_addr_type = BLE_ADDR_TYPE_PUBLIC;

  std::memset(s_advertisingParams.peer_addr, 0, sizeof(s_advertisingParams.peer_addr));
  s_advertisingParams.peer_addr_type = BLE_ADDR_TYPE_PUBLIC;
  s_advertisingParams.channel_map = ADV_CHNL_ALL;

  // Note: Possible to setup white-listing using this parameter
  s_advertisingParams.adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY;

  err = esp_ble_gap_start_advertising(&s_advertisingParams);
  if ( err == ESP_OK )
  {
    m_advertisingStatus |= ADVERTISING_ACTIVE;
    ESP_LOGI(LOG_TAG, "Start advertising");
  } else
  {
    ESP_LOGE(LOG_TAG, "Failed to start advertising");
    return false;
  }
  return true;
}
