#pragma once

#include <cstring>
#include "CppUTestExt\MockSupport.h"
#include "Override.h"
#include "DateTime.h"

class OverrideComparator : public MockNamedValueComparator
{
public:
  OverrideComparator()
  {

  }

  virtual bool isEqual(const void* object1, const void* object2)
  {
    /* Uses copy-constructor to derefer the pointer to an actual object of the type payload. */
    const Override *override1 = (Override *)object1;
    const Override *override2 = (Override *)object2;

    DateTime dateTime1;
    override1->getEndDateTime(dateTime1);
    DateTime dateTime2;
    override2->getEndDateTime(dateTime2);

    return (override1->getType() == override2->getType()
            && (override1->getMode() == override2->getMode())
            && (dateTime1.year == dateTime2.year)
            && (dateTime1.month == dateTime2.month)
            && (dateTime1.days == dateTime2.days)
            && (dateTime1.hours == dateTime2.hours)
            && (dateTime1.minutes == dateTime2.minutes)
            && (dateTime1.seconds == dateTime2.seconds)
            && (dateTime1.weekDay == dateTime2.weekDay));
  }

  SimpleString valueToString(const void* object)
  {
    const Override* override = ((Override*)object);
    DateTime dateTime;
    override->getEndDateTime(dateTime);

    SimpleString s = "";
    s += "type=";
    s += StringFrom(static_cast<uint8_t>(override->getType()));
    s += ", heatingMode=";
    s += StringFrom(static_cast<int>(override->getMode()));
    s += ", dateTime=";
    s += StringFrom(dateTime.year);
    s += "-";
    s += StringFrom(dateTime.month);
    s += "-";
    s += StringFrom(dateTime.days);
    s += " ";
    s += StringFrom(dateTime.hours);
    s += ":";
    s += StringFrom(dateTime.minutes);
    s += ":";
    s += StringFrom(dateTime.seconds);
    s += ", weekDay=";
    s += StringFrom(static_cast<int>(dateTime.weekDay));
    s += "  ";
    return s;
  }
};

static OverrideComparator overrideComparator;


class OverrideCopier : public MockNamedValueCopier
{
public:
  virtual void copy(void * dst, const void * src) {
    /* Uses copy-constructor to derefer the pointer to an actual object of the type payload. */
    Override *destOverride = (Override *)dst;
    const Override *srcOverride = (Override *)src;
    *destOverride = *srcOverride;
  }
};

static OverrideCopier overrideCopier;
