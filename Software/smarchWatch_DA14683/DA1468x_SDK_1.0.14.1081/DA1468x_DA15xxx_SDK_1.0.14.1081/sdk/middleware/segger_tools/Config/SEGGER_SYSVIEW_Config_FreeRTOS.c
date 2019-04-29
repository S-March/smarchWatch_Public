/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 2015 - 2016  SEGGER Microcontroller GmbH & Co. KG        *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
*                                                                    *
*       SEGGER SystemView * Real-time application analysis           *
*                                                                    *
**********************************************************************
*                                                                    *
* All rights reserved.                                               *
*                                                                    *
* SEGGER strongly recommends to not make any changes                 *
* to or modify the source code of this software in order to stay     *
* compatible with the RTT protocol and J-Link.                       *
*                                                                    *
* Redistribution and use in source and binary forms, with or         *
* without modification, are permitted provided that the following    *
* conditions are met:                                                *
*                                                                    *
* o Redistributions of source code must retain the above copyright   *
*   notice, this list of conditions and the following disclaimer.    *
*                                                                    *
* o Redistributions in binary form must reproduce the above          *
*   copyright notice, this list of conditions and the following      *
*   disclaimer in the documentation and/or other materials provided  *
*   with the distribution.                                           *
*                                                                    *
* o Neither the name of SEGGER Microcontroller GmbH & Co. KG         *
*   nor the names of its contributors may be used to endorse or      *
*   promote products derived from this software without specific     *
*   prior written permission.                                        *
*                                                                    *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND             *
* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,        *
* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF           *
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE           *
* DISCLAIMED. IN NO EVENT SHALL SEGGER Microcontroller BE LIABLE FOR *
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR           *
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT  *
* OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;    *
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF      *
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT          *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE  *
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH   *
* DAMAGE.                                                            *
*                                                                    *
**********************************************************************
*                                                                    *
*       SystemView version: V2.40a                                    *
*                                                                    *
**********************************************************************
-------------------------- END-OF-HEADER -----------------------------

File    : SEGGER_SYSVIEW_Config_FreeRTOS.c
Purpose : Sample setup configuration of SystemView with FreeRTOS.
Revision: $Rev: 3734 $
*/

#if dg_configSYSTEMVIEW

#include "FreeRTOS.h"
#include "task.h"
#include "SEGGER_SYSVIEW.h"
#include "sys_rtc.h"
#include "interrupts.h"

extern const SEGGER_SYSVIEW_OS_API SYSVIEW_X_OS_TraceAPI;

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/
// The application name to be displayed in SystemViewer
#define SYSVIEW_APP_NAME        "DemoApp"

// The target device name
#define SYSVIEW_DEVICE_NAME     "DA1468x"

// Frequency of the timestamp. Must match SEGGER_SYSVIEW_GET_TIMESTAMP in SEGGER_SYSVIEW_Conf.h
#define SYSVIEW_TIMESTAMP_FREQ  (configSYSTICK_CLOCK_HZ)

// System Frequency. SystemcoreClock is used in most CMSIS compatible projects.
#define SYSVIEW_CPU_FREQ        configCPU_CLOCK_HZ

// The lowest RAM address used for IDs (pointers)
#define SYSVIEW_RAM_BASE        (0x07fc0000)

/*********************************************************************
*
*       _cbSendSystemDesc()
*
*  Function description
*    Sends SystemView description strings.
*/
static void _cbSendSystemDesc(void) {
        /*
           * The maximum size of the string passed as argument to SEGGER_SYSVIEW_SendSysDesc()
           * should not exceed SEGGER_SYSVIEW_MAX_STRING_LEN (128) bytes. Values can be comma
           * seperated.
           */
          SEGGER_SYSVIEW_SendSysDesc("N="SYSVIEW_APP_NAME",D="SYSVIEW_DEVICE_NAME",O=FreeRTOS,I#16=BLE_WKUP_LP_IRQ,I#17=BLE_GEN_IRQ,I#35=TIM1_IRQ,I#42=DMA_IRQ");

          /*
           * More ISR entries could be added but this would result in a slower system and might
           * also affect time critical tasks or trigger assertions.
           *
           * This is because multiple SEGGER_SYSVIEW_SendSysDesc() calls will result in multiple
           * RTT transactions.
           *
           * Note also that _cbSendSystemDesc() is called multiple times from the host PC and not
           * just during initialization, so assertions may occur anytime during SystemView monitoring.
           *
           */
        #if 0
          SEGGER_SYSVIEW_SendSysDesc("I#16=BLE_WAKEUP_LP_Handler,I#17=BLE_GEN_Handler,I#18=FTDF_WAKEUP_Handler,I#19=FTDF_GEN_Handler,I#20=RFCAL_Handler");
          SEGGER_SYSVIEW_SendSysDesc("I#21=COEX_Handler,I#22=CRYPTO_Handler,I#23=MRM_Handler,I#24=UART_Handler,I#25=UART2_Handler,I#26=I2C_Handler");
          SEGGER_SYSVIEW_SendSysDesc("I#27=I2C2_Handler,I#28=SPI_Handler,I#29=SPI2_Handler,I#30=ADC_Handler,I#31=KEYBRD_Handler");
          SEGGER_SYSVIEW_SendSysDesc("I#32=IRGEN_Handler,I#33=WKUP_GPIO_Handler,I#34=SWTIM0_Handler,I#35=SWTIM1_Handler,I#36=QUADEC_Handler");
          SEGGER_SYSVIEW_SendSysDesc("I#37=USB_Handler,I#38=PCM_Handler,I#39=SRC_IN_Handler,I#40=SRC_OUT_Handler,I#41=VBUS_Handler");
          SEGGER_SYSVIEW_SendSysDesc("I#42=DMA_Handler,I#43=RF_DIAG_Handler,I#44=TRNG_Handler,I#45=DCDC_Handler,I#46=XTAL16RDY_Handler");
        #endif
}

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/
void SEGGER_SYSVIEW_Conf(void) {
  SEGGER_SYSVIEW_Init(SYSVIEW_TIMESTAMP_FREQ, SYSVIEW_CPU_FREQ,
                      &SYSVIEW_X_OS_TraceAPI, _cbSendSystemDesc);
  SEGGER_SYSVIEW_SetRAMBase(SYSVIEW_RAM_BASE);
}

U32 SEGGER_SYSVIEW_X_GetTimestamp(){
        if(in_interrupt()){
                return rtc_get_fromISR();
        }else{
                /*
                 * SEGGER_SYSVIEW_X_GetTimestamp() will always be called from within
                 * a SEGGER_RTT_LOCK()/UNLOCK zone, which sets PRIOMASK to 1 masking
                 * all interrupts except the non-maskable ones. It keeps track on the
                 * current priomask using a local variable.
                 *
                 * rtc_get() will also disable interrupts using vPortEnterCritical()
                 * but instead uses "cpsid" and therefore the status of the previous SEGGER_RTT_LOCK().
                 * Will be lost.  When vPortEnterCritical() gets called interrupts will be enabled again,
                 * but SEGGER will think it is still inside a protected zone until SEGGER_RTT_UNLOCK() is called.
                 * This will result in corrupted RTT packets.
                 *
                 * This is the reason for calling rtc_get_fromCPM() instead of rtc_get().
                 *
                 */
                uint32_t lp_prescaled_time;
                uint32_t lp_current_time;

                return rtc_get_fromCPM(&lp_prescaled_time, &lp_current_time);
        }
}

#endif /* dg_configSYSTEMVIEW */

/*************************** End of file ****************************/
