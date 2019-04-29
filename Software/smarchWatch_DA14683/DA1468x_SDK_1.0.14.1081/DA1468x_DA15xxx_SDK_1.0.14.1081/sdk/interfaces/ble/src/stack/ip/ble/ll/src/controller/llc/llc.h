/**
 ****************************************************************************************
 *
 * @file llc.h
 *
 * @brief Main API file for the Link Layer controller
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 ****************************************************************************************
 */

#ifndef LLC_H_
#define LLC_H_

/**
 ****************************************************************************************
 * @addtogroup LLC LLC
 * @ingroup CONTROLLER
 * @brief Link Layer Controller
 *
 * Declaration of the functions used by the logical link controller
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "co_bt.h"
#include "co_error.h"
#include "co_version.h"
#include "llc_task.h"
#include "llc_cntl.h"
#include "llm.h"
#include "lld_evt.h"
#include "ke_task.h"

#if (BLE_PERIPHERAL || BLE_CENTRAL)

/*
 * DEFINES
 ****************************************************************************************
 */

/*****************************************************************************************
 * LLC Flags (Status)
 ****************************************************************************************/
/// Flag indicating whether features have been exchanged or not
#define LLC_FLAG_FEAT_EXCH          (0x01)
/// Flag indicating if the peer version is already known
#define LLC_FLAG_PEER_VERS_KNOWN    (LLC_FLAG_FEAT_EXCH << 1)
/// Flag indicating if the Read Remote Version procedure was interrupted by an encryption procedure
#define LLC_FLAG_VERS_IND_RESTART   (LLC_FLAG_PEER_VERS_KNOWN << 1)
/// Flag indicating if LLC messages received have to be discarded
#define LLC_FLAG_LLCP_DISCARD       (LLC_FLAG_VERS_IND_RESTART << 1)
/// Connection TO pending status
#define LLC_FLAG_TO_PENDING         (LLC_FLAG_LLCP_DISCARD << 1)
/// Connection update ongoing flag
#define LLC_FLAG_UPDATE_PENDING     (LLC_FLAG_TO_PENDING << 1)
/// Connection update requested by host
#define LLC_FLAG_UPDATE_HOST_REQ    (LLC_FLAG_UPDATE_PENDING << 1)
/// Connection update event should be sent
#define LLC_FLAG_UPDATE_EVT_SENT    (LLC_FLAG_UPDATE_HOST_REQ << 1)
/// Synchronization found
#define LLC_FLAG_SYNC_FOUND         (LLC_FLAG_UPDATE_EVT_SENT << 1)
#if (RWBLE_SW_VERSION_MAJOR >= 8)
/// LE Length skip
#define LLC_FLAG_LE_LENGTH_SKIP     (LLC_FLAG_SYNC_FOUND << 1)
/// LE Length req pending
#define LLC_FLAG_LE_LENGTH_REQ_PEND (LLC_FLAG_LE_LENGTH_SKIP << 1)
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */

/*
 * MACROS
 ****************************************************************************************
 */

