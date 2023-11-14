#include "GrblInterface.h"

#include <Regexp.h>

#include <algorithm>

namespace
{
    constexpr auto VALUE_SEPARATOR = ',';
    constexpr auto OK_RESPONSE = "ok";
    constexpr auto ERROR_RESPONSE = "error";

    constexpr auto EOL = '\r';
    constexpr auto STATUS_REPORT_COMMAND = '?';
    constexpr auto STATUS_REPORT_MIN_INTERVAL_MS = 200; // Limits the status report query to 5Hz, as recommended by Grbl.

    namespace RegEx
    {
        // Note: To test Lua style regex, use the following tool:
        // https://montymahato.github.io/lua-pattern-tester/
        constexpr auto STATUS_REPORT = "<([%w:%d]+)%|(%w+):([-%d.,]+)[%|]?.*>";
    }

    namespace ResponseIndex
    {
        constexpr auto STATUS_REPORT_MACHINE_STATE = 0;
        constexpr auto STATUS_REPORT_POSITION_MODE = 1;
        constexpr auto STATUS_REPORT_POSITION = 2;
    }
}

GrblInterface::GrblInterface(Stream &stream) : m_stream(&stream)
{
}

void GrblInterface::update(uint16_t timeout)
{
    if (!m_stream->available())
    {
        return;
    }

    const auto timeoutAt = millis() + timeout;
    std::stringstream ss;

    while (m_stream->available() && millis() < timeoutAt)
    {
        const char c = m_stream->read();

        if (c == EOL)
        {
            m_buffer.append(ss.str());
            processBuffer();
            return;
        }

        ss << c;
    }

    m_buffer.append(ss.str());
}

void GrblInterface::processBuffer()
{
    static MatchState ms;
    char buffer[m_buffer.length() + 1];
    char tempBuffer[m_buffer.length() + 1];
    strcpy(buffer, m_buffer.c_str());
    ms.Target(buffer);
    m_buffer.clear();

    if (ms.Match((char *)RegEx::STATUS_REPORT) > 0)
    {
        ms.GetCapture(tempBuffer, ResponseIndex::STATUS_REPORT_MACHINE_STATE);
        auto machineState = getMachineState(tempBuffer);

        if (machineState == Grbl::MachineState::Unknown)
        {
            return;
        }

        ms.GetCapture(tempBuffer, ResponseIndex::STATUS_REPORT_POSITION_MODE);
        auto coordinateMode = getCoordinateMode(tempBuffer);

        if (coordinateMode == Grbl::CoordinateMode::Unknown)
        {
            return;
        }

        ms.GetCapture(tempBuffer, ResponseIndex::STATUS_REPORT_POSITION);
        extractPosition(tempBuffer);

        if (onPositionUpdate)
        {
            onPositionUpdate(machineState, coordinateMode);
        }
    }
}

void GrblInterface::setUnitOfMeasurement(const Grbl::UnitOfMeasurement unitOfMeasurement)
{
    switch (unitOfMeasurement)
    {
    case Grbl::UnitOfMeasurement::Imperial:
    {
        sendGCode(GCode::ImperialUnit);
        break;
    }
    case Grbl::UnitOfMeasurement::Metric:
    {
        sendGCode(GCode::MetricUnit);
        break;
    }
    }
}

void GrblInterface::setPositionMode(Grbl::PositionMode positionMode)
{
    switch (positionMode)
    {
    case Grbl::PositionMode::Absolute:
    {
        sendGCode(GCode::AbsoluteCoordinate);
        break;
    }
    case Grbl::PositionMode::Relative:
    {
        sendGCode(GCode::RelativeCoordinate);
        break;
    }
    }
}

void GrblInterface::setWorkCoordinate(const std::vector<PositionPair> &position)
{
    resetStringStream();
    appendGCode(GCode::DefineWorkCoordinate);
    serializePosition(position);
    send();
}

void GrblInterface::resetWorkCoordinate()
{
    sendGCode(GCode::ResetWorkCoordinate);
}

void GrblInterface::linearMoveRapid(const std::vector<PositionPair> &position)
{
    resetStringStream();
    appendGCode(GCode::LinearMoveRapid);
    serializePosition(position);
    send();
}

void GrblInterface::linearMoveFeedRate(float feedRate, const std::vector<PositionPair> &position)
{
    resetStringStream();
    appendGCode(GCode::LinearMoveFeedRate);
    m_stringStream << 'F' << feedRate;
    serializePosition(position);
    send();
}

