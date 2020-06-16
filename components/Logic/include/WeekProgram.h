#pragma once

#include "WeekProgramNodesInterface.h"
#include "HeatingMode.h"
#include "DateTime.h"
#include "WeekProgramNodes.h"

class WeekProgram
{

public:
  WeekProgram(WeekProgramNodesInterface &nodes);

  /* A 'WeekProgramNode' represents a part of a week program. */
  HeatingMode::Enum getCurrentStatus(const DateTime &time, uint16_t minToPreheat, bool &isComfortStatusFromNode);

  bool shouldUpdateASAP(void);
 
private:
 WeekProgramNodesInterface &m_weekProgramNodes;

};
