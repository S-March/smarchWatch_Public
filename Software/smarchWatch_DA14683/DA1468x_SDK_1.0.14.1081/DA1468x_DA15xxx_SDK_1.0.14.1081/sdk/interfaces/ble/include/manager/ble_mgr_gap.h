/**
 \addtogroup INTERFACES
 \{
 \addtogroup BLE
 \{
 \addtogroup MANAGER
 \{
 */

/**
 ****************************************************************************************
 *
 * @file ble_mgr_gap.h
 *
 * @brief BLE manager definitions and handlers for GAP
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BLE_MGR_GAP_H_
#define BLE_MGR_GAP_H_

#include <stdint.h>
#include <stdbool.h>
#include "osal.h"
#include "ble_mgr_cmd.h"
#include "ble_gap.h"

/** OP codes for GAP commands */
enum ble_cmd_gap_opcode {
        BLE_MGR_GAP_ADDRESS_SET_CMD  = BLE_MGR_CMD_CAT_FIRST(BLE_MGR_GAP_CMD_CAT),
        BLE_MGR_GAP_DEVICE_NAME_SET_CMD,
        BLE_MGR_GAP_APPEARANCE_SET_CMD,
        BLE_MGR_GAP_PPCP_SET_CMD,
        BLE_MGR_GAP_ADV_START_CMD,
        BLE_MGR_GAP_ADV_STOP_CMD,
        BLE_MGR_GAP_ADV_DATA_SET_CMD,
        BLE_MGR_GAP_SCAN_START_CMD,
        BLE_MGR_GAP_SCAN_STOP_CMD,
        BLE_MGR_GAP_CONNECT_CMD,
        BLE_MGR_GAP_CONNECT_CANCEL_CMD,
        BLE_MGR_GAP_DISCONNECT_CMD,
        BLE_MGR_GAP_CONN_RSSI_GET_CMD,
        BLE_MGR_GAP_ROLE_SET_CMD,
        BLE_MGR_GAP_MTU_SIZE_SET_CMD,
        BLE_MGR_GAP_CHANNEL_MAP_SET_CMD,
        BLE_MGR_GAP_CONN_PARAM_UPDATE_CMD,
        BLE_MGR_GAP_CONN_PARAM_UPDATE_REPLY_CMD,
        BLE_MGR_GAP_PAIR_CMD,
        BLE_MGR_GAP_PAIR_REPLY_CMD,
        BLE_MGR_GAP_PASSKEY_REPLY_CMD,
        BLE_MGR_GAP_UNPAIR_CMD,
        BLE_MGR_GAP_SET_SEC_LEVEL_CMD,
#if (dg_configBLE_SKIP_LATENCY_API == 1)
        BLE_MGR_GAP_SKIP_LATENCY_CMD,
#endif /* (dg_configBLE_SKIP_LATENCY_API == 1) */
        BLE_MGR_GAP_DATA_LENGTH_SET_CMD,
#if (dg_configBLE_SECURE_CONNECTIONS == 1)
        BLE_MGR_GAP_NUMERIC_REPLY_CMD,
#endif /* (dg_configBLE_SECURE_CONNECTIONS == 1) */
        BLE_MGR_GAP_ADDRESS_RESOLVE_CMD,
        /* Dummy command opcode, needs to be always defined after all commands */
        BLE_MGR_GAP_LAST_CMD,
};

/** GAP address set command message structure */
typedef struct {
        ble_mgr_msg_hdr_t    hdr;
        const own_address_t  *address;
        uint16_t             renew_dur;
} ble_mgr_gap_address_set_cmd_t;

/** GAP address set response message structure */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gap_address_set_rsp_t;

void ble_mgr_gap_address_set_cmd_handler(void *param);

/** GAP device name set command message structure */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        const char          *name;
        att_perm_t          perm;
} ble_mgr_gap_device_name_set_cmd_t;

/** GAP device name set response message structure */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gap_device_name_set_rsp_t;

void ble_mgr_gap_device_name_set_cmd_handler(void *param);

/** GAP appearance set command message structure */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        gap_appearance_t    appearance;
        att_perm_t          perm;
} ble_mgr_gap_appearance_set_cmd_t;

/** GAP appearance set response message structure */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gap_appearance_set_rsp_t;

void ble_mgr_gap_appearance_set_cmd_handler(void *param);

/** GAP peripheral preferred connection parameters set command message structure */
typedef struct {
        ble_mgr_msg_hdr_t        hdr;
        const gap_conn_params_t  *gap_ppcp;
} ble_mgr_gap_ppcp_set_cmd_t;

