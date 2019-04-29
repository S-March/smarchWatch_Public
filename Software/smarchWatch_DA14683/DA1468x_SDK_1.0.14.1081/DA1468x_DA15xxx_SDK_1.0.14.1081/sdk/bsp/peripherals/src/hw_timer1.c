/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup Timer1
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file hw_timer1.c
 *
 * @brief Implementation of the Timer1 Low Level Driver.
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configUSE_HW_TIMER1


#include <stdio.h>
#include <sdk_defs.h>
#include <hw_timer1.h>


#if dg_configUSER_CAN_USE_TIMER1 == 1


static hw_timer1_handler_cb intr_cb;

void hw_timer1_init(HW_TIMER1_MODE mode, const timer1_config *cfg)
{
        hw_timer1_disable();                        // disable timer

        /* Enable clock for peripheral */
        GLOBAL_INT_DISABLE();
        uint32_t clk_tmr_reg = CRG_TOP->CLK_TMR_REG;
        clk_tmr_reg &= ~CRG_TOP_CLK_TMR_REG_TMR1_DIV_Msk;        // division factor for timer (divided by 1)
        clk_tmr_reg &= ~CRG_TOP_CLK_TMR_REG_TMR1_CLK_SEL_Msk;    // select the clock source (DIVN clock)
        clk_tmr_reg |= CRG_TOP_CLK_TMR_REG_TMR1_ENABLE_Msk;      // enable timer clock
        CRG_TOP->CLK_TMR_REG = clk_tmr_reg ;
        GLOBAL_INT_RESTORE();

        /* Reset control register, i.e. disable timer */
        TIMER1->CAPTIM_CTRL_REG = 0;

        /* Disable NVIC interrupt */
        NVIC_DisableIRQ(SWTIM1_IRQn);

        intr_cb = NULL;

        hw_timer1_configure(mode, cfg);
}

void hw_timer1_configure(HW_TIMER1_MODE mode, const timer1_config *cfg)
{
        if (cfg) {
                hw_timer1_set_clk(cfg->clk_src);
                hw_timer1_set_prescaler(cfg->prescaler);

                if (mode == HW_TIMER1_MODE_ONESHOT) {
                        hw_timer1_configure_oneshot(&cfg->oneshot);
                } else {
                        hw_timer1_configure_timer(&cfg->timer);
                }

                /* always configure PWM */
                hw_timer1_configure_pwm(&cfg->pwm);
        }

        hw_timer1_set_mode(mode);
}

void hw_timer1_configure_timer(const timer1_config_timer_capture *cfg)
{
        hw_timer1_set_direction(cfg->direction);
        hw_timer1_set_reload(cfg->reload_val);
        hw_timer1_set_freerun(cfg->free_run);

        hw_timer1_set_event1_gpio(cfg->gpio1);
        hw_timer1_set_event1_trigger(cfg->trigger1);
        hw_timer1_set_event2_gpio(cfg->gpio2);
        hw_timer1_set_event2_trigger(cfg->trigger2);
}

void hw_timer1_configure_oneshot(const timer1_config_oneshot *cfg)
{
        hw_timer1_set_reload(cfg->delay);
        hw_timer1_set_shot_width(cfg->shot_width);
        hw_timer1_set_event1_gpio(cfg->gpio);
        hw_timer1_set_event1_trigger(cfg->trigger);
}

void hw_timer1_register_int(hw_timer1_handler_cb handler)
{
        intr_cb = handler;
        HW_TIMER1_REG_SETF(CTRL, CAPTIM_IRQ_EN, 1);
        NVIC_EnableIRQ(SWTIM1_IRQn);
}

void hw_timer1_unregister_int(void)
{
        NVIC_DisableIRQ(SWTIM1_IRQn);
        HW_TIMER1_REG_SETF(CTRL, CAPTIM_IRQ_EN, 0);
        intr_cb = NULL;
}

void hw_timer1_trigger_int(void)
{
        NVIC_SetPendingIRQ(SWTIM1_IRQn);
}

void SWTIM1_Handler(void)
{
        if (intr_cb != NULL) {
                intr_cb();
        }
}


#else // dg_configUSER_CAN_USE_TIMER1 == 0


void hw_timer1_lp_clk_init(void)
{
        uint32_t tmp_val;

        hw_timer1_disable();                        // disable timer

        tmp_val = CRG_TOP->CLK_TMR_REG;
        tmp_val &= ~CRG_TOP_CLK_TMR_REG_TMR1_DIV_Msk;           // division factor for timer (divided by 1)
        tmp_val &= ~CRG_TOP_CLK_TMR_REG_TMR1_CLK_SEL_Msk;       // select the clock source (DIVN clock)
        tmp_val |= CRG_TOP_CLK_TMR_REG_TMR1_ENABLE_Msk;         // enable timer clock
        CRG_TOP->CLK_TMR_REG = tmp_val;

        TIMER1->CAPTIM_CTRL_REG = TIMER1_CAPTIM_CTRL_REG_CAPTIM_FREE_RUN_MODE_EN_Msk;

#if dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A
        TIMER1->CAPTIM_PRESCALER_REG = dg_configTim1Prescaler;
#else
        TIMER1->CAPTIM_PRESCALER_REG = 0;
#endif
}
#endif // dg_configUSER_CAN_USE_TIMER1


void hw_timer1_configure_pwm(const timer1_config_pwm *cfg)
{
        hw_timer1_set_pwm_freq(cfg->frequency);
        hw_timer1_set_pwm_duty_cycle(cfg->duty_cycle);
}

#endif /* dg_configUSE_HW_TIMER1 */
/**
 * \}
 * \}
 * \}
 */
