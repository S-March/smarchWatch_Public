/**
 ****************************************************************************************
 *
 * @file gtl_eif.h
 *
 * @brief Transport module for the Generic Transport Layer
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef GTL_EIF_H_
#define GTL_EIF_H_

/**
 ****************************************************************************************
 * @addtogroup GTL Generic Transport Layer
 * @brief Generic Transport Layer, based on GTL functionality.
 *
 * This module creates the abstraction between UART driver and GTL generic functions
 * (designed for any transport layer).
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

#if (GTL_ITF)

#include <stdint.h>


/*
 * DEFINES
 ****************************************************************************************
 */

/// Kernel message type
#define GTL_KE_MSG_TYPE                             0x05

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief GTL EIF transport initialization.
 * Puts the UART driver in reception, waiting for simple 1 byte message type. Space for
 * reception is allocated with ke_msg_alloc and the pointer is handed to uart_env.rx. RX
 * interrupt is enabled.
 *****************************************************************************************
 */
void gtl_eif_init(void);

/**
 ****************************************************************************************
 * @brief GTL EIF write function.
 *
 * @param[in] type  Type of the buffer to be transmitted. It can take one of the following
 *                  values:
 *      - @ref GTL_EVT_MSG_TYPE for event message
 *      - @ref GTL_ACL_MSG_TYPE for ACL data
 *      - @ref GTL_SYNC_MSG_TYPE for synchronous data
 *
 * @param[in] buf   Pointer to the buffer to be transmitted. @note The buffer passed as
 *  parameter must have one free byte before the first payload byte, so that the GTL EIF
 *  module can put the type byte as first transmitted data.
 *
 * @param[in] len   Length of the buffer to be transmitted.
 *
 *****************************************************************************************
 */
void gtl_eif_write(uint8_t type, uint8_t *buf, uint16_t len);

#if (DEEP_SLEEP)
/**
 ****************************************************************************************
 * @brief Start UART flow
 *
 *****************************************************************************************
 */
void gtl_eif_start(void);

/**
 ****************************************************************************************
 * @brief Stop UART flow if possible
 *
 * @return true if UART flow was stopped, false otherwise
 *****************************************************************************************
 */
bool gtl_eif_stop(void);
#endif //DEEP_SLEEP

#endif //GTL_ITF

/// @} GTL_EIF
#endif // GTL_EIF_H_
