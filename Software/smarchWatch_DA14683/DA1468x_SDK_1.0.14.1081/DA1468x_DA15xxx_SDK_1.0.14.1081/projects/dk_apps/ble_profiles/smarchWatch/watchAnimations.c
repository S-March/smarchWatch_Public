/*
 * watchAnimations.c
 *
 *  Created on: Jan 22, 2019
 *      Author: samsonm
 */

#include "watchAnimations.h"
#include "displayDriver.h"
#include "math.h"

void displayDrawWatchHand(int RADIUS, int ANGLE, int HAND_COLOR)
{
    int baseAngleLeft = ANGLE-90;
    int baseAngleRight = ANGLE+90;
    int baseLength = RADIUS/25;

    displayDrawLinePolarThickness(WATCH_CENTER,WATCH_CENTER,RADIUS,ANGLE,HAND_COLOR,RADIUS/20);


    float newAngleLeft = 3.1416*baseAngleLeft/180;
    float newAngleRight = 3.1416*baseAngleRight/180;
    float pointAngle = 2;

    int baseAngleLeftX = WATCH_CENTER+baseLength*cos(newAngleLeft);
    int baseAngleLeftY = WATCH_CENTER+baseLength*sin(newAngleLeft);
    displayDrawLinePolarThickness(baseAngleLeftX,baseAngleLeftY,RADIUS,ANGLE+pointAngle,HAND_COLOR,RADIUS/60);
    int baseAngleRightX = WATCH_CENTER+baseLength*cos(newAngleRight);
    int baseAngleRightY = WATCH_CENTER+baseLength*sin(newAngleRight);
    displayDrawLinePolarThickness(baseAngleRightX,baseAngleRightY,RADIUS,ANGLE-pointAngle,HAND_COLOR,RADIUS/60);
}


void displayDrawSecondWatchHand(int RADIUS, int ANGLE, int HAND_COLOR, int X_CENTER, int Y_CENTER)
{
    int baseAngleLeft = ANGLE-90;
    int baseAngleRight = ANGLE+90;
    int baseLength = RADIUS/25;

    displayDrawLinePolarThickness(X_CENTER,Y_CENTER,RADIUS,ANGLE,HAND_COLOR,RADIUS/20);

    float newAngleLeft = 3.1416*baseAngleLeft/180;
    float newAngleRight = 3.1416*baseAngleRight/180;

    float pointAngle = 2;

    int baseAngleLeftX = X_CENTER+baseLength*cos(newAngleLeft);
    int baseAngleLeftY = Y_CENTER+baseLength*sin(newAngleLeft);
    displayDrawLinePolarThickness(baseAngleLeftX,baseAngleLeftY,RADIUS,ANGLE+pointAngle,HAND_COLOR,RADIUS/60);
    int baseAngleRightX = X_CENTER+baseLength*cos(newAngleRight);
    int baseAngleRightY = Y_CENTER+baseLength*sin(newAngleRight);
    displayDrawLinePolarThickness(baseAngleRightX,baseAngleRightY,RADIUS,ANGLE-pointAngle,HAND_COLOR,RADIUS/60);
}
//void displayClearWatchHandBMP(int RADIUS, int ANGLE, char *FILENAME, int NAME_SIZE)
//{
//    int baseAngleLeft = ANGLE-90;
//    int baseAngleRight = ANGLE+90;
//    int baseLength = RADIUS/25;
//
//    displayDrawLinePolarThicknessOfBMP(WATCH_CENTER,WATCH_CENTER,RADIUS,ANGLE,RADIUS/20,FILENAME,NAME_SIZE);
//
//
//    float newAngleLeft = 3.1416*baseAngleLeft/180;
//    float newAngleRight = 3.1416*baseAngleRight/180;
//    float pointAngle = 2;
//
//    int baseAngleLeftX = WATCH_CENTER+baseLength*cos(newAngleLeft);
//    int baseAngleLeftY = WATCH_CENTER+baseLength*sin(newAngleLeft);
//    displayDrawLinePolarThicknessOfBMP(baseAngleLeftX,baseAngleLeftY,RADIUS,ANGLE+pointAngle,RADIUS/60,FILENAME,NAME_SIZE);
//    int baseAngleRightX = WATCH_CENTER+baseLength*cos(newAngleRight);
//    int baseAngleRightY = WATCH_CENTER+baseLength*sin(newAngleRight);
//    displayDrawLinePolarThicknessOfBMP(baseAngleRightX,baseAngleRightY,RADIUS,ANGLE-pointAngle,RADIUS/60,FILENAME,NAME_SIZE);
//}


