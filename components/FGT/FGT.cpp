#include "EspCpp.hpp"
#include <cstring>
#include <algorithm>

#include "FGT.h"
#include "Constants.h"
#include "StringConverter.h"
#include "ButtonStatus.h"

namespace
{
  const char * LOG_TAG = "FGT";

  const uint16_t BLE_APP_ID = 0x55u;

  void generateAdvertisingName(char *advertisingName, const uint8_t *mac)
  {
    // Type
    advertisingName[0] = 'P'; // Panel
    advertisingName[1] = 'H'; // Heater

    // Aaddress
    advertisingName[2] = ((mac[4]>>4) % 0xa) + '0';
    advertisingName[3] = ((mac[4]&0x0f) % 0xa) + '0';
    advertisingName[4] = ((mac[5]>>4) % 0xa) + '0';
    advertisingName[5] = ((mac[5]&0x0f) % 0xa) + '0';
    advertisingName[6] = '\0';
  }

  #define DEC_TO_ASCII_HEX(num) (num > 9 ? num + 55 : num + '0')
  #define ASCII_HEX_TO_DEC(num) (num <= '9' ? num - '0' : num - 55)

  // BT OTA Constants
  #define FGTBOTA_COMMAND_CHECKSUM     (0x31u)    /* Verify the checksum for the bootloadable project   */
  #define FGTBOTA_COMMAND_REPORT_SIZE  (0x32u)    /* Report the programmable portions of flash          */
  #define FGTBOTA_COMMAND_APP_STATUS   (0x33u)    /* Gets status info about the provided app status     */
  #define FGTBOTA_COMMAND_ERASE        (0x34u)    /* Erase the specified flash row                      */
  #define FGTBOTA_COMMAND_SYNC         (0x35u)    /* Sync the bootloader and host application           */
  #define FGTBOTA_COMMAND_APP_ACTIVE   (0x36u)    /* Sets the active application                        */
  #define FGTBOTA_COMMAND_DATA         (0x37u)    /* Queue up a block of data for programming           */
  #define FGTBOTA_COMMAND_ENTER        (0x38u)    /* Enter the bootloader                               */
  #define FGTBOTA_COMMAND_PROGRAM      (0x39u)    /* Program the specified row                          */
  #define FGTBOTA_COMMAND_VERIFY       (0x3Au)    /* Compute flash row checksum for verification        */
  #define FGTBOTA_COMMAND_EXIT         (0x3Bu)    /* Exits the bootloader & resets the chip             */
  #define FGTBOTA_COMMAND_GET_METADATA (0x3Cu)    /* Reports the metadata for a selected application    */

  #define FGTBOTA_SOP                  (0x01u)    /* Start of Packet */
  #define FGTBOTA_EOP                  (0x17u)    /* End of Packet */

  /*******************************************************************************
  * Bootloader packet byte addresses:
  * [1-byte] [1-byte ] [2-byte] [n-byte] [ 2-byte ] [1-byte]
  * [ SOP  ] [Command] [ Size ] [ Data ] [Checksum] [ EOP  ]
  *******************************************************************************/
  #define FGTBOTA_SOP_ADDR             (0x00u)         /* Start of packet offset from beginning     */
  #define FGTBOTA_CMD_ADDR             (0x01u)         /* Command offset from beginning             */
  #define FGTBOTA_SIZE_ADDR            (0x02u)         /* Packet size offset from beginning         */
  #define FGTBOTA_DATA_ADDR            (0x04u)         /* Packet data offset from beginning         */
  #define FGTBOTA_CHK_ADDR(x)          (0x04u + (x))   /* Packet checksum offset from end           */
  #define FGTBOTA_EOP_ADDR(x)          (0x06u + (x))   /* End of packet offset from end             */
  #define FGTBOTA_MIN_PKT_SIZE         (7u)            /* The minimum number of bytes in a packet   */

  #define FGTBOTA_STANDARD  0
  #define FGTBOTA_RSP_NO    1
  #define FGTBOTA_RSP       2

  #define FGTBOTA_CMD_RSP_NO  0     // No special flags
  #define FGTBOTA_CMD_RSP     0x80  // Special flag

  #define FGTBOTA_CMD_MASK    0xC0

