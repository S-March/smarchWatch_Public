/**
 ****************************************************************************************
 *
 * @file gtl_env.h
 *
 * @brief This file contains definitions related to the Generic Transport Layer
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef GTL_ENV_H_
#define GTL_ENV_H_

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

#include "ke_msg.h"



/*
 * DEFINES
 ****************************************************************************************
 */

///Kernel message header length for transport through interface between App and SW stack.
#define KE_MSG_HDR_LEN       8

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

///GTL states for RX
enum GTL_STATES_RX
{
    /// GTL RX Start State - receive message type
    GTL_STATE_RX_START,
    /// GTL RX Header State - receive message header
    GTL_STATE_RX_HDR,
    /// GTL RX Header State - receive (rest of) message payload
    GTL_STATE_RX_PAYL,
    /// GTL RX Out Of Sync state - receive sync pattern
    GTL_STATE_RX_OUT_OF_SYNC,
};

///Structure for application system interface packet header
struct gtl_kemsghdr
{
    ///Message id
    ke_msg_id_t  id;
    ///Destination task identifier for KE
    ke_task_id_t dest_id;
    ///Source task identifier for KE
    ke_task_id_t src_id;
    ///Message parameter length
    uint16_t     param_len;
};

/// GTL out of synchronization recovery variables
struct gtl_out_of_sync_tag
{
    /// Current received byte
    uint8_t byte;
    /// Index of the sync pattern
    uint8_t index;
};

///GTL Environment context structure
struct gtl_env_tag
{
    /// list of TX buffer in pending queue
    struct co_list tx_queue;

    /// pointer to External interface api
    const struct rwip_eif_api* ext_if;

    /// Ongoing RX message
    struct ke_msg * p_msg_rx;

    /// Ongoing TX message
    struct ke_msg * p_msg_tx;

    /// GTL synchronization error parameters
    struct gtl_out_of_sync_tag out_of_sync;

    ///Latest received message header, 8 bytes buffer
    uint8_t  curr_hdr_buff[8];

    ///Rx state - can be receiving message type, header, payload or error
    uint8_t  rx_state;

    ///Latest received message type: KE/....
    uint8_t  curr_msg_type;
};

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */

///GTL environment structure external global variable declaration
extern struct gtl_env_tag gtl_env;


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */


#endif //GTL_ITF

/// @} GTL
#endif // GTL_ENV_H_
