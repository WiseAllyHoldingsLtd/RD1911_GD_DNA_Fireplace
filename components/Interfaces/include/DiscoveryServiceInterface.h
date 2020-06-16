#pragma once

#include "Constants.h"


struct ConnectionStrings_t {
  char primary[Constants::CLOUD_CONNECTION_STRING_MAX_LENGTH];
  char secondary[Constants::CLOUD_CONNECTION_STRING_MAX_LENGTH];
};

class DiscoveryServiceInterface
{
public:
  virtual ~DiscoveryServiceInterface() {}

  virtual bool registerDevice() = 0; // primary & secondary here?
};
