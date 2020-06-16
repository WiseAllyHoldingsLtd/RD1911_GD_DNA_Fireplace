#include "driver/i2c.h"
#include "EspCpp.hpp"

#include "FreeRTOS.hpp"
#include "Constants.h"
#include "HardwareSetup.h"

#include "TWIDriver.h"

#define TWI_SEND_TIMEOUT_TICKS (Constants::TWI_TRANSFER_TIMEOUT_MS/portTICK_RATE_MS)
#define TWI_LOCK_TIMEOUT_TICKS TWI_SEND_TIMEOUT_TICKS

static const char * LOG_TAG = "TWIDriver";

namespace TWIDriverHAL {
  // Pointer that ATCA HAL uses for I2C communication
  TWIDriver *instance = nullptr;
};


TWIDriver::~TWIDriver()
{
  TWIDriverHAL::instance = nullptr;
  i2c_driver_delete(I2C_NUM_0);
}


void TWIDriver::setup(uint32_t busFrequency)
{
  m_lock.take();
  i2c_config_t buscfg = {};
  buscfg.mode = I2C_MODE_MASTER;
  buscfg.scl_io_num = I2C_SCL_PIN;
  buscfg.scl_pullup_en = GPIO_PULLUP_ENABLE;
  buscfg.sda_io_num = I2C_SDA_PIN;
  buscfg.sda_pullup_en = GPIO_PULLUP_ENABLE;
  buscfg.master.clk_speed = busFrequency;

  if (i2c_param_config(I2C_PORT, &buscfg) != ESP_OK) {
    ESP_LOGE(LOG_TAG, "Failed to initialize TWI/I2C");
  }

  if (i2c_driver_install(I2C_PORT, I2C_MODE_MASTER, 0u, 0u, 0u) != ESP_OK) {
    ESP_LOGE(LOG_TAG, "Failed to install TWI/I2C driver");
  }

  if (i2c_set_timeout(I2C_PORT, ((APB_CLK_FREQ/1000) * Constants::TWI_TRANSFER_TIMEOUT_MS)) != ESP_OK) {
    ESP_LOGE(LOG_TAG, "Failed to set TWI/I2C timeout");

    int actualTimeout = 0;
    if (i2c_get_timeout(I2C_PORT, &actualTimeout) == ESP_OK){
      ESP_LOGI(LOG_TAG, "Using default TWI/I2C timeout value %i (number of cycles for APB clock)", actualTimeout);
    }
  }

// Reset sequence?
//  To ensure an I/O channel reset, the system should send the standard I2C software reset sequence,
//  as follows:
//  – A Start bit condition.
//  – Nine cycles of SCL, with SDA held high.
//  – Another Start bit condition.
//  – A Stop bit condition.
  m_lock.give();
  TWIDriverHAL::instance = this;
}


bool TWIDriver::_send(uint8_t slaveAddress, uint8_t *buffer, uint8_t sendLength)
{
  bool result = false;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  uint8_t address = static_cast<uint8_t>(slaveAddress << 1u); // LSB is reserved for R/W bit
  i2c_master_write_byte(cmd, (address | I2C_MASTER_WRITE), true);
  i2c_master_write(cmd, buffer, sendLength, true);
  i2c_master_stop(cmd);
  if (i2c_master_cmd_begin(I2C_PORT, cmd, TWI_SEND_TIMEOUT_TICKS) == ESP_OK) {
    result = true;
  }
  i2c_cmd_link_delete(cmd);
  return result;
}


bool TWIDriver::_recieve(uint8_t slaveAddress, uint8_t *buffer, uint8_t receiveLength)
{
  bool result = false;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  uint8_t address = static_cast<uint8_t>(slaveAddress << 1u); // LSB is reserved for R/W bit
  i2c_master_write_byte(cmd, (address | I2C_MASTER_READ), true);
  i2c_master_read(cmd, buffer, receiveLength, I2C_MASTER_ACK);
  i2c_master_stop(cmd);
  if (i2c_master_cmd_begin(I2C_PORT, cmd, TWI_SEND_TIMEOUT_TICKS) == ESP_OK) {
    result = true;
  }
  i2c_cmd_link_delete(cmd);
  return result;
}

bool TWIDriver::send(uint8_t slaveAddress, uint8_t *buffer, uint8_t sendLength)
{
  bool result = m_lock.take(TWI_LOCK_TIMEOUT_TICKS);

  if (result) {
    result = _send(slaveAddress, buffer, sendLength);
    m_lock.give();
  }

  return result;
}

bool TWIDriver::recieve(uint8_t slaveAddress, uint8_t *buffer, uint8_t receiveLength)
{
  bool result = m_lock.take(TWI_LOCK_TIMEOUT_TICKS);

  if (result) {
    result = _recieve(slaveAddress, buffer, receiveLength);
    m_lock.give();
  }

  return result;
}

bool TWIDriver::sendRecieve(uint8_t slaveAddress, uint8_t *buffer, uint8_t sendLength, uint8_t receiveLength)
{
  bool result = true;

  if (result){
    if (sendLength > 0u) {
      result = send(slaveAddress, buffer, sendLength);
    }
    if ((receiveLength > 0u) && (result)) {
      result = recieve(slaveAddress, buffer, receiveLength);
    }
  }
  return result;
}
