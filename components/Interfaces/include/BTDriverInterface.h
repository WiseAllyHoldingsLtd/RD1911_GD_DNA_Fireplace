#pragma once

#include <cstdint>
#include "Constants.h"

using GattAttributeIndex = uint16_t;

class BTDriverEventReceiverInterface
{
public:
  virtual ~BTDriverEventReceiverInterface() { }

  virtual void handleCharacteristicValueChanged(
      GattAttributeIndex attributeIndex,
      uint16_t size,
      uint8_t *value) = 0;
};

class BTDriverInterface
{
public:
  virtual ~BTDriverInterface() { }

  virtual bool resetAndInit() = 0;

  /**
   * Get Bluetooth MAC address.
   *
   * @param dstMac
   * Pre-allocated buffer (at least 6 bytes) for
   * storing the MAC address.
   */
  virtual bool getBleMac(uint8_t (&dstMac)[Constants::MAC_ADDRESS_SIZE]) = 0;

  /**
   * @param bleAppId
   * Gatt application id. Not sure what role this has. 0x55 was used in examples. Might be something internal.
   *
   * @param deviceName
   * The name to use in BLE advertising.
   *
   * @param eventSubscriber
   * Receiver of events from BTDriver.
   *
   * @param staticPasskey
   * Will be used as static passkey if > 0.
   */
  virtual bool startBleApp(uint16_t bleAppId,
                           const char *deviceName,
                           BTDriverEventReceiverInterface *eventSubscriber,
                           uint32_t staticPasskey=0) = 0;

  /**
   * Stop the BLE app, but don't disable the driver.
   */
  virtual void stopBleApp() = 0;

  /**
   * @return true if a BLE master device is connected
   */
  virtual bool isConnected() = 0;

  /**
   * @return true as long as BLE service is active (typ. advertising or connected)
   */
  virtual bool isServiceRunning() const = 0;

  /**
   * Stop BLE advertising and deinit Bluetooth stack
   */
  virtual void reset() = 0;

  /**
   * Undefine all db elements
   */
  virtual bool resetGattDb() = 0;

  /**
   * Start defining a GATT service. Subsequent characteristics
   * that are added will belong to this service.
   *
   * @param uuid
   * Make sure this memory stays allocated/doesn't go out of scope
   *
   * @param uuidSize
   * Size of uuid in bytes.
   *
   * @return true if service could be added.
   *
   * @remark
   * Only one simultaneous service assumed.
   */
  virtual bool addService(const uint8_t *uuid, size_t uuidSize, bool encrypted = true) = 0;

  /**
   * @brief
   * Add a GATT-characteristic. Cannot be done after calling startBleApp.
   *
   * @param value
   * Value for this characteristic. Not sure if this memory must stay
   * allocated/in scope or not.
   *
   * @param valueSize
   * Size of value, bytes.
   *
   * @param uuid
   * GATT characteristic universally unique ID that the GATT client
   * will use to identify the characteristic.
   *
   * @param uuidSize
   * Size of UUID.
   *
   * @param readable
   * If GATT client should be able to read the value for this characteristic.
   *
   * @param writable
   * If GATT client should be able to write the value for this characteristic.
   *
   * @param supportNotifications
   * If GATT client should be able to subscribe to notifications for this
   * characteristic. Note that a clientConfig!=nullptr must be provided to support notifications!
   *
   * @param clientConfig
   * Only needed for characteristics that support notifications
   * (or indications). Just assign a uint16_t variable that will
   * stay in scope. Or set it to nullptr if supportNotifications=false.
   *
   * @param [out] valueIndex
   * Handle to the Gatt Attribute Value of the characteristic, if successful.
   * May be used to read/write the value or send notifications.
   *
   * @return
   * true if characteristic could be added.
   */
  virtual bool addCharacteristic(uint8_t *value,
                                 size_t valueSize,
                                 const uint8_t *uuid,
                                 size_t uuidSize,
                                 bool readable,
                                 bool writable,
                                 bool supportNotifications,
                                 uint16_t *clientConfig,
                                 GattAttributeIndex *valueIndex,
								 bool encrypted = true) = 0;

  /**
   * Update a GATT characteristic value.
   */
  virtual bool writeCharacteristicValue(GattAttributeIndex valueIndex,
                                        const uint8_t *value,
                                        size_t valueSize) = 0;

  /**
   *
   */
  virtual bool sendCharacteristicNotification(GattAttributeIndex valueIndex,
                                              const uint8_t *value,
                                              size_t valueSize) = 0;
};
