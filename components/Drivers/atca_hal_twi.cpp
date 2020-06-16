#include <unistd.h>
#include <cstring>

#include "atca_hal.h"
#include "FreeRTOS.hpp"
#include "esp_log.h"

#include "TWIDriver.h"

// Word address values from Datasheet
#define ATCA_I2C_ADDRESS_RESET 0x00u
#define ATCA_I2C_ADDRESS_SLEEP 0x01u
#define ATCA_I2C_ADDRESS_IDLE 0x02u
#define ATCA_I2C_ADDRESS_COMMAND 0x03u


namespace TWIDriverHAL {
  // Defined in TWIDriver.cpp
  extern TWIDriver * instance;
}

namespace {
  // Verifies that a TWIDriver instance is available
  ATCA_STATUS checkInstance()
  {
    ATCA_STATUS status = ATCA_COMM_FAIL;
    if (TWIDriverHAL::instance != nullptr) {
      status = ATCA_SUCCESS;
    }
    return status;
  }

  const char LOG_TAG[] = "ATCA_HAL";
}


ATCA_STATUS hal_i2c_init(void *hal, ATCAIfaceCfg *cfg)
{
  return checkInstance();
}

ATCA_STATUS hal_i2c_post_init(ATCAIface iface)
{
  return checkInstance();
}

ATCA_STATUS hal_i2c_send(ATCAIface iface, uint8_t *txdata, int txlength)
{
  ATCAIfaceCfg *cfg = atgetifacecfg(iface);
  ATCA_STATUS status = checkInstance();
  if (status == static_cast<int>(ATCA_SUCCESS)) {
    ESP_LOGD(LOG_TAG, "i2c_send: txlength:%u", txlength);

    // for this implementation of I2C with CryptoAuth chips, txdata is assumed to have ATCAPacket format

    // other device types that don't require i/o tokens on the front end of a command need a different hal_i2c_send and wire it up instead of this one
    // this covers devices such as ATSHA204A and ATECCx08A that require a word address value pre-pended to the packet
    // txdata[0] is using _reserved byte of the ATCAPacket
    txdata[0] = ATCA_I2C_ADDRESS_COMMAND; // insert the Word Address Value, Command token
    txlength++;       // account for word address value byte.

    if ((txlength > 0) && (static_cast<unsigned>(txlength) <= UINT8_MAX)) {
      if (!TWIDriverHAL::instance->send(cfg->atcai2c.slave_address, txdata, static_cast<uint8_t>(txlength))) {
        status = ATCA_TX_TIMEOUT;
      }
    }
    else {
      status = ATCA_INVALID_SIZE;
    }
  }
  return status;
}

ATCA_STATUS hal_i2c_receive(ATCAIface iface, uint8_t *rxdata, uint16_t *rxlength)
{
  ATCAIfaceCfg *cfg = atgetifacecfg(iface);
  ATCA_STATUS status = checkInstance();
  if (status == static_cast<int>(ATCA_SUCCESS)) {
    ESP_LOGD(LOG_TAG, "i2c_receive: rxlength:%u", *rxlength);
    if (*rxlength <= UINT8_MAX) {
      if (TWIDriverHAL::instance->recieve(cfg->atcai2c.slave_address, rxdata, static_cast<uint8_t>(*rxlength))) {
        // First received byte is a count byte.
        if (static_cast<uint16_t>(rxdata[0]) > *rxlength) {
          status = ATCA_INVALID_SIZE;
        }
      }
      else {
        status = ATCA_RX_TIMEOUT;
      }
    }
    else {
      status = ATCA_INVALID_SIZE;
    }
  }
  return status;
}

ATCA_STATUS hal_i2c_wake(ATCAIface iface)
{
  uint8_t rxData[4] = {};
  uint8_t expected[4] = { 0x04u, 0x11u, 0x33u, 0x43u };
  uint8_t dummy = 0x00u;

  ATCAIfaceCfg *cfg = atgetifacecfg(iface);
  ATCA_STATUS status = checkInstance();

  if (status == static_cast<int>(ATCA_SUCCESS)) {
    // Create wake up pulse by sending a slave address 0f 0x00.
    // No one will ACK the address, so no data will be sent, and we ignore the return value
    TWIDriverHAL::instance->send(0x00u, &dummy, 0u);

    atca_delay_us(cfg->wake_delay); // wait tWHI + tWLO which is configured based on device type and configuration structure

    if (!TWIDriverHAL::instance->recieve(cfg->atcai2c.slave_address, rxData, 4u)) {
      status = ATCA_RX_NO_RESPONSE;
    }

    if (status == static_cast<int>(ATCA_SUCCESS)) {
      if (memcmp(rxData, expected, 4u) != 0) {
        status = ATCA_COMM_FAIL;
      }
    }
  }
  return status;
}

ATCA_STATUS hal_i2c_idle(ATCAIface iface)
{
  ATCAIfaceCfg *cfg = atgetifacecfg(iface);
  ATCA_STATUS status = checkInstance();
  uint8_t buffer = ATCA_I2C_ADDRESS_IDLE;  //!< Word Address Value = Idle

  if (status == static_cast<int>(ATCA_SUCCESS)) {
    if (!TWIDriverHAL::instance->send(cfg->atcai2c.slave_address, &buffer, 1u)) {
      status = ATCA_TX_TIMEOUT;
    }
  }

  return status;
}

ATCA_STATUS hal_i2c_sleep(ATCAIface iface)
{
  ATCAIfaceCfg *cfg = atgetifacecfg(iface);
  ATCA_STATUS status = checkInstance();
  uint8_t buffer = ATCA_I2C_ADDRESS_SLEEP;  //!< Word Address Value = Idle

  if (status == static_cast<int>(ATCA_SUCCESS)) {
    if (!TWIDriverHAL::instance->send(cfg->atcai2c.slave_address, &buffer, 1u)) {
      status = ATCA_TX_TIMEOUT;
    }
  }

  return status;
}

ATCA_STATUS hal_i2c_release(void *hal_data)
{
  return ATCA_UNIMPLEMENTED;
}

ATCA_STATUS hal_i2c_discover_buses(int i2c_buses[], int max_buses)
{
  return ATCA_UNIMPLEMENTED;
}

ATCA_STATUS hal_i2c_discover_devices(int bus_num, ATCAIfaceCfg *cfg, int *found)
{
  return ATCA_UNIMPLEMENTED;
}

void atca_delay_us(uint32_t delay)
{
  usleep(delay);
}

void atca_delay_10us(uint32_t delay)
{
  atca_delay_ms(delay * 10u);
}

void atca_delay_ms(uint32_t delay)
{
  FreeRTOS_delay_ms(delay);
}
