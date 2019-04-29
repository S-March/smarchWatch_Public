/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup Timer0
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file hw_timer0.c
 *
 * @brief Implementation of the Timer0 Low Level Driver.
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configUSE_HW_TIMER0


#include <stdio.h>
#include <sdk_defs.h>
#include <hw_timer0.h>

#if (dg_configSYSTEMVIEW)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif

static hw_timer0_interrupt_cb intr_cb;

void hw_timer0_init(const timer0_config *cfg)
{
        /* Enable clock for peripheral */
        GLOBAL_INT_DISABLE();
        uint32_t clk_tmr_reg = CRG_TOP->CLK_TMR_REG;
        clk_tmr_reg &= ~CRG_TOP_CLK_TMR_REG_TMR0_DIV_Msk;
        clk_tmr_reg &= ~CRG_TOP_CLK_TMR_REG_TMR0_CLK_SEL_Msk;
        clk_tmr_reg |= CRG_TOP_CLK_TMR_REG_TMR0_ENABLE_Msk;
        CRG_TOP->CLK_TMR_REG = clk_tmr_reg ;
        GLOBAL_INT_RESTORE();

        /* Reset control register, i.e. disable timer */
        GP_TIMERS->TIMER0_CTRL_REG = 0x0;

        /* Disable NVIC interrupt */
        NVIC_DisableIRQ(SWTIM0_IRQn);

        intr_cb = NULL;

        hw_timer0_configure(cfg);
}

void hw_timer0_configure(const timer0_config *cfg)
{
        if (cfg) {
                hw_timer0_set_clock_source(cfg->clk_src);
                hw_timer0_set_fast_clock_div(cfg->fast_clk_div);
                hw_timer0_set_on_clock_div(cfg->on_clock_div);
                hw_timer0_set_on_reload(cfg->on_reload);
                hw_timer0_set_t0_reload(cfg->t0_reload_m, cfg->t0_reload_n);
        }
}

void hw_timer0_register_int(hw_timer0_interrupt_cb handler)
{
        intr_cb = handler;
        NVIC_EnableIRQ(SWTIM0_IRQn);
}

void hw_timer0_unregister_int(void)
{
        NVIC_DisableIRQ(SWTIM0_IRQn);
        intr_cb = NULL;
}

void SWTIM0_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        if (intr_cb) {
                intr_cb();
        }

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

#endif /* dg_configUSE_HW_TIMER0 */
/**
 * \}
 * \}
 * \}
 */
