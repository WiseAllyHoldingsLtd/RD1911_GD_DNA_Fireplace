#include "HardwareSetup.h"
#include "EspCpp.hpp"

#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/spi_master.h"

static const char *LOG_TAG = "HWSetup";


namespace {
  typedef struct {
    ledc_channel_t channel;
    gpio_num_t gpio_num;
  } LEDCMap_t;

  const LEDCMap_t LEDC_MAPPING[] = {
    { DISPLAY_SEG_A_LEDC, DISPLAY_SEG_A_PIN},
    { DISPLAY_SEG_B_LEDC, DISPLAY_SEG_B_PIN},
    { DISPLAY_SEG_C_LEDC, DISPLAY_SEG_C_PIN},
    { DISPLAY_SEG_D_LEDC, DISPLAY_SEG_D_PIN},
    { DISPLAY_SEG_E_LEDC, DISPLAY_SEG_E_PIN},
    { DISPLAY_SEG_F_LEDC, DISPLAY_SEG_F_PIN},
    { DISPLAY_SEG_G_LEDC, DISPLAY_SEG_G_PIN}
  };
  const uint8_t LEDC_MAPPING_LENGTH = 7u;
}

void HardwareSetup::setup() {
  nvs_flash_init();
  setupDisplay();
  setupTouch();
  setupSPI();
}

void HardwareSetup::setupDisplay()
{
  // Set JTAG pins as GPIO instead
  gpio_pad_select_gpio(DISPLAY_DIG_2_PIN);
  gpio_pad_select_gpio(DISPLAY_SEG_C_PIN);
  gpio_pad_select_gpio(DISPLAY_SEG_D_PIN);
  gpio_pad_select_gpio(DISPLAY_SEG_G_PIN);

  gpio_set_direction(DISPLAY_DIG_2_PIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(DISPLAY_DIG_3_PIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(DISPLAY_DIG_4_PIN, GPIO_MODE_OUTPUT);

  // Segments use PWM LED controller:
  // Prepare and set configuration of timer
  ledc_timer_config_t ledc_timer;
  ledc_timer.duty_resolution = static_cast<ledc_timer_bit_t>(8u);  // resolution of PWM duty
  ledc_timer.freq_hz = 5000u;  // frequency of PWM signal
  ledc_timer.speed_mode = LEDC_HIGH_SPEED_MODE;
  ledc_timer.timer_num = LEDC_TIMER_0;

  // Set configuration of timer0 for high speed channels
  ledc_timer_config(&ledc_timer);

  ledc_channel_config_t ledc_channel;
  ledc_channel.duty = DISPLAY_LEDC_MAX_DUTY;
  ledc_channel.speed_mode = LEDC_HIGH_SPEED_MODE;
  ledc_channel.timer_sel = LEDC_TIMER_0;

  for (uint8_t i(0u); i != LEDC_MAPPING_LENGTH; ++i) {
    ledc_channel.channel = LEDC_MAPPING[i].channel;
    ledc_channel.gpio_num = LEDC_MAPPING[i].gpio_num;
    ledc_channel_config(&ledc_channel);
  }
}

void HardwareSetup::setupTouch()
{
  gpio_set_direction(QTOUCH_SW1_PIN, GPIO_MODE_INPUT);
  gpio_set_pull_mode(QTOUCH_SW1_PIN, GPIO_PULLUP_ONLY);

  gpio_set_direction(QTOUCH_SW2_PIN, GPIO_MODE_INPUT);
  gpio_set_pull_mode(QTOUCH_SW2_PIN, GPIO_PULLUP_ONLY);

  gpio_set_direction(QTOUCH_SW3_PIN, GPIO_MODE_INPUT);
  gpio_set_pull_mode(QTOUCH_SW3_PIN, GPIO_PULLUP_ONLY);
}

void HardwareSetup::setupSPI()
{
  // Set CS high (inactive)
  gpio_set_level(TEST_SPI_CS_PIN, 1u);

  spi_bus_config_t buscfg = {};
  buscfg.miso_io_num = TEST_SPI_MISO_PIN;
  buscfg.mosi_io_num = TEST_SPI_MOSI_PIN;
  buscfg.sclk_io_num = TEST_SPI_SCLK_PIN;
  buscfg.quadhd_io_num = -1;
  buscfg.quadwp_io_num = -1;

  //Initialize the SPI bus
  if (spi_bus_initialize(HSPI_HOST, &buscfg, 1) != ESP_OK) {
    ESP_LOGE(LOG_TAG, "Failed to initialize SPI bus");
  }
}
