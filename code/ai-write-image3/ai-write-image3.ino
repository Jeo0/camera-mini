#include "globals.h"
#include "camera_helpers.h"
#include "camera_functions.h"



void setup() {
  // Initialize LED
  pinMode(FLASH_PIN, OUTPUT);
  digitalWrite(FLASH_PIN, HIGH); // Ensure OFF initially

  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
    // Immediate capture
    captureSequence();
    
    // Safety Wait: Ensure button is released before sleeping
    // If we don't do this, the board will wake up again immediately
    pinMode(BUTTON_PIN, INPUT); 
    while(digitalRead(BUTTON_PIN) == HIGH) { 
        delay(10); 
    }
  }

  // Configure Wakeup
  esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_PIN, 1); // Wake on HIGH

  // Go to sleep
  esp_deep_sleep_start();
}

void loop() {

}
