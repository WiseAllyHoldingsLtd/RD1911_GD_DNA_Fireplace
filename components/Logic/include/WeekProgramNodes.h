#pragma once

#include "WeekProgramData.h"
#include "WeekProgramNode.h"
#include "WeekProgramNodesInterface.h"
#include <stdint.h>
#include "Settings.h"

class WeekProgramNodes : public WeekProgramNodesInterface
{
public:
  WeekProgramNodes();
  virtual int8_t weekProgramNodeCompare(const WeekProgramNode &first, const WeekProgramNode &second);
  virtual bool weekProgramNodeEqual(const WeekProgramNode &first, const WeekProgramNode &second);
  virtual WeekProgramNode getBestFittingWeekProgramNode(const WeekProgramNode &first, const WeekProgramNode &second, const DateTime &time);

  virtual uint8_t getNumOfNodes(void);

  virtual void getWeekProgramNode(uint8_t nodeIndex, WeekProgramNode &node);

  virtual void factoryReset();

  virtual uint16_t getNumOfMinTillNextComfort(const DateTime &currentTime, WeekProgramNode &nextComfNode);

  virtual bool hasNodesBeenUpdated(void);
  virtual void updateNodes(SettingsInterface &settings);

private:
  WeekProgramNode m_nodes[NODES_MAX];
  bool m_hasNodesBeenUpdated;
  uint8_t m_numOfNodes;

  virtual void addNewNode(WeekProgramNode node);
  virtual bool isNodeValid(WeekProgramNode &node);

  virtual void setNumOfNodes(uint8_t numOfNodes);

  virtual void sortNodes(WeekProgramNode nodeList[NODES_MAX]);

  Interval::Enum convertWeekDayToSingleIntervalDay(const WeekDay::Enum &weekday);
  void insertNodesForWeekDay(const WeekDay::Enum &weekday,
                             const WeekProgramData &weekProgramData);
  void insertNodesForWeekDay(const WeekDay::Enum &weekday,
                             const WeekProgramExtData &weekProgramExtData);
  uint16_t getMinutesIntoWeekForDatetime(const DateTime &datetime) const;
};
