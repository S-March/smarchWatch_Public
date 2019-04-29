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
 * @file ble_mgr_gattc.h
 *
 * @brief BLE manager definitions and handlers for GATTC
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BLE_MGR_GATTC_H_
#define BLE_MGR_GATTC_H_

#include <stdint.h>
#include "osal.h"
#include "ble_mgr_cmd.h"
#include "ble_att.h"
#include "ble_gatt.h"
#include "ble_gattc.h"

enum ble_cmd_gattc_opcode {
        BLE_MGR_GATTC_BROWSE_CMD = BLE_MGR_CMD_CAT_FIRST(BLE_MGR_GATTC_CMD_CAT),
        BLE_MGR_GATTC_DISCOVER_SVC_CMD,
        BLE_MGR_GATTC_DISCOVER_INCLUDE_CMD,
        BLE_MGR_GATTC_DISCOVER_CHAR_CMD,
        BLE_MGR_GATTC_DISCOVER_DESC_CMD,
        BLE_MGR_GATTC_READ_CMD,
        BLE_MGR_GATTC_WRITE_GENERIC_CMD,
        BLE_MGR_GATTC_WRITE_EXECUTE_CMD,
        BLE_MGR_GATTC_EXCHANGE_MTU_CMD,
        /* Dummy command opcode, needs to be always defined after all commands */
        BLE_MGR_GATTC_LAST_CMD,
};

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        const att_uuid_t    *uuid;
} ble_mgr_gattc_browse_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gattc_browse_rsp_t;

void ble_mgr_gattc_browse_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        const att_uuid_t    *uuid;
} ble_mgr_gattc_discover_svc_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gattc_discover_svc_rsp_t;

void ble_mgr_gattc_discover_svc_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        uint16_t            start_h;
        uint16_t            end_h;
} ble_mgr_gattc_discover_include_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gattc_discover_include_rsp_t;

void ble_mgr_gattc_discover_include_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        uint16_t            start_h;
        uint16_t            end_h;
        const att_uuid_t    *uuid;
} ble_mgr_gattc_discover_char_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gattc_discover_char_rsp_t;

void ble_mgr_gattc_discover_char_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        uint16_t            start_h;
        uint16_t            end_h;
} ble_mgr_gattc_discover_desc_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gattc_discover_desc_rsp_t;

void ble_mgr_gattc_discover_desc_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        uint16_t            handle;
        uint16_t            offset;
} ble_mgr_gattc_read_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gattc_read_rsp_t;

void ble_mgr_gattc_read_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        uint16_t            handle;
        bool                no_response:1;
        bool                signed_write:1;
        bool                prepare:1;
        uint16_t            offset;
        uint16_t            length;
        const uint8_t       *value;
} ble_mgr_gattc_write_generic_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gattc_write_generic_rsp_t;

void ble_mgr_gattc_write_generic_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        bool                commit;
} ble_mgr_gattc_write_execute_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gattc_write_execute_rsp_t;

void ble_mgr_gattc_write_execute_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
} ble_mgr_gattc_exchange_mtu_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gattc_exchange_mtu_rsp_t;

void ble_mgr_gattc_exchange_mtu_cmd_handler(void *param);

/**
 * BLE stack event handlers
 */
void ble_mgr_gattc_mtu_changed_ind_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gattc_sdp_svc_ind_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gattc_cmp__browse_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gattc_disc_svc_ind_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gattc_disc_svc_incl_ind_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gattc_disc_char_ind_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gattc_disc_char_desc_ind_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gattc_cmp__discovery_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gattc_read_ind_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gattc_cmp__read_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gattc_cmp__write_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gattc_event_ind_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gattc_event_req_ind_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gattc_svc_changed_cfg_ind_evt_handler(ble_gtl_msg_t *gtl);

#endif /* BLE_MGR_GATTC_H_ */
/**
 \}
 \}
 \}
 */
