 #pragma once

#include "CppUTestExt\MockSupport.h"
#include "BTDriverInterface.h"

#define NAME(method) "BTDriverMock::" method


class BTDriverMock : public BTDriverInterface
{
  virtual bool resetAndInit()
  {
    return mock().actualCall(NAME("resetAndInit")).returnBoolValueOrDefault(true);
  }

  virtual bool getBleMac(uint8_t (&dstMac)[Constants::MAC_ADDRESS_SIZE])
  {
    // Not properly implemented as it is not used.
    return mock().actualCall(NAME("getBleMac")).returnBoolValueOrDefault(true);
  }

  virtual bool startBleApp(uint16_t bleAppId,
                           const char *deviceName,
                           BTDriverEventReceiverInterface *eventSubscriber,
                           uint32_t staticPasskey=0)
  {
    // Not properly implemented as it is not used.
    return mock().actualCall(NAME("startBleApp")).returnBoolValueOrDefault(true);
  }

  virtual void stopBleApp()
  {
    mock().actualCall(NAME("stopBleApp"));
  }

  virtual bool isConnected()
  {
    return mock().actualCall(NAME("isConnected")).returnBoolValueOrDefault(true);
  }

  virtual bool isServiceRunning() const
  {
    return mock().actualCall(NAME("isServiceRunning")).returnBoolValueOrDefault(true);
  }

  virtual void reset()
  {
    mock().actualCall(NAME("reset"));
  }

  virtual bool resetGattDb()
  {
    return mock().actualCall(NAME("resetGattDb")).returnBoolValueOrDefault(true);
  }

  virtual bool addService(const uint8_t *uuid, size_t uuidSize, bool encrypted=true)
  {
    return mock().actualCall(NAME("addService"))
        .withConstPointerParameter("uuid", static_cast<const void*>(uuid))
        .withUnsignedIntParameter("uuidSize", uuidSize)
        .withBoolParameter("encrypted", encrypted)
        .returnBoolValueOrDefault(true);
  }

  virtual bool addCharacteristic(uint8_t *value,
                                 size_t valueSize,
                                 const uint8_t *uuid,
                                 size_t uuidSize,
                                 bool readable,
                                 bool writable,
                                 bool supportNotifications,
                                 uint16_t *clientConfig,
                                 GattAttributeIndex *valueIndex,
                                 bool encrypted = true)
  {
    // Not properly implemented as it is not used.
    return mock().actualCall(NAME("addCharacteristic")).returnBoolValueOrDefault(true);
  }

  virtual bool writeCharacteristicValue(GattAttributeIndex valueIndex,
                                        const uint8_t *value,
                                        size_t valueSize)
  {
    // Not properly implemented as it is not used.
    return mock().actualCall(NAME("writeCharacteristicValue")).returnBoolValueOrDefault(true);
  }

  virtual bool sendCharacteristicNotification(GattAttributeIndex valueIndex,
                                              const uint8_t *value,
                                              size_t valueSize)
  {
    // Not properly implemented as it is not used.
    return mock().actualCall(NAME("sendCharacteristicNotification")).returnBoolValueOrDefault(true);
  }
};
