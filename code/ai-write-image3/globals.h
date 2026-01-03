#pragma once
#include <Arduino.h>
#include <esp_camera.h>
#include <esp_sleep.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

// ================= PINS =================
#define CAMERA_MODEL_XIAO_ESP32S3
#include "camera_pins.h"

#define BUTTON_PIN      2     // D1 on board
#define FLASH_PIN       3     // D2 
#define SD_CS_PIN       21    // Internal
// #define LED_PIN         21    // Internal

RTC_DATA_ATTR int fileCount = 1; 

