#include "OtaManager.h"

#include <Preferences.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <esp_system.h>
#include <esp_ota_ops.h>
#include <mbedtls/md.h>
#include <mbedtls/sha256.h>
#include <qrcode.h>

#include "FirmwareVersion.h"

namespace
{
constexpr char OTA_NAMESPACE[] = "ora_ota";
constexpr unsigned long PAIRING_WINDOW_MS = 120000UL;
constexpr unsigned long CHALLENGE_WINDOW_MS = 30000UL;
TFT_eSPI* qrDisplay = nullptr;

String toHex(const uint8_t* bytes, size_t length)
{
    static const char digits[] = "0123456789abcdef";
    String result;
    result.reserve(length * 2);
    for (size_t i = 0; i < length; ++i)
    {
        result += digits[bytes[i] >> 4];
        result += digits[bytes[i] & 0x0f];
    }
    return result;
}

void drawPairingQr(esp_qrcode_handle_t code)
{
    if (qrDisplay == nullptr)
        return;

    const int size = esp_qrcode_get_size(code);
    if (size <= 0)
        return;

    const int module = min(6, min((qrDisplay->width() - 24) / size,
                                  (qrDisplay->height() - 60) / size));
    if (module < 1)
        return;

    const int side = size * module;
    const int left = (qrDisplay->width() - side) / 2;
    const int top = 42;
    qrDisplay->fillScreen(TFT_WHITE);
    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            if (esp_qrcode_get_module(code, x, y))
                qrDisplay->fillRect(left + x * module, top + y * module,
                                    module, module, TFT_BLACK);
        }
    }

    qrDisplay->setTextDatum(MC_DATUM);
    qrDisplay->setTextColor(TFT_BLACK, TFT_WHITE);
    qrDisplay->setTextFont(2);
    qrDisplay->drawString("Skanoni per çiftim", qrDisplay->width() / 2,
                          qrDisplay->height() - 10);
    qrDisplay->setTextDatum(TL_DATUM);
}

void drawPairingNotice(const char* text)
{
    if (qrDisplay == nullptr)
        return;
    qrDisplay->fillScreen(TFT_WHITE);
    qrDisplay->setTextDatum(MC_DATUM);
    qrDisplay->setTextColor(TFT_BLACK, TFT_WHITE);
    qrDisplay->setTextFont(4);
    qrDisplay->drawString(text, qrDisplay->width() / 2,
                          qrDisplay->height() / 2);
    qrDisplay->setTextDatum(TL_DATUM);
}

bool constantTimeEqual(const String& left, const String& right)
{
    if (left.length() != right.length())
        return false;
    uint8_t difference = 0;
    for (size_t i = 0; i < left.length(); ++i)
        difference |= static_cast<uint8_t>(left[i] ^ right[i]);
    return difference == 0;
}

bool isSha256Hex(const String& value)
{
    if (value.length() != 64)
        return false;
    for (size_t i = 0; i < value.length(); ++i)
    {
        const char c = value[i];
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
              (c >= 'A' && c <= 'F')))
            return false;
    }
    return true;
}

bool parseVersion(const String& value, uint32_t parts[3])
{
    size_t start = 0;
    for (size_t part = 0; part < 3; ++part)
    {
        const size_t end = part == 2 ? value.length() : value.indexOf('.', start);
        if (end == static_cast<size_t>(-1) || end <= start || end - start > 9)
            return false;
        uint64_t number = 0;
        for (size_t i = start; i < end; ++i)
        {
            if (value[i] < '0' || value[i] > '9')
                return false;
            number = number * 10 + static_cast<uint8_t>(value[i] - '0');
            if (number > UINT32_MAX)
                return false;
        }
        parts[part] = static_cast<uint32_t>(number);
        start = end + 1;
    }
    return start == value.length() + 1;
}

bool isNewerVersion(const String& candidate)
{
    uint32_t next[3];
    uint32_t current[3];
    if (!parseVersion(candidate, next) ||
        !parseVersion(ORA_FIRMWARE_VERSION, current))
        return false;
    for (size_t i = 0; i < 3; ++i)
    {
        if (next[i] != current[i])
            return next[i] > current[i];
    }
    return false;
}
}

OtaManager::OtaManager()
    : server(nullptr), paired(false), pairingActive(false),
      pairingExpiresAt(0), challengeExpiresAt(0), updateActive(false),
      rebootPending(false), updatePartition(nullptr), updateHandle(0),
      updateSize(0), updateOffset(0), updateMaximum(0),
      lastChunkAt(0), rebootAt(0), chunkOffset(0), chunkBytes(0),
      chunkRequestValid(false)
{
    mbedtls_sha256_init(&imageSha256);
    mbedtls_sha256_init(&chunkSha256);
}

