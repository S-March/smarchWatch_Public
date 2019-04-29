/**
 \addtogroup INTERFACES
 \{
 \addtogroup FTDF
 \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_ftdf_phy_api.h
 *
 * @brief FTDF PHY Adapter API
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef AD_FTDF_PHY_API_H
#define AD_FTDF_PHY_API_H

#include "ftdf.h"
#include "ad_ftdf_config.h"

/**
 * \brief Initialize adapter - create queues
 *
 */
void ad_ftdf_init(void);

/**
 * \brief Set Extended Address
 *
 * Sets interface extended address. This is thread safe.
 *
 * \param[in] address - The extended address to set
 */
void ad_ftdf_set_ext_address(ftdf_ext_address_t address);

/**
 * \brief Get Extended Address
 *
 * Gets interface extended address. This is thread safe.
 *
 * \returns The extended address of the interface
 */
ftdf_ext_address_t ad_ftdf_get_ext_address(void);

/**
 * \brief Transmits a frame
 *
 * Transmits a frame. Params:
 *
 * \param[in] frame_length - The total length of the frame passed in bytes
 * \param[in] frame - a pointer to the passed frame buffer
 * \param[in] channel - The channel to use for transmission in [11, 26]
 * \param[in] csma_suppress - If true, csma protocol (i.e. CCA) will not be performed
 * \param[in] pti - Packet Traffic Information that will be used for this transaction.
 */
ftdf_status_t ad_ftdf_send_frame_simple(ftdf_data_length_t    frame_length,
                                ftdf_octet_t*        frame,
                                ftdf_channel_number_t channel,
                                ftdf_pti_t           pti,
                                ftdf_boolean_t       csma_suppress);

/**
 * \brief Instructs the MAC and PHY to go to sleep
 *
 * \param[in] allow_deferred_sleep - If true, then if the MAC cannot go to sleep immediately
 *                                   (e.g. a transmission is pending), the MAC will go to sleep
 *                                   as soon as possible. If false, and the MAC cannot go to sleep
 *                                   immediately, sleep will be aborted.
 */
void ad_ftdf_sleep_when_possible(ftdf_boolean_t allow_deferred_sleep);

/**
 * \brief Instructs the MAC and PHY to wakeup, if sleeping
 *
 */
void ad_ftdf_wake_up(void);

#if FTDF_DBG_BUS_ENABLE
/**
 * \brief Configures GPIO pins for the FTDF debug bus
 *
 * if FTDF_DBG_BUS_USE_PORT_4 == 0, the FTDF debug bus uses the following (fixed) GPIO pins,
 *
 * bit 0: HW_GPIO_PORT_1, HW_GPIO_PIN_4
 * bit 1: HW_GPIO_PORT_1, HW_GPIO_PIN_5
 * bit 2: HW_GPIO_PORT_1, HW_GPIO_PIN_6
 * bit 3: HW_GPIO_PORT_1, HW_GPIO_PIN_7
 * bit 4: HW_GPIO_PORT_0, HW_GPIO_PIN_6
 * bit 5: HW_GPIO_PORT_0, HW_GPIO_PIN_7
 * bit 6: HW_GPIO_PORT_1, HW_GPIO_PIN_3
 * bit 7: HW_GPIO_PORT_2, HW_GPIO_PIN_3
 *
 * if FTDF_DBG_BUS_USE_PORT_4 == 1, the FTDF debug bus uses the following pins:
 *
 * bit 0: HW_GPIO_PORT_4, HW_GPIO_PIN_0
 * bit 1: HW_GPIO_PORT_4, HW_GPIO_PIN_1
 * bit 2: HW_GPIO_PORT_4, HW_GPIO_PIN_2
 * bit 3: HW_GPIO_PORT_4, HW_GPIO_PIN_3
 * bit 4: HW_GPIO_PORT_4, HW_GPIO_PIN_4
 * bit 5: HW_GPIO_PORT_4, HW_GPIO_PIN_5
 * bit 6: HW_GPIO_PORT_4, HW_GPIO_PIN_6
 * bit 7: HW_GPIO_PORT_4, HW_GPIO_PIN_7
 *
 */
void ad_ftdf_dbg_bus_gpio_config(void);
#endif /* FTDF_DBG_BUS_ENABLE */

#endif /* AD_FTDF_PHY_API_H */

/**
 \}
 \}
 \}
 */

