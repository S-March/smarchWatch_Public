/**
 ****************************************************************************************
 *
 * @file ble_common.c
 *
 * @brief BLE common API implementation
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ble_mgr.h"
#include "ble_mgr_common.h"
#include "ble_mgr_helper.h"
#include "ble_common.h"
#include "ble_gap.h"
#include "ble_gatts.h"
#include "rf.h"

ble_error_t ble_register_app(void)
{
        ble_mgr_common_register_cmd_t *cmd;
        ble_mgr_common_register_rsp_t *rsp;
        ble_error_t ret = BLE_ERROR_FAILED;

        /* Create new command and fill it */
        cmd = alloc_ble_msg(BLE_MGR_COMMON_REGISTER_CMD, sizeof(*cmd));
        cmd->task = OS_GET_CURRENT_TASK();

        if (!ble_cmd_execute(cmd, (void **) &rsp, ble_mgr_common_register_cmd_handler)) {
                return BLE_ERROR_FAILED;
        }

        ret = rsp->status;
        OS_FREE(rsp);

        return ret;
}

ble_error_t ble_enable(void)
{
        ble_mgr_common_enable_cmd_t *cmd;
        ble_mgr_common_enable_rsp_t *rsp;
        ble_error_t ret = BLE_ERROR_FAILED;
        ble_dev_params_t *dev_params;

        dev_params = ble_mgr_dev_params_acquire();
        if (dev_params->status == BLE_IS_ENABLED) {
                ble_mgr_dev_params_release();
                return BLE_ERROR_ALREADY_DONE;
        }

        ble_mgr_dev_params_release();

        /* Create new command and fill it */
        cmd = alloc_ble_msg(BLE_MGR_COMMON_ENABLE_CMD, sizeof(*cmd));

        if (!ble_cmd_execute(cmd, (void **) &rsp, ble_mgr_common_enable_cmd_handler)) {
                return BLE_ERROR_FAILED;
        }

        ret = rsp->status;
        OS_FREE(rsp);

        return ret;
}

ble_error_t ble_reset(void)
{
        ble_mgr_common_reset_cmd_t *cmd;
        ble_mgr_common_reset_rsp_t *rsp;
        ble_error_t ret = BLE_ERROR_FAILED;

        /* Create new command and fill it */
        cmd = alloc_ble_msg(BLE_MGR_COMMON_RESET_CMD, sizeof(*cmd));

        if (!ble_cmd_execute(cmd, (void **) &rsp, ble_mgr_common_reset_cmd_handler)) {
                return BLE_ERROR_FAILED;
        }

        ret = rsp->status;
        OS_FREE(rsp);

        return ret;
}

#if (dg_configBLE_CENTRAL == 1)
ble_error_t ble_central_start(void)
{
        ble_error_t ret = BLE_ERROR_FAILED;

        /* Create new command and fill it */
        ret = ble_enable();

        if ((ret == BLE_STATUS_OK) || (ret == BLE_ERROR_ALREADY_DONE)) {
                ret = ble_gap_role_set(GAP_CENTRAL_ROLE);
        }

        return ret;
}
#endif /* (dg_configBLE_CENTRAL == 1) */

#if (dg_configBLE_PERIPHERAL == 1)
ble_error_t ble_peripheral_start(void)
{
        ble_error_t ret = BLE_ERROR_FAILED;

        /* Create new command and fill it */
        ret = ble_enable();

        if ((ret == BLE_STATUS_OK) || (ret == BLE_ERROR_ALREADY_DONE)) {
                ret = ble_gap_role_set(GAP_PERIPHERAL_ROLE);
        }

        return ret;
}
#endif /* (dg_configBLE_PERIPHERAL == 1) */

ble_evt_hdr_t *ble_get_event(bool wait)
{
        OS_BASE_TYPE ret;
        ble_evt_hdr_t *evt = NULL;

        ret = ble_mgr_event_queue_get(&evt, wait ? OS_QUEUE_FOREVER : OS_QUEUE_NO_WAIT);

        if (ret != OS_QUEUE_OK) {
                goto done;
        }

#if (BLE_MGR_USE_EVT_LIST == 0)
        /* Notify BLE manager that the event has been consumed */
        ble_mgr_notify_event_consumed();
#endif /* (BLE_MGR_USE_EVT_LIST == 0) */

done:
        return evt;
}