void OtaManager::begin(WebServer& webServer)
{
    server = &webServer;
    Preferences prefs;
    prefs.begin(OTA_NAMESPACE, true);
    pairingKey = prefs.getString("key", "");
    paired = prefs.getBool("paired", false) && pairingKey.length() == 64;
    updateState = prefs.getString("result_state", "idle");
    targetVersion = prefs.getString("result_version", "");
    updateError = prefs.getString("result_error", "");
    updateSize = prefs.getUInt("result_size", 0);
    updateOffset = prefs.getUInt("result_rx", 0);
    prefs.end();
    const esp_partition_t* nextPartition = esp_ota_get_next_update_partition(nullptr);
    updateMaximum = nextPartition == nullptr ? 0 : nextPartition->size;
    if (updateState == "receiving")
    {
        updateState = "failed";
        updateError = "device_restarted_during_update";
        saveUpdateStatus();
    }

    server->on("/api/ota/challenge", HTTP_GET, [this]() { handleChallenge(); });
    server->on("/api/ota/pair/status", HTTP_GET, [this]() { handlePairStatus(); });
    server->on("/api/ota/pair/claim", HTTP_POST, [this]() { handlePairClaim(); });
    server->on("/api/ota/check", HTTP_POST, [this]() { handleOtaCheck(); });
    server->on("/api/ota/start", HTTP_POST, [this]() { handleOtaStart(); });
    server->on("/api/ota/chunk", HTTP_POST,
               [this]() { handleOtaUpload(); },
               [this]() { handleOtaUploadData(); });
    server->on("/api/ota/finish", HTTP_POST, [this]() { handleOtaFinish(); });
    server->on("/api/ota/abort", HTTP_POST, [this]() { handleOtaAbort(); });
    server->on("/api/ota/status", HTTP_GET, [this]() { handleOtaStatus(); });
}

bool OtaManager::startPairing(TFT_eSPI& display)
{
    if (WiFi.status() != WL_CONNECTED || updateActive)
        return false;

    uint8_t randomKey[32];
    for (size_t i = 0; i < sizeof(randomKey); i += sizeof(uint32_t))
    {
        const uint32_t randomWord = esp_random();
        memcpy(randomKey + i, &randomWord, sizeof(randomWord));
    }
    const String newKey = toHex(randomKey, sizeof(randomKey));
    memset(randomKey, 0, sizeof(randomKey));

    qrDisplay = &display;

    String payload = "ora://pair?v=1&model=" ORA_DEVICE_MODEL;
    payload += "&ip=" + WiFi.localIP().toString();
    payload += "&key=" + newKey;
    esp_qrcode_config_t config = ESP_QRCODE_CONFIG_DEFAULT();
    config.display_func = drawPairingQr;
    config.max_qrcode_version = 10;
    if (esp_qrcode_generate(&config, payload.c_str()) != ESP_OK)
    {
        qrDisplay = nullptr;
        return false;
    }

    pairingKey = newKey;
    Preferences prefs;
    prefs.begin(OTA_NAMESPACE, false);
    prefs.putString("key", pairingKey);
    prefs.putBool("paired", false);
    prefs.end();
    paired = false;
    pairingActive = true;
    pairingExpiresAt = millis() + PAIRING_WINDOW_MS;
    challenge = "";
    return true;
}

void OtaManager::handleChallenge()
{
    uint8_t randomBytes[16];
    for (size_t i = 0; i < sizeof(randomBytes); i += sizeof(uint32_t))
    {
        const uint32_t randomWord = esp_random();
        memcpy(randomBytes + i, &randomWord, sizeof(randomWord));
    }
    challenge = toHex(randomBytes, sizeof(randomBytes));
    memset(randomBytes, 0, sizeof(randomBytes));
    challengeExpiresAt = millis() + CHALLENGE_WINDOW_MS;

    String response = "{\"ok\":true,\"nonce\":\"" + challenge +
        "\",\"expiresInMs\":" + String(CHALLENGE_WINDOW_MS) + "}";
    sendJson(200, response);
}

void OtaManager::handlePairStatus()
{
    const bool pairingOpen = pairingActive &&
        static_cast<int32_t>(pairingExpiresAt - millis()) > 0;
    sendJson(200, String("{\"ok\":true,\"paired\":") +
        (paired ? "true" : "false") + ",\"pairingOpen\":" +
        (pairingOpen ? "true" : "false") + "}");
}

