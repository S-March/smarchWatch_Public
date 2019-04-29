/*
 * displayDriver.c
 *
 *  Created on: Jan 15, 2019
 *      Author: samsonm
 */

#include <stdint.h>
#include "math.h"
#include "displayDriver.h"
#include "platform_devices.h"
#include "ad_spi.h"
#include "ad_nvms.h"
#include "miniDB.h"

int absoluteValue(int NUMBER)
{
    if(NUMBER<0)
    {
        return -NUMBER;
    }
    else
    {
        return NUMBER;
    }
}

void displayWriteCommand(int COMMAND)
{
        spi_device displaySpi = ad_spi_open(DISPLAY_SPI);
        hw_spi_set_9th_bit(ad_spi_get_hw_spi_id(displaySpi),0);
        ad_spi_write(displaySpi,(uint8_t *)&COMMAND,1);
        ad_spi_close(displaySpi);
}
void displayWriteData(int DATA)
{
        spi_device displaySpi = ad_spi_open(DISPLAY_SPI);
        hw_spi_set_9th_bit(ad_spi_get_hw_spi_id(displaySpi),1);
        ad_spi_write(displaySpi,(uint8_t *)&DATA,1);
        ad_spi_close(displaySpi);
}
void displayWriteDataBuf(uint8_t DATA[], int DATA_SIZE)
{
        spi_device displaySpi = ad_spi_open(DISPLAY_SPI);
        hw_spi_set_9th_bit(ad_spi_get_hw_spi_id(displaySpi),1);
        ad_spi_write(displaySpi,DATA,DATA_SIZE);
        ad_spi_close(displaySpi);
}
void displayInit(void)
{
        //Software Reset
        hw_gpio_set_inactive(HW_GPIO_PORT_4, HW_GPIO_PIN_7);
        OS_DELAY_MS(10);
        hw_gpio_set_active(HW_GPIO_PORT_4,HW_GPIO_PIN_7);
        OS_DELAY_MS(10);
        //Out of sleep mode
        displayWriteCommand(ST7789_SLPOUT);
        OS_DELAY_MS(10);
        //Set the color mode to 16-bit
        displayWriteCommand(ST7789_COLMOD);
        displayWriteData(0x55);
        OS_DELAY_MS(10);
        //Set the memory access control so that
        //display data comes as row address then
        //column address
        //Also sets refresh from bottom to top
        displayWriteCommand(ST7789_MADCTL);
        displayWriteData(0x00);
        //Column address start and end
        displaySetColumn(ST7789_XSTART,ST7789_WIDTH);
        //Row address start and end
        displaySetRow(ST7789_YSTART,ST7789_HEIGHT);
        //Enable extended command table
        displayWriteCommand(ST7789_CMD2EN);
        displayWriteData(0x5A);
        displayWriteData(0x69);
        displayWriteData(0x02);
        displayWriteData(0x01);
        //Set frame rate to 111 Hz (max)
        displayWriteCommand(ST7789_FRCTRL2);
        displayWriteData(0x01);
        //Set inversion on
        displayWriteCommand(ST7789_INVON);
        //Set normal display on
        displayWriteCommand(ST7789_NORON);
        OS_DELAY_MS(10);
        //Turn display on
        displayWriteCommand(ST7789_DISPON);
        OS_DELAY_MS(10);

        OS_DELAY_MS(1000); //TODO: CAN THIS BE REDUCED?
        ad_nvms_init();
}
void displaySetRotation(int ORIENTATION)
{
    displayWriteCommand(ST7789_MADCTL);
    switch(ORIENTATION)
    {
        case 0:
            displayWriteData(ST7789_MADCTL_MX | ST7789_MADCTL_MY | ST7789_MADCTL_RGB);
            break;
        case 1:
            displayWriteData(ST7789_MADCTL_MY | ST7789_MADCTL_MV | ST7789_MADCTL_RGB);
            break;
        case 2:
            displayWriteData(ST7789_MADCTL_RGB);
            break;
        case 3:
            displayWriteData(ST7789_MADCTL_MX | ST7789_MADCTL_MY | ST7789_MADCTL_RGB);
            break;
        default:
            displayWriteData(ST7789_MADCTL_MX | ST7789_MADCTL_MV | ST7789_MADCTL_RGB);
            break;
    }
}
void displaySetWindow(int XSTART, int XEND, int YSTART, int YEND)
{
    //Set column
    displaySetColumn(XSTART,XEND);
    //Set row
    displaySetRow(YSTART,YEND);
    //Begin writing frame to RAM
    displayWriteCommand(ST7789_RAMWR);
}
void displaySetWindow2(int XSTART, int XEND, int YSTART, int YEND)
{
    //Set column
    displaySetColumn(XSTART,XEND);
    //Set row
    displaySetRow(YSTART,YEND);
    //Begin writing frame to RAM
    displayWriteCommand(ST7789_RAMWR);
}
void displaySetColumn(int XSTART, int XEND)
{
    uint8_t xStartHigh = XSTART >> 8;
    uint8_t xStartLow = XSTART & 0xFF;
    uint8_t xEndHigh = XEND >> 8;
    uint8_t xEndLow = XEND & 0xFF;

    //Set window X dimensions
    displayWriteCommand(ST7789_CASET);
    displayWriteData(xStartHigh);
    displayWriteData(xStartLow);
    displayWriteData(xEndHigh);
    displayWriteData(xEndLow);
}
void displaySetRow(int YSTART, int YEND)
{
    //Add y-offset for display
    YSTART += ST7789_HEIGHT_OFFSET;
    YEND += ST7789_HEIGHT_OFFSET;

    uint8_t yStartHigh = (YSTART) >> 8;
    uint8_t yStartLow = (YSTART) & 0xFF;
    uint8_t yEndHigh = (YEND) >> 8;
    uint8_t yEndLow = (YEND) & 0xFF;

    //Set window Y dimensions
    displayWriteCommand(ST7789_RASET);
    displayWriteData(yStartHigh);
    displayWriteData(yStartLow);
    displayWriteData(yEndHigh);
    displayWriteData(yEndLow);
}
int display24to16Color(int COLOR)
{
    uint8_t redComponent = ((COLOR & 0x00FF0000)>>19);
    uint8_t greenComponent = ((COLOR & 0x0000FF00)>>10);
    uint8_t blueComponent = ((COLOR & 0x000000FF)>>3);
    uint16_t newColor = ((redComponent<<11)|(greenComponent<<5))|(blueComponent);
    return newColor;
}
void displayClear(void)
{
    displaySetWindow(ST7789_XSTART,ST7789_WIDTH,ST7789_YSTART,ST7789_HEIGHT);
    for(int i = 0; i<(ST7789_WIDTH-ST7789_XSTART)*(ST7789_HEIGHT-ST7789_YSTART)*2;i++)
    {
        displayWriteData(0x00);
    }
}
void displayClearBuf(void)
{
    uint8_t writeBuffer[SPI_WRITE_BUFFER_SIZE] = {0};
    displaySetWindow(ST7789_XSTART,ST7789_WIDTH,ST7789_YSTART,ST7789_HEIGHT);
    for(int i = 0; i<(((ST7789_WIDTH-ST7789_XSTART)*(ST7789_HEIGHT-ST7789_YSTART)*2)/SPI_WRITE_BUFFER_SIZE);i++)
    {
            for(int j=0;j<SPI_WRITE_BUFFER_SIZE;j++)
            {
                    writeBuffer[j]=0x00;
            }
            displayWriteDataBuf(writeBuffer,SPI_WRITE_BUFFER_SIZE);
    }
}
void displayFillScreen(int COLOR)
{
    uint8_t colorHigh = COLOR >> 8;
    uint8_t colorLow = COLOR & 0xFF;
    displaySetWindow(ST7789_XSTART,ST7789_WIDTH,ST7789_YSTART,ST7789_HEIGHT);
    for(int i = 0; i<(ST7789_WIDTH-ST7789_XSTART)*(ST7789_HEIGHT-ST7789_YSTART);i++)
    {
        displayWriteData(colorHigh);
        displayWriteData(colorLow);
    }
}
void displayFillScreenBuf(int COLOR)
{
    uint8_t colorHigh = COLOR >> 8;
    uint8_t colorLow = COLOR & 0xFF;
    uint8_t writeBuffer[SPI_WRITE_BUFFER_SIZE] = {0};
    displaySetWindow(ST7789_XSTART,ST7789_WIDTH,ST7789_YSTART,ST7789_HEIGHT);
    for(int i = 0; i<(((ST7789_WIDTH-ST7789_XSTART)*(ST7789_HEIGHT-ST7789_YSTART)*2)/SPI_WRITE_BUFFER_SIZE);i++)
    {
            for(int j=0;j<SPI_WRITE_BUFFER_SIZE-1;j+=2)
            {
                    writeBuffer[j]=colorHigh;
                    writeBuffer[j+1]=colorLow;
            }
            displayWriteDataBuf(writeBuffer,SPI_WRITE_BUFFER_SIZE);
    }
}
void displayDrawPixel(int X_LOCATION, int Y_LOCATION, int COLOR)
{
    uint8_t colorHigh = COLOR >> 8;
    uint8_t colorLow = COLOR & 0xFF;
    displaySetWindow(X_LOCATION,ST7789_WIDTH-1,Y_LOCATION,ST7789_HEIGHT-1);
    displayWriteData(colorHigh);
    displayWriteData(colorLow);
}
void displayDrawPixelThickness(int X_LOCATION, int Y_LOCATION, int COLOR, int THICKNESS)
{
    int pixelXStart = X_LOCATION - THICKNESS/2;
    int pixelXEnd = X_LOCATION + THICKNESS/2;
    int pixelYStart = Y_LOCATION - THICKNESS/2;
    int pixelYEnd = Y_LOCATION + THICKNESS/2;
    displayDrawRectangle(pixelXStart, pixelXEnd, pixelYStart, pixelYEnd, COLOR);
}
void displayDrawLine(int START_X, int END_X, int START_Y, int END_Y, int COLOR)
{
    int deltaX = absoluteValue(END_X-START_X);
    int xIncrement = START_X<END_X ? 1 : -1;
    int deltaY = absoluteValue(END_Y-START_Y);
    int yIncrement = START_Y<END_Y ? 1 : -1;
    int lineError = (deltaX>deltaY ? deltaX : -deltaY)/2;
    int oldLineError;

        for(;;)
        {
            displayDrawPixel(START_X,START_Y,COLOR);
            if (START_X==END_X && START_Y==END_Y) break;
            oldLineError = lineError;
            if (oldLineError >-deltaX)
            {
                lineError -= deltaY;
                START_X += xIncrement;
            }
            if (oldLineError < deltaY)
            {
                lineError += deltaX;
                START_Y += yIncrement;
            }
        }
}
void displayDrawLinePolar(int START_X, int START_Y, int RADIUS, int ANGLE, int COLOR)
{
    float newAngle = 3.1416*ANGLE/180;
    int endX = START_X+RADIUS*cos(newAngle);
    int endY = START_Y+RADIUS*sin(newAngle);
    displayDrawLine(START_X,endX,START_Y,endY,COLOR);
}
void displayDrawLineThickness(int START_X, int END_X, int START_Y, int END_Y, int COLOR, int THICKNESS)
{
    for(int j = THICKNESS/2; j>=0; j--)
    {
        for(int i = THICKNESS/2; i>0; i--)
        {
            displayDrawLine(START_X-i, END_X-i, START_Y-j, END_Y-j, COLOR);
        }
        for(int i = 0; i<THICKNESS/2; i++)
        {
            displayDrawLine(START_X+i, END_X+i, START_Y-j, END_Y-j, COLOR);
        }
    }
    for(int j = 0; j<THICKNESS/2; j++)
    {
        for(int i = THICKNESS/2; i>0; i--)
        {
            displayDrawLine(START_X-i, END_X-i, START_Y+j, END_Y+j, COLOR);
        }
        for(int i = 0; i<THICKNESS/2; i++)
        {
            displayDrawLine(START_X+i, END_X+i, START_Y+j, END_Y+j, COLOR);
        }
    }
}
void displayDrawLineThickness2(int START_X, int END_X, int START_Y, int END_Y, int COLOR, int THICKNESS)
{
    float yLength = END_Y - START_Y;
    float currentYPosition = START_Y;
    if(END_X>START_X)
    {
        float xLength = END_X - START_X;
        float lineSlope = yLength/xLength;
        for(int i = START_X; i<=END_X; i+=THICKNESS)
        {
            displayDrawPixelThickness(i,currentYPosition,COLOR,THICKNESS);
            currentYPosition += lineSlope;
        }
    }
    else
    {
        float xLength = START_X - END_X;
        float lineSlope = yLength/xLength;
        for(int i = START_X; i>=END_X; i-=THICKNESS)
        {
            displayDrawPixelThickness(i,currentYPosition,COLOR,THICKNESS);
            currentYPosition += lineSlope;
        }
    }
}
void displayDrawLinePolarThickness(int START_X, int START_Y, int RADIUS, int ANGLE, int COLOR, int THICKNESS)
{
    for(int j = THICKNESS/2; j>=0; j--)
    {
        for(int i = THICKNESS/2; i>=0; i--)
        {
            displayDrawLinePolar(START_X-i,START_Y-j,RADIUS,ANGLE,COLOR);
        }
        for(int i = 0; i<THICKNESS/2; i++)
        {
            displayDrawLinePolar(START_X+i,START_Y-j,RADIUS,ANGLE,COLOR);
        }
    }
    for(int j = 0; j<THICKNESS/2; j++)
    {
        for(int i = THICKNESS/2; i>=0; i--)
        {
            displayDrawLinePolar(START_X-i,START_Y+j,RADIUS,ANGLE,COLOR);
        }
        for(int i = 0; i<THICKNESS/2; i++)
        {
            displayDrawLinePolar(START_X+i,START_Y+j,RADIUS,ANGLE,COLOR);
        }
    }
}
void displayDrawRectangle(int XSTART, int XEND, int YSTART, int YEND, int COLOR)
{
    uint8_t colorHigh = COLOR >> 8;
    uint8_t colorLow = COLOR & 0xFF;
    displaySetWindow(XSTART,XEND,YSTART,YEND);
    for(int i = 0; i<YEND*XEND*2;i++)
    {
        displayWriteData(colorHigh);
        displayWriteData(colorLow);
    }
}
void displayDrawRectangleBuf(int XSTART, int XEND, int YSTART, int YEND, int COLOR)
{
    uint8_t colorHigh = COLOR >> 8;
    uint8_t colorLow = COLOR & 0xFF;
    uint8_t rectangleWidth = (XEND>XSTART?(XEND-XSTART):(XSTART-XEND));
    uint8_t rectangleHeight = (YEND>YSTART?(YEND-YSTART):(YSTART-YEND));
    uint16_t leftOverData = (rectangleWidth*rectangleHeight*2)%SPI_WRITE_BUFFER_SIZE;
    uint8_t writeBuffer[SPI_WRITE_BUFFER_SIZE] = {0};

    displaySetWindow(XSTART,XEND,YSTART,YEND);
    for(int i = 0; i<((rectangleWidth*rectangleHeight*2)/SPI_WRITE_BUFFER_SIZE);i++)
    {
            for(int j=0;j<SPI_WRITE_BUFFER_SIZE-1;j+=2)
            {
                    writeBuffer[j]=colorHigh;
                    writeBuffer[j+1]=colorLow;
            }
            displayWriteDataBuf(writeBuffer,SPI_WRITE_BUFFER_SIZE);
    }
    if(leftOverData>0)
    {
            for(int k=0;k<leftOverData-1;k+=2)
            {
                    writeBuffer[k]=colorHigh;
                    writeBuffer[k+1]=colorLow;
            }
            displayWriteDataBuf(writeBuffer,leftOverData);
    }
}
void displayArrayBuf(int XSTART, int WIDTH, int YSTART, int HEIGHT, int (*ARRAY)[], int SIZE_OF_ARRAY)
{
//    uint16_t sizeOfArray = sizeof((*ARRAY))/sizeof((*ARRAY)[0]);
    uint16_t sizeOfArray = SIZE_OF_ARRAY;
    uint16_t leftOverData = (WIDTH*HEIGHT)%SPI_WRITE_BUFFER_SIZE;
    uint8_t writeBuffer[SPI_WRITE_BUFFER_SIZE] = {0};
    displaySetWindow(XSTART,(XSTART+WIDTH),YSTART,(YSTART+HEIGHT));

    for(int i = 0; i<((WIDTH*HEIGHT*2)/SPI_WRITE_BUFFER_SIZE);i++)
    {
            for(int j=0;j<SPI_WRITE_BUFFER_SIZE-1;j++)
            {
                    writeBuffer[(2*j)]=(*ARRAY)[((i*SPI_WRITE_BUFFER_SIZE)+j)] >> 8;
                    writeBuffer[(2*j)+1]=(*ARRAY)[((i*SPI_WRITE_BUFFER_SIZE)+j)] & 0xFF;
            }
            displayWriteDataBuf(writeBuffer,SPI_WRITE_BUFFER_SIZE);
    }
    if(leftOverData>0)
    {
            for(int k=0;k<leftOverData-1;k++)
            {
                    writeBuffer[(2*k)]=(*ARRAY)[((sizeOfArray-leftOverData)+k)] >> 8;
                    writeBuffer[(2*k)+1]=(*ARRAY)[((sizeOfArray-leftOverData)+k)] & 0xFF;
            }
            displayWriteDataBuf(writeBuffer,(2*leftOverData));
    }
}
void displayDrawCircle(int CENTER_X, int CENTER_Y, int RADIUS, int COLOR)
{
    int x = RADIUS-1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx-(RADIUS<<1);

    while(x>=y)
    {
        displayDrawPixel(CENTER_X-y, CENTER_Y+x, COLOR);
        displayDrawPixel(CENTER_X+y, CENTER_Y+x, COLOR);
        displayDrawPixel(CENTER_X-x, CENTER_Y+y, COLOR);
        displayDrawPixel(CENTER_X+x, CENTER_Y+y, COLOR);
        displayDrawPixel(CENTER_X-x, CENTER_Y-y, COLOR);
        displayDrawPixel(CENTER_X+x, CENTER_Y-y, COLOR);
        displayDrawPixel(CENTER_X-y, CENTER_Y-x, COLOR);
        displayDrawPixel(CENTER_X+y, CENTER_Y-x, COLOR);

        if(err <=0)
        {
            y++;
            err += dy;
            dy += 2;
        }
        else
        {
            x--;
            dx += 2;
            err += dx -(RADIUS<<1);
        }
    }
}
void displayTestPattern(void)
{
    displaySetWindow(ST7789_XSTART,ST7789_WIDTH,ST7789_YSTART,ST7789_HEIGHT);
    for(int i = 0; i<(ST7789_WIDTH-ST7789_XSTART)*(ST7789_HEIGHT-ST7789_YSTART)*2;i++)
    {
        uint8_t colorHigh = i;
        uint8_t colorLow = i;
        displayWriteData(colorHigh);
        displayWriteData(colorLow);
    }
}
void displayTestPattern2(void)
{
    displaySetWindow(ST7789_XSTART,ST7789_WIDTH,ST7789_YSTART,ST7789_HEIGHT);
    for(int i = 0; i<(ST7789_WIDTH-ST7789_XSTART)*(ST7789_HEIGHT-ST7789_YSTART)*2;i++)
    {
        if(i%2)
        {
            displayWriteData(0xF);
            displayWriteData(0xF);
        }
        else
        {
            displayWriteData(0x0);
            displayWriteData(0x0);
        }
    }
}

