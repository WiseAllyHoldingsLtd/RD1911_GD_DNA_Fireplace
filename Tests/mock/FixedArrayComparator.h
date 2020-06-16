#include "CppUTestExt\MockSupport.h"

/// @cond TEST

class FixedArrayComparator : public MockNamedValueComparator
{
private:
  uint8_t m_length;

public:
  FixedArrayComparator(uint8_t length) : m_length(length)
  {

  }

  virtual bool isEqual(const void* object1, const void* object2)
  {
    uint8_t * fixedArray1 = (uint8_t*)object1;
    uint8_t * fixedArray2 = (uint8_t*)object2;
    return memcmp(fixedArray1, fixedArray2, m_length) == 0;
  }

  virtual SimpleString valueToString(const void* object)
  {
    uint8_t * fixedArray = (uint8_t*)object;
    SimpleString s = "[";
    for (uint8_t i = 0; i < m_length; i++)
    {
      s += StringFrom(fixedArray[i]);
      if (i < m_length)
      {
        s += ", ";
      }
    }
    s += "]";
    return s;
  }
};

/// @endcond
