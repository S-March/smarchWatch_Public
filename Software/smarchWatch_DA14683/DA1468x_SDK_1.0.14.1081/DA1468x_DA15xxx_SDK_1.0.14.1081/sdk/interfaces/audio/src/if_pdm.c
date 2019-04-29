/**
 ****************************************************************************************
 *
 * @file if_pdm.c
 *
 * @brief PDM audio interface driver.
 *
 * Copyright (C) 2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configUSE_IF_PDM

#include "sdk_defs.h"
#include "if_pdm.h"
#include "hw_gpio.h"
#include <assert.h>

__RETAINED static if_pdm_src_interrupt_cb if_pdm_src_callback;

static void if_pdm_config_port_pins(const if_pdm_config *config)
{
        if (config->mode == IF_PDM_MODE_MASTER) {
                hw_gpio_configure_pin(config->clk_gpio.port, config->clk_gpio.pin,
                        HW_GPIO_MODE_OUTPUT, HW_GPIO_FUNC_PDM_CLK, false);
        } else {
                hw_gpio_configure_pin(config->clk_gpio.port, config->clk_gpio.pin,
                        HW_GPIO_MODE_INPUT, HW_GPIO_FUNC_PDM_CLK, false);
        }

        if (config->direction == IF_PDM_DIRECTION_IN) {
                hw_gpio_configure_pin(config->data_gpio.port, config->data_gpio.pin,
                        HW_GPIO_MODE_INPUT, HW_GPIO_FUNC_PDM_DI, false);
        } else {
                hw_gpio_configure_pin(config->data_gpio.port, config->data_gpio.pin,
                        HW_GPIO_MODE_OUTPUT, HW_GPIO_FUNC_PDM_DO, false);
        }
}

static void if_pdm_set_master_clock(unsigned int pdm_clock_divider)
{
        unsigned int div_field;
        if (pdm_clock_divider == 0) {
                div_field = 8;
        } else {
                div_field = pdm_clock_divider;
        }

        GLOBAL_INT_DISABLE();
        uint32_t reg = CRG_PER->PDM_DIV_REG;
        REG_SET_FIELD(CRG_PER, PDM_DIV_REG, PDM_DIV, reg, div_field);
        REG_SET_FIELD(CRG_PER, PDM_DIV_REG, CLK_PDM_EN, reg, 1);
        CRG_PER->PDM_DIV_REG = reg;
        GLOBAL_INT_RESTORE();
}

static void if_pdm_set_src_clock()
{
        GLOBAL_INT_DISABLE();
        uint32_t reg = CRG_PER->SRC_DIV_REG;
        REG_SET_FIELD(CRG_PER, SRC_DIV_REG, SRC_DIV, reg, 1);
        REG_SET_FIELD(CRG_PER, SRC_DIV_REG, CLK_SRC_EN, reg, 1);
        CRG_PER->SRC_DIV_REG = reg;
        GLOBAL_INT_RESTORE();
}

static void if_pdm_enable_interrupt(bool enable_interrupt, unsigned int int_prio,
        IF_PDM_DIRECTION direction, if_pdm_src_interrupt_cb callback)
{
        if (enable_interrupt) {
                if_pdm_src_callback = callback;

                if (direction == IF_PDM_DIRECTION_IN) {
                        NVIC_ClearPendingIRQ(SRC_OUT_IRQn);
                        NVIC_SetPriority(SRC_OUT_IRQn, int_prio);
                        NVIC_EnableIRQ(SRC_OUT_IRQn);
                } else {
                        NVIC_ClearPendingIRQ(SRC_IN_IRQn);
                        NVIC_SetPriority(SRC_IN_IRQn, int_prio);
                        NVIC_EnableIRQ(SRC_IN_IRQn);
                }
        } else {
                if_pdm_src_callback = NULL;
        }
}

static void if_pdm_set_src_sample_rate(IF_PDM_DIRECTION direction, unsigned int sample_rate)
{
        uint32_t fs_reg;
        unsigned int multiplier = 8192;

        /* sample_rate is up to 192000. */
        if (sample_rate > 96000) {
                multiplier = 8192 / 4;
        } else if (sample_rate > 48000) {
                multiplier = 8192 / 2;
        }
        fs_reg = (multiplier * sample_rate / 100);

        if (direction == IF_PDM_DIRECTION_IN) {
                APU->SRC1_OUT_FS_REG = fs_reg;
        } else {
                APU->SRC1_IN_FS_REG = fs_reg;
        }
}

static void if_pdm_set_src_filters(IF_PDM_DIRECTION direction, unsigned int sample_rate)
{
        int setting = 0;

        if (sample_rate > 96000) {
                setting = 3;
        } else if (sample_rate > 48000) {
                setting = 1;
        }

        if (direction == IF_PDM_DIRECTION_IN) {
                REG_SETF(APU, SRC1_CTRL_REG, SRC_OUT_US, setting);
        } else {
                REG_SETF(APU, SRC1_CTRL_REG, SRC_IN_DS, setting);
        }
}

