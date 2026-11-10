#include "ResetDiagnostics.h"
#include "RuntimePolicies.h"

#include <Preferences.h>
#include <esp_system.h>

namespace
{
constexpr char RESET_NAMESPACE[] = "resetdiag";
constexpr char RESET_HISTORY_KEY[] = "history";
constexpr size_t RESET_HISTORY_CAPACITY = 8;

Preferences resetPreferences;
runtime_policies::ResetHistory<RESET_HISTORY_CAPACITY> resetHistory;
uint32_t totalResets = 0;
esp_reset_reason_t lastResetReason = ESP_RST_UNKNOWN;

const char* resetReasonName(esp_reset_reason_t reason)
{
    switch (reason)
    {
        case ESP_RST_POWERON: return "Ndezje (power-on)";
        case ESP_RST_EXT: return "Reset i jashtëm";
        case ESP_RST_SW: return "Rinisje nga programi";
        case ESP_RST_PANIC: return "Gabim fatal (panic)";
        case ESP_RST_INT_WDT: return "Watchdog i ndërprerjeve";
        case ESP_RST_TASK_WDT: return "Watchdog i detyrës";
        case ESP_RST_WDT: return "Watchdog";
        case ESP_RST_DEEPSLEEP: return "Zgjim nga deep sleep";
        case ESP_RST_BROWNOUT: return "Brownout (rënie tensioni)";
        case ESP_RST_SDIO: return "Reset nga SDIO";
        default: return "E panjohur";
    }
}
}

void recordResetDiagnostics()
{
    lastResetReason = esp_reset_reason();

    if (!resetPreferences.begin(RESET_NAMESPACE, false))
    {
        Serial.println("RESET DIAGNOSTICS: opening NVS failed");
        return;
    }

    totalResets = resetPreferences.getUInt("count", 0);
    const uint8_t storedNextIndex = resetPreferences.getUChar("next", 0);

    const size_t storedHistoryLength =
        resetPreferences.getBytesLength(RESET_HISTORY_KEY);
    uint8_t storedHistory[RESET_HISTORY_CAPACITY] = {};
    if (storedHistoryLength == sizeof(storedHistory))
    {
        if (resetPreferences.getBytes(
                RESET_HISTORY_KEY,
                storedHistory,
                sizeof(storedHistory)
            ) != sizeof(storedHistory))
        {
            Serial.println("RESET DIAGNOSTICS: reading history from NVS failed");
            resetHistory.clear();
        }
        else if (!resetHistory.restore(
                     storedHistory,
                     storedNextIndex,
                     totalResets < RESET_HISTORY_CAPACITY
                         ? totalResets
                         : RESET_HISTORY_CAPACITY
                 ))
        {
            Serial.println("RESET DIAGNOSTICS: stored history is invalid");
            resetHistory.clear();
        }
    }
    else
    {
        resetHistory.clear();
    }

    resetHistory.push(static_cast<uint8_t>(lastResetReason));
    ++totalResets;

    const bool saved =
        resetPreferences.putBytes(
            RESET_HISTORY_KEY,
            resetHistory.data(),
            RESET_HISTORY_CAPACITY
        ) == RESET_HISTORY_CAPACITY &&
        resetPreferences.putUChar(
            "next",
            static_cast<uint8_t>(resetHistory.nextIndex())
        ) == sizeof(uint8_t) &&
        resetPreferences.putUInt("count", totalResets) ==
            sizeof(totalResets);

    resetPreferences.end();

    Serial.printf(
        "RESET DIAGNOSTICS: %s; total starts recorded: %lu\n",
        resetReasonName(),
        static_cast<unsigned long>(totalResets)
    );
    if (!saved)
        Serial.println("RESET DIAGNOSTICS: saving history to NVS failed");
}

const char* resetReasonName()
{
    return resetReasonName(lastResetReason);
}

uint32_t resetCount()
{
    return totalResets;
}

size_t resetHistoryCount()
{
    return resetHistory.size();
}

const char* resetHistoryReason(size_t oldestFirstIndex)
{
    uint8_t reason;
    if (!resetHistory.atOldestFirst(oldestFirstIndex, reason))
        return "E panjohur";

    return resetReasonName(
        static_cast<esp_reset_reason_t>(reason)
    );
}