  #define FGTRET_SUCCESS       (0x00u)  /* Successful */
  #define FGTRET_ERR_KEY       (0x01u)  /* The provided key does not match the expected value          */
  #define FGTRET_ERR_VERIFY    (0x02u)  /* The verification of flash failed                            */
  #define FGTRET_ERR_LENGTH    (0x03u)  /* The amount of data available is outside the expected range  */
  #define FGTRET_ERR_DATA      (0x04u)  /* The data is not of the proper form                          */
  #define FGTRET_ERR_CMD       (0x05u)  /* The command is not recognized                               */
  #define FGTRET_ERR_DEVICE    (0x06u)  /* The expected device does not match the detected device      */
  #define FGTRET_ERR_VERSION   (0x07u)  /* The bootloader version detected is not supported            */
  #define FGTRET_ERR_CHECKSUM  (0x08u)  /* The checksum does not match the expected value              */
  #define FGTRET_ERR_ARRAY     (0x09u)  /* The flash array is not valid                                */
  #define FGTRET_ERR_ROW       (0x0Au)  /* The flash row is not valid                                  */
  #define FGTRET_ERR_PROTECT   (0x0Bu)  /* The flash row is protected and can not be programmed        */
  #define FGTRET_ERR_APP       (0x0Cu)  /* The application is not valid and cannot be set as active    */
  #define FGTRET_ERR_ACTIVE    (0x0Du)  /* The application is currently marked as active               */
  #define FGTRET_ERR_UNK       (0x0Fu)  /* An unknown error occurred                                   */

  #define LO8(x)                  ((uint8_t) ((x) & 0xFFu))
  #define HI8(x)                  ((uint8_t) ((uint16_t)(x) >> 8))
}

// These must be kept in sync with the calls to addCharacteristic in startGattService()
enum AttributeIndices
{
  LCD	      = 2,
  TRIAC	    = 4,
  NTC       = 6,
  EOLT	    = 8,
  TEST	    = 10,
  PASSKEY   = 12,
  GDID	    = 14,
  FULL_INFO	= 16,
  SW_VER    = 18,
  WIFI_STATUS= 20,
  OTA_EN    = 22,
  BLDR_CMD  = 24,
  BLDR_CMD_NTF  = 25
};

FGT::FGT(DeviceMeta & meta,
    BTDriverInterface &btDriver,
    Settings &settings,
    SoftwareResetDriverInterface &swResetDriver,
    CryptoDriverInterface &cryptoDriver,
    DisplayDriverInterface &displayDriver,
    TouchDriverInterface &touchDriver,
    RUInterface &regUnit,
    OTADriverInterface &otaDriver,
    WifiDriverInterface &wifiDriver):
    m_meta(meta),
    m_btDriver(btDriver),
    m_settings(settings),
    m_swResetDriver(swResetDriver),
    m_cryptoDriver(cryptoDriver),
    m_displayDriver(displayDriver),
    m_touchDriver(touchDriver),
    m_regUnit(regUnit),
    m_otaDriver(otaDriver),
    m_wifiDriver(wifiDriver),
    m_btValWifiStatus(FgtWifiStatus::NotOperating),
    m_btValCMDCcc(0u)
{
  char gdid[Constants::GDID_SIZE_MAX];
  uint8_t i;
  uint32_t passkey;

  // LCD
  memset(m_btValLCD, 0, Constants::LCD_BT_SIZE_MAX);
  memset(m_displayText, 0, Constants::TEXT_MAX_LENGTH);
  m_btValLCD[0] = 2;

  // TRIAC
  memset(m_btValTRIAC, 0, Constants::TRIAC_BT_SIZE_MAX);
  m_btValTRIAC[0] = 1;

  // Temperature Sensor
  memset(m_btValNTC, 0, Constants::NTC_BT_SIZE_MAX);
  m_btValNTC[0] = 1;

  // End of Line Test
  m_btValEOLT = m_settings.getRunMode() == RunMode::test ? 0x00 : Constants::FGT_EOLT_COMPLETE;

  // Test
  m_btValTEST = 0;

  // Passkey
  passkey = m_meta.getBluetoothPasskey();
  m_btValPASSKEY[0] = 6;
  m_btValPASSKEY[1] = (uint8_t)((uint32_t)passkey & 0xFF);
  m_btValPASSKEY[2] = (uint8_t)((uint32_t)(passkey & 0xFF00)>>8);
  m_btValPASSKEY[3] = (uint8_t)((uint32_t)(passkey & 0xFF0000)>>16);
  m_btValPASSKEY[4] = (uint8_t)((uint32_t)(passkey & 0xFF000000)>>24);

  // GDID - ASCII conversion to HEX
  meta.getGDID(gdid, sizeof(gdid));
  for(i=0; i<Constants::GDID_BT_SIZE_MAX; i++){
    m_btValGDID[i] = ASCII_HEX_TO_DEC(gdid[(Constants::GDID_BT_SIZE_MAX-1-i)*2]) << 4;
    m_btValGDID[i]|= ASCII_HEX_TO_DEC(gdid[(Constants::GDID_BT_SIZE_MAX-1-i)*2+1]);
  }

  if(!m_cryptoDriver.readSerialNumber(m_cryptoSerial)){
    memset(m_cryptoSerial, 0, 9);
  }

  // Other information
  memset(m_btValFULL_INFO, 0, Constants::FULL_INFO_BT_SIZE_MAX);

  m_triacDriver = new TriacDriver(m_regUnit);
  m_temperature = 0;

#if FGT_OTA_EN
  m_OTA_method = FGTBOTA_STANDARD;
  m_btValOTAEn = 0;
  m_OTA_row = 0;
  memset(m_btValCMD, 0, sizeof(m_btValCMD));
  m_OTA_reset_flag = 0;
#endif

  m_btValSWVer[0] = 1;
  m_btValSWVer[1] = Constants::SW_VERSION / 10u;
  m_btValSWVer[2] = Constants::SW_VERSION % 10u;
}

