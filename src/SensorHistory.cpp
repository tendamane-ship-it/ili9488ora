#include "SensorHistory.h"

#include "Sensors.h"

#include <FS.h>
#include <SPIFFS.h>
#include <math.h>
#include <limits>
#include <stddef.h>
#include <time.h>

namespace SensorHistory
{
namespace
{
constexpr char HISTORY_PATH[] = "/sensor_history_v1.bin";
constexpr uint16_t RECORD_MAGIC = 0x4F52;
constexpr uint16_t VALID_TEMPERATURE = 1U << 0;
constexpr uint16_t VALID_HUMIDITY = 1U << 1;
constexpr uint16_t VALID_PRESSURE = 1U << 2;
constexpr uint16_t VALID_ILLUMINANCE = 1U << 3;
constexpr uint16_t VALID_GAS = 1U << 4;
constexpr uint16_t VALID_IAQ = 1U << 5;
constexpr uint16_t VALID_MASK =
    VALID_TEMPERATURE | VALID_HUMIDITY | VALID_PRESSURE |
    VALID_ILLUMINANCE | VALID_GAS | VALID_IAQ;
constexpr uint32_t SAMPLE_INTERVAL_MS = SAMPLE_INTERVAL_SECONDS * 1000UL;
constexpr uint32_t MAX_SENSOR_AGE_MS = 15000UL;
constexpr uint32_t HISTORY_FILE_SIZE = RECORD_CAPACITY * RECORD_SIZE_BYTES;
constexpr uint32_t MINIMUM_FREE_BYTES = 4096;
constexpr time_t MINIMUM_VALID_EPOCH = 1577836800; // 2020-01-01 UTC

static_assert(sizeof(Record) == RECORD_SIZE_BYTES, "Unexpected sensor history record size");

File historyFile;
bool storageReady = false;
bool utcTimeSynchronized = false;
uint32_t lastSampleAttemptMs = 0;
uint32_t storedRecordCount = 0;
uint32_t highestObservedSequence = 0;

uint16_t calculateCrc(const Record& record)
{
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&record);
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < offsetof(Record, crc); ++i)
    {
        crc ^= static_cast<uint16_t>(bytes[i]) << 8;
        for (uint8_t bit = 0; bit < 8; ++bit)
            crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : crc << 1;
    }
    return crc;
}

bool isValidRecord(const Record& record)
{
    return record.magic == RECORD_MAGIC &&
        record.id != 0 &&
        record.timestamp >= static_cast<uint32_t>(MINIMUM_VALID_EPOCH) &&
        (record.validMask & ~VALID_MASK) == 0 &&
        record.crc == calculateCrc(record);
}

uint32_t recordOffset(uint32_t id)
{
    return ((id - 1) % RECORD_CAPACITY) * RECORD_SIZE_BYTES;
}

bool readSlot(uint32_t slot, Record& record)
{
    if (!historyFile || slot >= RECORD_CAPACITY)
        return false;

    const uint32_t offset = slot * RECORD_SIZE_BYTES;
    if (!historyFile.seek(offset, SeekSet))
        return false;

    if (historyFile.read(reinterpret_cast<uint8_t*>(&record), sizeof(record)) != sizeof(record))
        return false;

    return isValidRecord(record) &&
        ((record.id - 1) % RECORD_CAPACITY) == slot;
}

bool readRawSlot(uint32_t slot, Record& record)
{
    if (!historyFile || slot >= RECORD_CAPACITY ||
        !historyFile.seek(slot * RECORD_SIZE_BYTES, SeekSet))
        return false;
    return historyFile.read(reinterpret_cast<uint8_t*>(&record), sizeof(record)) == sizeof(record);
}

bool readRecordAtId(uint32_t id, Record& record)
{
    if (!historyFile || id == 0 || !historyFile.seek(recordOffset(id), SeekSet))
        return false;

    if (historyFile.read(reinterpret_cast<uint8_t*>(&record), sizeof(record)) != sizeof(record))
        return false;

    return isValidRecord(record) && record.id == id;
}

bool writeZeroes(File& file, uint32_t start, uint32_t length)
{
    uint8_t zeroes[256] = {};
    if (!file.seek(start, SeekSet))
        return false;

    while (length != 0)
    {
        const size_t amount = length < sizeof(zeroes) ? length : sizeof(zeroes);
        if (file.write(zeroes, amount) != amount)
            return false;
        length -= amount;
    }
    file.flush();
    return true;
}