void OtaManager::handlePairClaim()
{
    const String body = server->arg("plain");
    if (!verifyRequest("POST", "/api/ota/pair/claim", body))
    {
        sendJson(401, "{\"ok\":false,\"error\":\"unauthorized\"}");
        return;
    }

    if (!pairingActive || static_cast<int32_t>(pairingExpiresAt - millis()) <= 0)
    {
        pairingActive = false;
        sendJson(410, "{\"ok\":false,\"error\":\"pairing_expired\"}");
        return;
    }

    pairingActive = false;
    paired = true;
    Preferences prefs;
    prefs.begin(OTA_NAMESPACE, false);
    prefs.putBool("paired", true);
    prefs.end();
    drawPairingNotice("APP PAIRED");
    qrDisplay = nullptr;
    sendJson(200, "{\"ok\":true,\"paired\":true}");
}

bool OtaManager::verifyRequest(const char* method, const String& path,
                               const String& body,
                               const String& bodyHashOverride,
                               const String& signedContext)
{
    if (pairingKey.length() != 64)
        return false;

    const String suppliedNonce = server->header("X-ORA-Nonce");
    const String suppliedAuth = server->header("X-ORA-Auth");
    if (challenge.isEmpty() || !constantTimeEqual(suppliedNonce, challenge) ||
        static_cast<int32_t>(challengeExpiresAt - millis()) <= 0)
        return false;

    uint8_t bodyHash[32];
    String bodyHashHex = bodyHashOverride;
    if (bodyHashHex.isEmpty())
    {
        mbedtls_sha256(reinterpret_cast<const uint8_t*>(body.c_str()),
                       body.length(), bodyHash, 0);
        bodyHashHex = toHex(bodyHash, sizeof(bodyHash));
    }
    else if (!isSha256Hex(bodyHashHex))
    {
        return false;
    }

    const String canonical = String(method) + "\n" + path + "\n" +
        suppliedNonce + "\n" + bodyHashHex + "\n" + signedContext;
    uint8_t expected[32];
    const mbedtls_md_info_t* md = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (md == nullptr || mbedtls_md_hmac(md,
            reinterpret_cast<const uint8_t*>(pairingKey.c_str()), pairingKey.length(),
            reinterpret_cast<const uint8_t*>(canonical.c_str()), canonical.length(),
            expected) != 0)
        return false;

    const bool valid = constantTimeEqual(suppliedAuth, toHex(expected, sizeof(expected)));
    memset(bodyHash, 0, sizeof(bodyHash));
    memset(expected, 0, sizeof(expected));
    if (valid)
    {
        challenge = "";
        challengeExpiresAt = 0;
    }
    return valid;
}

void OtaManager::handleOtaCheck()
{
    JsonDocument request;
    if (deserializeJson(request, server->arg("plain")))
    {
        sendJson(400, "{\"ok\":false,\"error\":\"invalid_json\"}");
        return;
    }

    const String model = request["model"] | "";
    const String version = request["version"] | "";
    const String sha256 = request["sha256"] | "";
    const uint32_t size = request["size"] | 0U;
    const esp_partition_t* next = esp_ota_get_next_update_partition(nullptr);
    const uint32_t capacity = next == nullptr ? 0 : next->size;
    String reason;
    if (model != ORA_DEVICE_MODEL)
        reason = "incompatible_device";
    else if (!isNewerVersion(version))
        reason = "version_not_newer_or_invalid";
    else if (!isSha256Hex(sha256))
        reason = "invalid_sha256";
    else if (size < 1024 || size > capacity)
        reason = "firmware_size_out_of_range";
    else if (updateActive)
        reason = "update_in_progress";

    String response = String("{\"ok\":true,\"available\":") +
        (reason.isEmpty() ? "true" : "false") +
        ",\"deviceModel\":\"" ORA_DEVICE_MODEL "\",\"currentVersion\":\"" +
        ORA_FIRMWARE_VERSION + "\",\"targetVersion\":\"" + version +
        "\",\"maximumSize\":" + String(capacity) +
        ",\"reason\":\"" + reason + "\"}";
    sendJson(200, response);
}