void displayImageFromMemory(int XSTART, int YSTART, int ADDRESS_IN_MEMORY)
{
        OS_TICK_TIME xNextWakeTime;
        xNextWakeTime = OS_GET_TICK_COUNT();
        uint8_t sizeOfImageBuffer[2]={0};
        nvms_t flashMemory = ad_nvms_open(NVMS_FLASH_STORAGE);
        ad_nvms_read(flashMemory, ADDRESS_IN_MEMORY, (uint8 *) sizeOfImageBuffer, sizeof(sizeOfImageBuffer));
        int imageAdressDataOffset = ADDRESS_IN_MEMORY+2;
        int widthOfImage = sizeOfImageBuffer[0];
        int heightOfImage = sizeOfImageBuffer[1];
        int sizeOfImageInBytes = widthOfImage*heightOfImage*2;
        int leftOverData = sizeOfImageInBytes%SPI_WRITE_BUFFER_SIZE;
        int currentPositionInData = 0;

        uint8_t writeBuffer[SPI_WRITE_BUFFER_SIZE] = {0};
        displaySetWindow(XSTART,(XSTART+widthOfImage),YSTART,YSTART+heightOfImage);
        for(int i = 0; i<(sizeOfImageInBytes/SPI_WRITE_BUFFER_SIZE);i++)
        {
                ad_nvms_read(flashMemory, ((i*SPI_WRITE_BUFFER_SIZE)+imageAdressDataOffset), (uint8 *) writeBuffer, sizeof(writeBuffer));
                displayWriteDataBuf(writeBuffer,SPI_WRITE_BUFFER_SIZE);
                currentPositionInData = i*SPI_WRITE_BUFFER_SIZE;
        }
        if(leftOverData>0)
        {
                ad_nvms_read(flashMemory, (currentPositionInData+SPI_WRITE_BUFFER_SIZE+imageAdressDataOffset), (uint8 *) writeBuffer, sizeof(writeBuffer));
                displayWriteDataBuf(writeBuffer,leftOverData);
        }
}

