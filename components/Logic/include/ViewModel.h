#pragma once

#include "ViewModelInterface.h"
//#include "TemperatureValuesInterface.h"
#include "SettingsInterface.h"
#include "DeviceMetaInterface.h"
//#include <stdint.h>
//#include "pgmspace.h"
//#include "Address.h"
//#include "Rtc.h"
//#include "WeekProgramNodesInterface.h"
//#include "Eeprom.h"
#include "UIState.h"
#include "Constants.h"

#include "HeatingMode.h"


/**
 * Contains (relatively abstract) elements of the GUI, generated based on the current state of the device.
 *
 * Intended use: Create a new instance of ViewModel at every iteration of the main loop.
 * An instance of this class is handed to Display when it's time to draw the screen.
 */
class ViewModel : public ViewModelInterface
{
public:

  /**
  * Generates (abstract) elements of the GUI based on the state of the device.
  * All temperature values are fixed decimal where 1 means 0.001 degrees C.
  */
  ViewModel(UIState uiState,
            SettingsInterface &settings,
            DeviceMetaInterface &deviceMeta,
            bool isHeatCurrentlyOn, bool isScreenFullyDimmed);

  /** This constructor is used to show error message
   */
  ViewModel(const char textString[]);

  virtual const char * getTextString() const;
  virtual bool isYellowLEDActive() const;
  virtual bool isRGBRedActive() const;
  virtual bool isRGBGreenActive() const;
  virtual bool isRGBBlueActive() const;

  void createMainScreenViewModel(int32_t setPoint,
    HeatingMode::Enum heatingMode,
    bool isHeatCurrentlyOn,
    bool isScreenFullyDimmed,
    bool isOverride
    );
  void createButtonTestScreen();
  void createBluetoothSetupScreen(DeviceMetaInterface &deviceMeta);
  void createFirmwareUpgradeScreen();
  void createSWVersionScreen();
  void createButtonLockedScreen();
  void createButtonUnlockedScreen();
  void createFactoryResetScreen();
  void createFactoryResetConfirmScreen();

protected:

  UIState m_uiState;
  char m_textString[Constants::TEXT_MAX_LENGTH];
  bool m_yellowLED;
  bool m_RGBRed;
  bool m_RGBGreen;
  bool m_RGBBlue;

  /**
   *  returns whether the blinking element should be on or off
   *  targetTimestamp is the timestamp to check period against
   */
  bool blink(uint32_t periodMS, uint64_t targetTimestamp) const;

  /* shorthand for blinking from last activity timestamp */
  bool blinkFromActivity(uint32_t periodMS) const;

  /* Returns a value 0, 1, 2, 3, 0, 1, 2, 3 in wrapping counter fashion */
  uint8_t quadStateBlink(uint32_t periodMS, uint64_t targetTimestamp) const;

  uint8_t nStateBlink(uint32_t periodMs,
                      uint64_t targetTimestamp,
                      uint32_t numStates) const;

  uint8_t nStateBlinkWithPause(uint32_t periodMs,
                               uint64_t targetTimestamp,
                               uint32_t numStates,
                               uint32_t pauseMs) const;
};
