#include "globals.h"
#include "camera_functions.h"

// ================= HELPER: ERROR BLINK =================
// convention, around 20-70ms delay
void blinkError(int p_times, int p_milli_duration) {
  for(int i=0; i<p_times; i++) {
    digitalWrite(FLASH_PIN, FLASH_ON); // ON
    delay(p_milli_duration);
    digitalWrite(FLASH_PIN, FLASH_OFF); // OFF
    delay(p_milli_duration);
  }
}