void displayPartialImageFromMemory(int SCREEN_XSTART, int SCREEN_YSTART, int IMAGE_XSTART, int IMAGE_YSTART, int IMAGE_PARTIAL_WIDTH, int IMAGE_PARTIAL_HEIGHT, int ADDRESS_IN_MEMORY)
{
        OS_TICK_TIME xNextWakeTime;
        xNextWakeTime = OS_GET_TICK_COUNT();
        nvms_t flashMemory = ad_nvms_open(NVMS_FLASH_STORAGE);
        uint8_t sizeOfImageBuffer[2]={0};
        ad_nvms_read(flashMemory, ADDRESS_IN_MEMORY, (uint8 *) sizeOfImageBuffer, sizeof(sizeOfImageBuffer));
        int widthOfImage = sizeOfImageBuffer[0];
        if(widthOfImage%2)
        {
                widthOfImage++;
        }
        int partialImageAdressDataOffset = (widthOfImage*IMAGE_YSTART*BYTES_PER_PIXEL)+(IMAGE_XSTART*BYTES_PER_PIXEL)+2+ADDRESS_IN_MEMORY;
        int bufferCounter = 0;
        int memoryReadSpot = 0;
        uint8_t writeBuffer[SPI_WRITE_BUFFER_SIZE] = {0};
        uint8_t partialImageWidthBuffer[(ST7789_WIDTH*BYTES_PER_PIXEL)] = {0};
        displaySetWindow(SCREEN_XSTART,(SCREEN_XSTART+IMAGE_PARTIAL_WIDTH-1),SCREEN_YSTART,(SCREEN_YSTART+IMAGE_PARTIAL_HEIGHT));

        for(int currentRow = 0;currentRow<(IMAGE_PARTIAL_HEIGHT);currentRow++)
        {
                memoryReadSpot = (currentRow*(widthOfImage)*BYTES_PER_PIXEL)+partialImageAdressDataOffset;
                ad_nvms_read(flashMemory,memoryReadSpot, (uint8 *)partialImageWidthBuffer, sizeof(partialImageWidthBuffer));
                if(((bufferCounter*(IMAGE_PARTIAL_WIDTH*BYTES_PER_PIXEL))+(IMAGE_PARTIAL_WIDTH*BYTES_PER_PIXEL))>SPI_WRITE_BUFFER_SIZE)
                {
                        memcpy(writeBuffer, partialImageWidthBuffer, IMAGE_PARTIAL_WIDTH*BYTES_PER_PIXEL);
                        displayWriteDataBuf(writeBuffer,SPI_WRITE_BUFFER_SIZE);
                        bufferCounter = 0;
                }
                else
                {
                        memcpy(&writeBuffer[((bufferCounter)*(IMAGE_PARTIAL_WIDTH*BYTES_PER_PIXEL))], partialImageWidthBuffer, IMAGE_PARTIAL_WIDTH*BYTES_PER_PIXEL);
                        bufferCounter++;
                }
        }
        if(bufferCounter>0)
        {
                displayWriteDataBuf(writeBuffer,(bufferCounter*(IMAGE_PARTIAL_WIDTH*BYTES_PER_PIXEL)));
        }
}

