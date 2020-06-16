#pragma once


class ViewModelInterface
{
public:
  virtual const char * getTextString() const = 0;
  virtual bool isYellowLEDActive() const = 0;
  virtual bool isRGBRedActive() const = 0;
  virtual bool isRGBGreenActive() const = 0;
  virtual bool isRGBBlueActive() const = 0;

  virtual ~ViewModelInterface() {};
};
