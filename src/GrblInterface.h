#pragma once

#include "Arduino.h"

constexpr auto BUFFER_SIZE = 128;
constexpr auto EOL = '\r';
constexpr auto TIMEOUT_MS = 100;
constexpr auto OK_RESPONSE = "ok";
constexpr auto ERROR_RESPONSE = "error";
constexpr auto STATUS_REPORT_COMMAND = '?';
constexpr auto STATUS_REPORT_START_CHARACTER = '<';
constexpr auto STATUS_REPORT_MIN_INTERVAL_MS = 200; // Limits the status report query to 5Hz, as recommended by Grbl.
constexpr auto STATUS_REPORT_REGEX = "/<(\w+)\|MPos:([\d\.,-]+),([\d\.,-]+),([\d\.,-]+)(.+)>/gm";

struct Position {
    float x{};
    float y{};
    float z{};
};

class GrblInterface
{
public:
    GrblInterface();
    bool requestStatusReport(const Stream &stream);
    bool encode(char c);
    Position getPosition();

    void (*positionUpdated)();

private:
    char m_buffer[BUFFER_SIZE];
    uint8_t m_currentIndex;
    Position m_pos;

    bool processBuffer();
};
