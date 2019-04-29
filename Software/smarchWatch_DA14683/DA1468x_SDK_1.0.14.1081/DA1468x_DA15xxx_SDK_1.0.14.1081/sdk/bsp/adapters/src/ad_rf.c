/**
 * \addtogroup BSP
 * \{
 * \addtogroup ADAPTERS
 * \{
 * \addtogroup RF
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_rf.c
 *
 * @brief Radio module access functions.
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configRF_ADAPTER

#include "ad_rf.h"
#include "platform_devices.h"
#include "hw_rf.h"
#include "FreeRTOS.h"
#include "osal.h"
#include "task.h"

#include "timers.h"
#include "sys_rtc.h"
#include "sys_tcs.h"
#include "osal.h"

#include "sys_power_mgr.h"
#include "hw_watchdog.h"

#include <stdbool.h>

PRIVILEGED_DATA OS_TICK_TIME last_calib_time;
PRIVILEGED_DATA int last_calib_temp;
PRIVILEGED_DATA TimerHandle_t recalib_timer;

static inline bool is_called_from_isr(void)
{
        return ( portNVIC_INT_CTRL_REG & 0x3f ) != 0;
}

/**
 * @brief Checks whether RF re-calibration is needed.
 *
 * Radio re-calibration is required in order to cope with temperature changes that affect its
 * response. The function is meant to be called when the node is activating sleep mode, and the
 * sleep duration must be known. Initially it performs time-related checks to determine whether
 * re-calibration is required. Subsequently, it connects to the temperature sensor to check whether
 * the temperature changed significantly.
 *
 * @param [in] Sleep duration in LP clock cycles.
 *
 * @return true when re-calibration is needed.
 */
static bool ad_rf_check_calibration(uint32_t sleep_duration_in_lp)
{
#if dg_configRF_ENABLE_RECALIBRATION == 1
        bool force = false;
        tempsens_source t_src;
        bool read_temp;
        int temp;

        /* If max recalibration timeout is enabled, and time since last calib is more than that,
         * force a recalibration
         */
        if (dg_configRF_MAX_RECALIBRATION_TIMEOUT != 0 &&
                (xTaskGetTickCount() - last_calib_time) >= dg_configRF_MAX_RECALIBRATION_TIMEOUT) {
                force = true;
        }


        if (!force) {
                if (sleep_duration_in_lp < dg_configRF_RECALIBRATION_DURATION)
                        return false;
                if ( (xTaskGetTickCount() - last_calib_time) < dg_configRF_MIN_RECALIBRATION_TIMEOUT)
                        return false;
        }


        t_src = ad_tempsens_open();
        /* Attempt to read temperature. If ADC is busy, don't block */
        read_temp = ad_tempsens_read_to(t_src, &temp, 0);
        ad_tempsens_close(t_src);

        /* If it failed to read the temp (because it couldn't acquire the adc lock)
         * abort recalibration
         */
        if (!read_temp)
                return false;

        last_calib_time = xTaskGetTickCount();

        int diff;

        if (temp > last_calib_temp) {
                diff = temp - last_calib_temp;
        } else {
                diff = last_calib_temp - temp;
        }


        if (!force && diff < dg_configRF_MIN_RECALIBRATION_TEMP_DIFF)
                return false;

        last_calib_temp = temp;
#endif
        return true;
}

static inline uint32_t get_distance(uint64_t wakeup)
{
        uint32_t distance = UINT32_MAX;
        uint64_t rtc_time;

        /* A wakeup time of pm_wakeup_xtal16m_time means infinite sleep
         * or disabled
         */
        if (wakeup != (uint64_t)pm_wakeup_xtal16m_time) {
                rtc_time = rtc_get();

                if (wakeup <= rtc_time) {
                        /* We've probably missed the deadline. No sleep_duration for the MAC */
                        distance = 0;
                } else {
                        distance = (uint32_t) (wakeup - rtc_time);
                }
        }

        return distance;
}

