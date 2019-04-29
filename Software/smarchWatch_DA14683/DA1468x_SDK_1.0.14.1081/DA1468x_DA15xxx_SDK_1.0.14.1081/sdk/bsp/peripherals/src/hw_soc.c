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
 * @file hw_soc.c
 *
 * @brief Implementation of the SOC Low Level Driver.
 *
 * Copyright (C) 2015-2018 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configUSE_HW_SOC


#include <stdio.h>
#include <string.h>
#include <hw_soc.h>

#define CO_BIT(pos) (1UL<<(pos))

const struct soc_cfg_struct soc_cfg_recommended = {
        .soc_ctrl1_reg = (0 << ANAMISC_SOC_CTRL1_REG_SOC_ENABLE_Pos) |
                         (0 << ANAMISC_SOC_CTRL1_REG_SOC_RESET_CHARGE_Pos) |
                         (0 << ANAMISC_SOC_CTRL1_REG_SOC_RESET_AVG_Pos) |
                         (0 << ANAMISC_SOC_CTRL1_REG_SOC_MUTE_Pos) |
                         (0 << ANAMISC_SOC_CTRL1_REG_SOC_GPIO_Pos) |
                         (0 << ANAMISC_SOC_CTRL1_REG_SOC_SIGN_Pos) |
                         (2 << ANAMISC_SOC_CTRL1_REG_SOC_IDAC_Pos) |
                         (0 << ANAMISC_SOC_CTRL1_REG_SOC_LPF_Pos) |
                         (4 << ANAMISC_SOC_CTRL1_REG_SOC_CLK_Pos) |
                         (1 << ANAMISC_SOC_CTRL1_REG_SOC_BIAS_Pos) |
                         (3 << ANAMISC_SOC_CTRL1_REG_SOC_CINT_Pos),

        .soc_ctrl2_reg = (2 << ANAMISC_SOC_CTRL2_REG_SOC_RVI_Pos) |
                         (2 << ANAMISC_SOC_CTRL2_REG_SOC_SCYCLE_Pos) |
                         (1 << ANAMISC_SOC_CTRL2_REG_SOC_DCYCLE_Pos) |
                         (1 << ANAMISC_SOC_CTRL2_REG_SOC_ICM_Pos) |
                         (7 << ANAMISC_SOC_CTRL2_REG_SOC_CHOP_Pos) |
                         (0 << ANAMISC_SOC_CTRL2_REG_SOC_CMIREG_ENABLE_Pos) |
                         (7 << ANAMISC_SOC_CTRL2_REG_SOC_MAW_Pos) |
                         (0 << ANAMISC_SOC_CTRL2_REG_SOC_DYNAVG_Pos),

// Last review date: July 03, 2018
#if dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A
        .soc_ctrl3_reg = (3 << ANAMISC_SOC_CTRL3_REG_SOC_VSAT_Pos) |
#else
        .soc_ctrl3_reg = (1 << ANAMISC_SOC_CTRL3_REG_SOC_VSAT_Pos) |
#endif
                         (0 << ANAMISC_SOC_CTRL3_REG_SOC_DYNTARG_Pos) |
                         (0 << ANAMISC_SOC_CTRL3_REG_SOC_DYNHYS_Pos) |
                         (1 << ANAMISC_SOC_CTRL3_REG_SOC_VCMI_Pos)
};

static inline uint32_t co_clz(uint32_t val)
{
#if defined(__GNUC__)
        return __builtin_clz(val);
#else
        #pragma message "Missing co_clz() implementation!"
#endif // defined(__GNUC__)
}

void hw_soc_init(const struct soc_cfg_struct *cfg)
{
        ANAMISC->SOC_CTRL1_REG = cfg->soc_ctrl1_reg;
        ANAMISC->SOC_CTRL2_REG = cfg->soc_ctrl2_reg;
        ANAMISC->SOC_CTRL3_REG = cfg->soc_ctrl3_reg;
}

uint64_t hw_soc_read_charge(void)
{
        uint64_t res;
        uint16_t charge2, charge3;
        uint16_t charge1_next, charge2_next, charge3_next;

        do {
                charge2 = ANAMISC->SOC_CHARGE_CNTR2_REG;
                charge3 = ANAMISC->SOC_CHARGE_CNTR3_REG;

                charge1_next = ANAMISC->SOC_CHARGE_CNTR1_REG;
                charge2_next = ANAMISC->SOC_CHARGE_CNTR2_REG;
                charge3_next = ANAMISC->SOC_CHARGE_CNTR3_REG;
        } while ((charge3 != charge3_next) || (charge2 != charge2_next));

        res = ((uint64_t)charge3_next << 32) |
                ((uint64_t)charge2_next << 16) |
                ((uint64_t)charge1_next);

        return res;
}

int32_t hw_soc_read_average(const struct soc_cfg_struct *cfg)
{
        int32_t res;
        uint32_t value, mul, tmp;
        uint16_t avg;
        int sign = 1;
        int div = 1000000;

        avg = ANAMISC->SOC_CHARGE_AVG_REG;

        // Check if it is negative.
        if ((avg & (1 << 15)) != 0) {
                avg = (avg & 0x7fff) ^ 0x7fff;
                sign = -1;
        }

        // Do the maths.
        value = 11944 * avg;

        mul = (1 << ((cfg->soc_ctrl2_reg & ANAMISC_SOC_CTRL2_REG_SOC_RVI_Msk) >> ANAMISC_SOC_CTRL2_REG_SOC_RVI_Pos))
                * (8 >> ((cfg->soc_ctrl1_reg & ANAMISC_SOC_CTRL1_REG_SOC_BIAS_Msk) >> ANAMISC_SOC_CTRL1_REG_SOC_BIAS_Pos))
                * (8 >> ((cfg->soc_ctrl1_reg & ANAMISC_SOC_CTRL1_REG_SOC_BIAS_Msk) >> ANAMISC_SOC_CTRL1_REG_SOC_BIAS_Pos))
                * (1 << ((cfg->soc_ctrl1_reg & ANAMISC_SOC_CTRL1_REG_SOC_IDAC_Msk) >> ANAMISC_SOC_CTRL1_REG_SOC_IDAC_Pos));
        mul = 31 - co_clz(mul);                 // Get trailing zeroes (power of 2 index)

        tmp = co_clz(value);

        if ((tmp != 0) && (tmp >= mul)) {
                value = value << mul;           // multiply
                value /= 1000000;
        } else {
                value = value << tmp;           // multiply
                mul = mul - tmp;                // divide
                if (mul <= 6) {
                        div = div >> mul;
                        mul = 0;
                } else {
                        div = div >> 6;
                        mul -= 6;
                }
                value /= div;
                value = value << mul;
        }

        res = value * sign;

        return res;
}

#endif /* dg_configUSE_HW_SOC */
/**
 * \}
 * \}
 * \}
 */
