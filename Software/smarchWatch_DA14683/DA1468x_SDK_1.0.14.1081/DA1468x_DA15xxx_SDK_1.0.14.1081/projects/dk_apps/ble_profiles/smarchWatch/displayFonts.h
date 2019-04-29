/*
 * displayFonts.h
 *
 *  Created on: Jan 25, 2019
 *      Author: samsonm
 */

#ifndef DISPLAYFONTS_H_
#define DISPLAYFONTS_H_

#define FONT_CHARACTER_WIDTH 15
#define FONT_CHARACTER_HEIGHT 25
#define FONT_CHARACTERS_COLUMNS 13
#define FONT_CHARACTERS_ROWS 6

int displayDrawCharacter(int X_START, int Y_START, char CHARACTER);
void displayDrawString(int X_START, int Y_START, int KERNING_SIZE, int MARGIN, int POINTER_TO_STRING);
int setCircularMargin(int CURRENT_Y_POSITION);

#endif /* DISPLAYFONTS_H_ */
