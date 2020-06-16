#include <cstring>
#include "base64.hpp"
#include "Base64Converter.h"
#include "EspCpp.hpp"

bool Base64::decode(const char* inputStr, uint8_t* outputData, uint32_t * outputDataSize)
{
  bool wasDecoded = false;

  if (*outputDataSize > 0U)
  {
    int retVal = mbedtls_base64_decode(outputData, *outputDataSize, outputDataSize, reinterpret_cast<const uint8_t*>(inputStr), strlen(inputStr));
    wasDecoded = (retVal == 0);
  }

  return wasDecoded;
}


uint32_t Base64::getDecodedSize(const char* inputStr) const
{
  /* Note that returned requiredSize includes space for a zero-term */
  uint32_t requiredSize;
  mbedtls_base64_decode(nullptr, 0, &requiredSize, reinterpret_cast<const uint8_t *>(inputStr), strlen(inputStr)); // returns requiredSize (size_t - 1) on error
  return requiredSize;
}


bool Base64::encode(const uint8_t* inputData, uint32_t inputDataSize, char* outputStr, uint32_t outputStrSize)
{
  /* Note that outputStrSize should include space for a zero-term. */
  bool wasEncoded = false;
  outputStr[0] = '\0';

  if (outputStrSize > 0U)
  {
    uint32_t bytesWritten = 0;

    int retVal = mbedtls_base64_encode(
        reinterpret_cast<uint8_t*>(outputStr),
        outputStrSize,
        &bytesWritten,
        inputData,
        inputDataSize);

    if (retVal == 0)
    {
      outputStr[bytesWritten] = '\0';
      wasEncoded = true;
    }
  }

  return wasEncoded;
}


uint32_t Base64::getEncodedSize(const uint8_t* inputData, uint32_t inputDataSize) const
{
  /* Note that returned requiredSize includes space for a zero-term */
  uint32_t requiredSize = 0U;
  mbedtls_base64_encode(nullptr, 0, &requiredSize, inputData, inputDataSize); // returns requiredSize (size_t - 1) on error
  return requiredSize;
}