/** GAP peripheral preferred connection parameters set response message structure */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gap_ppcp_set_rsp_t;

void ble_mgr_gap_ppcp_set_cmd_handler(void *param);

/** GAP advertising start command message structure */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        gap_conn_mode_t     adv_type;
} ble_mgr_gap_adv_start_cmd_t;

/** GAP advertising start response message structure */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ad_ble_status_t     status;
} ble_mgr_gap_adv_start_rsp_t;

void ble_mgr_gap_adv_start_cmd_handler(void *param);

/** GAP advertising stop command message structure */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
} ble_mgr_gap_adv_stop_cmd_t;

/** GAP advertising stop response message structure */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ad_ble_status_t     status;
} ble_mgr_gap_adv_stop_rsp_t;

void ble_mgr_gap_adv_stop_cmd_handler(void *param);

/** GAP set advertising data command message structure */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint8_t             adv_data_len;
        const uint8_t       *adv_data;
        uint8_t             scan_rsp_data_len;
        const uint8_t       *scan_rsp_data;
} ble_mgr_gap_adv_data_set_cmd_t;

/** GAP set advertising data response message structure */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gap_adv_data_set_rsp_t;

void ble_mgr_gap_adv_data_set_cmd_handler(void *param);

/** GAP scan start command message structure */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        gap_scan_type_t     type;
        gap_scan_mode_t     mode;
        uint16_t            interval;
        uint16_t            window;
        bool                filt_wlist;
        bool                filt_dupl;
} ble_mgr_gap_scan_start_cmd_t;

/** GAP scan start response message structure */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gap_scan_start_rsp_t;

void ble_mgr_gap_scan_start_cmd_handler(void *param);

/** GAP scan stop command message structure */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
} ble_mgr_gap_scan_stop_cmd_t;

/** GAP scan stop response message structure */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gap_scan_stop_rsp_t;

void ble_mgr_gap_scan_stop_cmd_handler(void *param);

/** GAP connect command message structure */
typedef struct {
        ble_mgr_msg_hdr_t       hdr;
        const bd_address_t      *peer_addr;
        const gap_conn_params_t *conn_params;
        uint16_t                ce_len_min;
        uint16_t                ce_len_max;
} ble_mgr_gap_connect_cmd_t;

/** GAP connect response message structure */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gap_connect_rsp_t;

void ble_mgr_gap_connect_cmd_handler(void *param);

/** GAP connect cancel command message structure */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
} ble_mgr_gap_connect_cancel_cmd_t;

/** GAP connect cancel response message structure */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gap_connect_cancel_rsp_t;

void ble_mgr_gap_connect_cancel_cmd_handler(void *param);

/** GAP disconnect command message structure */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        ble_hci_error_t     reason;
} ble_mgr_gap_disconnect_cmd_t;

/** GAP disconnect response message structure */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gap_disconnect_rsp_t;

void ble_mgr_gap_disconnect_cmd_handler(void *param);

/** GAP get connection RSSI command message structure */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
} ble_mgr_gap_conn_rssi_get_cmd_t;

/** GAP get connection RSSI response message structure */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        int8_t              conn_rssi;
        ble_error_t         status;
} ble_mgr_gap_conn_rssi_get_rsp_t;

void ble_mgr_gap_conn_rssi_get_cmd_handler(void *param);

/** GAP set role command message structure */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        gap_role_t          role;
} ble_mgr_gap_role_set_cmd_t;

/** GAP set role response message structure */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        gap_role_t          new_role;
        gap_role_t          previous_role;
        ble_error_t         status;
} ble_mgr_gap_role_set_rsp_t;

void ble_mgr_gap_role_set_cmd_handler(void *param);

/** GAP set MTU size command message structure */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            mtu_size;
} ble_mgr_gap_mtu_size_set_cmd_t;

/** GAP set MTU size response message structure */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            new_mtu_size;
        uint16_t            previous_mtu_size;
        ble_error_t         status;
} ble_mgr_gap_mtu_size_set_rsp_t;

void ble_mgr_gap_mtu_size_set_cmd_handler(void *param);

/** GAP set channel map command message structure */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        const uint64_t      *chnl_map;
} ble_mgr_gap_channel_map_set_cmd_t;

/** GAP set channel map response message structure */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gap_channel_map_set_rsp_t;

void ble_mgr_gap_channel_map_set_cmd_handler(void *param);