void OtaManager::handleOtaStart()
{
    const String body = server->arg("plain");
    if (!paired || !verifyRequest("POST", "/api/ota/start", body))
    {
        sendJson(401, "{\"ok\":false,\"error\":\"unauthorized\"}");
        return;
    }
    if (updateActive)
    {
        sendJson(409, "{\"ok\":false,\"error\":\"update_in_progress\"}");
        return;
    }

    JsonDocument request;
    if (deserializeJson(request, body))
    {
        sendJson(400, "{\"ok\":false,\"error\":\"invalid_json\"}");
        return;
    }
    const String model = request["model"] | "";
    targetVersion = request["version"] | "";
    expectedSha256 = request["sha256"] | "";
    updateSize = request["size"] | 0U;
    const esp_partition_t* next = esp_ota_get_next_update_partition(nullptr);
    updateMaximum = next == nullptr ? 0 : next->size;

    if (model != ORA_DEVICE_MODEL || !isNewerVersion(targetVersion))
    {
        updateState = "rejected";
        updateError = "incompatible_device_or_version";
        saveUpdateStatus();
        sendJson(422, "{\"ok\":false,\"error\":\"incompatible_device_or_version\"}");
        return;
    }
    if (!isSha256Hex(expectedSha256) || updateSize < 1024 ||
        updateSize > updateMaximum)
    {
        updateState = "rejected";
        updateError = "invalid_size_or_sha256";
        saveUpdateStatus();
        sendJson(422, "{\"ok\":false,\"error\":\"invalid_size_or_sha256\"}");
        return;
    }
    updatePartition = next;
    const esp_err_t beginResult = esp_ota_begin(
        updatePartition,
        OTA_WITH_SEQUENTIAL_WRITES,
        &updateHandle
    );
    if (beginResult != ESP_OK)
    {
        updateState = "failed";
        updateError = esp_err_to_name(beginResult);
        saveUpdateStatus();
        sendJson(507, String("{\"ok\":false,\"error\":\"") + updateError + "\"}");
        return;
    }
    updateActive = true;

    mbedtls_sha256_free(&imageSha256);
    mbedtls_sha256_init(&imageSha256);
    if (mbedtls_sha256_starts_ret(&imageSha256, 0) != 0)
    {
        abortUpdate("sha256_init_failed");
        sendJson(500, "{\"ok\":false,\"error\":\"sha256_init_failed\"}");
        return;
    }

    updateOffset = 0;
    lastChunkAt = millis();
    updateState = "receiving";
    updateError = "";
    saveUpdateStatus();
    sendJson(200, String("{\"ok\":true,\"state\":\"receiving\",\"size\":") +
        String(updateSize) + ",\"maximumSize\":" + String(updateMaximum) + "}");
}

void OtaManager::handleOtaUploadData()
{
    HTTPUpload& upload = server->upload();
    if (upload.status == UPLOAD_FILE_START)
    {
        chunkRequestValid = false;
        chunkBytes = 0;
        chunkOffset = server->header("X-ORA-Offset").toInt();
        chunkSha256Expected = server->header("X-ORA-Content-SHA256");
        if (!updateActive || !paired || chunkOffset != updateOffset ||
            !isSha256Hex(chunkSha256Expected) ||
            !verifyRequest("POST", "/api/ota/chunk", "",
                           chunkSha256Expected, String(chunkOffset)))
            return;

        mbedtls_sha256_free(&chunkSha256);
        mbedtls_sha256_init(&chunkSha256);
        if (mbedtls_sha256_starts_ret(&chunkSha256, 0) != 0)
            return;
        chunkRequestValid = true;
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        if (!chunkRequestValid)
            return;
        if (upload.currentSize == 0 || upload.currentSize > 4096 ||
            chunkBytes + upload.currentSize > 4096 ||
            chunkOffset + chunkBytes + upload.currentSize > updateSize)
        {
            abortUpdate("chunk_size_out_of_range");
            chunkRequestValid = false;
            return;
        }
        if (esp_ota_write(updateHandle, upload.buf, upload.currentSize) != ESP_OK ||
            mbedtls_sha256_update_ret(&chunkSha256, upload.buf,
                                      upload.currentSize) != 0 ||
            mbedtls_sha256_update_ret(&imageSha256, upload.buf,
                                      upload.currentSize) != 0)
        {
            abortUpdate("flash_write_failed");
            chunkRequestValid = false;
            return;
        }
        chunkBytes += upload.currentSize;
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (!chunkRequestValid)
            return;
        uint8_t digest[32];
        if (chunkBytes == 0 ||
            mbedtls_sha256_finish_ret(&chunkSha256, digest) != 0 ||
            !constantTimeEqual(toHex(digest, sizeof(digest)),
                               chunkSha256Expected))
        {
            abortUpdate("chunk_sha256_mismatch");
            chunkRequestValid = false;
            return;
        }
        updateOffset += chunkBytes;
        lastChunkAt = millis();
        chunkRequestValid = true;
    }
    else if (upload.status == UPLOAD_FILE_ABORTED)
    {
        abortUpdate("transfer_interrupted");
        chunkRequestValid = false;
    }
}

