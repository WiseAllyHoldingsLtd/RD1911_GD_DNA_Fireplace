#pragma once

#include <cstring>

#include "CppUTestExt\MockSupport.h"
#include "ViewModelInterface.h"
#include "Constants.h"


class ViewModelMock : public ViewModelInterface
{
public:
  virtual const char * getTextString() const
  {
    return mock().actualCall("getTextString").returnStringValue();
  }
  virtual bool isRGBRedActive() const
  {
    return mock().actualCall("isRGBRedActive").returnBoolValue();
  }
  virtual bool isRGBGreenActive() const
  {
    return mock().actualCall("isRGBGreenActive").returnBoolValue();
  }
  virtual bool isRGBBlueActive() const
  {
    return mock().actualCall("isRGBBlueActive").returnBoolValue();
  }
  virtual bool isYellowLEDActive() const
  {
    return mock().actualCall("isYellowLEDActive").returnBoolValue();
  }

};


class ViewModelDummy : public ViewModelInterface
{
public:
  ViewModelDummy() :
    m_isRGBRedActive(false),
    m_isRGBGreenActive(false),
    m_isRGBBlueActive(false),
    m_isYellowLEDActive(false)
  {
    memset(m_textString, '\0', Constants::TEXT_MAX_LENGTH);
  }
  virtual const char * getTextString() const
  {
    return m_textString;
  }

  virtual bool isRGBRedActive() const
  {
    return m_isRGBRedActive;
  }
  virtual bool isRGBGreenActive() const
  {
    return m_isRGBGreenActive;
  }
  virtual bool isRGBBlueActive() const
  {
    return m_isRGBBlueActive;
  }
  virtual bool isYellowLEDActive() const
  {
    return m_isYellowLEDActive;
  }

  char m_textString[Constants::TEXT_MAX_LENGTH];
  bool m_isRGBRedActive;
  bool m_isRGBGreenActive;
  bool m_isRGBBlueActive;
  bool m_isYellowLEDActive;
};