bool FGT::startGattService()
{
  bool success = false;
  char advertisingName[7] = {};
  uint8_t mac[6] = {};
  GattAttributeIndex indexOfFullInfoCharacteristic;

  success = m_btDriver.resetGattDb();
  if ( !success )
  {
    return false;
  }

  success = m_btDriver.addService(Constants::FGT_CONFIG_UUID, sizeof(Constants::FGT_CONFIG_UUID), false);
  if ( !success )
  {
    return false;
  }

  // LCD
  success = m_btDriver.addCharacteristic(reinterpret_cast<uint8_t *>(m_btValLCD),
      sizeof(m_btValLCD),
      Constants::LCD_UUID,
      sizeof(Constants::LCD_UUID),
      true, true, false, nullptr, nullptr,
      false);
  if ( !success )
  {
    return false;
  }

  // TRIAC
  success = m_btDriver.addCharacteristic(reinterpret_cast<uint8_t *>(m_btValTRIAC),
      sizeof(m_btValTRIAC),
      Constants::TRIAC_UUID,
      sizeof(Constants::TRIAC_UUID),
      true, true, false, nullptr, nullptr,
      false);
  if ( !success )
  {
    return false;
  }

  // Temperature Sensor
   success = m_btDriver.addCharacteristic(reinterpret_cast<uint8_t *>(m_btValNTC),
       sizeof(m_btValNTC),
       Constants::NTC_UUID,
       sizeof(Constants::NTC_UUID),
       true, true, false, nullptr, nullptr,
       false);
   if ( !success )
   {
     return false;
   }

  // EoLT State
  success = m_btDriver.addCharacteristic(reinterpret_cast<uint8_t *>(&m_btValEOLT),
      sizeof(m_btValEOLT),
      Constants::EOLT_UUID,
      sizeof(Constants::EOLT_UUID),
      true, true, false, nullptr, nullptr,
      false);
  if ( !success )
  {
    return false;
  }

  // Test Mode
  success = m_btDriver.addCharacteristic(reinterpret_cast<uint8_t *>(&m_btValTEST),
      sizeof(m_btValTEST),
      Constants::TEST_MODE_UUID,
      sizeof(Constants::TEST_MODE_UUID),
      true, true, false, nullptr, nullptr,
      false);
  if ( !success )
  {
    return false;
  }

  // Passkey
  success = m_btDriver.addCharacteristic(reinterpret_cast<uint8_t *>(&m_btValPASSKEY),
      sizeof(m_btValPASSKEY),
      Constants::PASSKEY_UUID,
      sizeof(Constants::PASSKEY_UUID),
      true, true, false, nullptr, nullptr,
      false);
  if ( !success )
  {
    return false;
  }

  // GDID
  success = m_btDriver.addCharacteristic(reinterpret_cast<uint8_t *>(m_btValGDID),
      sizeof(m_btValGDID),
      Constants::GDID_UUID,
      sizeof(Constants::GDID_UUID),
      true, true, false, nullptr, nullptr,
      false);
  if ( !success )
  {
    return false;
  }

  // Full Info
  success = m_btDriver.addCharacteristic(reinterpret_cast<uint8_t *>(m_btValFULL_INFO),
      sizeof(m_btValFULL_INFO),
      Constants::FULL_INFO_UUID,
      sizeof(Constants::FULL_INFO_UUID),
      true, true, false, nullptr, nullptr,
      false);
  if ( !success )
  {
    return false;
  }

  // SW Version
  success = m_btDriver.addCharacteristic(reinterpret_cast<uint8_t *>(m_btValSWVer),
        sizeof(m_btValSWVer),
        Constants::SW_VER_UUID,
        sizeof(Constants::SW_VER_UUID),
        true, true, false, nullptr, nullptr,
        false);
    if ( !success )
    {
      return false;
    }

  // Wifi status
  success = m_btDriver.addCharacteristic(reinterpret_cast<uint8_t *>(&m_btValWifiStatus),
        sizeof(m_btValWifiStatus),
        Constants::WIFI_STATUS_UUID,
        sizeof(Constants::WIFI_STATUS_UUID),
        true, true, false, nullptr, &indexOfFullInfoCharacteristic,
        false);
    if ( !success )
    {
      return false;
    }

    if ( indexOfFullInfoCharacteristic != AttributeIndices::WIFI_STATUS )
    {
      ESP_LOGE(LOG_TAG, "Predefined characteristic index mismatch");
      return false;
    }

#if FGT_OTA_EN
  // OTA En
  success = m_btDriver.addCharacteristic(reinterpret_cast<uint8_t *>(&m_btValOTAEn),
        sizeof(m_btValOTAEn),
        Constants::OTA_EN_UUID,
        sizeof(Constants::OTA_EN_UUID),
        true, true, false, nullptr, nullptr,
        false);
    if ( !success )
    {
      return false;
    }

  // Bootloader Command
  success = m_btDriver.addCharacteristic(reinterpret_cast<uint8_t *>(m_btValCMD),
        sizeof(m_btValCMD),
        Constants::BOOTLOADER_COMMAND,
        sizeof(Constants::BOOTLOADER_COMMAND),
        false, true, true, &m_btValCMDCcc, &indexOfFullInfoCharacteristic,
        false);
    if ( !success )
    {
      return false;
    }

    if ( indexOfFullInfoCharacteristic != AttributeIndices::BLDR_CMD )
    {
      ESP_LOGE(LOG_TAG, "Predefined characteristic index mismatch");
      return false;
    }
#endif

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

  success = m_btDriver.startBleApp(BLE_APP_ID, advertisingName, this, 0);
  if ( !success )
  {
    ESP_LOGE(LOG_TAG, "Failed to start GATT service");
  }

  m_displayText[0] = advertisingName[4];
  m_displayText[1] = advertisingName[5];

  return success;
}