void GrblInterface::linearMoveRapidInMachineCoordinate(const std::vector<PositionPair> &position)
{
    resetStringStream();
    appendGCode(GCode::LinearMoveMachineCoordinate);
    serializePosition(position);
    send();
}

void GrblInterface::jog(float feedRate, const std::vector<PositionPair> &position)
{
    resetStringStream();
    appendGCode(GCode::Jog);
    m_stringStream << 'F' << feedRate;
    serializePosition(position);
    send();
}

std::array<float, Grbl::MAX_NUMBER_OF_AXES> GrblInterface::getPosition()
{
    return m_position;
}

float GrblInterface::getPosition(const Grbl::Axis axis)
{
    return m_position[static_cast<int>(axis)];
}

char *GrblInterface::getMachineState(Grbl::MachineState machineState)
{
    if (machineState == Grbl::MachineState::Unknown)
    {
        return nullptr;
    }

    return Grbl::machineStates[static_cast<int>(machineState)];
}

Grbl::MachineState GrblInterface::getMachineState(char *state)
{
    for (auto i = 0; i < Grbl::machineStates.size(); i++)
    {
        if (strcmp(state, Grbl::machineStates[i]) == 0)
        {
            return static_cast<Grbl::MachineState>(i);
        }
    }

    return Grbl::MachineState::Unknown;
}

char GrblInterface::getAxis(Grbl::Axis axis)
{
    if (axis == Grbl::Axis::Unknown)
    {
        return '\0';
    }

    return Grbl::axes[static_cast<int>(axis)];
}

Grbl::Axis GrblInterface::getAxis(char axis)
{
    for (auto i = 0; i < Grbl::axes.size(); i++)
    {
        if (axis == Grbl::axes[i])
        {
            return static_cast<Grbl::Axis>(i);
        }
    }

    return Grbl::Axis::Unknown;
}

char *GrblInterface::getCoordinateMode(Grbl::CoordinateMode coordinateMode)
{
    if (coordinateMode == Grbl::CoordinateMode::Unknown)
    {
        return nullptr;
    }

    return Grbl::coordinateModes[static_cast<int>(coordinateMode)];
}

Grbl::CoordinateMode GrblInterface::getCoordinateMode(char *coordinateMode)
{
    for (auto i = 0; i < Grbl::coordinateModes.size(); i++)
    {
        if (strcmp(coordinateMode, Grbl::coordinateModes[i]) == 0)
        {
            return static_cast<Grbl::CoordinateMode>(i);
        }
    }

    return Grbl::CoordinateMode::Unknown;
}

void GrblInterface::test()
{
}

// --------------------------------------------------------------------------------------------------
// Private methods
// --------------------------------------------------------------------------------------------------

void GrblInterface::resetStringStream()
{
    std::stringstream ss;
    m_stringStream.swap(ss);
    m_stringStream.setf(std::ios::fixed);
    m_stringStream.precision(Grbl::FLOAT_PRECISION);
}

void GrblInterface::appendGCode(const GCode gCode)
{
    m_stringStream << Grbl::getGCode(gCode);
}

void GrblInterface::serializePosition(const std::vector<PositionPair> &position)
{
    std::for_each(position.begin(), position.end(), [this](const PositionPair &pos)
                  { m_stringStream << getAxis(pos.first) << pos.second; });
}

void GrblInterface::sendGCode(const GCode gCode)
{
    m_stream->println(Grbl::getGCode(gCode));
}

void GrblInterface::send()
{
    m_stream->println(m_stringStream.str().c_str());
}

void GrblInterface::extractPosition(const char *positionString)
{
    std::string pos(positionString);
    std::string position;
    std::stringstream ss(pos);
    const auto numberOfAxes = std::count(pos.begin(), pos.end(), VALUE_SEPARATOR) + 1;

    if (numberOfAxes > Grbl::MAX_NUMBER_OF_AXES) {
        return;
    }

    for (auto i = 0; i < numberOfAxes; i++)
    {
        std::getline(ss, position, VALUE_SEPARATOR);

        if (!position.empty())
        {
            try
            {
                m_position[i] = std::stof(position);
            }
            catch (std::invalid_argument &e)
            {
                return;
            }
        }
    }
}
