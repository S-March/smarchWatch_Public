/**
 ****************************************************************************************
 *
 * @file lld_data.h
 *
 * @brief Functions for data transmission/reception handling
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef LLD_DATA_H_
#define LLD_DATA_H_

/**
 ****************************************************************************************
 * @addtogroup LLDDATA LLDDATA
 * @ingroup LLD
 * @brief Functions for data transmission/reception handling
 *
 * This module implements the primitives allowing the LLC asking for data transmission. It
 * configures the Tx lists and handles the reception.
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>
#include "co_buf.h"
#include "lld.h"
#include "lld_evt.h"

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Ask for a data transmission
 *
 * This function is called by the LLC or LLM to request for a transmission. It chains the
 * descriptor passed as parameter in the list of descriptors ready for transmission.
 *
 * @param[in] elt    Pointer to the element for which a transmission is requested
 * @param[in] txnode Pointer to the TX node
 *
 ****************************************************************************************
 */
void lld_data_tx_push(struct ea_elt_tag *elt, struct co_buf_tx_node *txnode);

/**
 ****************************************************************************************
 * @brief Loop back the TX data in the exchange memory
 *
 * This function chains the next descriptor pointer of the last TX descriptor with the
 * first TX descriptor. It therefore creates a loop.
 *
 * @param[in] evt Event associated with the data to loop
 *
 ****************************************************************************************
 */
void lld_data_tx_loop(struct lld_evt_tag *evt);

/**
 ****************************************************************************************
 * @brief Prepare buffer pointers in the control structure for TX
 *
 * This function is called by the event scheduler when an event for a connection is ready
 * to be programmed. It chains the descriptors ready for transmissions with the ones
 * already programmed, and update the control structure with the pointer to the first
 * descriptor.
 *
 * @param[in] evt Event for which the buffers have to be programmed
 *
 ****************************************************************************************
 */
void lld_data_tx_prog(struct lld_evt_tag *evt);

/**
 ****************************************************************************************
 * @brief Check the packets that have been received and indicate them to the upper layers
 *
 * @param[in]  evt    Event for which received data have to be checked
 * @param[out] msg    Message structure to be filled with the number of received packets
 *                    and the pointer to the first RX descriptor
 * @param[in]  rx_cnt Number of buffers that have be handled
 *
 ****************************************************************************************
 */
void lld_data_rx_check(struct lld_evt_tag *evt, struct lld_data_ind *msg, uint8_t rx_cnt);

/**
 ****************************************************************************************
 * @brief Flush the data currently programmed for transmission
 *
 * @param[in] evt Pointer to the event for which TX data has to be flushed
 ****************************************************************************************
 */
void lld_data_tx_flush(struct lld_evt_tag *evt);
/**
 ****************************************************************************************
 * @brief Check the packets that have been received and indicate them to the upper layers
 *
 * @param[in]  evt    Event for which received data have to be checked
 * @param[out] msg    Message structure to be filled with the number of received packets
 *                    and the pointer to the first RX descriptor
 * @param[in]  rx_cnt Number of buffers that have be handled
 *
 ****************************************************************************************
 */
void lld_data_rx_flush(struct lld_evt_tag *evt, uint8_t rx_cnt);

/**
 ****************************************************************************************
 * @brief Check the packets that have been transmitted and confirm them to the upper layers
 *
 * @param[in] evt     Event for which transmitted data have to be checked
 * @param[out] msg    Message structure to be filled with the number of transmitted data
 *                    control and non-connected packets.
 *
 ****************************************************************************************
 */
#if (BLE_CENTRAL || BLE_PERIPHERAL)
void lld_data_tx_check(struct lld_evt_tag *evt, struct lld_data_ind *msg);
#endif

/// @} LLDDATA

#endif // LLD_DATA_H_
