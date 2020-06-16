#include <cstdint>

#include "DateTime.h"
#include "Constants.h"
#include "TWIDriverInterface.h"
#include "Rtc.h"


Rtc::Rtc(TWIDriverInterface &twiDriver) : m_twiDriver(twiDriver)
{
}

bool Rtc::setup(void)
{
  uint8_t buffer[4];
  bool result = true;

  // Set Control register 1
  buffer[0] = 0x00u;
  /*
  bits:
    7 - load capacitance: 0=7pF, 1=12.5pF
    6 - unused
    5 - 0=RTC time circuits running, 1=RTC time circuits frozen
    4 - 0=no software reset, 1=initiate software reset
    3 - 0=24 hour mode, 1=12 hour mode
    2 - second interrupt: 0=disabled, 1=enabled
    1 - alarm interrupt:  0=disabled, 1=enabled
    0 - correction interrupt: 0=disabled, 1=interrupt pulses generated at every
          correction cycle
  */
  buffer[1] = 0x00u;

  // Set Control register 2
  /*
  bits:
    7 - READONLY: watchdog timer interrupt flag (set when watchdog time A
          generates interrupt; cleared by reading control register 2(this one)
    6 - countdown timer A: 0=no interrupt generated, 1=interrupt generated
          flag must be manually cleared
    5 - countdown timer B: 0=no interrupt generated, 1=interrupt generated
          flag must be manually cleared
    4 - second interrupt: 0=no interrupt generated, 1=interrupt generated
          flag must be manually cleared
    3 - alarm interrupt: 0=no interrupt generated, 1=interrupt generated
          flag must be manually cleared
    2 - watchdog timer A interrupt: 0=disabled, 1=enabled
    1 - countdown timer A interrupt: 0=disabled, 1=enabled
    0 - countdown timer B interrupt: 0=disabled, 1=enabled
  */
  buffer[2] = 0x00u;

  // Set Control register 3
  /*
  bits
   7 to 5 - battery switch-over and battery low detection control
   4 - unused
   3 - battery switch-over interrupt (BSF): 0=no interrupt generated,
      1=interrupt generated
      flag must be manually cleared
   2 - READONLY battery status: 0=OK, 1= battery low
   1 - battery switch-over interrupt: 0=disabled, 1=enabled when BSF is set
   0 - battery low interrupt: 0=disabled, 1=enabled when BSF is set
  */
  buffer[3] = 0x00u;
  result = m_twiDriver.send(CLOCK_SLAVE_ADDRESS, buffer, 4u);

  // Tmr_CLKOUT_ctrl
  buffer[0] = 0x0Fu;
  /*
  bits
    7 - TAM:
      0 = permanent active interrupt for timer A and for the second interrupt timer
      1 = pulsed interrupt for timer A and the second interrupt timer
    6 - TBM:
      0 = permanent active interrupt for timer B
      1 = pulsed interrupt for timer B
    5 to 3 - COF[2:0]: CLKOUT frequency selection
    2 to 1 - TAC[1:0]:
      00 or 11 = timer A is disabled
      01 = timer A is configured as countdown timer
      10 = timer A is configured as watchdog timer
    0 - TBC:
      0 = timer B is disabled
      1 = timer B is enabled
  */
  buffer[1] = (0x07u << 3u); /* CLKOUT disabled */
  if (result) {
    result = m_twiDriver.send(CLOCK_SLAVE_ADDRESS, buffer, 2u);
  }
  return result;
}

void Rtc::setCurrentTime(const DateTime &dateTime)
{
  uint8_t buffer[8];

  /* Point to seconds register */
  buffer[0] = 0x03u;

  /* Seconds: BCD format, bits 6-4 ten's place,bits 3-0 unit place */
  buffer[1] = ((dateTime.seconds/10u) << 4u) | (dateTime.seconds%10u);
  /* Minutes: BCD format, bits 6-4 ten's place,bits 3-0 unit place */
  buffer[2] = ((dateTime.minutes/10u) << 4u) | (dateTime.minutes%10u);
  /* Hours (24h): BCD format, bits 5-4 ten's place,bits 3-0 unit place */
  buffer[3] = ((dateTime.hours/10u) << 4u) | (dateTime.hours%10u);
  /* Days: BCD format, bits 5-4 ten's place,bits 3-0 unit place */
  buffer[4] = ((dateTime.days/10u) << 4u) | (dateTime.days%10u);
  /* Weekday: actual weekday, bits 2-0 */
  buffer[5] = static_cast<uint8_t>(dateTime.weekDay);
  /* Month: BCD format, bit 4 ten's place,bits 3-0 unit place */
  buffer[6] = ((dateTime.month/10u) << 4u) | (dateTime.month%10u);
  /* Year: BCD format, bits 7-4 ten's place,bits 3-0 unit place */
  buffer[7] = ((dateTime.year/10u) << 4u) | (dateTime.year%10u);

  for (uint8_t i(0u); i<RETRY_COUNT; ++i) {
    if (m_twiDriver.send(CLOCK_SLAVE_ADDRESS, buffer, 8u)) {
      break;
    }
  }
}

TimeStatus::Enum Rtc::getCurrentTime(DateTime &dateTime)
{
  uint8_t buffer[7] = {0, 0, 0, 0, 0, 0, 0};
  TimeStatus::Enum result = TimeStatus::OK;

  /* Point to seconds register */
  buffer[0] = 0x03u;

  bool transferOK = false;
  if (m_twiDriver.send(CLOCK_SLAVE_ADDRESS, buffer, 1u)) {
    if (m_twiDriver.recieve(CLOCK_SLAVE_ADDRESS, buffer, 7u)) {
      transferOK = true;
      if ((buffer[0] & (1u << 7u)) != 0u) {
        result = TimeStatus::INTEGRITY_LOST;
      }
    }
  }
  if (!transferOK) {
    result = TimeStatus::TIMEOUT;
  }

  /* Return read data only if clock integrity is guaranteed. */

  if (result == TimeStatus::OK)
  {
    /* Seconds: BCD format, bits 6-4 ten's place,bits 3-0 unit place */
    dateTime.seconds = ((buffer[0] >> 4u) & 0x07u) * 10u + (buffer[0] & 0x0Fu);
    /* Minutes: BCD format, bits 6-4 ten's place,bits 3-0 unit place */
    dateTime.minutes = ((buffer[1] >> 4u) & 0x07u) * 10u + (buffer[1] & 0x0Fu);
    /* Hours (24h): BCD format, bits 5-4 ten's place,bits 3-0 unit place */
    dateTime.hours = ((buffer[2] >> 4u) & 0x03u) * 10u + (buffer[2] & 0x0Fu);
    /* Days: BCD format, bits 5-4 ten's place,bits 3-0 unit place */
    dateTime.days = ((buffer[3] >> 4u) & 0x03u) * 10u + (buffer[3] & 0x0Fu);
    /* Weekday: actual weekday, bits 2-0 */
    dateTime.weekDay = static_cast<WeekDay::Enum>(buffer[4] & 0x07u);
    /* Month: BCD format, bit 4 ten's place,bits 3-0 unit place */
    dateTime.month = ((buffer[5] >> 4u) & 0x01u) * 10u + (buffer[5] & 0x0Fu);
    /* Year: BCD format, bits 7-4 ten's place,bits 3-0 unit place */
    dateTime.year = ((buffer[6] >> 4u) & 0x0Fu) * 10u + (buffer[6] & 0x0Fu);
  }
  else
  {
    dateTime = Constants::DEFAULT_DATETIME;
  }

  return result;
}
