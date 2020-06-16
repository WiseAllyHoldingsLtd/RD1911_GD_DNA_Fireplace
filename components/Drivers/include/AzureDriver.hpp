#pragma once

#include "iothub_client.h"
#include "Task.hpp"
#include "QueueInterface.hpp"
#include "FrameParser.h"
#include "Constants.h"
#include "CryptoDriverInterface.h"
#include "AsyncMessageQueueInterface.h"
#include "TimerDriverInterface.h"
#include "AzurePayload.h"
#include "CloudStatus.hpp"
#include "AzureDriverInterface.h"
#include "TransferFrameResponse.h"
#include "CloudDeviceMethodStatus.h"


class AzureDriver : public Task, public AzureDriverInterface
{
public:
  AzureDriver(QueueInterface<char[Constants::CLOUD_CONNECTION_STRING_MAX_LENGTH]> &queue, const CryptoDriverInterface &crypto,
              AsyncMessageQueueReadInterface &asyncQueue, TimerDriverInterface &timerDriver, AzurePayload &azurePayload);

  bool isConnected() const;
  bool isIdle() const;
  CloudStatus::Enum getStatus() const;
  void requestShutdown();
  bool isUnableToSend() const;

  bool connect(const char (&connectionString)[Constants::CLOUD_CONNECTION_STRING_MAX_LENGTH]);

  virtual void run(bool eternalLoop);

protected:
  void doConnectionLoop(const char * connectionString);

  // User callback handlers
  virtual void updateConnectionStatus(IOTHUB_CLIENT_CONNECTION_STATUS status, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason);
  virtual int processMessage(const char *methodName, const unsigned char *payload, size_t size, unsigned char **response, size_t *responseSize);
  virtual void processSendConfirmation(IOTHUB_CLIENT_CONFIRMATION_RESULT result);

  // static callback handlers
  static void sConnectionStatusCallback(IOTHUB_CLIENT_CONNECTION_STATUS status, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void *userContextCallback);
  static int sDeviceMethodCallback(const char *methodName, const unsigned char *payload, size_t size, unsigned char **response, size_t *responseSize, void *userContextCallback);
  static void sSendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void *userContextCallback);

private:
  CloudDeviceMethodStatus::Enum processReceivedFrame(const FrameParser & frame, TransferFrameResponse & responseFrame);
  CloudDeviceMethodStatus::Enum handleTimeSyncInfoRequest(const FrameParser & frame, TransferFrameResponse & responseFrame);
  CloudDeviceMethodStatus::Enum handleFirmwareUpdateReadyRequest(const FrameParser & frame, TransferFrameResponse & responseFrame);
  CloudDeviceMethodStatus::Enum handleMultipleFrameRequest(const FrameParser & frame, TransferFrameResponse & responseFrame);
  CloudDeviceMethodStatus::Enum handleOperationInfoRequest(const FrameParser & frame, TransferFrameResponse & responseFrame);
  CloudDeviceMethodStatus::Enum handleUnsupportedRequest(const FrameParser & frame) const;

  bool sendDeviceToCloudAsyncIfPossible(IOTHUB_CLIENT_LL_HANDLE iotHubHandle);
  bool isSendConfirmationTimeoutReached() const;

  QueueInterface<char[Constants::CLOUD_CONNECTION_STRING_MAX_LENGTH]> & m_connectionStringQueue;
  const CryptoDriverInterface &m_cryptoDriver;
  AsyncMessageQueueReadInterface &m_asyncMsgQueue;
  TimerDriverInterface &m_timerDriver;
  AzurePayload &m_azurePayload;

  volatile CloudStatus::Enum m_status;
  volatile bool m_shutdownRequested;
  IOTHUB_MESSAGE_HANDLE m_asyncSendMessageHandle;
  uint32_t m_asyncSendMessageCounter;
  uint32_t m_numConsecutiveSendTimeouts;
  volatile bool m_isCurrentlySendingAsync;
  volatile uint64_t m_currentAsyncSendStartTimestamp;
};
