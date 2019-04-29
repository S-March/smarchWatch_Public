/**
 ****************************************************************************************
 *
 * @file llc_util.h
 *
 * @brief Link layer controller utilities definitions
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */
#ifndef LLC_UTIL_H_
#define LLC_UTIL_H_
/**
 ****************************************************************************************
 * @addtogroup LLCUTIL
 * @ingroup LLC
 * @brief Link layer controller utilities definitions
 *
 * full description
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>

#include "compiler.h"
#include "co_buf.h"
#include "llc_task.h"
#include "reg_ble_em_rx_desc.h"
#include "reg_ble_em_cs.h"
#include "llc.h"
#if (BLE_PERIPHERAL || BLE_CENTRAL)
/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Gets a free connection handle.
 *
 * This function allocates a new connection handle if possible.
 *
 * @param[in/out] conhdl         Pointer on the connection handle allocated.
 *
 * @return If the connection handle has been correctly allocated.
 *
 ****************************************************************************************
 */
uint8_t llc_util_get_free_conhdl(uint16_t *conhdl);


/**
 ****************************************************************************************
 * @brief Process the disconnection
 *
 * This function request to the Link Layer to stop sending any further packets, frees the
 * LLC conhdl and LLC task associated, notifies the host of the loss of connection.
 *
 * @param[in] conhdl         Connection handle disconnected.
 * @param[in] reason         reason of the disconnection.
 *
 ****************************************************************************************
 */
void llc_util_dicon_procedure(uint16_t conhdl, uint8_t reason);

/**
 ****************************************************************************************
 * @brief generates the  SKDm or SKDs
 *
 * The SKDx is generated from the Random Number and Init Vectors passed as parameter
 *
 * @param[out] skdx     Pointer to the SDKx to be generated
 * @param[in]  nb       Pointer to the Random Number
 * @param[in]  ivx      Pointer to the Init Vector
 *
 ****************************************************************************************
 */
void llc_util_gen_skdx(struct sess_k_div_x *skdx,
                       struct rand_nb const *nb,
                       struct init_vect  const *ivx);
#endif //(BLE_PERIPHERAL || BLE_CENTRAL)
/**
 ****************************************************************************************
 * @brief Gets the size of the packet received.
 *
 * This function gets the length of the packet received.
 *
 * @param[in] rxdesc         Pointer on the reception descriptor.
 *
 * @return The length of the data.
 *
 ****************************************************************************************
 */
__STATIC_INLINE uint8_t llc_util_rxlen_getf(struct co_buf_rx_desc *rxdesc)
{
    uint16_t localVal =  rxdesc->rxheader;
    return ((localVal & BLE_RXLEN_MASK) >> BLE_RXLEN_LSB);
}

/**
 ****************************************************************************************
 * @brief Gets the logical link identifier of the packet received.
 *
 * This function gets the LLID of the packet received.
 *
 * @param[in] rxdesc         Pointer on the reception descriptor.
 *
 * @return The LLID for the packet received.
 *
 ****************************************************************************************
 */
__STATIC_INLINE uint8_t llc_util_rxllid_getf(struct co_buf_rx_desc *rxdesc)
{
    uint16_t localVal =  rxdesc->rxheader;
    return ((localVal & BLE_RXLLID_MASK) >> BLE_RXLLID_LSB);
}


/**
 ****************************************************************************************
 * @brief Gets the RSSI of the packet received.
 *
 * This function gets the RSSI of the packet received.
 *
 * @param[in] rxdesc         Pointer on the reception descriptor.
 *
 * @return The RSSI for the packet received.
 *
 ****************************************************************************************
 */
__STATIC_INLINE uint8_t llc_util_rxrssi_getf(struct co_buf_rx_desc *rxdesc)
{
    uint16_t localVal = rxdesc->rxchass;
    return ((localVal & BLE_RSSI_MASK) >> BLE_RSSI_LSB);
}

/**
 ****************************************************************************************
 * @brief  Gets the channel used for the reception of this descriptor.
 *
 * This function gets the channel used for the reception of this descriptor
 *
 * @param[in] rxdesc         Pointer on the reception descriptor.
 *
 * @return The channel used to receive the packet.
 *
 ****************************************************************************************
 */
