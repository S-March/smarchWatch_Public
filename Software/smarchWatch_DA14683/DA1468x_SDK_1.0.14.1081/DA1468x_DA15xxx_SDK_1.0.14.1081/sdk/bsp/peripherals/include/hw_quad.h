/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup QUAD_Decoder
 * \{
 * \brief Quadrature Decoder
 */

/**
 ****************************************************************************************
 *
 * @file hw_quad.h
 *
 * @brief Definition of API for the QUAD Decoder Low Level Driver.
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_QUAD_H_
#define HW_QUAD_H_

#if dg_configUSE_HW_QUAD

#include <stdbool.h>
#include <stdint.h>
#include <sdk_defs.h>

/**
 * \brief Channels definitions
 *
 * Separate channels can be used as a bitmask to combine into channels sets.
 * For convenience, symbols for possible channels sets combinations are also provided.
 *
 */
typedef enum {
        HW_QUAD_CHANNEL_NONE = 0,
        HW_QUAD_CHANNEL_X = (1 << 0),
        HW_QUAD_CHANNEL_Y = (1 << 1),
        HW_QUAD_CHANNEL_Z = (1 << 2),
        HW_QUAD_CHANNEL_XY = HW_QUAD_CHANNEL_X | HW_QUAD_CHANNEL_Y,
        HW_QUAD_CHANNEL_XZ = HW_QUAD_CHANNEL_X | HW_QUAD_CHANNEL_Z,
        HW_QUAD_CHANNEL_YZ = HW_QUAD_CHANNEL_Y | HW_QUAD_CHANNEL_Z,
        HW_QUAD_CHANNEL_XYZ = HW_QUAD_CHANNEL_X | HW_QUAD_CHANNEL_Y | HW_QUAD_CHANNEL_Z,
        HW_QUAD_CHANNEL_ALL = HW_QUAD_CHANNEL_XYZ
} HW_QUAD_CHANNEL;

/**
 * \brief QUAD interrupt callback
 *
 */
typedef void (*hw_quad_handler_cb)(void);

/**
 * \brief Initialization of QUAD driver
 *
 * Set clock divider and enable NVIC interrupt.
 *
 * \param [in] clk_div number of the input clock cycles minus one, that
 *                     are required to generate one logic clock cycle
 *
 */
static inline void hw_quad_init(uint16_t clk_div)
{
        QUAD->QDEC_CLOCKDIV_REG = clk_div;
}

/**
 * \brief Enable QUAD driver
 *
 */
static inline void hw_quad_enable(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_PER, CLK_PER_REG, QUAD_ENABLE);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Disable QUAD driver
 *
 */
static inline void hw_quad_disable(void)
{
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_PER, CLK_PER_REG, QUAD_ENABLE);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Set channels state
 *
 * This sets all channels state according to \p ch_mask.
 *
 * \param [in] ch_mask channels state
 *
 * \sa hw_quad_enable_channels
 * \sa hw_quad_disable_channels
 *
 */
static inline void hw_quad_set_channels(HW_QUAD_CHANNEL ch_mask)
{
        static const uint16_t mask =
                QUAD_QDEC_CTRL_REG_CHX_PORT_EN_Msk |
                QUAD_QDEC_CTRL_REG_CHY_PORT_EN_Msk |
                QUAD_QDEC_CTRL_REG_CHZ_PORT_EN_Msk;
        uint16_t val;

        val = QUAD->QDEC_CTRL_REG;
        val &= ~mask;
        val |= (ch_mask << QUAD_QDEC_CTRL_REG_CHX_PORT_EN_Pos) & mask;
        QUAD->QDEC_CTRL_REG = val;
}

/**
 * \brief Enable channels
 *
 * This only enables channels specified by \p ch_mask. Other channels state is unchanged.
 *
 * \param [in] ch_mask channels to enable
 *
 * \sa hw_quad_set_channels
 * \sa hw_quad_disable_channels
 *
 */
static inline void hw_quad_enable_channels(uint8_t ch_mask)
{
        static const uint16_t mask =
                QUAD_QDEC_CTRL_REG_CHX_PORT_EN_Msk |
                QUAD_QDEC_CTRL_REG_CHY_PORT_EN_Msk |
                QUAD_QDEC_CTRL_REG_CHZ_PORT_EN_Msk;
        uint16_t val;

        val = QUAD->QDEC_CTRL_REG;
        val |= (ch_mask << QUAD_QDEC_CTRL_REG_CHX_PORT_EN_Pos) & mask;
        QUAD->QDEC_CTRL_REG = val;
}

/**
 * \brief Disable channels
 *
 * This only disables channels specified by \p ch_mask. Other channels state is unchanged.
 *
 * \param [in] ch_mask channels to disable
 *
 * \sa hw_quad_set_channels
 * \sa hw_quad_enable_channels
 *
 */
static inline void hw_quad_disable_channels(uint8_t ch_mask)
{
        static const uint16_t mask =
                QUAD_QDEC_CTRL_REG_CHX_PORT_EN_Msk |
                QUAD_QDEC_CTRL_REG_CHY_PORT_EN_Msk |
                QUAD_QDEC_CTRL_REG_CHZ_PORT_EN_Msk;
        uint16_t val;

        val = QUAD->QDEC_CTRL_REG;
        val &= ~((ch_mask << QUAD_QDEC_CTRL_REG_CHX_PORT_EN_Pos) & mask);
        QUAD->QDEC_CTRL_REG = val;
}


/**
 * \brief Get channels state
 *
 * \return channels state
 *
 * \sa hw_quad_set_channels
 *
 */
static inline HW_QUAD_CHANNEL hw_quad_get_channels(void)
{
        static const uint16_t mask =
                QUAD_QDEC_CTRL_REG_CHX_PORT_EN_Msk |
                QUAD_QDEC_CTRL_REG_CHY_PORT_EN_Msk |
                QUAD_QDEC_CTRL_REG_CHZ_PORT_EN_Msk;

        return (HW_QUAD_CHANNEL)
                ((QUAD->QDEC_CTRL_REG & mask) >> QUAD_QDEC_CTRL_REG_CHX_PORT_EN_Pos);
}

/**
 * \brief Turn on QUAD interrupt
 *
 * \param [in] handler function pointer to handler to call when an interrupt occurs
 * \param [in] tnum threshold value for interrupt
 *
 */
void hw_quad_register_interrupt(hw_quad_handler_cb handler, uint16_t tnum);

/**
 * \brief Check if interrupt has occurred
 *
 * \return Interrupt status
 *
 */
static inline bool hw_quad_is_irq_gen(void)
{
        return REG_GETF(QUAD, QDEC_CTRL_REG, QD_IRQ_STATUS);
}

/**
 * \brief Turn off QUAD interrupt
 *
 */
void hw_quad_unregister_interrupt(void);

/**
 * \brief Get the number of steps from X channel
 *
 * \return number of steps counted in X channel
 *
 */
static inline int16_t hw_quad_get_x(void)
{
        return QUAD->QDEC_XCNT_REG;
}

/**
 * \brief Get the number of steps from Y channel
 *
 * \return number of steps counted in Y channel
 *
 */
static inline int16_t hw_quad_get_y(void)
{
        return QUAD->QDEC_YCNT_REG;
}

/**
 * \brief Get the number of steps from Z channel
 *
 * \return number of steps counted in Z channel
 *
 */
static inline int16_t hw_quad_get_z(void)
{
        return QUAD->QDEC_ZCNT_REG;
}

#endif /* dg_configUSE_HW_QUAD */

#endif /* HW_QUAD_H_ */

/**
 * \}
 * \}
 * \}
 */
