#include "ViewModel.h"
#include "Constants.h"
#include "StaticStrings.h"
#include <stdio.h>

//for memset
#include <string.h>


ViewModel::ViewModel(UIState uiState,
                     SettingsInterface &settings,
                     DeviceMetaInterface &deviceMeta,
                     bool isHeatCurrentlyOn,
                     bool isScreenFullyDimmed)
: m_uiState(uiState), m_yellowLED(false), m_RGBRed(false), m_RGBGreen(false), m_RGBBlue(false)
{
  memset(m_textString,  static_cast<int32_t>(' '), sizeof(m_textString));
  m_textString[static_cast<uint8_t>(Constants::TEXT_MAX_LENGTH - 1u)] =  '\0';
  switch(m_uiState.currentScreen)
  {
  case Screen::mainScreen:
    createMainScreenViewModel(settings.getSetPoint(),
      settings.getHeatingMode(),
      isHeatCurrentlyOn,
      isScreenFullyDimmed,
      settings.isOverrideActive()
      );
    break;
  case Screen::buttonTestScreen:
    createButtonTestScreen();
    break;
  case Screen::bluetoothSetupScreen:
    createBluetoothSetupScreen(deviceMeta);
    break;
  case Screen::firmwareUpgradeScreen:
    createFirmwareUpgradeScreen();
    break;
  case Screen::swVersionScreen:
    createSWVersionScreen();
    break;
  case Screen::buttonLockedScreen:
    createButtonLockedScreen();
    break;
  case Screen::buttonUnlockedScreen:
    createButtonUnlockedScreen();
    break;
  case Screen::factoryResetScreen:
    // we need to break this, cause it's still supposed to run but not show anything on screen
    createFactoryResetScreen();
    break;
  case Screen::confirmFactoryResetScreen:
    createFactoryResetConfirmScreen();
    break;

  // TODO: remaining screens

  case Screen::undefinedScreen:
  default:
    // We should not show the main screen if the screen is not on this list. It f*** up some system tests.
    while (true)
    {
      // Trigger watch-doggy
      // FIXME: other method on FreeRTOS?
    }
    break;
  }
}

ViewModel::ViewModel(const char textString[]):
  m_yellowLED(false), m_RGBRed(false), m_RGBGreen(false), m_RGBBlue(false)
{
  memset(m_textString,  static_cast<int32_t>(' '), sizeof(m_textString));
  m_textString[static_cast<uint8_t>(Constants::TEXT_MAX_LENGTH - 1u)] =  '\0';

  strncpy(m_textString, textString, Constants::TEXT_MAX_LENGTH - 1u);
  m_textString[Constants::TEXT_MAX_LENGTH - 1u] = static_cast<char>('\0'); // Null-terminate
}

void ViewModel::createMainScreenViewModel(int32_t setPoint,
  HeatingMode::Enum heatingMode,
  bool isHeatCurrentlyOn,
  bool isScreenFullyDimmed,
  bool isOverrride
  )
{
  if (!isScreenFullyDimmed) {
    if ((setPoint == Constants::HEATER_OFF_SET_POINT) || (heatingMode == static_cast<int32_t>(HeatingMode::off))) {
      strncpy(m_textString, StaticStrings::HEATER_OFF_TEXT, Constants::TEXT_MAX_LENGTH - 1u);
    }
    else {
      snprintf(m_textString, static_cast<uint8_t>(Constants::TEXT_MAX_LENGTH), StaticStrings::SET_POINT_FORMAT, (setPoint / 1000));
    }
  }

  // Heating indicator
  if (isHeatCurrentlyOn) {
    m_yellowLED = true;
  }

  // Mode indicator
  if (!isOverrride || blink(1000u, m_uiState.currentScreenEnterTimestampMs)) {
    if (heatingMode == static_cast<int32_t>(HeatingMode::comfort)) {
      m_RGBRed = true;
    }
    else if (heatingMode ==  static_cast<int32_t>(HeatingMode::eco)) {
      m_RGBGreen = true;
    }
    else if (heatingMode == static_cast<int32_t>(HeatingMode::antiFrost)) {
      m_RGBBlue = true;
    }
    else {}
  }

}

void ViewModel::createButtonTestScreen()
{
  snprintf(m_textString, static_cast<uint8_t>(Constants::TEXT_MAX_LENGTH), StaticStrings::BUTTON_TEST_TEXT);
}

