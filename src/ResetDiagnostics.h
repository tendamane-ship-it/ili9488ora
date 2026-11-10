#ifndef RESET_DIAGNOSTICS_H
#define RESET_DIAGNOSTICS_H

#include <Arduino.h>

void recordResetDiagnostics();
const char* resetReasonName();
uint32_t resetCount();
size_t resetHistoryCount();
const char* resetHistoryReason(size_t oldestFirstIndex);

#endif
