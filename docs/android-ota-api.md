# ORA Android OTA API

The device API uses the existing HTTP server on port 80. Pairing must be started
from the device menu (`PAIR ANDROID APP`). The QR payload is
`ora://pair?v=1&model=...&ip=...&key=...`; `key` is a 64-character lowercase
hexadecimal string. The Android app must keep it in Android Keystore storage.
Starting pairing rotates the key and invalidates the previously paired app.

## Read-only endpoints

- `GET /api/device`: device/model, firmware version, Wi-Fi state, IP, RSSI, and
  uptime.
- `GET /api/status`: the existing status response, with `device`, `model`, and
  `firmwareVersion` added.
- `GET /api/ota/pair/status`: pairing state.
- `GET /api/ota/status`: update state and last result. The last result survives
  restart.
- `POST /api/ota/check`: JSON manifest `{ "model": "ILI9488-ESP32-S3",
  "version": "1.0.1", "size": 123456, "sha256": "<64 hex chars>" }`.
  Returns `available` and a rejection `reason`; it does not change device state.

## Pairing

1. Scan the QR while pairing is open (120 seconds).
2. Request `GET /api/ota/challenge` for a one-use, 30-second nonce.
3. Claim pairing with `POST /api/ota/pair/claim`, JSON body
   `{ "accept": true }`, plus HMAC headers described below.

The device stores the pairing key and pairing state in the separate Preferences
namespace `ora_ota`; it does not use the Wi-Fi or clock settings namespaces.

## Authenticated request format

For every authenticated request, first get a fresh challenge. Use the exact
request body bytes and lowercase hexadecimal SHA-256 digest. The HMAC key is the
64 ASCII characters from the QR payload (do not hex-decode it). The canonical
UTF-8 message is:

```text
METHOD\nPATH\nNONCE\nBODY_SHA256\nCONTEXT
```

Calculate HMAC-SHA256 over that message and send its lowercase hex digest in
`X-ORA-Auth`, with the challenge in `X-ORA-Nonce`. The nonce is consumed after a
valid request and cannot be reused.

`CONTEXT` is empty except for OTA block requests, where it is the decimal byte
offset. For a block, `BODY_SHA256` is the digest of the raw firmware block (the
file part), not the multipart boundaries. Also send `X-ORA-Offset` and
`X-ORA-Content-SHA256` with the same offset and digest. Send the binary block as
the `firmware` file in a `multipart/form-data` request. Blocks must be 1–4096
bytes, in order, with no gaps or overlaps.

## Update flow

1. `POST /api/ota/check` with the release manifest.
2. Get a fresh challenge; authenticated `POST /api/ota/start` with the same
   JSON manifest. The device checks that `model` matches this ORA model, that
   `version` is newer than the running ORA version, and that `size` fits the
   inactive OTA partition before beginning flash writes.
3. For each block, get a fresh challenge and authenticated `POST
   /api/ota/chunk` multipart upload.
4. Get a fresh challenge and authenticated `POST /api/ota/finish` with an empty
   body. The device checks the complete SHA-256, then calls ESP-IDF
   `esp_ota_end()` to validate the image before selecting the inactive OTA
   slot. It then reboots automatically.
5. Read `GET /api/ota/status` and `GET /api/device` after reconnecting to confirm
   the result and running firmware version.

An authenticated `POST /api/ota/abort` with an empty body cancels an active
transfer. An interrupted transfer times out after 30 seconds. `/api/ota/status`
reports `idle`, `receiving`, `installed`, `failed`, or `rejected`, plus byte
counts and an error code where applicable.

The HMAC authenticates requests but does not encrypt HTTP traffic. Pair and
update only over a trusted local Wi-Fi network; network observers can see the
QR-derived key only if they can view the physical display during pairing, but
they can observe update metadata and disrupt traffic.
