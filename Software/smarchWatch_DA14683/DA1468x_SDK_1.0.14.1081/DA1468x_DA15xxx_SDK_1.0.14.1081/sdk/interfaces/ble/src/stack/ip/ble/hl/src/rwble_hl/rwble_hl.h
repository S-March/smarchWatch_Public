/**
 ****************************************************************************************
 *
 * @file rwble_hl.h
 *
 * @brief Entry points of the BLE HL software
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef RWBLE_HL_H_
#define RWBLE_HL_H_

#include <stdint.h>

/**
 ****************************************************************************************
 * @addtogroup ROOT
 * @brief Entry points of the BLE Host stack
 *
 * This module contains the primitives that allow an application accessing and running the
 * BLE protocol stack
 *
 * @{
 ****************************************************************************************
 */

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @brief Initialize the BLE Host stack.
 ****************************************************************************************
 */
void rwble_hl_init(void);

/**
 ****************************************************************************************
 * @brief Initialize the host (reset requested)
 *
 ****************************************************************************************
 */
void rwble_hl_reset(void);

#if GTL_ITF
/**
 ****************************************************************************************
 * @brief Send an error message to Host.
 *
 * This function is used to send an error message to Host from platform.
 *
 * @param[in] error    Error detected by FW
 ****************************************************************************************
 */
void rwble_hl_send_message(uint32_t error);
#endif //GTL_ITF

/// @} RWBLE_HL

#endif // RWBLE_HL_H_
