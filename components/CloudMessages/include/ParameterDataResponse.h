#pragma once
#include "FrameBuilderUser.h"


class ParameterDataResponse : public FrameBuilderUser
{
public:
  ParameterDataResponse(void);

  bool addParameterData(uint16_t id, const uint8_t * data, uint8_t size);
  bool finalize(void);

private:
  bool m_isFinalized;
};
