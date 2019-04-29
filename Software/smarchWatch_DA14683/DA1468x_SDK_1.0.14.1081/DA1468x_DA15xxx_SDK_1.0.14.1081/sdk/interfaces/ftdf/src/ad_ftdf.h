/**
 \addtogroup INTERFACES
 \{
 \addtogroup FTDF
 \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_ftdf.h
 *
 * @brief FTDF Adapter internal header file
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef AD_FTDF_H
#define AD_FTDF_H

#ifndef FTDF_PHY_API
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#endif

#include "ftdf.h"
#include "ad_ftdf_config.h"

typedef enum {
        BLOCK_ACTIVE = 0, BLOCK_SLEEPING, BLOCK_WAKING_UP
} sleep_status_t;

extern sleep_status_t sleep_status;
extern ftdf_ext_address_t u_ext_address;
extern ftdf_boolean_t explicit_sleep;

void ad_ftdf_init_mac_api(void);
void ad_ftdf_init_phy_api(void);

void ad_ftdf_wake_up_async(void);
void ad_ftdf_wake_up_sync(void);

void sleep_when_possible( uint8_t explicit_request, uint32_t sleep_time );

#if FTDF_DBG_BUS_ENABLE
/**
 * \brief Configures GPIO pins for the FTDF debug bus
 *
 * Debug bus uses the following (fixed) GPIO pins:
 *
 * bit 0: HW_GPIO_PORT_1, HW_GPIO_PIN_4
 * bit 1: HW_GPIO_PORT_1, HW_GPIO_PIN_5
 * bit 2: HW_GPIO_PORT_1, HW_GPIO_PIN_6
 * bit 3: HW_GPIO_PORT_1, HW_GPIO_PIN_7
 * bit 4: HW_GPIO_PORT_0, HW_GPIO_PIN_6
 * bit 5: HW_GPIO_PORT_0, HW_GPIO_PIN_7
 * bit 6: HW_GPIO_PORT_1, HW_GPIO_PIN_3
 * bit 7: HW_GPIO_PORT_2, HW_GPIO_PIN_3
 */
void ad_ftdf_dbg_bus_gpio_config(void);
#endif /* FTDF_DBG_BUS_ENABLE */

#endif /* AD_FTDF_H */
/**
 \}
 \}
 \}
 */
