#include "GrblInterface.h"

#include <Regexp.h>

#include <algorithm>

namespace
{
    constexpr auto COORDINATE_SYSTEM_INDICATOR = 'P';
    constexpr auto RADIUS_INDICATOR = 'R';
    constexpr auto FEED_RATE_INDICATOR = 'F';
    constexpr auto VALUE_SEPARATOR = ',';
    constexpr auto OK_RESPONSE = "ok";
    constexpr auto ERROR_RESPONSE = "error";
    constexpr auto EOL = '\r';
    constexpr auto STATUS_REPORT_MIN_INTERVAL_MS = 200; // Limits the status report query to 5Hz, as recommended by Grbl.

    namespace RegEx
    {
        // Note: To test Lua style regex, use the following tool:
        // https://montymahato.github.io/lua-pattern-tester/
        constexpr auto STATUS_REPORT = "<([%w:%d]+)%|(%w+):([-%d.,]+)[%|]?.*>";
        constexpr auto FEED_AND_SPEED = "FS:(%-?%d+%.?%d*),(%-?%d+%.?%d*)";
        constexpr auto WORK_COORDINATE_OFFSET = "WCO:([%-?%d+%.?%d*,]*)";
    }

    namespace ResponseIndex
    {
        constexpr auto STATUS_REPORT_MACHINE_STATE = 0;
        constexpr auto STATUS_REPORT_POSITION_MODE = 1;
        constexpr auto STATUS_REPORT_POSITION = 2;
        constexpr auto STATUS_REPORT_FEED_RATE = 0;
        constexpr auto STATUS_REPORT_SPINDLE_SPEED = 1;
        constexpr auto STATUS_REPORT_WORK_COORDINATE_OFFSET = 0;
    }
}

GrblInterface::GrblInterface(Stream &stream)
    : m_stream(&stream),
      m_currentFeedRate(0),
      m_currentSpindleSpeed(0)
{
}

