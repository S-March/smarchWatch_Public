/*
 * displayDriver.h
 *
 *  Created on: Jan 15, 2019
 *      Author: samsonm
 */

#ifndef DISPLAYDRIVER_H_
#define DISPLAYDRIVER_H_

#define SPI_WRITE_BUFFER_SIZE 2880 //240*240*2/40 = 2880 which is also 6 lines per write

#define SPI_DELAY       0

#define BYTES_PER_PIXEL 2

#define ST7789_WIDTH    240
#define ST7789_HEIGHT   240
#define ST7789_HEIGHT_OFFSET 40

#define ST7789_XSTART 0
#define ST7789_YSTART 0

//Command registers
#define ST7789_NOP          0x00
#define ST7789_SWRESET      0x01
#define ST7789_SLPIN        0x10
#define ST7789_SLPOUT       0x11
#define ST7789_PTLON        0x12
#define ST7789_NORON        0x13
#define ST7789_INVOFF       0x20
#define ST7789_INVON        0x21
#define ST7789_DISPOFF      0x28
#define ST7789_DISPON       0x29
#define ST7789_CASET        0x2A
#define ST7789_RASET        0x2B
#define ST7789_RAMWR        0x2C
#define ST7789_PTLAR        0x30
#define ST7789_TEON         0x35
#define ST7789_MADCTL       0x36
#define ST7789_IDMOFF       0x38
#define ST7789_IDMON        0x39
#define ST7789_COLMOD       0x3A
#define ST7789_RAMWRC       0x3C
#define ST7789_PORCTRL      0xB2
#define ST7789_GCTRL        0xB7
#define ST7789_VCOMS        0xBB
#define ST7789_LCMCTRL      0xC0
#define ST7789_VDVVRHEN     0xC2
#define ST7789_VRHS         0xC3
#define ST7789_VDVS         0xC4
#define ST7789_FRCTRL2      0xC6
#define ST7789_PWCTRL1      0xD0
#define ST7789_CMD2EN       0xDF
#define ST7789_PVGAMCTRL    0xE0
#define ST7789_NVGAMCTRL    0xE1

#define ST7789_WRDISBV 0X51
#define ST7789_WRCTRLD 0X53

//Display orientations
#define ST7789_MADCTL_MY  0x80
#define ST7789_MADCTL_MX  0x40
#define ST7789_MADCTL_MV  0x20
#define ST7789_MADCTL_ML  0x10
#define ST7789_MADCTL_RGB 0x00

//Color definitions
#define DISPLAY_BLACK   0x0000
#define DISPLAY_BLUE    0x001F
#define DISPLAY_RED     0xF800
#define DISPLAY_GREEN   0x07E0
#define DISPLAY_CYAN    0x07FF
#define DISPLAY_MAGENTA 0xF81F
#define DISPLAY_YELLOW  0xFFE0
#define DISPLAY_WHITE   0xFFFF

//Bitmap offsets
#define BITMAP_SIZE_OFFSET 0x0002
#define BITMAP_DATA_OFFSET 0x000A
#define BITMAP_WIDTH_OFFSET 0x0012
#define BITMAP_HEIGHT_OFFSET 0x0016

void displayInit(void);
void displayWriteCommand(int COMMAND);
void displayWriteData(int DATA);
void displayWriteDataBuf(uint8_t DATA[], int DATA_SIZE);
void displaySetRotation(int ORIENTATION);
void displaySetColumn(int XSTART, int XEND);
void displaySetRow(int YSTART, int YEND);
void displaySetWindow(int XSTART, int XEND, int YSTART, int YEND);
void displaySetWindow2(int XSTART, int XEND, int YSTART, int YEND);
int  display24to16Color(int COLOR);
void displayClear(void);
void displayClearBuf(void);
void displayFillScreen(int COLOR);
void displayFillScreenBuf(int COLOR);
void displayDrawPixel(int X_LOCATION, int Y_LOCATION, int COLOR);
void displayDrawPixelThickness(int X_LOCATION, int Y_LOCATION, int COLOR, int THICKNESS);
void displayDrawLine(int START_X, int END_X, int START_Y, int END_Y, int COLOR);
void displayDrawLinePolar(int START_X, int START_Y, int RADIUS, int ANGLE, int COLOR);
void displayDrawLineThickness(int START_X, int END_X, int START_Y, int END_Y, int COLOR, int THICKNESS);
void displayDrawLineThickness2(int START_X, int END_X, int START_Y, int END_Y, int COLOR, int THICKNESS);
void displayDrawLinePolarThickness(int START_X, int START_Y, int RADIUS, int ANGLE, int COLOR, int THICKNESS);
void displayDrawRectangle(int XSTART, int XEND, int YSTART, int YEND, int COLOR);
void displayDrawRectangleBuf(int XSTART, int XEND, int YSTART, int YEND, int COLOR);
void displayDrawCircle(int CENTER_X, int CENTER_Y, int RADIUS, int COLOR);
void displayTestPattern(void);
void displayTestPattern2(void);
void displayArrayBuf(int XSTART, int WIDTH, int YSTART, int HEIGHT, int (*ARRAY)[], int SIZE_OF_ARRAY);
void displayImageFromMemory(int XSTART, int YSTART, int ADDRESS_IN_MEMORY);
void displayPartialImageFromMemory(int SCREEN_XSTART, int SCREEN_YSTART, int IMAGE_XSTART, int IMAGE_YSTART, int IMAGE_PARTIAL_WIDTH, int IMAGE_PARTIAL_HEIGHT, int ADDRESS_IN_MEMORY);

/*int getSizeOfImage(char *FILENAME, int NAME_SIZE);
int getDataOffsetBMP(char *FILENAME, int NAME_SIZE);
int getHeightOfBMP(char *FILENAME, int NAME_SIZE);
int getWidthOfBMP(char *FILENAME, int NAME_SIZE);

void displayDrawBMP(int START_X, int START_Y, char *FILENAME, int NAME_SIZE);
void displayDrawBMPClearBackground(int START_X, int START_Y, char *FILENAME, int NAME_SIZE, int BACKGROUND_COLOR);
void displayDrawBMPPartial(int START_X, int START_Y, int END_X, int END_Y, char *FILENAME, int NAME_SIZE);
void displayDrawBMPFast(int START_X, int START_Y, char *FILENAME, int NAME_SIZE, int NUMBER_OF_DIVISIONS);
void displayDrawBMPPartialFast(int START_X, int START_Y, int END_X, int END_Y, char *FILENAME, int NAME_SIZE, int NUMBER_OF_DIVISIONS);
void displayDrawBMPFastClearBackground(int START_X, int START_Y, char *FILENAME, int NAME_SIZE, int BACKGROUND_COLOR, int NUMBER_OF_DIVISIONS);
void displayDrawBMPPixel(int X_POSITION_DISPLAY, int Y_POSITION_DISPLAY, int X_POSITION_FILE, int Y_POSITION_FILE, char *FILENAME, int NAME_SIZE);
void displayDrawLineOfBMP(int START_X, int END_X, int START_Y, int END_Y, char *FILENAME, int NAME_SIZE);
void displayDrawLinePolarOfBMP(int START_X, int START_Y, int RADIUS, int ANGLE, char *FILENAME, int NAME_SIZE);
void displayDrawLinePolarThicknessOfBMP(int START_X, int START_Y, int RADIUS, int ANGLE, int THICKNESS, char *FILENAME, int NAME_SIZE);
*/

#endif /* DISPLAYDRIVER_H_ */
