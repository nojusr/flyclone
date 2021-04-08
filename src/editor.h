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




const int padding = 5;
const int min_gui_column_width = 200;
const int default_height = 25;
const int default_margin = 5; // margin between gui elements
const int drag_margin = 20;




Vector2 mouseDragStartPos;
Vector2 cameraDragStartInitialPos;
bool isPanning;

// LOGIC -----------------------------------------------------------------------

int GetScaledWidth(float ratio) {
    int test = (int)(GetScreenWidth() * ratio);
    if (min_gui_column_width > test) {
        return min_gui_column_width;
    }
    return test;
}

// DRAW ------------------------------------------------------------------------

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


    mainRec.y += default_height/2;
    GuiDrawText("Line Type Selection:", mainRec, TEXT_ALIGNMENT, BLACK);
    mainRec.y += default_height/2;
    

    halfLeftRec.y = mainRec.y;
    halfRightRec.y = mainRec.y;
    GuiButton(halfLeftRec, "Yellow");
    GuiButton(halfRightRec, "Red");

    mainRec.y += default_height+default_margin;


    halfLeftRec.y = mainRec.y;
    halfRightRec.y = mainRec.y;
    GuiButton(halfLeftRec, "Green");
    GuiButton(halfRightRec, "White");
    mainRec.y += default_height+default_margin;

    halfLeftRec.y = mainRec.y;
    halfRightRec.y = mainRec.y;
    GuiButton(halfLeftRec, "Magenta");
    mainRec.y += default_height+default_margin;


    mainRec.y += default_height/2;
    GuiDrawText("Game start/end points:", mainRec, TEXT_ALIGNMENT, BLACK);
    mainRec.y += default_height/2;

    halfLeftRec.y = mainRec.y;
    halfRightRec.y = mainRec.y;
    GuiButton(halfLeftRec, "Start");
    GuiButton(halfRightRec, "Goal");

    mainRec.y += default_height;

    mainRec.y += default_height/2;
    GuiDrawText("Save/Test:", mainRec, TEXT_ALIGNMENT, BLACK);
    mainRec.y += default_height/2;
    GuiButton(mainRec, "Test Level");
    mainRec.y += default_height+default_margin;

    GuiButton(mainRec, "Save Level");
    mainRec.y += default_height+default_margin;
}


Color GetEditorLineObstacleColor(LineObstacle input) {
    switch (input.type) {
        case LINE_GREEN:
            return spinColor;
        case LINE_WHITE:
            return WHITE;
        case LINE_RED:
            return jumpColor;
        case LINE_YELLOW:
            return YELLOW; // TODO: change to more accurate color
    }
}


void DrawEditorLevelLines() {
    Color lineColor;
    for (int i = 0; i < editorState.level.lineCount; i++) {
        LineObstacle obs = editorState.level.lines[i];
        lineColor = GetEditorLineObstacleColor(obs);
        DrawLineEx(
            obs.line.start,
            obs.line.end,
            6.0,
            lineColor
        );
        DrawLineEx(
            obs.line.start,
            obs.line.end,
            2.0,
            BLACK
        );
    }

}


// CAMERA ----------------------------------------------------------------------

void UpdateCameraFromEditorState() {
    camera.target = editorState.cameraPos;
    camera.zoom = editorState.cameraZoom;
}


// called inside the !windowShouldClose loop in main.c every frame, if currentScreen is set to SCREEN_LEVEL
void EditorScreenMainLoop(RenderTexture2D target) {

    // CAMERA PASS -------------------------------------------------------------
    UpdateCameraFromEditorState();


    // LOGIC PASS --------------------------------------------------------------
    editorState.mousePos = GetMousePosition();

    int sideBarWidth = GetScaledWidth(editorState.screenRatio);

    int viewPortCenterX = GetScaledWidth((1-editorState.screenRatio)/2);

    if (IsKeyUp(KEY_LEFT_CONTROL) && isPanning) {
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
            isPanning = false;     
    }

    if (editorState.isDragging) {
        float ratio = editorState.mousePos.x/GetScreenWidth();
        if (ratio < 1) {
            editorState.screenRatio = ratio;
        }
        
    } else if (isPanning) {
        
        editorState.cameraPos = Vector2Add(
            cameraDragStartInitialPos,
            Vector2Subtract(mouseDragStartPos, editorState.mousePos)
        ) ;
    }



    if (((int)editorState.mousePos.x) < sideBarWidth+drag_margin &&
        ((int)editorState.mousePos.x) > sideBarWidth-drag_margin) {
        SetMouseCursor(MOUSE_CURSOR_RESIZE_EW);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            editorState.isDragging = true;
        } if (IsMouseButtonUp(MOUSE_LEFT_BUTTON)) {
            editorState.isDragging = false;
        }
    } else if (((int)editorState.mousePos.x) > sideBarWidth+drag_margin) {

        if (IsKeyPressed(KEY_LEFT_CONTROL)) {
            cameraDragStartInitialPos = camera.target;
            mouseDragStartPos = editorState.mousePos;
            SetMouseCursor(MOUSE_CURSOR_RESIZE_ALL);
            isPanning = true;
        }
    } else {
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }


    

    if (IsKeyPressed(KEY_Q)) {
        exit(0);
    }

    if (IsKeyPressed(KEY_W)) {
        currentScreen = SCREEN_LEVEL;
    }

    // DRAW PASS ---------------------------------------------------------------
    BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(camera);
            DrawLevelLines();
        EndMode2D();


        DrawGuiBackground(editorState.screenRatio);
        DrawMainGuiColumn(editorState.screenRatio);
    EndDrawing();
}


#endif