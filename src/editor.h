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

// LOGIC -----------------------------------------------------------------------

int GetScaledWidth(float ratio) {
    int test = (int)(GetScreenWidth() * ratio);
    if (EDITOR_SIDEBAR_MIN_WIDTH > test) {
        return EDITOR_SIDEBAR_MIN_WIDTH;
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


    int halfPadding = (int)(EDITOR_ITEM_PADDING/2);

    Rectangle mainRec = (Rectangle){
        EDITOR_ITEM_PADDING,
        EDITOR_ITEM_PADDING,
        GetScaledWidth(editorState.screenRatio)-(EDITOR_ITEM_PADDING*2),
        EDITOR_ITEM_HEIGHT,
    };


    Rectangle halfLeftRec = (Rectangle){
        EDITOR_ITEM_PADDING,
        EDITOR_ITEM_PADDING,
        (GetScaledWidth(screenRatio)/2)-EDITOR_ITEM_PADDING-halfPadding,
        EDITOR_ITEM_HEIGHT,
    };

    Rectangle halfRightRec = (Rectangle) {
        (GetScaledWidth(screenRatio)/2)+halfPadding,
        EDITOR_ITEM_PADDING,
        (GetScaledWidth(screenRatio)/2)-EDITOR_ITEM_PADDING-halfPadding,
        EDITOR_ITEM_HEIGHT,
    };


    mainRec.y += EDITOR_ITEM_HEIGHT/2;
    GuiDrawText("Line Type Selection:", mainRec, TEXT_ALIGNMENT, BLACK);
    mainRec.y += EDITOR_ITEM_HEIGHT/2;
    

    halfLeftRec.y = mainRec.y;
    halfRightRec.y = mainRec.y;
    GuiButton(halfLeftRec, "Yellow");
    GuiButton(halfRightRec, "Red");

    mainRec.y += EDITOR_ITEM_HEIGHT+EDITOR_ITEM_MARGIN;


    halfLeftRec.y = mainRec.y;
    halfRightRec.y = mainRec.y;
    GuiButton(halfLeftRec, "Green");
    GuiButton(halfRightRec, "White");
    mainRec.y += EDITOR_ITEM_HEIGHT+EDITOR_ITEM_MARGIN;

    halfLeftRec.y = mainRec.y;
    halfRightRec.y = mainRec.y;
    GuiButton(halfLeftRec, "Magenta");
    mainRec.y += EDITOR_ITEM_HEIGHT+EDITOR_ITEM_MARGIN;


    mainRec.y += EDITOR_ITEM_MARGIN/2;
    GuiDrawText("Game start/end points:", mainRec, TEXT_ALIGNMENT, BLACK);
    mainRec.y += EDITOR_ITEM_MARGIN/2;

    halfLeftRec.y = mainRec.y;
    halfRightRec.y = mainRec.y;
    GuiButton(halfLeftRec, "Start");
    GuiButton(halfRightRec, "Goal");

    mainRec.y += EDITOR_ITEM_MARGIN;

    mainRec.y += EDITOR_ITEM_MARGIN/2;
    GuiDrawText("Save/Test:", mainRec, TEXT_ALIGNMENT, BLACK);
    mainRec.y += EDITOR_ITEM_MARGIN/2;
    GuiButton(mainRec, "Test Level");
    mainRec.y += EDITOR_ITEM_HEIGHT+EDITOR_ITEM_MARGIN;

    GuiButton(mainRec, "Save Level");
    mainRec.y += EDITOR_ITEM_HEIGHT+EDITOR_ITEM_MARGIN;

    mainRec.y += EDITOR_ITEM_MARGIN/2;
    GuiDrawText("Guide:", mainRec, TEXT_ALIGNMENT, BLACK);
    mainRec.y += EDITOR_ITEM_MARGIN/2;

    Rectangle tmpRec = mainRec;
    tmpRec.height = EDITOR_ITEM_HEIGHT*5;
    GuiTextBoxMulti(tmpRec, "Hold CTRL while mousing over the main screen to pan.", 15, false);
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
            return yellowLineColor; // TODO: change to more accurate color
    }
}


void DrawEditorLevelLines() { // TODO: redraw so that handles are drawn better
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

// INPUT -----------------------------------------------------------------------
void HandleMouseInput() {
    //TODO: implement
}

void HandleKeyboardInput() {
    if (IsKeyPressed(KEY_Q)) {
        exit(0);
    }

    if (IsKeyPressed(KEY_W)) {
        currentScreen = SCREEN_LEVEL;
    }
}

void HandleLineDrawing() {
    //TODO: implement/rename
}

// called inside the !windowShouldClose loop in main.c every frame, if currentScreen is set to SCREEN_LEVEL
void EditorScreenMainLoop(RenderTexture2D target) {

    // CAMERA PASS -------------------------------------------------------------
    UpdateCameraFromEditorState();


    // LOGIC PASS --------------------------------------------------------------
    editorState.mousePos = GetMousePosition();

    HandleMouseInput();
    HandleKeyboardInput();
    HandleLineDrawing();

    // DRAW PASS ---------------------------------------------------------------
    BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(camera);
            DrawEditorLevelLines();
        EndMode2D();

        DrawGuiBackground(editorState.screenRatio);
        DrawMainGuiColumn(editorState.screenRatio);
    EndDrawing();
}


#endif