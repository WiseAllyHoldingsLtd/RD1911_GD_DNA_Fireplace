#include "CppUTest\TestHarness.h"

#include "Constants.h"
#include "PersistentStorageDriverMock.h"
#include "DeviceMetaStorage.h"


static PersistentStorageDriverDummy *driverDummy;
static DeviceMetaStorage *storage;


TEST_GROUP(metaStorage)
{
  TEST_SETUP()
  {
    driverDummy = new PersistentStorageDriverDummy();
    storage = new DeviceMetaStorage(*driverDummy);
  }

  TEST_TEARDOWN()
  {
    delete storage;
    delete driverDummy;
  }
};


TEST(metaStorage, testWhenValueNotSetRetrieveShouldReturnFalse)
{
  char gdid[Constants::GDID_SIZE_MAX];
  CHECK_FALSE(storage->retrieveGDID(gdid, sizeof(gdid)));
}


TEST(metaStorage, testSettingAndRetrievingGDID)
{
  char gdid[Constants::GDID_SIZE_MAX] = {};

  storage->storeGDID("012345678901");
  CHECK_TRUE(storage->retrieveGDID(gdid, sizeof(gdid)));
  STRNCMP_EQUAL("012345678901", gdid, sizeof(gdid));
}


TEST(metaStorage, testSettingAndRetrievingBTPasskey)
{
  uint32_t passkey = 0;

  storage->storeBluetoothPasskey(224433);

  CHECK_TRUE(storage->retrieveBluetoothPasskey(passkey));
  CHECK_EQUAL(224433, passkey);
}


TEST(metaStorage, testSettingAndRetrievingEOLStatus)
{
  EOLStatus::Enum eolStatus = EOLStatus::notCompleted;

  storage->storeEOLStatus(EOLStatus::completed);
  CHECK_TRUE(storage->retrieveEOLStatus(eolStatus));
  CHECK_EQUAL(EOLStatus::completed, eolStatus);

  storage->storeEOLStatus(EOLStatus::notCompleted);
  CHECK_TRUE(storage->retrieveEOLStatus(eolStatus));
  CHECK_EQUAL(EOLStatus::notCompleted, eolStatus);
}
