/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup Timer2
 * \{
 * \brief Timer2
 */

/**
 ****************************************************************************************
 *
 * @file hw_timer2.h
 *
 * @brief Definition of API for the Timer2 Low Level Driver.
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_TIMER2_H_
#define HW_TIMER2_H_

#if dg_configUSE_HW_TIMER2

#include <stdint.h>
#include <sdk_defs.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * \brief Get the mask of a field of an TIMER2 register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to access
 *
 */
#define HW_TIMER2_REG_FIELD_MASK(reg, field) \
        (GP_TIMERS_TRIPLE_PWM_##reg##_REG_##field##_Msk)

/**
 * \brief Get the bit position of a field of an TIMER2 register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to access
 *
 */
#define HW_TIMER2_REG_FIELD_POS(reg, field) \
        (GP_TIMERS_TRIPLE_PWM_##reg##_REG_##field##_Pos)

/**
 * \brief Prepare (i.e. shift and mask) a value to be used for an TIMER2 register field.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to access
 * \param [in] val is the value to prepare
 *
 */
#define HW_TIMER2_FIELD_VAL(reg, field, val) \
        (((val) << HW_TIMER2_REG_FIELD_POS(reg, field)) & HW_TIMER2_REG_FIELD_MASK(reg, field))

/**
 * \brief Get the value of a field of an TIMER2 register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to write
 *
 * \return the value of the register field
 *
 */
#define HW_TIMER2_REG_GETF(reg, field) \
                ((GP_TIMERS->TRIPLE_PWM_##reg##_REG & (GP_TIMERS_TRIPLE_PWM_##reg##_REG_##field##_Msk)) >> \
                (GP_TIMERS_TRIPLE_PWM_##reg##_REG_##field##_Pos))

/**
 * \brief Set the value of a field of an TIMER2 register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to write
 * \param [in] new_val is the value to write
 *
 */
#define HW_TIMER2_REG_SETF(reg, field, new_val) \
                GP_TIMERS->TRIPLE_PWM_##reg##_REG = ((GP_TIMERS->TRIPLE_PWM_##reg##_REG & ~(GP_TIMERS_TRIPLE_PWM_##reg##_REG_##field##_Msk)) | \
                ((GP_TIMERS_TRIPLE_PWM_##reg##_REG_##field##_Msk) & ((new_val) << (GP_TIMERS_TRIPLE_PWM_##reg##_REG_##field##_Pos))))

/*
 * \brief Max value that timer 2 counts down from
 *
 * Timer2 has 14 bit frequency register.
 *
 */
#define HW_TIMER2_MAX_VALUE ((1 << 14) - 1)

/**
 * \brief PWM selector for some functions like set_duty_cycle
 *
 */
typedef enum {
        HW_TIMER2_PWM_2 = 0,
        HW_TIMER2_PWM_3,
        HW_TIMER2_PWM_4
} HW_TIMER2_PWM;

/**
 * \brief Division factor for timer 2
 *
 */
typedef enum {
        HW_TIMER2_DIV_1 = 0,
        HW_TIMER2_DIV_2,
        HW_TIMER2_DIV_4,
        HW_TIMER2_DIV_8
} HW_TIMER2_DIV;


#if dg_configBLACK_ORCA_IC_REV != BLACK_ORCA_IC_REV_A
/**
 * \brief Clock source for timer 2 (either system or LP clock)
 *
 */
typedef enum {
        HW_TIMER2_CLK_SRC_LPCLK = 0,
        HW_TIMER2_CLK_SRC_SYSCLK
} HW_TIMER2_CLK_SRC;
#endif

/**
 * Timer configuration
 *
 */
typedef struct {
        uint16_t        frequency;      /**< timer clock frequency */

        uint16_t        pwm2_start;     /**< PWM2 duty cycle start */
        uint16_t        pwm2_end;       /**< PWM2 duty cycle end */
        uint16_t        pwm3_start;     /**< PWM3 duty cycle start */
        uint16_t        pwm3_end;       /**< PWM3 duty cycle end */
        uint16_t        pwm4_start;     /**< PWM4 duty cycle start */
        uint16_t        pwm4_end;       /**< PWM4 duty cycle end */
} timer2_config;

/**
 * \brief Initialize the driver module
 *
 * Turn on clock for timer and configure timer. After initialization timer is disabled. \p cfg can
 * be NULL - no configuration is performed in such case.
 *
 * \param [in] cfg configuration
 *
 */
void hw_timer2_init(timer2_config *cfg);

/**
 * \brief Configure timer
 *
 * Shortcut to call appropriate set functions. If \p cfg is NULL, this function does nothing.
 *
 * \param [in] cfg timer configuration
 *
 */
void hw_timer2_configure(timer2_config *cfg);

/**
 * \brief Select timer 2 division factor
 *
 * This function sets division factor for timer 2.
 *
 * \note This is only valid if the selected clk source is the sys_clk.
 *
 * \param [in] div_factor division factor for timer 2
 *
 */
static inline void hw_timer2_set_division_factor(HW_TIMER2_DIV div_factor)
{
        GLOBAL_INT_DISABLE();
        REG_SETF(CRG_TOP, CLK_TMR_REG, TMR2_DIV, div_factor);
        GLOBAL_INT_RESTORE();
}

#if dg_configBLACK_ORCA_IC_REV != BLACK_ORCA_IC_REV_A
/**
 * \brief Select timer 2 clock
 *
 * This function sets the clock source to be used by timer 2.
 *
 * \param [in] clk The clock. Either sys_clk or the LP clk
 *
 */
static inline void hw_timer2_set_clk(HW_TIMER2_CLK_SRC clk)
{
        HW_TIMER2_REG_SETF(CTRL, TRIPLE_PWM_CLK_SEL, clk);
}
#endif

/**
 * \brief Get division factor of timer 2
 *
 * This function gets division factor for timer 2.
 *
 * \return division factor for timer 2
 *
 */
static inline HW_TIMER2_DIV hw_timer2_get_division_factor(void)
{
        return REG_GETF(CRG_TOP, CLK_TMR_REG, TMR2_DIV);
}

/**
 * \brief Enable the timer
 *
 * This starts the timer 2, frequency and required duty cycles
 * should be configured before.
 *
 */
static inline void hw_timer2_enable(void)
{
        HW_TIMER2_REG_SETF(CTRL, TRIPLE_PWM_ENABLE, 1);
}

/**
 * \brief Disable the timer
 *
 * Stops timer 2.
 *
 */
static inline void hw_timer2_disable(void)
{
        HW_TIMER2_REG_SETF(CTRL, TRIPLE_PWM_ENABLE, 0);
}

/**
 * \brief Set max value for timer 2
 *
 * \param [in] frequency initial value for timer 2. This value is used to divide input clock to
 *                       generate output frequency in range (16/2..16/(2^14 - 1)) MHz,
 *                       1 - means half the main clock frequency
 *
 */
static inline void hw_timer2_set_frequency(uint16_t frequency)
{
        /* Make sure the requested frequency is within range */
        ASSERT_ERROR(frequency <= HW_TIMER2_MAX_VALUE);
        GP_TIMERS->TRIPLE_PWM_FREQUENCY = frequency;
}

/**
 * \brief Get value for timer 2
 *
 * This value is reset value for timer 2, there is not current value
 * available.
 *
 * \return initial value for timer 2
 *
 */
static inline uint16_t hw_timer2_get_frequency(void)
{
        return REG_GETF(GP_TIMERS, TRIPLE_PWM_FREQUENCY, FREQ);
}

/**
 * \brief Set duty cycle for PWM2, PWM3 or PWM4
 *
 * \param [in] pwm selects one of 3 PWMs
 * \param [in] duty_cycle value to set pwm to, it must be less or equal to value passed
 *                        to hw_timer2_set_frequency to work correctly
 *
 */
void hw_timer2_set_pwm_duty_cycle(HW_TIMER2_PWM pwm, uint16_t duty_cycle);

/**
 * \brief Get duty cycle currently set PWM2, PWM3 or PWM4
 *
 * \param [in] pwm selects one of 3 PWMs
 *
 * \return value currently set to specified pwm
 *
 */
uint16_t hw_timer2_get_pwm_duty_cycle(HW_TIMER2_PWM pwm);

/**
 * \brief Set duty cycle for PWM2, PWM3 or PWM4
 *
 * If \p start value is grater than value passed to hw_timer2_set_frequency PWM will not be
 * generated. If \p stop value is grater then \p start, PWM will not be generated.
 * hw_timer2_set_pwm_start_end(..., 0, stop) is equivalent to hw_timer2_set_duty_cycle(..., stop)
 *
 * \param [in] pwm selects one of 3 PWMs
 * \param [in] start when timer2 reaches this value, output will be set to high
 * \param [in] stop when timer2 reaches this value, output will be set to low
 *
 * \sa hw_timer2_set_pwm_duty_cycle
 * \sa hw_timer2_set_frequency
 *
 */
void hw_timer2_set_pwm_start_end(HW_TIMER2_PWM pwm, uint16_t start, uint16_t stop);

/**
 * \brief Get duty cycle for PWM2, PWM3 or PWM4
 *
 * \param [in] pwm selects one of 3 PWMs
 * \param [out] start start value for PWM
 * \param [out] stop end value for PWM
 *
 * \sa hw_timer2_set_pwm_start_end
 *
 */
void hw_timer2_get_pwm_start_end(HW_TIMER2_PWM pwm, uint16_t *start, uint16_t *stop);

/**
 * \brief Enable/disable hardware pause for PWM generation
 *
 * Calling this will enable or disable hardware pause during PWM generations.
 *
 * When hardware pause is enabled PWM counter is stopped during RF activity and all PWM outputs are
 * set to 0 for RF duration. This can suppress the PWM switching noise.
 *
 * When hardware pause is disabled timer2 will not be stopped during RF transmission, PWM will
 * operate normally. This can introduce PWM switching noise.
 *
 */
static inline void hw_timer2_set_hw_pause(bool enable)
{
        HW_TIMER2_REG_SETF(CTRL, HW_PAUSE_EN, enable);
}

/**
 * \brief Get status of timer hardware pause
 *
 * \return true if timer is paused, false otherwise
 */
static inline bool hw_timer2_get_hw_pause(void)
{
        return HW_TIMER2_REG_GETF(CTRL, HW_PAUSE_EN);
}

/**
 * \brief Enable/disable software pause for PWM generation
 *
 * When software pause is enabled then output PWM pins are set to 0.
 * This allows to reduce PWM switching noise.
 *
 * When software pause is disabled then timer 2 is unblocked from software pause.
 * PWM can still be paused if hardware pause is enabled.
 *
 */
static inline void hw_timer2_set_sw_pause(bool enable)
{
        HW_TIMER2_REG_SETF(CTRL, SW_PAUSE_EN, enable);
}

/**
 * \brief Get status of timer software pause
 *
 * \return true if timer is paused, false otherwise
 *
 */
static inline bool hw_timer2_get_sw_pause(void)
{
        return HW_TIMER2_REG_GETF(CTRL, SW_PAUSE_EN);
}

/**
 * \brief Freeze timer
 *
 */
static inline void hw_timer2_freeze(void)
{
        GPREG->SET_FREEZE_REG = GPREG_SET_FREEZE_REG_FRZ_SWTIM2_Msk;
}

/**
 * \brief Unfreeze timer
 *
 */
static inline void hw_timer2_unfreeze(void)
{
        GPREG->RESET_FREEZE_REG = GPREG_SET_FREEZE_REG_FRZ_SWTIM2_Msk;
}

#ifdef __cplusplus
}
#endif

#endif /* dg_configUSE_HW_TIMER2 */

#endif /* HW_TIMER2_H_ */

/**
 * \}
 * \}
 * \}
 */
