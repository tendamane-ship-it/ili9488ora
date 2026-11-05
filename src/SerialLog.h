#ifndef SERIAL_LOG_H
#define SERIAL_LOG_H

#include <Arduino.h>

class SerialLog : public Print
{
public:
    static constexpr size_t LOG_CAPACITY = 32;
    static constexpr size_t LOG_LINE_LENGTH = 128;
    static constexpr size_t DISPLAY_LINES = 20;

    void begin(unsigned long baud);
    size_t write(uint8_t value) override;
    size_t write(const uint8_t* buffer, size_t size) override;
    String getRecentLogs();

private:
    char lines_[LOG_CAPACITY][LOG_LINE_LENGTH] = {};
    char pendingLine_[LOG_LINE_LENGTH] = {};
    size_t pendingLength_ = 0;
    size_t nextLine_ = 0;
    size_t lineCount_ = 0;
    portMUX_TYPE mux_ = portMUX_INITIALIZER_UNLOCKED;

    void capture(const uint8_t* buffer, size_t size);
    void finishLine();
};

extern SerialLog serialLog;

#define Serial serialLog

#endif
