#include "GrblInterface.h"

#define GRBL_SERIAL Serial2
#define GRBL_RX 16
#define GRBL_TX 17
#define GRBL_BAUD_RATE 115200

GrblInterface grblInterface(GRBL_SERIAL);

void setup() {
  Serial.begin(115200);
  GRBL_SERIAL.begin(GRBL_BAUD_RATE, SERIAL_8N1, GRBL_RX, GRBL_TX);

  while (!Serial) {}

  grblInterface.onPositionUpdate = [](const Grbl::MachineState &machineState,
                                      const Grbl::CoordinateMode &coordinateMode) {
    Serial.print("Machine state:");
    Serial.println(grblInterface.getMachineState(machineState));
    Serial.print("Position mode:");
    Serial.println(grblInterface.getCoordinateMode(coordinateMode));
    Serial.print("Position: ");

    for (const auto &axisPosition : grblInterface.getPosition()) {
      Serial.print(axisPosition);
      Serial.print(' ');
    }

    Serial.println();
  };

  // grblInterface.setPosition(Grbl::Axes::X, 100);
  // grblInterface.setPosition(Grbl::Axes::Y, 50);
  // grblInterface.sendLinearMove();
}

void loop() {
  grblInterface.update();
}
