#ifndef CONSTS_H
#define CONSTS_H
// CONSTANTS -------------------------------------------------------------------

#include "raylib.h"

// game related
#define DEBUG true
#define SCREEN_WIDTH 852
#define SCREEN_HEIGHT 480
#define GRAVITY 9.8
#define HORIZONTAL_DRAG 0.95 // between 0 and 1

// wrench related
#define JUMP_STRENGTH 350
#define FLY_SPEED 75
#define MAX_FLY_SPEED 500
#define WRENCH_WIDTH 50
#define WRENCH_THICKNESS 10
#define WRENCH_MAX_ANGLE 30
#define WRENCH_SPIN_SPEED 35

// limits
#define MAX_LINES 2048
#define CAM_SMOOTH_MAX_DATA 200
#define CAM_SMOOTH_AVG_COUNT 30

// colors
static const Color jumpColor = {
    233,
    65,
    56,
    255,
};

static const Color spinColor = {
    200,
    250,
    100,
    255,
};

#endif