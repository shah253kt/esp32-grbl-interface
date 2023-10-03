#include "GrblInterface.h"

#include <Regexp.h>

constexpr auto EOL = '\r';
constexpr auto VALUE_SEPARATOR = ",";
constexpr auto OK_RESPONSE = "ok";
constexpr auto ERROR_RESPONSE = "error";

constexpr auto STATUS_REPORT_COMMAND = '?';
constexpr auto STATUS_REPORT_MIN_INTERVAL_MS = 200; // Limits the status report query to 5Hz, as recommended by Grbl.
constexpr auto STATUS_REPORT_REGEX = "<([%w:%d]+)%|(%w+):([-%d.,]+)[%|]?.*>";

constexpr auto GCODE_LINEAR_RAPID_MOVE = "G0";
constexpr auto GCODE_LINEAR_MOVE = "G1";

GrblInterface::GrblInterface(Stream &stream) : m_currentIndex(0)
{
    m_stream = &stream;
}

bool GrblInterface::update(uint16_t timeout)
{
    const auto timedOutAt = millis() + timeout;

    do
    {
        const auto sent = requestStatusReport();
        if (m_stream->available())
        {
            break;
        }
    } while (millis() < timedOutAt);

    if (millis() >= timedOutAt)
    {
        return false;
    }

    return encode(m_stream->read());
}

bool GrblInterface::encode(const char c)
{
    if (m_currentIndex >= Grbl::BUFFER_SIZE)
    {
        return false;
    }

    if (c == EOL)
    {
        return processBuffer();
    }

    m_buffer[m_currentIndex++] = c;
    m_buffer[m_currentIndex] = '\0';

    return true;
}

void GrblInterface::setPosition(const Grbl::Axis &axis, float value)
{
    m_targetPosition.setValue(axis, value);
}

void GrblInterface::sendLinearMove() const
{
    char gcode[Grbl::MAX_GCODE_LENGTH];
    strcpy(gcode, GCODE_LINEAR_RAPID_MOVE);

    for (const auto &axis : Grbl::AXES) {
        const auto positionPair = m_targetPosition.get(*axis);

        if (!positionPair->isSet) {
            continue;
        }

        char floatingPointString[Grbl::FLOATING_POINT_STRING_LENGTH + 1];
        dtostrf(positionPair->value, Grbl::FLOATING_POINT_FRACTIONAL_PART_LENGTH + 3, Grbl::FLOATING_POINT_FRACTIONAL_PART_LENGTH, floatingPointString);
        char axisString[strlen(floatingPointString) + strlen(axis->name())];
        sprintf(axisString, "%s%s", axis->name(), floatingPointString);
        strcat(gcode, axisString);
    }

    Serial.println(gcode);
    m_stream->print(gcode);
    m_stream->print(EOL);
}

void GrblInterface::sendLinearMove(float feedRate)
{
    // m_stream->print(gcode);
    m_stream->print(EOL);
}

Grbl::MachineState GrblInterface::getMachineState(char *state)
{
    auto index = 0;
    for (const auto machineState : Grbl::MACHINE_STATES)
    {
        if (strcmp(state, machineState->name()) >= 0)
        {
            return *machineState;
        }

        index++;
    }

    return Grbl::MachineStates::Unknown;
}
Grbl::PositionMode GrblInterface::getPositionMode(char *mode)
{
    auto index = 0;
    for (const auto positionMode : Grbl::POSITION_MODES)
    {
        if (strcmp(mode, positionMode->name()) >= 0)
        {
            return *positionMode;
        }

        index++;
    }

    return Grbl::PositionModes::Unknown;
}

bool GrblInterface::processBuffer()
{
    MatchState ms;
    ms.Target(m_buffer);
    auto result = ms.Match((char *)STATUS_REPORT_REGEX);
    m_currentIndex = 0;

    if (result == 0)
    {
        return false;
    }

    ms.GetCapture(m_tempBuffer, 0);
    auto machineState = getMachineState(m_tempBuffer);
    ms.GetCapture(m_tempBuffer, 1);
    auto positionMode = getPositionMode(m_tempBuffer);
    ms.GetCapture(m_tempBuffer, 2);
    processPosition(m_tempBuffer);

    if (onPositionUpdated != nullptr)
    {
        onPositionUpdated(machineState, positionMode, m_currentPosition);
    }

    return true;
}

void GrblInterface::processPosition(char *posString)
{
    char *token = strtok(posString, VALUE_SEPARATOR);
    char floatingPointString[Grbl::FLOATING_POINT_STRING_LENGTH + 1];

    for (const auto &axis : Grbl::AXES)
    {
        if (token == NULL)
        {
            break;
        }

        float value = atof(token);
        m_currentPosition.setValue(*axis, value);
        token = strtok(NULL, VALUE_SEPARATOR);
    }
}

bool GrblInterface::requestStatusReport()
{
    static uint32_t nextAvailableAt = 0;

    if (millis() < nextAvailableAt)
    {
        return false;
    }

    m_stream->print(STATUS_REPORT_COMMAND);
    nextAvailableAt += STATUS_REPORT_MIN_INTERVAL_MS;
    return true;
}
