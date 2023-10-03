#include "GrblInterface.h"

GrblInterface grblInterface(Serial2);

void setup() {
  Serial.begin(115200);
  Serial2.setTX(4);
  Serial2.setRX(5);
  Serial2.begin(115200);

  while (!Serial) {}

  grblInterface.onPositionUpdated = [](const Grbl::MachineState &machineState, const Grbl::PositionMode &positionMode, const Grbl::Position &position) {
    Serial.print("Machine state:");
    Serial.println(machineState.name());
    Serial.print("Position mode:");
    Serial.println(positionMode.name());
    Serial.print("Position: ");

    for (const auto &axis : Grbl::AXES) {
      Serial.print(position.getValue(*axis));
      Serial.print(' ');
    }

    Serial.println();
  };

  grblInterface.setPosition(Grbl::Axes::X, 100);
  grblInterface.setPosition(Grbl::Axes::Y, 50);
  grblInterface.sendLinearMove();
}

void loop() {
  auto updated = grblInterface.update();
}