void FGT::updateDatabase(void){
  m_btDriver.writeCharacteristicValue(AttributeIndices::LCD,        reinterpret_cast<uint8_t *>(m_btValLCD),        Constants::LCD_BT_SIZE_MAX);
  m_btDriver.writeCharacteristicValue(AttributeIndices::TRIAC,      reinterpret_cast<uint8_t *>(m_btValTRIAC),      Constants::TRIAC_BT_SIZE_MAX);
  m_btDriver.writeCharacteristicValue(AttributeIndices::NTC,        reinterpret_cast<uint8_t *>(m_btValNTC),        Constants::NTC_BT_SIZE_MAX);
  m_btDriver.writeCharacteristicValue(AttributeIndices::EOLT,       reinterpret_cast<uint8_t *>(&m_btValEOLT),      Constants::EOLT_BT_SIZE_MAX);
  m_btDriver.writeCharacteristicValue(AttributeIndices::TEST,       reinterpret_cast<uint8_t *>(&m_btValTEST),      Constants::TEST_BT_SIZE_MAX);
  m_btDriver.writeCharacteristicValue(AttributeIndices::PASSKEY,    reinterpret_cast<uint8_t *>(m_btValPASSKEY),    Constants::PASSKEY_BT_SIZE_MAX);
  m_btDriver.writeCharacteristicValue(AttributeIndices::GDID,       reinterpret_cast<uint8_t *>(m_btValGDID),       Constants::GDID_BT_SIZE_MAX);
  m_btDriver.writeCharacteristicValue(AttributeIndices::FULL_INFO,  reinterpret_cast<uint8_t *>(m_btValFULL_INFO),  Constants::FULL_INFO_BT_SIZE_MAX);
  m_btDriver.writeCharacteristicValue(AttributeIndices::SW_VER,     reinterpret_cast<uint8_t *>(m_btValSWVer),      Constants::SW_VER_BT_SIZE_MAX);
  m_btDriver.writeCharacteristicValue(AttributeIndices::WIFI_STATUS, reinterpret_cast<uint8_t *>(&m_btValWifiStatus), Constants::WIFI_STATUS_BT_SIZE_MAX);
#if FGT_OTA_EN
  m_btDriver.writeCharacteristicValue(AttributeIndices::OTA_EN,     reinterpret_cast<uint8_t *>(&m_btValOTAEn),     Constants::OTA_EN_BT_SIZE_MAX);
#endif
}
#if FGT_OTA_EN
static uint16_t FGTBOTA_CalcPacketChecksum(const uint8_t buffer[], uint16_t size)
{
  uint16_t sum = 0u;

  while (size > 0u)
  {
    sum += buffer[size - 1u];
    size--;
  }

  return(( uint16_t )1u + ( uint16_t )(~sum));
}
#endif
void FGT::handleCharacteristicValueChanged(GattAttributeIndex attributeIndex,
    uint16_t size, // TODO, might have to include offset param as well
    uint8_t *value)
{
  std::size_t memberMaxSize = 0U;
  uint32_t passkey = 0;
#if FGT_OTA_EN
  uint16_t len;
  uint8_t cmd;
  uint16_t pktChecksum;
  uint8_t ackCode = FGTRET_SUCCESS;
  uint8_t buf_notif[FGTBOTA_MIN_PKT_SIZE+10];
  uint32_t RXedRow;
#endif
  if(value == nullptr) return;

  switch ( attributeIndex )
  {
  case AttributeIndices::LCD:
    if (m_btValTEST)
    {
      if(value[0] <= 2)  m_btValLCD[0] = value[0];
    }
    break;

  case AttributeIndices::TRIAC:
    if(m_btValTEST){
      if((value[0] == 1) && (value[2] <= 1)){
        memcpy(m_btValTRIAC, value, 3);
      }
    }
    break;

  case AttributeIndices::NTC:
    if(value[0] == 1){
      ESP_LOGI(LOG_TAG, "Temperature: %i", m_temperature);
      m_btValNTC[0] = value[0];
      m_btValNTC[1] = (uint8_t)((int32_t)m_temperature/1000);
      m_btValNTC[2] = (uint8_t)((int32_t)((int32_t)m_temperature%1000)/100);
    }
    break;

    // End of Line Test Status
  case AttributeIndices::EOLT:
    // If test mode is set
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
    }
    m_btValEOLT = m_settings.getRunMode() == RunMode::test ? 0x00 : Constants::FGT_EOLT_COMPLETE;
    break;

    // Test Mode
  case AttributeIndices::TEST:
    // Enable Test mode - "Write enable"
    if(*value)
    {
      m_btValTEST = 1;
    }
    // If we exit Test mode - Reset the processor
    else if(m_btValTEST)
    {
      m_swResetDriver.reset();
    }
    break;

    // Passkey
  case AttributeIndices::PASSKEY:
    // If test mode is set
    if (m_btValTEST && (value[0] == 6))
    {
      passkey = (uint32_t)value[1];
      passkey |= (uint32_t)value[2] << 8;
      passkey |= (uint32_t)value[3] << 16;
      passkey |= (uint32_t)value[4] << 24;

      if(passkey <= 999999){
        m_meta.setBluetoothPasskey(passkey);
        memcpy(m_btValPASSKEY, value, 5);
      }
    }
    break;

  case AttributeIndices::GDID:
    char gdid[Constants::GDID_SIZE_MAX];
    uint8_t i;

    // If test mode is set
    if(m_btValTEST){
      memberMaxSize = Constants::GDID_BT_SIZE_MAX;
      memcpy(m_btValGDID, value, std::min(static_cast<std::size_t>(size), memberMaxSize));

      ESP_LOGI(LOG_TAG, "GDID value received: %02X %02X %02X %02X %02X %02X",
          m_btValGDID[5], m_btValGDID[4], m_btValGDID[3], m_btValGDID[2], m_btValGDID[1], m_btValGDID[0]);

      for(i=0; i<Constants::GDID_BT_SIZE_MAX; i++){
        gdid[i*2]   = DEC_TO_ASCII_HEX( ((m_btValGDID[Constants::GDID_BT_SIZE_MAX-1-i] & 0xF0) >> 4));
        gdid[i*2+1] = DEC_TO_ASCII_HEX( ( m_btValGDID[Constants::GDID_BT_SIZE_MAX-1-i] & 0x0F));
      }
      gdid[i*2] = 0;  // Trailing zero
      // Save GDID
      m_meta.setGDID(gdid);
    }
    break;

  case AttributeIndices::FULL_INFO:
    // It should be used for ECC serial number exchange, URLs and the rest...

    memset(m_btValFULL_INFO, 0, Constants::FULL_INFO_BT_SIZE_MAX);
    m_btValFULL_INFO[0] = value[0];
    m_btValFULL_INFO[1] = value[1];

    // Parameter Switch
    // ECC Serial
    if(value[1] == 1){
      memcpy(&m_btValFULL_INFO[2], m_cryptoSerial, 9);
    }

    break;

  case AttributeIndices::SW_VER:

    break;

  case AttributeIndices::WIFI_STATUS:
    ESP_LOGI(LOG_TAG, "Wifi status value received: %u", static_cast<uint32_t>(*value));
    if ( m_btValWifiStatus != FgtWifiStatus::ScanningInProgress )
    {
      if ( *value == FgtWifiStatus::ScanningInProgress )
      {
        ESP_LOGI(LOG_TAG, "Configuring wifi test");
        m_btValWifiStatus = FgtWifiStatus::ScanningInProgress;
      }
    }
    break;
#if FGT_OTA_EN
  case OTA_EN:
    ESP_LOGI(LOG_TAG, "OTA Enable: %u", value[0]);
    m_btValOTAEn = value[0];
    break;

  case BLDR_CMD_NTF:
    ESP_LOGI(LOG_TAG, "OTA Notifications: %u %u", value[0], value[1]);
    break;

  case BLDR_CMD:
    if(!m_btValOTAEn) break;

    //ESP_LOGI(LOG_TAG, "OTA Bootloader CMD");
    do {
      if(size < FGTBOTA_MIN_PKT_SIZE){
        ackCode = FGTRET_ERR_LENGTH;
        ESP_LOGE(LOG_TAG, "OTA CMD Length");
        break;
      }

      len = ((uint16_t)((uint16_t)value[FGTBOTA_SIZE_ADDR + 1u] << 8u)) |
                                  value[FGTBOTA_SIZE_ADDR];

      if((size == len + FGTBOTA_MIN_PKT_SIZE) && \
        (value[0] == 0x01) && \
        (value[FGTBOTA_EOP_ADDR(len)] == 0x17)) {

        // Choose OTA Method
        cmd = value[FGTBOTA_CMD_ADDR];

        if( (cmd  & FGTBOTA_CMD_MASK) == FGTBOTA_CMD_RSP_NO){
          m_OTA_method = FGTBOTA_RSP_NO;
        } else if((cmd  & FGTBOTA_CMD_MASK) == FGTBOTA_CMD_RSP){
          m_OTA_method = FGTBOTA_RSP;
        }
        value[FGTBOTA_CMD_ADDR] &=~ FGTBOTA_CMD_MASK;
        cmd = value[FGTBOTA_CMD_ADDR];

        pktChecksum = ((uint16_t)((uint16_t)value[FGTBOTA_CHK_ADDR(len) + 1u] << 8u)) |
                                            value[FGTBOTA_CHK_ADDR(len)];

        if(pktChecksum != FGTBOTA_CalcPacketChecksum(value, len + FGTBOTA_DATA_ADDR)) {
          ackCode = FGTRET_ERR_CHECKSUM;
          ESP_LOGE(LOG_TAG, "OTA CMD Checksum");
          break;
        }

        switch(cmd) {
          case FGTBOTA_COMMAND_ENTER:
            ESP_LOGI(LOG_TAG, "OTA Bootloader Enter");
            m_otaDriver.resetAndInit();
            if(!m_otaDriver.beginUpdate()){
              ackCode = FGTRET_ERR_DEVICE;
            }
            m_OTA_row = 0;

            size = 8;
            // Silicon ID
            buf_notif[FGTBOTA_DATA_ADDR]    = '3';
            buf_notif[FGTBOTA_DATA_ADDR+1]  = 'P';
            buf_notif[FGTBOTA_DATA_ADDR+2]  = 'S';
            buf_notif[FGTBOTA_DATA_ADDR+3]  = 'E';
            // Revision
            buf_notif[FGTBOTA_DATA_ADDR+4] = 1;
            // Bootloader Versioin
            buf_notif[FGTBOTA_DATA_ADDR+5] = 0;
            buf_notif[FGTBOTA_DATA_ADDR+6] = 0;
            buf_notif[FGTBOTA_DATA_ADDR+7] = 1;
            break;

          case FGTBOTA_COMMAND_PROGRAM:
            RXedRow = value[FGTBOTA_DATA_ADDR + 0];
            RXedRow |= (uint32_t)value[FGTBOTA_DATA_ADDR + 1]<<8;
            RXedRow |= (uint32_t)value[FGTBOTA_DATA_ADDR + 2]<<16;

            //ESP_LOGI(LOG_TAG, "OTA Bootloader Program %u RXed %u", m_OTA_row, RXedRow);

            if(RXedRow != m_OTA_row){
              ackCode = FGTRET_ERR_ROW;
            }

            m_OTA_row++;

            if(!m_otaDriver.write((const char *)&value[FGTBOTA_DATA_ADDR + 3], len-3)){
              ackCode = FGTRET_ERR_DEVICE;
            }

            size = 0;
            break;

          case FGTBOTA_COMMAND_EXIT:
            ESP_LOGI(LOG_TAG, "OTA Bootloader Exit");
            m_OTA_reset_flag = 1;
            size = 0;
            break;

          default:
            ackCode = FGTRET_ERR_CMD;
            ESP_LOGE(LOG_TAG, "OTA CMD Unknown CMD: %u, Len: %u", cmd, len);
            break;
        }

      } else {
        ackCode = FGTRET_ERR_DATA;
        ESP_LOGE(LOG_TAG, "OTA CMD Data");
        size = 0;
        break;
      }
        if ((m_OTA_method == FGTBOTA_RSP_NO) && (ackCode == FGTRET_SUCCESS)){
          // Every time clear method flag
          m_OTA_method = FGTBOTA_STANDARD;
          break;
        }

        /* Start of packet. */
        buf_notif[FGTBOTA_SOP_ADDR]        = FGTBOTA_SOP;
        buf_notif[FGTBOTA_CMD_ADDR]        = ackCode;
        buf_notif[FGTBOTA_SIZE_ADDR]       = LO8(size);
        buf_notif[FGTBOTA_SIZE_ADDR + 1u]  = HI8(size);

        /* Compute checksum. */
        pktChecksum = FGTBOTA_CalcPacketChecksum(buf_notif, size + FGTBOTA_DATA_ADDR);

        buf_notif[FGTBOTA_CHK_ADDR(size)]       = LO8(pktChecksum);
        buf_notif[FGTBOTA_CHK_ADDR(1u + size)]  = HI8(pktChecksum);
        buf_notif[FGTBOTA_EOP_ADDR(size)]       = FGTBOTA_EOP;

    //  DBG_PRINT_TEXT("\n\rBT TX ");
    //  DBG_PRINT_DEC(size + BootloaderEmulator_MIN_PKT_SIZE);
    //  DBG_PRINT_TEXT("  : ");
    //  DBG_PRINT_ARRAY(buffer, size + BootloaderEmulator_MIN_PKT_SIZE);
    //  DBG_PRINT_TEXT("\r\n");

      if(m_OTA_method == FGTBOTA_STANDARD){
          /* Start packet transmit. */
          //return(CyBtldrCommWrite(buffer, size + BootloaderEmulator_MIN_PKT_SIZE, &size, 150u));
      } else {
        ESP_LOGI(LOG_TAG, "OTA CMD Notify sz: %u ack: 0x%02X", size, ackCode);
        m_btDriver.sendCharacteristicNotification(AttributeIndices::BLDR_CMD,
                                 reinterpret_cast<const uint8_t *>(buf_notif),
                                 size + FGTBOTA_MIN_PKT_SIZE);
        m_OTA_method = FGTBOTA_STANDARD;
      }
    } while(0);



    break;
#endif
  default:
    ESP_LOGI(LOG_TAG, "Unhandled characteristic value changed!");
    break;
  }

  updateDatabase();

  if (m_settings.isModified())
  {
    m_settings.storeSettings();
  }

  if (m_meta.isModified())
  {
    m_meta.storeMeta();
  }
}

