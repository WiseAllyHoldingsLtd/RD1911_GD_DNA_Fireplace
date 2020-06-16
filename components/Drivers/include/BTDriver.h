#pragma once

#include "esp_bt.h"

#include <esp_gap_ble_api.h>
#include <esp_gatts_api.h>
#include <esp_bt_defs.h>
#include <esp_bt_main.h>

#include <limits>

#include "BTDriverInterface.h"
#include "Constants.h"

class BTDriver: public BTDriverInterface
{
public:
  BTDriver();

  BTDriver(const BTDriver &) = delete;
  BTDriver &operator=(const BTDriver &) = delete;

  virtual bool resetAndInit();
  virtual bool getBleMac(uint8_t (&dstMac)[Constants::MAC_ADDRESS_SIZE]);
  virtual bool startBleApp(uint16_t bleAppId,
                           const char *deviceName,
                           BTDriverEventReceiverInterface *eventReceiver,
                           uint32_t staticPasskey); // also event subscriber i/f?
  virtual void stopBleApp();
  virtual bool isConnected();
  virtual bool isServiceRunning() const;

  virtual void reset();
  virtual bool resetGattDb();
  virtual bool addService(const uint8_t *uuid, size_t uuidSize, bool encrypted = true);
  virtual bool addCharacteristic(uint8_t *value,
                                 size_t valueSize,
                                 const uint8_t *uuid,
                                 size_t uuidSize,
                                 bool readable,
                                 bool writable,
                                 bool supportNotifications,
                                 uint16_t *clientConfig,
                                 GattAttributeIndex *valueIndex,
								 bool encrypted = true);
  virtual bool writeCharacteristicValue(GattAttributeIndex valueIndex,
                                        const uint8_t *value,
                                        size_t valueSize);
  virtual bool sendCharacteristicNotification(GattAttributeIndex valueIndex,
                                              const uint8_t *value,
                                              size_t valueSize);

private:
  static void staticGapEventHandler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
  static void staticGattEventHandler(esp_gatts_cb_event_t event,
                                     esp_gatt_if_t interface,
                                     esp_ble_gatts_cb_param_t *param);
  void gapEventHandler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
  void gattEventHandler(esp_gatts_cb_event_t event,
                        esp_gatt_if_t interface,
                        esp_ble_gatts_cb_param_t *param);
  bool init();
  bool setSecurityParameters();

  void sendWriteEvent(const esp_ble_gatts_cb_param_t &param);

  bool startAdvertising();

  bool isDriverInitialized() const
  {
    return m_driverInitialized;
  }
  void setDriverInitialized(bool initialized)
  {
    m_driverInitialized = true;
  }

  void setServiceRunning(bool running)
  {
    m_serviceRunning = running;
  }

  bool isAppRegistered() const
  {
    return m_appRegistered;
  }
  void setAppRegistered(bool registered)
  {
    m_appRegistered = registered;
  }

  bool isDbRegistered() const
  {
    return m_dbRegistered;
  }
  void setDbRegistered(bool dbRegistered, const uint16_t *handles);

  volatile bool m_driverInitialized;
  volatile bool m_serviceRunning;
  volatile bool m_appRegistered;
  volatile bool m_dbRegistered;

  size_t m_numDbEntries = 0u;
  size_t m_numUuids = 0u;
  static const size_t UUID_SIZE_128 = 16u;
  esp_gatts_attr_db_t m_gattDb[Constants::GATTDB_ENTRIES_MAX];
  uint16_t m_attributeHandles[Constants::GATTDB_ENTRIES_MAX];
  uint8_t m_uuids[Constants::GATT_UUIDS_MAX*UUID_SIZE_128];

  GattAttributeIndex handleToIndex(uint16_t handle);

  BTDriverEventReceiverInterface *m_eventReceiver;

  volatile uint8_t m_advertisingStatus;

  char m_deviceName[Constants::BT_DEVICE_NAME_SIZE_MAX];

  // Stuff related to current BT session
  esp_gatt_if_t m_gattInterface;
  uint16_t m_connectionId;
  bool m_connected;
};