__STATIC_INLINE uint8_t llc_util_used_ch_idx_getf(struct co_buf_rx_desc *rxdesc)
{
    uint16_t localVal = rxdesc->rxchass;
    return ((localVal & BLE_USED_CH_IDX_MASK) >> BLE_USED_CH_IDX_LSB);
}

/**
 ****************************************************************************************
 * @brief  Gets the error on the reception.
 *
 * This function gets the error field for the reception of this descriptor
 *
 * @param[in] rxdesc         Pointer on the reception descriptor.
 *
 * @return The error field.
 *
 ****************************************************************************************
 */
__STATIC_INLINE uint16_t llc_util_rxerr_getf(struct co_buf_rx_desc *rxdesc)
{
    uint16_t localVal =  rxdesc->rxstatus;
    return ((localVal & (BLE_RXTIMEERR_BIT | BLE_NESN_ERR_BIT | BLE_SN_ERR_BIT | BLE_MIC_ERR_BIT
                       | BLE_CRC_ERR_BIT | BLE_LEN_ERR_BIT | BLE_TYPE_ERR_BIT | BLE_SYNC_ERR_BIT)) >> 0);
}

/**
 ****************************************************************************************
 * @brief  Gets the link identifier of the reception.
 *
 * This function gets the link identifier for the reception of this descriptor
 *
 * @param[in] rxdesc         Pointer on the reception descriptor.
 *
 * @return The link value.
 *
 ****************************************************************************************
 */
__STATIC_INLINE uint8_t llc_util_rxlink_getf(struct co_buf_rx_desc *rxdesc)
{
    uint16_t localVal =  rxdesc->rxstatus;
    return ((localVal & BLE_RXLINKLBL_MASK) >> BLE_RXLINKLBL_LSB);
}
#if (BLE_PERIPHERAL || BLE_CENTRAL)
/**
 ****************************************************************************************
 * @brief  Checks the number of active(s) link(s).
 *
 * This function returns the number of active(s) link(s).
 *
 * @return The number of active(s) link(s).
 *
 ****************************************************************************************
 */
__STATIC_INLINE uint8_t llc_util_get_active_conhdl(void)
{
    uint8_t idx;
    uint8_t cpt_active=0;
    ke_task_id_t llc_free;
    for(idx = 0; idx < BLE_CONNECTION_MAX; idx++)
    {
        // build a task ID dedicated to the conhdl
        llc_free = KE_BUILD_ID(TASK_LLC, idx);
        // gets the state
        if(ke_state_get(llc_free)!= LLC_FREE)
        {
            cpt_active++;
        }
    }
    return cpt_active;
}

/**
 ****************************************************************************************
 * @brief  Checks if the meta event is enabled.
 *
 * This function checks if the meta event is authorized to be sent to the host.
 *
 * @param[in] meta_event         Event to be checked.
 *
 * @return The is the event is authorized.
 *
 ****************************************************************************************
 */
__STATIC_INLINE bool llc_util_event_enabled(uint8_t meta_event)
{
    uint8_t idx;
    uint8_t cpt_active=0;
    ke_task_id_t llc_free;
    for(idx = 0; idx < BLE_CONNECTION_MAX; idx++)
    {
        // build a task ID dedicated to the conhdl
        llc_free = KE_BUILD_ID(TASK_LLC, idx);
        // gets the state
        if(ke_state_get(llc_free)!= LLC_FREE)
        {
            cpt_active++;
        }
    }
    return cpt_active;
}

/**
 ****************************************************************************************
 * @brief  Gets the L2CAP length.
 *
 * This function gets in the data buffer the L2CAP length.
 *
 * @param[in] data         Pointer on the data buffer .
 *
 * @return The L2CAP data length.
 *
 ****************************************************************************************
 */
__STATIC_INLINE uint16_t llc_util_rxl2clen_getf(uint8_t *data)
{
    uint16_t localVal =  *data++ & 0x00FF; //gets the lsb first
    localVal |= (uint16_t) (*data << 8) & 0xFF00;
    return (localVal);
}

/**
 ****************************************************************************************
 * @brief  Set the IVm in the control structure
 *
 * @param[in] conhdl    Handle of the connection for which the IVm is set
 * @param[in] ivm       Pointer to the IVm to set for this connection
 *
 ****************************************************************************************
 */