void ad_rf_retry_calibration()
{
        /* Power-cucle RF */
        hw_rf_poweroff();
        hw_rf_poweron();

        sys_tcs_apply(tcs_radio);
        hw_rf_set_recommended_settings();

        if (!hw_rf_calibration()) {
                /* Something VERY bad happened. Issue a watchdog reset */
                hw_watchdog_set_pos_val(1);
                hw_watchdog_gen_RST();
                hw_watchdog_unfreeze();
                while (1);
        }
}

uint64_t hw_rf_get_start_iff_time(void)
{

        return rtc_get_fromISR();
}



bool hw_rf_check_iff_timeout(uint64_t start_time)
{
        if ((rtc_get_fromISR() - start_time) > dg_configRF_IFF_CALIBRATION_TIMEOUT)
                return true;
        return false;
}

/* Overridden weak function, from hw_rf.c*/
bool hw_rf_preoff_cb(void)
{
        /* Default sleep duration value is UINT_MAX. This value
         * marks that the MACs either never sleep, or don't
         * want to be waken up by a timer.
         */
        uint64_t sleep_duration_in_lp = UINT64_MAX;

        /* Stop recalib timer */
        if (dg_configRF_RECALIBRATION_TIMER_TIMEOUT > 0) {
                if (recalib_timer != NULL) {
                        if (is_called_from_isr()) {
                                OS_TIMER_STOP_FROM_ISR(recalib_timer);
                        } else {
                                OS_TIMER_STOP(recalib_timer, OS_TIMER_FOREVER);
                        }
                }
        }

        if (dg_configRF_ENABLE_RECALIBRATION == 0) {
                return false;
        }

#if defined(CONFIG_USE_BLE) && defined(CONFIG_USE_FTDF)
        uint64_t ble_wakeup = pm_get_mac_wakeup_time(PM_BLE_ID);
        uint64_t ftdf_wakeup = pm_get_mac_wakeup_time(PM_FTDF_ID);
        uint32_t ble_distance = get_distance(ble_wakeup);
        uint32_t ftdf_distance = get_distance(ftdf_wakeup);

        if (ftdf_distance > ble_distance)
                sleep_duration_in_lp = ble_distance;
        else
                sleep_duration_in_lp = ftdf_distance;

#elif defined(CONFIG_USE_BLE)
        uint64_t ble_wakeup = pm_get_mac_wakeup_time(PM_BLE_ID);

        sleep_duration_in_lp = get_distance(ble_wakeup);

#elif defined(CONFIG_USE_FTDF)
        uint64_t ftdf_wakeup = pm_get_mac_wakeup_time(PM_FTDF_ID);

        sleep_duration_in_lp = get_distance(ftdf_wakeup);

#endif

        if (ad_rf_check_calibration(sleep_duration_in_lp) == true) {

                ad_rf_start_and_check_calibration();
                return true;
        }

        return false;
}

static void recalib_timer_cb( TimerHandle_t pxTimer )
{
        ad_rf_start_calibration();
}

/* Overridden weak function, from hw_rf.c */
void hw_rf_precalib_cb(void)
{
        pm_stay_alive();
}

/* Overridden weak function, from hw_rf.c */
void hw_rf_postcalib_cb(void)
{
        pm_resume_sleep();
}

/* Overridden weak function, from hw_rf.c */
void hw_rf_postconf_cb(void)
{
        if (dg_configRF_RECALIBRATION_TIMER_TIMEOUT == 0)
                return;

        /* Start/Reset Timer */
        if (is_called_from_isr()) {
                OS_TIMER_RESET_FROM_ISR(recalib_timer);
        } else {
                OS_TIMER_RESET(recalib_timer, OS_TIMER_FOREVER);
        }

}

void hw_rf_apply_tcs_cb(void)
{
        sys_tcs_apply(tcs_radio);
}

void ad_rf_init(void)
{
        if (dg_configRF_RECALIBRATION_TIMER_TIMEOUT > 0) {
                /* Initialize timer */
                recalib_timer = OS_TIMER_CREATE("RFRecalibTimer",
                        dg_configRF_RECALIBRATION_TIMER_TIMEOUT,
                        pdFALSE,
                        NULL,
                        recalib_timer_cb);
                OS_ASSERT(recalib_timer != NULL);
        }
}

#endif /* dg_configRF_ADAPTER */

/**
 * \}
 * \}
 * \}
 */
