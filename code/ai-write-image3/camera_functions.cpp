#include "globals.h"
#include "camera_functions.h"

// ================= HELPER: ERROR BLINK =================
void blinkError(int times) {
  for(int i=0; i<times; i++) {
    digitalWrite(FLASH_PIN, LOW); // ON
    delay(20);
    digitalWrite(FLASH_PIN, HIGH); // OFF
    delay(20);
  }
}
