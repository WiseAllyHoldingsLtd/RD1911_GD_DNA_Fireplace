#include <cstring>
#include "WeekProgramData.h"


WeekProgramData::WeekProgramData(void)
{
  memset(m_data, Constants::CLOUD_PARAMETER_INVALID_VALUE, sizeof(m_data));
}


WeekProgramData::WeekProgramData(const WeekProgramData& other)
{
  memcpy(m_data, other.m_data, sizeof(m_data));
}


bool WeekProgramData::operator==(const WeekProgramData& rhs)
{
  return (memcmp(m_data, rhs.m_data, sizeof(m_data)) == 0);
}


bool WeekProgramData::operator!=(const WeekProgramData& rhs)
{
  return (memcmp(m_data, rhs.m_data, sizeof(m_data)) != 0);
}


void WeekProgramData::getData(uint8_t (&data)[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY]) const
{
  memcpy(data, m_data, sizeof(data));
}


void WeekProgramData::setData(const uint8_t (&data)[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY])
{
  memcpy(m_data, data, sizeof(m_data));
}


uint8_t WeekProgramData::getNumberOfSlots(void) const
{
  return Constants::WEEKPROGRAM_SLOTS_PER_WEEKDAY;
}


bool WeekProgramData::getWeekProgramNodeFromSlot(uint8_t slotNo, WeekProgramNode & outNode) const
{
  CloudTimerHeatingMode::Enum mode;
  uint8_t startHour, startMinute, endHour, endMinute;
  bool isSlotValid = getSlotData(slotNo, mode, startHour, startMinute, endHour, endMinute);

  if (isSlotValid)
  {
    HeatingMode::Enum heatingMode = convertSlotModeToHeatingMode(mode);
    uint8_t hours = convertValueToHours(startHour);
    uint8_t minutes = convertValueToMinutes(startMinute);

    WeekProgramNode node(outNode.interval, heatingMode, hours, minutes);
    outNode = node;
  }

  return isSlotValid;
}


bool WeekProgramData::isValid() const
{
  CloudTimerHeatingMode::Enum dummyMode = CloudTimerHeatingMode::comfort;
  uint8_t dummyStartHour = 0u,
          dummyStartMinute = 0u,
          dummyEndHour = 0u,
          dummyEndMinute = 0u;

  // If first slot is invalid, the entire dataset is seen as invalid
  bool isSlotDataValid = getSlotData(1u,
                                     dummyMode,
                                     dummyStartHour,
                                     dummyStartMinute,
                                     dummyEndHour,
                                     dummyEndMinute);
  return isSlotDataValid;
}


bool WeekProgramData::getSlotData(uint8_t slotNo, CloudTimerHeatingMode::Enum & mode, uint8_t & startHour, uint8_t & startMinute, uint8_t & endHour, uint8_t & endMinute) const
{
  bool isSlotDataValid = false;

  if (isSlotNoWithinRange(slotNo))
  {
    uint8_t startIndex = (slotNo - 1U) * Constants::WEEKPROGRAM_SLOT_SIZE;

    mode = static_cast<CloudTimerHeatingMode::Enum>(m_data[startIndex]);
    startHour = m_data[startIndex + 1U];
    startMinute = m_data[startIndex + 2U];
    endHour = m_data[startIndex + 3U];
    endMinute = m_data[startIndex + 4U];

    isSlotDataValid = ((mode != Constants::CLOUD_PARAMETER_INVALID_VALUE)
                        && (startHour != Constants::CLOUD_PARAMETER_INVALID_VALUE)
                        && (startMinute != Constants::CLOUD_PARAMETER_INVALID_VALUE)
                        && (endHour != Constants::CLOUD_PARAMETER_INVALID_VALUE)
                        && (endMinute != Constants::CLOUD_PARAMETER_INVALID_VALUE));
  }

  return isSlotDataValid;
}


bool WeekProgramData::isSlotNoWithinRange(uint8_t slotNo) const
{
  return ((slotNo > 0U) && (slotNo <= getNumberOfSlots()));
}


HeatingMode::Enum WeekProgramData::convertSlotModeToHeatingMode(CloudTimerHeatingMode::Enum mode) const
{
  HeatingMode::Enum heatingMode;

  switch (mode) // Defined in parameter model for Compact Heaters: 0=Away, 1=Eco, 2=Comfort, 3=Off,
  {
  case CloudTimerHeatingMode::away:
    heatingMode = HeatingMode::antiFrost;
    break;
  case CloudTimerHeatingMode::eco:
    heatingMode = HeatingMode::eco;
    break;
  case CloudTimerHeatingMode::shutdown:
    heatingMode = HeatingMode::off;
    break;
  default:
    heatingMode = HeatingMode::comfort;
    break;
  }

  return heatingMode;
}


uint8_t WeekProgramData::convertValueToHours(uint8_t value) const
{
  return (value % 24U);
}


uint8_t WeekProgramData::convertValueToMinutes(uint8_t value) const
{
  return (value % 60U);
}

