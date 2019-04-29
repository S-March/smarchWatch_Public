/**
 ****************************************************************************************
 *
 * @file llc_data.h
 *
 * @brief Functions for data transmission/reception handling
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef LLC_DATA_H_
#define LLC_DATA_H_

/**
 ****************************************************************************************
 * @addtogroup LLCDATA LLCDATA
 * @ingroup LLC
 * @brief Functions for data pdu transmission/reception handling
 *
 * This module implements the functions allowing the handling of the transmission and
 * reception of the data pdu.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "co_buf.h"

#if (BLE_PERIPHERAL || BLE_CENTRAL)
/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Handles the reception of the data packet.
 *
 * This function forward the data packet to the host.
 *
 * @param[in] conhdl        Connection handle on which the pdu is received.
 * @param[in] rxdesc        Pointer oh the reception descriptor where the data are
 *                          available.
 ****************************************************************************************
 */

void llc_data_rcv(uint16_t conhdl, uint8_t hdl);

#endif // #if (BLE_PERIPHERAL || BLE_CENTRAL)
/// @} LLCDATA

#endif // LLC_DATA_H_
