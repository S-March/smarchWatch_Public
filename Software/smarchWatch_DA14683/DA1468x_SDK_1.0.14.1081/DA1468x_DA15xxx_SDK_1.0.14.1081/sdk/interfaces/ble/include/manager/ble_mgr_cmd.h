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
 * @file ble_mgr_cmd.h
 *
 * @brief BLE manager command definitions
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BLE_MGR_CMD_H_
#define BLE_MGR_CMD_H_

#include <stdbool.h>
#include "osal.h"

typedef void (* ble_mgr_cmd_handler_t) (void *param);

/**
 * Common header for all BLE messages
 */
typedef struct {
        uint16_t        op_code;
        uint16_t        msg_len;
        uint8_t         payload[0];
} ble_mgr_msg_hdr_t;

/**
 * BLE command categories
 */
enum ble_cmd_cat {
        BLE_MGR_COMMON_CMD_CAT  = 0x00,
        BLE_MGR_GAP_CMD_CAT     = 0x01,
        BLE_MGR_GATTS_CMD_CAT   = 0x02,
        BLE_MGR_GATTC_CMD_CAT   = 0x03,
        BLE_MGR_L2CAP_CMD_CAT   = 0x04,
        BLE_MGR_LAST_CMD_CAT,
};

#define BLE_MGR_CMD_CAT_FIRST(CAT) (CAT << 8)

#define BLE_MGR_CMD_GET_CAT(OPCODE) (OPCODE >> 8)
#define BLE_MGR_CMD_GET_IDX(OPCODE) (OPCODE & 0xFF)

bool ble_mgr_cmd_handle(void *cmd);

#endif /* BLE_MGR_CMD_H_ */
/**
 \}
 \}
 \}
 */
