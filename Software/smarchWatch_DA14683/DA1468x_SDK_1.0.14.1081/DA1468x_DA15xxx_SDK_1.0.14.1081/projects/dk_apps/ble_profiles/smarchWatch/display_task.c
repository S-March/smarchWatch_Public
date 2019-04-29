/*
 * display_task.c
 *
 *  Created on: Jan 15, 2019
 *      Author: samsonm
 */

#include "osal.h"
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "util/list.h"
#include "util/queue.h"
#include "svc_types.h"
#include "hw_wkup.h"
#include "sys_watchdog.h"
#include "displayDriver.h"
#include "displayFonts.h"
#include "watchAnimations.h"
#include "ad_spi.h"
#include "miniDB.h"
#include "imageOffsets.h"

#define UPDATE_DISPLAY_MASK (1<<0)

void display_task(void *params)
{
        setDisplayTaskHandle(OS_GET_CURRENT_TASK());
        //NOTE: IN hw_spi.c
        //REG_SET_FIELD(CRG_PER, CLK_PER_REG, SPI_CLK_SEL, clk_per_reg_local, 0); // select SPI clock
        //Needs to be changed to
        //REG_SET_FIELD(CRG_PER, CLK_PER_REG, SPI_CLK_SEL, clk_per_reg_local, 1); // select SPI clock
        //In order to use the PLL as the source clock for the SPI bus
        ad_spi_init();
        displayInit();
        displayFillScreenBuf(display24to16Color(0x000000));
//        bool firstRun = true;
//        char messageFromTitle[]="FROM";
//        char messageContentTitle[]="MESSAGE";
//        char fakeTitle[]="MARISSA KOVEN";

//        displayImageFromMemory(0,0,WATCH_FACE_OFFSET);
//        displayImageFromMemory(22,45,FONT_OFFSET);

        for(;;)
        {
//                displayClearBuf();
//                displayDrawString(0,0,2,0,quickBrownCAPS);
//                OS_DELAY_MS(1000);
//                displayClearBuf();
//                displayDrawString(0,0,2,0,quickBrownLC);
//                OS_DELAY_MS(1000);
//                displayClearBuf();
////                displayDrawString(0,0,2,0,numberSymbol);
////                OS_DELAY_MS(2500);

//                for(int ycounter=0;ycounter<3;ycounter++)
//                {
//                        for(int xcounter=0;xcounter<3;xcounter++)
//                        {
//                                OS_DELAY_MS(500);
////                                displayFillScreenBuf(display24to16Color(0x00000));
////                                displayFillScreenBuf(display24to16Color(0xFFFFFF));
////                                displayPartialImageFromMemory(22+(xcounter*FONT_CHARACTER_WIDTH),45+(ycounter*FONT_CHARACTER_HEIGHT),xcounter*FONT_CHARACTER_WIDTH,ycounter*FONT_CHARACTER_HEIGHT,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
//                                displayPartialImageFromMemory(30+(xcounter*60),30+(ycounter*60),xcounter*60,ycounter*60,60,60,WATCH_FACE_OFFSET);
//                        }
//                }
//                for(int ycounter=0;ycounter<FONT_CHARACTERS_ROWS;ycounter++)
//                {
//                        for(int xcounter=0;xcounter<FONT_CHARACTERS_COLUMNS;xcounter++)
//                        {
//                                OS_DELAY_MS(500);
////                                displayFillScreenBuf(display24to16Color(0x00000));
////                                displayFillScreenBuf(display24to16Color(0xFFFFFF));
//                                displayPartialImageFromMemory(22+(xcounter*FONT_CHARACTER_WIDTH),45+(ycounter*FONT_CHARACTER_HEIGHT),xcounter*FONT_CHARACTER_WIDTH,ycounter*FONT_CHARACTER_HEIGHT,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,FONT_OFFSET);
////                                displayPartialImageFromMemory(30+(xcounter*FONT_CHARACTER_WIDTH),30+(ycounter*FONT_CHARACTER_HEIGHT),xcounter*FONT_CHARACTER_WIDTH,ycounter*FONT_CHARACTER_HEIGHT,FONT_CHARACTER_WIDTH,FONT_CHARACTER_HEIGHT,WATCH_FACE_OFFSET);
//                        }
//                }

                OS_BASE_TYPE ret;
                uint32_t notif;

                /*
                 * Wait on any of the notification bits, then clear them all
                 */
                ret = OS_TASK_NOTIFY_WAIT(0, OS_TASK_NOTIFY_ALL_BITS, &notif, OS_TASK_NOTIFY_FOREVER);
                OS_ASSERT(ret == OS_OK);

                /* Notified from BLE manager, can get event */
                if (notif & UPDATE_DISPLAY_MASK)
                {
//                        displayImageFromMemory(0,0,MARISSA_OFFSET);
//                        displayImageFromMemory(0,175,NEW_MESSAGE_OFFSET);
//                        OS_DELAY_MS(2500);
                        displayClearBuf();
//                        displayDrawString(ST7789_XSTART, ST7789_YSTART, 10, 0, 0xC618, messageFromTitle);//Ends at 20+10+5 = 35
//                        displayDrawString(ST7789_XSTART, 35, 10, 0, DISPLAY_WHITE, getANCSTitle());//Ends at 35+10+5+10+5 = 65
                        displayDrawString(0,0,2,0, getANCSTitle());//Ends at 35+10+5+10+5 = 65
//                        displayDrawString(ST7789_XSTART, 65, 10, 0, messageContentTitle);//Ends at 65+10+5=80
                        displayDrawString(0,70,2,0, getANCSMessage());
                        OS_DELAY_MS(4500);
                        displayImageFromMemory(0,0,WATCH_FACE_OFFSET);
                }
        }
}
