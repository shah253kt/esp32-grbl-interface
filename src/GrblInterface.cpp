#include "GrblInterface.h"
#include "Utils.h"

#include <Regexp.h>

#include <algorithm>

namespace
{
    constexpr auto COORDINATE_SYSTEM_INDICATOR = 'P';
    constexpr auto RADIUS_INDICATOR = 'R';
    constexpr auto FEED_RATE_INDICATOR = 'F';
    constexpr auto VALUE_SEPARATOR = ',';
    constexpr auto ERROR_RESPONSE = "error";
    constexpr auto EOL = '\r';
    constexpr auto STATUS_REPORT_MIN_INTERVAL_MS = 200; // Limits the status report query to 5Hz, as recommended by Grbl.
    constexpr auto RESPONSE_TIMEOUT = 200;

    namespace RegEx
    {
        // Note: To test Lua style regex, use the following tool:
        // https://montymahato.github.io/lua-pattern-tester/
        constexpr auto STATUS_REPORT = "<([%w:%d]+)%|(%w+):([-%d.,]+)[%|]?.*>";
        constexpr auto FEED_AND_SPEED = "FS:(%-?%d+%.?%d*),(%-?%d+%.?%d*)";
        constexpr auto WORK_COORDINATE_OFFSET = "WCO:([%-?%d+%.?%d*,]*)";
        constexpr auto OK_RESPONSE = "ok";
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
        sendCommand(Grbl::Command::StatusReport, false);
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
        // Serial.print(c);

        if (c == EOL)
        {
            m_buffer.append(ss.str());
            processBuffer();
            return;
        }

        ss << c;
    }

    if (!ss.str().empty())
    {
        m_buffer.append(ss.str());
    }
}

void GrblInterface::clearBuffer()
{
    while (m_stream->available())
    {
        m_stream->read();
    }
}

// G-codes
bool GrblInterface::setUnitOfMeasurement(const Grbl::UnitOfMeasurement unitOfMeasurement)
{
    switch (unitOfMeasurement)
    {
    case Grbl::UnitOfMeasurement::Inches:
    {
        return sendCommand(Grbl::Command::G20_UnitsInches);
    }
    case Grbl::UnitOfMeasurement::Millimeters:
    {
        return sendCommand(Grbl::Command::G21_UnitsMillimeters);
    }
    }
}

bool GrblInterface::setDistanceMode(Grbl::DistanceMode distanceMode)
{
    switch (distanceMode)
    {
    case Grbl::DistanceMode::Absolute:
    {
        return sendCommand(Grbl::Command::G90_DistanceModeAbsolute);
    }
    case Grbl::DistanceMode::Incremental:
    {
        return sendCommand(Grbl::Command::G91_DistanceModeIncremental);
    }
    }
}

bool GrblInterface::setCoordinateOffset(const std::vector<PositionPair> &position)
{
    resetStringStream();
    appendCommand(Grbl::Command::G92_CoordinateOffset);
    serializePosition(position);
    send();
}

bool GrblInterface::clearCoordinateOffset()
{
    sendCommand(Grbl::Command::G92_1_ClearCoordinateSystemOffsets);
}

bool GrblInterface::linearRapidPositioning(const std::vector<PositionPair> &position)
{
    resetStringStream();
    appendCommand(Grbl::Command::G0_RapidPositioning);
    serializePosition(position);
    return sendWaitingForOkResponse(RESPONSE_TIMEOUT);
}

bool GrblInterface::linearInterpolationPositioning(float feedRate, const std::vector<PositionPair> &position)
{
    resetStringStream();
    appendCommand(Grbl::Command::G1_LinearInterpolation);
    appendValue(FEED_RATE_INDICATOR, feedRate);
    serializePosition(position);
    return sendWaitingForOkResponse(RESPONSE_TIMEOUT);
}

bool GrblInterface::linearPositioningInMachineCoordinate(const std::vector<PositionPair> &position)
{
    resetStringStream();
    appendCommand(Grbl::Command::G53_MoveInAbsoluteCoordinates);
    serializePosition(position);
    return sendWaitingForOkResponse(RESPONSE_TIMEOUT);
}

bool GrblInterface::arcInterpolationPositioning(Grbl::ArcMovement direction,
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
    return sendWaitingForOkResponse(RESPONSE_TIMEOUT);
}

bool GrblInterface::arcInterpolationPositioning(Grbl::ArcMovement direction,
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
    return sendWaitingForOkResponse(RESPONSE_TIMEOUT);
}

bool GrblInterface::dwell(uint16_t durationSeconds)
{
    resetStringStream();
    appendCommand(Grbl::Command::G4_Dwell);
    appendValue('P', durationSeconds);
    return sendWaitingForOkResponse(RESPONSE_TIMEOUT);
}