bool ble_has_event(void)
{
        const ble_mgr_interface_t *mgr_if = ble_mgr_get_interface();

#if (BLE_MGR_USE_EVT_LIST == 0)
        return OS_QUEUE_MESSAGES_WAITING( mgr_if->evt_q );
#else
        OS_ENTER_CRITICAL_SECTION();

        bool msg_waiting = ( mgr_if->evt_q == NULL ) ? false : true;

        OS_LEAVE_CRITICAL_SECTION();

        return msg_waiting;
#endif /* (BLE_MGR_USE_EVT_LIST == 0) */
}

ble_error_t ble_read_tx_power(uint16_t conn_idx, tx_power_level_type_t type, uint8_t *tx_power)
{
        ble_mgr_common_read_tx_power_cmd_t *cmd;
        ble_mgr_common_read_tx_power_rsp_t *rsp;
        ble_error_t ret = BLE_ERROR_FAILED;

        /* Create new command and fill it */
        cmd = alloc_ble_msg(BLE_MGR_COMMON_READ_TX_POWER_CMD, sizeof(*cmd));
        cmd->conn_idx = conn_idx;
        cmd->type = type;

        if (!ble_cmd_execute(cmd, (void **) &rsp, ble_mgr_common_read_tx_power_cmd_handler)) {
                return BLE_ERROR_FAILED;
        }

        ret = rsp->status;
        *tx_power = rsp->tx_power_level;

        /* free message */
        OS_FREE(rsp);

        return ret;

}

void ble_handle_event_default(ble_evt_hdr_t *hdr)
{
        switch (hdr->evt_code) {
#if (dg_configBLE_CENTRAL == 1) || (dg_configBLE_PERIPHERAL == 1)
        case BLE_EVT_GAP_CONN_PARAM_UPDATE_REQ:
        {
                ble_evt_gap_conn_param_update_req_t *evt = (ble_evt_gap_conn_param_update_req_t *) hdr;
                ble_gap_conn_param_update_reply(evt->conn_idx, true);
                break;
        }
#endif /* (dg_configBLE_CENTRAL == 1) || (dg_configBLE_PERIPHERAL == 1) */
#if (dg_configBLE_PERIPHERAL == 1)
        case BLE_EVT_GAP_PAIR_REQ:
        {
                ble_evt_gap_pair_req_t *evt = (ble_evt_gap_pair_req_t *) hdr;
                ble_gap_pair_reply(evt->conn_idx, false, false);
                break;
        }
#endif /* (dg_configBLE_PERIPHERAL == 1) */
#if (dg_configBLE_GATT_SERVER == 1)
        case BLE_EVT_GATTS_READ_REQ:
        {
                ble_evt_gatts_read_req_t *evt = (ble_evt_gatts_read_req_t *) hdr;
                ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_READ_NOT_PERMITTED, 0, NULL);
                break;
        }
        case BLE_EVT_GATTS_WRITE_REQ:
        {
                ble_evt_gatts_write_req_t *evt = (ble_evt_gatts_write_req_t *) hdr;
                ble_gatts_write_cfm(evt->conn_idx, evt->handle, ATT_ERROR_WRITE_NOT_PERMITTED);
                break;
        }
        case BLE_EVT_GATTS_PREPARE_WRITE_REQ:
        {
                ble_evt_gatts_prepare_write_req_t *evt = (ble_evt_gatts_prepare_write_req_t *) hdr;
                ble_gatts_prepare_write_cfm(evt->conn_idx, evt->handle, 0, ATT_ERROR_WRITE_NOT_PERMITTED);
                break;
        }
#endif /* (dg_configBLE_GATT_SERVER == 1) */
        }
}

#if dg_configBLE_EVENT_NOTIF_TYPE == BLE_EVENT_NOTIF_USER_TASK

#if dg_configBLE_EVENT_NOTIF_RUNTIME_CONTROL == 1
PRIVILEGED_DATA static uint32_t ble_event_notif_mask;

void ble_event_notif_enable_end_event(void)
{
        OS_ENTER_CRITICAL_SECTION();
        ble_event_notif_mask |= dg_configBLE_EVENT_NOTIF_MASK_END_EVENT;
        OS_LEAVE_CRITICAL_SECTION();
}