//void displayClearSecondWatchHandBMP(int RADIUS, int ANGLE, int X_CENTER, int Y_CENTER, char *FILENAME, int NAME_SIZE)
//{
//    int baseAngleLeft = ANGLE-90;
//    int baseAngleRight = ANGLE+90;
//    int baseLength = RADIUS/25;
//
//    displayDrawLinePolarThicknessOfBMP(X_CENTER,Y_CENTER,RADIUS,ANGLE,RADIUS/20,FILENAME,NAME_SIZE);
//
//
//    float newAngleLeft = 3.1416*baseAngleLeft/180;
//    float newAngleRight = 3.1416*baseAngleRight/180;
//    float pointAngle = 2;
//
//    int baseAngleLeftX = X_CENTER+baseLength*cos(newAngleLeft);
//    int baseAngleLeftY = Y_CENTER+baseLength*sin(newAngleLeft);
//    displayDrawLinePolarThicknessOfBMP(baseAngleLeftX,baseAngleLeftY,RADIUS,ANGLE+pointAngle,RADIUS/60,FILENAME,NAME_SIZE);
//    int baseAngleRightX = X_CENTER+baseLength*cos(newAngleRight);
//    int baseAngleRightY = Y_CENTER+baseLength*sin(newAngleRight);
//    displayDrawLinePolarThicknessOfBMP(baseAngleRightX,baseAngleRightY,RADIUS,ANGLE-pointAngle,RADIUS/60,FILENAME,NAME_SIZE);
//}

