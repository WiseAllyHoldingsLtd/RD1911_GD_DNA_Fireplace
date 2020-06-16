#include "EspCpp.hpp"
#include "WeekProgramNodes.h"


namespace
{
  const char LOG_TAG[] = "WeekProgram";
}


WeekProgramNodes::WeekProgramNodes() : m_hasNodesBeenUpdated(false), m_numOfNodes(0u)
{
  //no need to initialize node array, it is "empty" and won't be read due to m_numOfNodes is 0.
  factoryReset();
}

int8_t WeekProgramNodes::weekProgramNodeCompare(const WeekProgramNode &first, const WeekProgramNode &second)
{
  int8_t ret = 0;

  int8_t intervalDiff = static_cast<int8_t>(first.interval) - static_cast<int8_t>(second.interval);
  int8_t hoursDiff = static_cast<int8_t>(first.hours) - static_cast<int8_t>(second.hours);
  int8_t minutesDiff = static_cast<int8_t>(first.minutes) - static_cast<int8_t>(second.minutes);

  if (intervalDiff == 0)
  {
    if (hoursDiff == 0)
    {
      ret = minutesDiff;
    }
    else
    {
      ret = hoursDiff;
    }
  }
  else
  {
    ret = intervalDiff;
  }

  if (ret < 0)
  {
    ret = -1;
  }
  else if (ret > 0)
  {
    ret = 1;
  }
  else
  {
  }

  return ret;
}

bool WeekProgramNodes::weekProgramNodeEqual(const WeekProgramNode &first, const WeekProgramNode &second)
{
  bool ret = false;

  if ((first.interval == second.interval) &&
    (first.status == second.status) &&
    (first.hours == second.hours) &&
    (first.minutes == second.minutes))
  {
    ret = true;
  }

  return ret;
}

void WeekProgramNodes::setNumOfNodes(uint8_t numOfNodes)
{
  if (numOfNodes >= NODES_MAX)
  {
    numOfNodes = NODES_MAX;
  }
  m_numOfNodes = numOfNodes;
}

uint8_t WeekProgramNodes::getNumOfNodes(void)
{
  return m_numOfNodes;
}

void WeekProgramNodes::getWeekProgramNode(uint8_t nodeIndex, WeekProgramNode &node)
{
  if (nodeIndex >= NODES_MAX)
  {
    nodeIndex = NODES_MAX - 1u;
  }
  node = m_nodes[nodeIndex];
}

void WeekProgramNodes::addNewNode(WeekProgramNode node)
{
  bool isSpaceLeft = (m_numOfNodes < NODES_MAX);

  if ((isNodeValid(node)) && isSpaceLeft)
  {
    // Adds node to list
    m_nodes[m_numOfNodes] = node;
    m_hasNodesBeenUpdated = true;

    //Sorts list
    setNumOfNodes(m_numOfNodes + 1u);
    sortNodes(m_nodes);
  }
  else
  {
    if (!isSpaceLeft)
    {
      ESP_LOGW(LOG_TAG, "Adding week program node failed, maximum number of nodes reached.");
    }
    else
    {
      ESP_LOGW(LOG_TAG, "Adding week program node failed, invalid node data. Interval=%d, hours=%d, minutes=%d, status=%d",
          node.interval, node.hours, node.minutes, node.status);
    }
  }
}

void WeekProgramNodes::sortNodes(WeekProgramNode nodeList[NODES_MAX])
{
  /* Just do a simple bubble sort. Optimize if needed, but n is relatively small
  here so it should be fine. */
  uint8_t n = m_numOfNodes;

  do
  {
    uint8_t newN = 0u;
    for (uint8_t i = 1u; i < n; i++)
    {
      int8_t compareFactor = 0;
      compareFactor = weekProgramNodeCompare(nodeList[i], nodeList[i - 1u]);
      if (compareFactor < 0)
      {
        WeekProgramNode temp = nodeList[i];
        nodeList[i] = nodeList[i - 1u];
        nodeList[i - 1u] = temp;

        newN = i;
      }
    }
    /* All elements after newN (inclusive) are now in their proper place */
    n = newN;
  } while (n > 0u);
}

bool WeekProgramNodes::isNodeValid(WeekProgramNode &node)
{
  bool isValid = true;
  uint8_t numOfNodesFromEeprom = 0u;
  numOfNodesFromEeprom = getNumOfNodes();

  for (uint8_t i = 0u; (i < numOfNodesFromEeprom) && isValid; i++)
  {
    /* To prevent us from returning false when this check is done against itself
    when editing */
    WeekProgramNode currentEepromNode;
    getWeekProgramNode(i, currentEepromNode);
    if ((node.hours == currentEepromNode.hours) &&
      (node.minutes == currentEepromNode.minutes))
    {
      /*
      Build bit maps of the two nodes we're comparing and see if they overlap.
      ie. bitmap for INTERVAL_MON = 0b00000001, INTERVAL_MONSUN = 0b01111111
      */
      uint8_t firstMap = node.buildBitMap();
      uint8_t secondMap = currentEepromNode.buildBitMap();

      if ((firstMap & secondMap) != 0u)
      {
        isValid = false;
        break;
      }
    }
  }

  return isValid;
}

