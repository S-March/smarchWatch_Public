/**
 ****************************************************************************************
 *
 * @file ble_l2cap.c
 *
 * @brief BLE L2CAP Connection Oriented Channels API implementation
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include "FreeRTOS.h"
#include <stdint.h>
#include <string.h>
#include "ble_common.h"
#include "osal.h"
#include "ble_gap.h"
#include "ble_mgr_helper.h"
#include "ble_l2cap.h"
#include "ble_mgr.h"
#include "ble_mgr_l2cap.h"
#include "storage.h"

#if (dg_configBLE_L2CAP_COC == 1)

static ble_error_t listen(uint16_t conn_idx, uint16_t psm, gap_sec_level_t sec_level,
                                uint16_t initial_credits, uint16_t *scid, bool defer_setup)
{
        ble_mgr_l2cap_listen_cmd_t *cmd;
        ble_mgr_l2cap_listen_rsp_t *rsp;
        ble_error_t ret = BLE_ERROR_FAILED;

        /* Create new command and fill it */
        cmd = alloc_ble_msg(BLE_MGR_L2CAP_LISTEN_CMD, sizeof(*cmd));
        cmd->conn_idx = conn_idx;
        cmd->psm = psm;
        cmd->sec_level = sec_level;
        cmd->initial_credits = initial_credits;
        cmd->defer_setup = defer_setup;

        if (!ble_cmd_execute(cmd, (void **) &rsp, ble_mgr_l2cap_listen_cmd_handler)) {
                return BLE_ERROR_FAILED;
        }

        ret = rsp->status;

        if (ret == BLE_STATUS_OK && scid) {
                *scid = rsp->scid;
        }

        OS_FREE(rsp);

        return ret;
}

ble_error_t ble_l2cap_listen(uint16_t conn_idx, uint16_t psm, gap_sec_level_t sec_level,
                                                        uint16_t initial_credits, uint16_t *scid)
{
        return listen(conn_idx, psm, sec_level, initial_credits, scid, false);
}

ble_error_t ble_l2cap_listen_defer_setup(uint16_t conn_idx, uint16_t psm,
                                        gap_sec_level_t sec_level, uint16_t initial_credits,
                                        uint16_t *scid)
{
        return listen(conn_idx, psm, sec_level, initial_credits, scid, true);
}

ble_error_t ble_l2cap_connection_cfm(uint16_t conn_idx, uint16_t scid,
                                                        enum ble_l2cap_connection_status status)
{
        ble_mgr_l2cap_connection_cfm_cmd_t *cmd;
        ble_mgr_l2cap_connection_cfm_rsp_t *rsp;
        ble_error_t ret = BLE_ERROR_FAILED;

        /* Create new command and fill it */
        cmd = alloc_ble_msg(BLE_MGR_L2CAP_CONNECTION_CFM_CMD, sizeof(*cmd));
        cmd->conn_idx = conn_idx;
        cmd->scid = scid;
        cmd->status = status;

        if (!ble_cmd_execute(cmd, (void **) &rsp, ble_mgr_l2cap_connection_cfm_cmd_handler)) {
                return BLE_ERROR_FAILED;
        }

        ret = rsp->status;

        OS_FREE(rsp);

        return ret;
}

ble_error_t ble_l2cap_stop_listen(uint16_t conn_idx, uint16_t cid)
{
        ble_mgr_l2cap_stop_listen_cmd_t *cmd;
        ble_mgr_l2cap_stop_listen_rsp_t *rsp;
        ble_error_t ret = BLE_ERROR_FAILED;

        /* Create new command and fill it */
        cmd = alloc_ble_msg(BLE_MGR_L2CAP_STOP_LISTEN_CMD, sizeof(*cmd));
        cmd->conn_idx = conn_idx;
        cmd->scid = cid;

        if (!ble_cmd_execute(cmd, (void **) &rsp, ble_mgr_l2cap_stop_listen_cmd_handler)) {
                return BLE_ERROR_FAILED;
        }

        ret = rsp->status;

        OS_FREE(rsp);

        return ret;
}

