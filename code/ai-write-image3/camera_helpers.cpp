#include "camera_helpers.h"
#include "camera_functions.h"

Preferences g_prefs;
uint32_t g_imageCounter = 0;
// RTC_DATA_ATTR uint32_t g_imageCounter = 0;

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
    config.xclk_freq_hz = XCLK_FREQ_HZ;

    config.pixel_format = PIXFORMAT_JPEG;
    config.fb_count = 1; // 
    config.fb_location = CAMERA_FB_IN_PSRAM; // 
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY; // 


    // camera_settings
    config.frame_size = FRAMESIZE_UXGA; // 1600x1200
    // SVGA = 800x600

    // Lower quality slightly for faster write speed
    // 10 = Best/Slowest, 12 = Great/Faster, 15 = Good/Fastest
    config.jpeg_quality = JPEG_QUALITY; // 20 


    if(PWDN_GPIO_NUM != -1) {
        pinMode(PWDN_GPIO_NUM, OUTPUT);
        digitalWrite(PWDN_GPIO_NUM, LOW); 
    }

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        char e_buffer[150] {};
#ifdef _DEBUG_PRINT_SHOW_
        snprintf(e_buffer, sizeof(e_buffer), "Camera init failed with error 0x%x", err);
        Serial.printf("%s\n", e_buffer);
#endif
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


void cameraSettingsInit() {
    // changing settings of the camera 
    // ----- see the camera settings here: 
    // https://randomnerdtutorials.com/esp32-cam-ov2640-camera-settings/

    sensor_t * camera_settings = esp_camera_sensor_get();
    // 0=disable, 1=enable
    // =========== ORIENTATION
    camera_settings->set_hmirror(camera_settings, 1);       // so we can read whatevers being outputted there // left-right swap
    camera_settings->set_vflip(camera_settings, 0);         // up-down swap

    // =========== IMAGE QUALITY & COLOR CORRECTION
    camera_settings->set_lenc(camera_settings, 1);       // lens correction (fixes the default dark corners/vignette) 
    camera_settings->set_dcw(camera_settings, 0);        // downsize compare white (helps with auto-exposure)

    // =========== WB and WG
    camera_settings->set_whitebal(camera_settings, 1);      // if 0, colors may look weird when lighting changes
    camera_settings->set_awb_gain(camera_settings, 1);      // automatic white balance
    camera_settings->set_wb_mode(camera_settings, 0);       // 0=auto, 1=sunny, 2=cloudy, 3=office, 4=home

    // =========== EXPOSURE
    // If images are still too dark or noisy, you can tweak these
    camera_settings->set_aec2(camera_settings, 1);       // automatic exposure control 2 (DSP)
    camera_settings->set_ae_level(camera_settings, 0);   // auto exposure level (-2 to 2)
    camera_settings->set_gain_ctrl(camera_settings, 0);  // auto gain control (on)
    
    // ============ SETTINGS TO ACHIEVE THAT VHS/RETRO
    camera_settings->set_contrast(camera_settings, 1);      // semi contrast        -2 to 2
    camera_settings->set_brightness(camera_settings, 1);    // boost brightness     -2 to 2
    camera_settings->set_saturation(camera_settings, 1);    // achieve that bleeding colors??   -2 to 2

}


// ================= FILENAME CREATION =================
void createNextFilename(char* p_buffer, const char* p_prefix, const char* p_extension){



    // -------- OPTION C: DIRECTORY SHARDING --------
    // keeps FAT directories small even with millions of images

    uint32_t img = g_imageCounter++;
    uint16_t dir = img / 1000;   // 1000 images per folder
    uint16_t file = img % 1000;

    char dirPath[64];
    sprintf(dirPath, "/DCIM/%03u", dir);

    if (!SD.exists(dirPath)) {
        SD.mkdir(dirPath);
    }

    sprintf(p_buffer, "%s/%s_%03u.%s",
            dirPath,
            p_prefix,
            file,
            p_extension);
}

void initFilenameSystem() {
    g_prefs.begin("camera", false);
    g_imageCounter = g_prefs.getUInt("img", 0);
}

void commitFilenameCounter() {
    // write only once per photo to reduce flash wear
    g_prefs.putUInt("img", g_imageCounter);
    g_prefs.end();
}
