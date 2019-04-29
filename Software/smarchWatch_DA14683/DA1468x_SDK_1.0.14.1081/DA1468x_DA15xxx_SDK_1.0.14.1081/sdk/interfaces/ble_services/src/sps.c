/**
 ****************************************************************************************
 *
 * @file sps.c
 *
 * @brief Serial Port Service sample implementation
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "osal.h"
#include "ble_storage.h"
#include "ble_bufops.h"
#include "ble_gatts.h"
#include "ble_uuid.h"
#include "svc_defines.h"
#include "sps.h"

#define UUID_SPS                "0783b03e-8535-b5a0-7140-a304d2495cb7"
#define UUID_SPS_SERVER_TX      "0783b03e-8535-b5a0-7140-a304d2495cb8"
#define UUID_SPS_SERVER_RX      "0783b03e-8535-b5a0-7140-a304d2495cba"
#define UUID_SPS_FLOW_CTRL      "0783b03e-8535-b5a0-7140-a304d2495cb9"

static const char sps_tx_desc[] = "Server TX Data";
static const char sps_rx_desc[] = "Server RX Data";
static const char sps_flow_control_desc[] = "Flow Control";

/* Serial Port Service reference application: Table 1: Size of characteristics */
static const uint16_t sps_server_tx_size = 160;
static const uint16_t sps_server_rx_size = 160;

typedef struct {
        ble_service_t svc;

        sps_callbacks_t *cb;

        uint16_t sps_tx_val_h;
        uint16_t sps_tx_ccc_h;

        uint16_t sps_rx_val_h;

        uint16_t sps_flow_ctrl_val_h;
        uint16_t sps_flow_ctrl_ccc_h;
} sp_service_t;

static uint16_t get_tx_length(sp_service_t *sps, uint16_t conn_idx)
{
        uint16_t length = 0x00;

        ble_storage_get_u16(conn_idx, sps->sps_tx_val_h, &length);

        return length;
}

static void set_tx_length(sp_service_t *sps, uint16_t conn_idx, uint16_t length)
{
        ble_storage_put_u32(conn_idx, sps->sps_tx_val_h, length, false);
}

static int calculate_length(int length)
{
        return length > 20 ? 20 : length;
}

static bool send_tx_data(sp_service_t *sps, uint16_t conn_idx, uint16_t length, uint8_t *data)
{
        uint8_t status;

        status = ble_gatts_send_event(conn_idx, sps->sps_tx_val_h, GATT_EVENT_NOTIFICATION,
                                                                                length, data);

        return status == BLE_STATUS_OK ? true : false;
}

static void notify_flow_ctrl(sp_service_t *sps, uint16_t conn_idx, sps_flow_control_t value)
{
        uint8_t flow_ctrl = value;

        ble_gatts_send_event(conn_idx, sps->sps_flow_ctrl_val_h, GATT_EVENT_NOTIFICATION,
                                                                sizeof(flow_ctrl), &flow_ctrl);
}

static att_error_t handle_flow_ctrl_ccc_write(sp_service_t *sps,  uint16_t conn_idx,
                                        uint16_t offset, uint16_t length, const uint8_t *value)
{
        uint16_t ccc;

        if (offset) {
                return ATT_ERROR_ATTRIBUTE_NOT_LONG;
        }

        if (length != sizeof(ccc)) {
                return ATT_ERROR_APPLICATION_ERROR;
        }

        ccc = get_u16(value);

        ble_storage_put_u32(conn_idx, sps->sps_flow_ctrl_ccc_h, ccc, true);

        /* Send notification if client enabled notifications */
        if (ccc & GATT_CCC_NOTIFICATIONS) {
                uint8_t flow_ctrl = SPS_FLOW_CONTROL_OFF;

                ble_storage_get_u8(conn_idx, sps->sps_flow_ctrl_val_h, &flow_ctrl);

                notify_flow_ctrl(sps, conn_idx, flow_ctrl);
        }

        return ATT_ERROR_OK;
}

static att_error_t handle_tx_ccc_write(sp_service_t *sps,  uint16_t conn_idx,
                                        uint16_t offset, uint16_t length, const uint8_t *value)
{
        uint16_t ccc;

        if (offset) {
                return ATT_ERROR_ATTRIBUTE_NOT_LONG;
        }

        if (length != sizeof(ccc)) {
                return ATT_ERROR_APPLICATION_ERROR;
        }

        ccc = get_u16(value);

        ble_storage_put_u32(conn_idx, sps->sps_tx_ccc_h, ccc, true);

        return ATT_ERROR_OK;
}