bool GrblInterface::setCoordinateSystemOrigin(Grbl::CoordinateOffset coordinateOffset,
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
    return sendWaitingForOkResponse(RESPONSE_TIMEOUT);
}

bool GrblInterface::setPlane(Grbl::Plane plane)
{
    switch (plane)
    {
    case Grbl::Plane::XY:
    {
        return sendCommand(Grbl::Command::G17_PlaneSelectionXY);
    }
    case Grbl::Plane::ZX:
    {
        return sendCommand(Grbl::Command::G18_PlaneSelectionZX);
    }
    case Grbl::Plane::YZ:
    {
        return sendCommand(Grbl::Command::G19_PlaneSelectionYZ);
    }
    }
}

// M-codes
bool GrblInterface::spindleOn(RotationDirection direction)
{
    switch (direction)
    {
    case RotationDirection::Clockwise:
    {
        return sendCommand(Grbl::Command::M3_SpindleControlCW);
    }
    case RotationDirection::CounterClockwise:
    {
        return sendCommand(Grbl::Command::M4_SpindleControlCCW);
    }
    }
}

bool GrblInterface::spindleOff()
{
    return sendCommand(Grbl::Command::M5_SpindleStop);
}

// $ commands
bool GrblInterface::reboot()
{
    return sendCommand(Grbl::Command::RebootProcessor);
}

bool GrblInterface::softReset()
{
    return sendCommand(Grbl::Command::SoftReset);
}

bool GrblInterface::pause()
{
    return sendCommand(Grbl::Command::Pause);
}

bool GrblInterface::resume()
{
    return sendCommand(Grbl::Command::Resume);
}

bool GrblInterface::runHomingCycle()
{
    return sendCommand(Grbl::Command::RunHomingCycle, false);
}

bool GrblInterface::runHomingCycle(const Grbl::Axis axis)
{
    resetStringStream();
    m_stringStream << Grbl::getCommand(Grbl::Command::RunHomingCycle) << getAxis(axis);
    send();
    return true;
}

bool GrblInterface::clearAlarm()
{
    return sendCommand(Grbl::Command::ClearAlarmLock);
}

bool GrblInterface::jog(float feedRate, const std::vector<PositionPair> &position)
{
    resetStringStream();
    appendCommand(Grbl::Command::RunJoggingMotion);
    appendValue(FEED_RATE_INDICATOR, feedRate);
    serializePosition(position);
    return sendWaitingForOkResponse(RESPONSE_TIMEOUT);
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
    for (auto i = 0; i < Grbl::MAX_NUMBER_OF_AXES; i++)
    {
        m_machineCoordinate[i] = toMachineCoordinate(m_workCoordinate[i], m_workCoordinateOffset[i]);
    }

    return m_machineCoordinate;
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

bool GrblInterface::machineIsAt(const std::vector<PositionPair> &position)
{
    return std::all_of(position.begin(), position.end(), [this](const PositionPair &pos)
                       { return Utils::equals(pos.second, getMachineCoordinate(pos.first)); });
}

Grbl::MachineState GrblInterface::currentMachineState()
{
    update();
    return m_machineState;
}

char *GrblInterface::getMachineState(Grbl::MachineState machineState)
{
    if (machineState == Grbl::MachineState::Unknown)
    {
        return "N/A";
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
        return "N/A";
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

        m_machineState = machineState;
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

    if (ms.Match((char *)RegEx::OK_RESPONSE) > 0 && onOkResponseReceived)
    {
        onOkResponseReceived(true);
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

void GrblInterface::send()
{
    if (onGCodeAboutToBeSent)
    {
        onGCodeAboutToBeSent(m_stringStream.str());
    }

    m_stream->println(m_stringStream.str().c_str());
}

bool GrblInterface::sendCommand(const Grbl::Command command, bool waitForResponse)
{
    resetStringStream();
    m_stringStream << Grbl::getCommand(command);

    if (waitForResponse)
    {
        return sendWaitingForOkResponse(RESPONSE_TIMEOUT);
    }

    send();
    return true;
}

bool GrblInterface::sendWaitingForOkResponse(uint16_t timeout)
{
    uint32_t timeoutAt = millis() + timeout;
    std::stringstream ss;
    bool okResponseReceived = false;

    onOkResponseReceived = [&okResponseReceived](bool result)
    {
        okResponseReceived = result;
    };

    send();

    while (millis() < timeoutAt)
    {
        update(); // Process current buffer

        if (okResponseReceived)
        {
            return true;
        }
    }

    return false;
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
