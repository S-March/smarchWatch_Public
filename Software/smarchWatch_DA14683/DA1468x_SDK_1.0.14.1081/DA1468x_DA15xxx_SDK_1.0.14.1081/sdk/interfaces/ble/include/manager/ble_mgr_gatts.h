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
 * @file ble_mgr_gatts.h
 *
 * @brief BLE manager definitions and handlers for GATTS
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BLE_MGR_GATTS_H_
#define BLE_MGR_GATTS_H_

#include <stdint.h>
#include "osal.h"
#include "ble_mgr_cmd.h"
#include "ble_att.h"
#include "ble_gatt.h"
#include "ble_gatts.h"

enum ble_cmd_gatts_opcode {
        BLE_MGR_GATTS_SERVICE_ADD_CMD = BLE_MGR_CMD_CAT_FIRST(BLE_MGR_GATTS_CMD_CAT),
        BLE_MGR_GATTS_SERVICE_INCLUDE_ADD_CMD,
        BLE_MGR_GATTS_SERVICE_CHARACTERISTIC_ADD_CMD,
        BLE_MGR_GATTS_SERVICE_DESCRIPTOR_ADD_CMD,
        BLE_MGR_GATTS_SERVICE_REGISTER_CMD,
        BLE_MGR_GATTS_SERVICE_ENABLE_CMD,
        BLE_MGR_GATTS_SERVICE_DISABLE_CMD,
        BLE_MGR_GATTS_SERVICE_CHARACTERISTIC_GET_PROP_CMD,
        BLE_MGR_GATTS_SERVICE_CHARACTERISTIC_SET_PROP_CMD,
        BLE_MGR_GATTS_GET_VALUE_CMD,
        BLE_MGR_GATTS_SET_VALUE_CMD,
        BLE_MGR_GATTS_READ_CFM_CMD,
        BLE_MGR_GATTS_WRITE_CFM_CMD,
        BLE_MGR_GATTS_PREPARE_WRITE_CFM_CMD,
        BLE_MGR_GATTS_SEND_EVENT_CMD,
        BLE_MGR_GATTS_SERVICE_CHANGED_IND_CMD,
        /* Dummy command opcode, needs to be always defined after all commands */
        BLE_MGR_GATTS_LAST_CMD,
};

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        att_uuid_t          uuid;
        gatt_service_t      type;
        uint16_t            num_attrs;
} ble_mgr_gatts_service_add_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gatts_service_add_rsp_t;

void ble_mgr_gatts_service_add_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            handle;
} ble_mgr_gatts_service_add_include_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
        uint16_t            h_offset;
} ble_mgr_gatts_service_add_include_rsp_t;

void ble_mgr_gatts_service_add_include_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        att_uuid_t          uuid;
        uint16_t            prop;
        uint16_t            perm;
        uint16_t            max_len;
        gatts_flag_t        flags;
} ble_mgr_gatts_service_add_characteristic_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
        uint16_t            h_offset;
        uint16_t            h_val_offset;
} ble_mgr_gatts_service_add_characteristic_rsp_t;

void ble_mgr_gatts_service_add_characteristic_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        att_uuid_t          uuid;
        uint16_t            perm;
        uint16_t            max_len;
        gatts_flag_t        flags;
} ble_mgr_gatts_service_add_descriptor_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
        uint16_t            h_offset;
} ble_mgr_gatts_service_add_descriptor_rsp_t;

void ble_mgr_gatts_service_add_descriptor_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
} ble_mgr_gatts_service_register_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
        uint16_t            handle;
} ble_mgr_gatts_service_register_rsp_t;

void ble_mgr_gatts_service_register_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            handle;
} ble_mgr_gatts_service_enable_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gatts_service_enable_rsp_t;

void ble_mgr_gatts_service_enable_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            handle;
} ble_mgr_gatts_service_disable_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gatts_service_disable_rsp_t;

void ble_mgr_gatts_service_disable_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            handle;
} ble_mgr_gatts_service_characteristic_get_prop_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
        gatt_prop_t         prop;
        att_perm_t          perm;
} ble_mgr_gatts_service_characteristic_get_prop_rsp_t;

void ble_mgr_gatts_service_characteristic_get_prop_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            handle;
        gatt_prop_t         prop;
        att_perm_t          perm;
} ble_mgr_gatts_service_characteristic_set_prop_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gatts_service_characteristic_set_prop_rsp_t;

void ble_mgr_gatts_service_characteristic_set_prop_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            handle;
        uint16_t            max_len;
} ble_mgr_gatts_get_value_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
        uint16_t            length;
        uint8_t             value[0];
} ble_mgr_gatts_get_value_rsp_t;

void ble_mgr_gatts_get_value_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            handle;
        uint16_t            length;
        uint8_t             value[0];
} ble_mgr_gatts_set_value_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gatts_set_value_rsp_t;

void ble_mgr_gatts_set_value_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        uint16_t            handle;
        ble_error_t         status;
        uint16_t            length;
        uint8_t             value[0];
} ble_mgr_gatts_read_cfm_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gatts_read_cfm_rsp_t;

void ble_mgr_gatts_read_cfm_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        uint16_t            handle;
        uint8_t             status;
} ble_mgr_gatts_write_cfm_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gatts_write_cfm_rsp_t;

void ble_mgr_gatts_write_cfm_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        uint16_t            handle;
        uint16_t            length;
        uint8_t             status;
} ble_mgr_gatts_prepare_write_cfm_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gatts_prepare_write_cfm_rsp_t;

void ble_mgr_gatts_prepare_write_cfm_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        uint16_t            handle;
        gatt_event_t        type;
        uint16_t            length;
        uint8_t             value[0];
} ble_mgr_gatts_send_event_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gatts_send_event_rsp_t;

void ble_mgr_gatts_send_event_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        uint16_t            start_handle;
        uint16_t            end_handle;
} ble_mgr_gatts_service_changed_ind_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_gatts_service_changed_ind_rsp_t;

void ble_mgr_gatts_service_changed_ind_cmd_handler(void *param);

/**
 * BLE stack event handlers
 */
void ble_mgr_gatts_read_value_req_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gatts_write_value_req_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gatts_prepare_write_req_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gatts_event_sent_evt_handler(ble_gtl_msg_t *gtl);

#endif /* BLE_MGR_GATTS_H_ */
/**
 \}
 \}
 \}
 */