static att_error_t set_flow_control_req(sp_service_t *sps, uint16_t conn_idx,
                                        uint16_t offset, uint16_t length, const uint8_t *value)
{
        if (offset) {
                return ATT_ERROR_ATTRIBUTE_NOT_LONG;
        }

        if (length != sizeof(uint8_t)) {
                return ATT_ERROR_INVALID_VALUE_LENGTH;
        }

        if (value[0] != SPS_FLOW_CONTROL_ON && value[0] != SPS_FLOW_CONTROL_OFF) {
                return ATT_ERROR_APPLICATION_ERROR;
        }

        if (sps->cb && sps->cb->set_flow_control) {
                sps->cb->set_flow_control(&sps->svc, conn_idx, value[0]);
        }

        return ATT_ERROR_OK;
}

static att_error_t handle_rx_data(sp_service_t *sps, uint16_t conn_idx,
                                        uint16_t offset, uint16_t length, const uint8_t *value)
{
        if (sps->cb && sps->cb->rx_data) {
                sps->cb->rx_data(&sps->svc, conn_idx, value, length);
        }

        return ATT_ERROR_OK;
}

static void handle_write_req(ble_service_t *svc, const ble_evt_gatts_write_req_t *evt)
{
        sp_service_t *sps = (sp_service_t *) svc;
        att_error_t status = ATT_ERROR_ATTRIBUTE_NOT_FOUND;
        uint16_t handle = evt->handle;

        if (handle == sps->sps_tx_ccc_h) {
                status = handle_tx_ccc_write(sps, evt->conn_idx, evt->offset, evt->length, evt->value);
        }
        if (handle == sps->sps_flow_ctrl_ccc_h) {
                status = handle_flow_ctrl_ccc_write(sps, evt->conn_idx, evt->offset, evt->length, evt->value);
        }

        if (handle == sps->sps_flow_ctrl_val_h) {
                status = set_flow_control_req(sps, evt->conn_idx, evt->offset, evt->length, evt->value);
        }

        if (handle == sps->sps_rx_val_h) {
                status = handle_rx_data(sps, evt->conn_idx, evt->offset, evt->length, evt->value);
        }

        ble_gatts_write_cfm(evt->conn_idx, evt->handle, status);
}

static void handle_read_req(ble_service_t *svc, const ble_evt_gatts_read_req_t *evt)
{
        sp_service_t *sps = (sp_service_t *) svc;

        if (evt->handle == sps->sps_flow_ctrl_ccc_h || evt->handle == sps->sps_tx_ccc_h) {
                uint16_t ccc = 0x0000;
                if (evt->handle == sps->sps_flow_ctrl_ccc_h) {
                        ble_storage_get_u16(evt->conn_idx, sps->sps_flow_ctrl_ccc_h, &ccc);
                } else {
                        ble_storage_get_u16(evt->conn_idx, sps->sps_tx_ccc_h, &ccc);
                }
                // we're little-endian, ok to write directly from uint16_t
                ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_OK, sizeof(ccc), &ccc);
        } else {
                ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_READ_NOT_PERMITTED, 0, NULL);
        }
}

static void handle_event_sent(ble_service_t *svc, const ble_evt_gatts_event_sent_t *evt)
{
        sp_service_t *sps = (sp_service_t *) svc;
        uint16_t length, conn_idx = evt->conn_idx;

        if (evt->handle == sps->sps_tx_val_h) {
                length = get_tx_length(sps, conn_idx);
                set_tx_length(sps, conn_idx, 0x00);

                sps->cb->tx_done(&sps->svc, conn_idx, length);
        }
}

static void cleanup(ble_service_t *svc)
{
        sp_service_t *sps = (sp_service_t *) svc;

        ble_storage_remove_all(sps->sps_flow_ctrl_val_h);
        ble_storage_remove_all(sps->sps_flow_ctrl_ccc_h);
        ble_storage_remove_all(sps->sps_tx_ccc_h);
        ble_storage_remove_all(sps->sps_tx_val_h);

        OS_FREE(sps);
}

