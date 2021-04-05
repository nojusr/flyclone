#ifndef GLOBALS_H
#define GLOBALS_H

#include "raylib.h"
#include "types.h"
#include "consts.h"

// global wrench state of the last frame
WrenchState lastState; 
// global wrench state for wrench/player
WrenchState state; 

// the time between the last frame and the current frame
// gets updated every frame
float deltaTime = 0.0;

// used for smoothly animating the wrench while it's in spinmode
double spinningWrenchAngle = 0; // might move this out to wrenchState

// main camera global
Camera2D camera; 

// contains the coordinate data used for camera smoothing
double floatingAvgSetX[CAM_SMOOTH_MAX_DATA];
double floatingAvgSetY[CAM_SMOOTH_MAX_DATA];

// global index for the two arrays above
int currentAvgIndex;

// global level state.
LevelState levelState;

int targetFps;
int camSmoothAvgCount;





#endif