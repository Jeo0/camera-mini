/*
  XIAO ESP32S3 Sense - Low Power Camera Capture
  
  STRATEGY FOR OVERHEATING MITIGATION:
  - Default state: DEEP SLEEP (camera not initialized)
  - Button press (GPIO2 HIGH): Wake → Init camera → Capture → Save → Deinit → Deep sleep
  - Camera only exists during capture (minimal heating)
  
  BUTTON WIRING:
  - Unpressed: GPIO2 = LOW (pulled down)
  - Pressed: GPIO2 = HIGH (3.3V)
  
  Power consumption:
  - Deep sleep: ~44-150µA
  - Active capture: ~200-300mA (brief)
*/

#include <Arduino.h>
#include <esp_camera.h>
#include <esp_sleep.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

#define CAMERA_MODEL_XIAO_ESP32S3
#include "camera_pins.h"

// ================= CONFIG =================
#define BUTTON_PIN      2     // GPIO2 - button input
#define SD_CS_PIN       21    // SD card CS pin
// ==========================================

RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR int fileCount = 1;

// ---------------- CAMERA FUNCTIONS ----------------

bool initCamera() {
  camera_config_t config;
  
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  
  config.pin_xclk  = XCLK_GPIO_NUM;
  config.pin_pclk  = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href  = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn  = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size   = FRAMESIZE_UXGA;  // High quality
  config.jpeg_quality = 10;
  config.fb_count     = 1;
  config.fb_location  = CAMERA_FB_IN_PSRAM;
  config.grab_mode    = CAMERA_GRAB_WHEN_EMPTY;
  
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("❌ Camera init failed: 0x%x\n", err);
    return false;
  }
  
  Serial.println("✅ Camera initialized");
  return true;
}

void deinitCamera() {
  esp_camera_deinit();
  Serial.println("🔌 Camera deinitialized");
}

bool captureAndSave() {
  // Capture image
  Serial.println("📸 Capturing image...");
  camera_fb_t *fb = esp_camera_fb_get();
  
  if (!fb) {
    Serial.println("❌ Capture failed");
    return false;
  }
  
  Serial.printf("✅ Captured %u bytes\n", fb->len);
  
  // Initialize SD card
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("❌ SD init failed");
    esp_camera_fb_return(fb);
    return false;
  }
  
  // Create filename with counter
  char filename[32];
  sprintf(filename, "/photo_%03d.jpg", fileCount);
  
  // Write to SD card
  File file = SD.open(filename, FILE_WRITE);
  if (!file) {
    Serial.println("❌ File open failed");
    esp_camera_fb_return(fb);
    return false;
  }
  
  file.write(fb->buf, fb->len);
  file.close();
  
  Serial.printf("💾 Saved: %s\n", filename);
  
  // Release frame buffer
  esp_camera_fb_return(fb);
  
  // Increment counter for next boot
  fileCount++;
  
  return true;
}

// ---------------- SETUP ----------------

void setup() {
  Serial.begin(115200);
  delay(500);
  
  bootCount++;
  Serial.printf("\n🔔 Boot #%d\n", bootCount);
  
  // Check PSRAM
  if (!psramFound()) {
    Serial.println("❌ PSRAM not found!");
    while (1);
  }
  
  // Configure button pin
  pinMode(BUTTON_PIN, INPUT);
  
  // Check why we woke up
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  
  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
    Serial.println("⚡ Woke from button press");
    
    // Wait for button to be actually HIGH (debounce)
    delay(50);
    
    if (digitalRead(BUTTON_PIN) == HIGH) {
      Serial.println("🎯 Button confirmed HIGH - starting capture");
      
      // Initialize camera
      if (initCamera()) {
        // Small delay for camera to stabilize
        delay(100);
        
        // Capture and save
        captureAndSave();
        
        // Deinitialize camera (important for power saving!)
        deinitCamera();
      }
      
      // Wait for button release
      Serial.println("⏳ Waiting for button release...");
      while (digitalRead(BUTTON_PIN) == HIGH) {
        delay(50);
      }
      Serial.println("✅ Button released");
    }
    
  } else {
    Serial.println("🌅 First boot or reset");
  }
  
  // Configure wake-up on GPIO2 HIGH
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_2, 1);  // 1 = wake on HIGH
  
  Serial.println("😴 Entering deep sleep...");
  Serial.println("Press button to capture\n");
  Serial.flush();
  delay(100);
  
  // Enter deep sleep
  esp_deep_sleep_start();
}

// ---------------- LOOP ----------------
// Never reached because of deep sleep
void loop() {
  // Not used
}