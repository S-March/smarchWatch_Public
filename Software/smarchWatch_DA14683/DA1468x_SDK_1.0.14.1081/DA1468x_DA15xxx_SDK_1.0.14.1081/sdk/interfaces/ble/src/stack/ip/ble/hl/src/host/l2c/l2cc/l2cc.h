/**
 ****************************************************************************************
 *
 * @file l2cc.h
 *
 * @brief Header file - L2CC.
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef L2CC_H_
#define L2CC_H_

/**
 ****************************************************************************************
 * @addtogroup L2CC L2CAP Controller
 * @ingroup L2C
 * @brief L2CAP block for data processing and per device connection
 *
 * The L2CC is responsible for all the data processing related
 * functions of the L2CAP block per device connection.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_CENTRAL || BLE_PERIPHERAL)

#include <stdint.h>
#include <stdbool.h>
#include "co_list.h"

/*
 * DEFINES
 ****************************************************************************************
 */
/// Maximum number of instances of the L2CC task
#define L2CC_IDX_MAX            (BLE_CONNECTION_MAX)

/*
 * STRUCTURES
 ****************************************************************************************
 */

struct l2cc_pdu_recv_ind;

/// L2CAP environment structure
struct l2cc_env_tag
{
    /// Send PDU request
    struct l2cc_pdu_send_req *p_send_req;
    /// Received PDU Buffer
    struct l2cc_pdu_recv_ind *p_recv_ind;
    /// List for temporarily received segments
    struct co_list segment_list;
};



/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */

/// L2CAP environment pool
extern struct l2cc_env_tag *l2cc_env[L2CC_IDX_MAX];

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Create and Initialize the L2CAP controller task.
 *
 * @param[in] reset   true if it's requested by a reset; false if it's boot initialization
 *
 ****************************************************************************************
 */
void l2cc_init(bool reset);

/**
struct l2cc_pdu_recv_ind;
 *
 ****************************************************************************************
 * @brief Initialize the link layer controller task.
 *
 * @param[in] conidx            Connection index
 *
 ****************************************************************************************
 */
void l2cc_create(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief De-initialize the task.
 *
 * @param[in] conidx            Connection index
 *
 ****************************************************************************************
 */
void l2cc_cleanup(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Update state of all L2Cap controller task according to number of buffer
 * available
 ****************************************************************************************
 */
void l2cc_update_state(void);

#endif //(BLE_CENTRAL || BLE_PERIPHERAL)

/// @} L2CC

#endif // L2CC_H_
