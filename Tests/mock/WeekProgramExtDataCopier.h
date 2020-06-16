#pragma once

#include <cstring>
#include "CppUTestExt\MockSupport.h"
#include "WeekProgramExtData.h"

class WeekProgramExtDataCopier : public MockNamedValueCopier
{
public:
  virtual void copy(void * dst, const void * src)
  {
    WeekProgramExtData *destData = (WeekProgramExtData *)dst;
    const WeekProgramExtData *srcData = (WeekProgramExtData *)src;

    uint8_t bytesToCopy[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY_EXT_MAX];
    uint8_t numBytesToCopy = srcData->getCurrentSize();
    srcData->getData(bytesToCopy, numBytesToCopy);
    destData->setData(bytesToCopy, numBytesToCopy);
  }
};
