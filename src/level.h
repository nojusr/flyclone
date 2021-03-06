// various functions for handling the main game-related logic
#ifndef LEVEL_H
#define LEVEL_H
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


// GAME LOGIC ------------------------------------------------------------------

void ApplyGravityToWrench() {
    state.velY += (GRAVITY*deltaTime);
}

void ApplyDragToWrench() {
    if (state.isSpinning == false) { // spinmode -> no horizontal drag
        state.velX *= (HORIZONTAL_DRAG);

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
    for (int i = 0; i < levelState.level.lineCount; i++) {
        LineObstacle lineObs = levelState.level.lines[i];
        if (CheckCollisionLineObsWrench(lineObs)) {
            if (lineObs.type == LINE_YELLOW) {
                if (state.isSpinning) {
                    //https://math.stackexchange.com/questions/13261/how-to-get-a-reflection-vector

                    Line line = lineObs.line;
                    float dx = line.end.x - line.start.x;
                    float dy = line.end.y - line.start.y;
                    Vector2 n = Vector2Normalize((Vector2){-dy, dx});


                    Vector2 d = (Vector2){state.velX, state.velY};

                    float dot = Vector2DotProduct(d, n);

                    Vector2 sub = (Vector2){2*n.x*dot,2*n.y*dot};

                    Vector2 r = Vector2Subtract(d, sub);

                    state.velX = r.x * WRENCH_BOUNCE_LOSS_RATIO;
                    state.velY = r.y * WRENCH_BOUNCE_LOSS_RATIO;
                } else {
                    levelState.isDead = true;
                    levelState.tries++;
                }
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
        spinningWrenchAngle += WRENCH_SPIN_SPEED*deltaTime;
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
            return yellowLineColor; // TODO: change to more accurate color
    }
}


void DrawLevelLines() {
    Color lineColor;
    for (int i = 0; i < levelState.level.lineCount; i++) {
        LineObstacle obs = levelState.level.lines[i];
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

    for (int i = currentAvgIndex; i > currentAvgIndex-camSmoothAvgCount; i--) {
        int j = i;
        if (j < 0) {
            j += (CAM_SMOOTH_MAX_DATA-1);
        }
        xSum += floatingAvgSetX[j];
        ySum += floatingAvgSetY[j];
    }
    
    double finalX = (xSum / ((double)camSmoothAvgCount))-(WRENCH_WIDTH/2);
    double finalY = ySum / ((double)camSmoothAvgCount);

    camera.target = (Vector2){finalX, finalY};
    //camera.target = (Vector2){lockedCameraX, lockedCameraY};
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

    SetMouseCursor(MOUSE_CURSOR_DEFAULT);

    if (IsKeyDown(KEY_Q) && DEBUG) {
        exit(0);
    }

    if (IsKeyPressed(KEY_W)) {
        currentScreen = SCREEN_EDITOR;
    }

    if (IsKeyPressed(KEY_R) && DEBUG) {
        state.posX = levelState.level.spawnPoint.x;
        state.posY = levelState.level.spawnPoint.y;
        state.velX = 0;
        state.velY = 0;
        levelState.isDead = false;
        return;
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
        state.posX = levelState.level.spawnPoint.x;
        state.posY = levelState.level.spawnPoint.y;
        state.velX = 0;
        levelState.isDead = false;
        }
    }
}

// called inside the !windowShouldClose loop in main.c every frame, if currentScreen is set to SCREEN_LEVEL
void LevelScreenMainLoop(RenderTexture2D target) {
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

#endif