static void if_pdm_set_apu_reg(IF_PDM_DIRECTION direction, IF_PDM_SRC_DIRECTION src_direction)
{
        if (src_direction == IF_PDM_SRC_DIRECTION_REG) {
                if (direction == IF_PDM_DIRECTION_IN) {
                        REG_SETF(APU, APU_MUX_REG, PDM1_MUX_IN, 1);
                } else {
                        REG_SETF(APU, APU_MUX_REG, SRC1_MUX_IN, 2);
                        REG_SETF(APU, APU_MUX_REG, PDM1_MUX_IN, 0);
                }
        } else {
                if (direction == IF_PDM_DIRECTION_IN) {
                        REG_SETF(APU, APU_MUX_REG, PDM1_MUX_IN, 1);
                        REG_SETF(APU, APU_MUX_REG, PCM1_MUX_IN, 1);
                } else {
                        REG_SETF(APU, APU_MUX_REG, SRC1_MUX_IN, 1);
                        REG_SETF(APU, APU_MUX_REG, PDM1_MUX_IN, 0);
                }
        }
 }

void if_pdm_enable(const if_pdm_config *config)
{
        if_pdm_disable();

        if_pdm_config_port_pins(config);
        GLOBAL_INT_DISABLE();
        REG_SETF(CRG_PER, PDM_DIV_REG, PDM_MASTER_MODE, config->mode & 0x1);
        GLOBAL_INT_RESTORE();
        if (config->mode == IF_PDM_MODE_MASTER) {
                if_pdm_set_master_clock(config->pdm_div);
        }

        if_pdm_enable_interrupt(config->enable_interrupt, config->interrupt_priority,
                config->direction, config->callback);

        if_pdm_set_apu_reg(config->direction, config->src_direction);
        if_pdm_set_src_clock();

        if (config->src_direction == IF_PDM_SRC_DIRECTION_REG) {
                if_pdm_set_src_sample_rate(config->direction, config->src_sample_rate);
        }

        if_pdm_set_src_filters(config->direction, config->src_sample_rate);
        REG_SETF(APU, SRC1_CTRL_REG, SRC_OUT_AMODE,
                (config->direction == IF_PDM_DIRECTION_IN &&
                        config->src_direction == IF_PDM_SRC_DIRECTION_PCM) ? 1 : 0);
        REG_SETF(APU, SRC1_CTRL_REG, SRC_IN_AMODE,
                (config->direction != IF_PDM_DIRECTION_IN &&
                        config->src_direction == IF_PDM_SRC_DIRECTION_PCM) ? 1 : 0);
        REG_SETF(APU, SRC1_CTRL_REG, SRC_PDM_MODE, config->direction & 0x3);
        REG_SETF(APU, SRC1_CTRL_REG, SRC_OUT_CAL_BYPASS, config->bypass_out_filter & 0x1);
        REG_SETF(APU, SRC1_CTRL_REG, SRC_IN_CAL_BYPASS, config->bypass_in_filter & 0x1);
        REG_SETF(APU, SRC1_CTRL_REG, SRC_DITHER_DISABLE, !config->enable_dithering & 0x1);

        REG_SET_BIT(APU, SRC1_CTRL_REG, SRC_EN);
}

void if_pdm_disable(void)
{
        NVIC_DisableIRQ(SRC_IN_IRQn);
        NVIC_DisableIRQ(SRC_OUT_IRQn);
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_PER, PDM_DIV_REG, CLK_PDM_EN);
        REG_CLR_BIT(CRG_PER, SRC_DIV_REG, CLK_SRC_EN);
        GLOBAL_INT_RESTORE();
}

void SRC_IN_Handler(void)
{
        if (if_pdm_src_callback) {
                if_pdm_src_isr_data data;
                data.src_out1_value = 0;
                data.src_out2_value = 0;

                data.src_in1_reg = (uint32_t *)&(APU->SRC1_IN1_REG);
                data.src_in2_reg = (uint32_t *)&(APU->SRC1_IN2_REG);

                if_pdm_src_callback(&data);
        }
}

void SRC_OUT_Handler(void)
{
        if (if_pdm_src_callback) {
                if_pdm_src_isr_data data;

                data.src_out1_value = APU->SRC1_OUT1_REG;
                data.src_out2_value = APU->SRC1_OUT2_REG;

                data.src_in1_reg = (uint32_t *) NULL;
                data.src_in2_reg = (uint32_t *) NULL;

                if_pdm_src_callback(&data);
        }
}

#endif /* dg_configUSE_IF_PDM */