bool prepareHistoryFile()
{
    const uint32_t used = SPIFFS.usedBytes();
    const uint32_t total = SPIFFS.totalBytes();
    const uint32_t free = total > used ? total - used : 0;

    if (!SPIFFS.exists(HISTORY_PATH))
    {
        if (free < HISTORY_FILE_SIZE + MINIMUM_FREE_BYTES)
        {
            Serial.printf(
                "History: insufficient SPIFFS space (%lu free, %lu required)\n",
                static_cast<unsigned long>(free),
                static_cast<unsigned long>(HISTORY_FILE_SIZE + MINIMUM_FREE_BYTES)
            );
            return false;
        }

        File createFile = SPIFFS.open(HISTORY_PATH, FILE_WRITE);
        if (!createFile || !writeZeroes(createFile, 0, HISTORY_FILE_SIZE))
        {
            Serial.println("History: failed to initialize SPIFFS ring file");
            createFile.close();
            return false;
        }
        createFile.close();
    }
    else
    {
        File existingFile = SPIFFS.open(HISTORY_PATH, "r+");
        if (!existingFile)
        {
            Serial.println("History: failed to open SPIFFS ring file");
            return false;
        }

        const uint32_t existingSize = existingFile.size();
        if (existingSize > HISTORY_FILE_SIZE)
        {
            Serial.println("History: stored file has an unsupported size; leaving it untouched");
            existingFile.close();
            return false;
        }
        if (existingSize < HISTORY_FILE_SIZE)
        {
            if (free < HISTORY_FILE_SIZE - existingSize + MINIMUM_FREE_BYTES ||
                !writeZeroes(existingFile, existingSize, HISTORY_FILE_SIZE - existingSize))
            {
                Serial.println("History: failed to recover/extend incomplete ring file");
                existingFile.close();
                return false;
            }
        }
        existingFile.close();
    }

    historyFile = SPIFFS.open(HISTORY_PATH, "r+");
    if (!historyFile || historyFile.size() != HISTORY_FILE_SIZE)
    {
        Serial.println("History: SPIFFS ring file is unavailable or has an invalid size");
        historyFile.close();
        return false;
    }
    return true;
}

void rebuildIndex()
{
    storedRecordCount = 0;
    highestObservedSequence = 0;

    for (uint32_t slot = 0; slot < RECORD_CAPACITY; ++slot)
    {
        Record record{};
        if (!readRawSlot(slot, record))
            continue;

        const bool plausibleSequence =
            record.id != 0 &&
            record.timestamp >= static_cast<uint32_t>(MINIMUM_VALID_EPOCH) &&
            ((record.id - 1) % RECORD_CAPACITY) == slot;
        if (plausibleSequence && record.id > highestObservedSequence)
            highestObservedSequence = record.id;

        if (!isValidRecord(record))
            continue;

        ++storedRecordCount;
    }
}

template <typename T>
T scaledValue(float value, float scale)
{
    const float scaled = value * scale;
    if (!isfinite(scaled))
        return 0;
    const float minimum = static_cast<float>(std::numeric_limits<T>::lowest());
    const float maximum = static_cast<float>(std::numeric_limits<T>::max());
    if (scaled < minimum)
        return std::numeric_limits<T>::lowest();
    if (scaled > maximum)
        return std::numeric_limits<T>::max();
    return static_cast<T>(lroundf(scaled));
}

void appendSample(uint32_t timestamp)
{
    if (highestObservedSequence == UINT32_MAX)
    {
        Serial.println("History: sequence ID space exhausted; refusing duplicate IDs");
        return;
    }

    Record record{};
    record.magic = RECORD_MAGIC;
    record.id = highestObservedSequence + 1;
    record.timestamp = timestamp;
    const uint32_t nowMs = millis();
    const bool bmeFresh = bmeReadingOK && lastBmeReading != 0 &&
        nowMs - lastBmeReading <= MAX_SENSOR_AGE_MS;
    const bool lightFresh = bh1750ReadingOK && lastLuxReading != 0 &&
        nowMs - lastLuxReading <= MAX_SENSOR_AGE_MS;

    if (bmeFresh)
    {
        record.temperatureCentiC = scaledValue<int16_t>(bmeTemp, 100.0f);
        record.humidityCentiPercent = scaledValue<uint16_t>(bmeHumidity, 100.0f);
        // BME680 raw pressure is in Pa; store 0.1 hPa units.
        record.pressureDeciHpa = scaledValue<uint16_t>(bmePressure, 0.1f);
        record.gasDeciKOhm = scaledValue<uint32_t>(bmeGas, 10.0f);
        record.validMask |= VALID_TEMPERATURE | VALID_HUMIDITY |
            VALID_PRESSURE | VALID_GAS;
    }
    if (lightFresh && bh1750Lux >= 0.0f)
    {
        record.illuminanceLux = scaledValue<uint16_t>(bh1750Lux, 1.0f);
        record.validMask |= VALID_ILLUMINANCE;
    }
    if (bmeFresh && bmeIaqReady && bmeIaqAccuracy > 0)
    {
        record.iaq = scaledValue<uint16_t>(bmeIaq, 1.0f);
        record.validMask |= VALID_IAQ;
    }

    const uint32_t offset = recordOffset(record.id);
    Record staged = record;
    staged.crc = 0;
    highestObservedSequence = record.id;
    if (!historyFile.seek(offset, SeekSet) ||
        historyFile.write(reinterpret_cast<const uint8_t*>(&staged), sizeof(staged)) != sizeof(staged))
    {
        Serial.println("History: SPIFFS staging write failed");
        return;
    }
    historyFile.flush();

    record.crc = calculateCrc(record);
    if (!historyFile.seek(offset + offsetof(Record, crc), SeekSet) ||
        historyFile.write(reinterpret_cast<const uint8_t*>(&record.crc), sizeof(record.crc)) != sizeof(record.crc))
    {
        Serial.println("History: SPIFFS CRC commit marker write failed");
        return;
    }
    historyFile.flush();

    Record verified{};
    if (!readRecordAtId(record.id, verified))
    {
        Serial.println("History: record verification failed; invalid slot will be ignored");
        return;
    }

    rebuildIndex();
}
}

