#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <TFT_eSPI.h>
#include <esp_ota_ops.h>
#include <mbedtls/sha256.h>

class OtaManager
{
public:
    OtaManager();

    void begin(WebServer& server);
    bool startPairing(TFT_eSPI& display);
    void handleLoop();

private:
    WebServer* server;
    String pairingKey;
    String challenge;
    bool paired;
    bool pairingActive;
    unsigned long pairingExpiresAt;
    unsigned long challengeExpiresAt;
    bool updateActive;
    bool rebootPending;
    const esp_partition_t* updatePartition;
    esp_ota_handle_t updateHandle;
    uint32_t updateSize;
    uint32_t updateOffset;
    uint32_t updateMaximum;
    unsigned long lastChunkAt;
    unsigned long rebootAt;
    String targetVersion;
    String expectedSha256;
    String updateState;
    String updateError;
    mbedtls_sha256_context imageSha256;
    mbedtls_sha256_context chunkSha256;
    String chunkSha256Expected;
    uint32_t chunkOffset;
    uint32_t chunkBytes;
    bool chunkRequestValid;

    void handleChallenge();
    void handlePairStatus();
    void handlePairClaim();
    void handleOtaCheck();
    void handleOtaStart();
    void handleOtaFinish();
    void handleOtaAbort();
    void handleOtaStatus();
    void abortUpdate(const String& reason);
    void saveUpdateStatus();
    bool verifyRequest(const char* method, const String& path, const String& body,
                       const String& bodyHashOverride = "",
                       const String& signedContext = "");
    void handleOtaUpload();
    void handleOtaUploadData();
    void sendJson(int status, const String& body);
};
