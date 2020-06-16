#include <cstring>
#include "EspCpp.hpp"
#include "FirmwareVerifier.h"


namespace
{
  const char LOG_TAG[] = "FWVerify";
}


FirmwareVerifier::FirmwareVerifier(SettingsInterface & settings)
  : m_settings(settings), m_versionCode(0U), m_attemptNo(0U)
{
  memset(m_firmwareID, 0, sizeof(m_firmwareID));
}


void FirmwareVerifier::init(void)
{
  m_attemptNo = m_settings.getFirmwareUpgradeCount();
  m_versionCode = m_settings.getFirmwareVersionCode();
  m_settings.getFirmwareID(m_firmwareID, sizeof(m_firmwareID));
}

void FirmwareVerifier::saveToSettings(void)
{
  m_settings.setFirmwareUpgradeCount(m_attemptNo);
  m_settings.setFirmwareVersionCode(m_versionCode);
  m_settings.setFirmwareID(m_firmwareID);
}

const char * FirmwareVerifier::getFirmwareUpgradeID(void) const
{
  return m_firmwareID;
}

uint32_t FirmwareVerifier::getFirmwareVersionCode(void) const
{
  return m_versionCode;
}

uint8_t FirmwareVerifier::getFirmwareUpgradeCount(void) const
{
  return m_attemptNo;
}

bool FirmwareVerifier::isUpgrading(void) const
{
  return (strcmp("", getFirmwareUpgradeID()) != 0);
}

bool FirmwareVerifier::isUpgradeBlocked() const
{
  return (isUpgrading() && (getFirmwareUpgradeCount() >= Constants::FW_UPGRADE_MAX_ATTEMPTS));
}

bool FirmwareVerifier::registerUpgradeAttempt(const char * upgradeUrl)
{
  bool wasRegistered = false;

  if ((upgradeUrl != nullptr)
      && (strnlen(upgradeUrl, Constants::FW_DOWNLOAD_URL_MAX_LENGTH) > 0U)
      && (strnlen(upgradeUrl, Constants::FW_DOWNLOAD_URL_MAX_LENGTH) < Constants::FW_DOWNLOAD_URL_MAX_LENGTH))
  {
    char id[Constants::FW_DOWNLOAD_ID_MAX_LENGTH];

    if (getFirmwareIDFromUrl(upgradeUrl, id, sizeof(id)))
    {
      if (strcmp(id, m_firmwareID) != 0)
      {
        // new ID, register new attempt
        strcpy(m_firmwareID, id);
        m_attemptNo = 1U;
        m_versionCode = calculateVersionCode();
        wasRegistered = true;
        ESP_LOGI(LOG_TAG, "Firmware upgrade attemp no %u registered for ID %s.", m_attemptNo, m_firmwareID);
      }
      else
      {
        // same ID as before, register retry
        if (!isUpgradeBlocked())
        {
          ++m_attemptNo;
          wasRegistered = true;
          ESP_LOGI(LOG_TAG, "Firmware upgrade attemp no %u registered for ID %s.", m_attemptNo, m_firmwareID);
        }
        else
        {
          ESP_LOGW(LOG_TAG, "Firmware upgrade blocked - failed too many times (%u).", m_attemptNo);
        }
      }
    }
    else
    {
      ESP_LOGE(LOG_TAG, "Firmware upgrade blocked - URL invalid format (%s)", upgradeUrl);
    }
  }
  else
  {
    ESP_LOGE(LOG_TAG, "Firmware upgrade blocked - URL invalid size");
  }

  return wasRegistered;
}

bool FirmwareVerifier::verifyUpgrade(void)
{
  bool wasVerified = false;

  if (isUpgrading())
  {
    // we simply evaluate by checking if current installed FW version is newer (i.e. higher value) than
    // the version code registered in settings
    uint32_t currentVersionCode = calculateVersionCode();

    if (currentVersionCode > m_versionCode)
    {
      m_versionCode = 0U;
      m_attemptNo = 0U;
      memset(m_firmwareID, 0, sizeof(m_firmwareID));
      wasVerified = true;
      ESP_LOGI(LOG_TAG, "Verification OK - running FW version code is 0x%08x, version before upgrade attempt was 0x%08x",
          currentVersionCode,
          m_versionCode);
    }
    else
    {
      ESP_LOGI(LOG_TAG, "Verification failed - running FW version code is 0x%08x, version before upgrade attempt was 0x%08x",
          currentVersionCode,
          m_versionCode);
    }
  }
  else
  {
    ESP_LOGI(LOG_TAG, "Verification failed - no upgrade in progress.");
  }

  return wasVerified;
}

bool FirmwareVerifier::getFirmwareIDFromUrl(const char * url, char * id, uint32_t idSize) const
{
  /*
   * We want the URL to be unique in order for blocking/unblocking to work. If URL doesn't look as expected we
   * just store the first part of the string as a last resort.
   * Expected URL format: https://<some-azure-url>/firmwares/directAppliance/<uuid-like-data>_<filename>?<queryString>.
   *   => we look for first occurence of '?', then the last occurence of '/' in the string before '?'
   *   => if no '?' found we use everything from last '/'.
   *   => if no '/' found we just use the last part of the url string (which clearly isn't a URL)
   */
  bool wasSet = false;
  uint32_t urlLength = strlen(url);
  char * queryString = strchr(url, '?');
  char * searchString = nullptr;

  if (queryString != nullptr)
  {
    // create a separate string for url up to but not including the '?'
    uint32_t searchStringLength = (urlLength - strlen(queryString) + 1U);
    searchString = new char[searchStringLength];

    strncpy(searchString, url, searchStringLength);
    searchString[searchStringLength - 1U] = '\0'; // this removes the '?'
  }

  // Now we either have what we need is searchString or we must use the url
  char * fileName;

  if (searchString != nullptr)
  {
    fileName = strrchr(searchString, '/');
  }
  else
  {
    fileName = strrchr(url, '/');
  }

  // Now we either have the filename we want or we have no clue what 'url' really is...
  if (fileName != nullptr)
  {
    strncpy(id, fileName, idSize);
    id[idSize - 1U] = '\0';
    wasSet = true;
  }

  if (searchString != nullptr)
  {
    delete [] searchString;
    searchString = nullptr;
  }

  return wasSet;
}

uint32_t FirmwareVerifier::calculateVersionCode(void) const
{
  return static_cast<uint32_t>((Constants::SW_VERSION << 8U) + Constants::SW_TEST_NO);
}