ble_service_t *sps_init(sps_callbacks_t *cb)
{
        uint16_t num_attr, sps_tx_desc_h, sps_rx_desc_h, sps_flow_ctrl_desc_h;
        sp_service_t *sps;
        att_uuid_t uuid;

        sps = OS_MALLOC(sizeof(*sps));
        memset(sps, 0, sizeof(*sps));

        num_attr = ble_gatts_get_num_attr(0, 3, 5);

        ble_uuid_from_string(UUID_SPS, &uuid);
        ble_gatts_add_service(&uuid, GATT_SERVICE_PRIMARY, num_attr);

        /* SPS Server TX */
        ble_uuid_from_string(UUID_SPS_SERVER_TX, &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_NOTIFY, ATT_PERM_NONE,
                                                sps_server_tx_size, 0, NULL, &sps->sps_tx_val_h);

        ble_uuid_create16(UUID_GATT_CLIENT_CHAR_CONFIGURATION, &uuid);
        ble_gatts_add_descriptor(&uuid, ATT_PERM_RW, 2, 0, &sps->sps_tx_ccc_h);

        ble_uuid_create16(UUID_GATT_CHAR_USER_DESCRIPTION, &uuid);
        ble_gatts_add_descriptor(&uuid, ATT_PERM_READ, sizeof(sps_tx_desc), 0, &sps_tx_desc_h);

        /* SPS Server RX */
        ble_uuid_from_string(UUID_SPS_SERVER_RX, &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_WRITE_NO_RESP, ATT_PERM_WRITE,
                                                sps_server_rx_size, 0, NULL, &sps->sps_rx_val_h);

        ble_uuid_create16(UUID_GATT_CHAR_USER_DESCRIPTION, &uuid);
        ble_gatts_add_descriptor(&uuid, ATT_PERM_READ, sizeof(sps_rx_desc), 0, &sps_rx_desc_h);

        /* SPS Flow Control */
        ble_uuid_from_string(UUID_SPS_FLOW_CTRL, &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_WRITE_NO_RESP | GATT_PROP_NOTIFY, ATT_PERM_WRITE,
                                                1, 0, NULL, &sps->sps_flow_ctrl_val_h);

        ble_uuid_create16(UUID_GATT_CLIENT_CHAR_CONFIGURATION, &uuid);
        ble_gatts_add_descriptor(&uuid, ATT_PERM_RW, 2, 0, &sps->sps_flow_ctrl_ccc_h);

        ble_uuid_create16(UUID_GATT_CHAR_USER_DESCRIPTION, &uuid);
        ble_gatts_add_descriptor(&uuid, ATT_PERM_READ, sizeof(sps_flow_control_desc), 0, &sps_flow_ctrl_desc_h);

        /* Register SPS Service */
        ble_gatts_register_service(&sps->svc.start_h, &sps->sps_tx_val_h, &sps->sps_tx_ccc_h,
                                                &sps_tx_desc_h, &sps->sps_rx_val_h, &sps_rx_desc_h,
                                                &sps->sps_flow_ctrl_val_h, &sps->sps_flow_ctrl_ccc_h,
                                                &sps_flow_ctrl_desc_h, 0);

        /* Set value of Characteristic Descriptions */
        ble_gatts_set_value(sps_tx_desc_h, sizeof(sps_tx_desc), sps_tx_desc);
        ble_gatts_set_value(sps_rx_desc_h, sizeof(sps_rx_desc), sps_rx_desc);
        ble_gatts_set_value(sps_flow_ctrl_desc_h, sizeof(sps_flow_control_desc),
                                                                        sps_flow_control_desc);

        sps->svc.end_h = sps->svc.start_h + num_attr;
        sps->svc.write_req = handle_write_req;
        sps->svc.read_req = handle_read_req;
        sps->svc.event_sent = handle_event_sent;
        sps->svc.cleanup = cleanup;
        sps->cb = cb;

        ble_service_add(&sps->svc);

        return &sps->svc;
}

void sps_set_flow_control(ble_service_t *svc, uint16_t conn_idx, sps_flow_control_t value)
{
        sp_service_t *sps = (sp_service_t *) svc;
        uint16_t ccc = 0x0000;

        ble_storage_put_u32(conn_idx, sps->sps_flow_ctrl_val_h, value, false);

        ble_storage_get_u16(conn_idx, sps->sps_flow_ctrl_ccc_h, &ccc);
        if (!(ccc & GATT_CCC_NOTIFICATIONS)) {
                return;
        }

        notify_flow_ctrl(sps, conn_idx, value);
}

void sps_tx_data(ble_service_t *svc, uint16_t conn_idx, uint8_t *data, uint16_t length)
{
        sp_service_t *sps = (sp_service_t *) svc;
        uint16_t ccc = 0x0000;
        uint8_t flow_ctrl = SPS_FLOW_CONTROL_OFF;

        if (get_tx_length(sps, conn_idx) != 0x00) {
                return;
        }

        if (!sps->cb->tx_done) {
                return;
        }

        /* Check if remote client registered for TX data */
        ble_storage_get_u16(conn_idx, sps->sps_tx_ccc_h, &ccc);
        if (!(ccc & GATT_CCC_NOTIFICATIONS)) {
                return;
        }

        /* Check if flow control is enabled */
        ble_storage_get_u8(conn_idx, sps->sps_flow_ctrl_val_h, &flow_ctrl);
        if (flow_ctrl != SPS_FLOW_CONTROL_ON) {
                return;
        }

        length = calculate_length(length);

        if (send_tx_data(sps, conn_idx, length, data)) {
                set_tx_length(sps, conn_idx, length);
        }
}
