/*
Sample Grbl strings:
<Idle|MPos:0.000,0.000,0.000|FS:0,0>
<Run|MPos:5.529,0.560,7.000|FS:1200,8000>
<Hold|MPos:10.123,20.456,30.789|FS:0,0|Ov:100,100,100>
<Jog|MPos:-5.000,-5.000,-5.000|FS:1000,8000>
<Alarm|MPos:0.000,0.000,0.000|FS:0,0>
<Idle|MPos:5.529,0.560,7.000|FS:400,1000|WCO:1.234,5.678,9.123>
*/

#include "GrblInterface.h"

GrblInterface grbl(Serial);

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200);

  grbl.onPositionUpdate = [](const Grbl::MachineState machineState,
                             const Grbl::CoordinateMode coordinateMode) {
    Serial.print("Machine state: ");
    Serial.println(grbl.getMachineState(machineState));

    Serial.print("Position mode: ");
    Serial.println(grbl.getCoordinateMode(coordinateMode));

    Serial.print("Work coordinate: ");
    for (const auto &coordinate : grbl.getWorkCoordinate()) {
      Serial.print(coordinate);
      Serial.print(' ');
    }

    Serial.print("Machine coordinate: ");
    for (const auto &coordinate : grbl.getMachineCoordinate()) {
      Serial.print(coordinate);
      Serial.print(' ');
    }

    Serial.print("\nWork Coordinate Offset: ");
    for (const auto &coordinate : grbl.getWorkCoordinateOffset()) {
      Serial.print(coordinate);
      Serial.print(' ');
    }

    Serial.print("\nCurrent feed rate: ");
    Serial.println(grbl.getCurrentFeedRate());

    Serial.print("Current spindle speed: ");
    Serial.println(grbl.getCurrentSpindleSpeed());

    Serial.println();
  };
}

void loop() {
  grbl.update();

  // G-codes
  grbl.setUnitOfMeasurement(Grbl::UnitOfMeasurement::Inches);
  grbl.setUnitOfMeasurement(Grbl::UnitOfMeasurement::Millimeters);

  grbl.setDistanceMode(Grbl::DistanceMode::Absolute);
  grbl.setDistanceMode(Grbl::DistanceMode::Incremental);

  grbl.clearCoordinateOffset();
  grbl.setCoordinateOffset({ { Grbl::Axis::X, 1.234 }, { Grbl::Axis::Y, 56.789 } });

  grbl.linearRapidPositioning({ { Grbl::Axis::X, 1.234 }, { Grbl::Axis::Y, 56.789 } });
  grbl.linearInterpolationPositioning(200.00, { { Grbl::Axis::X, 1.234 }, { Grbl::Axis::Y, 56.789 } });
  grbl.linearPositioningInMachineCoordinate({ { Grbl::Axis::X, 1.234 }, { Grbl::Axis::Y, 56.789 } });

  grbl.arcInterpolationPositioning(Grbl::ArcMovement::Clockwise, { { Grbl::Axis::X, 10.00 }, { Grbl::Axis::Y, 10.00 } }, 5.00, 10.00);
  grbl.arcInterpolationPositioning(Grbl::ArcMovement::CounterClockwise, { { Grbl::Axis::X, 10.00 }, { Grbl::Axis::Y, 10.00 } }, { 5.0, 5.0 }, 10.00);

  grbl.dwell(3000);

  grbl.setCoordinateSystemOrigin(Grbl::CoordinateOffset::Absolute, Grbl::CoordinateSystem::P1, { { Grbl::Axis::X, 0.00 }, { Grbl::Axis::Y, 0.00 }, { Grbl::Axis::Z, 0.00 } });
  grbl.setCoordinateSystemOrigin(Grbl::CoordinateOffset::Relative, Grbl::CoordinateSystem::P2, { { Grbl::Axis::X, 0.00 }, { Grbl::Axis::Y, 0.00 }, { Grbl::Axis::Z, 0.00 } });

  // M-codes
  grbl.spindleOn();
  grbl.spindleOn(RotationDirection::CounterClockwise);
  grbl.spindleOff();
  
  // $ commands
  grbl.runHomingCycle();
  grbl.clearAlarm();
  grbl.jog(100.00, { { Grbl::Axis::X, 0.01 } });

  grbl.test();

  Serial.println("-----------------------");

  delay(5000);
}
