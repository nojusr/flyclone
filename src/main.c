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
2. implement binary serialization/deserialization of levels (and by extension lines) via http://troydhanson.github.io/tpl/
3. implement bouncing/collision physics
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
    levelState.spawnPoint = (Vector2){0.0, 0.0};
    levelState.levelLoaded = true;
    levelState.tries = 0;
    levelState.loadedLineCount = 0;

}

// GAME LOGIC ------------------------------------------------------------------

void ApplyGravityToWrench() {
    state.velY += GRAVITY;
}

void ApplyDragToWrench() {
    if (state.isSpinning == false) { // spinmode -> no horizontal drag
        state.velX *= HORIZONTAL_DRAG;

        if (fabs(state.velX) <= 1) {
            state.velX = 0;
        }
    }
}

void SetWrenchPositionFromVelocity() {
    state.posX += state.velX*deltaTime;
    state.posY += state.velY*deltaTime;
}

void HandleCollisions() {
    for (int i = 0; i < levelState.loadedLineCount; i++) {
        LineObstacle lineObs = levelState.loadedLines[i];
        if (CheckCollisionLineObsWrench(lineObs)) {
            if (lineObs.type == LINE_YELLOW && !state.isSpinning) {
                levelState.isDead = true;
                levelState.tries++;
            }
        }
    }
}


// DRAWING ---------------------------------------------------------------------
// all functions here are to be called inside the draw pass
// (in between BeginDrawing and endDrawing)

void DrawWrench() {
    Color wrenchColor;
    wrenchColor.a = 255;

    Rectangle rec;
    rec.y = state.posY;
    
    Vector2 origin;
    origin.x = WRENCH_WIDTH/2;
    origin.y = WRENCH_THICKNESS/2;

    if (state.isJumping) {
        spinningWrenchAngle = 0;
        wrenchColor = jumpColor;
        rec.x =  state.posX - (WRENCH_THICKNESS/2);
        rec.width = WRENCH_THICKNESS;
        rec.height = WRENCH_WIDTH/2;
    } else if (state.isSpinning) {
        spinningWrenchAngle += WRENCH_SPIN_SPEED;
        wrenchColor = spinColor;
        rec.x = state.posX - (WRENCH_WIDTH/2);
        rec.width = WRENCH_WIDTH;
        rec.height = WRENCH_THICKNESS;
    } else {
        spinningWrenchAngle = 0;
        wrenchColor = WHITE;
        rec.x = state.posX - (WRENCH_WIDTH/2);
        rec.width =WRENCH_WIDTH;
        rec.height = WRENCH_THICKNESS;
    }

    
    double angle = (state.velX/MAX_FLY_SPEED) * WRENCH_MAX_ANGLE;

    if (angle > WRENCH_MAX_ANGLE) {
        angle = WRENCH_MAX_ANGLE;
    } else if (angle < -WRENCH_MAX_ANGLE) {
        angle = -WRENCH_MAX_ANGLE;
    }

    DrawRectanglePro(
        rec,
        origin,
        angle + spinningWrenchAngle,
        wrenchColor
    );
}


