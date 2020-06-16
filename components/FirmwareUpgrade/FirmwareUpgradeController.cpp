#include <cstring>
#include <cstdio>
#include "HTTPText.h"
#include "HTTPBinary.h"
#include "HTTPHead.h"
#include "FirmwareUpgradeController.h"
#include "OTADriverInterface.h"
#include "FreeRTOS.hpp"


namespace
{
  const char RANGE_KEY[] = "Range";
  const char RANGE_VALUE_TEMPLATE[] = "bytes=%u-%u";
  const uint32_t MAXIMUM_RANGE_VALUE_LENGTH = 26U; /* %u = uint32_t, i.e. max 9 bytes */
  const uint32_t DEFAULT_DELAY_BETWEEN_DOWNLOADS_MS = 1000U;
}


FirmwareUpgradeController::FirmwareUpgradeController(
    OTADriverInterface &otaDriver, CloudDriverInterface &cloudDriver, FirmwareVerifierInterface &verifier,
    QueueInterface<ConnectionStatusChangeRequest::Enum> &connStatusChangeQueue)
  : m_otaDriver(otaDriver), m_cloudDriver(cloudDriver), m_verifier(verifier), m_fwUpdateReadyQueue(connStatusChangeQueue)
{
  m_upgradeUrl[0U] = '\0';
}


void FirmwareUpgradeController::setFirmwareUrl(const char * url)
{
  strncpy(m_upgradeUrl, url, Constants::FW_DOWNLOAD_URL_MAX_LENGTH);
  m_upgradeUrl[Constants::FW_DOWNLOAD_URL_MAX_LENGTH - 1U] = '\0';
}

bool FirmwareUpgradeController::signalFirmwareUpgradeRequest(void)
{
  bool wasRegistered = m_verifier.registerUpgradeAttempt(m_upgradeUrl);

  if (wasRegistered)
  {
    m_verifier.saveToSettings();
    m_fwUpdateReadyQueue.push(ConnectionStatusChangeRequest::fwUpgradeReady, 0U);
  }

  return wasRegistered;
}

bool FirmwareUpgradeController::isFirmwareUpgradeRequested(void) const
{
  return (strnlen(m_upgradeUrl, Constants::FW_DOWNLOAD_URL_MAX_LENGTH) > 0U);
}


bool FirmwareUpgradeController::initOTA(void)
{
  m_otaDriver.resetAndInit();
  return m_otaDriver.beginUpdate();
}


bool FirmwareUpgradeController::commitOTA(void)
{
  bool wasSaved = m_otaDriver.endUpdate();

  if (wasSaved)
  {
    wasSaved = m_otaDriver.activateUpdatedPartition();
  }

  return wasSaved;
}


uint32_t FirmwareUpgradeController::getOTADataSize(void) const
{
  uint32_t dataSize = 0U;

  if (isFirmwareUpgradeRequested())
  {
    HTTPHead httpResponse;
    bool requestReturnedOK = m_cloudDriver.headRequest(m_upgradeUrl, httpResponse);

    if (requestReturnedOK)
    {
      dataSize = httpResponse.getContentLength();
    }
  }

  return dataSize;
}


bool FirmwareUpgradeController::downloadAndWriteOTAData(uint32_t startByteIndex, uint32_t endByteIndex, uint8_t numOfDownloadAttempts)
{
  bool wasWritten = false;

  if (isFirmwareUpgradeRequested() && (startByteIndex < endByteIndex))
  {
    uint32_t requestedBytes = static_cast<uint32_t>(endByteIndex - startByteIndex + 1U);

    char httpRangeHeaderValue[MAXIMUM_RANGE_VALUE_LENGTH];
    prepareHttpHeaderRangeValue(startByteIndex, endByteIndex, httpRangeHeaderValue);
    const char * rangeHeader[2U] = { RANGE_KEY, httpRangeHeaderValue };

    char fileContents[Constants::FW_DOWNLOAD_CHUNK_SIZE];

    uint8_t attemptNo = 0U;
    bool requestReturnedOK = false;

    do
    {
      ++attemptNo;
      HTTPBinary response(fileContents, sizeof(fileContents));
      requestReturnedOK = m_cloudDriver.getRequestWithCustomHeader(m_upgradeUrl, response, rangeHeader);

      if (requestReturnedOK)
      {
        uint32_t receivedBytes = response.getDataLength();

        if (receivedBytes == requestedBytes)
        {
          wasWritten = m_otaDriver.write(fileContents, receivedBytes);
        }
      }
      else
      {
        FreeRTOS_delay_ms(attemptNo * DEFAULT_DELAY_BETWEEN_DOWNLOADS_MS);
      }

    } while(!requestReturnedOK && (attemptNo < numOfDownloadAttempts));
  }

  return wasWritten;
}


void FirmwareUpgradeController::prepareHttpHeaderRangeValue(uint32_t startByteIndex, uint32_t endByteIndex, char * value) const
{
  snprintf(value, MAXIMUM_RANGE_VALUE_LENGTH, RANGE_VALUE_TEMPLATE, startByteIndex, endByteIndex);
}
