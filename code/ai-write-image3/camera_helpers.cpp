#include "camera_helpers.h"
// ================= CAMERA & SD =================
bool initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_UXGA; // 1600x1200
  
  // OPTIMIZATION 1: Lower quality slightly for faster write speed
  // 10 = Best/Slowest, 12 = Great/Faster, 15 = Good/Fastest
  config.jpeg_quality = 4; 
  
  config.fb_count = 1;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  
  if(PWDN_GPIO_NUM != -1) {
    pinMode(PWDN_GPIO_NUM, OUTPUT);
    digitalWrite(PWDN_GPIO_NUM, LOW); 
  }

  esp_err_t err = esp_camera_init(&config);
  return (err == ESP_OK);
}

void captureSequence() {
  // LED ON (Indicates "Shutter Open / Busy")
  digitalWrite(FLASH_PIN, LOW); 

  // OPTIMIZATION 2: Increase SD SPI Speed to 16MHz (Default is 4MHz)
  // This drastically reduces the time spent saving the file.
  if (!SD.begin(SD_CS_PIN, SPI, 16000000)) {
    blinkError(5);
    return;
  }

  if (!initCamera()) {
    blinkError(5);
    return;
  }

  // OPTIMIZATION 3: Reduced Warmup
  // 1 Frame is usually enough to clear the black buffer on OV2640
  // Reduced delay to 20ms
  camera_fb_t * warmup_fb = esp_camera_fb_get();
  esp_camera_fb_return(warmup_fb);
  delay(20);

  // Capture
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    blinkError(5);
    return;
  }

  // Filename logic
  // NOTE: If you remove the battery, fileCount resets to 1.
  // The loop below will find the next gap, but it takes time if you have 1000 photos.
  char filename[32];
  while (true) {
    sprintf(filename, "/image_%03d.jpg", fileCount);
    if (SD.exists(filename)) {
      fileCount++;
    } else {
      break;
    }
  }

  // Save
  File file = SD.open(filename, FILE_WRITE);
  if (file) {
    file.write(fb->buf, fb->len);
    file.close();
    fileCount++;
  } else {
    blinkError(2);
  }

  esp_camera_fb_return(fb);
  
  // Turn LED OFF (Capture Complete)
  digitalWrite(FLASH_PIN, HIGH);
  delay(50);
  
  // Power down camera immediately
  esp_camera_deinit();
  if(PWDN_GPIO_NUM != -1) digitalWrite(PWDN_GPIO_NUM, HIGH); 
}