void ViewModel::createBluetoothSetupScreen(DeviceMetaInterface &deviceMeta)
{
  enum BtPages : uint8_t
  {
    btEnter = 0u,
    gdidStart,
    gdid0,
    gdid1,
    gdid2,
    gdid3,
    gdid4,
    gdid5,
    pinStart,
    pin0,
    pin1,
    pin2,
    numPages
  } pageId =
      static_cast<BtPages>(nStateBlinkWithPause(Constants::BLUETOOTH_SCROLL_PAGE_DURATION_MS,
                                                m_uiState.currentScreenEnterTimestampMs,
                                                numPages,
                                                Constants::BLUETOOTH_SCROLL_PAGE_PAUSE_MS));

  char gdidString[Constants::GDID_SIZE_MAX] = {};
  char btPasskeyString[Constants::BT_PASSKEY_STRING_SIZE_MAX] = {};
  deviceMeta.getGDID(gdidString, sizeof(gdidString));
  uint32_t btPasskey = deviceMeta.getBluetoothPasskey();
  snprintf(btPasskeyString,
           sizeof(btPasskeyString),
           "%06u",
           btPasskey);

  uint8_t gdidCharacterIndex = 0u;
  uint8_t passkeyCharacterIndex = 0u;

  switch ( pageId )
  {
  case BtPages::btEnter:
    snprintf(m_textString, static_cast<uint8_t>(Constants::TEXT_MAX_LENGTH), StaticStrings::BLUETOOTH_SETUP_TEXT);
    break;
  case BtPages::gdidStart:
    snprintf(m_textString, static_cast<uint8_t>(Constants::TEXT_MAX_LENGTH), StaticStrings::GDID_START_TEXT);
    break;
  case BtPages::gdid0:
  case BtPages::gdid1:
  case BtPages::gdid2:
  case BtPages::gdid3:
  case BtPages::gdid4:
  case BtPages::gdid5:
    gdidCharacterIndex = (pageId-BtPages::gdid0)*2u;
    snprintf(m_textString,
             static_cast<uint8_t>(Constants::TEXT_MAX_LENGTH),
             StaticStrings::GDID_NUMBERS_TEXT,
             gdidString[gdidCharacterIndex],
             gdidString[gdidCharacterIndex+1u]);
    break;
  case BtPages::pinStart:
    snprintf(m_textString, static_cast<uint8_t>(Constants::TEXT_MAX_LENGTH), StaticStrings::PIN_START_TEXT);
    break;
  case BtPages::pin0:
  case BtPages::pin1:
  case BtPages::pin2:
    passkeyCharacterIndex = (pageId-BtPages::pin0)*2u;
    snprintf(m_textString,
             static_cast<uint8_t>(Constants::TEXT_MAX_LENGTH),
             StaticStrings::PIN_NUMBERS_TEXT,
             btPasskeyString[passkeyCharacterIndex],
             btPasskeyString[passkeyCharacterIndex+1u]);
    break;
  case BtPages::numPages:
    // pause
    m_textString[0] = 0u;
    break;
  default:
    snprintf(m_textString, static_cast<uint8_t>(Constants::TEXT_MAX_LENGTH), StaticStrings::BLUETOOTH_SETUP_TEXT);
    break;
  }

  //snprintf(m_textString, static_cast<uint8_t>(Constants::TEXT_MAX_LENGTH), StaticStrings::BLUETOOTH_SETUP_TEXT);

  // LED constant on if bluetooth hasn't been activated, blink slowly when activated but no connection, blink fast if connected
  if (!m_uiState.isBluetoothActive)
  {
    m_RGBBlue = true;
  }
  else
  {
    if (m_uiState.isBluetoothConnected)
    {
      m_RGBBlue = blink(Constants::BLUETOOTH_LED_PERIOD_CONNECTED_MS, m_uiState.currentScreenEnterTimestampMs);
    }
    else
    {
      m_RGBBlue = blink(Constants::BLUETOOTH_LED_PERIOD_NO_CONNECTION_MS, m_uiState.currentScreenEnterTimestampMs);
    }
  }
}

void ViewModel::createFirmwareUpgradeScreen()
{
  // We shall display an 'F' followed by horizontal line going in "loop" upwards with on blank between top => bottom
  uint8_t quadStateValue = quadStateBlink(1000u, m_uiState.currentScreenEnterTimestampMs);
  char lineChar;

  switch (quadStateValue)
  {
  case 1U:
    lineChar = '_';
    break;
  case 2U:
    lineChar = '-';
    break;
  case 3U:
    lineChar = '^';
    break;
  default:
    lineChar = ' ';
    break;
  }

  snprintf(m_textString, static_cast<uint8_t>(Constants::TEXT_MAX_LENGTH), StaticStrings::FW_UPGRADE_FORMAT, lineChar);
}

