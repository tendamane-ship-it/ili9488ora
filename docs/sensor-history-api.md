# ORA sensor history API, schema version 1

The history API is additive. Existing routes and their payloads are unchanged. All timestamps are UTC Unix seconds. Records are ordered by ascending sequence ID. Synchronizing or reading records never deletes them from the ESP32.

## `GET /api/history/info`

Returns the storage state and available time range. The partition is mounted with `SPIFFS.begin(false)`; a mount failure does not format the filesystem. `oldestId`, `newestId`, `oldestTimestamp`, and `newestTimestamp` are `null` when no retained records are available.

Example:

```json
{
  "schemaVersion": 1,
  "available": true,
  "storage": "SPIFFS",
  "recordSizeBytes": 28,
  "sampleIntervalSeconds": 300,
  "retentionSeconds": 604800,
  "capacity": 2016,
  "count": 3,
  "oldestId": 101,
  "newestId": 103,
  "oldestTimestamp": 1795500000,
  "newestTimestamp": 1795500600,
  "storageTotalBytes": 1572864,
  "storageUsedBytes": 57344,
  "storageFreeBytes": 1515520,
  "retentionCutoffTimestamp": 1794895800
}
```

The storage byte values are examples; the endpoint reports measured SPIFFS values from the device.

## `GET /api/history`

Supported query parameters:

| Parameter | Meaning |
|---|---|
| `from` | Inclusive UTC Unix timestamp lower bound. |
| `to` | Inclusive UTC Unix timestamp upper bound. |
| `after` | Exclusive sequence ID cursor. Preferred for pagination and incremental sync. |
| `afterTimestamp` | Exclusive UTC Unix timestamp cursor; cannot be combined with `after`. |
| `limit` | Page size from 1 to 100; default is 100. |

`from`/`to` can be combined with either cursor. Invalid ranges/parameters return HTTP 400. If SPIFFS is unavailable, the endpoint returns HTTP 503 with `history_unavailable`.

Example request:

```text
GET /api/history?from=1795500000&to=1795500600&limit=2
```

Example response:

```json
{
  "schemaVersion": 1,
  "records": [
    {
      "id": 101,
      "timestamp": 1795500000,
      "validMask": 63,
      "temperatureC": 22.34,
      "humidityPercent": 48.12,
      "pressureHpa": 1013.2,
      "illuminanceLux": 326,
      "gasResistanceKOhm": 87.4,
      "iaq": 42
    },
    {
      "id": 102,
      "timestamp": 1795500300,
      "validMask": 15,
      "temperatureC": 22.41,
      "humidityPercent": 48.05,
      "pressureHpa": 1013.1,
      "illuminanceLux": 328,
      "gasResistanceKOhm": null,
      "iaq": null
    }
  ],
  "nextAfter": 102,
  "nextAfterTimestamp": 1795500300,
  "hasMore": true
}
```

Each unavailable sensor value is JSON `null`. `validMask` uses bit 0 for temperature, bit 1 for humidity, bit 2 for pressure, bit 3 for illuminance, bit 4 for gas resistance, and bit 5 for IAQ. For the next page, send `after=<nextAfter>` and preserve any time bounds. An empty result returns an empty `records` array and `hasMore: false`.

The flash record is a fixed 28-byte structure: magic, 32-bit sequence ID, 32-bit UTC timestamp, scaled sensor values, validity mask, and CRC-16/CCITT. A record body is written and flushed before its CRC commit marker; invalid/torn records fail CRC validation and are skipped. Startup scans all ring slots, including staged records with a valid ID/timestamp, so a sequence ID from an interrupted write is not reused.

Capacity is 2,016 records (7 days at 5-minute intervals). The ring overwrites the oldest slot when full. API responses are limited to 100 records per request. Samples are skipped until NTP synchronization is confirmed; timestamp zero or unsynchronized local time is never recorded.
