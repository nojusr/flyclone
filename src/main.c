#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#define max(a, b) ((a)>(b)? (a) : (b))
#define min(a, b) ((a)<(b)? (a) : (b))
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

// CONSTANTS -------------------------------------------------------------------

// game related
#define DEBUG true
#define SCREEN_WIDTH 852
#define SCREEN_HEIGHT 480
#define GRAVITY 9.8
#define HORIZONTAL_DRAG 0.95 // between 0 and 1

// wrench related
#define JUMP_STRENGTH 350
#define FLY_SPEED 75
#define MAX_FLY_SPEED 500
#define WRENCH_WIDTH 50
#define WRENCH_THICKNESS 10
#define WRENCH_MAX_ANGLE 30
#define WRENCH_SPIN_SPEED 35

// limits
#define MAX_LINES 2048
#define CAM_SMOOTH_MAX_DATA 200
#define CAM_SMOOTH_AVG_COUNT 30

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

// STRUCTS ---------------------------------------------------------------------
 
typedef struct {
    double posX;
    double posY;
    double velX;
    double velY;
    bool isJumping;
    bool isSpinning;
} WrenchState;


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


// GLOBALS ---------------------------------------------------------------------

WrenchState lastState; // global state of the last frame
WrenchState state; // global state for wrench/player
float deltaTime = 0.0; // gets updated every frame
double spinningWrenchAngle = 0; // used to animate the wrench spinning

// related to camera
Camera2D camera; // main camera global
double floatingAvgSetX[CAM_SMOOTH_MAX_DATA];
double floatingAvgSetY[CAM_SMOOTH_MAX_DATA];
int currentAvgIndex;

// related to levels
LineObstacle loadedLines[MAX_LINES];
int loadedLineCount = 0;

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


// TODO: REFACTOR COLLISION LOGIC TO BETTER FIT WITH RAYLIB.
// MOVE COLLISION LOGIC OUT TO SEPARATE FILE

// checks if a line has a collision with another line
bool IsLineCollidingWithLine(Line a, Line b) {
    float x1, y1, x2, y2, x3, y3, x4, y4;
    x1 = a.start.x;
    x2 = a.end.x;
    x3 = b.start.x;
    x4 = b.end.x;
    y1 = a.start.y;
    y2 = a.end.y;
    y3 = b.start.y;
    y4 = b.end.y;

    // calculate the distance to intersection point
    float uA = ((x4-x3)*(y1-y3) - (y4-y3)*(x1-x3)) / ((y4-y3)*(x2-x1) - (x4-x3)*(y2-y1));
    float uB = ((x2-x1)*(y1-y3) - (y2-y1)*(x1-x3)) / ((y4-y3)*(x2-x1) - (x4-x3)*(y2-y1));   

    // if uA and uB are between 0-1, lines are colliding
    if (uA >= 0 && uA <= 1 && uB >= 0 && uB <= 1) {
    return true;
    }
    return false;
}


bool IsLineCollidingWithPoint(Vector2 point, Line line) {
    float buffer = 0.1; 
    float lineLen = Vector2Distance(line.start, line.end);
    float d1 = Vector2Distance(point, line.start);
    float d2 = Vector2Distance(point, line.end);

    if (d1+d2 >= lineLen-buffer && d1+d2 <= lineLen+buffer) {
        return true;
    }
    return false;
}


bool IsCircleCollidingWithLine(Vector2 circlePos, float radius, Line line) {
    bool insideStart = CheckCollisionPointCircle(line.start, circlePos, radius);
    bool insideEnd = CheckCollisionPointCircle(line.end, circlePos, radius);

    if (insideStart || insideEnd) {
        return true;
    }

    // MAP:
    // x1 line.s.x
    // y1 line.s.y
    // x2 line.e.x
    // y2 line.e.y

    float cx, cy;
    cx = circlePos.x;
    cy = circlePos.y;

    float x1, y1, x2, y2;
    x1 = line.start.x;
    y1 = line.start.y;
    x2 = line.end.x;
    y2 = line.end.y;

    float distX = x1 - x2;
    float distY = y1 - y2;
    float len = sqrt( (distX*distX) + (distY*distY) );

    float dot = ( ((cx-x1)*(x2-x1)) + ((cy-y1)*(y2-y1)) ) / pow(len,2);

    float closestX = x1 + (dot * (x2-x1));
    float closestY = y1 + (dot * (y2-y1));

    bool onSegment = IsLineCollidingWithPoint((Vector2){closestX, closestY}, line);
    if (!onSegment) {
        return false;
    }

    distX = closestX - cx;
    distY = closestY - cy;
    float distance = sqrt( (distX*distX) + (distY*distY) );

    if (distance <= radius) {
        return true;
    }

    return false;
}

// gets all lines of a rectangle, puts em in an array
// order is: left, top, right, bottom
void GetLinesOfRectangle (Line *output, Rectangle input) {

    double leftX = input.x-(input.width/2);
    double rightX = input.x+(input.width/2);    

    double bottomY = input.y+(input.height/2);
    double topY = input.y-(input.height/2);


    output[0] = (Line){(Vector2){leftX, bottomY},(Vector2){leftX, topY}};
    output[1] = (Line){(Vector2){leftX, topY},(Vector2){rightX, topY}};
    output[2] = (Line){(Vector2){rightX, topY},(Vector2){rightX, bottomY}};
    output[3] = (Line){(Vector2){rightX, bottomY},(Vector2){leftX, bottomY}};
    return;
}

