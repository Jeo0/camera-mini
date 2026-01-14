#include "globals.h"
#include "camera_state_machine.h"
#include "camera_states.h"
// #include "camera_helpers.h"
// #include "camera_functions.h"

CameraContext* mainCamera = nullptr;

void setup() {
  Serial.begin(115200);

  pinMode(FLASH_PIN, OUTPUT);
  digitalWrite(FLASH_PIN, HIGH);

  // initialization of state machine
  // DecisionState enter()
  mainCamera = new CameraContext();
}

void loop() {
  // tick state machine
  if(mainCamera) {
    mainCamera->update();
  }

  delay(1);
}