/*int getSizeOfImage(char *FILENAME, int NAME_SIZE)
{
    int sizeOfFile = 0;
    char filePointer;
    char fileToOpen[NAME_SIZE];
    int fileNameLetterCounter = 0;
    while(FILENAME[fileNameLetterCounter]!=0)
    {
        fileToOpen[fileNameLetterCounter] = FILENAME[fileNameLetterCounter];
        fileNameLetterCounter++;
    }
    fileToOpen[NAME_SIZE-1] = 0;
    filePointer = sdCard_fopen(fileToOpen, "r");
    sdCard_fseek(filePointer, BITMAP_SIZE_OFFSET);
    for(int byteNumberToRead = 0; byteNumberToRead<4;byteNumberToRead++)
    {
        sizeOfFile |= sdCard_fgetc(filePointer)<<(8*byteNumberToRead);
    }
    sdCard_fclose(filePointer);
    return sizeOfFile;
}
int getDataOffsetBMP(char *FILENAME, int NAME_SIZE)
{
    int dataOffsetLocation = 0;
    char filePointer;
    char fileToOpen[NAME_SIZE];
    int fileNameLetterCounter = 0;
    while(FILENAME[fileNameLetterCounter]!=0)
    {
        fileToOpen[fileNameLetterCounter] = FILENAME[fileNameLetterCounter];
        fileNameLetterCounter++;
    }
    fileToOpen[NAME_SIZE-1] = 0;
    filePointer = sdCard_fopen(fileToOpen, "r");
    sdCard_fseek(filePointer, BITMAP_DATA_OFFSET);
    for(int byteNumberToRead = 0; byteNumberToRead<4;byteNumberToRead++)
    {
        dataOffsetLocation |= sdCard_fgetc(filePointer)<<(8*byteNumberToRead);
    }
    sdCard_fclose(filePointer);
    return dataOffsetLocation;
}
int getHeightOfBMP(char *FILENAME, int NAME_SIZE)
{
    int heightOfImage = 0;
    char filePointer;
    char fileToOpen[NAME_SIZE];
    int fileNameLetterCounter = 0;
    while(FILENAME[fileNameLetterCounter]!=0)
    {
        fileToOpen[fileNameLetterCounter] = FILENAME[fileNameLetterCounter];
        fileNameLetterCounter++;
    }
    fileToOpen[NAME_SIZE-1] = 0;
    filePointer = sdCard_fopen(fileToOpen, "r");
    sdCard_fseek(filePointer, BITMAP_HEIGHT_OFFSET);
    for(int byteNumberToRead = 0; byteNumberToRead<4;byteNumberToRead++)
    {
        heightOfImage |= sdCard_fgetc(filePointer)<<(8*byteNumberToRead);
    }
    sdCard_fclose(filePointer);
    return heightOfImage;
}
int getWidthOfBMP(char *FILENAME, int NAME_SIZE)
{
    int widthOfImage = 0;
    char filePointer;
    char fileToOpen[NAME_SIZE];
    int fileNameLetterCounter = 0;
    while(FILENAME[fileNameLetterCounter]!=0)
    {
        fileToOpen[fileNameLetterCounter] = FILENAME[fileNameLetterCounter];
        fileNameLetterCounter++;
    }
    fileToOpen[NAME_SIZE-1] = 0;
    filePointer = sdCard_fopen(fileToOpen, "r");
    sdCard_fseek(filePointer, BITMAP_WIDTH_OFFSET);
    for(int byteNumberToRead = 0; byteNumberToRead<4;byteNumberToRead++)
    {
        widthOfImage |= sdCard_fgetc(filePointer)<<(8*byteNumberToRead);
    }
    sdCard_fclose(filePointer);
    return widthOfImage;
}
void displayDrawBMP(int START_X, int START_Y, char *FILENAME, int NAME_SIZE)
{
    char filePointer;
    char fileToOpen[NAME_SIZE];
    int fileNameLetterCounter = 0;
    uint offset;
    uint8 colorLow;
    uint8 colorHigh;
    while(FILENAME[fileNameLetterCounter]!=0)
    {
        fileToOpen[fileNameLetterCounter] = FILENAME[fileNameLetterCounter];
        fileNameLetterCounter++;
    }
    fileToOpen[NAME_SIZE-1] = 0;
    int widthOfImage = getWidthOfBMP(fileToOpen,NAME_SIZE);
    int heightOfImage = getHeightOfBMP(fileToOpen,NAME_SIZE);
    int sizeOfImage = getSizeOfImage(fileToOpen,NAME_SIZE);
    int dataOffsetOfImage = getDataOffsetBMP(fileToOpen,NAME_SIZE);

    filePointer = sdCard_fopen(fileToOpen, "r");
    displaySetWindow(START_X,widthOfImage,START_Y,heightOfImage);
    for(int currentRow = 1; currentRow <= heightOfImage; currentRow++)
    {
        sdCard_fseek(filePointer, (sizeOfImage-currentRow*(widthOfImage*2)));
        for(int currentColumn = 0; currentColumn<widthOfImage;currentColumn++)
        {
            colorHigh = sdCard_fbgetc(filePointer);
            colorLow = sdCard_fbgetc(filePointer);
            displayWriteData(colorLow);
            displayWriteData(colorHigh);
        }
    }
    sdCard_fclose(filePointer);
}
void displayDrawBMPClearBackground(int START_X, int START_Y, char *FILENAME, int NAME_SIZE, int BACKGROUND_COLOR)
{
    char filePointer;
    char fileToOpen[NAME_SIZE];
    int fileNameLetterCounter = 0;
    uint offset;
    uint8 colorLow;
    uint8 colorHigh;
    while(FILENAME[fileNameLetterCounter]!=0)
    {
        fileToOpen[fileNameLetterCounter] = FILENAME[fileNameLetterCounter];
        fileNameLetterCounter++;
    }
    fileToOpen[NAME_SIZE-1] = 0;
    int widthOfImage = getWidthOfBMP(fileToOpen,NAME_SIZE);
    int heightOfImage = getHeightOfBMP(fileToOpen,NAME_SIZE);
    int sizeOfImage = getSizeOfImage(fileToOpen,NAME_SIZE);
    int dataOffsetOfImage = getDataOffsetBMP(fileToOpen,NAME_SIZE);

    filePointer = sdCard_fopen(fileToOpen, "r");
    displaySetWindow(START_X,widthOfImage,START_Y,heightOfImage);
    for(int currentRow = 1; currentRow <= heightOfImage; currentRow++)
    {
        sdCard_fseek(filePointer, (sizeOfImage-currentRow*(widthOfImage*2)));
        for(int currentColumn = 0; currentColumn<widthOfImage;currentColumn++)
        {
            colorHigh = sdCard_fbgetc(filePointer);
            colorLow = sdCard_fbgetc(filePointer);
            if(!colorLow)
            {
                colorLow = BACKGROUND_COLOR >> 8;
            }
            if(!colorHigh)
            {
                colorHigh = BACKGROUND_COLOR & 0xFF;
            }
            displayWriteData(colorLow);
            displayWriteData(colorHigh);
        }
    }
    sdCard_fclose(filePointer);
}
void displayDrawBMPPartial(int START_X, int START_Y, int END_X, int END_Y, char *FILENAME, int NAME_SIZE)
{
    char filePointer;
    char fileToOpen[NAME_SIZE];
    int fileNameLetterCounter = 0;
    uint offset;
    uint8 colorLow;
    uint8 colorHigh;
    while(FILENAME[fileNameLetterCounter]!=0)
    {
        fileToOpen[fileNameLetterCounter] = FILENAME[fileNameLetterCounter];
        fileNameLetterCounter++;
    }
    fileToOpen[NAME_SIZE-1] = 0;
    int widthOfImage = getWidthOfBMP(fileToOpen,NAME_SIZE);
    int heightOfImage = getHeightOfBMP(fileToOpen,NAME_SIZE);
    int sizeOfImage = getSizeOfImage(fileToOpen,NAME_SIZE);
    int dataOffsetOfImage = getDataOffsetBMP(fileToOpen,NAME_SIZE);

    filePointer = sdCard_fopen(fileToOpen, "r");
    displaySetWindow(START_X,widthOfImage,START_Y,heightOfImage);
    for(int currentRow = 1; currentRow <= END_Y; currentRow++)
    {
        sdCard_fseek(filePointer, (sizeOfImage-currentRow*(widthOfImage*2)));
        for(int currentColumn = 0; currentColumn<END_X;currentColumn++)
        {
            colorHigh = sdCard_fbgetc(filePointer);
            colorLow = sdCard_fbgetc(filePointer);
            displayWriteData(colorLow);
            displayWriteData(colorHigh);
        }
    }
    sdCard_fclose(filePointer);
}
void displayDrawBMPFast(int START_X, int START_Y, char *FILENAME, int NAME_SIZE, int NUMBER_OF_DIVISIONS)
{
    char filePointer;
    char fileToOpen[NAME_SIZE];
    int fileNameLetterCounter = 0;
    uint offset;
    uint8 colorLow;
    uint8 colorHigh;
    while(FILENAME[fileNameLetterCounter]!=0)
    {
        fileToOpen[fileNameLetterCounter] = FILENAME[fileNameLetterCounter];
        fileNameLetterCounter++;
    }
    fileToOpen[NAME_SIZE-1] = 0;
    int widthOfImage = getWidthOfBMP(fileToOpen,NAME_SIZE);
    int heightOfImage = getHeightOfBMP(fileToOpen,NAME_SIZE);
    int sizeOfImage = getSizeOfImage(fileToOpen,NAME_SIZE);
    int dataOffsetOfImage = getDataOffsetBMP(fileToOpen,NAME_SIZE);

    filePointer = sdCard_fopen(fileToOpen, "r");
    for(int rowStagger = 0; rowStagger<NUMBER_OF_DIVISIONS;rowStagger++)
    {
        for(int currentRow = 1+rowStagger; currentRow <= heightOfImage; currentRow+=NUMBER_OF_DIVISIONS)
        {
            displaySetWindow(START_X,widthOfImage,currentRow+START_Y,currentRow+1+START_Y);
            sdCard_fseek(filePointer, (sizeOfImage-currentRow*(widthOfImage*2)));
            for(int currentColumn = 0; currentColumn<widthOfImage;currentColumn++)
            {
                colorHigh = sdCard_fbgetc(filePointer);
                colorLow = sdCard_fbgetc(filePointer);
                displayWriteData(colorLow);
                displayWriteData(colorHigh);
            }
        }
    }
    sdCard_fclose(filePointer);
}
void displayDrawBMPPartialFast(int START_X, int START_Y, int END_X, int END_Y, char *FILENAME, int NAME_SIZE, int NUMBER_OF_DIVISIONS)
{

    char filePointer;
    char fileToOpen[NAME_SIZE];
    int fileNameLetterCounter = 0;
    uint offset;
    uint8 colorLow;
    uint8 colorHigh;
    while(FILENAME[fileNameLetterCounter]!=0)
    {
        fileToOpen[fileNameLetterCounter] = FILENAME[fileNameLetterCounter];
        fileNameLetterCounter++;
    }
    fileToOpen[NAME_SIZE-1] = 0;
    int widthOfImage = getWidthOfBMP(fileToOpen,NAME_SIZE);
    int heightOfImage = getHeightOfBMP(fileToOpen,NAME_SIZE);
    int sizeOfImage = getSizeOfImage(fileToOpen,NAME_SIZE);
    int dataOffsetOfImage = getDataOffsetBMP(fileToOpen,NAME_SIZE);

    filePointer = sdCard_fopen(fileToOpen, "r");
    for(int rowStagger = 0; rowStagger<NUMBER_OF_DIVISIONS;rowStagger++)
    {
        for(int currentRow = 1+rowStagger; currentRow <= END_Y; currentRow+=NUMBER_OF_DIVISIONS)
        {
            displaySetWindow(START_X,widthOfImage,currentRow+START_Y,currentRow+1+START_Y);
            sdCard_fseek(filePointer, (sizeOfImage-currentRow*(widthOfImage*2)));
            for(int currentColumn = 0; currentColumn<END_X;currentColumn++)
            {
                colorHigh = sdCard_fbgetc(filePointer);
                colorLow = sdCard_fbgetc(filePointer);
                displayWriteData(colorLow);
                displayWriteData(colorHigh);
            }
        }
    }
    sdCard_fclose(filePointer);
}
void displayDrawBMPFastClearBackground(int START_X, int START_Y, char *FILENAME, int NAME_SIZE, int BACKGROUND_COLOR, int NUMBER_OF_DIVISIONS)
{
    char filePointer;
    char fileToOpen[NAME_SIZE];
    int fileNameLetterCounter = 0;
    uint offset;
    uint8 colorLow;
    uint8 colorHigh;
    while(FILENAME[fileNameLetterCounter]!=0)
    {
        fileToOpen[fileNameLetterCounter] = FILENAME[fileNameLetterCounter];
        fileNameLetterCounter++;
    }
    fileToOpen[NAME_SIZE-1] = 0;
    int widthOfImage = getWidthOfBMP(fileToOpen,NAME_SIZE);
    int heightOfImage = getHeightOfBMP(fileToOpen,NAME_SIZE);
    int sizeOfImage = getSizeOfImage(fileToOpen,NAME_SIZE);
    int dataOffsetOfImage = getDataOffsetBMP(fileToOpen,NAME_SIZE);

    filePointer = sdCard_fopen(fileToOpen, "r");
    for(int rowStagger = 0; rowStagger<NUMBER_OF_DIVISIONS;rowStagger++)
    {
        for(int currentRow = 1+rowStagger; currentRow <= heightOfImage; currentRow+=NUMBER_OF_DIVISIONS)
        {
            displaySetWindow(START_X,widthOfImage,currentRow+START_Y,currentRow+1+START_Y);
            sdCard_fseek(filePointer, (sizeOfImage-currentRow*(widthOfImage*2)));
            for(int currentColumn = 0; currentColumn<widthOfImage;currentColumn++)
            {
                colorHigh = sdCard_fbgetc(filePointer);
                colorLow = sdCard_fbgetc(filePointer);
                if(!colorLow)
                {
                    colorLow = BACKGROUND_COLOR >> 8;
                }
                if(!colorHigh)
                {
                    colorHigh = BACKGROUND_COLOR & 0xFF;
                }
                displayWriteData(colorLow);
                displayWriteData(colorHigh);
            }
        }
    }
    sdCard_fclose(filePointer);
}
void displayDrawBMPPixel(int X_POSITION_DISPLAY, int Y_POSITION_DISPLAY, int X_POSITION_FILE, int Y_POSITION_FILE, char *FILENAME, int NAME_SIZE)
{
    char filePointer;
    char fileToOpen[NAME_SIZE];
    int fileNameLetterCounter = 0;
    uint offset;
    uint8 colorLow;
    uint8 colorHigh;
    while(FILENAME[fileNameLetterCounter]!=0)
    {
        fileToOpen[fileNameLetterCounter] = FILENAME[fileNameLetterCounter];
        fileNameLetterCounter++;
    }
    fileToOpen[NAME_SIZE-1] = 0;
    int sizeOfImage = getSizeOfImage(fileToOpen,NAME_SIZE);
    int dataOffsetOfImage = getDataOffsetBMP(fileToOpen,NAME_SIZE);

    filePointer = sdCard_fopen(fileToOpen, "r");
    displaySetWindow(X_POSITION_DISPLAY,ST7789_WIDTH-1,Y_POSITION_DISPLAY,ST7789_HEIGHT-1);
    sdCard_fseek(filePointer, (sizeOfImage-Y_POSITION_DISPLAY*(ST7789_WIDTH*2)));
    for(int wastedPixels = 0; wastedPixels<X_POSITION_FILE;wastedPixels++)
    {
        colorHigh = sdCard_fbgetc(filePointer);
        colorLow = sdCard_fbgetc(filePointer);
    }
    colorHigh = sdCard_fbgetc(filePointer);
    colorLow = sdCard_fbgetc(filePointer);
    displayWriteData(colorLow);
    displayWriteData(colorHigh);
    sdCard_fclose(filePointer);
}
void displayDrawLineOfBMP(int START_X, int END_X, int START_Y, int END_Y, char *FILENAME, int NAME_SIZE)
{
    int deltaX = absoluteValue(END_X-START_X);
    int xIncrement = START_X<END_X ? 1 : -1;
    int deltaY = absoluteValue(END_Y-START_Y);
    int yIncrement = START_Y<END_Y ? 1 : -1;
    int lineError = (deltaX>deltaY ? deltaX : -deltaY)/2;
    int oldLineError;

    for(;;)
    {
        displayDrawBMPPixel(START_X, START_Y, START_X, START_Y, FILENAME, NAME_SIZE);

        if (START_X==END_X && START_Y==END_Y) break;
        oldLineError = lineError;
        if (oldLineError >-deltaX)
        {
            lineError -= deltaY;
            START_X += xIncrement;
        }
        if (oldLineError < deltaY)
        {
            lineError += deltaX;
            START_Y += yIncrement;
        }
    }
}
void displayDrawLinePolarOfBMP(int START_X, int START_Y, int RADIUS, int ANGLE, char *FILENAME, int NAME_SIZE)
{
    float newAngle = 3.1416*ANGLE/180;
    int endX = START_X+RADIUS*cos(newAngle);
    int endY = START_Y+RADIUS*sin(newAngle);
    displayDrawLineOfBMP(START_X, endX, START_Y, endY, FILENAME, NAME_SIZE);
}
void displayDrawLinePolarThicknessOfBMP(int START_X, int START_Y, int RADIUS, int ANGLE, int THICKNESS, char *FILENAME, int NAME_SIZE)
{
    for(int j = THICKNESS/2; j>=0; j--)
    {
        for(int i = THICKNESS/2; i>=0; i--)
        {
            displayDrawLinePolarOfBMP(START_X-i,START_Y-j,RADIUS,ANGLE,FILENAME,NAME_SIZE);
        }
        for(int i = 0; i<THICKNESS/2; i++)
        {
            displayDrawLinePolarOfBMP(START_X+i,START_Y-j,RADIUS,ANGLE,FILENAME,NAME_SIZE);
        }
    }
    for(int j = 0; j<THICKNESS/2; j++)
    {
        for(int i = THICKNESS/2; i>=0; i--)
        {
            displayDrawLinePolarOfBMP(START_X-i,START_Y+j,RADIUS,ANGLE,FILENAME,NAME_SIZE);
        }
        for(int i = 0; i<THICKNESS/2; i++)
        {
            displayDrawLinePolarOfBMP(START_X+i,START_Y+j,RADIUS,ANGLE,FILENAME,NAME_SIZE);
        }
    }
}
*/
