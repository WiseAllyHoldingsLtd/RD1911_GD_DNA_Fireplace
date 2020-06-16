#include "StringConverter.h"
#include <cstring>


namespace {
  uint8_t displayMap(const unsigned char symbol) {
    uint8_t value = 0u;
    switch(symbol) {
      case '0': value = 0x3Fu; break;
      case '1': value = 0x06u; break;
      case '2': value = 0x5Bu; break;
      case '3': value = 0x4Fu; break;
      case '4': value = 0x66u; break;
      case '5': value = 0x6Du; break;
      case '6': value = 0x7Du; break;
      case '7': value = 0x07u; break;
      case '8': value = 0x7Fu; break;
      case '9': value = 0x6Fu; break;
      case 'a':
      case 'A': value = 0x77u; break;
      case 'B':
      case 'b': value = 0x7Cu; break;
      case 'C': value = 0x39u; break;
      case 'c': value = 0x58u; break;
      case 'D':
      case 'd': value = 0x5Eu; break;
      case 'e':
      case 'E': value = 0x79u; break;
      case 'f':
      case 'F': value = 0x71u; break;
      case 'H': value = 0x76u; break;
      case 'h': value = 0x74u; break;
      case 'i': value = 0x10u; break;
      case 'I': value = 0x30u; break;
      case 'l':
      case 'L': value = 0x38u; break;
      case 'N':
      case 'n': value = 0x54u; break;
      case 'O': value = 0x3Fu; break;
      case 'o': value = 0x5Cu; break;
      case 'P':
      case 'p': value = 0x73u; break;
      case 'R':
      case 'r': value = 0x50u; break;
      case 's':
      case 'S': value = 0x6Du; break;
      case 't': 
      case 'T': value = 0x78u; break;
      case 'u': value = 0x1Cu; break;
      case 'U': value = 0x3Eu; break;
      case '-': value = 0x40u; break;
      case '_': value = 0x08u; break;
      case '^': value = 0x01u; break; // used as horizontal line on top row

      case ' ':
      default:
        value = 0u;
        break;
    }
    return value;
  }
}


uint32_t StringConverter::convert(const char input[], uint8_t output[], uint32_t maxOutputLength)
{
  uint32_t index = 0u;
  uint32_t length = strlen(input);
  for (uint32_t i(0u); i < length; ++i) {
    if (index >= maxOutputLength) {
      break;
    }
    output[index] = displayMap(input[i]);
    ++index;
  }
  return index;
}
