#include "globals.h"
#include "camera_state_machine.h"
#include "camera_states.h"
// #include "camera_helpers.h"
// #include "camera_functions.h"
// #define _WITH_RECORD_STATE_    // turn this on when you have the record (implementation is still unstable 
                                  // as the termination logic requires you to hold the button)

CameraContext* mainCamera = nullptr;

void setup() {
  Serial.begin(115200);

  pinMode(FLASH_PIN, OUTPUT);
  digitalWrite(FLASH_PIN, FLASH_OFF);

  // initialization of state machine
  // DecisionState enter()
  mainCamera = new CameraContext();
}

void loop() {
#ifdef _WITH_RECORD_STATE_
  // im removing the record feature
  // tick state machine
  if(mainCamera) {
    mainCamera->update();
  }
#endif

  delay(1);
}
