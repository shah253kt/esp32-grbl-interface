#pragma once

#include <array>

namespace Grbl
{
    constexpr auto DEFAULT_TIMEOUT_MS = 100;
    constexpr auto MAX_NUMBER_OF_AXES = 6;
    constexpr auto FLOAT_PRECISION = 3;

    enum class UnitOfMeasurement
    {
        Inches,
        Millimeters
    };

    enum class MachineState
    {
        Idle,
        Run,
        Hold,
        Jog,
        Alarm,
        Door,
        Check,
        Home,
        Sleep,
        Unknown
    };

    inline constexpr std::array<char *, 9> machineStates = {"Idle",
                                                             "Run",
                                                             "Hold",
                                                             "Jog",
                                                             "Alarm",
                                                             "Door",
                                                             "Check",
                                                             "Home",
                                                             "Sleep"};

    enum class Axis
    {
        X,
        Y,
        Z,
        A,
        B,
        C,
        Unknown
    };

    inline constexpr std::array<char, 6> axes = {'X', 'Y', 'Z', 'A', 'B', 'C'};

    enum class CoordinateMode
    {
        Machine,
        Work,
        WorkCoordinateOffset,
        Unknown
    };

    inline constexpr std::array<char *, 3> coordinateModes = {"MPos", "WPos", "WCO"};

    enum class DistanceMode
    {
        Absolute,
        Incremental
    };

    enum class ArcMovement
    {
        Clockwise,
        CounterClockwise
    };

    enum class CoordinateOffset
    {
        Absolute,
        Relative
    };

    enum class CoordinateSystem
    {
        P1,
        P2,
        P3,
        P4,
        P5,
        P6
    };
}