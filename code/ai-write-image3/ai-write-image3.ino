#include "globals.h"
#include "camera_state_machine.h"
#include "camera_states.h"
// #include "camera_helpers.h"
// #include "camera_functions.h"

CameraContext* mainCamera = nullptr;

void setup() {
#ifdef _DEBUG_PRINT_SHOW_
  Serial.begin(115200);
#endif

  pinMode(FLASH_PIN, OUTPUT);
  digitalWrite(FLASH_PIN, FLASH_OFF);

  // initialization of state machine
  // DecisionState enter()
  mainCamera = new CameraContext();
  if(mainCamera) 
    mainCamera->update();
}

void loop() {
  // this is effectively not ran
  // im removing the record feature
  // tick state machine
  if(mainCamera) {
    mainCamera->update();
  }

  delay(1);
}
