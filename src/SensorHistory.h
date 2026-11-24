#pragma once

#include <Arduino.h>

namespace SensorHistory
{
constexpr uint32_t SAMPLE_INTERVAL_SECONDS = 300;
constexpr uint32_t RETENTION_SECONDS = 7UL * 24UL * 60UL * 60UL;
constexpr uint32_t RECORD_CAPACITY = RETENTION_SECONDS / SAMPLE_INTERVAL_SECONDS;
constexpr uint32_t RECORD_SIZE_BYTES = 28;

struct Record
{
    uint16_t magic;
    uint32_t id;
    uint32_t timestamp;
    int16_t temperatureCentiC;
    uint16_t humidityCentiPercent;
    uint16_t pressureDeciHpa;
    uint16_t illuminanceLux;
    uint32_t gasDeciKOhm;
    uint16_t iaq;
    uint16_t validMask;
    uint16_t crc;
} __attribute__((packed));

struct Info
{
    bool available;
    uint32_t count;
    uint32_t oldestId;
    uint32_t newestId;
    uint32_t oldestTimestamp;
    uint32_t newestTimestamp;
    uint32_t retentionCutoffTimestamp;
    uint32_t storageTotalBytes;
    uint32_t storageUsedBytes;
    uint32_t storageFreeBytes;
};

bool begin();
void update(bool utcSynchronized);
void getInfo(Info& info);
bool readRecord(uint32_t id, Record& record);
}
