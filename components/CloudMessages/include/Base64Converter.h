#pragma once

#include "Base64Interface.h"

class Base64 : public Base64Interface
{
public:
  virtual bool decode(const char* inputStr, uint8_t* outputData, uint32_t * outputDataSize);
  virtual uint32_t getDecodedSize(const char* inputStr) const;

  virtual bool encode(const uint8_t* inputData, uint32_t inputDataSize, char* outputStr, uint32_t outputStrSize);
  virtual uint32_t getEncodedSize(const uint8_t* inputData, uint32_t inputDataSize) const;
};
