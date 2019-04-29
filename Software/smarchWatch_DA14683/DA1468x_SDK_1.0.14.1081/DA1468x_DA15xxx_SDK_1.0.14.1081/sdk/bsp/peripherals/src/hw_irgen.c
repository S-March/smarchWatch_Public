/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup IR_Generator
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file hw_irgen.c
 *
 * @brief Implementation of the IR generator Low Level Driver.
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configUSE_HW_IRGEN


#include <stdint.h>
#include "hw_irgen.h"

#if (dg_configSYSTEMVIEW)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif

static hw_irgen_interrupt_cb *intr_cb;

void hw_irgen_init(irgen_config *cfg)
{
        NVIC_DisableIRQ(IRGEN_IRQn);

        /* reset registers to default values */
        IR->IR_FREQ_CARRIER_ON_REG = 1;
        IR->IR_FREQ_CARRIER_OFF_REG = 1;
        IR->IR_LOGIC_ZERO_TIME_REG = 0x0101; // mark and space duration = 1 clock cycle
        IR->IR_LOGIC_ONE_TIME_REG = 0x0101;  // same here
        IR->IR_CTRL_REG = 0x0003; // flush both code and repeat FIFO
        IR->IR_REPEAT_TIME_REG = 0;

        /* enable clock and IR block */
        GLOBAL_INT_DISABLE();
        REG_SETF(CRG_PER, CLK_PER_REG, IR_CLK_ENABLE, 1);
        GLOBAL_INT_RESTORE();
        REG_SETF(IR, IR_CTRL_REG, IR_ENABLE, 1);

        hw_irgen_configure(cfg);
}

void hw_irgen_configure(irgen_config *cfg)
{
        if (cfg) {
                hw_irgen_set_carrier_freq(cfg->carrier_hi, cfg->carrier_lo);
                hw_irgen_set_logic0_param(cfg->logic0.format, cfg->logic0.mark_time,
                                                                        cfg->logic0.space_time);
                hw_irgen_set_logic1_param(cfg->logic1.format, cfg->logic1.mark_time,
                                                                        cfg->logic1.space_time);
                hw_irgen_set_repeat_fifo(cfg->repeat_fifo);
                hw_irgen_set_repeat_time(cfg->repeat_time);
                hw_irgen_set_output_type(cfg->output);
        }
}

void hw_irgen_register_interrupt(hw_irgen_interrupt_cb cb)
{
        intr_cb = cb;
        REG_SETF(IR, IR_CTRL_REG, IR_IRQ_EN, 1);
        NVIC_EnableIRQ(IRGEN_IRQn);
}

void hw_irgen_unregister_interrupt(void)
{
        NVIC_DisableIRQ(IRGEN_IRQn);
        REG_SETF(IR, IR_CTRL_REG, IR_IRQ_EN, 0);
        intr_cb = NULL;
}

void IRGEN_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        if (intr_cb != NULL) {
                intr_cb();
        }

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

#endif /* dg_configUSE_HW_IRGEN */
/**
 * \}
 * \}
 * \}
 */
