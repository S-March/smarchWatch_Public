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
 * @file ble_mgr_common.h
 *
 * @brief BLE manager common definitions and handlers
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BLE_MGR_COMMON_H_
#define BLE_MGR_COMMON_H_

#include "osal.h"
#include "ble_mgr.h"
#include "ble_mgr_cmd.h"

enum ble_mgr_common_cmd_opcode {
        BLE_MGR_COMMON_STACK_MSG = BLE_MGR_CMD_CAT_FIRST(BLE_MGR_COMMON_CMD_CAT),
        BLE_MGR_COMMON_REGISTER_CMD,
        BLE_MGR_COMMON_ENABLE_CMD,
        BLE_MGR_COMMON_RESET_CMD,
        BLE_MGR_COMMON_READ_TX_POWER_CMD,
        /* Dummy command opcode, needs to be always defined after all commands */
        BLE_MGR_COMMON_LAST_CMD,
};

/** Definition of BLE stack message */
typedef struct ble_msg {
        ble_mgr_msg_hdr_t     hdr;        /**< Message header (op_code and msg_len) */
        ble_stack_msg_type_t  msg_type;   /**< Stack message type (GTL, HCI CMD, HCI ACL,
                                                                   HCI SCO or HCI EVT) */
        ble_stack_msg_t       msg;        /**< Stack message place holder */
} ble_mgr_common_stack_msg_t;

void ble_mgr_common_stack_msg_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        OS_TASK             task;
} ble_mgr_common_register_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_common_register_rsp_t;

void ble_mgr_common_register_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
} ble_mgr_common_enable_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ad_ble_status_t     status;
} ble_mgr_common_enable_rsp_t;

void ble_mgr_common_enable_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
} ble_mgr_common_reset_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ad_ble_status_t     status;
} ble_mgr_common_reset_rsp_t;

void ble_mgr_common_reset_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        tx_power_level_type_t type;
} ble_mgr_common_read_tx_power_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
        uint8_t             tx_power_level;
} ble_mgr_common_read_tx_power_rsp_t;

void ble_mgr_common_read_tx_power_cmd_handler(void *param);

#endif /* BLE_MGR_COMMON_H_ */
/**
 \}
 \}
 \}
 */