__STATIC_INLINE void llc_util_ivm_set(uint16_t conhdl, uint8_t const *ivm)
{
    for (int i = 0; i < 2; i++)
    {
        // Sets the lower part of the IV with the IVm
        ble_iv_setf(conhdl, i , co_read16p(&ivm[2*i]));
    }
}

/**
 ****************************************************************************************
 * @brief  Set the IVs in the control structure
 *
 * @param[in] conhdl    Handle of the connection for which the IVs is set
 * @param[in] ivs       Pointer to the IVs to set for this connection
 *
 ****************************************************************************************
 */
__STATIC_INLINE void llc_util_ivs_set(uint16_t conhdl, uint8_t const *ivs)
{
    for (int i = 0; i < 2; i++)
    {
        // Sets the upper part of the IV with the IVs
        ble_iv_setf(conhdl, i + 2 , co_read16p(&ivs[2*i]));
    }
}

/**
 ****************************************************************************************
 * @brief  Update the channel map of a specific link
 *
 * @param[in] conhdl    Handle of the connection for which the IVs is set
 * @param[in] map       Pointer to the channel map
 ****************************************************************************************
 */
void llc_util_update_channel_map(uint16_t conhdl, struct le_chnl_map *map);

/**
 ****************************************************************************************
 * @brief  Enable/disable LLCP discard
 *
 * @param[in] conhdl    Connection handle
 ****************************************************************************************
 */
void llc_util_set_llcp_discard_enable(uint16_t conhdl, bool enable);

/**
 ****************************************************************************************
 * @brief  Calculates and sets an appropriate margin for the authenticated payload timeout
 *
 * @param[in] conhdl    Handle of the connection for which the IVs is set
 * @param[in] enable    True/false
 ****************************************************************************************
 */
void llc_util_set_auth_payl_to_margin(struct lld_evt_tag *evt);

/**
 ****************************************************************************************
 * @brief  Check disc command param
 *
 * @param[in] reason    Reason for disconnection
 *
 * @return reason accepted or rejected
 ****************************************************************************************
 */
__STATIC_INLINE bool llc_util_disc_reason_ok(uint8_t reason)
{
    // disconnection reason range
    uint8_t valid_r[7] = {
            CO_ERROR_AUTH_FAILURE,
            CO_ERROR_REMOTE_USER_TERM_CON,
            CO_ERROR_REMOTE_DEV_TERM_LOW_RESOURCES,
            CO_ERROR_REMOTE_DEV_POWER_OFF,
            CO_ERROR_UNSUPPORTED_REMOTE_FEATURE,
            CO_ERROR_PAIRING_WITH_UNIT_KEY_NOT_SUP,
            CO_ERROR_UNACCEPTABLE_CONN_INT
    };

    for (uint8_t i = 0; i < 7; i++)
        if (reason == valid_r[i]) return (true);

    return (false);
}

/**
 ****************************************************************************************
 * @brief Set operation pointer
 *
 * @param[in] op_type       Operation type.
 * @param[in] op            Operation pointer.
 *
 ****************************************************************************************
 */
__STATIC_INLINE void llc_util_set_operation_ptr(uint16_t conhdl, uint8_t op_type, void* op)
{
    ASSERT_ERR(op_type < LLC_OP_MAX);
    struct llc_env_tag *llc_env_ptr = llc_env[conhdl];
    // update operation pointer
    llc_env_ptr->operation[op_type] = op;
}

/**
 ****************************************************************************************
 * @brief Get operation pointer
 *
 * @param[in] op_type       Operation type.
 *
 * @return operation pointer on going
 ****************************************************************************************
 */
__STATIC_INLINE void* llc_util_get_operation_ptr(uint16_t conhdl, uint8_t op_type)
{
    ASSERT_ERR(op_type < LLC_OP_MAX);
    struct llc_env_tag *llc_env_ptr = llc_env[conhdl];
    // return operation pointer
    return llc_env_ptr->operation[op_type];
}
#endif // #if (BLE_PERIPHERAL || BLE_CENTRAL)
/// @} LLCUTIL
#endif // LLC_UTIL_H_
