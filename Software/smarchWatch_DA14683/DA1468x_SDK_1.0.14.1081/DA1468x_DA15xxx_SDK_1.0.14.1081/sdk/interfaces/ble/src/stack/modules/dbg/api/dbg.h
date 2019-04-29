/**
****************************************************************************************
*
* @file dbg.h
*
* @brief Debug function
*
* Copyright (C) RivieraWaves 2009-2014
*
*
****************************************************************************************
*/
#ifndef DBG_H_
#define DBG_H_

/**
****************************************************************************************
* @addtogroup DBG
* @ingroup CONTROLLER
* @brief Debug
*
* @{
****************************************************************************************
*/


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // stack configuration

#include "dbg_swdiag.h"        // sw profiling definitions


/*
 * FUNCTION DECLARATION
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialization of the BT Debug task
 *
 * This function initializes the the DBG task
 *
 ****************************************************************************************
 */
void dbg_init(void);

#if (RW_DEBUG)
/**
 ****************************************************************************************
 * @brief Print debug messages.
 *
 * @param[in] format         data buffer for printing
 *
 * @return Status
 *
 ****************************************************************************************
 */
void dbg_warning(const char *format, ...);
#endif //RW_DEBUG

#if BLE_EMB_PRESENT
/**
 ****************************************************************************************
 * @brief Send back to host status of platform reset request.
 *
 * @param status Reset error code
 *
 ****************************************************************************************
 */
void dbg_platform_reset_complete(uint32_t error);
#endif //BLE_EMB_PRESENT

///@} DBG

#endif // DBG_H_
