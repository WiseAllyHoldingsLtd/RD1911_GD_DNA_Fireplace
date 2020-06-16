#pragma once

#include <stdint.h>


class StringConverter
{
public:
  /*
   * Converts a string to Display sector data.
   * @param input Null-terminated C-string.
   * @param output buffer to write converted data to
   * @param maxOutputLength maximum length of output buffer
   * @return number of bytes written to output buffer.
   */
  static uint32_t convert(const char input[], uint8_t output[], uint32_t maxOutputLength);
};
