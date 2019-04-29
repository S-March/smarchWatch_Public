/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup QUAD_Decoder
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file hw_quad.c
 *
 * @brief Implementation of the QUAD Decoder Low Level Driver.
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configUSE_HW_QUAD


#include <stdio.h>
#include <hw_quad.h>

#if (dg_configSYSTEMVIEW)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif

static volatile hw_quad_handler_cb int_handler;

void hw_quad_register_interrupt(hw_quad_handler_cb handler, uint16_t thres_num)
{
        NVIC_EnableIRQ(QUADEC_IRQn);
        uint32_t qdec_ctrl_reg = QUAD->QDEC_CTRL_REG;
        REG_SET_FIELD(QUAD, QDEC_CTRL_REG, QD_IRQ_THRES, qdec_ctrl_reg, thres_num);
        REG_SET_FIELD(QUAD, QDEC_CTRL_REG, QD_IRQ_MASK, qdec_ctrl_reg, 1);
        QUAD->QDEC_CTRL_REG = qdec_ctrl_reg;
        int_handler = handler;
}

void hw_quad_unregister_interrupt(void)
{
        NVIC_DisableIRQ(QUADEC_IRQn);
        REG_CLR_BIT(QUAD, QDEC_CTRL_REG, QD_IRQ_MASK);
        int_handler = NULL;
}

void QUADEC_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        if (int_handler != NULL) {
                int_handler();
        }

        REG_SET_BIT(QUAD, QDEC_CTRL_REG, QD_IRQ_CLR);

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

#endif /* dg_configUSE_HW_QUAD */
/**
 * \}
 * \}
 * \}
 */
