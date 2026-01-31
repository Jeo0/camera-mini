#include "camera_helpers.h"
#include "camera_functions.h"

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
    config.fb_count = 1; // 
    config.fb_location = CAMERA_FB_IN_PSRAM; // 
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY; // 


    config.frame_size = FRAMESIZE_UXGA; // 1600x1200
    // SVGA = 800x600

    // Lower quality slightly for faster write speed
    // 10 = Best/Slowest, 12 = Great/Faster, 15 = Good/Fastest
    config.jpeg_quality = 20; //  


    if(PWDN_GPIO_NUM != -1) {
        pinMode(PWDN_GPIO_NUM, OUTPUT);
        digitalWrite(PWDN_GPIO_NUM, LOW); 
    }

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        char e_buffer[150] {};
        snprintf(e_buffer, sizeof(e_buffer), "Camera init failed with error 0x%x", err);
        Serial.printf("%s\n", e_buffer);
        return false;
    }
    cameraSettingsInit();
    // we turn off this SD card, so we can still use it even we do not have any thing inserted
    // initialize SD card 
    /*
    if (!SD.begin(SD_PIN_CS)) {
        Serial.println("SD card initialization failed!");
        blinkError(BLINK_TIMES_ERROR_SDCARD, BLINK_MILLI_DURATION_MAJOR);
        return;
    }

    uint8_t cardType = SD.cardType();

    // Determine if the type of SD card is available
    if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
        // return;
    }

    Serial.print("SD Card Type: ");
      if(cardType == CARD_MMC){
        Serial.println("MMC");
      } else if(cardType == CARD_SD){
        Serial.println("SDSC");
      } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
      } else {
        Serial.println("UNKNOWN");
      }
    */

    // once all is good
    return true;
}

void createNextFilename(char* p_buffer, const char* p_prefix, const char* p_extension){
    int iii_count = 1;
    while (true) {
        sprintf(p_buffer, "/%s%03d.%s", p_prefix, iii_count, p_extension);
        if (!SD.exists(p_buffer)) {
            break; 
        }
        iii_count++;
    }
}

void cameraSettingsInit() {
    // changing settings of the camera 
    // ----- see the camera settings here: 
    // https://randomnerdtutorials.com/esp32-cam-ov2640-camera-settings/

    sensor_t * camera_settings = esp_camera_sensor_get();
    // 0=disable, 1=enable
    camera_settings->set_hmirror(camera_settings, 0);       // so we can read whatevers being outputted there // left-right swap
    camera_settings->set_vflip(camera_settings, 1);         // up-down swap
    camera_settings->set_whitebal(camera_settings, 1);     // if 0, colors may look weird when lighting changes
    camera_settings->set_awb_gain(camera_settings, 1);      // automatic white balance
    camera_settings->set_wb_mode(camera_settings, 0);       // 0=auto, 1=sunny, 2=cloudy, 3=office, 4=home
}


/*
void captureSequence() {
    // LED ON (Indicates "Shutter Open / Busy")
    digitalWrite(FLASH_PIN, LOW); 

    // Increase SD SPI Speed to 16MHz (Default is 4MHz)
    // helna, we use 4MHZ; final
    // This drastically reduces the time spent saving the file.
    if (!SD.begin(SD_CS_PIN, SPI, 4000000)) {
        blinkError(5);
        return;
    }

    if (!initCamera()) {
        blinkError(5);
        return;
    }

    // Reduced Warmup
    // 1 Frame is usually enough to clear the black buffer on OV2640
    // Reduced delay to 20ms
    camera_fb_t* warmup_fb = esp_camera_fb_get();
    esp_camera_fb_return(warmup_fb);
    delay(20);

    // Capture
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        blinkError(5);
        return;
    }

    // Filename logic
    // NOTE: If you remove the battery, gFileCount resets to 1.
    // The loop below will find the next gap, but it takes time if you have 1000 photos.
    char filename[32];
    while (true) {
        sprintf(filename, "/image_%03d.jpg", gFileCount);
        if (SD.exists(filename)) {
            gFileCount++;
        } else {
            break;
        }
    }

    // Save
    File file = SD.open(filename, FILE_WRITE);
    if (file) {
        file.write(fb->buf, fb->len);
        file.close();
        gFileCount++;
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


void recordSequence() {
    char filename[32];
    sprintf(filename, "/video%d.avi", gFileCount);
    videoFile = SD.open(filename, FILE_WRITE);
    if (!videoFile) {
        Serial.println("Error opening video file!");
        return;
    }
    Serial.printf("Recording video：%s\n", filename);


    // Start capturing video frames
    //
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Error getting framebuffer!");
        break;
    }
    videoFile.write(fb->buf, fb->len);
    esp_camera_fb_return(fb);
    }


    // Close the video file
    videoFile.close();
    Serial.printf("Video saved: %s\n", filename);
    gFileCount++;

    Serial.println("Video will begin in one minute, please be ready.");
}
*/
