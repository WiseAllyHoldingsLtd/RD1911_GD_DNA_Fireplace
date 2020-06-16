#pragma once

struct WifiConnectionStatus
{
  /**
    These constants mirror the communication sequence diagram in
    "GD Gateway BLE Interface Specification V0.9" from Firmwave.
  */
  enum Enum : uint16_t
  {
    IN_PROGRESS=1000u, // "Initial status characteristic value
                      // - Undefined/In progress"

    CONNECTION_REQUEST=2000u,

    INTERNET_CONNECTION_ESTABLISHED=400u,

    FULLY_FUNCTIONAL=0u,
    DISCOVERY_AMQP_CONNECTION_REJECTED=301u,
    DISCOVERY_AMQP_SERVICE_SERVER_INACCESSIBLE=302u,
    DISCOVERY_SERVICE_DEVICE_REJECTED=303u,

    FW_AUTH_SECURE_BOOT_FAILURE=101u,
    HW_FAILURE_DEADLOCK=102u,
    DHCP_FAILURE=201u,
    DNS_HOST_RESOLUTION_FAILURE=202u,
    ETHERNET_SETTINGS_INVALID=203u,
    WIFI_NOT_DISCOVERED=204u,
    WIFI_PASSWORD_INVALID=205u
  };
};
