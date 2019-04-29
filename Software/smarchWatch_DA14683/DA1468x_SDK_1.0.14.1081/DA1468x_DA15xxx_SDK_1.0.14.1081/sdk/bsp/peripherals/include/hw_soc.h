/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup State_Of_Charge
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file hw_soc.h
 *
 * @brief Definition of API for the SOC Low Level Driver.
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_SOC_H
#define HW_SOC_H

#if dg_configUSE_HW_SOC

#include <stdbool.h>
#include <stdint.h>
#include <sdk_defs.h>

struct soc_cfg_struct {
        uint16_t soc_ctrl1_reg;
        uint16_t soc_ctrl2_reg;
        uint16_t soc_ctrl3_reg;
};

extern const struct soc_cfg_struct soc_cfg_recommended;


/**
 * \brief Initialize the SOC block. Use hw_soc_enable() to activate it.
 *
 * \param cfg The preferred settings for the 3 control registers of the SOC.
 *
 */
void hw_soc_init(const struct soc_cfg_struct *cfg);

/**
 * \brief Reset the SOC charge counter.
 *
 */
__STATIC_INLINE void hw_soc_reset_charge(void)
{
        REG_SET_BIT(ANAMISC, SOC_CTRL1_REG, SOC_RESET_CHARGE);
}

/**
 * \brief Reset the SOC AVG counter.
 *
 */
__STATIC_INLINE void hw_soc_reset_avg(void)
{
        REG_SET_BIT(ANAMISC, SOC_CTRL1_REG, SOC_RESET_AVG);
}

/**
 * \brief Release the SOC counters from reset.
 *
 */
__STATIC_INLINE void hw_soc_release_counters(void)
{
        uint32_t soc_ctrl1_reg = ANAMISC->SOC_CTRL1_REG;
        REG_SET_FIELD(ANAMISC, SOC_CTRL1_REG, SOC_RESET_CHARGE, soc_ctrl1_reg, 0);
        REG_SET_FIELD(ANAMISC, SOC_CTRL1_REG, SOC_RESET_AVG, soc_ctrl1_reg, 0);
        ANAMISC->SOC_CTRL1_REG = soc_ctrl1_reg;
}

/**
 * \brief Activate the SOC block.
 *
 */
__STATIC_INLINE void hw_soc_enable(void)
{
        hw_soc_reset_charge();
        hw_soc_reset_avg();
        REG_SET_BIT(ANAMISC, SOC_CTRL1_REG, SOC_ENABLE);
        hw_soc_release_counters();
}

/**
 * \brief Disable the SOC block.
 *
 */
__STATIC_INLINE void hw_soc_disable(void)
{
        REG_CLR_BIT(ANAMISC, SOC_CTRL1_REG, SOC_ENABLE);
}

/**
 * \brief Read the SOC charge counter.
 *
 * \return The charge counter value (48-bit).
 *
 */
uint64_t hw_soc_read_charge(void);

/**
 * \brief Read the SOC average counter.
 *
 * \param cfg The preferred settings of the 3 control registers of the SOC.
 *
 * \return The average counter value in 10ths of uA (i.e. 1023 means 10.23mA). The result is
 *        negative during discharging and positive when charging. The accuracy is good for currents
 *        less than 500mA.
 *
 */
int32_t hw_soc_read_average(const struct soc_cfg_struct *cfg);

#endif /* dg_configUSE_HW_SOC */

#endif /* HW_SOC_H */

/**
 * \}
 * \}
 * \}
 */
