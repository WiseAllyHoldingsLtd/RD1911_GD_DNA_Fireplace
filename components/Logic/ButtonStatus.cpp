#include "ButtonStatus.h"

bool ButtonStatus::isButtonActivity() const
{
  bool buttonWasActive = false;
  for (uint8_t i = 0U; i < static_cast<uint8_t>(NUMBER_OF_BUTTONS); ++i)
  {
    if ((wasButtonPressedAndReleased[i] == true) || (isButtonHeld[i] == true))
    {
      buttonWasActive = true;
      break;
    }
  }
  return buttonWasActive;
}

bool ButtonStatus::isAnyButtonPressedAndReleased() const
{
  bool buttonWasActive = false;
  for (uint8_t i = 0U; i < static_cast<uint8_t>(NUMBER_OF_BUTTONS); ++i)
  {
    if(wasButtonPressedAndReleased[i] == true)
    {
      buttonWasActive = true;
      break;
    }
  }
  return buttonWasActive;
}
