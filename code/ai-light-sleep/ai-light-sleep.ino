/*
  XIAO ESP32S3 Sense
  GPIO2-controlled Camera ON/OFF with Light Sleep
*/

#include <Arduino.h>
#include <esp_camera.h>
#include <esp_sleep.h>

// Camera pin map
#define CAMERA_MODEL_XIAO_ESP32S3
#include "camera_pins.h"

// ===================== USER CONFIG =====================
#define CAM_ENABLE_PIN 2        // HIGH = camera ON, LOW = camera OFF
#define XCLK_FREQ_HZ 20000000   // Camera clock
// ======================================================

// Camera state flags
volatile bool camToggleRequest = false;
bool cameraActive = false;

// ISR — only sets a flag
void IRAM_ATTR camEnableISR() {
  camToggleRequest = true;
}

// Camera configuration
camera_config_t camConfig;

void setupCameraConfig() {
  camConfig.ledc_channel = LEDC_CHANNEL_0;
  camConfig.ledc_timer   = LEDC_TIMER_0;

  camConfig.pin_d0 = Y2_GPIO_NUM;
  camConfig.pin_d1 = Y3_GPIO_NUM;
  camConfig.pin_d2 = Y4_GPIO_NUM;
  camConfig.pin_d3 = Y5_GPIO_NUM;
  camConfig.pin_d4 = Y6_GPIO_NUM;
  camConfig.pin_d5 = Y7_GPIO_NUM;
  camConfig.pin_d6 = Y8_GPIO_NUM;
  camConfig.pin_d7 = Y9_GPIO_NUM;

  camConfig.pin_xclk  = XCLK_GPIO_NUM;
  camConfig.pin_pclk  = PCLK_GPIO_NUM;
  camConfig.pin_vsync = VSYNC_GPIO_NUM;
  camConfig.pin_href  = HREF_GPIO_NUM;
  camConfig.pin_sscb_sda = SIOD_GPIO_NUM;
  camConfig.pin_sscb_scl = SIOC_GPIO_NUM;
  camConfig.pin_pwdn  = PWDN_GPIO_NUM;
  camConfig.pin_reset = RESET_GPIO_NUM;

  camConfig.xclk_freq_hz = XCLK_FREQ_HZ;
  camConfig.pixel_format = PIXFORMAT_JPEG;
  camConfig.frame_size   = FRAMESIZE_SVGA;
  camConfig.jpeg_quality = 12;
  camConfig.fb_count     = 1;
  camConfig.fb_location  = CAMERA_FB_IN_PSRAM;
  camConfig.grab_mode    = CAMERA_GRAB_WHEN_EMPTY;
}

void powerDownCamera() {
#ifdef PWDN_GPIO_NUM
  pinMode(PWDN_GPIO_NUM, OUTPUT);
  digitalWrite(PWDN_GPIO_NUM, HIGH);  // Sensor power down
#endif
}

void powerUpCamera() {
#ifdef PWDN_GPIO_NUM
  pinMode(PWDN_GPIO_NUM, OUTPUT);
  digitalWrite(PWDN_GPIO_NUM, LOW);   // Sensor power up
  delay(10);
#endif
}

void turnCameraOn() {
  Serial.println("📷 Turning camera ON");

  powerUpCamera();
  setupCameraConfig();

  esp_err_t err = esp_camera_init(&camConfig);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x\n", err);
    return;
  }

  cameraActive = true;
  Serial.println("📷 Camera ACTIVE");
}

void turnCameraOff() {
  Serial.println("📷 Turning camera OFF");

  esp_camera_deinit();
  powerDownCamera();

  cameraActive = false;
  Serial.println("📷 Camera OFF");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(CAM_ENABLE_PIN, INPUT_PULLDOWN);
  attachInterrupt(CAM_ENABLE_PIN, camEnableISR, CHANGE);

  // Initial state check
  if (digitalRead(CAM_ENABLE_PIN)) {
    turnCameraOn();
  } else {
    powerDownCamera();
    Serial.println("Camera starts OFF");
  }
}

void loop() {

  // Handle GPIO-triggered state change
  if (camToggleRequest) {
    camToggleRequest = false;

    bool wantCamera = digitalRead(CAM_ENABLE_PIN);

    if (wantCamera && !cameraActive) {
      turnCameraOn();
    }
    else if (!wantCamera && cameraActive) {
      turnCameraOff();
    }
  }

  // If camera is OFF, enter light sleep
  if (!cameraActive) {
    Serial.println("😴 Entering light sleep");
    delay(50);

    esp_sleep_enable_ext0_wakeup(
      (gpio_num_t)CAM_ENABLE_PIN, 1  // Wake on HIGH
    );

    esp_light_sleep_start();
    Serial.println("⚡ Woke up");
  }

  delay(100);
}
