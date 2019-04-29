/**
 ****************************************************************************************
 *
 * @file radio.c
 *
 * @brief RF functions
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include "radio.h"
#include "cc2420.h"
#include "cc2420_ctrl.h"
#include <sdk_defs.h>
//#include "spi.h"
#include <string.h>
//=========================== defines =========================================


//=========================== variables =======================================

typedef struct {
        cc2420_status_t radioStatusByte;
        radio_state_t   state;
} radio_vars_t;

radio_vars_t radio_vars;

//=========================== prototypes ======================================
#if dg_configUSE_FTDF_DDPHY == 0
void radio_spiStrobe     (uint8_t strobe, cc2420_status_t* statusRead);
void radio_spiWriteReg   (uint8_t reg,    cc2420_status_t* statusRead, uint16_t regValueToWrite);
void radio_spiReadReg    (uint8_t reg,    cc2420_status_t* statusRead, uint8_t* regValueRead);
void radio_spiWriteTxFifo(                cc2420_status_t* statusRead, uint8_t* bufToWrite, uint8_t  lenToWrite);
void radio_spiReadRxFifo (                cc2420_status_t* statusRead, uint8_t* bufRead,    uint8_t* lenRead, uint8_t maxBufLen);
#endif
//=========================== public ==========================================

//===== admin
void radio_init(void) {
        /*
         * Init System
         */
        GLOBAL_INT_DISABLE();
        REG_SETF(CRG_TOP, CLK_AMBA_REG, HCLK_DIV, 0);
        REG_SETF(CRG_TOP, CLK_AMBA_REG, PCLK_DIV, 0);
        REG_SETF(CRG_TOP, CLK_CTRL_REG, SYS_CLK_SEL, 0);        // 0:XTAL16M, 1:RC16M, 2:LP_CLK, 3:PLL96M

        // Enable RF Control Unit clock
        REG_SET_BIT(CRG_TOP, CLK_RADIO_REG, RFCU_ENABLE);
        GLOBAL_INT_RESTORE();
}

