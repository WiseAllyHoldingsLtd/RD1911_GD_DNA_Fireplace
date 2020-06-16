#pragma once

#include <cstdint>


class BacklightDriverInterface
{
public:
  /*
   * Sets the backlight level
   * a value of 0-200 sets level in half-percent steps (100 => 50%)
   */
  virtual void setBacklightLevel(uint8_t level) = 0;

  virtual ~BacklightDriverInterface() {}
};

