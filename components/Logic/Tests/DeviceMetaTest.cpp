#include "CppUTest\TestHarness.h"
#include "CppUTestExt\MockSupport.h"

#include "DeviceMeta.h"
#include "Constants.h"
#include "PersistentStorageDriverMock.h"
#include "DeviceMetaStorage.h"

static DeviceMeta* meta;
static PersistentStorageDriverDummy * storageDriverDummy;
static DeviceMetaStorage * storage;


TEST_GROUP(deviceMeta)
{
  TEST_SETUP()
  {
    storageDriverDummy = new PersistentStorageDriverDummy();
    storage = new DeviceMetaStorage(*storageDriverDummy);
    meta = new DeviceMeta(*storage);
  }

  TEST_TEARDOWN()
  {
    mock().checkExpectations();
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
    delete meta;
    delete storage;
    delete storageDriverDummy;  }
};


TEST(deviceMeta, testObjectIsNotModifiedByDefault)
{
  CHECK_FALSE(meta->isModified());
}


TEST(deviceMeta, testGetAndSetGDID)
{
  char gdid[Constants::GDID_SIZE_MAX] = {};

  meta->setGDID("deviceGDID");
  meta->getGDID(gdid, sizeof(gdid));
  STRNCMP_EQUAL("deviceGDID", gdid, sizeof(gdid));
  CHECK_EQUAL(strlen("deviceGDID"), strlen(gdid));
}

TEST(deviceMeta, testGetGDIDAsBCD)
{
  meta->setGDID("123456789009");  // default size is 12

  uint8_t gdidAsBCD[Constants::GDID_BCD_SIZE] = {};
  meta->getGDIDAsBCD(gdidAsBCD);

  uint8_t expectedResult[Constants::GDID_BCD_SIZE] = { 0x09, 0x90, 0x78, 0x56, 0x34, 0x12 };
  MEMCMP_EQUAL(expectedResult, gdidAsBCD, Constants::GDID_BCD_SIZE);
}

TEST(deviceMeta, testGetGDIDAsBCDWithHex)
{
  meta->setGDID("123456789ABF");  // default size is 12

  uint8_t gdidAsBCD[Constants::GDID_BCD_SIZE] = {};
  meta->getGDIDAsBCD(gdidAsBCD);

  uint8_t expectedResult[Constants::GDID_BCD_SIZE] = { 0xBF, 0x9A, 0x78, 0x56, 0x34, 0x12 };
  MEMCMP_EQUAL(expectedResult, gdidAsBCD, Constants::GDID_BCD_SIZE);
}

TEST(deviceMeta, testGetGDIDAsBCDUnevenSize)
{
  meta->setGDID("123456789");

  uint8_t gdidAsBCD[Constants::GDID_BCD_SIZE] = {};
  meta->getGDIDAsBCD(gdidAsBCD);

  uint8_t expectedResult[Constants::GDID_BCD_SIZE] = { 0x89, 0x67, 0x45, 0x23, 0x01, 0x00 };
  MEMCMP_EQUAL(expectedResult, gdidAsBCD, Constants::GDID_BCD_SIZE);
}

TEST(deviceMeta, testGetGDIDWithTooShortBufferProvided)
{
  char gdid[2U];

  meta->setGDID("myGDID");
  meta->getGDID(gdid, sizeof(gdid));

  STRNCMP_EQUAL("m", gdid, sizeof(gdid));
  CHECK_EQUAL(1U, strlen(gdid));
}

TEST(deviceMeta, testSetGDIDLongerThanSupported)
{
  char gdid[Constants::GDID_SIZE_MAX + 2U];

  for (uint8_t i = 0; i < sizeof(gdid); i++)
  {
    gdid[i] = 'g';
  }

  meta->setGDID(gdid);
  meta->getGDID(gdid, sizeof(gdid));

  CHECK_EQUAL(Constants::GDID_SIZE_MAX - 1U, strlen(gdid));
  CHECK_EQUAL('g', gdid[0U]);
  CHECK_EQUAL('g', gdid[Constants::GDID_SIZE_MAX - 2U]);
}


