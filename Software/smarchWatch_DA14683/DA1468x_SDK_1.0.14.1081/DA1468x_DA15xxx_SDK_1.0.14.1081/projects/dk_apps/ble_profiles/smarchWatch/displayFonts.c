/*
 * displayFonts.c

 *
 *  Created on: Jan 25, 2019
 *      Author: samsonm
 */

#include "displayFonts.h"
#include "displayDriver.h"
#include "imageOffsets.h"
#include "math.h"

int displayDrawCharacter(int X_START, int Y_START, char CHARACTER)
{
    int letterWidth = 0;
    switch(CHARACTER)
    {
        case 'A':
                displayPartialImageFromMemory(X_START,Y_START,0,0,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 10;
                break;
        case 'a':
                displayPartialImageFromMemory(X_START,Y_START,0,50,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 8;
                break;
        case 'B':
                displayPartialImageFromMemory(X_START,Y_START,15,0,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 9;
                break;
        case 'b':
                displayPartialImageFromMemory(X_START,Y_START,15,50,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 8;
                break;
        case 'C':
                displayPartialImageFromMemory(X_START,Y_START,30,0,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 10;
                break;
        case 'c':
                displayPartialImageFromMemory(X_START,Y_START,30,50,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 7;
                break;
        case 'D':
                displayPartialImageFromMemory(X_START,Y_START,45,0,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 9;
                break;
        case 'd':
                displayPartialImageFromMemory(X_START,Y_START,45,50,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 8;
                break;
        case 'E':
                displayPartialImageFromMemory(X_START,Y_START,60,0,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 7;
                break;
        case 'e':
                displayPartialImageFromMemory(X_START,Y_START,60,50,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 8;
                break;
        case 'F':
                displayPartialImageFromMemory(X_START,Y_START,75,0,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 6;
                break;
        case 'f':
                displayPartialImageFromMemory(X_START,Y_START,75,50,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 5;
                break;
        case 'G':
                displayPartialImageFromMemory(X_START,Y_START,90,0,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 10;
                break;
        case 'g':
                displayPartialImageFromMemory(X_START,Y_START,90,50,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 9;
                break;
        case 'H':
                displayPartialImageFromMemory(X_START,Y_START,105,0,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 10;
                break;
        case 'h':
                displayPartialImageFromMemory(X_START,Y_START,105,50,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 7;
                break;
        case 'I':
                displayPartialImageFromMemory(X_START,Y_START,120,0,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 2;
                break;
        case 'i':
                displayPartialImageFromMemory(X_START,Y_START,120,50,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 2;
                break;
        case 'J':
                displayPartialImageFromMemory(X_START,Y_START,135,0,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 5;
                break;
        case 'j':
                displayPartialImageFromMemory(X_START,Y_START,135,50,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 4;
                break;
        case 'K':
                displayPartialImageFromMemory(X_START,Y_START,150,0,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 10;
                break;
        case 'k':
                displayPartialImageFromMemory(X_START,Y_START,150,50,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 9;
                break;
        case 'L':
                displayPartialImageFromMemory(X_START,Y_START,165,0,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 7;
                break;
        case 'l':
                displayPartialImageFromMemory(X_START,Y_START,165,50,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 2;
                break;
        case 'M':
                displayPartialImageFromMemory(X_START,Y_START,180,0,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 12;
                break;
        case 'm':
                displayPartialImageFromMemory(X_START,Y_START,180,50,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 14;
                break;
        case 'N':
                displayPartialImageFromMemory(X_START,Y_START,0,25,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 10;
                break;
        case 'n':
                displayPartialImageFromMemory(X_START,Y_START,0,75,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 8;
                break;
        case 'O':
                displayPartialImageFromMemory(X_START,Y_START,15,25,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 11;
                break;
        case 'o':
                displayPartialImageFromMemory(X_START,Y_START,15,75,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 8;
                break;
        case 'P':
                displayPartialImageFromMemory(X_START,Y_START,30,25,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 9;
                break;
        case 'p':
                displayPartialImageFromMemory(X_START,Y_START,30,75,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 8;
                break;
        case 'Q':
                displayPartialImageFromMemory(X_START,Y_START,45,25,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 11;
                break;
        case 'q':
                displayPartialImageFromMemory(X_START,Y_START,45,75,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 8;
                break;
        case 'R':
                displayPartialImageFromMemory(X_START,Y_START,60,25,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 9;
                break;
        case 'r':
                displayPartialImageFromMemory(X_START,Y_START,60,75,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 5;
                break;
        case 'S':
                displayPartialImageFromMemory(X_START,Y_START,75,25,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 9;
                break;
        case 's':
                displayPartialImageFromMemory(X_START,Y_START,75,75,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 7;
                break;
        case 'T':
                displayPartialImageFromMemory(X_START,Y_START,90,25,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 9;
                break;
        case 't':
                displayPartialImageFromMemory(X_START,Y_START,90,75,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 6;
                break;
        case 'U':
                displayPartialImageFromMemory(X_START,Y_START,105,25,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 9;
                break;
        case 'u':
                displayPartialImageFromMemory(X_START,Y_START,105,75,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 8;
                break;
        case 'V':
                displayPartialImageFromMemory(X_START,Y_START,120,25,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 11;
                break;
        case 'v':
                displayPartialImageFromMemory(X_START,Y_START,120,75,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 8;
                break;
        case 'W':
                displayPartialImageFromMemory(X_START,Y_START,135,25,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 15;
                break;
        case 'w':
                displayPartialImageFromMemory(X_START,Y_START,135,75,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 12;
                break;
        case 'X':
                displayPartialImageFromMemory(X_START,Y_START,150,25,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 10;
                break;
        case 'x':
                displayPartialImageFromMemory(X_START,Y_START,150,75,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 8;
                break;
        case 'Y':
                displayPartialImageFromMemory(X_START,Y_START,165,25,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 11;
                break;
        case 'y':
                displayPartialImageFromMemory(X_START,Y_START,165,75,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 8;
                break;
        case 'Z':
                displayPartialImageFromMemory(X_START,Y_START,180,25,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 8;
                break;
        case 'z':
                displayPartialImageFromMemory(X_START,Y_START,180,75,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 6;
                break;
        case '1':
                displayPartialImageFromMemory(X_START,Y_START,0,100,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 5;
                break;
        case '2':
                displayPartialImageFromMemory(X_START,Y_START,15,100,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 8;
                break;
        case '3':
                displayPartialImageFromMemory(X_START,Y_START,30,100,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 8;
                break;
        case '4':
                displayPartialImageFromMemory(X_START,Y_START,45,100,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 9;
                break;
        case '5':
                displayPartialImageFromMemory(X_START,Y_START,60,100,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 8;
                break;
        case '6':
                displayPartialImageFromMemory(X_START,Y_START,75,100,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 9;
                break;
        case '7':
                displayPartialImageFromMemory(X_START,Y_START,90,100,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 7;
                break;
        case '8':
                displayPartialImageFromMemory(X_START,Y_START,105,100,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 8;
                break;
        case '9':
                displayPartialImageFromMemory(X_START,Y_START,120,100,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 9;
                break;
        case '0':
                displayPartialImageFromMemory(X_START,Y_START,135,100,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 9;
                break;
        case '.':
                displayPartialImageFromMemory(X_START,Y_START,0,125,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 2;
                break;
        case ',':
                displayPartialImageFromMemory(X_START,Y_START,15,125,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 2;
                break;
        case '!':
                displayPartialImageFromMemory(X_START,Y_START,30,125,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 2;
                break;
        case '?':
                displayPartialImageFromMemory(X_START,Y_START,45,125,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 8;
                break;
        case ':':
                displayPartialImageFromMemory(X_START,Y_START,60,125,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 2;
                break;
        case '/':
                displayPartialImageFromMemory(X_START,Y_START,75,125,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 6;
                break;
        case '"':
                displayPartialImageFromMemory(X_START,Y_START,90,125,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 6;
                break;
        case '\'':
                displayPartialImageFromMemory(X_START,Y_START,105,125,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 2;
                break;
        case '(':
                displayPartialImageFromMemory(X_START,Y_START,120,125,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 4;
                break;
        case ')':
                displayPartialImageFromMemory(X_START,Y_START,135,125,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 4;
                break;
        case '#':
                displayPartialImageFromMemory(X_START,Y_START,150,125,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 9;
                break;
        case '$':
                displayPartialImageFromMemory(X_START,Y_START,165,125,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 9;
                break;
        case '@':
                displayPartialImageFromMemory(X_START,Y_START,180,125,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
                letterWidth = 15;
                break;
        default:
                letterWidth = 5;
        break;
    }
    return letterWidth;
}
int setCircularMargin(int CURRENT_Y_POSITION)
{
    int minimumCircularMargin = 80;
    int xMargin = 0;

    if(CURRENT_Y_POSITION<=20||CURRENT_Y_POSITION>=220)
    {
        xMargin = minimumCircularMargin*9/10;
    }
    else if(CURRENT_Y_POSITION<=40||CURRENT_Y_POSITION>=200)
    {
        xMargin = minimumCircularMargin*6/10;
    }
    else if(CURRENT_Y_POSITION<=60||CURRENT_Y_POSITION>=180)
    {
        xMargin = minimumCircularMargin*4/10;
    }
    else if(CURRENT_Y_POSITION<=80||CURRENT_Y_POSITION>=160)
    {
        xMargin = minimumCircularMargin*3/10;
    }
    else if(CURRENT_Y_POSITION<=100||CURRENT_Y_POSITION>=140)
    {
        xMargin = minimumCircularMargin*2/10;
    }
    else
    {
        xMargin = minimumCircularMargin*1/10;
    }
    return xMargin;
}
void displayDrawString(int X_START, int Y_START, int KERNING_SIZE, int MARGIN, int POINTER_TO_STRING)
{
    char stringToWrite[255]={NULL};
    strncpy(stringToWrite,POINTER_TO_STRING,255);
    int numberOfCharacters = 0;
    int stringLength = strlen(stringToWrite);
    int currentXLocation = X_START;
    int currentYLocation = Y_START;
    int nextSpaceCount = 0;
    int xMargin = 0;
    int letterSize = FONT_CHARACTER_WIDTH;

    int minimumCircularMargin = 20;

    if(Y_START<MARGIN+minimumCircularMargin)
    {
        currentYLocation = MARGIN+minimumCircularMargin;
    }

    xMargin = setCircularMargin(currentYLocation)+MARGIN;

    if(X_START<xMargin)
    {
        currentXLocation = xMargin;
    }
    while(numberOfCharacters!=stringLength)
    {
        if(stringToWrite[numberOfCharacters]==' ')
        {
            nextSpaceCount = 0;
            for(int nextSpaceLocation = 1; nextSpaceLocation<(stringLength-numberOfCharacters);nextSpaceLocation++)
            {
                if(stringToWrite[numberOfCharacters+nextSpaceLocation]==' ')
                {
                    break;
                }
                nextSpaceCount++;
            }
            //Check if the next space comes after the text reaches the edge of the writing area
            if(((currentXLocation+(nextSpaceCount*(FONT_CHARACTER_WIDTH+KERNING_SIZE)))>(ST7789_WIDTH-xMargin))||(nextSpaceCount==(stringLength-numberOfCharacters)))
            {
                numberOfCharacters++;
                currentYLocation += FONT_CHARACTER_HEIGHT;
                if((currentYLocation+FONT_CHARACTER_HEIGHT+MARGIN+minimumCircularMargin)>ST7789_HEIGHT)
                {
                    break;
                }
                xMargin = setCircularMargin(currentYLocation)+MARGIN;
                if(X_START>xMargin)
                {
                    currentXLocation = X_START;
                }
                else
                {
                    currentXLocation = xMargin;
                }
            }
        }
        if(((currentXLocation+(FONT_CHARACTER_WIDTH+KERNING_SIZE)+xMargin)>ST7789_WIDTH)&&(stringToWrite[numberOfCharacters]==' '))
        {
            currentYLocation += FONT_CHARACTER_HEIGHT;
            if((currentYLocation+FONT_CHARACTER_HEIGHT+MARGIN+minimumCircularMargin)>ST7789_HEIGHT)
            {
                break;
            }
            xMargin = setCircularMargin(currentYLocation)+MARGIN;
            if(X_START>xMargin)
            {
                currentXLocation = X_START;
            }
            else
            {
                currentXLocation = xMargin;
            }
        }
        else
        {
            letterSize = displayDrawCharacter(currentXLocation, currentYLocation, stringToWrite[numberOfCharacters]);
            currentXLocation += (letterSize+KERNING_SIZE);
        }
        numberOfCharacters++;
    }
}
/*
void displayDrawCharacter(int X_START, int Y_START, int SIZE, int COLOR, char CHARACTER)
{
    int xStartLocation = X_START;
    int xEndLocation = xStartLocation + SIZE;
    int yStartLocation = Y_START;
    int yEndLocation = yStartLocation + SIZE;
    switch(CHARACTER)
    {
        case 'A':
        case 'a':
            displayDrawLineThickness(xStartLocation,xStartLocation+SIZE/2,yEndLocation,yStartLocation,COLOR,SIZE/5);
            displayDrawLineThickness(xStartLocation+SIZE/2,xEndLocation,yStartLocation,yEndLocation,COLOR,SIZE/5);
            displayDrawLineThickness(xStartLocation+SIZE/3,xStartLocation+SIZE*2/3,yStartLocation+SIZE*2/3,yStartLocation+SIZE*2/3,COLOR,SIZE/5);
        break;
        case 'B':
        case 'b':
            displayDrawLineThickness(xStartLocation,xStartLocation,yStartLocation,yEndLocation,COLOR, SIZE/5);
            displayDrawLineThickness(xStartLocation,xEndLocation-SIZE/3,yStartLocation,yStartLocation,COLOR,SIZE/5);
            displayDrawLineThickness(xEndLocation-SIZE/3,xEndLocation-SIZE/3,yStartLocation,yStartLocation+SIZE/2,COLOR,SIZE/5);
            displayDrawLineThickness(xEndLocation-SIZE/3,xStartLocation,yStartLocation+SIZE/2,yStartLocation+SIZE/2,COLOR,SIZE/5);
            displayDrawLineThickness(xStartLocation+SIZE/2,xEndLocation-SIZE/6,yStartLocation+SIZE/2,yStartLocation+SIZE*4/6,COLOR,SIZE/5);
            displayDrawLineThickness(xEndLocation-SIZE/6,xEndLocation-SIZE/6,yStartLocation+SIZE*4/6,yEndLocation,COLOR,SIZE/5);
            displayDrawLineThickness(xEndLocation-SIZE/6,xStartLocation,yEndLocation,yEndLocation,COLOR,SIZE/5);
        break;
        case 'C':
        case 'c':
            displayDrawLineThickness(xStartLocation,xEndLocation-SIZE/2,yStartLocation+SIZE/6,yStartLocation,COLOR,SIZE/5);
            displayDrawLineThickness(xEndLocation-SIZE/2,xEndLocation,yStartLocation,yStartLocation+SIZE/6,COLOR,SIZE/5);
            displayDrawLineThickness(xStartLocation,xStartLocation,yStartLocation+SIZE/6,yEndLocation-SIZE/6,COLOR, SIZE/5);
            displayDrawLineThickness(xStartLocation,xEndLocation-SIZE/2,yEndLocation-SIZE/6,yEndLocation,COLOR,SIZE/5);
            displayDrawLineThickness(xEndLocation-SIZE/2,xEndLocation,yEndLocation,yEndLocation-SIZE/6,COLOR,SIZE/5);
        break;
        case 'D':
        case 'd':
            displayDrawLineThickness(xStartLocation,xStartLocation,yStartLocation,yEndLocation,COLOR, SIZE/5);
            displayDrawLineThickness(xStartLocation,xEndLocation-SIZE/2,yStartLocation,yStartLocation,COLOR,SIZE/5);
            displayDrawLineThickness(xEndLocation-SIZE/2,xEndLocation,yStartLocation,yStartLocation+SIZE/6,COLOR,SIZE/5);
            displayDrawLineThickness(xEndLocation,xEndLocation,yStartLocation+SIZE/6,yEndLocation-SIZE/6,COLOR,SIZE/5);
            displayDrawLineThickness(xStartLocation,xEndLocation-SIZE/2,yEndLocation,yEndLocation,COLOR,SIZE/5);
            displayDrawLineThickness(xEndLocation-SIZE/2,xEndLocation,yEndLocation,yEndLocation-SIZE/6,COLOR,SIZE/5);
        break;
        case 'E':
        case 'e':
            displayDrawLineThickness(xStartLocation,xStartLocation,yStartLocation,yEndLocation,COLOR, SIZE/5);
            displayDrawLineThickness(xStartLocation,xEndLocation,yStartLocation,yStartLocation,COLOR,SIZE/5);
            displayDrawLineThickness(xStartLocation,xStartLocation+SIZE/2,yStartLocation+SIZE/2,yStartLocation+SIZE/2,COLOR,SIZE/5);
            displayDrawLineThickness(xStartLocation,xEndLocation,yEndLocation,yEndLocation,COLOR,SIZE/5);
        break;
        case 'F':
        case 'f':
            displayDrawLineThickness(xStartLocation,xStartLocation,yStartLocation,yEndLocation,COLOR, SIZE/5);
            displayDrawLineThickness(xStartLocation,xEndLocation,yStartLocation,yStartLocation,COLOR,SIZE/5);
            displayDrawLineThickness(xStartLocation,xStartLocation+SIZE/2,yStartLocation+SIZE/2,yStartLocation+SIZE/2,COLOR,SIZE/5);
        break;
        case 'G':
        case 'g':
            displayDrawLineThickness(xStartLocation,xEndLocation-SIZE/2,yStartLocation+SIZE/6,yStartLocation,COLOR,SIZE/5);
            displayDrawLineThickness(xEndLocation-SIZE/2,xEndLocation,yStartLocation,yStartLocation+SIZE/6,COLOR,SIZE/5);
            displayDrawLineThickness(xStartLocation,xStartLocation,yStartLocation+SIZE/6,yEndLocation-SIZE/6,COLOR, SIZE/5);
            displayDrawLineThickness(xStartLocation,xEndLocation-SIZE/2,yEndLocation-SIZE/6,yEndLocation,COLOR,SIZE/5);
            displayDrawLineThickness(xEndLocation-SIZE/2,xEndLocation,yEndLocation,yEndLocation-SIZE/6,COLOR,SIZE/5);
            displayDrawLineThickness(xStartLocation+SIZE/2,xEndLocation,yEndLocation-SIZE/2,yEndLocation-SIZE/2,COLOR,SIZE/5);
            displayDrawLineThickness(xEndLocation,xEndLocation,yEndLocation-SIZE/2,yEndLocation,COLOR,SIZE/5);
        break;
        case 'H':
        case 'h':
            displayDrawLineThickness(xStartLocation,xStartLocation,yStartLocation,yEndLocation,COLOR, SIZE/5);
            displayDrawLineThickness(xStartLocation,xEndLocation,yStartLocation+SIZE/2,yStartLocation+SIZE/2,COLOR,SIZE/5);
            displayDrawLineThickness(xEndLocation,xEndLocation,yStartLocation,yEndLocation,COLOR, SIZE/5);
        break;
        case 'I':
        case 'i':
            displayDrawLineThickness(xStartLocation+SIZE/3,xStartLocation+SIZE*2/3,yStartLocation,yStartLocation,COLOR, SIZE/5);
            displayDrawLineThickness(xStartLocation+SIZE/2,xStartLocation+SIZE/2,yStartLocation,yEndLocation,COLOR, SIZE/5);
            displayDrawLineThickness(xStartLocation+SIZE/3,xStartLocation+SIZE*2/3,yEndLocation,yEndLocation,COLOR, SIZE/5);
        break;
        case 'J':
        case 'j':
            displayDrawLineThickness(xEndLocation,xEndLocation,yStartLocation,yEndLocation-SIZE/6,COLOR, SIZE/5);
            displayDrawLineThickness(xStartLocation,xEndLocation-SIZE/2,yEndLocation-SIZE/6,yEndLocation,COLOR,SIZE/5);
            displayDrawLineThickness(xEndLocation-SIZE/2,xEndLocation,yEndLocation,yEndLocation-SIZE/6,COLOR,SIZE/5);
        break;
        case 'K':
        case 'k':
            displayDrawLineThickness(xStartLocation,xStartLocation,yStartLocation,yEndLocation,COLOR, SIZE/5);
            displayDrawLineThickness(xStartLocation+SIZE/4,xStartLocation+SIZE*2/3,yStartLocation+SIZE/2,yStartLocation,COLOR,SIZE/5);
            displayDrawLineThickness(xStartLocation,xStartLocation+SIZE/4,yStartLocation+SIZE/2,yStartLocation+SIZE/2,COLOR,SIZE/5);
            displayDrawLineThickness(xStartLocation+SIZE/4,xEndLocation-SIZE/4,yStartLocation+SIZE/2,yEndLocation,COLOR,SIZE/5);
        break;
        case 'L':
        case 'l':
            displayDrawLineThickness(xStartLocation,xStartLocation,yStartLocation,yEndLocation,COLOR, SIZE/5);
            displayDrawLineThickness(xStartLocation,xEndLocation-SIZE/4,yEndLocation,yEndLocation,COLOR,SIZE/5);
        break;
        case 'M':
        case 'm':
            displayDrawLineThickness(xStartLocation,xStartLocation,yStartLocation,yEndLocation,COLOR, SIZE/5);
            displayDrawLineThickness(xStartLocation,xStartLocation+SIZE/2,yStartLocation,yStartLocation+SIZE/2,COLOR, SIZE/5);
            displayDrawLineThickness(xStartLocation+SIZE/2,xEndLocation,yStartLocation+SIZE/2,yStartLocation,COLOR, SIZE/5);
            displayDrawLineThickness(xEndLocation,xEndLocation,yStartLocation,yEndLocation,COLOR, SIZE/5);
        break;
        case 'N':
        case 'n':
            displayDrawLineThickness(xStartLocation,xStartLocation,yStartLocation,yEndLocation,COLOR, SIZE/5);
            displayDrawLineThickness(xStartLocation,xEndLocation,yStartLocation,yEndLocation,COLOR, SIZE/5);
            displayDrawLineThickness(xEndLocation,xEndLocation,yStartLocation,yEndLocation,COLOR, SIZE/5);
        break;
        case 'O':
        case 'o':
            displayDrawLineThickness(xStartLocation,xEndLocation-SIZE/2,yStartLocation+SIZE/6,yStartLocation,COLOR,SIZE/5);
            displayDrawLineThickness(xEndLocation-SIZE/2,xEndLocation,yStartLocation,yStartLocation+SIZE/6,COLOR,SIZE/5);
            displayDrawLineThickness(xStartLocation,xStartLocation,yStartLocation+SIZE/6,yEndLocation-SIZE/6,COLOR, SIZE/5);
            displayDrawLineThickness(xStartLocation,xEndLocation-SIZE/2,yEndLocation-SIZE/6,yEndLocation,COLOR,SIZE/5);
            displayDrawLineThickness(xEndLocation-SIZE/2,xEndLocation,yEndLocation,yEndLocation-SIZE/6,COLOR,SIZE/5);
            displayDrawLineThickness(xEndLocation,xEndLocation,yStartLocation+SIZE/6,yEndLocation-SIZE/6,COLOR, SIZE/5);
        break;
        case 'P':
        case 'p':
            displayDrawLineThickness(xStartLocation,xStartLocation,yStartLocation,yEndLocation,COLOR, SIZE/5);
            displayDrawLineThickness(xStartLocation,xEndLocation-SIZE/6,yStartLocation,yStartLocation,COLOR,SIZE/5);
            displayDrawLineThickness(xEndLocation-SIZE/6,xEndLocation-SIZE/6,yStartLocation,yStartLocation+SIZE/2,COLOR,SIZE/5);
            displayDrawLineThickness(xEndLocation-SIZE/6,xStartLocation,yStartLocation+SIZE/2,yStartLocation+SIZE/2,COLOR,SIZE/5);
        break;
        case 'Q':
        case 'q':
            displayDrawLineThickness(xStartLocation,xEndLocation-SIZE/2,yStartLocation+SIZE/6,yStartLocation,COLOR,SIZE/5);
            displayDrawLineThickness(xEndLocation-SIZE/2,xEndLocation,yStartLocation,yStartLocation+SIZE/6,COLOR,SIZE/5);
            displayDrawLineThickness(xStartLocation,xStartLocation,yStartLocation+SIZE/6,yEndLocation-SIZE/6,COLOR, SIZE/5);
            displayDrawLineThickness(xStartLocation,xEndLocation-SIZE/2,yEndLocation-SIZE/6,yEndLocation,COLOR,SIZE/5);
            displayDrawLineThickness(xEndLocation-SIZE/2,xEndLocation,yEndLocation,yEndLocation-SIZE/6,COLOR,SIZE/5);
            displayDrawLineThickness(xEndLocation,xEndLocation,yStartLocation+SIZE/6,yEndLocation-SIZE/6,COLOR, SIZE/5);
            displayDrawLineThickness(xStartLocation+SIZE/2,xEndLocation,yStartLocation+SIZE/2,yEndLocation,COLOR, SIZE/5);
        break;
        case 'R':
        case 'r':
            displayDrawLineThickness(xStartLocation,xStartLocation,yStartLocation,yEndLocation,COLOR, SIZE/5);
            displayDrawLineThickness(xStartLocation,xEndLocation-SIZE/3,yStartLocation,yStartLocation,COLOR,SIZE/5);
            displayDrawLineThickness(xEndLocation-SIZE/3,xEndLocation-SIZE/3,yStartLocation,yStartLocation+SIZE*2/6,COLOR,SIZE/5);
            displayDrawLineThickness(xEndLocation-SIZE/3,xStartLocation,yStartLocation+SIZE*2/6,yStartLocation+SIZE/2,COLOR,SIZE/5);
            displayDrawLineThickness(xStartLocation+SIZE/2,xEndLocation,yStartLocation+SIZE/2,yEndLocation,COLOR, SIZE/5);
        break;
        case 'S':
        case 's':
            displayDrawLineThickness(xStartLocation,xStartLocation+SIZE/2,yStartLocation+SIZE/4,yStartLocation,COLOR, SIZE/5);
            displayDrawLineThickness(xStartLocation+SIZE/2,xEndLocation,yStartLocation,yStartLocation+SIZE/4,COLOR, SIZE/5);
            displayDrawLineThickness(xStartLocation,xEndLocation,yStartLocation+SIZE/4,yEndLocation-SIZE/4,COLOR, SIZE/5);
            displayDrawLineThickness(xStartLocation,xStartLocation+SIZE/2,yEndLocation-SIZE/4,yEndLocation,COLOR, SIZE/5);
            displayDrawLineThickness(xStartLocation+SIZE/2,xEndLocation,yEndLocation,yEndLocation-SIZE/4,COLOR, SIZE/5);
        break;
        case 'T':
        case 't':
            displayDrawLineThickness(xStartLocation,xEndLocation,yStartLocation,yStartLocation,COLOR, SIZE/5);
            displayDrawLineThickness(xStartLocation+SIZE/2,xStartLocation+SIZE/2,yStartLocation,yEndLocation,COLOR, SIZE/5);
        break;
        case 'U':
        case 'u':
            displayDrawLineThickness(xStartLocation,xStartLocation,yStartLocation,yEndLocation-SIZE/4,COLOR, SIZE/5);
            displayDrawLineThickness(xStartLocation,xStartLocation+SIZE/2,yEndLocation-SIZE/4,yEndLocation,COLOR, SIZE/5);
            displayDrawLineThickness(xStartLocation+SIZE/2,xEndLocation,yEndLocation,yEndLocation-SIZE/4,COLOR, SIZE/5);
            displayDrawLineThickness(xEndLocation,xEndLocation,yStartLocation,yEndLocation-SIZE/4,COLOR, SIZE/5);
        break;
        case 'V':
        case 'v':
            displayDrawLineThickness(xStartLocation,xEndLocation-SIZE/2,yStartLocation,yEndLocation,COLOR, SIZE/5);
            displayDrawLineThickness(xStartLocation+SIZE/2,xEndLocation,yEndLocation,yStartLocation,COLOR, SIZE/5);
        break;
        case 'W':
        case 'w':
            displayDrawLineThickness(xStartLocation,xStartLocation,yStartLocation,yEndLocation,COLOR, SIZE/5);
            displayDrawLineThickness(xStartLocation,xStartLocation+SIZE/2,yEndLocation,yEndLocation-SIZE/2,COLOR, SIZE/5);
            displayDrawLineThickness(xStartLocation+SIZE/2,xEndLocation,yEndLocation-SIZE/2,yEndLocation,COLOR, SIZE/5);
            displayDrawLineThickness(xEndLocation,xEndLocation,yStartLocation,yEndLocation,COLOR, SIZE/5);
        break;
        case 'X':
        case 'x':
            displayDrawLineThickness(xStartLocation,xEndLocation,yStartLocation,yEndLocation,COLOR, SIZE/5);
            displayDrawLineThickness(xStartLocation,xEndLocation,yEndLocation,yStartLocation,COLOR, SIZE/5);
        break;
        case 'Y':
        case 'y':
            displayDrawLineThickness(xStartLocation,xEndLocation-SIZE/2,yStartLocation,yEndLocation-SIZE/2,COLOR, SIZE/5);
            displayDrawLineThickness(xStartLocation+SIZE/2,xEndLocation,yEndLocation-SIZE/2,yStartLocation,COLOR, SIZE/5);
            displayDrawLineThickness(xStartLocation+SIZE/2,xStartLocation+SIZE/2,yEndLocation-SIZE/2,yEndLocation,COLOR, SIZE/5);
        break;
        case 'Z':
        case 'z':
            displayDrawLineThickness(xStartLocation,xEndLocation,yStartLocation,yStartLocation,COLOR, SIZE/5);
            displayDrawLineThickness(xEndLocation,xStartLocation,yStartLocation,yEndLocation,COLOR, SIZE/5);
            displayDrawLineThickness(xStartLocation,xEndLocation,yEndLocation,yEndLocation,COLOR, SIZE/5);
        break;
        default:
        break;
    }
}
*/