/// Set LLC status flag
#define LLC_FLAG_SET(conhdl, flag) \
                (llc_env[conhdl]->llc_status |= (LLC_FLAG_ ## flag))
/// Reset LLC status flag
#define LLC_FLAG_RESET(conhdl, flag) \
                (llc_env[conhdl]->llc_status &= (~(LLC_FLAG_ ## flag)))
/// Get LLC status flag
#define LLC_FLAG_GET(conhdl, flag) \
                (llc_env[conhdl]->llc_status & (LLC_FLAG_ ## flag))

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Remote version information structure
struct rem_version
{
    /// LMP version
    uint8_t vers;
    /// Manufacturer ID
    uint16_t compid;
    /// LMP subversion
    uint16_t subvers;
};

/// Encryption structure
struct encrypt
{
    /// Session key diversifier
    struct sess_k_div   skd;
    /// Long term key
    struct ltk          ltk;
};

/// LLC environment structure
struct llc_env_tag
{
    /// Request operation Kernel message
    void* operation[LLC_OP_MAX];
    /// Pointer to the associated @ref LLD event
    struct ea_elt_tag *elt;
    /// Peer version obtained using the LL_VERSION_IND LLCP message
    struct rem_version  peer_version;

    /// Link supervision time out
    uint16_t            sup_to;
    /// New link supervision time out to be applied
    uint16_t            n_sup_to;
    /// Authenticated payload time out (expressed in units of 10 ms)
    uint16_t            auth_payl_to;
    /// Authenticated payload time out margin (expressed in units of 10 ms)
    uint16_t            auth_payl_to_margin;
    /// Variable to save the previous state
    ke_task_id_t        previous_state;
    /// LLC status
    uint16_t            llc_status;
    ///Current channel map
    struct le_chnl_map  ch_map;
    ///New channel map - Will be applied at instant when a channel map update is pending
    struct le_chnl_map  n_ch_map;
    /// Received signal strength indication
    int8_t              rssi;
    /// Features used by the stack
    struct le_features  feats_used;
    /// Encryption state
    uint8_t             enc_state;
    /// Structure dedicated for the encryption
    struct encrypt      encrypt;
    /// Transmit packet counter
    uint8_t             tx_pkt_cnt;
    /// Disconnection reason
    uint8_t             disc_reason;

    /// rx status
    uint8_t             rx_status;
    /// feature request received first check
    bool                first_check;

    #if (BLE_CHNL_ASSESS)
    /// Channel Assessment - Number of packets received on each channel
    uint8_t            chnl_assess_pkt_cnt[LE_DATA_FREQ_LEN];
    /**
     * Channel Assessment - Number of packets received with a RSSI greater than the min
     * RSSI threshold and without found synchronization on each channel
     */
    uint8_t            chnl_assess_bad_pkt_cnt[LE_DATA_FREQ_LEN];
    #endif //(BLE_CHNL_ASSESS)
    #if (BLE_TESTER)
    struct hci_tester_set_le_params_cmd tester_params;
    #endif
#if (RWBLE_SW_VERSION_MAJOR >= 8)
    uint16_t            connMaxTxOctets;
    uint16_t            connMaxRxOctets;
    uint16_t            connRemoteMaxTxOctets;
    uint16_t            connRemoteMaxRxOctets;
    uint16_t            connEffectiveMaxTxOctets;
    uint16_t            connEffectiveMaxRxOctets;
    uint16_t            connMaxTxTime;
    uint16_t            connMaxRxTime;
    uint16_t            connRemoteMaxTxTime;
    uint16_t            connRemoteMaxRxTime;
    uint16_t            connEffectiveMaxTxTime;
    uint16_t            connEffectiveMaxRxTime;
    //our value to take time converted to octets into account
    uint16_t            connEffectiveMaxTxOctets_Time;
    /// length request received 
    bool                llcp_length_req_first_check;
    /// length response received and queues
    bool                llcp_length_rsp_queued;
#if (RWBLE_SW_VERSION_MINOR >= 1)
    /// packet counter free running
    uint32_t pkt_cnt_tot;
    /// bad packet counter free running
    uint32_t pkt_cnt_bad_tot;
    /// packet counter temporary for operations
    uint32_t pkt_cnt;
    /// bad packet counter temporary for operations
    uint32_t pkt_cnt_bad;
#endif /* (RWBLE_SW_VERSION_MINOR >= 1) */
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */
};

/*
 * DEFINES
 ****************************************************************************************
 */
/// Default values
#define LLC_DFT_INTERV          3200
#define LLC_DFT_LATENCY         500
#define LLC_DFT_SUP_TO          3200
#define LLC_DFT_RSP_TO          4000
#define LLC_DFT_AUTH_PAYL_TO    3000 // expressed in units of 10 ms, so 30 seconds
#define LLC_DFT_CE_LEN          65535
#define LLC_DFT_WIN_OFF         0
#define LLC_DFT_WIN_SIZE        8
#define LLC_DFT_HOP_INC         1
#define LLC_DFT_INST_MAP_UPD    6

/// Connection interval min (N*1.250ms)
#define LLC_CNX_INTERVAL_MIN            6       //(0x06)
/// Connection interval Max (N*1.250ms)
#define LLC_CNX_INTERVAL_MAX            3200    //(0xC80)
/// Connection latency min (N*cnx evt)
#define LLC_CNX_LATENCY_MIN             0       //(0x00)
/// Connection latency Max (N*cnx evt
#define LLC_CNX_LATENCY_MAX             500     //(0x1F4)
/// Supervision TO min (N*10ms)
#define LLC_CNX_SUP_TO_MIN              10      //(0x0A)
/// Supervision TO Max (N*10ms)
#define LLC_CNX_SUP_TO_MAX              3200    //(0xC80)
/// Connection event length min (N*0.625ms)
#define LLC_CNX_CE_LGTH_MIN             0       //(0x00)
/// Connection event length  Max (N*0.625ms)
#define LLC_CNX_CE_LGTH_MAX             65535   //(0xFFFF)

/// Base index dedicated for the control packet
#define LLC_LE_CNTL_PKT_BASE_IDX         (BLE_TX_DESC_CNT - BLE_TX_DESC_CNTL - BLE_TX_DESC_ADV)

// Encryption state
/// Encryption enabled in TX
#define ENC_TX                          CO_BIT(0)
/// Encryption enabled in RX
#define ENC_RX                          CO_BIT(1)
/// Encryption key refresh procedure is pending
#define ENC_REFRESH_PENDING             CO_BIT(2)
/// An encryption procedure is ongoing, only specific TX flow is now allowed
#define ENC_TX_FLOW_CONTROLLED          CO_BIT(3)
/// An encryption procedure is ongoing, only specific RX flow is now allowed
#define ENC_RX_FLOW_CONTROLLED          CO_BIT(4)
/// Encryption is enabled
#define ENC_ENABLED                     (ENC_TX | ENC_RX)

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */
extern struct llc_env_tag* llc_env[BLE_CONNECTION_MAX];

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Initialization of the BLE LLC task
 *
 * This function initializes the LLC task, as well as assessment statistics
 *
 ****************************************************************************************
 */
void llc_init(void);

/**
 ****************************************************************************************
 * @brief Reset of the BLE LLC task
 *
 * This function reset the LLC task
 *
 ****************************************************************************************
 */
void llc_reset(void);

/**
 ****************************************************************************************
 * @brief Start the BLE LLC task
 *
 * This function set the state of the task, the initiating link supervision time out, the
 * feature used as well as the environment of the LLC and send the connection completed
 * event
 *
 * @param[in] param         Pointer on the structure which contains all the parameters
 *                          needed to create and maintain the link.
 * @param[in] conhdl        Connection handle on which the connection is created
 * @param[in] evt           Pointer to the event associated with this connection
 *
 ****************************************************************************************
 */
void llc_start(struct llc_create_con_req_ind const *param, struct ea_elt_tag *elt);
/**
 ****************************************************************************************
 * @brief Stop the BLE LLC task
 *
 * This function clears the state of the task, the environment of the LLC and send the connection completed
 * event
 *
 * @param[in] conhdl        Connection handle on which the connection is created
 ****************************************************************************************
 */
void llc_stop(uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Sends the disconnection complete event
 *
 * This function notify the host that the disconnection happened
 *
 * @param[in] src_id        Source of the disconnection
 * @param[in] status        Status on the completion of the disconnection
 * @param[in] conhdl        Connection handle on which the disconnection happened
 * @param[in] reason        Why the disconnection happened
 ****************************************************************************************
 */
void llc_discon_event_complete_send(ke_task_id_t src_id, uint8_t status, uint8_t conhdl, uint8_t reason);

/**
 ****************************************************************************************
 * @brief Sends the LE connection complete event
 *
 * This function notify the host that the connection happened
 *
 * @param[in] status        Status on the completion of the connection
 * @param[in] conhdl        Connection handle on which the connection happened
 * @param[in] param         Pointer on the structure which contains all the parameters
 *                          needed to create and maintain the link.
 ****************************************************************************************
 */
void llc_le_con_cmp_evt_send(uint8_t status, uint16_t conhdl, struct llc_create_con_req_ind const *param);

/**
 ****************************************************************************************
 * @brief Sends the update connection complete event.
 *
 * This function notify the host that the update of the connection's parameters happened.
 *
 * @param[in] status        Status on the completion of the update connection.
 * @param[in] conhdl        Connection handle on which the update connection happened.
 * @param[in] evt           Pointer to the event structure linked to the connection.
 ****************************************************************************************
 */
void llc_con_update_complete_send(uint8_t status, uint16_t conhdl, struct lld_evt_tag *evt);

/**
 ****************************************************************************************
 * @brief Sends the command complete event.
 *
 * This function notify the host that the command is completed.
 *
 * @param[in] opcode        Command opcode
 * @param[in] status        Status on the completion of the command.
 * @param[in] conhdl        Connection handle on which the command has been processed.
 ****************************************************************************************
 */
void llc_common_cmd_complete_send(uint16_t opcode, uint8_t status, uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Sends the command status event.
 *
 * This function notify the host that the command is understood.
 *
 * @param[in] opcode        Command opcode
 * @param[in] status        Status on the understanding of the command.
 * @param[in] conhdl        Connection handle on which the command has been processed.
 ****************************************************************************************
 */
void llc_common_cmd_status_send(uint16_t opcode, uint8_t status, uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Sends the number of completed packet event.
 *
 * This function notify the host of the number of packets acknowledged
 *
 * @param[in] conhdl        Connection handle on which the packet has been acknowledged
 * @param[in] nb_of_pkt     Number of acknowledged packets
 ****************************************************************************************
 */
void llc_common_nb_of_pkt_comp_evt_send(uint16_t conhdl, uint8_t nb_of_pkt);

/**
 ****************************************************************************************
 * @brief Sends the read remote used features meta-event.
 *
 * @param[in] status        Status of the event
 * @param[in] conhdl        Connection handle on which the remote features have been read
 * @param[in] feats         Read remote features
 *
 ****************************************************************************************
 */
void llc_feats_rd_event_send(uint8_t status,
                              uint16_t conhdl,
                              struct le_features const *feats);

/**
 ****************************************************************************************
 * @brief Sends the remote version indication event.
 *
 * @param[in] status        Status of the event
 * @param[in] conhdl        Connection handle on which the remote version have been read
 *
 ****************************************************************************************
 */
void llc_version_rd_event_send(uint8_t status, uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Sends the encryption change event.
 *
 * This function notify the host of the new encryption status.
 *
 * @param[in] conhdl        Connection handle on which the status of the encryption has
 *                          been changed
 * @param[in] enc_status    Status of the encryption (ON or OFF)
 * @param[in] status        Inform if the change is successfully done or not.
 ****************************************************************************************
 */
void llc_common_enc_change_evt_send(uint16_t conhdl, uint8_t enc_status, uint8_t status);

/**
 ****************************************************************************************
 * @brief Sends the flush occurred event.
 *
 * This function notify the host that a flush of packet occurred.
 *
 * @param[in] conhdl        Connection handle on which the flush occurred.
 ****************************************************************************************
 */
void llc_common_flush_occurred_send(uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Sends the encryption key refresh completed event.
 *
 * This function notify the host that the resume is done for the encryption.
 *
 * @param[in] conhdl        Connection handle on which the key has been refreshed.
 ****************************************************************************************
 */
void llc_common_enc_key_ref_comp_evt_send(uint16_t conhdl, uint8_t status);

/**
 ****************************************************************************************
 * @brief Sends the long term key request.
 *
 * This function request to the host for a LTK.
 *
 * @param[in] conhdl        Connection handle on which the LTK is requested.
 * @param[in] param         Pointer to the parameters of the LL_ENC_REQ
 ****************************************************************************************
 */
void llc_ltk_req_send(uint16_t conhdl, struct llcp_enc_req const *param);

/**
 ****************************************************************************************
 * @brief Indicates that the parameter update has occurred
 *
 * @param[in] conhdl        Connection handle on which the update occurred.
 * @param[in] evt_new       Pointer to the new LLD event that is used for this connection.
 ****************************************************************************************
 */
void llc_con_update_ind(uint16_t conhdl, struct ea_elt_tag *elt_new);

/**
 ****************************************************************************************
 * @brief Indicates that the channel map update should be done.
 *
 * @param[in] conhdl        Connection handle on which the update occurred.
 ****************************************************************************************
 */
void llc_map_update_ind(uint16_t conhdl);

#if (BLE_CENTRAL)
/**
 ****************************************************************************************
 * @brief Compute the instant when the LL_CHANNEL_MAP_REQ PDU will be sent and program it
 *
 * @param[in] conhdl        Connection handle
 ****************************************************************************************
 */
void llc_chnl_map_req_send(uint8_t conhdl);

#if (BLE_CHNL_ASSESS)
/**
 ****************************************************************************************
 * @brief Randomly add a channel that had been previously removed (e.g.: by the channel
 * assessment mechanism) based on the channel classification set by the host.
 *
 * @param[in] conhdl        Connection handle
 * @param[in] nb_chnl       Number of channels to add
 ****************************************************************************************
 */
void llc_add_bad_chnl(uint8_t conhdl, uint8_t nb_chnl);
#endif //(BLE_CHNL_ASSESS)
#endif //(BLE_CENTRAL)

/**
 ****************************************************************************************
 * @brief Before the connection update instant used the greater LSTO
 *
 * @param[in] conhdl        Connection handle
 ****************************************************************************************
 */
void llc_lsto_con_update(uint16_t conhdl);
#endif // #if (BLE_PERIPHERAL || BLE_CENTRAL)
/// @} LLC

#if (RWBLE_SW_VERSION_MAJOR >= 8)
bool llc_le_length_effective(uint16_t conhdl);
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */

#endif // LLC_H_
