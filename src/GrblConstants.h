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

    enum class Plane
    {
        XY,
        ZX,
        YZ,
    };

    enum class Alarm
    {
        None,
        HardLimit,                          // 1: Hard limit Hard limit has been triggered. You must send the reset command 0X18 or ctrl+X from the keyboard on FluidTerm. It may be a special button on your gcode sender. Machine position is likely lost due to sudden halt. Re-homing is highly recommended.
        SoftLimit,                          // 2: Soft limit Soft limit alarm. G-code motion target exceeds machine travel. Machine position retained. Alarm may be safely unlocked.
        AbortDuringCycle,                   // 3: Abort during cycle Reset while in motion. Machine position is likely lost due to sudden halt. Re-homing is highly recommended.
        ProbeFailNotInExpectedInitialState, // 4: Probe fail Probe fail. Probe is not in the expected initial state before starting probe cycle when G38.2 and G38.3 is not triggered and G38.4 and G38.5 is triggered.
        ProbeFailNoContact,                 // 5: Probe fail Probe fail. Probe did not contact the workpiece within the programmed travel for G38.2 and G38.4
        HomingFailCycleReset,               // 6: Homing fail Homing fail. The active homing cycle was reset.
        HomingFailSafetyDoorOpened,         // 7: Homing fail Homing fail. Safety door was opened during homing cycle.
        HomingFailLimitSwitchNotCleared,    // 8: Homing fail Homing fail. Pull off travel failed to clear limit switch. Try increasing pull-off setting or check wiring.
        HomingFailLimitSwitchNotFound,      // 9: Homing fail Homing fail. Could not find limit switch within search distances. Try increasing max travel, decreasing pull-off distance, or check wiring.
        SpindleControl,                     // 10: Spindle Control @ On GRBL, this could also indicate homing failo on dual axis machines when it could not find the second limit switch for self-squaring.
        ControlPin,                         // 11: Control Pin
        AmbiguousSwitch,                    // 12: Ambiguous Switch There is a limit switch active, but FluidNC does not have enough info to clear the switch. See this.
        HardStop,                           // 13: Hard Stop
        Unhomed,                            // 14: Unhomed Your machine needs to be homed. See the must_home item in the config file. You home with $H. You can clear the error with $Alarm/Disable or $X.
        Init                                // 15 Init
    };

    enum class Error
    {
        None,
        ExpectedGCodeCommandLetter,              // 1: G-code words consist of a letter and a value. Letter was not found.
        BadGCodeNumberFormat,                    // 2: Numeric value format is not valid or missing an expected value.
        InvalidGrblStatement,                    // 3: Grbl '$' system command was not recognized or supported.
        NegativeValue,                           // 4: Negative value received for an expected positive value.
        SettingDisabled,                         // 5: Homing cycle is not enabled via settings.
        StepPulseTooShort,                       // 6: Minimum step pulse time must be greater than 3usec.
        FailedToReadSettings,                    // 7: EEPROM read failed. Reset and restored to default values.
        CommandRequiresIdleState,                // 8: Grbl '$' command cannot be used unless Grbl is IDLE. Ensures smooth operation during a job.
        GCodeCannotBeExecutedInLockOrAlarmState, // 9: G-code locked out during alarm or jog state
        SoftLimitError,                          // 10: Soft limits cannot be enabled without homing also enabled.
        LineTooLong,                             // 11: Max characters per line exceeded. Line was not processed and executed.
        MaxStepRateExceeded,                     // 12: (Compile Option) Grbl '$' setting value exceeds the maximum step rate supported.
        CheckDoor,                               // 13: Safety door detected as opened and door state initiated.
        StartupLineTooLong,                      // 14: (Grbl-Mega Only) Build info or startup line exceeded EEPROM line length limit.
        MaxTravelExceededDuringJog,              // 15: Jog target exceeds machine travel. Command ignored.
        InvalidJogCommand,                       // 16: Jog command with no '=' or contains prohibited g-code.
        LaserModeRequiresPwmOutput,              // 17: Laser mode requires PWM output.
        NoHoming,                                // 18: No Homing/Cycle defined in settings.
        SingleAxisHomingNotAllowed,              // 19: Single axis homing not allowed.
        UnsupportedGCodeCommand,                 // 20: Unsupported or invalid g-code command found in block.
        GCodeModalGroupViolation,                // 21: More than one g-code command from same modal group found in block.
        GCodeUndefinedFeedRate,                  // 22: Feed rate has not yet been set or is undefined.
        GCodeCommandValueNotInteger,             // 23: G-code command in block requires an integer value.
        GCodeAxisCommandConflict,                // 24: Two G-code commands that both require the use of the XYZ axis words were detected in the block.
        GCodeWordRepeated,                       // 25: A G-code word was repeated in the block.
        GCodeNoAxisWords,                        // 26: A G-code command implicitly or explicitly requires XYZ axis words in the block, but none were detected.
        GCodeInvalidLineNumber,                  // 27: N line number value is not within the valid range of 1 - 9,999,999.
        GCodeValueWordMissing,                   // 28: A G-code command was sent, but is missing some required P or L value words in the line.
        GCodeUnsupportedCoordinateSystem,        // 29: Grbl supports six work coordinate systems G54-G59. G59.1, G59.2, and G59.3 are not supported.
        GCodeG53InvalidMotionMode,               // 30: The G53 G-code command requires either a G0 seek or G1 feed motion mode to be active. A different motion was active.
        GCodeExtraAxisWords,                     // 31: There are unused axis words in the block and G80 motion mode cancel is active.
        GCodeNoAxisWordsInPlane,                 // 32: A G2 or G3 arc was commanded but there are no XYZ axis words in the selected plane to trace the arc.
        GCodeInvalidTarget,                      // 33: The motion command has an invalid target. G2, G3, and G38.2 generates this error, if the arc is impossible to generate or if the probe target is the current position.
        GCodeArcRadiusError,                     // 34: A G2 or G3 arc, traced with the radius definition, had a mathematical error when computing the arc geometry. Try either breaking up the arc into semi-circles or quadrants, or redefine them with the arc offset definition.
        GCodeNoOffsetsInPlane,                   // 35: A G2 or G3 arc, traced with the offset definition, is missing the IJK offset word in the selected plane to trace the arc.
        GCodeUnusedWords,                        // 36: There are unused, leftover G-code words that aren't used by any command in the block.
        GCodeG43DynamicAxisError,                // 37: The G43.1 dynamic tool length offset command cannot apply an offset to an axis other than its configured axis. The Grbl default axis is the Z-axis.
        GCodeMaxValueExceeded,                   // 38: Tool number greater than max supported value.
        PParamMaxExceeded,                       // 39: P param max exceeded
        CheckControlPins,                        // 40: Control pins cannot be active at startup
        FailedToMountDevice = 60,                // 60: Failed to mount device
        ReadFailed = 61,                         // 61: Read failed
        FailedToOpenDirectory = 62,              // 62: Failed to open directory
        DirectoryNotFound = 63,                  // 63: Directory not found
        FileEmpty = 64,                          // 64: File empty
        FileNotFound = 65,                       // 65: File not found
        FailedToOpenFile = 66,                   // 66: Failed to open file
        DeviceIsBusy = 67,                       // 67: Device is busy
        FailedToDeleteDirectory = 68,            // 68: Failed to delete directory
        FailedToDeleteFile = 69,                 // 69: Failed to delete file
        BluetoothFailedToStart = 70,             // 70: Bluetooth failed to start
        WiFiFailedToStart = 71,                  // 71: WiFi failed to start
        NumberOutOfRangeForSetting = 80,         // 80: Number out of range for setting
        InvalidValueForSetting = 81,             // 81: Invalid value for setting
        FailedToCreateFile = 82,                 // 82: Failed to create file
        FailedToSendMessage = 90,                // 90: Failed to send message
        FailedToStoreSetting = 100,              // 100: Failed to store setting
        FailedToGetSettingStatus = 101,          // 101: Failed to get setting status
        AuthenticationFailed = 110,              // 110: Authentication failed!
        EndOfLine = 111,                         // 111: End of line
        EndOfFile = 112,                         // 112: End of file
        AnotherInterfaceIsBusy = 120,            // 120: Another interface is busy
        JogCancelled = 130,                      // 130: Jog Cancelled
        BadPinSpecification = 150,               // 150: Bad Pin Specification
        ConfigurationIsInvalid = 152,            // 152: Configuration is invalid. Check boot messages for ERR's.
        FileUploadFailed = 160,                  // 160: File Upload Failed
        FileDownloadFailed = 161                 // 161: File Download Failed
    };
}