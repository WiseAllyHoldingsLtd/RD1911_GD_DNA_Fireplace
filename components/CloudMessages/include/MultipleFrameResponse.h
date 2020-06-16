#pragma once
#include "ParameterDataResponse.h"
#include "FrameBuilderUser.h"


class MultipleFrameResponse : public FrameBuilderUser
{
public:
  MultipleFrameResponse(void);
  bool addParameterDataResponse(const ParameterDataResponse & parameterData);
  uint16_t getNumberOfFrames(void) const;

private:
  uint16_t m_numFrames;
};
