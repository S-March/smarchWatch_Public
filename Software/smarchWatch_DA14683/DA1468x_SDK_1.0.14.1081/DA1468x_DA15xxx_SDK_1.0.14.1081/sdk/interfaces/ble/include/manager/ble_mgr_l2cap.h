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
 * @file ble_mgr_l2cap.h
 *
 * @brief BLE manager definitions and handlers for L2CAP
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BLE_MGR_L2CAP_H_
#define BLE_MGR_L2CAP_H_

#include <stdint.h>
#include <stdbool.h>
#include "osal.h"
#include "ble_mgr_cmd.h"
#include "ble_mgr_gtl.h"
#include "ble_l2cap.h"

/** OP codes for L2CAP commands */
enum ble_cmd_l2cap_opcode {
        BLE_MGR_L2CAP_LISTEN_CMD  = BLE_MGR_CMD_CAT_FIRST(BLE_MGR_L2CAP_CMD_CAT),
        BLE_MGR_L2CAP_STOP_LISTEN_CMD,
        BLE_MGR_L2CAP_CONNECTION_CFM_CMD,
        BLE_MGR_L2CAP_CONNECT_CMD,
        BLE_MGR_L2CAP_DISCONNECT_CMD,
        BLE_MGR_L2CAP_ADD_CREDITS_CMD,
        BLE_MGR_L2CAP_SEND_CMD,
        /* Dummy command opcode, needs to be always defined after all commands */
        BLE_MGR_L2CAP_LAST_CMD,
};

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        uint16_t            psm;
        gap_sec_level_t     sec_level;
        uint16_t            initial_credits;
        bool                defer_setup;
} ble_mgr_l2cap_listen_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        ble_error_t         status;
        uint16_t            scid;
} ble_mgr_l2cap_listen_rsp_t;

void ble_mgr_l2cap_listen_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        uint16_t            scid;
} ble_mgr_l2cap_stop_listen_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        ble_error_t         status;
        uint16_t            scid;
} ble_mgr_l2cap_stop_listen_rsp_t;

void ble_mgr_l2cap_stop_listen_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        uint16_t            scid;
        uint16_t            status;
} ble_mgr_l2cap_connection_cfm_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_l2cap_connection_cfm_rsp_t;

void ble_mgr_l2cap_connection_cfm_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        uint16_t            psm;
        uint16_t            initial_credits;
} ble_mgr_l2cap_connect_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
        uint16_t            scid;
} ble_mgr_l2cap_connect_rsp_t;

void ble_mgr_l2cap_connect_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        uint16_t            scid;
} ble_mgr_l2cap_disconnect_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
        uint16_t            scid;
} ble_mgr_l2cap_disconnect_rsp_t;

void ble_mgr_l2cap_disconnect_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        uint16_t            scid;
        uint16_t            credits;
} ble_mgr_l2cap_add_credits_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
        uint16_t            conn_idx;
        uint16_t            scid;
        uint16_t            credits;
} ble_mgr_l2cap_add_credits_rsp_t;

void ble_mgr_l2cap_add_credits_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        uint16_t            scid;
        uint16_t            length;
        uint8_t             data[0];
} ble_mgr_l2cap_send_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_l2cap_send_rsp_t;

void ble_mgr_l2cap_send_cmd_handler(void *param);

/**
 * BLE stack event handlers
 */
void ble_mgr_l2cap_connect_ind_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_l2cap_disconnect_ind_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_l2cap_connect_req_ind_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_l2cap_add_ind_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_l2cap_pdu_send_rsp_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_l2cap_lecnx_data_recv_ind_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gapc_cmp__le_cb_connection_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_l2cap_disconnect_ind(uint16_t conn_idx);

#endif /* BLE_MGR_L2CAP_H_ */
/**
 \}
 \}
 \}
 */
