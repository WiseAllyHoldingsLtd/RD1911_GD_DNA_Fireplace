#pragma once

#include "FrameParser.h"


struct OperationType
{
  enum Enum : uint8_t
  {
    forgetMe = 0u,
    invalid = 1u
  };
};

struct OperationStatus
{
  enum Enum : uint8_t
  {
    success = 0u,
    forgetMeError = 1u,
    invalid = 2u
  };
};


class OperationInfo
{
public:
  OperationInfo(const FrameParser &frame);
  bool isValid(void) const;
  OperationType::Enum getType() const;
  OperationStatus::Enum getStatus() const;

private:
  const FrameParser &m_frame;
};
