/**
 ****************************************************************************************
 *
 * @file ble_mgr_cmd.c
 *
 * @brief BLE command handlers
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include "ble_mgr.h"
#include "ble_mgr_cmd.h"
#include "ble_mgr_common.h"
#include "ble_mgr_gap.h"
#include "ble_mgr_gatts.h"
#include "ble_mgr_gattc.h"
#include "ble_mgr_l2cap.h"

static const ble_mgr_cmd_handler_t h_common[BLE_MGR_CMD_GET_IDX(BLE_MGR_COMMON_LAST_CMD)] = {
        ble_mgr_common_stack_msg_handler,
        ble_mgr_common_register_cmd_handler,
#ifndef BLE_STACK_PASSTHROUGH_MODE
        ble_mgr_common_enable_cmd_handler,
        ble_mgr_common_reset_cmd_handler,
        ble_mgr_common_read_tx_power_cmd_handler,
#endif
};

#ifndef BLE_STACK_PASSTHROUGH_MODE
static const ble_mgr_cmd_handler_t h_gap[BLE_MGR_CMD_GET_IDX(BLE_MGR_GAP_LAST_CMD)] = {
        ble_mgr_gap_address_set_cmd_handler,
        ble_mgr_gap_device_name_set_cmd_handler,
        ble_mgr_gap_appearance_set_cmd_handler,
        ble_mgr_gap_ppcp_set_cmd_handler,
        ble_mgr_gap_adv_start_cmd_handler,
        ble_mgr_gap_adv_stop_cmd_handler,
        ble_mgr_gap_adv_data_set_cmd_handler,
        ble_mgr_gap_scan_start_cmd_handler,
        ble_mgr_gap_scan_stop_cmd_handler,
        ble_mgr_gap_connect_cmd_handler,
        ble_mgr_gap_connect_cancel_cmd_handler,
        ble_mgr_gap_disconnect_cmd_handler,
        ble_mgr_gap_conn_rssi_get_cmd_handler,
        ble_mgr_gap_role_set_cmd_handler,
        ble_mgr_gap_mtu_size_set_cmd_handler,
        ble_mgr_gap_channel_map_set_cmd_handler,
        ble_mgr_gap_conn_param_update_cmd_handler,
        ble_mgr_gap_conn_param_update_reply_cmd_handler,
        ble_mgr_gap_pair_cmd_handler,
        ble_mgr_gap_pair_reply_cmd_handler,
        ble_mgr_gap_passkey_reply_cmd_handler,
        ble_mgr_gap_unpair_cmd_handler,
        ble_mgr_gap_set_sec_level_cmd_handler,
#if (dg_configBLE_SKIP_LATENCY_API == 1)
        ble_mgr_gap_skip_latency_cmd_handler,
#endif /* (dg_configBLE_SKIP_LATENCY_API == 1) */
        ble_mgr_gap_data_length_set_cmd_handler,
#if (dg_configBLE_SECURE_CONNECTIONS == 1)
        ble_mgr_gap_numeric_reply_cmd_handler,
#endif /* (dg_configBLE_SECURE_CONNECTIONS == 1) */
        ble_mgr_gap_address_resolve_cmd_handler,
};

static const ble_mgr_cmd_handler_t h_gatts[BLE_MGR_CMD_GET_IDX(BLE_MGR_GATTS_LAST_CMD)] = {
        ble_mgr_gatts_service_add_cmd_handler,
        ble_mgr_gatts_service_add_include_cmd_handler,
        ble_mgr_gatts_service_add_characteristic_cmd_handler,
        ble_mgr_gatts_service_add_descriptor_cmd_handler,
        ble_mgr_gatts_service_register_cmd_handler,
        ble_mgr_gatts_service_enable_cmd_handler,
        ble_mgr_gatts_service_disable_cmd_handler,
        ble_mgr_gatts_service_characteristic_get_prop_cmd_handler,
        ble_mgr_gatts_service_characteristic_set_prop_cmd_handler,
        ble_mgr_gatts_get_value_cmd_handler,
        ble_mgr_gatts_set_value_cmd_handler,
        ble_mgr_gatts_read_cfm_cmd_handler,
        ble_mgr_gatts_write_cfm_cmd_handler,
        ble_mgr_gatts_prepare_write_cfm_cmd_handler,
        ble_mgr_gatts_send_event_cmd_handler,
        ble_mgr_gatts_service_changed_ind_cmd_handler,
};

static const ble_mgr_cmd_handler_t h_gattc[BLE_MGR_CMD_GET_IDX(BLE_MGR_GATTC_LAST_CMD)] = {
        ble_mgr_gattc_browse_cmd_handler,
        ble_mgr_gattc_discover_svc_cmd_handler,
        ble_mgr_gattc_discover_include_cmd_handler,
        ble_mgr_gattc_discover_char_cmd_handler,
        ble_mgr_gattc_discover_desc_cmd_handler,
        ble_mgr_gattc_read_cmd_handler,
        ble_mgr_gattc_write_generic_cmd_handler,
        ble_mgr_gattc_write_execute_cmd_handler,
        ble_mgr_gattc_exchange_mtu_cmd_handler,
};

static const ble_mgr_cmd_handler_t h_l2cap[BLE_MGR_CMD_GET_IDX(BLE_MGR_L2CAP_LAST_CMD)] = {
        ble_mgr_l2cap_listen_cmd_handler,
        ble_mgr_l2cap_stop_listen_cmd_handler,
        ble_mgr_l2cap_connection_cfm_cmd_handler,
        ble_mgr_l2cap_connect_cmd_handler,
        ble_mgr_l2cap_disconnect_cmd_handler,
        ble_mgr_l2cap_add_credits_cmd_handler,
        ble_mgr_l2cap_send_cmd_handler,
};
#endif

static const ble_mgr_cmd_handler_t* handlers[BLE_MGR_LAST_CMD_CAT] = {
        h_common,
#ifndef BLE_STACK_PASSTHROUGH_MODE
        h_gap,
        h_gatts,
        h_gattc,
        h_l2cap,
#endif
};

static const uint8_t handlers_num[BLE_MGR_LAST_CMD_CAT] = {
        BLE_MGR_CMD_GET_IDX(BLE_MGR_COMMON_LAST_CMD),
#ifndef BLE_STACK_PASSTHROUGH_MODE
        BLE_MGR_CMD_GET_IDX(BLE_MGR_GAP_LAST_CMD),
        BLE_MGR_CMD_GET_IDX(BLE_MGR_GATTS_LAST_CMD),
        BLE_MGR_CMD_GET_IDX(BLE_MGR_GATTC_LAST_CMD),
        BLE_MGR_CMD_GET_IDX(BLE_MGR_L2CAP_LAST_CMD),
#endif
};

bool ble_mgr_cmd_handle(void *cmd)
{
        ble_mgr_msg_hdr_t *hdr = cmd;
        uint8_t cat, idx;
        const ble_mgr_cmd_handler_t *h;

        cat = BLE_MGR_CMD_GET_CAT(hdr->op_code);
        idx = BLE_MGR_CMD_GET_IDX(hdr->op_code);

        /* Make sure the message has a valid category and ID */
        OS_ASSERT(cat < BLE_MGR_LAST_CMD_CAT);
        OS_ASSERT(idx < handlers_num[cat]);

        h = handlers[cat];
        h += idx;

        if (!(*h)) {
                /* No handler provided */
                return false;
        }

        (*h)(cmd);

        return true;
}
