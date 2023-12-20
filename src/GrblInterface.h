#pragma once

#include "Arduino.h"
#include "GrblConstants.h"
#include "GrblCommands.h"

#include <sstream>
#include <vector>

#if not defined(ESP32)
#error "This library only support ESP32"
#endif

using PositionPair = std::pair<Grbl::Axis, float>;
using Coordinate = std::array<float, Grbl::MAX_NUMBER_OF_AXES>;
using Point = std::pair<float, float>;

enum class RotationDirection
{
    Clockwise,
    CounterClockwise
};

class GrblInterface
{
public:
    GrblInterface(Stream &stream);

    void update(uint16_t timeout = Grbl::DEFAULT_TIMEOUT_MS);

    // G-codes
    [[nodiscard]] bool setUnitOfMeasurement(Grbl::UnitOfMeasurement unitOfMeasurement);
    [[nodiscard]] bool setDistanceMode(Grbl::DistanceMode distanceMode);

    [[nodiscard]] bool setCoordinateOffset(const std::vector<PositionPair> &position);
    [[nodiscard]] bool clearCoordinateOffset();

    [[nodiscard]] bool linearRapidPositioning(const std::vector<PositionPair> &position);
    [[nodiscard]] bool linearInterpolationPositioning(float feedRate, const std::vector<PositionPair> &position);
    [[nodiscard]] bool linearPositioningInMachineCoordinate(const std::vector<PositionPair> &position);

    [[nodiscard]] bool arcInterpolationPositioning(Grbl::ArcMovement direction,
                                                   const std::vector<PositionPair> &endPosition,
                                                   float radius,
                                                   float feedRate);
    [[nodiscard]] bool arcInterpolationPositioning(Grbl::ArcMovement direction,
                                                   const std::vector<PositionPair> &endPosition,
                                                   Point centerPoint,
                                                   float feedRate);

    [[nodiscard]] bool dwell(uint16_t durationSeconds);

    [[nodiscard]] bool setCoordinateSystemOrigin(Grbl::CoordinateOffset coordinateOffset,
                                   Grbl::CoordinateSystem coordinateSystem,
                                   const std::vector<PositionPair> &position);

    [[nodiscard]] bool setPlane(Grbl::Plane plane);

    // M-codes
    [[nodiscard]] bool spindleOn(RotationDirection direction = RotationDirection::Clockwise);
    [[nodiscard]] bool spindleOff();

    // $ commands
    [[nodiscard]] bool reboot();
    [[nodiscard]] bool softReset();
    [[nodiscard]] bool pause();
    [[nodiscard]] bool resume();
    [[nodiscard]] bool runHomingCycle();
    [[nodiscard]] bool clearAlarm();
    [[nodiscard]] bool jog(float feedRate, const std::vector<PositionPair> &position);

    [[nodiscard]] float getCurrentFeedRate();
    [[nodiscard]] float getCurrentSpindleSpeed();

    [[nodiscard]] Coordinate &getWorkCoordinate();
    [[nodiscard]] float getWorkCoordinate(Grbl::Axis axis);

    [[nodiscard]] Coordinate &getMachineCoordinate();
    [[nodiscard]] float getMachineCoordinate(Grbl::Axis axis);

    [[nodiscard]] Coordinate &getWorkCoordinateOffset();
    [[nodiscard]] float getWorkCoordinateOffset(Grbl::Axis axis);

    [[nodiscard]] char *getMachineState(Grbl::MachineState machineState);
    [[nodiscard]] Grbl::MachineState getMachineState(char *state);

    [[nodiscard]] char getAxis(Grbl::Axis axis);
    [[nodiscard]] Grbl::Axis getAxis(char axis);

    [[nodiscard]] char *getCoordinateMode(Grbl::CoordinateMode coordinateMode);
    [[nodiscard]] Grbl::CoordinateMode getCoordinateMode(char *coordinateMode);

    void test();

    std::function<void(Grbl::MachineState, Grbl::CoordinateMode)> onPositionUpdate;

private:
    Stream *m_stream;
    std::string m_buffer;
    Coordinate m_workCoordinate;
    Coordinate m_workCoordinateOffset;
    Coordinate m_machineCoordinate;
    std::stringstream m_stringStream;
    float m_currentFeedRate;
    float m_currentSpindleSpeed;

    void processBuffer();
    void resetStringStream();
    void appendCommand(Grbl::Command command, char postpend = ' ');
    void appendValue(char indicator, float value, char postpend = ' ');
    void appendValue(char indicator, int value, char postpend = ' ');
    void serializePosition(const std::vector<PositionPair> &position);
    void send();
    [[nodiscard]] bool sendCommand(Grbl::Command command, bool waitForResponse = true);
    [[nodiscard]] bool sendWaitingForOkResponse(uint16_t timeout);
    void extractPosition(const char *positionString, Coordinate *positionArray);
    [[nodiscard]] float toWorkCoordinate(float machineCoordinate, float offset);
    [[nodiscard]] float toMachineCoordinate(float workCoordinate, float offset);
};