#pragma once

#include "WeekProgramNode.h"
#include "SettingsInterface.h"

class WeekProgramNodesInterface
{
public:
  virtual ~WeekProgramNodesInterface(){};

  virtual int8_t weekProgramNodeCompare(const WeekProgramNode &first, const WeekProgramNode &second) = 0;
  virtual bool weekProgramNodeEqual(const WeekProgramNode &first, const WeekProgramNode &second) = 0;
  virtual WeekProgramNode getBestFittingWeekProgramNode(const WeekProgramNode &first, const WeekProgramNode &second, const DateTime &time) = 0;

  virtual uint8_t getNumOfNodes(void) = 0;

  virtual void getWeekProgramNode(uint8_t nodeIndex, WeekProgramNode &node) = 0;

  virtual void factoryReset() = 0;

  virtual uint16_t getNumOfMinTillNextComfort(const DateTime &currentTime, WeekProgramNode &nextComfNode) = 0;

  virtual bool hasNodesBeenUpdated(void) = 0;
  virtual void updateNodes(SettingsInterface &settings) = 0;
};
