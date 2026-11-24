# ORA Mobile network API change (source only)

## Compatibility boundary

The installed ORA firmware remains unchanged. Firmware 1.0.1 does not implement `/api/network/config`, so ORA Mobile must show **Kërkon firmware të ri** and must not enable the network configuration form when this route is missing. No firmware image was uploaded as part of this work.

The source-only firmware change adds `GET` and `POST /api/network/config` in `src/WebServerManager.cpp` and persistent DHCP/static IPv4 settings in `src/WiFiManager.h` and `src/WiFiManager.cpp`. The API is for a later firmware release; do not treat it as available on 1.0.1.

## API contract

`GET /api/network/config` returns `mode` (`dhcp` or `static`), `ipAddress`, `gateway`, `subnetMask`, `dns1`, `dns2`, `currentIp`, `wifiConnected`, and `rssi`.

`POST /api/network/config` accepts JSON `mode`, `ipAddress`, `gateway`, `subnetMask`, `dns1`, and `dns2`. Static values are validated for IPv4 syntax, contiguous subnet mask, host address, and same-subnet gateway. A valid configuration is persisted in the existing `wifi` NVS namespace and the ESP32 restarts to apply it. Errors return HTTP 400 with a short `error` code. Existing Wi-Fi credentials are not returned or modified by this API.

## Files

- `src/WebServerManager.cpp`: route registration and JSON API handlers.
- `src/WiFiManager.h`: persisted network configuration data and methods.
- `src/WiFiManager.cpp`: NVS loading, DHCP/static connection setup, validation, and persistence.

This API source change is intentionally separate from the already authorized OTA metadata work in `src/OtaManager.cpp`. Build and on-device validation are required before publishing a firmware release. This document does not indicate that the API has been installed on an ORA device.
