
// Include required libraries
#include <Arduino.h>
#include <esp_camera.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <I2S.h>

 
// Define camera model & pinout
#define CAMERA_MODEL_XIAO_ESP32S3  // Has PSRAM
#include "camera_pins.h"

// Audio record time setting (in seconds, max value 240)
#define RECORD_TIME 10
 
// Audio settings
#define SAMPLE_RATE 16000U
#define SAMPLE_BITS 16
#define WAV_HEADER_SIZE 44
#define VOLUME_GAIN 3
 
// Camera status variable
bool camera_status = false;
 
// MicroSD status variable
bool sd_status = false;
 
// File Counter
int fileCount = 1;
 
// Touch Switch variables
int threshold = 1500;  // Adjust if not responding properly
bool touch1detected = false;
