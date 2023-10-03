#pragma once

#include "Arduino.h"
#include "EnumClass.h"

namespace Grbl
{
    constexpr auto DEFAULT_TIMEOUT_MS = 100;
    constexpr auto BUFFER_SIZE = 128;
    constexpr auto TEMP_BUFFER_SIZE = 70;
    constexpr auto MAX_AXES = 6;

    constexpr auto FLOATING_POINT_INTEGRAL_PART_LENGTH = 5;
    constexpr auto FLOATING_POINT_FRACTIONAL_PART_LENGTH = 3;
    constexpr auto FLOATING_POINT_STRING_LENGTH = FLOATING_POINT_INTEGRAL_PART_LENGTH + FLOATING_POINT_FRACTIONAL_PART_LENGTH + 1;
    constexpr auto MAX_GCODE_LENGTH = ((FLOATING_POINT_STRING_LENGTH + 1) * MAX_AXES) + 3;

    typedef EnumClass MachineState;

    namespace MachineStates
    {
        inline const auto Unknown = MachineState("Unknown", 0);
        inline const auto Idle = MachineState("Idle", 1);
        inline const auto Run = MachineState("Run", 2);
        inline const auto Hold = MachineState("Hold", 3);
        inline const auto Jog = MachineState("Jog", 4);
        inline const auto Alarm = MachineState("Alarm", 5);
        inline const auto Door = MachineState("Door", 6);
        inline const auto Check = MachineState("Check", 7);
        inline const auto Home = MachineState("Home", 8);
        inline const auto Sleep = MachineState("Sleep", 9);
    }

    inline const MachineState *MACHINE_STATES[] = {
        &MachineStates::Unknown,
        &MachineStates::Idle,
        &MachineStates::Run,
        &MachineStates::Hold,
        &MachineStates::Jog,
        &MachineStates::Alarm,
        &MachineStates::Door,
        &MachineStates::Check,
        &MachineStates::Home,
        &MachineStates::Sleep};

    typedef EnumClass PositionMode;

    namespace PositionModes
    {
        inline const auto Unknown = PositionMode("Unknown", 0);
        inline const auto Machine = PositionMode("MPos", 1);
        inline const auto Work = PositionMode("WPos", 2);
    }

    inline const PositionMode *POSITION_MODES[] = {
        &PositionModes::Unknown,
        &PositionModes::Machine,
        &PositionModes::Work};

    typedef EnumClass Axis;

    namespace Axes
    {
        inline const auto X = Axis("X", 0x01);
        inline const auto Y = Axis("Y", 0x02);
        inline const auto Z = Axis("Z", 0x04);
        inline const auto A = Axis("A", 0x08);
        inline const auto B = Axis("B", 0x10);
        inline const auto C = Axis("C", 0x20);
    }

    inline const Axis *AXES[] = {
        &Axes::X,
        &Axes::Y,
        &Axes::Z,
        &Axes::A,
        &Axes::B,
        &Axes::C};

    struct PositionPair
    {
        Axis axis{};
        float value{0};
        bool isSet{false};

        void setValue(float val)
        {
            value = val;
            isSet = true;
        }

        void reset()
        {
            value = 0;
            isSet = false;
        }
    };

    class Position
    {
    public:
        [[nodiscard]] PositionPair *get(const Axis &targetAxis) const
        {
            for (auto &positionPair : positionPairs)
            {
                if (positionPair.axis == targetAxis)
                {
                    return &positionPair;
                }
            }
            return nullptr;
        }

        [[nodiscard]] float getValue(const Axis &targetAxis) const
        {
            const auto positionPair = get(targetAxis);
            if (positionPair == nullptr)
            {
                return 0;
            }
            return positionPair->value;
        }

        bool setValue(const Axis &targetAxis, float value)
        {
            const auto positionPair = get(targetAxis);
            if (positionPair == nullptr)
            {
                return false;
            }
            positionPair->setValue(value);
            return true;
        }

    private:
        mutable PositionPair positionPairs[MAX_AXES] = {
            {.axis = Axes::X},
            {.axis = Axes::Y},
            {.axis = Axes::Z},
            {.axis = Axes::A},
            {.axis = Axes::B},
            {.axis = Axes::C}};
    };
}

class GrblInterface
{
public:
    GrblInterface(Stream &stream);

    [[nodiscard]] bool update(uint16_t timeout = Grbl::DEFAULT_TIMEOUT_MS);
    [[nodiscard]] bool encode(char c);

    void setPosition(const Grbl::Axis &axis, float value);
    void sendLinearMove() const;
    void sendLinearMove(float feedRate);

    void (*onPositionUpdated)(const Grbl::MachineState &machineState, const Grbl::PositionMode &positionMode, const Grbl::Position &position) = nullptr;

private:
    Stream *m_stream;
    char m_buffer[Grbl::BUFFER_SIZE];
    char m_tempBuffer[Grbl::TEMP_BUFFER_SIZE];
    uint8_t m_currentIndex;
    mutable Grbl::Position m_currentPosition;
    mutable Grbl::Position m_targetPosition;

    [[nodiscard]] Grbl::MachineState getMachineState(char *state);
    [[nodiscard]] Grbl::PositionMode getPositionMode(char *mode);

    [[nodiscard]] bool processBuffer();
    void processPosition(char *posString);
    [[nodiscard]] bool requestStatusReport();
};
