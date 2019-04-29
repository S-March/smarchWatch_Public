/**
 ****************************************************************************************
 *
 * @file gtl.h
 *
 * @brief This file contains definitions related to the Generic Transport Layer
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef GTL_H_
#define GTL_H_

/**
 ****************************************************************************************
 * @addtogroup GTL Generic Transport Layer
 * @ingroup GTL
 * @brief Generic Transport Layer, based on GTL functionality.
 *
 *@{
 *
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

#if (GTL_ITF)
#include "rwip.h"            // SW interface

#include "stdbool.h"         // boolean definition
#include "ke_msg.h"          // kernel message definition
#include "co_list.h"         // list API

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief GTL initialization function: initializes states and transport.
 *****************************************************************************************
 */
void gtl_init(const struct rwip_eif_api* eif);

#if DEEP_SLEEP
/**
 ****************************************************************************************
 * @brief Stop GTL flow to enter in sleep mode
 *
 * WARNING FOR CHIPS WITHOUT EXTERNAL WAKE UP !!!
 * This function also disable the RTS signal on the UART side if UART is used as physical
 * interface.
 * After sleep, RTS should be re-enabled.
 *
 * @return true if GTL has entered sleep, false otherwise
 *****************************************************************************************
 */
bool gtl_enter_sleep(void);

/**
 ****************************************************************************************
 * @brief Restart GTL flow to exit from sleep mode
 *****************************************************************************************
 */
void gtl_exit_sleep(void);
#endif //DEEP_SLEEP

/**
 * Handles message to send over GTL interface.
 * This function check message type to send it in correct format (HCI, FE, ...)
 *
 * @param[in] msg Kernel message to handle
 */
void gtl_send_msg(struct ke_msg * msg);

#endif //GTL_ITF

/// @} GTL
#endif // GTL_H_
