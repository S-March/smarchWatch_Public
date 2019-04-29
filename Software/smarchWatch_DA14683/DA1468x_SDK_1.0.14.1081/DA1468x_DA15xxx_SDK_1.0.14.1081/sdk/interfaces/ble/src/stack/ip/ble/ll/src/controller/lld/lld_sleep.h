/**
 ****************************************************************************************
 *
 * @file lld_sleep.h
 *
 * @brief Functions for RWBLE core sleep mode management
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef LLD_SLEEP_H_
#define LLD_SLEEP_H_

/**
 ****************************************************************************************
 * @addtogroup LLDSLEEP LLDSLEEP
 * @ingroup LLD
 * @brief Functions for RWBLE core sleep mode management
 *
 * This module implements the function that manages deep sleep of BLE core.
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (DEEP_SLEEP)
#include <stdint.h>
#include <stdbool.h>

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

#if (RW_BLE_SUPPORT)

/**
 ****************************************************************************************
 * @brief Initialize sleep module
 ****************************************************************************************
 */
void lld_sleep_init(void);

/**
 ****************************************************************************************
 * @brief The Sleep function. Enter BLE Core in deep sleep
 *
 * @param[in]    sleep_duration  Duration of deep sleep (slot time duration)
 * @param[in]    ext_wakeup      False: external wake-up disabled / True: enabled
 ****************************************************************************************
 */
void lld_sleep_enter(uint32_t sleep_duration, bool ext_wakeup);

/**
 ****************************************************************************************
 * @brief Function to wake up BLE core
 ****************************************************************************************
 */
void lld_sleep_wakeup(void);

/**
 ****************************************************************************************
 * @brief Function to handle the end of BLE core wake up
 ****************************************************************************************
 */
void lld_sleep_wakeup_end(void);

#endif // RW_BLE_SUPPORT

/**
 ****************************************************************************************
 * @brief Check if sleep mode is possible
 *
 * The function takes as argument the allowed sleep duration that must not be increased.
 * If BLE needs an earlier wake-up than initial duration, the allowed sleep duration
 * is updated.
 * If BLE needs a shorter duration than the wake-up delay, sleep is not possible and
 * the function returns a bad status.
 *
 * @param[in/out] sleep_duration   Initial allowed sleep duration (in slot of 625us)
 * @param[in]     wakeup_delay     Delay for system wake-up (in slot of 625us)
 *
 * @return true if sleep is allowed, false otherwise
 ****************************************************************************************
 */
//bool lld_sleep_check(uint32_t *sleep_duration, uint32_t wakeup_delay);

#endif // DEEP_SLEEP

/// @} LLDSLEEP

#endif // LLD_SLEEP_H_
