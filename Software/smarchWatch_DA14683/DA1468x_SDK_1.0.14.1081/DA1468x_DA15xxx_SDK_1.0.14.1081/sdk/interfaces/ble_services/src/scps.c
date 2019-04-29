/**
 ****************************************************************************************
 *
 * @file scps.c
 *
 * @brief Scan Parameters Service sample implementation
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdbool.h>
#include <stddef.h>
#include "osal.h"
#include "util/queue.h"
#include "ble_att.h"
#include "ble_bufops.h"
#include "ble_common.h"
#include "ble_gatt.h"
#include "ble_gatts.h"
#include "ble_storage.h"
#include "ble_uuid.h"
#include "scps.h"

#define UUID_SCAN_INTERVAL_WINDOW       (0x2A4F)
#define UUID_SCAN_REFRESH               (0x2A31)

typedef struct {
        ble_service_t svc;

        // callbacks
        const scps_callbacks_t *cb;

        // handles
        uint16_t siw_val_h;
        uint16_t sr_val_h;
        uint16_t sr_ccc_h;

        queue_t scan_intv_wins;
} scp_service_t;

typedef struct {
        void *next;

        uint16_t conn_idx;
        uint32_t scan_intv_win;
} conn_dev_t;

static void handle_disconnected_evt(ble_service_t *svc, const ble_evt_gap_disconnected_t *evt);
static void handle_read_req(ble_service_t *svc, const ble_evt_gatts_read_req_t *evt);
static void handle_write_req(ble_service_t *svc, const ble_evt_gatts_write_req_t *evt);

static att_error_t do_siw_val_write(scp_service_t *scps, uint16_t conn_idx, uint16_t offset,
                                                                uint16_t length, const uint8_t *value)
{
        uint16_t scan_intv;
        uint16_t scan_win;
        conn_dev_t *conn_dev;

        if (offset) {
                return ATT_ERROR_ATTRIBUTE_NOT_LONG;
        }

        if (length < 4) {
                return ATT_ERROR_INVALID_VALUE_LENGTH;
        }

        scan_intv = get_u16(value);
        scan_win = get_u16(value + 2);

        if (scps->cb && scps->cb->disconnected) {
                conn_dev = OS_MALLOC(sizeof(*conn_dev));
                conn_dev->scan_intv_win = (scan_intv << 16) | scan_win;
                conn_dev->conn_idx = conn_idx;
                queue_push_front(&scps->scan_intv_wins, conn_dev);
        }

        if (scps->cb && scps->cb->scan_updated) {
                scps->cb->scan_updated(conn_idx, scan_intv, scan_win);
        }

        return ATT_ERROR_OK;
}

static att_error_t do_sr_ccc_write(scp_service_t *scps, uint16_t conn_idx, uint16_t offset,
                                                                uint16_t length, const uint8_t *value)
{
        uint16_t ccc;

        if (offset) {
                return ATT_ERROR_ATTRIBUTE_NOT_LONG;
        }

        if (length != sizeof(ccc)) {
                return ATT_ERROR_APPLICATION_ERROR;
        }

        ccc = get_u16(value);

        ble_storage_put_u32(conn_idx, scps->sr_ccc_h, ccc, true);

        return ATT_ERROR_OK;
}

static bool conn_dev_conn_idx_match(const void *data, const void *match_data)
{
        conn_dev_t *conn_dev = (conn_dev_t *) data;
        uint16_t conn_idx = (*(uint16_t *) match_data);

        return conn_dev->conn_idx == conn_idx;
}

static void handle_disconnected_evt(ble_service_t *svc, const ble_evt_gap_disconnected_t *evt)
{
        scp_service_t *scps = (scp_service_t *) svc;
        uint32_t scan_intv_win = 0;

        if (scps->cb && scps->cb->disconnected) {
                conn_dev_t *conn_dev;

                conn_dev = queue_remove(&scps->scan_intv_wins, conn_dev_conn_idx_match,
                                                                                &evt->conn_idx);

                if (conn_dev) {
                        scan_intv_win = conn_dev->scan_intv_win;
                        OS_FREE(conn_dev);
                }

                scps->cb->disconnected(evt->conn_idx, scan_intv_win >> 16, scan_intv_win);
        }
}

static void handle_read_req(ble_service_t *svc, const ble_evt_gatts_read_req_t *evt)
{
        scp_service_t *scps = (scp_service_t *) svc;

        if (evt->handle == scps->sr_ccc_h) {
                uint16_t ccc = 0x0000;

                ble_storage_get_u16(evt->conn_idx, scps->sr_ccc_h, &ccc);

                // we're little-endian, ok to write directly from uint16_t
                ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_OK, sizeof(ccc), &ccc);
        } else {
                ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_READ_NOT_PERMITTED, 0, NULL);
        }
}

static void handle_write_req(ble_service_t *svc, const ble_evt_gatts_write_req_t *evt)
{
        scp_service_t *scps = (scp_service_t *) svc;
        att_error_t status = ATT_ERROR_ATTRIBUTE_NOT_FOUND;

        if (evt->handle == scps->siw_val_h) {
                status = do_siw_val_write(scps, evt->conn_idx, evt->offset, evt->length, evt->value);
                goto done;
        }

        if (evt->handle == scps->sr_ccc_h) {
                status = do_sr_ccc_write(scps, evt->conn_idx, evt->offset, evt->length, evt->value);
                goto done;
        }

done:
        ble_gatts_write_cfm(evt->conn_idx, evt->handle, status);
}

static void cleanup(ble_service_t *svc)
{
        scp_service_t *scps = (scp_service_t *) svc;

        ble_storage_remove_all(scps->sr_ccc_h);

        OS_FREE(scps);
}

ble_service_t *scps_init(const scps_callbacks_t *cb)
{
        scp_service_t *scps;
        uint16_t num_attr;
        att_uuid_t uuid;

        scps = OS_MALLOC(sizeof(*scps));
        memset(scps, 0, sizeof(*scps));

        num_attr = ble_gatts_get_num_attr(0, 2, 1);

        ble_uuid_create16(UUID_SERVICE_SCPS, &uuid);
        ble_gatts_add_service(&uuid, GATT_SERVICE_PRIMARY, num_attr);

        ble_uuid_create16(UUID_SCAN_INTERVAL_WINDOW, &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_WRITE_NO_RESP, ATT_PERM_WRITE,
                                                                        4, 0, NULL, &scps->siw_val_h);

        ble_uuid_create16(UUID_SCAN_REFRESH, &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_NOTIFY, ATT_PERM_NONE,
                                                                        1, 0, NULL, &scps->sr_val_h);

        ble_uuid_create16(UUID_GATT_CLIENT_CHAR_CONFIGURATION, &uuid);
        ble_gatts_add_descriptor(&uuid, ATT_PERM_RW, 1, 0, &scps->sr_ccc_h);

        ble_gatts_register_service(&scps->svc.start_h, &scps->siw_val_h, &scps->sr_val_h,
                                                                                &scps->sr_ccc_h, 0);

        scps->svc.end_h = scps->svc.start_h + num_attr;

        scps->svc.disconnected_evt = handle_disconnected_evt;
        scps->svc.read_req = handle_read_req;
        scps->svc.write_req = handle_write_req;
        scps->svc.cleanup = cleanup;
        scps->cb = cb;

        ble_service_add(&scps->svc);

        return &scps->svc;
}

void scps_notify_refresh(ble_service_t *svc, uint16_t conn_idx)
{
        scp_service_t *scps = (scp_service_t *) svc;
        uint16_t ccc = 0x0000;
        uint8_t value = 0x00;

        ble_storage_get_u16(conn_idx, scps->sr_ccc_h, &ccc);

        if (!(ccc & GATT_CCC_NOTIFICATIONS)) {
                // notification disabled by client
                return;
        }

        ble_gatts_send_event(conn_idx, scps->sr_val_h, GATT_EVENT_NOTIFICATION, sizeof(value), &value);
}

void scps_notify_refresh_all(ble_service_t *svc)
{
        uint8_t num_conn;
        uint16_t *conn_idx;

        ble_gap_get_connected(&num_conn, &conn_idx);

        while (num_conn--) {
                scps_notify_refresh(svc, conn_idx[num_conn]);
        }

        if (conn_idx) {
                OS_FREE(conn_idx);
        }
}
