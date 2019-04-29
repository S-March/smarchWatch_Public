/**
 ****************************************************************************************
 *
 * @file l2cm.h
 *
 * @brief Header file - L2CM.
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef L2CM_H_
#define L2CM_H_

/**
 ****************************************************************************************
 * @addtogroup L2C Logical Link Control and Adaptation Protocol
 * @ingroup HOST
 * @brief Logical Link Control and Adaptation Protocol - BLE Host.
 *
 * The L2C module is responsible for L2CAP provides connection-oriented
 * and connection-less data services to upper layer protocols with protocol
 * multiplexing capability and segmentation and re-assembly operation.
 *
 * @{
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @addtogroup L2CM L2CAP Manager
 * @ingroup L2C
 * @brief L2CAP block for signaling and resource management functions
 *
 * The L2CM is an internal L2CAP block responsible for managing
 * the signaling messages and propagation of connection and disconnection
 * related L2CAP messages within the Host.
 *
 * @{
 ****************************************************************************************
 */
#include "rwip_config.h"
#include "co_version.h"

#include <stdbool.h>
#include <stdint.h>
#include "compiler.h"

#if (RWBLE_SW_VERSION_MAJOR >= 8)
#include "llc.h"
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#if (BLE_L2CM)

/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */

/// L2CM buffer management
struct l2cm_buf_mon
{
    /// LE buffer watch
    uint16_t    le_acl_data_pkt_len;
    uint16_t    le_acl_total_nb_acl_pkt;
    /// number of buffers still available
    uint16_t    nb_buffer_avail;
};
/// L2CAP Manager environment structure to be saved
struct l2cm_env_tag
{
    /// buffer information
    struct l2cm_buf_mon buf_mon;
 };

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */
extern struct l2cm_env_tag l2cm_env;
/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Acquire low layer LE ACL packet
 ****************************************************************************************
 */
__STATIC_INLINE void l2cm_buffer_acquire(void)
{
    l2cm_env.buf_mon.nb_buffer_avail--;
}

/**
 ****************************************************************************************
 * @brief Release low layer LE ACL packet
 *
 * @parm[in] nb Number of buffer to release.
 ****************************************************************************************
 */
__STATIC_INLINE void l2cm_buffer_release(uint16_t nb)
{
    l2cm_env.buf_mon.nb_buffer_avail += nb;
}

/**
 ****************************************************************************************
 * @brief Retrieve number of low layer buffer available
 *
 * @return Number of low layer buffer available
 ****************************************************************************************
 */
__STATIC_INLINE uint16_t l2cm_get_nb_buffer_available(void)
{
    return l2cm_env.buf_mon.nb_buffer_avail;
}

/**
 ****************************************************************************************
 * @brief Retrieve low layer LE ACL Buffers size
 *
 * @return size of LE ACL Buffers
 ****************************************************************************************
 */
#if (RWBLE_SW_VERSION_MAJOR >= 8)
__STATIC_INLINE uint16_t l2cm_get_buffer_size(uint16_t conidx)
{
    //take time into account
    return llc_env[conidx]->connEffectiveMaxTxOctets_Time>LE_LENGTH_EXT_OCTETS_MIN?llc_env[conidx]->connEffectiveMaxTxOctets_Time:LE_LENGTH_EXT_OCTETS_MIN;
}
#else
__STATIC_INLINE uint16_t l2cm_get_buffer_size(void)
{
    return l2cm_env.buf_mon.le_acl_data_pkt_len;
}
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */
/**
 ****************************************************************************************
 * @brief Create and Initialize the L2CAP manager task.
 *
 * @param[in] reset   true if it's requested by a reset; false if it's boot initialization
 *
 ****************************************************************************************
 */
void l2cm_init(bool reset);



/**
 ****************************************************************************************
 * @brief Allocates an L2CAP resource.
 *
 * @param[in] conidx           connection index
 *
 ****************************************************************************************
 */
void l2cm_create(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief De-initializes the L2CAP resources for specified link.
 *
 * @param[in] conidx           connection index
 *
 ****************************************************************************************
 */
void l2cm_cleanup(uint8_t conidx);


/**
 ****************************************************************************************
 * @brief Sets link layer buffer size capacity
 *
 * @param[in] pkt_len           length of the packet
 * @param[in] nb_acl            number of acl
 *
 ****************************************************************************************
 */
void l2cm_set_link_layer_buff_size(uint16_t pkt_len, uint16_t nb_acl);

/// @} L2CM

#endif //(BLE_L2CM)

#endif // L2CM_H_
