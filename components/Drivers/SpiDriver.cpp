#include <cstring>

#include "esp_log.h"

#include "SpiDriver.h"
#include "Constants.h"
#include "HardwareSetup.h"

static const char *LOG_TAG = "SPI";


SpiDriver::SpiDriver()
: m_sysTestDevice(nullptr)
{
}

SpiDriver::~SpiDriver()
{
  freeDevices();
}

void SpiDriver::freeDevices()
{
  if (m_sysTestDevice != nullptr) {
    if (spi_bus_remove_device(m_sysTestDevice) != ESP_OK) {
      ESP_LOGW(LOG_TAG, "Could not remove SysTest SPI device");
    }
    m_sysTestDevice = nullptr;
  }
}

void SpiDriver::resetAndInit(void)
{
  freeDevices();

  // Initialize SysTest SPI device
  spi_device_interface_config_t devcfg = {};
  devcfg.clock_speed_hz = 115200;
  devcfg.mode = 0;  //SPI mode 0
  devcfg.spics_io_num = TEST_SPI_CS_PIN;
  devcfg.queue_size = 2; // number of queued transactions
//  devcfg.post_cb = FUNC; // We could add callback function here. And bu using transaction.user field we could know which type of request it is

  if (spi_bus_add_device(HSPI_HOST, &devcfg, &m_sysTestDevice) != ESP_OK) {
    ESP_LOGE(LOG_TAG, "Failed to add SysTest SPI device");
  }
}

void SpiDriver::sendReceive(SpiTarget::Enum target, uint8_t sendBuffer[], uint8_t receiveBuffer[], uint8_t bufferLength)
{
  spi_transaction_t transaction = {};
  transaction.tx_buffer = sendBuffer;
  transaction.length = (Constants::SPI_SYSTEM_TEST_BUFFER_LENGTH * 8u);  // length in bits
  transaction.rx_buffer = receiveBuffer;
  transaction.rxlength = 0; // uses same as .length

  if (target == SpiTarget::SystemTest) {
    if (spi_device_transmit(m_sysTestDevice, &transaction) != ESP_OK) {
      ESP_LOGE(LOG_TAG, "Failed to sendReceive");
    }
  }
  // Other targets must be implemented here
}
