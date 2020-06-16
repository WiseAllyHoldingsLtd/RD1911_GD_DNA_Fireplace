#pragma once

struct CloudDeviceMethodStatus {
  enum Enum {
    OK = 200,
    METHOD_NOT_ALLOWED = 405,
    TIMEOUT = 408,
  };
};
