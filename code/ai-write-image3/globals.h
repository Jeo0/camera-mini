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
#define FLASH_OFF       HIGH
#define FLASH_ON        LOW

// #define MODE_PIN        5     // D4
// #define REDFLASH_PIN    6     // D5

#define SD_CS_PIN       21    // Internal
// #define LED_PIN         21    // Internal


// used for error blinks to be used with the function 
// blinkError 
#define BLINK_TIMES_ERROR_CAMERA    5
#define BLINK_TIMES_ERROR_SDCARD    3
#define BLINK_MILLI_DURATION_MINOR  20
#define BLINK_MILLI_DURATION_MAJOR  70

// sd card speed 16MHZ to 4 MHZ 
#define SD_SPI_SPEED    4000000
//#define SD_SPI_SPEED    16000000
