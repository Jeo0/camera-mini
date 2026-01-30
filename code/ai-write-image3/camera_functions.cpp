#include "globals.h"
#include "camera_functions.h"

// ================= HELPER: ERROR BLINK =================
void blinkError(int times) {
  for(int i=0; i<times; i++) {
    digitalWrite(FLASH_PIN, FLASH_ON); // ON
    delay(20);
    digitalWrite(FLASH_PIN, FLASH_OFF); // OFF
    delay(20);
  }
}
