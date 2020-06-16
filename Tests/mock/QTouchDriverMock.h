#pragma once
#include <cstring>
#include "TouchDriverInterface.h"


class QTouchDriverMock : public TouchDriverInterface
{
public:
  void setTouchReturnValue(const bool (&isButtonTouched)[NUMBER_OF_BUTTONS])
  {
    memcpy(m_isButtonTouched, isButtonTouched, NUMBER_OF_BUTTONS);
  }

  virtual void getTouchStatus(bool (&isButtonTouched)[NUMBER_OF_BUTTONS])
  {
    //This is too slow when calling thousands of times, so use an alternative method in stead. mock().actualCall("getTouchStatus").withOutputParameter("isButtonTouched", isButtonTouched);
    memcpy(isButtonTouched, m_isButtonTouched, NUMBER_OF_BUTTONS);
  }

private:
    bool m_isButtonTouched[NUMBER_OF_BUTTONS];
};
