#pragma once
#include "camera_state_machine.h"

// check MODE_PIN to know if we are on video or photo mode
class DecisionState : public State {
public:
    void enter(CameraContext& p_ctx) override;
    void update(CameraContext& p_ctx) override;
    void exit(CameraContext& p_ctx) override {};
};

// photo; take 1 pic, then deep sleep 
class PhotoState : public State {
private:
    int jpegIncrement;
public:
    void enter(CameraContext& p_ctx) override;
    void update(CameraContext& p_ctx) override;
    void exit(CameraContext& p_ctx) override;
};

// video; start record when button clicked, stops record when button is pressed again
class RecordState : public State {
private:
    File videoFile;
    bool isRecording;
    int fileIncrement;
public:
    void enter(CameraContext& p_ctx) override;
    void update(CameraContext& p_ctx) override; // record 1 frame per tick
    void exit(CameraContext& p_ctx) override;
};

// Sleeping
class DeepSleepState : public State {
public:
    void enter(CameraContext& p_ctx) override;
    void update(CameraContext& p_ctx) override; // will never run as it is sleeping
    void exit(CameraContext& p_ctx) override {}; 
};
