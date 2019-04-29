/*
 * watchAnimations.h
 *
 *  Created on: Jan 22, 2019
 *      Author: samsonm
 */

#ifndef WATCHANIMATIONS_H_
#define WATCHANIMATIONS_H_

#define WATCH_CENTER 120
#define TICK_LENGTH 20

void displayDrawWatchHand(int RADIUS, int ANGLE, int HAND_COLOR);
void displayDrawSecondWatchHand(int RADIUS, int ANGLE, int HAND_COLOR, int X_CENTER, int Y_CENTER);
void displayDrawWatchFace(int BACKGROUND_COLOR, int TICK_COLOR);
void displayDrawWatchNumbers(int BACKGROUND_COLOR, int NUMBER_COLOR);

void displayDrawCharacterFromArray(int X_START, int Y_START, int COLOR, char CHARACTER);

#endif /* WATCHANIMATIONS_H_ */
