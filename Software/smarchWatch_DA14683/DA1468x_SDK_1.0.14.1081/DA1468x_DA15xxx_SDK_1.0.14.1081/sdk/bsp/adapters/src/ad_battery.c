/**
 * \addtogroup BSP
 * \{
 * \addtogroup ADAPTERS
 * \{
 * \addtogroup BATTERY
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_battery.c
 *
 * @brief Temperature sensor adapter implementation
 *
 * Copyright (C) 2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#if dg_configBATTERY_ADAPTER

#include <stdint.h>
#include "sdk_defs.h"
#include "interrupts.h"
#include <sys_power_mgr.h>
#include <stdarg.h>
#include <platform_devices.h>
#include <ad_gpadc.h>
#include <ad_battery.h>

battery_source ad_battery_open(void)
{
        return (battery_source) ad_gpadc_open(BATTERY_LEVEL);
}

void ad_battery_close(battery_source src)
{
        ad_gpadc_close(src);
}

uint16_t ad_battery_read(battery_source src)
{
        uint16_t val;

        ad_gpadc_read(src, &val);

        return val;
}

void ad_battery_read_async(battery_source src, ad_battery_user_cb cb, void *user_data)
{
        ad_gpadc_read_async(src, cb, user_data);
}

#endif /* dg_configBATTERY_ADAPTER */

/**
 * \}
 * \}
 * \}
 */
