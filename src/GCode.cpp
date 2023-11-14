#include "GCode.h"

#include <array>

constexpr std::array<char *, 38> gCodes = {
    "G27",   // Park,
    "G28",   // AutoHome,
    "G0",    // LinearMoveRapid
    "G1",    // LinearMoveFeedRate
    "G2",    // ClockwiseArcMove
    "G3",    // CounterClockwiseArcMove
    "G38.2", // ProbeTowardsWorkpiece
    "G38.3", // ProbeTowardsWorkpieceWithErrorSuppression
    "G38.4", // ProbeAwayFromWorkpiece
    "G38.5", // ProbeAwayFromWorkpieceWithErrorSuppression
    "G80",   // CancelMotionMode
    "G53",   // LinearMoveMachineCoordinate
    "G54",   // CoordinateSystem1
    "G55",   // CoordinateSystem2
    "G56",   // CoordinateSystem3
    "G57",   // CoordinateSystem4
    "G58",   // CoordinateSystem5
    "G59",   // CoordinateSystem6
    "G59.1", // CoordinateSystem7
    "G59.2", // CoordinateSystem8
    "G59.3", // CoordinateSystem9
    "G17",   // SelectPlaneXY
    "G18",   // SelectPlaneZX
    "G19",   // SelectPlaneYZ
    "G90",   // AbsoluteCoordinate
    "G91",   // RelativeCoordinate
    "G92",   // DefineWorkCoordinate
    "G92.1", // ResetWorkCoordinate
    "G20",   // ImperialUnit
    "G21",   // MetricUnit
    "M0",    // UnconditionalStop
    "M3",    // SpindleClockwise
    "M4",    // SpindleCounterClockwise
    "M5",    // SpindleOff
    "M7",    // MistCoolantOn
    "M8",    // FloodOrAirCoolantOn
    "M9",    // CoolantOff
    "$J="    // Jog
};

char *Grbl::getGCode(GCode gCode)
{
    return gCodes[static_cast<int>(gCode)];
}