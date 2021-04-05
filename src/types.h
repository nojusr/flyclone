#ifndef TYPES_H
#define TYPES_H

#include "raylib.h"

typedef struct {
    Vector2 start;
    Vector2 end;
} Line;

typedef enum LINE_TYPE {
    LINE_RED, // wrench has to be in "jump mode"
    LINE_YELLOW, // allows boincing off of if in "spinmode", otherwise kills 
    LINE_GREEN, // kills unless ship is in "spin mode"
    LINE_WHITE // kills unless ship is in "neutral mode"
} LINE_TYPE;

typedef struct {
    LINE_TYPE type;
    Line line;
} LineObstacle;

typedef struct {
    double posX;
    double posY;
    double velX;
    double velY;
    bool isJumping;
    bool isSpinning;
} WrenchState;

typedef struct {
    bool isDead;
    bool levelLoaded;
    int tries;
    Vector2 spawnPoint;
    Vector2 finishPoint;
    LineObstacle loadedLines[MAX_LINES];
    int loadedLineCount;
} LevelState; // TEMPTEMPTEMP

#endif