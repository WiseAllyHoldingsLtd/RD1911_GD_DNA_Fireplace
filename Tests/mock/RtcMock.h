#pragma once

#include <cstring>
#include "RtcInterface.h"
#include "CppUTestExt\MockSupport.h"

#define NAME(method) "RtcMock::" method

class DateTimeComparator : public MockNamedValueComparator
{
public:
  DateTimeComparator()
  {

  }

  virtual bool isEqual(const void* object1, const void* object2)
  {
    /* Uses copy-constructor to derefer the pointer to an actual object of the type payload. */
    const DateTime *dateTime1 = (DateTime *)object1;
    const DateTime *dateTime2 = (DateTime *)object2;
    return ((dateTime1->year == dateTime2->year) &&
            (dateTime1->month == dateTime2->month) &&
            (dateTime1->days == dateTime2->days) &&
            (dateTime1->hours == dateTime2->hours) &&
            (dateTime1->minutes == dateTime2->minutes) &&
            (dateTime1->seconds == dateTime2->seconds) &&
            (dateTime1->weekDay == dateTime2->weekDay));
  }

  SimpleString valueToString(const void* object)
  {
    DateTime* dateTime = ((DateTime*)object);

    SimpleString s = "";
    s += StringFrom(dateTime->weekDay);
    s += " ";
    s += StringFrom(dateTime->hours);
    s += ":";
    s += StringFrom(dateTime->minutes);

    return s;
  }
};
static DateTimeComparator dateTimeComparator;

class DateTimeCopier : public MockNamedValueCopier
{
public:
  virtual void copy(void * dst, const void * src) {
    memcpy(dst, src, sizeof(DateTime));
  }
};

static DateTimeCopier dateTimeCopier;


class RtcMock : public RtcInterface
{
public:
  virtual bool setup()
  {
    return mock().actualCall(NAME("setup")).returnBoolValue();
  }

  virtual void setCurrentTime(const DateTime &dateTime)
  {
    mock().actualCall(NAME("setCurrentTime")).withParameterOfType("DateTime", "dateTime", (void*)&dateTime);
  }
  virtual TimeStatus::Enum getCurrentTime(DateTime &dateTime)
  {
    return static_cast<TimeStatus::Enum> (mock().actualCall(NAME("getCurrentTime"))
      .withOutputParameterOfType("DateTime", "dateTime", (void*)&dateTime)
      .returnIntValue());
  }
};
