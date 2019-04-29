/**
 ****************************************************************************************
 *
 * @file hw_trng.c
 *
 * @brief Implementation of the True Random Number Generator Low Level Driver.
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configUSE_HW_TRNG

#include <hw_trng.h>

#if (dg_configSYSTEMVIEW)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif

#define HW_TRNG_FIFO_DEPTH      (32)

static hw_trng_cb trng_cb;

void hw_trng_disable_clk(void)
{
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_TOP, CLK_AMBA_REG, TRNG_CLK_ENABLE);
        GLOBAL_INT_RESTORE();
}

void hw_trng_clear_pending(void)
{
        uint32_t dummy __attribute__((unused));
        /* read TRNG_FIFOLVL_REG to clear level-sensitive source. */
        dummy = TRNG->TRNG_FIFOLVL_REG;
        NVIC_ClearPendingIRQ(TRNG_IRQn);
}

void hw_trng_enable(hw_trng_cb callback)
{
        if (callback != NULL) {
                trng_cb = callback;
                hw_trng_clear_pending();
                NVIC_EnableIRQ(TRNG_IRQn);
        }

        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, CLK_AMBA_REG, TRNG_CLK_ENABLE);
        GLOBAL_INT_RESTORE();
        REG_SET_BIT(TRNG, TRNG_CTRL_REG, TRNG_ENABLE);
}

void hw_trng_get_numbers(uint32_t* buffer, uint8_t size)
{
        if (size > HW_TRNG_FIFO_DEPTH)
                size = HW_TRNG_FIFO_DEPTH;

        for (int i = 0; i < size ; i++) {
                buffer[i] = hw_trng_get_number();
        }
}

__RETAINED_CODE uint8_t hw_trng_get_fifo_level(void)
{
        return (TRNG->TRNG_FIFOLVL_REG) & (REG_MSK(TRNG, TRNG_FIFOLVL_REG, TRNG_FIFOLVL) |
                                           REG_MSK(TRNG, TRNG_FIFOLVL_REG, TRNG_FIFOFULL));
}

void hw_trng_disable_interrupt(void)
{
        NVIC_DisableIRQ(TRNG_IRQn);
        trng_cb = NULL;
}

void hw_trng_disable(void)
{
        hw_trng_stop();
        hw_trng_disable_interrupt();
        hw_trng_clear_pending();
        hw_trng_disable_clk();
}

void TRNG_Handler(void)
{
        uint32_t dummy __attribute__((unused));

        SEGGER_SYSTEMVIEW_ISR_ENTER();

        if (trng_cb != NULL)
                trng_cb();

        /* read TRNG_FIFOLVL_REG to clear level-sensitive source. */
        dummy = TRNG->TRNG_FIFOLVL_REG;

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

#endif /* dg_configUSE_HW_TRNG */
