/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup Breath_Timer
 * \{
 * \brief Breath Timer
 */

/**
 ****************************************************************************************
 *
 * @file hw_breath.h
 *
 * @brief Definition of API for the Breath timer Low Level Driver.
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_BREATH_H_
#define HW_BREATH_H_

#include <stdint.h>
#include <sdk_defs.h>

/**
 * \brief Output polarity
 *
 * With negative polarity PWM signal is inverted, i.e. duty cycle will be (100% - duty_cycle).
 *
 */
typedef enum {
        HW_BREATH_PWM_POL_POS = 0,      /**< positive */
        HW_BREATH_PWM_POL_NEG = 1       /**< negative */
} HW_BREATH_PWM_POL;

/**
 * \brief Breath timer configuration
 */
typedef struct {
        uint8_t             dc_min;     /**< duty cycle min value */
        uint8_t             dc_max;     /**< duty cycle max value */
        uint8_t             dc_step;    /**< duty cycle value change step */
        uint8_t             freq_div;   /**< system clock division factor */
        HW_BREATH_PWM_POL   polarity;   /**< output polarity */
} breath_config;

/* forward declaration for hw_breath_init() */
static inline void hw_breath_configure(breath_config *cfg);

/**
 * \brief Init the breath timer
 *
 * Turn on clock for the breath timer and configure timer. \p cfg can be NULL - no configuration is
 * performed in such case.
 *
 * \param [in] cfg configuration
 *
 */
static inline void hw_breath_init(breath_config *cfg)
{
        GLOBAL_INT_DISABLE();
        REG_SETF(CRG_TOP, CLK_TMR_REG, BREATH_ENABLE, 1);
        GLOBAL_INT_RESTORE();

        hw_breath_configure(cfg);
}

/**
 * \brief Set minimum duty cycle value
 *
 * Actual duty cycle set is \p value / (freq_div + 1)
 *
 * \param [in] value duty cycle value
 *
 * \sa hw_breath_configure
 *
 */
static inline void hw_breath_set_dc_min(uint8_t value)
{
        GP_TIMERS->BREATH_DUTY_MIN_REG = value;
}

/**
 * \brief Set maximum duty cycle value
 *
 * \p value should not be larger than system clock division factor set.
 * Actual duty cycle set is \p value / (freq_div + 1)
 *
 * \param [in] value duty cycle value
 *
 * \sa hw_breath_set_freq_div
 * \sa hw_breath_configure
 *
 */
static inline void hw_breath_set_dc_max(uint8_t value)
{
        GP_TIMERS->BREATH_DUTY_MAX_REG = value;

}

/**
 * \brief Set duty cycle change step
 *
 * PWM duty cycle will change from min to max values set in \p step increments/decrements.
 *
 * \param [in] step step value
 *
 * \sa hw_breath_configure
 *
 */
static inline void hw_breath_set_dc_step(uint8_t step)
{
        REG_SETF(GP_TIMERS, BREATH_CFG_REG, BRTH_STEP, step);
}

/**
 * \brief Set system clock division factor
 *
 * This is also maximum value for maximum duty cycle to be set.
 *
 * \param [in] div system clock division factor
 *
 * \sa hw_breath_configure
 *
 */
static inline void hw_breath_set_freq_div(uint8_t div)
{
        REG_SETF(GP_TIMERS, BREATH_CFG_REG, BRTH_DIV, div);
}

/**
 * \brief Set output polarity of PWM signal
 *
 * \param [in] pol output polarity
 *
 */
static inline void hw_breath_set_polarity(HW_BREATH_PWM_POL pol)
{
        REG_SETF(GP_TIMERS, BREATH_CTRL_REG, BRTH_PWM_POL, pol);
}

/**
 * \brief Get minimum duty cycle value
 *
 * Actual duty cycle set is \p value / (freq_div + 1)
 *
 * \return minimum duty cycle value
 *
 */
static inline uint8_t hw_breath_get_dc_min(void)
{
        return GP_TIMERS->BREATH_DUTY_MIN_REG;
}

/**
 * \brief Get maximum duty cycle value
 *
 * Actual duty cycle set is \p value / (freq_div + 1)
 *
 * \return maximum duty cycle value
 *
 */
static inline uint8_t hw_breath_get_dc_max(void)
{
        return GP_TIMERS->BREATH_DUTY_MAX_REG;
}

/**
 * \brief Get duty cycle change step
 *
 * \return duty cycle change step
 *
 */
static inline uint8_t hw_breath_get_dc_step(void)
{
        return REG_GETF(GP_TIMERS, BREATH_CFG_REG, BRTH_STEP);
}

/**
 * \brief Get system clock division factor
 *
 * \return system clock division factor
 *
 */
static inline uint8_t hw_breath_get_freq_div(void)
{
        return REG_GETF(GP_TIMERS, BREATH_CFG_REG, BRTH_DIV);
}

/**
 * \brief Get output polarity of PWM signal
 *
 * \return output polarity
 *
 */
static inline HW_BREATH_PWM_POL hw_breath_get_polarity(void)
{
        return (HW_BREATH_PWM_POL) REG_GETF(GP_TIMERS, BREATH_CTRL_REG, BRTH_PWM_POL);
}

/**
 * \brief Configure breath timer
 *
 * Shortcut to call appropriate set functions. If \p cfg is NULL, this function does nothing.
 *
 * \param [in] cfg timer configuration
 *
 * \sa hw_breath_set_dc_min
 * \sa hw_breath_set_dc_max
 * \sa hw_breath_set_dc_step
 * \sa hw_breath_set_freq_div
 * \sa hw_breath_set_polarity
 *
 */
static inline void hw_breath_configure(breath_config *cfg)
{
        ASSERT_WARNING(cfg->dc_max > cfg->dc_min);

        if (cfg) {
                hw_breath_set_dc_min(cfg->dc_min);
                hw_breath_set_dc_max(cfg->dc_max);
                hw_breath_set_dc_step(cfg->dc_step);
                hw_breath_set_freq_div(cfg->freq_div);
                hw_breath_set_polarity(cfg->polarity);
        }
}

/**
 * \brief Enable the breath timer operation
 *
 */
static inline void hw_breath_enable(void)
{
        REG_SETF(GP_TIMERS, BREATH_CTRL_REG, BRTH_EN, 1);
}

/**
 * \brief Disable the breath timer operation
 *
 */
static inline void hw_breath_disable(void)
{
        REG_SETF(GP_TIMERS, BREATH_CTRL_REG, BRTH_EN, 0);
}

#endif /* HW_BREATH_H_ */

/**
 * \}
 * \}
 * \}
 */
