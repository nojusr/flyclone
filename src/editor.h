#ifndef EDITOR_H
#define EDITOR_H

// raylib-related
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

// raygui-related
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

// first-party
#include "consts.h"
#include "collisions.h"
#include "globals.h"
#include "types.h"
#include "debug.h"




const int padding = 10;
const int min_gui_column_width = 200;
const int default_height = 25;
const int default_margin = 10; // margin between gui elements

const int drag_margin = 20;

float screenRatio = 0.25;
Vector2 mousePos = {0.0, 0.0};

bool isDragging = false;

int GetScaledWidth(float ratio) {
    int test = (int)(GetScreenWidth() * ratio);
    if (min_gui_column_width > test) {
        return min_gui_column_width;
    }
    return test;
}


void DrawGuiBackground(float screenRatio) {
    DrawRectangle(
        0,
        0,
        GetScaledWidth(screenRatio),
        GetScreenHeight(),
        GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_NORMAL))
    );
}

void DrawMainGuiColumn(float screenRatio) {


    int halfPadding = (int)(padding/2);

    Rectangle mainRec = (Rectangle){
        padding,
        padding,
        GetScaledWidth(screenRatio)-(padding*2),
        default_height,
    };


    Rectangle halfLeftRec = (Rectangle){
        padding,
        padding,
        (GetScaledWidth(screenRatio)/2)-padding-halfPadding,
        default_height,
    };

    Rectangle halfRightRec = (Rectangle) {
        (GetScaledWidth(screenRatio)/2)+halfPadding,
        padding,
        (GetScaledWidth(screenRatio)/2)-padding-halfPadding,
        default_height,
    };


    GuiButton(mainRec, "VIBE CHECK");
    mainRec.y += default_height+default_margin;
    GuiButton(mainRec, "VIBE CHECK2");
    mainRec.y += default_height+default_margin;
    halfLeftRec.y = mainRec.y;
    halfRightRec.y = mainRec.y;
    GuiButton(halfLeftRec, "LeftHalf");
    GuiButton(halfRightRec, "RightHalf");

} 

// called inside the !windowShouldClose loop in main.c every frame, if currentScreen is set to SCREEN_LEVEL
void EditorScreenMainLoop(RenderTexture2D target) {

    // LOGIC PASS --------------------------------------------------------------
    mousePos = GetMousePosition();

    int sideBarWidth = GetScaledWidth(screenRatio);

    if (isDragging) {
        float ratio = mousePos.x/GetScreenWidth();
        if (ratio < 1) {
            screenRatio = ratio;
        }
        
    }

    if (((int)mousePos.x) < sideBarWidth+drag_margin && ((int)mousePos.x) > sideBarWidth-drag_margin) {
        SetMouseCursor(MOUSE_CURSOR_RESIZE_EW);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            isDragging = true;
        } if (IsMouseButtonUp(MOUSE_LEFT_BUTTON)) {
            isDragging = false;
        }
    } else {
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }



    // DRAW PASS ---------------------------------------------------------------
    BeginDrawing();
        ClearBackground(BLACK);
        DrawGuiBackground(screenRatio);
        DrawMainGuiColumn(screenRatio);
    EndDrawing();
}


#endif