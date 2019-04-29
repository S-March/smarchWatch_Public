/*
 * miniDB.c
 *
 *  Created on: Jan 22, 2019
 *      Author: samsonm
 */

#include "miniDB.h"

TaskHandle_t DisplayTaskHandle;
TaskHandle_t ANCSTaskHandle;
char titleBuffer[50];
char messageBuffer[250];
bool imageLoaderIsDone;

void setDisplayTaskHandle(TaskHandle_t TASK_HANDLE)
{
        DisplayTaskHandle = TASK_HANDLE;
}
TaskHandle_t getDisplayTaskHandle()
{
        return DisplayTaskHandle;
}
void setANCSTaskHandle(TaskHandle_t TASK_HANDLE)
{
        ANCSTaskHandle = TASK_HANDLE;
}
TaskHandle_t getANCSTaskHandle()
{
        return ANCSTaskHandle;
}
void setANCSTitle(char TITLE[])
{
        strncpy(titleBuffer,TITLE,50);
}
int getANCSTitle(void)
{
        return titleBuffer;
}
void setANCSMessage(char MESSAGE[])
{
        strncpy(messageBuffer,MESSAGE,255);
}
int getANCSMessage(void)
{
        return messageBuffer;
}
void setImageLoaderComplete(bool IS_SET)
{
        imageLoaderIsDone = IS_SET;
}
bool getImageLoaderComplete(void)
{
        return imageLoaderIsDone;
}