void GrblInterface::update(uint16_t timeout)
{
    static uint32_t nextStatusReportRequestAt = 0;

    if (millis() >= nextStatusReportRequestAt)
    {
        sendCommand(Grbl::Command::StatusReport);
        nextStatusReportRequestAt = millis() + STATUS_REPORT_MIN_INTERVAL_MS;
    }

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

void GrblInterface::setUnitOfMeasurement(const Grbl::UnitOfMeasurement unitOfMeasurement)
{
    switch (unitOfMeasurement)
    {
    case Grbl::UnitOfMeasurement::Inches:
    {
        sendCommand(Grbl::Command::G20_UnitsInches);
        break;
    }
    case Grbl::UnitOfMeasurement::Millimeters:
    {
        sendCommand(Grbl::Command::G21_UnitsMillimeters);
        break;
    }
    }
}

void GrblInterface::setDistanceMode(Grbl::DistanceMode distanceMode)
{
    switch (distanceMode)
    {
    case Grbl::DistanceMode::Absolute:
    {
        sendCommand(Grbl::Command::G90_DistanceModeAbsolute);
        break;
    }
    case Grbl::DistanceMode::Incremental:
    {
        sendCommand(Grbl::Command::G91_DistanceModeIncremental);
        break;
    }
    }
}

void GrblInterface::setCoordinateOffset(const std::vector<PositionPair> &position)
{
    resetStringStream();
    appendCommand(Grbl::Command::G92_CoordinateOffset);
    serializePosition(position);
    send();
}

void GrblInterface::clearCoordinateOffset()
{
    sendCommand(Grbl::Command::G92_1_ClearCoordinateSystemOffsets);
}

void GrblInterface::linearRapidPositioning(const std::vector<PositionPair> &position)
{
    resetStringStream();
    appendCommand(Grbl::Command::G0_RapidPositioning);
    serializePosition(position);
    send();
}

void GrblInterface::linearInterpolationPositioning(float feedRate, const std::vector<PositionPair> &position)
{
    resetStringStream();
    appendCommand(Grbl::Command::G1_LinearInterpolation);
    appendValue(FEED_RATE_INDICATOR, feedRate);
    serializePosition(position);
    send();
}

void GrblInterface::linearPositioningInMachineCoordinate(const std::vector<PositionPair> &position)
{
    resetStringStream();
    appendCommand(Grbl::Command::G53_MoveInAbsoluteCoordinates);
    serializePosition(position);
    send();
}

void GrblInterface::arcInterpolationPositioning(Grbl::ArcMovement direction,
                                                const std::vector<PositionPair> &endPosition,
                                                float radius,
                                                float feedRate)
{
    resetStringStream();
    switch (direction)
    {
    case Grbl::ArcMovement::Clockwise:
    {
        appendCommand(Grbl::Command::G2_ClockwiseCircularInterpolation);
        break;
    }
    case Grbl::ArcMovement::CounterClockwise:
    {
        appendCommand(Grbl::Command::G3_CounterclockwiseCircularInterpolation);
        break;
    }
    }

    serializePosition(endPosition);
    appendValue(RADIUS_INDICATOR, radius);
    appendValue(FEED_RATE_INDICATOR, feedRate);
    send();
}

void GrblInterface::arcInterpolationPositioning(Grbl::ArcMovement direction,
                                                const std::vector<PositionPair> &endPosition,
                                                Point centerPoint,
                                                float feedRate)
{
    resetStringStream();
    switch (direction)
    {
    case Grbl::ArcMovement::Clockwise:
    {
        appendCommand(Grbl::Command::G2_ClockwiseCircularInterpolation);
        break;
    }
    case Grbl::ArcMovement::CounterClockwise:
    {
        appendCommand(Grbl::Command::G3_CounterclockwiseCircularInterpolation);
        break;
    }
    }

    serializePosition(endPosition);
    appendValue('I', centerPoint.first);
    appendValue('J', centerPoint.second);
    appendValue(FEED_RATE_INDICATOR, feedRate);
    send();
}

void GrblInterface::dwell(uint16_t durationMS)
{
    resetStringStream();
    appendCommand(Grbl::Command::G4_Dwell);
    appendValue('P', durationMS);
    send();
}

void GrblInterface::setCoordinateSystemOrigin(Grbl::CoordinateOffset coordinateOffset,
                                              Grbl::CoordinateSystem coordinateSystem,
                                              const std::vector<PositionPair> &position)
{
    resetStringStream();

    switch (coordinateOffset)
    {
    case Grbl::CoordinateOffset::Absolute:
    {
        appendCommand(Grbl::Command::G10_L2_SetWorkCoordinateOffsets);
        break;
    }
    case Grbl::CoordinateOffset::Relative:
    {
        appendCommand(Grbl::Command::G10_L20_SetWorkCoordinateOffsets);
        break;
    }
    }

    appendValue(COORDINATE_SYSTEM_INDICATOR, (static_cast<int>(coordinateSystem) + 1));
    serializePosition(position);
    send();
}

void GrblInterface::setPlane(Grbl::Plane plane)
{
    switch (plane)
    {
    case Grbl::Plane::XY:
    {
        sendCommand(Grbl::Command::G17_PlaneSelectionXY);
        break;
    }
    case Grbl::Plane::ZX:
    {
        sendCommand(Grbl::Command::G18_PlaneSelectionZX);
        break;
    }
    case Grbl::Plane::YZ:
    {
        sendCommand(Grbl::Command::G19_PlaneSelectionYZ);
        break;
    }
    }
}

void GrblInterface::jog(float feedRate, const std::vector<PositionPair> &position)
{
    resetStringStream();
    appendCommand(Grbl::Command::RunJoggingMotion);
    appendValue(FEED_RATE_INDICATOR, feedRate);
    serializePosition(position);
    send();
}

float GrblInterface::getCurrentFeedRate()
{
    return m_currentFeedRate;
}

float GrblInterface::getCurrentSpindleSpeed()
{
    return m_currentSpindleSpeed;
}

Coordinate &GrblInterface::getWorkCoordinate()
{
    return m_workCoordinate;
}

float GrblInterface::getWorkCoordinate(const Grbl::Axis axis)
{
    if (axis == Grbl::Axis::Unknown)
    {
        return 0;
    }

    return m_workCoordinate[static_cast<int>(axis)];
}

Coordinate &GrblInterface::getMachineCoordinate()
{
    Coordinate machineCoordinate;

    for (auto i = 0; i < Grbl::MAX_NUMBER_OF_AXES; i++)
    {
        machineCoordinate[i] = toMachineCoordinate(m_workCoordinate[i], m_workCoordinateOffset[i]);
    }

    return machineCoordinate;
}

float GrblInterface::getMachineCoordinate(const Grbl::Axis axis)
{
    if (axis == Grbl::Axis::Unknown)
    {
        return 0;
    }

    const auto i = static_cast<int>(axis);
    return toMachineCoordinate(m_workCoordinate[i], m_workCoordinateOffset[i]);
}

Coordinate &GrblInterface::getWorkCoordinateOffset()
{
    return m_workCoordinateOffset;
}

float GrblInterface::getWorkCoordinateOffset(const Grbl::Axis axis)
{
    return m_workCoordinateOffset[static_cast<int>(axis)];
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

void GrblInterface::processBuffer()
{
    static MatchState ms;
    char buffer[m_buffer.length() + 1];
    char tempBuffer[m_buffer.length() + 1];
    strcpy(buffer, m_buffer.c_str());
    ms.Target(buffer);
    m_buffer.clear();

    if (ms.Match((char *)RegEx::FEED_AND_SPEED) > 0)
    {
        ms.GetCapture(tempBuffer, ResponseIndex::STATUS_REPORT_FEED_RATE);
        m_currentFeedRate = atof(tempBuffer);
        ms.GetCapture(tempBuffer, ResponseIndex::STATUS_REPORT_SPINDLE_SPEED);
        m_currentSpindleSpeed = atof(tempBuffer);
    }

    if (ms.Match((char *)RegEx::WORK_COORDINATE_OFFSET) > 0)
    {
        ms.GetCapture(tempBuffer, ResponseIndex::STATUS_REPORT_WORK_COORDINATE_OFFSET);
        extractPosition(tempBuffer, &m_workCoordinateOffset);
    }

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
        extractPosition(tempBuffer, &m_workCoordinate);

        if (coordinateMode == Grbl::CoordinateMode::Machine)
        {
            for (auto i = 0; i < Grbl::MAX_NUMBER_OF_AXES; i++)
            {
                m_workCoordinate[i] = toWorkCoordinate(m_workCoordinate[i], m_workCoordinateOffset[i]);
            }
        }

        if (onPositionUpdate)
        {
            onPositionUpdate(machineState, coordinateMode);
        }
    }
}

void GrblInterface::resetStringStream()
{
    std::stringstream ss;
    m_stringStream.swap(ss);
    m_stringStream.setf(std::ios::fixed);
    m_stringStream.precision(Grbl::FLOAT_PRECISION);
}

void GrblInterface::appendCommand(const Grbl::Command command, char postpend)
{
    m_stringStream << Grbl::getCommand(command) << postpend;
}

void GrblInterface::appendValue(char indicator, float value, char postpend)
{
    m_stringStream << indicator << value << postpend;
}

void GrblInterface::appendValue(char indicator, int value, char postpend)
{
    m_stringStream << indicator << value << postpend;
}

void GrblInterface::serializePosition(const std::vector<PositionPair> &position)
{
    std::for_each(position.begin(), position.end(), [this](const PositionPair &pos)
                  { m_stringStream << getAxis(pos.first) << pos.second << ' '; });
}

void GrblInterface::sendCommand(const Grbl::Command command)
{
    m_stream->println(Grbl::getCommand(command));
}

void GrblInterface::send()
{
    m_stream->println(m_stringStream.str().c_str());
}

void GrblInterface::extractPosition(const char *positionString, Coordinate *positionArray)
{
    std::string pos(positionString);
    std::string position;
    std::stringstream ss(pos);
    const auto numberOfAxes = std::count(pos.begin(), pos.end(), VALUE_SEPARATOR) + 1;

    if (numberOfAxes > Grbl::MAX_NUMBER_OF_AXES)
    {
        return;
    }

    for (auto i = 0; i < numberOfAxes; i++)
    {
        std::getline(ss, position, VALUE_SEPARATOR);

        if (!position.empty())
        {
            try
            {
                (*positionArray)[i] = std::stof(position);
            }
            catch (std::invalid_argument &e)
            {
                return;
            }
        }
    }
}

float GrblInterface::toWorkCoordinate(float machineCoordinate, float offset)
{
    // WPos = MPos - WCO
    return machineCoordinate - offset;
}

float GrblInterface::toMachineCoordinate(float workCoordinate, float offset)
{
    // MPos = WPos + WCO
    return workCoordinate + offset;
}
