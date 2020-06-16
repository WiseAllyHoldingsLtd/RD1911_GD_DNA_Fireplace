#pragma once

#include "Constants.h"
#include "DateTime.h"
#include "HeatingMode.h"
#include "WeekProgramNode.h"

/*
 * Week program extended data as transferred from cloud and stored in persistent memory
 * Expected format (variable size, variable number of slots (n) of five bytes each):
 *  Bytes 0,1: Slot 1 startTime [hours,minutes]
 *  Bytes 2,3: Slot 1 endTime [hours, minutes]
 *  Byte 4: Slot 1 mode (different mapping than specified in HeatingMode.h)
 *  Byte 5,6: Slot 2 startTime [hours,minutes]
 *  ....
 *  Bytes n*5: Slot 4 endTime [hours, minutes]
 */
class WeekProgramExtData
{
public:
  WeekProgramExtData(void);
  WeekProgramExtData(const WeekProgramExtData& other);
  ~WeekProgramExtData();
  bool operator==(const WeekProgramExtData& rhs);
  bool operator!=(const WeekProgramExtData& rhs);
  WeekProgramExtData &operator=(const WeekProgramExtData &rhs);

  void getData(uint8_t *data, uint8_t size) const;
  void setData(const uint8_t *data, uint8_t size);

  uint8_t getCurrentSize() const;

  uint8_t getNumberOfSlots() const;
  bool getWeekProgramNodeFromSlot(uint8_t slotNo, WeekProgramNode & outNode) const;

  bool isValid() const;
private:
  bool getSlotData(uint8_t slotNo,
                   CloudTimerHeatingModeExt::Enum & mode,
                   uint8_t & startHour,
                   uint8_t & startMinute,
                   uint8_t & endHour,
                   uint8_t & endMinute) const;
  bool isSlotNoWithinRange(uint8_t slotNo) const;

  HeatingMode::Enum convertSlotModeExtToHeatingMode(CloudTimerHeatingModeExt::Enum mode) const;
  uint8_t convertValueToHours(uint8_t value) const;
  uint8_t convertValueToMinutes(uint8_t value) const;

  uint8_t *m_data;
  uint8_t m_size;
};