void WeekProgramNodes::factoryReset(void)
{
  setNumOfNodes(0u);
  m_hasNodesBeenUpdated = true;

  WeekProgramData weekProgram;
  weekProgram.setData(Constants::DEFAULT_WEEK_PROGRAM_DATA);

  for (uint8_t i = 0u; i < NUM_OF_WEEKDAYS; i++)
  {
    WeekDay::Enum weekday = static_cast<WeekDay::Enum>(i);
    insertNodesForWeekDay(weekday, weekProgram);
  }
}

WeekProgramNode WeekProgramNodes::getBestFittingWeekProgramNode(const WeekProgramNode &first, const WeekProgramNode &second, const DateTime &time)
{
  WeekProgramNode weekProgramNode;

  uint16_t firstMinutesIntoWeek = first.getBestFittingMinutesIntoWeek(time);
  uint16_t secondMinutesIntoWeek = second.getBestFittingMinutesIntoWeek(time);
  uint16_t timeMinutesIntoWeek = getMinutesIntoWeekForDatetime(time);

  /* Is _only_ WeekProgramNode 'first' before 'time'? */
  if ((firstMinutesIntoWeek <= timeMinutesIntoWeek) &&
    (secondMinutesIntoWeek > timeMinutesIntoWeek))
  {
    weekProgramNode = first;
  }
  /* Or _only_ WeekProgramNode 'second'? */
  else if ((firstMinutesIntoWeek > timeMinutesIntoWeek) &&
    (secondMinutesIntoWeek <= timeMinutesIntoWeek))
  {
    weekProgramNode = second;
  }
  /* If they are equal we pick the one with the lowest interval value - so that
  for example INTERVAL_MON gets prioritized over INTERVAL_MONSUN */
  else if (firstMinutesIntoWeek == secondMinutesIntoWeek)
  {
    if (first.interval < second.interval)
    {
      weekProgramNode = first;
    }
    else
    {
      weekProgramNode = second;
    }
  }
  /* If either both are before 'time' or after 'time', we pick the latest one */
  else
  {
    if (firstMinutesIntoWeek >= secondMinutesIntoWeek)
    {
      weekProgramNode = first;
    }
    else
    {
      weekProgramNode = second;
    }
  }

  return weekProgramNode;
}


uint16_t WeekProgramNodes::getNumOfMinTillNextComfort(const DateTime &currentTime, WeekProgramNode &nextComfNode)
{
  uint16_t minTillNextComfort = static_cast<uint16_t>(UINT16_MAX);

  uint16_t weekDayValue = static_cast<uint16_t>(currentTime.weekDay - 1);
  if (currentTime.weekDay == static_cast<int16_t>(WeekDay::SUNDAY))
  {
    weekDayValue = 6u;
  }


  bool foundOneNode = false;
  WeekProgramNode firstNodeWithComfort;

  uint16_t timeMinutesIntoWeek = weekDayValue * 1440u +
    static_cast<uint16_t>(currentTime.hours) * 60u +
    static_cast<uint16_t>(currentTime.minutes);

  for (uint8_t i = 0u; i < m_numOfNodes; i++)
  {
    WeekProgramNode nodeToRetrieve;
    getWeekProgramNode(i, nodeToRetrieve);

    if (nodeToRetrieve.status == static_cast<int16_t>(HeatingMode::comfort))
    {
      if (!foundOneNode)
      {
        firstNodeWithComfort = nodeToRetrieve;
        foundOneNode = true;
      }
      uint16_t nodeMinutesIntoWeek = nodeToRetrieve.getNumOfMinIntoWeekForThisNode(currentTime);

      // is Node time after currentTime?
      if (nodeMinutesIntoWeek > timeMinutesIntoWeek)
      {
        uint16_t numOfMinFromCurrentTimeToThisNodeTime = nodeMinutesIntoWeek - timeMinutesIntoWeek;
        if ((numOfMinFromCurrentTimeToThisNodeTime < minTillNextComfort)) // is there a node that's after and closer to current time?
        {
          minTillNextComfort = numOfMinFromCurrentTimeToThisNodeTime;
          nextComfNode = nodeToRetrieve;
        }
      }
    }
  }

  if (foundOneNode) // we check if the first node "next week" is closer to current date-time
  {
    uint16_t nodeMinutesIntoWeek = firstNodeWithComfort.getNumOfMinIntoWeekForThisNode(currentTime);
    // 10080 is the num of minutes in a whole week
    uint16_t timeRemainingInWeek = 10080u -
      (weekDayValue * 1440u +
      static_cast<uint16_t>(currentTime.hours) * 60u +
      static_cast<uint16_t>(currentTime.minutes));

    uint16_t numOfMinFromCurrentTimeToThisNodeTimeWithLoopingAroundToNextWeek = nodeMinutesIntoWeek + timeRemainingInWeek;

    if ((numOfMinFromCurrentTimeToThisNodeTimeWithLoopingAroundToNextWeek < minTillNextComfort))
    {
      minTillNextComfort = numOfMinFromCurrentTimeToThisNodeTimeWithLoopingAroundToNextWeek;
      nextComfNode = firstNodeWithComfort;
    }


  }

  return minTillNextComfort;
}

