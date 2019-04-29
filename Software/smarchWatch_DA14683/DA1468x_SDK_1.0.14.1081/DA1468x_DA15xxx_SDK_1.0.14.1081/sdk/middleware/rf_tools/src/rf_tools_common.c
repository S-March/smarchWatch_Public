/**
 ****************************************************************************************
 *
 * @file rf_tools_common.c
 *
 * @brief RF Tools common code
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#if defined(CONFIG_USE_BLE) || defined(CONFIG_USE_FTDF)
#include <string.h>
#include <stdbool.h>
#include "osal.h"
#include "sdk_defs.h"

#include "rf_tools_common.h"

systick_cb_t systick_cb = NULL;

void rf_tools_start_systick(systick_cb_t cb, uint32_t ticks)
{
        portENTER_CRITICAL();
        SysTick->LOAD = (ticks << 4) - 1;
        SysTick->VAL = 0;
        SysTick->CTRL |= 0x07;  /* Use 1MHz clock, generate irq, enable */
        systick_cb = cb;
        portEXIT_CRITICAL();
}

void rf_tools_stop_systick(void)
{
        portENTER_CRITICAL();
        SysTick->CTRL = 0;
        systick_cb = NULL;
        NVIC_DisableIRQ(SysTick_IRQn);
        NVIC_ClearPendingIRQ(SysTick_IRQn);
        portEXIT_CRITICAL();
}


void SysTick_Handler(void)
{
        if (systick_cb != NULL)
                systick_cb();
}

#endif