void ble_event_notif_enable_cscnt_event(void)
{
        OS_ENTER_CRITICAL_SECTION();
        ble_event_notif_mask |= dg_configBLE_EVENT_NOTIF_MASK_CSCNT_EVENT;
        OS_LEAVE_CRITICAL_SECTION();
}

void ble_event_notif_enable_fine_event(void)
{
        OS_ENTER_CRITICAL_SECTION();
        ble_event_notif_mask |= dg_configBLE_EVENT_NOTIF_MASK_FINE_EVENT;
        OS_LEAVE_CRITICAL_SECTION();
}

void ble_event_notif_disable_end_event(void)
{
        OS_ENTER_CRITICAL_SECTION();
        ble_event_notif_mask &= ~dg_configBLE_EVENT_NOTIF_MASK_END_EVENT;
        OS_LEAVE_CRITICAL_SECTION();
}

void ble_event_notif_disable_cscnt_event(void)
{
        OS_ENTER_CRITICAL_SECTION();
        ble_event_notif_mask &= ~dg_configBLE_EVENT_NOTIF_MASK_CSCNT_EVENT;
        OS_LEAVE_CRITICAL_SECTION();
}

void ble_event_notif_disable_fine_event(void)
{
        OS_ENTER_CRITICAL_SECTION();
        ble_event_notif_mask &= ~dg_configBLE_EVENT_NOTIF_MASK_FINE_EVENT;
        OS_LEAVE_CRITICAL_SECTION();
}
#endif

/* Default implementations for app task notification callbacks
   (called from ISR)
 */
void ble_event_notif_app_task_end_event(void)
{
#if dg_configBLE_EVENT_NOTIF_RUNTIME_CONTROL == 1
        if ((ble_event_notif_mask & dg_configBLE_EVENT_NOTIF_MASK_END_EVENT) == 0) {
                return;
        }
#endif
        ble_mgr_notify_app_task(dg_configBLE_EVENT_NOTIF_MASK_END_EVENT);
}

void ble_event_notif_app_task_cscnt_event(void)
{
#if dg_configBLE_EVENT_NOTIF_RUNTIME_CONTROL == 1
        if ((ble_event_notif_mask & dg_configBLE_EVENT_NOTIF_MASK_CSCNT_EVENT) == 0) {
                return;
        }
#endif

        ble_mgr_notify_app_task(dg_configBLE_EVENT_NOTIF_MASK_CSCNT_EVENT);
}

void ble_event_notif_app_task_fine_event(void)
{
#if dg_configBLE_EVENT_NOTIF_RUNTIME_CONTROL == 1
        if ((ble_event_notif_mask & dg_configBLE_EVENT_NOTIF_MASK_FINE_EVENT) == 0) {
                return;
        }
#endif

        ble_mgr_notify_app_task(dg_configBLE_EVENT_NOTIF_MASK_FINE_EVENT);
}

#endif

const char *ble_address_to_string(const bd_address_t *address)
{
        static char buf[18];

        sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X", address->addr[5], address->addr[4],
                                                      address->addr[3], address->addr[2],
                                                      address->addr[1], address->addr[0]);

        return buf;
}

bool ble_address_from_string(const char *str, addr_type_t addr_type, bd_address_t *address)
{
        int8_t i;
        char *ptr;

        /*
         * The algorithm checks if the str has correct format: "xx:xx:xx:xx:xx:xx", where:
         * xx - number written in hexadecimal format in 2 bytes (e.g. 15, 4A, C4, FF, ...)
         * :  - required colon character
         *
         * Any data written after this 6 octets are discarded.
         */
        for (i = sizeof(address->addr) - 1; i >= 0; --i) {
                address->addr[i] = (uint8_t) strtol(str, &ptr, 16);

                if ((ptr - str) != 2) {
                        return false;
                }

                if (*ptr != ':') {
                        break;
                }

                str = ptr + 1;
        }

        address->addr_type = addr_type;

        return i == 0;
}

ble_error_t ble_set_fem_voltage_trim(uint8_t channel, uint8_t value)
{
        if ((channel > 39) || (value > 7)) {
                return BLE_ERROR_INVALID_PARAM;
        }

        rf_ble_set_ant_trim(channel, value);

        return BLE_STATUS_OK;
}