ble_error_t ble_l2cap_connect(uint16_t conn_idx, uint16_t psm, uint16_t initial_credits,
                                                                                uint16_t *scid)
{
        ble_mgr_l2cap_connect_cmd_t *cmd;
        ble_mgr_l2cap_connect_rsp_t *rsp;
        ble_error_t ret = BLE_ERROR_FAILED;

        /* Create new command and fill it */
        cmd = alloc_ble_msg(BLE_MGR_L2CAP_CONNECT_CMD, sizeof(*cmd));
        cmd->conn_idx = conn_idx;
        cmd->psm = psm;
        cmd->initial_credits = initial_credits;

        if (!ble_cmd_execute(cmd, (void **) &rsp, ble_mgr_l2cap_connect_cmd_handler)) {
                return BLE_ERROR_FAILED;
        }

        ret = rsp->status;

        if (ret == BLE_STATUS_OK && scid) {
                *scid = rsp->scid;
        }

        OS_FREE(rsp);

        return ret;
}

ble_error_t ble_l2cap_disconnect(uint16_t conn_idx, uint16_t scid)
{
        ble_mgr_l2cap_disconnect_cmd_t *cmd;
        ble_mgr_l2cap_disconnect_rsp_t *rsp;
        ble_error_t ret = BLE_ERROR_FAILED;

        /* Create new command and fill it */
        cmd = alloc_ble_msg(BLE_MGR_L2CAP_DISCONNECT_CMD, sizeof(*cmd));
        cmd->conn_idx = conn_idx;
        cmd->scid = scid;

        if (!ble_cmd_execute(cmd, (void **) &rsp, ble_mgr_l2cap_disconnect_cmd_handler)) {
                return BLE_ERROR_FAILED;
        }

        ret = rsp->status;

        OS_FREE(rsp);

        return ret;
}

ble_error_t ble_l2cap_add_credits(uint16_t conn_idx, uint16_t scid, uint16_t credits)
{
        ble_mgr_l2cap_add_credits_cmd_t *cmd;
        ble_mgr_l2cap_add_credits_rsp_t *rsp;
        ble_error_t ret = BLE_ERROR_FAILED;

        /* Create new command and fill it */
        cmd = alloc_ble_msg(BLE_MGR_L2CAP_ADD_CREDITS_CMD, sizeof(*cmd));
        cmd->conn_idx = conn_idx;
        cmd->scid = scid;
        cmd->credits = credits;

        if (!ble_cmd_execute(cmd, (void **) &rsp, ble_mgr_l2cap_add_credits_cmd_handler)) {
                return BLE_ERROR_FAILED;
        }

        ret = rsp->status;

        OS_FREE(rsp);

        return ret;
}

ble_error_t ble_l2cap_send(uint16_t conn_idx, uint16_t cid, uint16_t length, const void *data)
{
        ble_mgr_l2cap_send_cmd_t *cmd;
        ble_mgr_l2cap_send_rsp_t *rsp;
        ble_error_t ret = BLE_ERROR_FAILED;

        /* Create new command and fill it */
        cmd = alloc_ble_msg(BLE_MGR_L2CAP_SEND_CMD, sizeof(*cmd) + length);
        cmd->conn_idx = conn_idx;
        cmd->scid = cid;
        cmd->length = length;
        memcpy(cmd->data, data, length);

        if (!ble_cmd_execute(cmd, (void **) &rsp, ble_mgr_l2cap_send_cmd_handler)) {
                return BLE_ERROR_FAILED;
        }

        ret = rsp->status;

        OS_FREE(rsp);

        return ret;
}
#endif /* (dg_configBLE_L2CAP_COC == 1) */

ble_error_t ble_l2cap_conn_param_update(uint16_t conn_idx, const gap_conn_params_t *conn_params)
{
        /* Only ble_gap_conn_param_update() is supported (Deprecated). */

        return BLE_ERROR_FAILED;
}
