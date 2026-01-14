#pragma once
#include <memory>
#include "globals.h"

class CameraContext;

class State {
public:
    virtual ~State() = default;
    virtual void enter(CameraContext& ctx) = 0;
    virtual void update(CameraContext& ctx) = 0;    // runs every loop
    virtual void exit(CameraContext& ctx) = 0;
};

class CameraContext{
private:
    std::unique_ptr<State> currentState;

public:
    CameraContext();

    // record <-> sleep
    void changeState(std::unique_ptr<State> p_new_state);
    
    // logic for the current state
    void update();
};

