/*
  XIAO ESP32S3 Sense Camera with Microphone Demo
  xiao-camera-mic-demo.ino
  Tests onboard Camera, MEMS Microphone, and MicroSD Card
  Takes a picture and a 10-second recording when Touch Switch is pressed
  Saves to MicroSD card in JPG & WAV format
  
  DroneBot Workshop 2023
  https://dronebotworkshop.com
*/
 
#include "globals.h"
#include "operations.h"
 
 
 
void setup() {
  // Start Serial Monitor, wait until port is ready
  Serial.begin(115200);
  while (!Serial)
    ;
 
  // I2S Setup and start
  I2S.setAllPins(-1, 42, 41, -1, -1);
  if (!I2S.begin(PDM_MONO_MODE, SAMPLE_RATE, SAMPLE_BITS)) {
    Serial.println("Failed to initialize I2S!");
    while (1)
      ;
  }
 
  // Attach touch switch to interrupt handler
  touchAttachInterrupt(T1, gotTouch1, threshold);
 
  // Define Camera Parameters and Initialize
  CameraParameters();
 
  // Camera is good, set status
  camera_status = true;
  Serial.println("Camera OK!");
 
  // Initialize the MicroSD card
  if (!SD.begin(21)) {
    Serial.println("Failed to mount MicroSD Card!");
    while (1)
      ;
  }
 
  // Determine what type of MicroSD card is mounted
  uint8_t cardType = SD.cardType();
 
  if (cardType == CARD_NONE) {
    Serial.println("No MicroSD card inserted!");
    return;
  }
 
  // Print card type
  Serial.print("MicroSD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
  // MicroSD is good, set status
  sd_status = true;
}
 
void loop() {
 
  // Make sure the camera and MicroSD are ready
  if (camera_status && sd_status) {
 
    // See if the touch switch has been selected
    if (touch1detected) {
      // Take picture and record sound
      if (touchInterruptGetLastStatus(T1)) {
        Serial.println("Touched, take a picture");
        // Create image file name
        char imageFileName[32];
        sprintf(imageFileName, "/image%d.jpg", fileCount);
 
        // Take a picture
        photo_save(imageFileName);
        Serial.printf("Saving picture: %s\r\n", imageFileName);
 
        // Create audio file name
        Serial.print("Recording ");
        Serial.print(RECORD_TIME);
        Serial.println(" seconds of sound");
        char audioFileName[32];
        sprintf(audioFileName, "/image%d.wav", fileCount);
 
        // Record audio from microphone
        record_wav(audioFileName);
        Serial.printf("Saving sound: %s\r\n", audioFileName);
 
        // Increment file counter
        fileCount++;
      }
      // Reset the touch variable
      touch1detected = false;
    }
    delay(80);
  }
}
