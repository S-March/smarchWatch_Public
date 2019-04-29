/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup Temperature_Sensor
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file hw_tempsens.c
 *
 * @brief Definition of API for the Hardware Temperature Sensor interface abstraction layer.
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configUSE_HW_TEMPSENS


#include <stdio.h>
#include <string.h>
#include <hw_gpadc.h>
#include <hw_tempsens.h>
#include "hw_usb_charger.h"

static uint32_t charger_test_prev;
static bool charger_on;

void hw_tempsens_enable(void)
{
        charger_test_prev = REG_GETF(ANAMISC, CHARGER_CTRL2_REG, CHARGER_TEST);
        charger_on = hw_charger_is_charging();
        if (charger_on) {
                hw_charger_stop_charging();
        }

        REG_SETF(ANAMISC, CHARGER_CTRL2_REG, CHARGER_TEST, 1);
}

void hw_tempsens_disable(void)
{
        REG_SETF(ANAMISC, CHARGER_CTRL2_REG, CHARGER_TEST, charger_test_prev);
        if (charger_on) {
                hw_charger_start_charging();
        }
}

void hw_tempsens_prepare(void)
{
        hw_gpadc_init(NULL);
        hw_gpadc_set_input(HW_GPADC_INPUT_SE_TEMPSENS);
}

int hw_tempsens_convert_to_temperature(uint16_t val)
{
        /*
         * To get the result in C degrees below equation must be used to convert digital value to
         * temperature.
         * adcvaltemp = 712 + 2.44 * t  ->  t = (adcvaltemp - 712) * 100 / 244
         */
        return ((int) val - 712) * 100 / 244;
}

uint16_t hw_tempsens_raw_read(void)
{
        hw_gpadc_set_input(HW_GPADC_INPUT_SE_TEMPSENS);
        /* Wait a while after connecting the input. */
        for (volatile int i = 10; i > 0; i--);
        hw_gpadc_start();
        while (hw_gpadc_in_progress());
        return hw_gpadc_get_value();
}

int hw_tempsens_read(void)
{
        uint16_t val;

        val = hw_tempsens_raw_read();

        return hw_tempsens_convert_to_temperature(val);
}

#endif /* dg_configUSE_HW_TEMPSENS */
/**
 * \}
 * \}
 * \}
 */
