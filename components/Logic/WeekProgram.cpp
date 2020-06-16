#include "WeekProgram.h"
#include "Constants.h"

WeekProgram::WeekProgram(WeekProgramNodesInterface &nodes) : m_weekProgramNodes(nodes)
{

}

HeatingMode::Enum WeekProgram::getCurrentStatus(const DateTime &time, uint16_t minToPreheat, bool &isComfortStatusFromNode)
{
  isComfortStatusFromNode = false;
  HeatingMode::Enum status = HeatingMode::comfort;

  uint8_t numOfWeekProgramNodes = 0u;
  numOfWeekProgramNodes = m_weekProgramNodes.getNumOfNodes();
  
  WeekProgramNode dummyNode;
  
  uint16_t minTillNextComfort = static_cast<uint16_t>(UINT16_MAX);
  minTillNextComfort = m_weekProgramNodes.getNumOfMinTillNextComfort(time, dummyNode);

  WeekProgramNode bestWeekProgramNode;

  if (numOfWeekProgramNodes > 0u)
  {
    m_weekProgramNodes.getWeekProgramNode(0u, bestWeekProgramNode);

    for (uint8_t i = 1u; i< numOfWeekProgramNodes; i++)
    {
      WeekProgramNode currentNodeAtIndexI;
      m_weekProgramNodes.getWeekProgramNode(i, currentNodeAtIndexI);
      bestWeekProgramNode = m_weekProgramNodes.getBestFittingWeekProgramNode(bestWeekProgramNode, currentNodeAtIndexI, time);
    }

    if (bestWeekProgramNode.status == static_cast<int16_t>(HeatingMode::comfort))
    {
      isComfortStatusFromNode = true;
    }
  }

  if (minTillNextComfort <= minToPreheat)
  {
    status = HeatingMode::comfort; // this is already comfort mode, but we'll just set explicitly here as well.
  }
  else if (numOfWeekProgramNodes > 0u)
  {
    status = bestWeekProgramNode.status;
  }
  else
  {
    // misra demands that all else if clauses should have final else block, and its demands will be met
  }

  return status;
}

bool WeekProgram::shouldUpdateASAP(void)
{
  return m_weekProgramNodes.hasNodesBeenUpdated();
}
