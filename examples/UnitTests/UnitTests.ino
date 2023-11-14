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

  grbl.onPositionUpdate = [](const Grbl::MachineState machineState,
                             const Grbl::CoordinateMode coordinateMode) {
    Serial.print("Machine state: ");
    Serial.println(grbl.getMachineState(machineState));
    Serial.print("Position mode: ");
    Serial.println(grbl.getCoordinateMode(coordinateMode));
    Serial.print("Position: ");

    for (const auto &coordinate : grbl.getPosition()) {
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

  grbl.setUnitOfMeasurement(Grbl::UnitOfMeasurement::Imperial);
  grbl.setUnitOfMeasurement(Grbl::UnitOfMeasurement::Metric);

  grbl.setPositionMode(Grbl::PositionMode::Absolute);
  grbl.setPositionMode(Grbl::PositionMode::Relative);

  grbl.resetWorkCoordinate();
  grbl.setWorkCoordinate({ { Grbl::Axis::X, 1.234 }, { Grbl::Axis::Y, 56.789 } });

  grbl.linearMoveRapid({ { Grbl::Axis::X, 1.234 }, { Grbl::Axis::Y, 56.789 } });
  grbl.linearMoveFeedRate(200.00, { { Grbl::Axis::X, 1.234 }, { Grbl::Axis::Y, 56.789 } });
  grbl.linearMoveRapidInMachineCoordinate({ { Grbl::Axis::X, 1.234 }, { Grbl::Axis::Y, 56.789 } });

  grbl.jog(100.00, { { Grbl::Axis::X, 0.01 } });

  grbl.test();

  Serial.println("-----------------------");

  delay(5000);
}
