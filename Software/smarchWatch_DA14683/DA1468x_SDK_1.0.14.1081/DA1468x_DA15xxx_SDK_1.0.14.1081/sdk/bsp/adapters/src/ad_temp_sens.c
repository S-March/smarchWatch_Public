/**
 * \addtogroup BSP
 * \{
 * \addtogroup ADAPTERS
 * \{
 * \addtogroup TEMPSENS
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_temp_sens.c
 *
 * @brief Temperature sensor adapter implementation
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configTEMPSENS_ADAPTER

#include <stdint.h>
#include "sdk_defs.h"
#include "interrupts.h"
#include <sys_power_mgr.h>
#include <stdarg.h>
#include <platform_devices.h>
#include <ad_gpadc.h>
#include <ad_temp_sens.h>

tempsens_source ad_tempsens_open(void)
{
        return (tempsens_source) ad_gpadc_open(TEMP_SENSOR);
}

void ad_tempsens_close(tempsens_source src)
{
        ad_gpadc_close(src);
}

int ad_tempsens_read(tempsens_source src)
{
        int temp;
        bool acquired __attribute__((unused));

        acquired = ad_tempsens_read_to(src, &temp, RES_WAIT_FOREVER);
        OS_ASSERT(acquired);
        return temp;
}

bool ad_tempsens_read_to(tempsens_source src, int *value, uint32_t timeout)
{
        bool ret = false;
        uint16_t temp_val;

        if (ad_gpadc_read_to(src, &temp_val, timeout)) {
                *value = hw_tempsens_convert_to_temperature(temp_val);
                ret = true;
        }

        return ret;
}

void ad_tempsens_read_async(tempsens_source src, ad_tempsens_user_cb cb, void *user_data)
{
        ad_gpadc_read_async(src, cb, user_data);
}

#endif // dg_configTEMPSENS_ADAPTER
/**
 * \}
 * \}
 * \}
 */
