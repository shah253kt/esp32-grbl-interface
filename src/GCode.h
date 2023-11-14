#pragma once

enum class GCode
{
    // Auto motion
    Park,
    AutoHome,

    // Motion mode
    LinearMoveRapid,
    LinearMoveFeedRate,
    ClockwiseArcMove,
    CounterClockwiseArcMove,
    ProbeTowardsWorkpiece,
    ProbeTowardsWorkpieceWithErrorSuppression,
    ProbeAwayFromWorkpiece,
    ProbeAwayFromWorkpieceWithErrorSuppression,
    CancelMotionMode,

    // Coordinate system selection
    LinearMoveMachineCoordinate,
    CoordinateSystem1,
    CoordinateSystem2,
    CoordinateSystem3,
    CoordinateSystem4,
    CoordinateSystem5,
    CoordinateSystem6,
    CoordinateSystem7,
    CoordinateSystem8,
    CoordinateSystem9,

    // Plane selection
    SelectPlaneXY,
    SelectPlaneZX,
    SelectPlaneYZ,

    // Distance mode
    AbsoluteCoordinate,
    RelativeCoordinate,

    // Set position
    DefineWorkCoordinate,
    ResetWorkCoordinate,
    
    // Units of measurement
    ImperialUnit,
    MetricUnit,

    // Unconditional stop
    UnconditionalStop,

    // Spindle control
    SpindleClockwise,
    SpindleCounterClockwise,
    SpindleOff,

    // Coolant control
    MistCoolantOn,
    FloodOrAirCoolantOn,
    CoolantOff,

    // Jog
    Jog
};

namespace Grbl
{
    [[nodiscard]] char *getGCode(GCode gCode);
}