Color GetLineObstacleColor(LineObstacle input) {
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


void DrawLevelLines() {
    Color lineColor;
    for (int i = 0; i < levelState.loadedLineCount; i++) {
        LineObstacle obs = levelState.loadedLines[i];
        lineColor = GetLineObstacleColor(obs);
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

// CAMERA RELATED --------------------------------------------------------------

void UpdateCameraFromWrenchState() {
    double lockedCameraX = state.posX-((double)SCREEN_WIDTH)/2;
    double lockedCameraY = state.posY-((double)SCREEN_HEIGHT)/2;

    double velModX = lockedCameraX + (state.velX*0.7);
    double velModY = lockedCameraY + (state.velY*0.25);

    currentAvgIndex++;

    if (currentAvgIndex > CAM_SMOOTH_MAX_DATA-1) {
        currentAvgIndex = 0;
    }
    floatingAvgSetX[currentAvgIndex] = velModX;
    floatingAvgSetY[currentAvgIndex] = velModY;

    double xSum = 0;
    double ySum = 0;

    for (int i = currentAvgIndex; i > currentAvgIndex-CAM_SMOOTH_AVG_COUNT; i--) {
        int j = i;
        if (j < 0) {
            j += (CAM_SMOOTH_MAX_DATA-1);
        }
        xSum += floatingAvgSetX[j];
        ySum += floatingAvgSetY[j];
    }
    
    double finalX = (xSum / CAM_SMOOTH_AVG_COUNT)-(WRENCH_WIDTH/2);
    double finalY = ySum / CAM_SMOOTH_AVG_COUNT;

    camera.target = (Vector2){finalX, finalY};
}

// INPUT HANDLING --------------------------------------------------------------

void HandleJump() {
    if (!lastState.isJumping) {
        state.velY -= JUMP_STRENGTH;
    }
    state.isJumping = true;
}

void HandleSpin() {
    state.isSpinning = true;
}

void HandleWrenchNoAction() {
    state.isSpinning = false;
    state.isJumping = false;
}

void HandleFlyRight() {
    if (state.isSpinning == false) {
        state.velX += FLY_SPEED;
        if (state.velX >= MAX_FLY_SPEED) {
            state.velX = MAX_FLY_SPEED;
        }
    }
}

void HandleFlyLeft() {
    if (state.isSpinning == false) {
        state.velX -= FLY_SPEED;
        
        if (state.velX <= -MAX_FLY_SPEED) {
            state.velX = -MAX_FLY_SPEED;
        }

    }
}

void HandleInput() {
    if (IsKeyDown(KEY_Q) && DEBUG) {
        exit(0);
    }

    if (!levelState.isDead){
        // jumping and spinning are mutually exclusive
        if (IsKeyDown(KEY_X)) {
            HandleJump();
        } else if (IsKeyDown(KEY_Z)) {
            HandleSpin();
        } else {
            HandleWrenchNoAction();
        }

        if (IsKeyDown(KEY_RIGHT)) {
            HandleFlyRight();
        }
        if (IsKeyDown(KEY_LEFT)) {
            HandleFlyLeft();
        }
    } else {
        if (IsKeyPressed(KEY_R)) {
            state.posX = levelState.spawnPoint.x;
            state.posY = levelState.spawnPoint.y;
            levelState.isDead = false;
        }
    }
}

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Flyclone");
    InitWrenchState();
    InitCamera();
    LoadObstacleDummyData();
    SetTargetFPS(60); 


    int gameScreenWidth = 852;
    int gameScreenHeight = 480;
    RenderTexture2D target = LoadRenderTexture(gameScreenWidth, gameScreenHeight);


    while (!WindowShouldClose()) {
        // CAMERA PASS ---------------------------------------------------------
        if (!levelState.isDead) {
            UpdateCameraFromWrenchState();
        }

        // LOGIC PASS ----------------------------------------------------------
        deltaTime = GetFrameTime();
        lastState = state;

        if (!levelState.isDead) {
            ApplyGravityToWrench();
            ApplyDragToWrench();
        }

        // temporary hack so that the wrench doesn't fly out of screen
        if (state.posY >= 500) {
            state.posY = 500;
            state.velY = 0;
        }

        HandleInput();
        HandleCollisions();
        SetWrenchPositionFromVelocity();

        // DRAW PASS -----------------------------------------------------------
        BeginDrawing();
            ClearBackground(BLACK);
            BeginTextureMode(target);
                ClearBackground(BLACK);
                BeginMode2D(camera);
                    DrawLevelLines();
                    if (!levelState.isDead){
                        DrawWrench();
                    }
                EndMode2D();
            EndTextureMode();

            DrawTexturePro(
                target.texture,
                (Rectangle){0.0, 0.0, target.texture.width, -target.texture.height},
                (Rectangle){0.0, 0.0, GetScreenWidth(), GetScreenHeight()}, 
                (Vector2){0.0, 0.0}, 
                0.0, WHITE);

            const char* text = "Flyclone test (x to flap, z to spin, arrow keys to move)";
            const Vector2 text_size = MeasureTextEx(GetFontDefault(), text, 15, 1);
            DrawText(text, GetScreenWidth() / 2 - text_size.x / 2, 10, 15, WHITE);

            if (levelState.isDead) {
                const char* deathText = "You are dead. (r to restart)";
                const Vector2 death_text_size = MeasureTextEx(GetFontDefault(), deathText, 15, 1);
                DrawText(deathText, GetScreenWidth() / 2 - death_text_size.x / 2, GetScreenHeight()/2, 15, WHITE);
            }
        EndDrawing();
    }

    UnloadRenderTexture(target);
    CloseWindow();
    return 0;
}
