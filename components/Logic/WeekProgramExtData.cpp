#include <cstring>
#include <algorithm>
#include "WeekProgramExtData.h"

WeekProgramExtData::WeekProgramExtData():
m_data(nullptr),
m_size(0u)
{

}

WeekProgramExtData::~WeekProgramExtData()
{
  if ( m_data != nullptr )
  {
    delete[] m_data;
    m_data = nullptr;
  }
}

WeekProgramExtData::WeekProgramExtData(const WeekProgramExtData &other):
m_data(nullptr),
m_size(0u)
{
  m_data = new uint8_t[other.m_size];
  memcpy(m_data, other.m_data, other.m_size);
  m_size = other.m_size;
}

WeekProgramExtData &WeekProgramExtData::operator=(const WeekProgramExtData &rhs)
{
  setData(rhs.m_data, rhs.m_size);
  return *this;
}

bool WeekProgramExtData::operator==(const WeekProgramExtData &rhs)
{
  return ( rhs.m_size == m_size && memcmp(m_data, rhs.m_data, rhs.m_size) == 0 );
}

bool WeekProgramExtData::operator!=(const WeekProgramExtData &rhs)
{
  return !this->operator==(rhs);
}

void WeekProgramExtData::getData(uint8_t *data, uint8_t size) const
{
  std::size_t numBytesToCopy = std::min(size, m_size);

  if ( numBytesToCopy > 0u )
  {
    std::memcpy(data, m_data, numBytesToCopy);
  }
}

void WeekProgramExtData::setData(const uint8_t *data, uint8_t size)
{
  if ( m_data != nullptr )
  {
    delete[] m_data;
    m_data = nullptr;
  }

  m_data = new uint8_t[size];
  std::memcpy(m_data, data, size);
  m_size = size;
}

uint8_t WeekProgramExtData::getCurrentSize() const
{
  return m_size;
}

uint8_t WeekProgramExtData::getNumberOfSlots() const
{
  uint8_t numSlots = m_size / Constants::WEEKPROGRAM_SLOT_SIZE;
  return numSlots;
}

bool WeekProgramExtData::getWeekProgramNodeFromSlot(uint8_t slotNo, WeekProgramNode &outNode) const
{
  CloudTimerHeatingModeExt::Enum mode;
  uint8_t startHour, startMinute, endHour, endMinute;
  bool isSlotValid = getSlotData(slotNo, mode, startHour, startMinute, endHour, endMinute);

  if (isSlotValid)
  {
    HeatingMode::Enum heatingMode = convertSlotModeExtToHeatingMode(mode);
    uint8_t hours = convertValueToHours(startHour);
    uint8_t minutes = convertValueToMinutes(startMinute);

    WeekProgramNode node(outNode.interval, heatingMode, hours, minutes);
    outNode = node;
  }

  return isSlotValid;
}

bool WeekProgramExtData::isValid() const
{
  CloudTimerHeatingModeExt::Enum dummyMode = CloudTimerHeatingModeExt::comfort;
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

bool WeekProgramExtData::getSlotData(uint8_t slotNo,
                                     CloudTimerHeatingModeExt::Enum &mode,
                                     uint8_t &startHour,
                                     uint8_t &startMinute,
                                     uint8_t &endHour,
                                     uint8_t &endMinute) const
{
  bool isSlotDataValid = false;

  if (isSlotNoWithinRange(slotNo))
  {
    uint8_t startIndex = (slotNo - 1U) * Constants::WEEKPROGRAM_SLOT_SIZE;

    startHour = m_data[startIndex];
    startMinute = m_data[startIndex + 1u];
    endHour = m_data[startIndex + 2u];
    endMinute = m_data[startIndex + 3u];
    mode = static_cast<CloudTimerHeatingModeExt::Enum>(m_data[startIndex+4u]);

    isSlotDataValid = ((startHour != Constants::CLOUD_PARAMETER_INVALID_VALUE)
                        && (startMinute != Constants::CLOUD_PARAMETER_INVALID_VALUE)
                        && (endHour != Constants::CLOUD_PARAMETER_INVALID_VALUE)
                        && (endMinute != Constants::CLOUD_PARAMETER_INVALID_VALUE)
                        && (mode != Constants::CLOUD_PARAMETER_INVALID_VALUE));
  }

  return isSlotDataValid;
}

bool WeekProgramExtData::isSlotNoWithinRange(uint8_t slotNo) const
{
  return ((slotNo > 0U) && (slotNo <= getNumberOfSlots()));
}


HeatingMode::Enum WeekProgramExtData::convertSlotModeExtToHeatingMode(CloudTimerHeatingModeExt::Enum mode) const
{
  HeatingMode::Enum heatingMode;

  switch (mode) // Defined in parameter model for Compact Heaters: 0=Away, 1=Eco, 2=Comfort, 3=Off, 4=Standby(?)
  {
  case CloudTimerHeatingModeExt::away:
    heatingMode = HeatingMode::antiFrost;
    break;
  case CloudTimerHeatingModeExt::eco:
    heatingMode = HeatingMode::eco;
    break;
  case CloudTimerHeatingModeExt::shutdown:
  case CloudTimerHeatingModeExt::standby: // intentionally treat shutdown and standby as off
    heatingMode = HeatingMode::off;
    break;
  case CloudTimerHeatingModeExt::comfort: // intentionally treat any uncrecognized modes as comfort
  default:
    heatingMode = HeatingMode::comfort;
    break;
  }

  return heatingMode;
}


uint8_t WeekProgramExtData::convertValueToHours(uint8_t value) const
{
  return (value % 24U);
}


uint8_t WeekProgramExtData::convertValueToMinutes(uint8_t value) const
{
  return (value % 60U);
}
