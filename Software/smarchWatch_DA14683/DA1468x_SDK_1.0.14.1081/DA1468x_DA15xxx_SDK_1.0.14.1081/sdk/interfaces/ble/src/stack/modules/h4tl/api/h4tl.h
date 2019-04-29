/**
 ****************************************************************************************
 *
 * @file h4tl.h
 *
 * @brief H4 UART Transport Layer header file.
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef H4TL_H_
#define H4TL_H_

/**
 ****************************************************************************************
 * @addtogroup H4TL H4 UART Transport Layer
 * @ingroup H4TL
 * @brief H4 UART Transport Layer
 *
 * This module creates the abstraction between External UART driver and HCI generic functions
 * (designed for H4 UART transport layer).
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"  // stack configuration

#if (H4TL_SUPPORT)
#include "rwip.h"            // SW interface

#include <stdint.h>       // standard integer definition
#include <stdbool.h>      // standard boolean definition

/*
 * DEFINES
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
 * @brief H4TL transport initialization.
 *
 * Puts the External Interface driver in reception, waiting for simple 1 byte message type. Space for
 * reception is allocated with ke_msg_alloc and the pointer is handed to env.rx. RX
 * interrupt is enabled.
 *
 * @param[in] len   Length of the buffer to be transmitted.
 *
 *****************************************************************************************
 */
void h4tl_init(const struct rwip_eif_api* eif);


/**
 ****************************************************************************************
 * @brief H4TL write function.
 *
 * @param[in] type  Type of the buffer to be transmitted. It can take one of the following
 *                  values:
 *      - @ref HCI_EVT_MSG_TYPE for event message
 *      - @ref HCI_ACL_MSG_TYPE for ACL data
 *      - @ref HCI_SYNC_MSG_TYPE for synchronous data
 *
 * @param[in] buf   Pointer to the buffer to be transmitted. @note The buffer passed as
 *  parameter must have one free byte before the first payload byte, so that the H4TL
 *  module can put the type byte as first transmitted data.
 *
 * @param[in] len   Length of the buffer to be transmitted.
 * @param[in] tx_callback   Callback for indicating the end of transfer
 *****************************************************************************************
 */
void h4tl_write(uint8_t type, uint8_t *buf, uint16_t len, void (*tx_callback)(void));

#if DEEP_SLEEP
/**
 ****************************************************************************************
 * @brief Start External Interface input flow
 *
 *****************************************************************************************
 */
void h4tl_start(void);

/**
 ****************************************************************************************
 * @brief Stop External Interface input flow if possible
 *
 * @return true if External Interface flow was stopped, false otherwise
 *****************************************************************************************
 */
bool h4tl_stop(void);
#endif //DEEP_SLEEP

#endif //H4TL_SUPPORT

/// @} H4TL

#endif // H4TL_H_
