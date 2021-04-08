#ifndef CONSTS_H
#define CONSTS_H
// CONSTANTS -------------------------------------------------------------------

#include "raylib.h"

// game related
#define DEBUG true
#define SCREEN_WIDTH 852
#define SCREEN_HEIGHT 480
#define GRAVITY 900
#define HORIZONTAL_DRAG 0.95
#define INITIAL_FPS 60


// wrench related
#define JUMP_STRENGTH 350
#define FLY_SPEED 75
#define MAX_FLY_SPEED 500
#define WRENCH_WIDTH 50
#define WRENCH_THICKNESS 10
#define WRENCH_MAX_ANGLE 30
#define WRENCH_SPIN_SPEED 3000
#define WRENCH_BOUNCE_LOSS_RATIO 0.9

// editor screen related
#define EDITOR_SIDEBAR_MIN_WIDTH 200
#define EDITOR_ITEM_HEIGHT 25
#define EDITOR_ITEM_PADDING 5
#define EDITOR_ITEM_MARGIN 5 // margin between gui elements
#define EDITOR_DRAG_HANDLE_MARGIN 20 // mul by 2 to get drag handle width

// limits
#define MAX_LINES 2048
#define CAM_SMOOTH_MAX_DATA 200

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

static const Color yellowLineColor = {
    240,
    235,
    35,
    255,
};

#endif