void ViewModel::createSWVersionScreen()
{
  uint16_t changeViewTime = (1000U * Constants::SW_VERSION_SCREEN_VISIBLE_TIME_S) / 3U;

  enum SwDisplayState: uint8_t
  {
    Major=0u,
    Minor,
    Test
  } swVersionDisplayState =
      static_cast<SwDisplayState>(nStateBlink(changeViewTime, m_uiState.currentScreenEnterTimestampMs, 3u));

  // TODO split the SW_VERSION constant into 2 uint8_t constants and change how they are used all over the code base
  uint8_t versionMajor = Constants::SW_VERSION / static_cast<uint8_t>(10u);
  uint8_t versionMinor = Constants::SW_VERSION % static_cast<uint8_t>(10u);
  uint8_t versionToShow = 0u;
  switch ( swVersionDisplayState )
  {
  case SwDisplayState::Major:
    versionToShow = versionMajor;
    break;
  case SwDisplayState::Minor:
    versionToShow = versionMinor;
    break;
  case SwDisplayState::Test: // Show test number for any unexpected states (fall-through)
  default:
    versionToShow = Constants::SW_TEST_NO;
    break;
  }
  snprintf(m_textString,
           static_cast<uint8_t>(Constants::TEXT_MAX_LENGTH),
           StaticStrings::SW_VERSION_FORMAT,
           versionToShow);

  if (blink(1000u, m_uiState.currentScreenEnterTimestampMs))
  {
    if (m_uiState.isAzureConnected)
    {
      m_RGBGreen = true;
    }
    else if (m_uiState.isWifiConnected)
    {
      m_RGBRed = true;
      m_RGBGreen = true;
    }
    else
    {
      m_RGBRed = true;
    }
  }
}

void ViewModel::createButtonLockedScreen()
{
  if (blinkFromActivity(1000u)) {
    snprintf(m_textString, static_cast<uint8_t>(Constants::TEXT_MAX_LENGTH), StaticStrings::BUTTON_LOCK_ENABLED_TEXT);
  }
}

void ViewModel::createButtonUnlockedScreen()
{
  if (blinkFromActivity(1000u)) {
    snprintf(m_textString, static_cast<uint8_t>(Constants::TEXT_MAX_LENGTH), StaticStrings::BUTTON_LOCK_DISABLED_TEXT);
  }
}

void ViewModel::createFactoryResetScreen()
{
  if (blinkFromActivity(1000u))
  {
    snprintf(m_textString, static_cast<uint8_t>(Constants::TEXT_MAX_LENGTH), StaticStrings::FACTORY_RESET);
  }
}

void ViewModel::createFactoryResetConfirmScreen()
{
  if (blinkFromActivity(1000u))
  {
    snprintf(m_textString, static_cast<uint8_t>(Constants::TEXT_MAX_LENGTH), StaticStrings::FACTORY_RESET_CONFIRMED);
  }
}

const char * ViewModel::getTextString() const
{
  return m_textString;
}

bool ViewModel::isYellowLEDActive() const
{
  return m_yellowLED;
}

bool ViewModel::isRGBRedActive() const
{
  return m_RGBRed;
}

bool ViewModel::isRGBGreenActive() const
{
  return m_RGBGreen;
}

bool ViewModel::isRGBBlueActive() const
{
  return m_RGBBlue;
}


bool ViewModel::blinkFromActivity(uint32_t periodMS) const
{
  return blink(periodMS, m_uiState.timestampLastActiveMs);
}

bool ViewModel::blink(uint32_t periodMS, uint64_t targetTimestamp) const
{
  // (seconds / periodMS) % 2 to determine whether the remainder is an odd or even number, if it's even then it returns true.
  bool isBlinkCurrentlyOn = ((((m_uiState.timeSinceBootMs - targetTimestamp) / (periodMS)) % 2u )== 0u);
  return isBlinkCurrentlyOn;
}

uint8_t ViewModel::quadStateBlink(uint32_t periodMS, uint64_t targetTimestamp) const
{
  return (((m_uiState.timeSinceBootMs - targetTimestamp) / (periodMS)) % 4u );
}

uint8_t ViewModel::nStateBlink(uint32_t periodMs,
                               uint64_t targetTimestamp,
                               uint32_t numStates) const
{
  return (((m_uiState.timeSinceBootMs - targetTimestamp) / (periodMs)) % numStates );
}

uint8_t ViewModel::nStateBlinkWithPause(uint32_t periodMs,
                                        uint64_t targetTimestamp,
                                        uint32_t numStates,
                                        uint32_t pauseMs) const
{
   uint8_t state = (((m_uiState.timeSinceBootMs - targetTimestamp) / (periodMs)) % numStates );
   uint64_t timeIntoPeriod = (m_uiState.timeSinceBootMs - targetTimestamp) % static_cast<uint64_t>(periodMs);
   if ( static_cast<uint64_t>(periodMs) - timeIntoPeriod < static_cast<uint64_t>(pauseMs) )
   {
     state = numStates; // indicate pause
   }
   return state;
}

