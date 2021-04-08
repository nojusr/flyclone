// various debugging functions
#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include "globals.h"
#include "types.h"

void PrintWrenchState() {
    printf("-----------------------------------\n");
    printf(
        "posX: %f || posY: %f || velX: %f || velY: %f\n ",
        state.posX,
        state.posY,
        state.velX,
        state.velY
    );
}

void DrawColliders() {

    static Color colliderColor = (Color) {
        255,
        0,
        0,
        64
    };


    if (state.isSpinning) { // collider is circle with r = WRENCH_WIDTH/2
        //return CheckCollisionCircleLine((Vector2){state.posX, state.posY}, WRENCH_WIDTH/2, obs.line);
        DrawCircle(
            state.posX-(WRENCH_WIDTH/2),
            state.posY,
            WRENCH_WIDTH/2,
            colliderColor
        );
    
    } else if (state.isJumping) { // collider is rect with h = WRENCH_WIDTH/2, w = WRENCH_THICKNESS
        Rectangle collider = {
            state.posX-(WRENCH_WIDTH/2)-(WRENCH_THICKNESS/2),
            state.posY - (WRENCH_THICKNESS/2),
            WRENCH_THICKNESS,
            WRENCH_WIDTH/2,  
        };

        DrawRectangleRec(collider, colliderColor);

    } else { // collider is rect with h = WRENCH_THICKNESS, w = WRENCH_WIDTH
        Rectangle collider = {
            state.posX-WRENCH_WIDTH,
            state.posY - (WRENCH_THICKNESS/2),
            WRENCH_WIDTH,
            WRENCH_THICKNESS  
        };

        DrawRectangleRec(collider, colliderColor);
    }
}

// just an ever-changing function used for testing lineObstacles in levels
Level LoadObstacleDummyData() {
    
    //int totalLines = 16;
    
    Vector2 tmpStartVec;
    Vector2 tmpEndVec;
    LineObstacle tmp;

    Level dummyLevel;
    dummyLevel.lineCount = 0;
    dummyLevel.spawnPoint = (Vector2){0.0, 0.0};

    tmpStartVec.x = 200;
    tmpStartVec.y = 300;
    tmpEndVec.x = 500;
    tmpEndVec.y = 10;
    tmp.line.start = tmpStartVec;
    tmp.line.end = tmpEndVec;
    tmp.type = LINE_YELLOW;
    dummyLevel.lines[dummyLevel.lineCount] = tmp;
    dummyLevel.lineCount++;

    tmpStartVec.x = 200;
    tmpStartVec.y = 300;
    tmpEndVec.x = -200;
    tmpEndVec.y = 300;
    tmp.line.start = tmpStartVec;
    tmp.line.end = tmpEndVec;
    tmp.type = LINE_YELLOW;
    dummyLevel.lines[dummyLevel.lineCount] = tmp;
    dummyLevel.lineCount++;

    tmpStartVec.x = -200;
    tmpStartVec.y = 300;
    tmpEndVec.x = -500;
    tmpEndVec.y = 10;
    tmp.line.start = tmpStartVec;
    tmp.line.end = tmpEndVec;
    tmp.type = LINE_YELLOW;
    dummyLevel.lines[dummyLevel.lineCount] = tmp;
    dummyLevel.lineCount++;

    return dummyLevel;
}

#endif