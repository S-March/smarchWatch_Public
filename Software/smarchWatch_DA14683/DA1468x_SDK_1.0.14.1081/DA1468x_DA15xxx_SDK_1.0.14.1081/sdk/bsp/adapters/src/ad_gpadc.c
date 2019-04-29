/**
 * \addtogroup BSP
 * \{
 * \addtogroup ADAPTERS
 * \{
 * \addtogroup GPADC
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_gpadc.c
 *
 * @brief GPADC adapter implementation
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configGPADC_ADAPTER

#include <stdint.h>
#include "sdk_defs.h"
#include "interrupts.h"
#include <sys_power_mgr.h>
#include <stdarg.h>
#include <resmgmt.h>
#include <hw_gpadc.h>
#include <hw_tempsens.h>
#include <ad_gpadc.h>

PRIVILEGED_DATA static gpadc_dynamic_data dynamic_data;

#undef GPADC_SOURCE
#undef GPADC_INIT

#define GPADC_SOURCE(name, _clock_source, _input_mode, _input, _sample_time, cancel_offset, \
                                                                _oversampling, _input_voltage) \
        gpadc_src_dynamic_data src_data_##name; \
        const gpadc_source_config src_##name = { \
                .hw_init.clock = _clock_source, \
                .hw_init.input_mode = _input_mode, \
                .hw_init.input = _input, \
                .hw_init.sample_time = _sample_time, \
                .hw_init.chopping = cancel_offset, \
                .hw_init.oversampling = _oversampling, \
                .hw_init.input_attenuator = _input_voltage, \
                .gpadc_data = &dynamic_data, \
        }; \
        const void *const name = &src_##name;

#include <platform_devices.h>

static bool ad_gpadc_prepare_for_sleep(void)
{
        /* Do not sleep when current source exists and is used */
        if (dynamic_data.current_source && dynamic_data.gpadc_acquire_count != 0) {
                return false;
        }

        /* When source exists and is not used reset current source */
        if (dynamic_data.current_source) {
                dynamic_data.current_source = NULL;
        }

        return true;
}

static void ad_gpadc_sleep_canceled(void)
{
}

static void ad_gpadc_wake_up_ind(bool arg)
{
}

const adapter_call_backs_t ad_gpadc_pm_call_backs = {
        .ad_prepare_for_sleep = ad_gpadc_prepare_for_sleep,
        .ad_sleep_canceled = ad_gpadc_sleep_canceled,
        .ad_wake_up_ind = ad_gpadc_wake_up_ind,
        .ad_xtal16m_ready_ind = NULL,
        .ad_sleep_preparation_time = 0
};

void ad_gpadc_init(void)
{
        pm_register_adapter(&ad_gpadc_pm_call_backs);
        gpadc_dynamic_data *data = &dynamic_data;
        OS_EVENT_CREATE(data->event);
}

static void ad_gpadc_input_initialize(void)
{
        const gpadc_source_config *source = dynamic_data.current_source;

        if (dynamic_data.gpadc_acquire_count == 1) {
                if ((source->hw_init).input == HW_GPADC_INPUT_SE_TEMPSENS) {
                        hw_tempsens_enable();
                }
        }
}

static void ad_gpadc_source_finalize(void)
{
        const gpadc_source_config *source = dynamic_data.current_source;

        if ((source->hw_init).input == HW_GPADC_INPUT_SE_TEMPSENS) {
                hw_tempsens_disable();
        }
}

static void ad_gpadc_apply_config(gpadc_source_config *source)
{
        dynamic_data.current_source = source;
        hw_gpadc_reset();
        hw_gpadc_configure(&(source->hw_init));
}

void ad_gpadc_acquire(void)
{
        bool acquired __attribute__((unused));

        acquired = ad_gpadc_acquire_to(RES_WAIT_FOREVER);
        OS_ASSERT(acquired);
}

bool ad_gpadc_acquire_to(uint32_t timeout)
{
        uint32_t ret = false;

        OS_TASK current_task = OS_GET_CURRENT_TASK();

        if (current_task == dynamic_data.owner) {
                dynamic_data.gpadc_acquire_count++;
                return true;
        }

        if (resource_acquire(RES_MASK(RES_ID_GPADC), timeout)) {
                dynamic_data.owner = current_task;
                dynamic_data.gpadc_acquire_count++;
                ret = true;
        }

        return ret;
}

void ad_gpadc_release(void)
{
        /* Make sure that gpadc is acquired at least once before attempting to release it */
        OS_ASSERT(dynamic_data.gpadc_acquire_count > 0);

        if (--dynamic_data.gpadc_acquire_count == 0) {
                /* A device release can only happen from the same task that owns it, or from an ISR */
                OS_ASSERT(in_interrupt() || (OS_GET_CURRENT_TASK() == dynamic_data.owner));
                if (dynamic_data.current_source) {
                        ad_gpadc_source_finalize();
                }

                dynamic_data.owner = NULL;
                resource_release(RES_MASK(RES_ID_GPADC));
        }
}

gpadc_source ad_gpadc_open(const gpadc_source_id id)
{
        gpadc_source_config *config = (gpadc_source_config *) id;

        return (gpadc_source) config;
}

void ad_gpadc_close(gpadc_source src)
{
}

static void ad_gpadc_cb(void)
{
        int val;

        val = hw_gpadc_get_value();

        hw_gpadc_clear_interrupt();

        dynamic_data.read_in_progress = false;

        ad_gpadc_release();

        dynamic_data.read_cb(dynamic_data.user_data, val);
}

bool ad_gpadc_read_async(gpadc_source src, ad_gpadc_user_cb read_async_cb, void *user_data)
{
        gpadc_source_config *source = (gpadc_source_config *) src;

        if (!read_async_cb) {
                return false;
        }

        ad_gpadc_acquire();

        if (dynamic_data.read_in_progress) {
                ad_gpadc_release();
                return false;
        }

        dynamic_data.read_in_progress = true;

        if (dynamic_data.current_source != source) {
                ad_gpadc_apply_config(source);
        }

        ad_gpadc_input_initialize();

        dynamic_data.read_cb = read_async_cb;
        dynamic_data.user_data = user_data;

        hw_gpadc_register_interrupt(ad_gpadc_cb);

        /* Start actual conversion */
        hw_gpadc_start();

        return true;
}

void ad_gpadc_read(gpadc_source src, uint16_t *value)
{
        bool acquired __attribute__((unused));

        acquired = ad_gpadc_read_to(src, value, RES_WAIT_FOREVER);
        OS_ASSERT(acquired);
}

bool ad_gpadc_read_to(gpadc_source src, uint16_t *value, uint32_t timeout)
{
        gpadc_source_config *source = (gpadc_source_config *) src;
        bool ret = false;

        if (ad_gpadc_acquire_to(timeout)) {
                hw_gpadc_unregister_interrupt();

                if (dynamic_data.current_source != source) {
                        ad_gpadc_apply_config(source);
                }

                ad_gpadc_input_initialize();

                hw_gpadc_adc_measure();

                *value = hw_gpadc_get_value();
                ad_gpadc_release();
                ret =  true;
        }

        return ret;
}

uint16_t ad_gpadc_get_source_max(gpadc_source source)
{
        gpadc_source_config *cfg = (gpadc_source_config *) source;

        return 0xFFFF >> (6 - MIN(6, cfg->hw_init.oversampling));
}

ADAPTER_INIT(ad_gpadc_adapter, ad_gpadc_init);

#endif /* dg_configGPADC_ADAPTER */

/**
 * \}
 * \}
 * \}
 */