TEST(deviceMeta, testGetAndSetBluetoothPasskey)
{
  meta->setBluetoothPasskey(123U);
  CHECK_EQUAL(123U, meta->getBluetoothPasskey());

  meta->setBluetoothPasskey(456U);
  CHECK_EQUAL(456U, meta->getBluetoothPasskey());
}


TEST(deviceMeta, testGetAndSetEOLStatus)
{
  meta->setEndOfLineStatus(EOLStatus::completed);
  CHECK_EQUAL(EOLStatus::completed, meta->getEndOfLineStatus());

  meta->setEndOfLineStatus(EOLStatus::notCompleted);
  CHECK_EQUAL(EOLStatus::notCompleted, meta->getEndOfLineStatus());
}

TEST(deviceMeta, testSetInvalidEOLStatusInFactSetsDefault)
{
  meta->setEndOfLineStatus(static_cast<EOLStatus::Enum>(0x45678));
  CHECK_EQUAL(Constants::DEFAULT_EOL_STATUS, meta->getEndOfLineStatus());
}

TEST(deviceMeta, testUsingGettersShouldNotTriggerModified)
{
  char gdid[Constants::GDID_SIZE_MAX] = {};
  meta->getGDID(gdid, sizeof(gdid));
  meta->getBluetoothPasskey();
  meta->getEndOfLineStatus();

  CHECK_FALSE(meta->isModified());
}

TEST(deviceMeta, testUsingSettersShouldTriggerModified)
{
  meta->setBluetoothPasskey(123U);
  CHECK_TRUE(meta->isModified());
}

TEST(deviceMeta, testUsingSettersForStringsShouldTriggerModified)
{
  meta->setGDID("id");
  CHECK_TRUE(meta->isModified())
}

TEST(deviceMeta, testStoringMetaSavesToPersistentStorage)
{
  meta->setGDID("someGDID");
  meta->setBluetoothPasskey(3421U);
  meta->setEndOfLineStatus(EOLStatus::completed);
  meta->storeMeta();

  char gdid[Constants::GDID_SIZE_MAX] = {};
  uint32_t btKey = 0U;
  EOLStatus::Enum eolStatus = EOLStatus::notCompleted;

  CHECK_TRUE(storage->retrieveGDID(gdid, sizeof(gdid)));
  STRCMP_EQUAL("someGDID", gdid);

  CHECK_TRUE(storage->retrieveBluetoothPasskey(btKey));
  CHECK_EQUAL(3421U, btKey);

  CHECK_TRUE(storage->retrieveEOLStatus(eolStatus));
  CHECK_EQUAL(EOLStatus::completed, eolStatus);
}

TEST(deviceMeta, testRetrievingMetaLoadsFromPersistentStorage)
{
  CHECK_TRUE(strlen("gdidString") != strlen(Constants::DEFAULT_GDID));
  static_assert(123456U != Constants::DEFAULT_BT_PASSKEY, "Use different testing BTPasskey");
  static_assert(EOLStatus::completed != Constants::DEFAULT_EOL_STATUS, "Use different testing EOLStatus");
  storage->storeGDID("gdidString");
  storage->storeBluetoothPasskey(123456U);
  storage->storeEOLStatus(EOLStatus::completed);

  meta->retrieveMeta();

  char retrievedGDID[Constants::GDID_SIZE_MAX];
  meta->getGDID(retrievedGDID, sizeof(retrievedGDID));

  STRCMP_EQUAL("gdidString", retrievedGDID);
  CHECK_EQUAL(123456U, meta->getBluetoothPasskey());
  CHECK_EQUAL(EOLStatus::completed, meta->getEndOfLineStatus());
}
