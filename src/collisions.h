#ifndef COLLISIONS_H
#define COLLISIONS_H

#include "raylib.h"
#include "consts.h"
#include "types.h"
#include "globals.h"

#define max(a, b) ((a)>(b)? (a) : (b))
#define min(a, b) ((a)<(b)? (a) : (b))

// checks if a line has a collision with another line
bool CheckCollisionLineLine(Line a, Line b) {
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


bool CheckCollisionLinePoint(Vector2 point, Line line) {
    float buffer = 10; 
    float lineLen = Vector2Distance(line.start, line.end);
    float d1 = Vector2Distance(point, line.start);
    float d2 = Vector2Distance(point, line.end);

    if (d1+d2 >= lineLen-buffer && d1+d2 <= lineLen+buffer) {
        return true;
    }
    return false;
}


bool CheckCollisionCircleLine(Vector2 circlePos, float radius, Line line) {
    bool insideStart = CheckCollisionPointCircle(line.start, circlePos, radius);
    bool insideEnd = CheckCollisionPointCircle(line.end, circlePos, radius);

    if (insideStart || insideEnd) {
        return true;
    }

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

    bool onSegment = CheckCollisionLinePoint((Vector2){closestX, closestY}, line);
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

bool CheckCollisionLineObsWrench(LineObstacle obs) {
    if (state.isSpinning) { // collider is circle with r = WRENCH_WIDTH/2
        return CheckCollisionCircleLine((Vector2){state.posX-(WRENCH_WIDTH/2), state.posY}, WRENCH_WIDTH/2, obs.line);
    } else if (state.isJumping) { // collider is rect with h = WRENCH_WIDTH/2, w = WRENCH_THICKNESS
        Rectangle collider = {
            state.posX-(WRENCH_WIDTH/2)-(WRENCH_THICKNESS/2),
            state.posY - (WRENCH_THICKNESS/2),
            WRENCH_THICKNESS,
            WRENCH_WIDTH/2,  
        };
        Line rectangleLines[4] = {(Vector2){0, 0}};

        GetLinesOfRectangle(rectangleLines, collider);

        bool left, top, right, bottom;

        left = CheckCollisionLineLine(rectangleLines[0], obs.line);
        top = CheckCollisionLineLine(rectangleLines[1], obs.line);
        right = CheckCollisionLineLine(rectangleLines[2], obs.line);
        bottom = CheckCollisionLineLine(rectangleLines[3], obs.line);

        if (left || top || right || bottom) {
            return true;
        }
        return false;

    } else { // collider is rect with h = WRENCH_THICKNESS, w = WRENCH_WIDTH
        Rectangle collider = {
            state.posX-WRENCH_WIDTH,
            state.posY - (WRENCH_THICKNESS/2),
            WRENCH_WIDTH,
            WRENCH_THICKNESS  
        };

        Line rectangleLines[4] = {(Vector2){0, 0}};

        GetLinesOfRectangle(rectangleLines, collider);

        bool left, top, right, bottom;

        left = CheckCollisionLineLine(rectangleLines[0], obs.line);
        top = CheckCollisionLineLine(rectangleLines[1], obs.line);
        right = CheckCollisionLineLine(rectangleLines[2], obs.line);
        bottom = CheckCollisionLineLine(rectangleLines[3], obs.line);

        if (left || top || right || bottom) {
            return true;
        }
        return false;
    }
}

#endif