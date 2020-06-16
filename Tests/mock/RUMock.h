#pragma once

#include "RUInterface.h"
#include "CppUTestExt\MockSupport.h"

#define NAME(method) "RUMock::" method

class RURegulatorComparator : public MockNamedValueComparator
{
public:
  RURegulatorComparator(){}

  virtual bool isEqual(const void* object1, const void* object2)
  {
    const RURegulator * enum1 = static_cast<const RURegulator *>(object1);
    const RURegulator * enum2 = static_cast<const RURegulator *>(object2);
    return *enum1 == *enum2;
  }

  SimpleString valueToString(const void* object)
  {
    const RURegulator * enum1 = static_cast<const RURegulator *>(object);

    return StringFrom(*enum1);
  }
};
static RURegulatorComparator ruRegulatorComparator;


class RUMock : public RUInterface
{
  virtual bool updateConfig(RURegulator regulator, bool useInternalTemp, bool enableRUPot, int32_t newSetpointFP)
  {
    return mock().actualCall(NAME("updateConfig"))
        .withParameterOfType("RURegulator", "regulator", (void*)&regulator)
        .withBoolParameter("useInternalTemp", useInternalTemp)
        .withBoolParameter("enableRUPot", enableRUPot)
        .withIntParameter("newSetpointFP", newSetpointFP)
        .returnBoolValue();
  }

  virtual int getSWVersion()
  {
    return mock().actualCall(NAME("getSWVersion")).returnIntValue();
  }

  virtual bool isHighPowerVersion() const
  {
    return mock().actualCall(NAME("isHighPowerVersion")).returnBoolValueOrDefault(true);
  }

  virtual bool updateExternalTemperatureFP(int32_t temperatureFP)
  {
    return mock().actualCall(NAME("updateExternalTemperatureFP"))
        .withIntParameter("temperatureFP", temperatureFP)
        .returnBoolValue();
  }

  virtual int32_t getEstimatedRoomTemperatureFP()
  {
    return mock().actualCall(NAME("getEstimatedRoomTemperatureFP")).returnIntValue();
  }

  virtual int32_t getRegulatorPotMeterFP()
  {
    return mock().actualCall(NAME("getRegulatorPotMeterFP")).returnIntValue();
  }

  virtual bool isHeatElementActive()
  {
    return mock().actualCall(NAME("isHeatElementActive")).returnBoolValue();
  }

  virtual bool performSelfTest()
  {
    return mock().actualCall(NAME("performSelfTest")).returnBoolValue();
  }
};
