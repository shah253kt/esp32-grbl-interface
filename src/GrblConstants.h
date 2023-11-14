#pragma once

#include <array>

namespace Grbl
{
    constexpr auto DEFAULT_TIMEOUT_MS = 100;
    constexpr auto NUMBER_OF_AXES = 6;
    constexpr auto FLOAT_PRECISION = 3;

    enum class UnitOfMeasurement
    {
        Metric,
        Imperial
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

    inline constexpr std::array<char *, 10> machineStates = {"Idle",
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
        Unknown
    };

    enum class PositionMode
    {
        Absolute,
        Relative
    };

    inline constexpr std::array<char *, 3> coordinateModes = {"MPos", "WPos"};
}