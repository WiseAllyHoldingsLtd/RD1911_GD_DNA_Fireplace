#pragma once

#include <stdint.h>


class DisplayDriverInterface
{
public:
  virtual void resetAndInit(void) = 0;
  /*
   * Set the data to display.
   * Byte 1-2: Digits 1-2: LSB/bit1=SegA, bit2=SegB, etc.
   * Byte 3: LSB/bit1=YellowLED, bit2=RGBRed, bit3=RGBGreen, bit4=RGBBlue
   */
  virtual void setData(uint8_t data[3]) = 0;

  virtual ~DisplayDriverInterface() {};
};
