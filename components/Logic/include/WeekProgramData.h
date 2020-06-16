#pragma once
#include "Constants.h"
#include "DateTime.h"
#include "HeatingMode.h"
#include "WeekProgramNode.h"

/*
 * Week program data as transferred from cloud and stored in persistent memory
 * Expected format (20 bytes, four slots of five bytes each):
 *  Byte 0: Slot 1 mode (different mapping than specified in HeatingMode.h)
 *  Bytes 1,2: Slot 1 startTime [hours,minutes]
 *  Bytes 3,4: Slot 1 endTime [hours, minutes]
 *  Byte 5: Slot 2 mode
 *  ....
 *  Bytes 18,19: Slot 4 endTime [hours, minutes]
 */

class WeekProgramData
{
public:
  WeekProgramData(void);
  WeekProgramData(const WeekProgramData& other);
  bool operator==(const WeekProgramData& rhs);
  bool operator!=(const WeekProgramData& rhs);

  void getData(uint8_t (&data)[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY]) const;
  void setData(const uint8_t (&data)[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY]);

  uint8_t getNumberOfSlots(void) const;
  bool getWeekProgramNodeFromSlot(uint8_t slotNo, WeekProgramNode & outNode) const;

  bool isValid() const;
private:
  bool getSlotData(uint8_t slotNo, CloudTimerHeatingMode::Enum & mode, uint8_t & startHour, uint8_t & startMinute, uint8_t & endHour, uint8_t & endMinute) const;
  bool isSlotNoWithinRange(uint8_t slotNo) const;

  HeatingMode::Enum convertSlotModeToHeatingMode(CloudTimerHeatingMode::Enum mode) const;
  uint8_t convertValueToHours(uint8_t value) const;
  uint8_t convertValueToMinutes(uint8_t value) const;

  uint8_t m_data[Constants::WEEKPROGRAM_STORAGE_SIZE_PER_DAY];
};
