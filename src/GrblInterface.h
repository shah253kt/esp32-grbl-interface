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

class GrblInterface
{
public:
    GrblInterface(Stream &stream);

    void update(uint16_t timeout = Grbl::DEFAULT_TIMEOUT_MS);
    void processBuffer();

    void setUnitOfMeasurement(Grbl::UnitOfMeasurement unitOfMeasurement);
    void setPositionMode(Grbl::PositionMode positionMode);

    void setWorkCoordinate(const std::vector<PositionPair> &position);
    void resetWorkCoordinate();

    void linearMoveRapid(const std::vector<PositionPair> &position);
    void linearMoveFeedRate(float feedRate, const std::vector<PositionPair> &position);
    void linearMoveRapidInMachineCoordinate(const std::vector<PositionPair> &position);

    void jog(float feedRate, const std::vector<PositionPair> &position);

    [[nodiscard]] std::array<float, Grbl::MAX_NUMBER_OF_AXES> getPosition();
    [[nodiscard]] float getPosition(Grbl::Axis axis);

    char *getMachineState(Grbl::MachineState machineState);
    Grbl::MachineState getMachineState(char *state);

    char getAxis(Grbl::Axis axis);
    Grbl::Axis getAxis(char axis);

    char *getCoordinateMode(Grbl::CoordinateMode coordinateMode);
    Grbl::CoordinateMode getCoordinateMode(char *coordinateMode);

    void test();

    std::function<void(Grbl::MachineState, Grbl::CoordinateMode)> onPositionUpdate;

private:
    Stream *m_stream;
    std::string m_buffer;
    std::array<float, Grbl::MAX_NUMBER_OF_AXES> m_position;
    std::stringstream m_stringStream;

    void resetStringStream();
    void appendGCode(GCode gCode);
    void serializePosition(const std::vector<PositionPair> &position);
    void sendGCode(GCode gCode);
    void send();
    void extractPosition(const char *positionString);
};