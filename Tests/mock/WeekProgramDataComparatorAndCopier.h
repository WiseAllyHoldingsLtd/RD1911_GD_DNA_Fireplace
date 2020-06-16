#pragma once

#include <cstring>
#include "CppUTestExt\MockSupport.h"
#include "WeekProgramData.h"

class WeekProgramDataComparator : public MockNamedValueComparator
{
public:
  WeekProgramDataComparator()
  {

  }

  virtual bool isEqual(const void* object1, const void* object2)
  {
    /* Uses copy-constructor to derefer the pointer to an actual object of the type payload. */
    const WeekProgramData *weekProg1 = (WeekProgramData *)object1;
    const WeekProgramData *weekProg2 = (WeekProgramData *)object2;

    uint8_t dataProg1[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY];
    uint8_t dataProg2[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY];

    weekProg1->getData(dataProg1);
    weekProg2->getData(dataProg2);

    return (memcmp(dataProg1, dataProg2, sizeof(dataProg2)) == 0);
  }

  SimpleString valueToString(const void* object)
  {
    WeekProgramData* weekProg = ((WeekProgramData*)object);

    uint8_t dataProg[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY];
    weekProg->getData(dataProg);

    static_assert(Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY == 20U, "Unexpected size of weekProgramData");

    SimpleString s = "";

    // Four slots, five bytes per slots => 20 bytes
    for (uint8_t i = 0; i < 4U; ++i)
    {
      uint8_t slotStart = i * 5;
      s += StringFrom(dataProg[slotStart + 1U]); // start hour
      s += ":";
      s += StringFrom(dataProg[slotStart + 2U]); // start minute
      s+= "-";
      s += StringFrom(dataProg[slotStart + 3U]); // end hour
      s += ":";
      s += StringFrom(dataProg[slotStart + 4U]); // end minute
      s += "=>";
      s += StringFrom(dataProg[slotStart]); // mode
      s += "   ";
    }

    return s;
  }
};

static WeekProgramDataComparator weekProgramDataComparator;


class WeekProgramDataCopier : public MockNamedValueCopier
{
public:
  virtual void copy(void * dst, const void * src) {
    /* Uses copy-constructor to derefer the pointer to an actual object of the type payload. */
    WeekProgramData *destData = (WeekProgramData *)dst;
    const WeekProgramData *srcData = (WeekProgramData *)src;

    uint8_t bytesToCopy[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY];
    srcData->getData(bytesToCopy);
    destData->setData(bytesToCopy);
  }
};

static WeekProgramDataCopier weekProgramDataCopier;
