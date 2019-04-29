/**
 * \addtogroup BSP
 * \{
 * \addtogroup ADAPTERS
 * \{
 * \addtogroup RF_ADAPTER
 *
 * \brief RF adapter
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_rf.h
 *
 * @brief Radio module access API.
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef AD_RF_H_
#define AD_RF_H_

#if dg_configRF_ADAPTER

#include <stdbool.h>
#include <stdint.h>
#include "osal.h"
#include "platform_devices.h"

#include "hw_rf.h"

/**
 * @brief Performs RF adapter initialization
 */
void ad_rf_init(void);

/**
 * \brief Retry a failed calibration
 *
 * This will power-cycle RF, reapply tcs and recommended settings, and
 * retry calibration. If calibration fails again, it will reset the system
 * (using the wdog)
 */
void ad_rf_retry_calibration();

/**
 * \brief Start Calibration procedure and check if it succeeds
 *
 * This will start the calibration procedure, and check if the calibration initial
 * part (the iff calibration) succeeds. If not, it will reset the RF block and retry.
 * If the calibration still fails after the second attempt, it will trigger a
 * watchdog reset
 *
 */
static inline void ad_rf_start_and_check_calibration()
{
        if (!hw_rf_start_calibration())
                 ad_rf_retry_calibration();
}

/**
 * \brief Perform RF system initialization.
 *
 * This will preform a full RF system init, and check if the
 * calibration initial part (the iff calibration) succeeds. If not, it will
 * reset the RF block and retry.
 * If the calibration still fails after the second attempt, it will trigger a
 * watchdog reset
 *
 */
static inline void ad_rf_system_init()
{
        if (!hw_rf_system_init())
                 ad_rf_retry_calibration();
}

/**
 * \brief Start Calibration procedure and return.
 *
 * This will block for some time (with interrupts disabled) in order to perform
 * The first part of calibration (IFF, DC offset and the start of gain calib).
 *
 */
static inline void ad_rf_start_calibration()
{
        OS_ENTER_CRITICAL_SECTION();
        ad_rf_start_and_check_calibration();
        OS_LEAVE_CRITICAL_SECTION();
}

/**
 * \brief Sets parameters according to their recommended values, taking RF state into account.
 *
 * Acts like \ref hw_rf_set_recommended_settings but makes sure that the RF power domain is on and
 * unconfigured. Disables interrupts.
 *
 */
static inline void ad_rf_request_recommended_settings(void)
{
        OS_ENTER_CRITICAL_SECTION();
        hw_rf_request_recommended_settings();
        OS_LEAVE_CRITICAL_SECTION();
}

/**
 * \brief Requests that the RF is turned on
 *
 * Requests that the RF is turned on, if not already on. Disables interrupts.
 *
 * \param [in] mode_ble True, if the rf is needed for ble
 *
 */
static inline void ad_rf_request_on(bool mode_ble)
{
        OS_ENTER_CRITICAL_SECTION();
        hw_rf_request_on(mode_ble);
        OS_LEAVE_CRITICAL_SECTION();
}

/**
 * \brief Requests that the RF is turned off
 *
 * Requests that the RF is turned off, if not already off.
 * The RF will be turned off only if there are no more
 * requests (ie. all requesters have called ad_rf_request_off())
 * Disables interrupts
 *
 * \param [in] mode_ble True, if the rf was needed for ble
 */
static inline void ad_rf_request_off(bool mode_ble)
{
        ad_gpadc_acquire();
        OS_ENTER_CRITICAL_SECTION();
        hw_rf_request_off(mode_ble);
        OS_LEAVE_CRITICAL_SECTION();
        ad_gpadc_release();
}

#endif /* dg_configRF_ADAPTER */

#endif /* AD_RF_H_ */

/**
 * \}
 * \}
 * \}
 */
