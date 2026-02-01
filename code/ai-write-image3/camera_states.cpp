#include "camera_states.h"
#include "camera_helpers.h"
#include "camera_functions.h"
#include <driver/rtc_io.h> 

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
#ifdef _DEBUG_PRINT_SHOW_
    Serial.println("Checking Mode Pin State enter");
    Serial.println("jajaja photyostate immediately");
#endif
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
    initFilenameSystem();
#ifdef _DEBUG_PRINT_SHOW_
    Serial.println("PhotoState enter");
#endif

    // LED ON (Indicates "Shutter Open / Busy")
    digitalWrite(FLASH_PIN, FLASH_ON); 


    // STEP 0: init SD
    // Increase SD SPI Speed to 16MHz (Default is 4MHz)
    // helna, we use 16MHZ; final
    // This drastically reduces the time spent saving the file.
    if (!SD.begin(SD_CS_PIN, SPI, SD_SPI_SPEED)) {
#ifdef _DEBUG_PRINT_SHOW_
        Serial.println("Error reading SD enter");
        delay(100);     // give time for serial flush
#endif
        blinkError(BLINK_TIMES_ERROR_SDCARD, BLINK_MILLI_DURATION_MAJOR);
        p_ctx.changeState(std::unique_ptr<DeepSleepState>(new DeepSleepState()));
        return;
    }


    // STEP 1: init CAMERA
    if (!initCamera()) {
#ifdef _DEBUG_PRINT_SHOW_
        Serial.println("Error Camera Initialization");
        delay(100);     // give time for serial flush
#endif
        blinkError(BLINK_TIMES_ERROR_CAMERA, BLINK_MILLI_DURATION_MAJOR);
        SD.end();
        p_ctx.changeState(std::unique_ptr<DeepSleepState>(new DeepSleepState()));
        return;  // ← Exit early on error
    } 
    

    // STEP 2: all is good
    delay(25);  // addition for better power stablization
    // Reduced Warmup
    // 1 Frame is usually enough to clear the black buffer on OV2640
    // Reduced delay to 20ms
    camera_fb_t* warmup_fb = esp_camera_fb_get();
    esp_camera_fb_return(warmup_fb);
    //delay(20);
    delay(25);


    // STEP 3: capture
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
#ifdef _DEBUG_PRINT_SHOW_
        Serial.println("cannot capture???");
#endif
        esp_camera_deinit();  
        SD.end();  
        blinkError(BLINK_TIMES_ERROR_CAMERA, BLINK_MILLI_DURATION_MAJOR);
        p_ctx.changeState(std::unique_ptr<DeepSleepState>(new DeepSleepState()));
        return;
    } 


    // STEP 4: generation of file name and save
    char filename[32];
    // createNextFilename(filename, "image_", "jpg");
    createNextFilename(filename, "KATEY_image", "jpg");

    File file = SD.open(filename, FILE_WRITE);
    if (!file) {
#ifdef _DEBUG_PRINT_SHOW_
        Serial.println("cant create file");
#endif
        esp_camera_deinit();  
        SD.end();  
        blinkError(BLINK_TIMES_ERROR_SDCARD, BLINK_MILLI_DURATION_MINOR);
        p_ctx.changeState(std::unique_ptr<DeepSleepState>(new DeepSleepState()));
        return;
    } 
    file.write(fb->buf, fb->len);
    file.close();
    char e_buffer[100] {};
#ifdef _DEBUG_PRINT_SHOW_
    snprintf(e_buffer, sizeof(e_buffer), "Saved: %s\n", filename);
    Serial.printf(e_buffer);
#endif
    commitFilenameCounter();

    // STEP success: last parts
    esp_camera_fb_return(fb);
    esp_camera_deinit();
    SD.end();
    SPI.end();
    
    digitalWrite(FLASH_PIN, FLASH_OFF);

    // POWER DA FAK DOWN CAMERA U GOING FAKENG HOT 
    if(PWDN_GPIO_NUM != -1) {               
        pinMode(PWDN_GPIO_NUM, OUTPUT);
        digitalWrite(PWDN_GPIO_NUM, HIGH);
    }
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
#ifdef _DEBUG_PRINT_SHOW_
    Serial.println("RecordingState enter");
#endif

    if(!SD.begin(SD_CS_PIN, SPI, SD_SPI_SPEED) || !initCamera()){
#ifdef _DEBUG_PRINT_SHOW_
        Serial.println("Error reading SD enter");
#endif
        blinkError(BLINK_TIMES_ERROR_SDCARD, BLINK_MILLI_DURATION_MAJOR);
        p_ctx.changeState(std::unique_ptr<DeepSleepState>(new DeepSleepState()));
        return;
    }

    char filename[32];
    // createNextFilename(filename, "video", "avi");
    createNextFilename(filename, "A_video", "avi");

    videoFile = SD.open(filename, FILE_WRITE);
    if(!videoFile){
#ifdef _DEBUG_PRINT_SHOW_
        Serial.println("Error opening video file");;
#endif
        blinkError(BLINK_TIMES_ERROR_SDCARD, BLINK_MILLI_DURATION_MINOR);
        p_ctx.changeState(std::unique_ptr<DeepSleepState>(new DeepSleepState()));
        return;
    }

    char e_buffer[100] {};
#ifdef _DEBUG_PRINT_SHOW_
    snprintf(e_buffer, sizeof(e_buffer), "Recording to %s", filename);
    Serial.printf(e_buffer);
#endif
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
#ifdef _DEBUG_PRINT_SHOW_
        Serial.println("Butten pressed; recording stop");
#endif
        p_ctx.changeState(std::unique_ptr<DeepSleepState>(new DeepSleepState()));
    }
}

void RecordState::exit(CameraContext& p_ctx){
    if(videoFile){
        videoFile.close();
#ifdef _DEBUG_PRINT_SHOW_
        Serial.println("Video saved:");
#endif
    }

    digitalWrite(FLASH_PIN, FLASH_OFF); // OFF LED
    esp_camera_deinit();
}


// ====================================================
void DeepSleepState::enter(CameraContext& p_ctx) {
#ifdef _DEBUG_PRINT_SHOW_
    Serial.println("DeepSleepState enter");
#endif

    // every deep sleep we enter pinmode to ensure
    pinMode(BUTTON_PIN, INPUT);
    while(digitalRead(BUTTON_PIN) == HIGH) delay(10); 

    // always reset the FLASH PIN TO OFF (HIGH)
    digitalWrite(FLASH_PIN, FLASH_OFF);

    // this prevents current leakage through RTC domain
    setCpuFrequencyMhz(80);
    
    // wake up configuration
    esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_PIN, 1); // Wake on HIGH
    esp_deep_sleep_start();
}


void DeepSleepState::update(CameraContext& p_ctx) {}
