
#pragma once

class CloudServiceInterface
{
public:
  virtual ~CloudServiceInterface() { }
  virtual bool isRunning() const = 0;
  virtual uint8_t getCurrentHealth() const = 0;
  virtual bool requestPause() = 0;
  virtual bool requestUnpause() = 0;
  virtual bool isPaused() const = 0;
  virtual void waitUntilPausedOrUnpaused(bool waitForPaused) = 0;
  virtual bool wasDiscoverySuccessfulSinceLastRestart() const = 0;
};

