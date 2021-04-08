#include <stdio.h>
#include <math.h>
#include <stdlib.h>

// raylib-related
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

// first-party
#include "consts.h"
#include "collisions.h"
#include "globals.h"
#include "types.h"
#include "debug.h"
#include "level.h" 
#include "editor.h"
/*
#define max(a, b) ((a)>(b)? (a) : (b))
#define min(a, b) ((a)<(b)? (a) : (b))*/
// GENERAL IDEAS:
/*
i think that movement speed should be reduced when isJumping
level format filename: .fclv (flyclonelevel)

*/

// GENERAL TODO:
/*
1. implement drawing of obstacle lines DONE
2. implement serialization/deserialization of levels (and by extension lines) via http://troydhanson.github.io/tpl/
3. implement bouncing/collision physics DONE
4. implement camera movement (should move ahead of the wrench) DONE
5. implmenet a global 'screen state' for menus and level select screens
6. implement animated jumping
7. reduce movement speed when jumping??
8. add a height speed limit, but make it very lax
*/

// FUNCTIONS -------------------------------------------------------------------

// INIT ------------------------------------------------------------------------
void InitWrenchState() {
    state.isJumping = false;
    state.isSpinning = false;
    state.posX = SCREEN_WIDTH/2;
    state.posY = SCREEN_HEIGHT/2;
    state.velX = 0;
    state.velY = 0;
}

void InitCamera() {
    Camera2D output = { 0 };
    output.target = (Vector2){state.posX, state.posY};
    output.rotation = 0.0;
    output.zoom = 1.0;
    camera = output;
}

void InitLevelState() {
    levelState.isDead = false;
    levelState.levelLoaded = true;
    levelState.tries = 0;
    levelState.level = LoadObstacleDummyData();
}

void InitEditorState() {
    editorState.cameraPos = (Vector2){0.0, 0.0};
    editorState.cameraZoom = 1.0;
    editorState.level = LoadObstacleDummyData();
    editorState.isDragging = false;
}

void InitTimingGlobals() {
    targetFps = INITIAL_FPS;
    camSmoothAvgCount = 30;
}

void InitScreenState() {
    currentScreen = SCREEN_EDITOR;
}

// MAIN ------------------------------------------------------------------------

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitScreenState();
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Flyclone");
    InitWrenchState();
    InitCamera();
    InitTimingGlobals();
    InitLevelState();
    InitEditorState();
    SetTargetFPS(targetFps); 


    int gameScreenWidth = 852;
    int gameScreenHeight = 480;
    RenderTexture2D target = LoadRenderTexture(gameScreenWidth, gameScreenHeight);


    while (!WindowShouldClose()) {

        switch (currentScreen) {
        case SCREEN_LEVEL:
            LevelScreenMainLoop(target);
            break;
        case SCREEN_EDITOR:
            EditorScreenMainLoop(target);
            break;
        default:
            printf("No screen state set. aborting...");
            exit(1);
            //break;
        }
    }

    UnloadRenderTexture(target);
    CloseWindow();
    return 0;
}