bool begin()
{
    lastSampleAttemptMs = millis();
    if (!SPIFFS.begin(false))
    {
        Serial.println("History: SPIFFS mount failed; filesystem was not formatted");
        return false;
    }

    const uint32_t totalBytes = SPIFFS.totalBytes();
    const uint32_t usedBytes = SPIFFS.usedBytes();
    const uint32_t freeBytes = totalBytes > usedBytes ? totalBytes - usedBytes : 0;
    Serial.printf(
        "History: SPIFFS total=%lu used=%lu free=%lu bytes\n",
        static_cast<unsigned long>(totalBytes),
        static_cast<unsigned long>(usedBytes),
        static_cast<unsigned long>(freeBytes)
    );

    if (!prepareHistoryFile())
        return false;

    storageReady = true;
    rebuildIndex();
    Serial.printf(
        "History: ready, %lu/%lu records recovered\n",
        static_cast<unsigned long>(storedRecordCount),
        static_cast<unsigned long>(RECORD_CAPACITY)
    );
    return true;
}

void update(bool utcSynchronized)
{
    utcTimeSynchronized = utcSynchronized;
    if (!storageReady)
        return;

    const uint32_t nowMs = millis();
    if (nowMs - lastSampleAttemptMs < SAMPLE_INTERVAL_MS)
        return;
    lastSampleAttemptMs = nowMs;

    const time_t now = time(nullptr);
    if (!utcSynchronized || now < MINIMUM_VALID_EPOCH ||
        static_cast<uint64_t>(now) > UINT32_MAX)
    {
        Serial.println("History: sample skipped until UTC time is synchronized");
        return;
    }

    appendSample(static_cast<uint32_t>(now));
}

void getInfo(Info& info)
{
    info.available = storageReady;
    info.count = 0;
    info.oldestId = 0;
    info.newestId = 0;
    info.oldestTimestamp = 0;
    info.newestTimestamp = 0;
    info.retentionCutoffTimestamp = 0;

    const time_t now = time(nullptr);
    const uint32_t cutoff = utcTimeSynchronized &&
        now >= MINIMUM_VALID_EPOCH + RETENTION_SECONDS
        ? static_cast<uint32_t>(now - RETENTION_SECONDS)
        : 0;
    info.retentionCutoffTimestamp = cutoff;
    if (storageReady)
    {
        for (uint32_t slot = 0; slot < RECORD_CAPACITY; ++slot)
        {
            Record record{};
            if (!readSlot(slot, record) ||
                (cutoff != 0 && record.timestamp < cutoff))
                continue;

            ++info.count;
            if (info.oldestId == 0 || record.id < info.oldestId)
            {
                info.oldestId = record.id;
                info.oldestTimestamp = record.timestamp;
            }
            if (record.id > info.newestId)
            {
                info.newestId = record.id;
                info.newestTimestamp = record.timestamp;
            }
        }
    }
    info.storageTotalBytes = storageReady ? SPIFFS.totalBytes() : 0;
    info.storageUsedBytes = storageReady ? SPIFFS.usedBytes() : 0;
    info.storageFreeBytes = info.storageTotalBytes > info.storageUsedBytes
        ? info.storageTotalBytes - info.storageUsedBytes
        : 0;
}

bool readRecord(uint32_t id, Record& record)
{
    return storageReady && readRecordAtId(id, record);
}
}