void FGT::stopGattService()
{
  m_btDriver.resetAndInit();
}

void FGT::runTest(void){
  uint8_t displayData[3] = {};
  bool isButtonTouched[NUMBER_OF_BUTTONS];
  ESP_LOGI(LOG_TAG, "Entering EoLT Loop 2023, EoLT flag: %02X", m_btValEOLT);
  uint8_t timeout = 0;

  while(1)
  {
    FreeRTOS_delay_ms(50u);
    // While not connected display Two digits and blink blue LED
    if(!m_btDriver.isConnected()){
      displayData[2] ^= 0b1000u;
      StringConverter::convert((const char*)m_displayText, displayData, Constants::TEXT_MAX_LENGTH-1u);
      m_displayDriver.setData(displayData);
      // Turn off the triac
      m_btValTRIAC[1] = 0;
#if FGT_OTA_EN
      // Reset OTA
      m_btValOTAEn = 0;
#endif
      FreeRTOS_delay_ms(450u);
      timeout++; // Half of second increment

      m_touchDriver.getTouchStatus(isButtonTouched);

      if(timeout > 120) {
        ESP_LOGI(LOG_TAG, "Timeout. Entering normal mode.");
        break;
      }

      if (isButtonTouched[Constants::PLUS_BUTTON_INDEX] && isButtonTouched[Constants::MINUS_BUTTON_INDEX])
      {
        ESP_LOGI(LOG_TAG, "Aborted by key press. Entering normal mode.");
        break;
      }

    // If connected - display depending on LCD characteristic value
    } else {
      timeout = 0;

      // All off (except Bluetooth LED)
      if(m_btValLCD[0] == 0){
        displayData[0] = 0;
        displayData[1] = 0;
        displayData[2] = 0b1000u;
      // All on
      } else  if(m_btValLCD[0] == 1){
        displayData[0] = 0x7F;
        displayData[1] = 0x7F;
        displayData[2] = 0x0F;
      // Depending on button press
      } else {
        displayData[0] = 0x00;
        displayData[2] = 0b1000u;
        // Display if the heating element is on
        if(m_regUnit.isHeatElementActive()){
          displayData[2] |= 0b1u;
        }
        m_touchDriver.getTouchStatus(isButtonTouched);

        if(  isButtonTouched[Constants::PLUS_BUTTON_INDEX] &&
            (!isButtonTouched[Constants::MINUS_BUTTON_INDEX]) &&
            (!isButtonTouched[Constants::BACK_BUTTON_INDEX]) ){
          StringConverter::convert((const char*)"1", &displayData[1], 1);
        } else if(   (!isButtonTouched[Constants::PLUS_BUTTON_INDEX]) &&
            (isButtonTouched[Constants::MINUS_BUTTON_INDEX]) &&
            (!isButtonTouched[Constants::BACK_BUTTON_INDEX]) ){
          StringConverter::convert((const char*)"2", &displayData[1], 1);
        } else if(   (!isButtonTouched[Constants::PLUS_BUTTON_INDEX]) &&
            (!isButtonTouched[Constants::MINUS_BUTTON_INDEX]) &&
            (isButtonTouched[Constants::BACK_BUTTON_INDEX]) ){
          StringConverter::convert((const char*)"3", &displayData[1], 1);
        } else if(   (!isButtonTouched[Constants::PLUS_BUTTON_INDEX]) &&
            (!isButtonTouched[Constants::MINUS_BUTTON_INDEX]) &&
            (!isButtonTouched[Constants::BACK_BUTTON_INDEX]) ){
          displayData[1] = 0x00;
        } else {
          StringConverter::convert((const char*)"--", &displayData[0], 2);
        }

      }
      m_displayDriver.setData(displayData);
#if FGT_OTA_EN
      if(m_btValOTAEn && m_OTA_reset_flag){
        m_OTA_reset_flag = 0;
        if(m_otaDriver.endUpdate() && m_otaDriver.activateUpdatedPartition()){
          ESP_LOGI(LOG_TAG, "OTA Reset");
          m_swResetDriver.reset();
        } else {
          ESP_LOGI(LOG_TAG, "OTA Error");
        }
      }
#endif
    }

    // Triac control
    if(m_btValTRIAC[1]) {
      m_triacDriver->turnOn();
    } else {
      m_triacDriver->turnOff();
    }

    // Update Temperature
    m_temperature = m_regUnit.getEstimatedRoomTemperatureFP();
    if((m_btValNTC[0] == 1) && (m_temperature != INT32_MIN)){
      m_btValNTC[1] = (uint8_t)((int32_t)m_temperature/1000);
      m_btValNTC[2] = (uint8_t)((int32_t)((int32_t)m_temperature%1000)/100);
    }

    if ( m_btValWifiStatus == FgtWifiStatus::ScanningInProgress )
    {
      performWifiTest();
    }

    updateDatabase();

    // If Disconnected in the Test Mode - Reset, because changes possibly were made
    if (m_btValTEST && (!m_btDriver.isConnected()))
    {
      m_swResetDriver.reset();
    }

  }
}

void FGT::performWifiTest(void)
{
  ESP_LOGI(LOG_TAG, "Starting wifi test");

  bool wifiResult = false;
  wifiResult = m_wifiDriver.resetAndInit();
  if ( !wifiResult )
  {
    m_btValWifiStatus = FgtWifiStatus::ConnectionFailed;
    return;
  }

  wifiResult = m_wifiDriver.configureConnection(Constants::FGT_WIFI_TEST_SSID,
                                                Constants::FGT_WIFI_TEST_PASSWORD);
  if ( !wifiResult )
  {
    m_btValWifiStatus = FgtWifiStatus::ConnectionFailed;
    return;
  }

  wifiResult = m_wifiDriver.connectToWlan();
  if ( wifiResult )
  {
    ESP_LOGI(LOG_TAG, "Wifi test successful");
    m_btValWifiStatus = FgtWifiStatus::ConnectionSuccessful;
    m_wifiDriver.disconnectFromWlan();
  }
  else
  {
    m_btValWifiStatus = FgtWifiStatus::ConnectionFailed;
    m_wifiDriver.disconnectFromWlan();
    ESP_LOGE(LOG_TAG, "Wifi test connect failed");
  }
}
