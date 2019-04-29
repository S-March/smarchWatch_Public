/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup Timer2
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file hw_timer2.c
 *
 * @brief Implementation of the Timer2 Low Level Driver.
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configUSE_HW_TIMER2


#include <sdk_defs.h>
//#include <hw_gpio.h>
#include <hw_timer2.h>

void hw_timer2_init(timer2_config *cfg)
{
        /* Enable clock for peripheral */
        GLOBAL_INT_DISABLE();
        uint32_t clk_tmr_reg = CRG_TOP->CLK_TMR_REG;
        clk_tmr_reg &= ~CRG_TOP_CLK_TMR_REG_TMR2_DIV_Msk;        // division factor for timer (divided by 1)
        clk_tmr_reg &= ~CRG_TOP_CLK_TMR_REG_TMR2_CLK_SEL_Msk;    // select the clock source (DIVN clock)
        clk_tmr_reg |= CRG_TOP_CLK_TMR_REG_TMR2_ENABLE_Msk;      // enable timer clock
        CRG_TOP->CLK_TMR_REG = clk_tmr_reg ;
        GLOBAL_INT_RESTORE();

        /* Reset control register, i.e. disable timer */
        GP_TIMERS->TRIPLE_PWM_CTRL_REG = 0x8;                    // By default, the fast clock is selected
        GP_TIMERS->TRIPLE_PWM_FREQUENCY = 0;
        GP_TIMERS->PWM2_START_CYCLE = 0;
        GP_TIMERS->PWM2_END_CYCLE = 0;
        GP_TIMERS->PWM3_START_CYCLE = 0;
        GP_TIMERS->PWM3_END_CYCLE = 0;
        GP_TIMERS->PWM4_START_CYCLE = 0;
        GP_TIMERS->PWM4_END_CYCLE = 0;

        hw_timer2_configure(cfg);
}

void hw_timer2_configure(timer2_config *cfg)
{
        if (cfg) {
                hw_timer2_set_frequency(cfg->frequency);
                hw_timer2_set_pwm_start_end(HW_TIMER2_PWM_2, cfg->pwm2_start, cfg->pwm2_end);
                hw_timer2_set_pwm_start_end(HW_TIMER2_PWM_3, cfg->pwm3_start, cfg->pwm3_end);
                hw_timer2_set_pwm_start_end(HW_TIMER2_PWM_4, cfg->pwm4_start, cfg->pwm4_end);
        }
}

void hw_timer2_set_pwm_duty_cycle(HW_TIMER2_PWM pwm, uint16_t duty_cycle)
{
        switch (pwm) {
        case HW_TIMER2_PWM_2:
                GP_TIMERS->PWM2_START_CYCLE = 0;
                GP_TIMERS->PWM2_END_CYCLE = duty_cycle;
                break;
        case HW_TIMER2_PWM_3:
                GP_TIMERS->PWM3_START_CYCLE = 0;
                GP_TIMERS->PWM3_END_CYCLE = duty_cycle;
                break;
        case HW_TIMER2_PWM_4:
                GP_TIMERS->PWM4_START_CYCLE = 0;
                GP_TIMERS->PWM4_END_CYCLE = duty_cycle;
                break;
        default:
                ASSERT_ERROR(0);
        }
}

uint16_t hw_timer2_get_pwm_duty_cycle(HW_TIMER2_PWM pwm)
{
        switch (pwm) {
        case HW_TIMER2_PWM_2:
                return ( (GP_TIMERS->PWM2_END_CYCLE) & HW_TIMER2_MAX_VALUE ) -
                                                ( (GP_TIMERS->PWM2_START_CYCLE) & HW_TIMER2_MAX_VALUE );
        case HW_TIMER2_PWM_3:
                return ( (GP_TIMERS->PWM3_END_CYCLE) & HW_TIMER2_MAX_VALUE ) -
                                                ( (GP_TIMERS->PWM3_START_CYCLE) & HW_TIMER2_MAX_VALUE );
        case HW_TIMER2_PWM_4:
                return ( (GP_TIMERS->PWM4_END_CYCLE) & HW_TIMER2_MAX_VALUE ) -
                                                ( (GP_TIMERS->PWM4_START_CYCLE) & HW_TIMER2_MAX_VALUE );
        default:
                ASSERT_ERROR(0);
                return 0;
        }
}

void hw_timer2_set_pwm_start_end(HW_TIMER2_PWM pwm, uint16_t start, uint16_t stop)
{
        switch (pwm) {
        case HW_TIMER2_PWM_2:
                GP_TIMERS->PWM2_START_CYCLE = start;
                GP_TIMERS->PWM2_END_CYCLE = stop;
                break;
        case HW_TIMER2_PWM_3:
                GP_TIMERS->PWM3_START_CYCLE = start;
                GP_TIMERS->PWM3_END_CYCLE = stop;
                break;
        case HW_TIMER2_PWM_4:
                GP_TIMERS->PWM4_START_CYCLE = start;
                GP_TIMERS->PWM4_END_CYCLE = stop;
                break;
        default:
                ASSERT_ERROR(0);
        }
}

void hw_timer2_get_pwm_start_end(HW_TIMER2_PWM pwm, uint16_t *start, uint16_t *stop)
{
        switch (pwm) {
        case HW_TIMER2_PWM_2:
                *start = GP_TIMERS->PWM2_START_CYCLE;
                *stop = GP_TIMERS->PWM2_END_CYCLE;
                break;
        case HW_TIMER2_PWM_3:
                *start = GP_TIMERS->PWM3_START_CYCLE;
                *stop = GP_TIMERS->PWM3_END_CYCLE;
                break;
        case HW_TIMER2_PWM_4:
                *start = GP_TIMERS->PWM4_START_CYCLE;
                *stop = GP_TIMERS->PWM4_END_CYCLE;
                break;
        default:
                ASSERT_ERROR(0);
        }
}

#endif /* dg_configUSE_HW_TIMER2 */
/**
 * \}
 * \}
 * \}
 */
