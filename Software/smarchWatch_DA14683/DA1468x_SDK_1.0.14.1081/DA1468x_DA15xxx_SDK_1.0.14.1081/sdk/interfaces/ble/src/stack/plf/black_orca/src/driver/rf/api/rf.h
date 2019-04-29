/**
 ****************************************************************************************
 *
 * @file rf.h
 *
 * @brief Common header file for all radios.
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef RF_H_
#define RF_H_

/**
 ****************************************************************************************
 * @addtogroup RF
 * @ingroup DRIVERS
 * @brief Common definitions for radio modules.
 *
 * This module declares the functions and constants that have to be defined for all RF.
 *
 * @{
 ****************************************************************************************
 */


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

struct rwip_rf_api;  // forward declaration to avoid including rw.h

/**
 *****************************************************************************************
 * @brief Initialization of RF.
 *
 * This function initializes the RF and fills the structure containing the function
 * pointers and parameters required by the RW BT stack.
 *
 * @param[out]  api  Pointer to the BT RF API structure
 *
 *****************************************************************************************
 */
void rf_init(struct rwip_rf_api *api);

/**
 *****************************************************************************************
 * @brief Set ANT_TRIM values for BLE RF
 *
 * This function sets the ANT_TRIM_VALUE (read: the frequency index table in the EM) for a
 * specific frequency index (i.e. an RF physical channel), to a specific value.
 * More specifically, it sets the provided value to bits 6:4 of the frequency table entry.
 *
 * @param[in]  freq_idx: The frequency index (0-39). Equal to (freq - 2402)/2, where freq
 *             is the channel frequency
 * @param[in]  value: The value to set to bits 6:4 of the frequency table entry
 *             (corresponding to the 3 GPIOs output by the hw for ANT_TRIM signals).
 *
 *****************************************************************************************
 */
void rf_ble_set_ant_trim(uint8_t freq_idx, uint8_t value);
/// @} RF

#endif // RF_H_
