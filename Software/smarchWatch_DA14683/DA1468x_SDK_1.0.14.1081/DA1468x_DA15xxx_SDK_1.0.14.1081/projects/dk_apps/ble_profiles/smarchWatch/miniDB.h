/*
 * miniDB.h
 *
 *  Created on: Jan 22, 2019
 *      Author: samsonm
 */

#ifndef MINIDB_H_
#define MINIDB_H_

#include "osal.h"
#include "resmgmt.h"

void            setDisplayTaskHandle(TaskHandle_t TASK_HANDLE);
TaskHandle_t    getDisplayTaskHandle();
void            setANCSTaskHandle(TaskHandle_t TASK_HANDLE);
TaskHandle_t    getANCSTaskHandle();

void            setANCSTitle(char TITLE[]);
int             getANCSTitle(void);

void            setANCSMessage(char MESSAGE[]);
int             getANCSMessage(void);


void            setImageLoaderComplete(bool IS_SET);
bool            getImageLoaderComplete(void);


#endif /* MINIDB_H_ */
