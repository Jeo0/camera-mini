#include "camera_states.h"
#include "camera_helpers.h"
#include "camera_functions.h"

CameraContext::CameraContext() {
    // decide what state based on pin
    currentState = std::unique_ptr<DecisionState>(new DecisionState());
    currentState->enter(*this);
}

void CameraContext::changeState(std::unique_ptr<State> p_new_state){
    if (currentState) currentState->exit(*this);
    currentState = std::move(p_new_state);
    if (currentState) currentState->enter(*this);
}

void CameraContext::update() {
    if(currentState) currentState->update(*this);
}


// =================================================================
void DecisionState::enter(CameraContext& p_ctx) {
    Serial.println("Checking Mode Pin State enter");
    Serial.println("jajaja photyostate immediately");
#ifdef _WITH_RECORD_STATE_
    pinMode(MODE_PIN, INPUT_PULLDOWN);
#endif

    // we want to have less power usage
    // normally, we are on photo mode
    // so we use internal resistor when we are on LOW (photo)
    // and use the external resistor when we are on HIGH (recording)
    // so by default, it is LOW + the pin is stable at that state
    
    // we are removing the record state as it heats the camera too much inside the case 
    // also to reduce the physical size requirements so no more heatsink
    p_ctx.changeState(std::unique_ptr<PhotoState>(new PhotoState()));
#ifdef _WITH_RECORD_STATE_
    if(digitalRead(MODE_PIN) == HIGH) p_ctx.changeState(std::unique_ptr<RecordState>(new RecordState()));
    else                p_ctx.changeState(std::unique_ptr<PhotoState>(new PhotoState()));
#endif


}

void DecisionState::update(CameraContext& p_ctx) {} // no update; instant


// =================================================================
void PhotoState::enter(CameraContext& p_ctx){
    Serial.println("PhotoState enter");

    // LED ON (Indicates "Shutter Open / Busy")
    digitalWrite(FLASH_PIN, FLASH_ON); 

    // init SD
    // Increase SD SPI Speed to 16MHz (Default is 4MHz)
    // helna, we use 16MHZ; final
    // This drastically reduces the time spent saving the file.
    if (!SD.begin(SD_CS_PIN, SPI, 16000000)) {
        blinkError(10);
    }

    if (!initCamera()) {
        blinkError(20);
        SD.end();
        p_ctx.changeState(std::unique_ptr<DeepSleepState>(new DeepSleepState()));
        return;  // ← Exit early on error
    } else {
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
        } else {
            // generation of file name and save
            char filename[32];
            // createNextFilename(filename, "image_", "jpg");
            createNextFilename(filename, "A_image", "jpg");

            File file = SD.open(filename, FILE_WRITE);
            if (file) {
                file.write(fb->buf, fb->len);
                file.close();
                char e_buffer[100] {};
                snprintf(e_buffer, sizeof(e_buffer), "Saved: %s\n", filename);
                Serial.printf(e_buffer);
            } else {
                blinkError(2);
            }
            esp_camera_fb_return(fb);

        }
        
        esp_camera_deinit();
        SD.end();
    }
    
    digitalWrite(FLASH_PIN, FLASH_OFF);
    if(PWDN_GPIO_NUM != -1) digitalWrite(PWDN_GPIO_NUM, HIGH);
    delay(50);

    p_ctx.changeState(std::unique_ptr<DeepSleepState>(new DeepSleepState()));

}

void PhotoState::update(CameraContext& p_ctx){
    // this isnt reached
    // p_ctx.changeState(std::unique_ptr<DeepSleepState>(new DeepSleepState()));
}

void PhotoState::exit(CameraContext& p_ctx) {}


// =================================================================
void RecordState::enter(CameraContext& p_ctx) {
    Serial.println("RecordingState enter");

    if(!SD.begin(SD_CS_PIN, SPI, 16000000) || !initCamera()){
        Serial.println("Error reading SD enter");
        blinkError(5);
        p_ctx.changeState(std::unique_ptr<DeepSleepState>(new DeepSleepState()));
        return;
    }

    char filename[32];
    // createNextFilename(filename, "video", "avi");
    createNextFilename(filename, "A_video", "avi");

    videoFile = SD.open(filename, FILE_WRITE);
    if(!videoFile){
        Serial.println("Error opening video file");;
        blinkError(3);
        p_ctx.changeState(std::unique_ptr<DeepSleepState>(new DeepSleepState()));
        return;
    }

    char e_buffer[100] {};
    snprintf(e_buffer, sizeof(e_buffer), "Recording to %s", filename);
    Serial.printf(e_buffer);
    isRecording = true;
    digitalWrite(FLASH_PIN, FLASH_ON); // LED ON
}

void RecordState::update(CameraContext& p_ctx) {
    if(!isRecording) return;
    
    // capture frame
    camera_fb_t *fb = esp_camera_fb_get();
    if(fb){
        videoFile.write(fb->buf, fb->len);
        esp_camera_fb_return(fb);
    }
    
    // checking exit condition
    if(digitalRead(BUTTON_PIN) == LOW){
        Serial.println("Butten pressed; recording stop");
        p_ctx.changeState(std::unique_ptr<DeepSleepState>(new DeepSleepState()));
    }
}

void RecordState::exit(CameraContext& p_ctx){
    if(videoFile){
        videoFile.close();
        Serial.println("Video saved:");
    }

    digitalWrite(FLASH_PIN, FLASH_OFF); // OFF LED
    esp_camera_deinit();
}


// ====================================================
void DeepSleepState::enter(CameraContext& p_ctx) {
    Serial.println("DeepSleepState enter");

    // every deep sleep we enter pinmode to ensure
    pinMode(BUTTON_PIN, INPUT);
    while(digitalRead(BUTTON_PIN) == HIGH) delay(10); 

    // always reset the FLASH PIN TO OFF (HIGH)
    digitalWrite(FLASH_PIN, FLASH_OFF);

    // wake up configuration
    esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_PIN, 1); // Wake on HIGH
    esp_deep_sleep_start();
}


void DeepSleepState::update(CameraContext& p_ctx) {}
