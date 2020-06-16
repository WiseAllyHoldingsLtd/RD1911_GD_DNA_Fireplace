#pragma once

#include <cstdint>

class Base64Interface
{
public:
  virtual ~Base64Interface(void) {}

  virtual bool decode(const char* inputStr, uint8_t* outputData, uint32_t * outputDataSize) = 0;
  virtual uint32_t getDecodedSize(const char* inputStr) const = 0;

  virtual bool encode(const uint8_t* inputData, uint32_t inputDataSize, char* outputStr, uint32_t outputStrSize) = 0;
  virtual uint32_t getEncodedSize(const uint8_t* inputData, uint32_t inputDataSize) const = 0;
};