void OtaManager::handleOtaUpload()
{
    if (!chunkRequestValid || !updateActive)
    {
        const bool unauthorized = !paired || !updateActive;
        sendJson(unauthorized ? 401 : 422,
                 unauthorized
                    ? "{\"ok\":false,\"error\":\"unauthorized_or_no_update\"}"
                    : "{\"ok\":false,\"error\":\"invalid_chunk\"}");
        return;
    }
    sendJson(200, String("{\"ok\":true,\"received\":") +
        String(updateOffset) + ",\"size\":" + String(updateSize) + "}");
}

void OtaManager::handleOtaFinish()
{
    const String body = server->arg("plain");
    if (!paired || !verifyRequest("POST", "/api/ota/finish", body))
    {
        sendJson(401, "{\"ok\":false,\"error\":\"unauthorized\"}");
        return;
    }
    if (!updateActive || updateOffset != updateSize)
    {
        sendJson(409, "{\"ok\":false,\"error\":\"incomplete_image\"}");
        return;
    }

    uint8_t digest[32];
    if (mbedtls_sha256_finish_ret(&imageSha256, digest) != 0 ||
        !constantTimeEqual(toHex(digest, sizeof(digest)), expectedSha256))
    {
        abortUpdate("image_sha256_mismatch");
        sendJson(422, "{\"ok\":false,\"error\":\"image_sha256_mismatch\"}");
        return;
    }
    const esp_err_t endResult = esp_ota_end(updateHandle);
    updateHandle = 0;
    updateActive = false;
    if (endResult != ESP_OK)
    {
        updateState = "failed";
        updateError = esp_err_to_name(endResult);
        saveUpdateStatus();
        sendJson(422, String("{\"ok\":false,\"error\":\"") + updateError + "\"}");
        return;
    }

    const esp_err_t selectResult = esp_ota_set_boot_partition(updatePartition);
    if (selectResult != ESP_OK)
    {
        updateState = "failed";
        updateError = esp_err_to_name(selectResult);
        saveUpdateStatus();
        sendJson(500, String("{\"ok\":false,\"error\":\"") + updateError + "\"}");
        return;
    }

    updateState = "installed";
    updateError = "";
    saveUpdateStatus();
    rebootPending = true;
    rebootAt = millis() + 1500UL;
    sendJson(200, String("{\"ok\":true,\"state\":\"installed\",\"version\":\"") +
        targetVersion + "\",\"rebootInMs\":1500}");
}

void OtaManager::handleOtaAbort()
{
    const String body = server->arg("plain");
    if (!paired || !verifyRequest("POST", "/api/ota/abort", body))
    {
        sendJson(401, "{\"ok\":false,\"error\":\"unauthorized\"}");
        return;
    }
    if (updateActive)
        abortUpdate("cancelled_by_client");
    sendJson(200, "{\"ok\":true,\"state\":\"aborted\"}");
}

void OtaManager::handleOtaStatus()
{
    String response = String("{\"ok\":true,\"state\":\"") + updateState +
        "\",\"version\":\"" + targetVersion + "\",\"received\":" +
        String(updateOffset) + ",\"size\":" + String(updateSize) +
        ",\"maximumSize\":" + String(updateMaximum) +
        ",\"error\":\"" + updateError + "\",\"currentVersion\":\"" +
        ORA_FIRMWARE_VERSION + "\",\"paired\":" +
        (paired ? "true" : "false") + "}";
    sendJson(200, response);
}

void OtaManager::abortUpdate(const String& reason)
{
    if (updateActive)
    {
        esp_ota_abort(updateHandle);
        updateHandle = 0;
    }
    updateActive = false;
    updateState = "failed";
    updateError = reason;
    saveUpdateStatus();
}

void OtaManager::handleLoop()
{
    if (pairingActive &&
        static_cast<int32_t>(pairingExpiresAt - millis()) <= 0)
    {
        pairingActive = false;
        drawPairingNotice("PAIRING EXPIRED");
        qrDisplay = nullptr;
    }
    if (updateActive && millis() - lastChunkAt > 30000UL)
        abortUpdate("transfer_timeout");
    if (rebootPending && static_cast<int32_t>(millis() - rebootAt) >= 0)
    {
        rebootPending = false;
        ESP.restart();
    }
}

void OtaManager::saveUpdateStatus()
{
    Preferences prefs;
    prefs.begin(OTA_NAMESPACE, false);
    prefs.putString("result_state", updateState);
    prefs.putString("result_version", targetVersion);
    prefs.putString("result_error", updateError);
    prefs.putUInt("result_size", updateSize);
    prefs.putUInt("result_rx", updateOffset);
    prefs.end();
}

void OtaManager::sendJson(int status, const String& body)
{
    server->send(status, "application/json", body);
}
