#pragma once

#include "Arduino.h"
#include "GrblConstants.h"
#include "GCode.h"

#include <sstream>
#include <vector>

#if not defined(ESP32)
#error "This library only support ESP32"
#endif

using PositionPair = std::pair<Grbl::Axis, float>;
using Coordinate = std::array<float, Grbl::MAX_NUMBER_OF_AXES>;

class GrblInterface
{
public:
    GrblInterface(Stream &stream);

    void update(uint16_t timeout = Grbl::DEFAULT_TIMEOUT_MS);

    void setUnitOfMeasurement(Grbl::UnitOfMeasurement unitOfMeasurement);
    void setPositionMode(Grbl::PositionMode positionMode);

    void setWorkCoordinate(const std::vector<PositionPair> &position);
    void resetWorkCoordinate();

    void linearMoveRapid(const std::vector<PositionPair> &position);
    void linearMoveFeedRate(float feedRate, const std::vector<PositionPair> &position);
    void linearMoveRapidInMachineCoordinate(const std::vector<PositionPair> &position);

    void jog(float feedRate, const std::vector<PositionPair> &position);

    [[nodiscard]] float getCurrentFeedRate();
    [[nodiscard]] float getCurrentSpindleSpeed();

    [[nodiscard]] Coordinate getPosition();
    [[nodiscard]] float getPosition(Grbl::Axis axis);

    [[nodiscard]] Coordinate getWorkCoordinateOffset();
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
    Coordinate m_position;
    Coordinate m_workCoordinateOffset;
    std::stringstream m_stringStream;
    float m_currentFeedRate;
    float m_currentSpindleSpeed;

    void processBuffer();
    void resetStringStream();
    void appendGCode(GCode gCode);
    void serializePosition(const std::vector<PositionPair> &position);
    void sendGCode(GCode gCode);
    void send();
    void extractPosition(const char *positionString, Coordinate *positionArray);
};