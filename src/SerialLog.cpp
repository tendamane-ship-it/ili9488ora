#include "SerialLog.h"

#undef Serial

SerialLog serialLog;

void SerialLog::begin(unsigned long baud)
{
    ::Serial.begin(baud);
}

size_t SerialLog::write(uint8_t value)
{
    const size_t written = ::Serial.write(value);
    if (written != 0)
        capture(&value, 1);
    return written;
}

size_t SerialLog::write(const uint8_t* buffer, size_t size)
{
    const size_t written = ::Serial.write(buffer, size);
    if (written != 0)
        capture(buffer, written);
    return written;
}

void SerialLog::capture(const uint8_t* buffer, size_t size)
{
    portENTER_CRITICAL(&mux_);
    for (size_t i = 0; i < size; ++i)
    {
        const uint8_t value = buffer[i];
        if (value == '\r')
            continue;

        if (value == '\n')
        {
            finishLine();
            continue;
        }

        if (pendingLength_ == LOG_LINE_LENGTH - 1)
            finishLine();

        pendingLine_[pendingLength_++] = static_cast<char>(value);
        pendingLine_[pendingLength_] = '\0';
    }
    portEXIT_CRITICAL(&mux_);
}

void SerialLog::finishLine()
{
    pendingLine_[pendingLength_] = '\0';
    memcpy(lines_[nextLine_], pendingLine_, pendingLength_ + 1);
    nextLine_ = (nextLine_ + 1) % LOG_CAPACITY;
    if (lineCount_ < LOG_CAPACITY)
        ++lineCount_;

    pendingLength_ = 0;
    pendingLine_[0] = '\0';
}

String SerialLog::getRecentLogs()
{
    char snapshot[DISPLAY_LINES][LOG_LINE_LENGTH];
    size_t snapshotCount = 0;

    portENTER_CRITICAL(&mux_);
    const size_t completedCount =
        lineCount_ < DISPLAY_LINES ? lineCount_ : DISPLAY_LINES;
    const size_t start =
        (nextLine_ + LOG_CAPACITY - completedCount) % LOG_CAPACITY;

    for (size_t i = 0; i < completedCount; ++i)
    {
        memcpy(
            snapshot[snapshotCount++],
            lines_[(start + i) % LOG_CAPACITY],
            LOG_LINE_LENGTH
        );
    }

    if (pendingLength_ > 0)
    {
        if (snapshotCount == DISPLAY_LINES)
        {
            memmove(
                snapshot[0],
                snapshot[1],
                (DISPLAY_LINES - 1) * LOG_LINE_LENGTH
            );
            --snapshotCount;
        }
        memcpy(snapshot[snapshotCount++], pendingLine_, pendingLength_ + 1);
    }
    portEXIT_CRITICAL(&mux_);

    String result;
    result.reserve(snapshotCount * 48);
    for (size_t i = 0; i < snapshotCount; ++i)
    {
        result += snapshot[i];
        if (i + 1 < snapshotCount)
            result += '\n';
    }
    return result;
}