/* returns true if a node has been deleted, edited or added. Will be false if called a second time without a new operation (delete, add, edit)*/
bool WeekProgramNodes::hasNodesBeenUpdated(void)
{
  bool hasBeenUpdated = false;
  if (m_hasNodesBeenUpdated)
  {
    hasBeenUpdated = true;
  }
  m_hasNodesBeenUpdated = false;
  return hasBeenUpdated;
}


Interval::Enum WeekProgramNodes::convertWeekDayToSingleIntervalDay(const WeekDay::Enum &weekday)
{
  Interval::Enum returnedInterval;

  switch (weekday)
  {
  case WeekDay::TUESDAY:
    returnedInterval = Interval::INTERVAL_TUE;
    break;
  case WeekDay::WEDNESDAY:
    returnedInterval = Interval::INTERVAL_WED;
    break;
  case WeekDay::THURSDAY:
    returnedInterval = Interval::INTERVAL_THU;
    break;
  case WeekDay::FRIDAY:
    returnedInterval = Interval::INTERVAL_FRI;
    break;
  case WeekDay::SATURDAY:
    returnedInterval = Interval::INTERVAL_SAT;
    break;
  case WeekDay::SUNDAY:
    returnedInterval = Interval::INTERVAL_SUN;
    break;
  default:
    returnedInterval = Interval::INTERVAL_MON;
    break;
  }

  return returnedInterval;
}


void WeekProgramNodes::insertNodesForWeekDay(const WeekDay::Enum &weekday, const WeekProgramData &weekProgramData)
{
  uint8_t numberOfSlots = weekProgramData.getNumberOfSlots();

  WeekProgramNode node;
  node.interval = convertWeekDayToSingleIntervalDay(weekday);

  for (uint8_t i = 0U; i < numberOfSlots; i++)
  {
    bool isSlotValid = weekProgramData.getWeekProgramNodeFromSlot(i + 1, node); // slots are numbered 1 to 4

    if (isSlotValid)
    {
      addNewNode(node);
    }
  }
}

void WeekProgramNodes::insertNodesForWeekDay(const WeekDay::Enum &weekday, const WeekProgramExtData &weekProgramExtData)
{
  uint8_t numberOfSlots = weekProgramExtData.getNumberOfSlots();

  WeekProgramNode node;
  node.interval = convertWeekDayToSingleIntervalDay(weekday);

  for (uint8_t i = 0U; i < numberOfSlots; i++)
  {
    bool isSlotValid = weekProgramExtData.getWeekProgramNodeFromSlot(i + 1, node); // slots are numbered 1 to numberOfSlots

    if (isSlotValid)
    {
      addNewNode(node);
    }
  }
}

void WeekProgramNodes::updateNodes(SettingsInterface &settings)
{
  if (settings.isWeekProgramUpdated())
  {
    m_numOfNodes = 0u;
    m_hasNodesBeenUpdated = true;

    for (uint8_t i = 0u; i < NUM_OF_WEEKDAYS; i++)
    {
      WeekDay::Enum weekday = static_cast<WeekDay::Enum>(i);
      WeekProgramExtData weekProgramExtData;

      bool extendedWeekProgramValid = false;

      settings.getWeekProgramExtDataForDay(weekday, weekProgramExtData);
      extendedWeekProgramValid = weekProgramExtData.isValid();

      if ( extendedWeekProgramValid )
      {
        ESP_LOGI(LOG_TAG, "Updating week program using extended data for day %u",
            static_cast<uint32_t>(i));
        insertNodesForWeekDay(weekday, weekProgramExtData);
      }
      else
      {
        WeekProgramData weekProgramData;
        settings.getWeekProgramDataForDay(weekday, weekProgramData);
        if ( weekProgramData.isValid() )
        {
          ESP_LOGI(LOG_TAG, "Updating week program using \"old\" 4-node data for day %u",
              static_cast<uint32_t>(i));
          insertNodesForWeekDay(weekday, weekProgramData);
        }
      }

    }
  }
}

uint16_t WeekProgramNodes::getMinutesIntoWeekForDatetime(const DateTime &datetime) const
{
  uint16_t weekDayValue = static_cast<uint16_t>(datetime.weekDay - 1);
  if (datetime.weekDay == static_cast<int>(WeekDay::SUNDAY)) {
    weekDayValue = 6u;
  }
  return weekDayValue * 1440u +
    static_cast<uint16_t>(datetime.hours)* 60u +
    static_cast<uint16_t>(datetime.minutes);
}
