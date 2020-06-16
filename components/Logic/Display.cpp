#include <stdint.h>
#include "Display.h"
#include "StringConverter.h"
#include "Constants.h"


Display::Display(DisplayDriverInterface& displayDriver) : m_displayDriver(displayDriver)
{
}


/**
 * Resets and initializes display
 */
void Display::setup()
{
  m_displayDriver.resetAndInit();
}


//#pragma optimize=speed
void Display::drawScreen(const ViewModelInterface& viewModel)
{
  uint8_t displayData[3] = {0, 0, 0};

  // Set sector 1-2 (7 seg)
  StringConverter::convert(viewModel.getTextString(), displayData, Constants::TEXT_MAX_LENGTH-1u);

  // Set sector 3 (indicator LEDs)
  if (viewModel.isYellowLEDActive()) {
    displayData[2] |= 0b1u;
  }

  if (viewModel.isRGBRedActive()) {
    displayData[2] |= 0b10u;
  }

  if (viewModel.isRGBGreenActive()) {
    displayData[2] |= 0b100u;
  }

  if (viewModel.isRGBBlueActive()) {
    displayData[2] |= 0b1000u;
  }

  m_displayDriver.setData(displayData);
}