void displayDrawWatchFace(int BACKGROUND_COLOR, int TICK_COLOR)
{
    displayFillScreen(BACKGROUND_COLOR);
    for(int tickAngle=0;tickAngle<360;tickAngle+=30)
    {
        displayDrawLinePolarThickness(WATCH_CENTER,WATCH_CENTER,WATCH_CENTER,tickAngle,TICK_COLOR,WATCH_CENTER/30);
        displayDrawLinePolarThickness(WATCH_CENTER,WATCH_CENTER,WATCH_CENTER-TICK_LENGTH,tickAngle,BACKGROUND_COLOR,WATCH_CENTER/20);
    }
}
void displayDrawWatchNumbers(int BACKGROUND_COLOR, int NUMBER_COLOR)
{
    int offsetFromEdge = 25;
    int polarX = 0;
    int polarY = 0;
    float polarAngle = 0;
    displayFillScreen(BACKGROUND_COLOR);
//    displayDrawCircle(WATCH_CENTER,WATCH_CENTER,WATCH_CENTER-offsetFromEdge-24,NUMBER_COLOR);
//    displayDrawCircle(WATCH_CENTER,WATCH_CENTER,WATCH_CENTER-offsetFromEdge-25,NUMBER_COLOR);
//    displayDrawCircle(WATCH_CENTER,WATCH_CENTER,WATCH_CENTER-offsetFromEdge-26,NUMBER_COLOR);

    //XII
    displayDrawLineThickness(110, WATCH_CENTER, offsetFromEdge, offsetFromEdge+10, NUMBER_COLOR, 3);
    displayDrawLineThickness(110, WATCH_CENTER, offsetFromEdge+10, offsetFromEdge, NUMBER_COLOR, 3);
    displayDrawLineThickness(125, 125, offsetFromEdge, offsetFromEdge+10, NUMBER_COLOR, 3);
    displayDrawLineThickness(130, 130, offsetFromEdge, offsetFromEdge+10, NUMBER_COLOR, 3);
    //I
    polarAngle = 3.1416*300/180;
    polarX = WATCH_CENTER+(120-offsetFromEdge)*cos(polarAngle);
    polarY = WATCH_CENTER+(120-offsetFromEdge)*sin(polarAngle);
    displayDrawLineThickness(polarX, polarX, polarY-5, polarY+5, NUMBER_COLOR, 3);
    //II
    polarAngle = 3.1416*330/180;
    polarX = WATCH_CENTER+(120-offsetFromEdge)*cos(polarAngle);
    polarY = WATCH_CENTER+(120-offsetFromEdge)*sin(polarAngle);
    displayDrawLineThickness(polarX-5, polarX-5, polarY-5, polarY+5, NUMBER_COLOR, 3);
    displayDrawLineThickness(polarX, polarX, polarY-5, polarY+5, NUMBER_COLOR, 3);
    //III
    displayDrawLineThickness(230-offsetFromEdge, 230-offsetFromEdge, 115, 125, NUMBER_COLOR, 3);
    displayDrawLineThickness(235-offsetFromEdge, 235-offsetFromEdge, 115, 125, NUMBER_COLOR, 3);
    displayDrawLineThickness(240-offsetFromEdge, 240-offsetFromEdge, 115, 125, NUMBER_COLOR, 3);
    //IV
    polarAngle = 3.1416*30/180;
    polarX = WATCH_CENTER+(120-offsetFromEdge)*cos(polarAngle);
    polarY = WATCH_CENTER+(120-offsetFromEdge)*sin(polarAngle);
    displayDrawLineThickness(polarX-10, polarX-10, polarY+5, polarY-5, NUMBER_COLOR, 3);
    displayDrawLineThickness(polarX-5, polarX, polarY-5, polarY+5, NUMBER_COLOR, 3);
    displayDrawLineThickness(polarX, polarX+5, polarY+5, polarY-5, NUMBER_COLOR, 3);
    //V
    polarAngle = 3.1416*60/180;
    polarX = WATCH_CENTER+(120-offsetFromEdge)*cos(polarAngle);
    polarY = WATCH_CENTER+(120-offsetFromEdge)*sin(polarAngle);
    displayDrawLineThickness(polarX-5, polarX, polarY-5, polarY+5, NUMBER_COLOR, 3);
    displayDrawLineThickness(polarX, polarX+5, polarY+5, polarY-5, NUMBER_COLOR, 3);
    //VI
    displayDrawLineThickness(110, 115, 230-offsetFromEdge, 240-offsetFromEdge, NUMBER_COLOR, 3);
    displayDrawLineThickness(115, WATCH_CENTER, 240-offsetFromEdge, 230-offsetFromEdge, NUMBER_COLOR, 3);
    displayDrawLineThickness(130, 130, 230-offsetFromEdge, 240-offsetFromEdge, NUMBER_COLOR, 3);
    //VII
    polarAngle = 3.1416*120/180;
    polarX = WATCH_CENTER+(120-offsetFromEdge)*cos(polarAngle);
    polarY = WATCH_CENTER+(120-offsetFromEdge)*sin(polarAngle);
    displayDrawLineThickness(polarX+5, polarX, polarY-5, polarY+5, NUMBER_COLOR, 3);
    displayDrawLineThickness(polarX, polarX-5, polarY+5, polarY-5, NUMBER_COLOR, 3);
    displayDrawLineThickness(polarX+10, polarX+10, polarY+5, polarY-5, NUMBER_COLOR, 3);
    displayDrawLineThickness(polarX+15, polarX+15, polarY+5, polarY-5, NUMBER_COLOR, 3);
    //VIII
    polarAngle = 3.1416*150/180;
    polarX = WATCH_CENTER+(120-offsetFromEdge)*cos(polarAngle);
    polarY = WATCH_CENTER+(120-offsetFromEdge)*sin(polarAngle);
    displayDrawLineThickness(polarX+5, polarX, polarY-5, polarY+5, NUMBER_COLOR, 3);
    displayDrawLineThickness(polarX, polarX-5, polarY+5, polarY-5, NUMBER_COLOR, 3);
    displayDrawLineThickness(polarX+10, polarX+10, polarY+5, polarY-5, NUMBER_COLOR, 3);
    displayDrawLineThickness(polarX+15, polarX+15, polarY+5, polarY-5, NUMBER_COLOR, 3);
    displayDrawLineThickness(polarX+20, polarX+20, polarY+5, polarY-5, NUMBER_COLOR, 3);
    //IX
    displayDrawLineThickness(offsetFromEdge, offsetFromEdge, 115, 125, NUMBER_COLOR, 3);
    displayDrawLineThickness(5+offsetFromEdge, 15+offsetFromEdge, 115, 125, NUMBER_COLOR, 3);
    displayDrawLineThickness(5+offsetFromEdge, 15+offsetFromEdge, 125, 115, NUMBER_COLOR, 3);
    //X
    polarAngle = 3.1416*210/180;
    polarX = WATCH_CENTER+(120-offsetFromEdge)*cos(polarAngle);
    polarY = WATCH_CENTER+(120-offsetFromEdge)*sin(polarAngle);
    displayDrawLineThickness(polarX-5, polarX+5, polarY-5, polarY+5, NUMBER_COLOR, 3);
    displayDrawLineThickness(polarX-5, polarX+5, polarY+5, polarY-5, NUMBER_COLOR, 3);
    //XI
    polarAngle = 3.1416*240/180;
    polarX = WATCH_CENTER+(120-offsetFromEdge)*cos(polarAngle);
    polarY = WATCH_CENTER+(120-offsetFromEdge)*sin(polarAngle);
    displayDrawLineThickness(polarX-5, polarX+5, polarY-5, polarY+5, NUMBER_COLOR, 3);
    displayDrawLineThickness(polarX-5, polarX+5, polarY+5, polarY-5, NUMBER_COLOR, 3);
    displayDrawLineThickness(polarX+10, polarX+10, polarY+5, polarY-5, NUMBER_COLOR, 3);
}
