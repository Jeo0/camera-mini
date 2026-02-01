#pragma once
#include <Preferences.h>
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




// ================= DEBUG =================
#define _DEBUG_PRINT_SHOW_
// #define _WITH_RECORD_STATE_    // turn this on when you have the record (implementation is still unstable 
                                  // as the termination logic requires you to hold the button)



// ================= GLOBAL CONSTANTS =================
// survives deep sleep, resets on power loss
extern uint32_t g_imageCounter;
extern Preferences g_prefs;


// used for error blinks to be used with the function 
// blinkError 
#define BLINK_TIMES_ERROR_CAMERA     7
#define BLINK_TIMES_ERROR_SDCARD    3
#define BLINK_MILLI_DURATION_MINOR  50
#define BLINK_MILLI_DURATION_MAJOR  100

// sd card speed 16MHZ to 4 MHZ 
// #define SD_SPI_SPEED    4000000
#define SD_SPI_SPEED    16000000
#define XCLK_FREQ_HZ    16000000// 20000000     // 10MHz is too nice (no streaks of lines), 20MHz has too many; now choose 15MHz

// camera settings:
#define JPEG_QUALITY    24