bool IsLineCollidingWithWrench(LineObstacle obs) {
    if (state.isSpinning) { // collider is circle with r = WRENCH_WIDTH/2
        return IsCircleCollidingWithLine((Vector2){state.posX, state.posY}, WRENCH_WIDTH/2, obs.line);
    } else if (state.isJumping) { // collider is rect with h = WRENCH_WIDTH/2, w = WRENCH_THICKNESS
        Rectangle collider = {
            state.posY,
            state.posX - (WRENCH_THICKNESS/2),
            WRENCH_THICKNESS,
            WRENCH_WIDTH/2,  
        };

        Line rectangleLines[4] = {(Vector2){0, 0}};

        GetLinesOfRectangle(rectangleLines, collider);

        bool left, top, right, bottom;

        left = IsLineCollidingWithLine(rectangleLines[0], obs.line);
        top = IsLineCollidingWithLine(rectangleLines[1], obs.line);
        right = IsLineCollidingWithLine(rectangleLines[2], obs.line);
        bottom = IsLineCollidingWithLine(rectangleLines[3], obs.line);

        if (left || top || right || bottom) {
            return true;
        }
        return false;

    } else { // collider is rect with h = WRENCH_THICKNESS, w = WRENCH_WIDTH
        Rectangle collider = {
            state.posY,
            state.posX - (WRENCH_THICKNESS/2),
            WRENCH_WIDTH,
            WRENCH_THICKNESS  
        };

        Line rectangleLines[4] = {(Vector2){0, 0}};

        GetLinesOfRectangle(rectangleLines, collider);

        bool left, top, right, bottom;

        left = IsLineCollidingWithLine(rectangleLines[0], obs.line);
        top = IsLineCollidingWithLine(rectangleLines[1], obs.line);
        right = IsLineCollidingWithLine(rectangleLines[2], obs.line);
        bottom = IsLineCollidingWithLine(rectangleLines[3], obs.line);

        if (left || top || right || bottom) {
            return true;
        }
        return false;
    }
}

bool isWrenchCollidingWithLines() {
    for (int i = 0; i < loadedLineCount; i++) {
        if (IsLineCollidingWithWrench(loadedLines[i])) {
            return true;
        }
    }
    return false;
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
    for (int i = 0; i < loadedLineCount; i++) {
        LineObstacle obs = loadedLines[i];
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
            4.0,
            BLACK
        );
    }

}

// DEBUG -----------------------------------------------------------------------

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


// just an ever-changing function used for testing lineObstacles in levels
void LoadObstacleDummyData() {
    
    int totalLines = 16;
    
    Vector2 tmpStartVec;
    Vector2 tmpEndVec;
    LineObstacle tmp;

    tmpStartVec.x = 200;
    tmpStartVec.y = 300;
    tmpEndVec.x = 500;
    tmpEndVec.y = 10;
    tmp.line.start = tmpStartVec;
    tmp.line.end = tmpEndVec;
    tmp.type = LINE_YELLOW;
    loadedLines[loadedLineCount] = tmp;
    loadedLineCount++;

    tmpStartVec.x = 200;
    tmpStartVec.y = 300;
    tmpEndVec.x = -200;
    tmpEndVec.y = 300;
    tmp.line.start = tmpStartVec;
    tmp.line.end = tmpEndVec;
    tmp.type = LINE_YELLOW;
    loadedLines[loadedLineCount] = tmp;
    loadedLineCount++;

    tmpStartVec.x = -200;
    tmpStartVec.y = 300;
    tmpEndVec.x = -500;
    tmpEndVec.y = 10;
    tmp.line.start = tmpStartVec;
    tmp.line.end = tmpEndVec;
    tmp.type = LINE_YELLOW;
    loadedLines[loadedLineCount] = tmp;
    loadedLineCount++;
}

// CAMERA RELATED --------------------------------------------------------------

void UpdateCameraFromWrenchState() {
//    double lockedCameraX = state.posX-((double)screenWidth)/2;
//    double lockedCameraY = state.posY-((double)screenHeight)/2;

    double lockedCameraX = state.posX-((double)SCREEN_WIDTH)/2;
    double lockedCameraY = state.posY-((double)SCREEN_HEIGHT)/2;

    double velModX = lockedCameraX + (state.velX*0.7);
    double velModY = lockedCameraY + (state.velY*0.25);

    currentAvgIndex++;

    if (currentAvgIndex > CAM_SMOOTH_MAX_DATA) {
        currentAvgIndex = 0;
    }
    floatingAvgSetX[currentAvgIndex] = velModX;
    floatingAvgSetY[currentAvgIndex] = velModY;

    double xSum = 0;
    double ySum = 0;

    for (int i = currentAvgIndex; i > currentAvgIndex-CAM_SMOOTH_AVG_COUNT; i--) {
        int j = i;
        if (j < 0) {
            j += CAM_SMOOTH_MAX_DATA;
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
    } if (IsKeyDown(KEY_Q) && DEBUG) {
        exit(0);
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
        // DEBUG ---------------------------------------------------------------
        if (DEBUG) {
            //PrintWrenchState();
            if (isWrenchCollidingWithLines()) {
                printf("COLLISION!!!\n");
            }
        }

        
        // CAMERA PASS ---------------------------------------------------------
        UpdateCameraFromWrenchState();
    

        // LOGIC PASS ----------------------------------------------------------
        deltaTime = GetFrameTime();
        lastState = state;

        ApplyGravityToWrench();
        ApplyDragToWrench();

        // temporary hack so that the wrench doesn't fly out of screen
        if (state.posY >= 500) {
            state.posY = 500;
            state.velY = 0;
        }

        HandleInput();   
        SetWrenchPositionFromVelocity();

        // DRAW PASS -----------------------------------------------------------
        BeginDrawing();
            ClearBackground(BLACK);

            BeginTextureMode(target);
                ClearBackground(BLACK);

                BeginMode2D(camera);
                    DrawLevelLines();
                    DrawWrench();
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
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