/** GAP connection parameter update command message */
typedef struct {
        ble_mgr_msg_hdr_t        hdr;
        uint16_t                 conn_idx;
        const gap_conn_params_t  *conn_params;
} ble_mgr_gap_conn_param_update_cmd_t;

/** GAP connection parameter update response message */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gap_conn_param_update_rsp_t;

void ble_mgr_gap_conn_param_update_cmd_handler(void *param);

/** GAP connection parameter update reply command message */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        bool                accept;
} ble_mgr_gap_conn_param_update_reply_cmd_t;

/** GAP connection parameter update reply response message */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gap_conn_param_update_reply_rsp_t;

void ble_mgr_gap_conn_param_update_reply_cmd_handler(void *param);

/** GAP pair command message */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        bool                bond;
} ble_mgr_gap_pair_cmd_t;

/** GAP pair response message */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gap_pair_rsp_t;

void ble_mgr_gap_pair_cmd_handler(void *param);

/** GAP pair reply command message */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        bool                accept;
        bool                bond;
} ble_mgr_gap_pair_reply_cmd_t;

/** GAP pair reply response message */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gap_pair_reply_rsp_t;

void ble_mgr_gap_pair_reply_cmd_handler(void *param);

/** GAP passkey reply command message */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        bool                accept;
        uint32_t            passkey;
} ble_mgr_gap_passkey_reply_cmd_t;

/** GAP passkey reply response message */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gap_passkey_reply_rsp_t;

void ble_mgr_gap_passkey_reply_cmd_handler(void *param);

#if (dg_configBLE_SECURE_CONNECTIONS == 1)
/** GAP numeric comparison reply command message */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        bool                accept;
} ble_mgr_gap_numeric_reply_cmd_t;

/** GAP numeric comparison reply response message */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gap_numeric_reply_rsp_t;

void ble_mgr_gap_numeric_reply_cmd_handler(void *param);
#endif /* (dg_configBLE_SECURE_CONNECTIONS == 1) */

/** GAP unpair command message */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        bd_address_t        addr;
} ble_mgr_gap_unpair_cmd_t;

/** GAP unpair response message */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gap_unpair_rsp_t;

void ble_mgr_gap_unpair_cmd_handler(void *param);

/** GAP get security level command message */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        gap_sec_level_t     level;
} ble_mgr_gap_set_sec_level_cmd_t;

/** GAP get security level response message */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gap_set_sec_level_rsp_t;

void ble_mgr_gap_set_sec_level_cmd_handler(void *param);

#if (dg_configBLE_SKIP_LATENCY_API == 1)
/** GAP skip latency command message */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        bool                enable;
} ble_mgr_gap_skip_latency_cmd_t;

/** GAP skip latency response message */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gap_skip_latency_rsp_t;

void ble_mgr_gap_skip_latency_cmd_handler(void *param);
#endif /* (dg_configBLE_SKIP_LATENCY_API == 1) */

/** GAP data length set command message */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        uint16_t            tx_length;
        uint16_t            tx_time;
} ble_mgr_gap_data_length_set_cmd_t;

/** GAP data length set response message */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gap_data_length_set_rsp_t;

void ble_mgr_gap_data_length_set_cmd_handler(void *param);

/** GAP address resolve command message */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        bd_address_t        address;
} ble_mgr_gap_address_resolve_cmd_t;

/** GAP address resolve response message */
typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gap_address_resolve_rsp_t;

void ble_mgr_gap_address_resolve_cmd_handler(void *param);

/**
 * BLE stack event handlers
 */
void ble_mgr_gap_dev_bdaddr_ind_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gap_adv_report_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gap_connected_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gap_get_device_info_req_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gap_set_device_info_req_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gap_disconnected_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gap_peer_version_ind_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gap_peer_features_ind_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gap_conn_param_update_req_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gap_conn_param_updated_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gapm_adv_cmp_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gapm_scan_cmp_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gapm_connect_cmp_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gap_bond_req_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gap_bond_ind_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gap_security_ind_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gap_sign_counter_ind_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gap_encrypt_req_ind_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gapc_cmp__disconnect_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gapc_cmp__update_params_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gapc_cmp__bond_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gap_encrypt_ind_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gap_addr_solved_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gap_le_pkt_size_ind_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gap_cmp__data_length_set_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gapm_cmp__address_resolve_evt_handler(ble_gtl_msg_t *gtl);

#endif /* BLE_MGR_GAP_H_ */
/**
 \}
 \}
 \}
 */
