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

typedef enum SCREEN {
    SCREEN_MAIN_MENU,
    SCREEN_EDITOR,
    SCREEN_LEVEL,
} SCREEN;

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
    Vector2 spawnPoint;
    Vector2 finishPoint;
    LineObstacle lines[MAX_LINES];
    int lineCount;
} Level;

typedef struct {
    bool isDead;
    bool levelLoaded;
    int tries;
    Level level;
} LevelScreenState;

typedef struct {
    float screenRatio;
    Vector2 mousePos;
    Level level;
    Vector2 cameraPos;
    float cameraZoom;
    bool isDragging;
    Vector2 mouseDragStartPos;
    Vector2 cameraDragStartInitialPos;
    bool isPanning;
    bool isDrawingLine;
    int currentlyEditingLineIndex;
    
} EditorScreenState;



#endif