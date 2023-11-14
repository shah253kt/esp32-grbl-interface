#include "GrblInterface.h"

GrblInterface grbl(Serial);

void setup() {
  Serial.begin(115200);
}

void loop() {
  grbl.setUnitOfMeasurement(Grbl::UnitOfMeasurement::Imperial);
  grbl.setUnitOfMeasurement(Grbl::UnitOfMeasurement::Metric);

  grbl.setPositionMode(Grbl::PositionMode::Absolute);
  grbl.setPositionMode(Grbl::PositionMode::Relative);

  grbl.resetWorkCoordinate();
  grbl.setWorkCoordinate({ { Grbl::Axis::X, 1.234 }, { Grbl::Axis::Y, 56.789 } });

  grbl.linearMoveRapid({ { Grbl::Axis::X, 1.234 }, { Grbl::Axis::Y, 56.789 } });
  grbl.linearMoveFeedRate(200.00, { { Grbl::Axis::X, 1.234 }, { Grbl::Axis::Y, 56.789 } });

  delay(5000);
}
