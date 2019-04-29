/**
 ****************************************************************************************
 *
 * @file wss.c
 *
 * @brief Weight Scale Service sample implementation
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
#include "ble_att.h"
#include "ble_bufops.h"
#include "ble_common.h"
#include "ble_gatt.h"
#include "ble_gatts.h"
#include "ble_uuid.h"
#include "ble_storage.h"
#include "wss.h"

#define UUID_WEIGHT_SCALE_FEATURE       (0x2A9E)
#define UUID_WEIGHT_MEASUREMENT         (0x2A9D)

typedef enum {
        WSS_WM_FLAGS_UNITS_SI                   = 0x00,
        WSS_WM_FLAGS_UNITS_IMPERIAL             = 0x01,
        WSS_WM_FLAGS_DATE_TIME_PRESENT          = 0x02,
        WSS_WM_FLAGS_USER_ID_PRESENT            = 0x04,
        WSS_WM_FLAGS_BMI_AND_HEIGHT_PRESENT     = 0x08,
} wss_weight_measurement_flags_t;

typedef struct {
        ble_service_t svc;

        // handles
        uint16_t features_h;
        uint16_t measurement_h;
        uint16_t wm_ccc_h;

        // callbacks
        const wss_callbacks_t *cb;

        // features flags
        wss_feature_t features;
} ws_service_t;

static att_error_t do_wm_ccc_write(ws_service_t *wss, uint16_t conn_idx, uint16_t offset,
                                                        uint16_t length, const uint8_t *value)
{
        uint16_t ccc_val;

        if (offset) {
                return ATT_ERROR_ATTRIBUTE_NOT_LONG;
        }

        if (length != sizeof(ccc_val)) {
                return ATT_ERROR_APPLICATION_ERROR;
        }

        ccc_val = get_u16(value);

        ble_storage_put_u32(conn_idx, wss->wm_ccc_h, ccc_val, true);

        if (wss->cb && wss->cb->indication_changed) {
                wss->cb->indication_changed(conn_idx, ccc_val & GATT_CCC_INDICATIONS);
        }

        return ATT_ERROR_OK;
}

static void handle_read_req(ble_service_t *svc, const ble_evt_gatts_read_req_t *evt)
{
        ws_service_t *wss = (ws_service_t *) svc;

        if (evt->handle == wss->wm_ccc_h) {
                uint16_t ccc = 0x0000;

                ble_storage_get_u16(evt->conn_idx, wss->wm_ccc_h, &ccc);

                // we're little-endian, ok to write directly from uint16_t
                ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_OK, sizeof(ccc), &ccc);
        } else {
                ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_READ_NOT_PERMITTED, 0,
                                                                                        NULL);
        }
}

static void handle_write_req(ble_service_t *svc, const ble_evt_gatts_write_req_t *evt)
{
        ws_service_t *wss = (ws_service_t *) svc;
        att_error_t status = ATT_ERROR_ATTRIBUTE_NOT_FOUND;

        if (evt->handle == wss->wm_ccc_h) {
                status = do_wm_ccc_write(wss, evt->conn_idx, evt->offset, evt->length, evt->value);
                goto done;
        }

done:
        ble_gatts_write_cfm(evt->conn_idx, evt->handle, status);
}

static void handle_connected_evt(ble_service_t *svc, const ble_evt_gap_connected_t *evt)
{
        ws_service_t *wss = (ws_service_t *) svc;

        if (wss->cb && wss->cb->indication_changed) {
                uint16_t ccc_val = 0x0000;

                ble_storage_get_u16(evt->conn_idx, wss->wm_ccc_h, &ccc_val);

                wss->cb->indication_changed(evt->conn_idx, ccc_val & GATT_CCC_INDICATIONS);
        }
}

static void handle_event_sent_evt(ble_service_t *svc, const ble_evt_gatts_event_sent_t *evt)
{
        ws_service_t *wss = (ws_service_t *) svc;

        if (wss->cb && wss->cb->indication_sent) {
                wss->cb->indication_sent(evt->conn_idx, evt->status);
        }
}

static void cleanup(ble_service_t *svc)
{
        ws_service_t *wss = (ws_service_t *) svc;

        ble_storage_remove_all(wss->wm_ccc_h);

        OS_FREE(wss);
}

ble_service_t *wss_init(const ble_service_config_t *config, wss_feature_t features,
                                                                        const wss_callbacks_t *cb)
{
        uint16_t num_attr;
        ws_service_t *wss;
        uint8_t value[4];
        att_uuid_t uuid;
        att_perm_t read_perm;

        wss = OS_MALLOC(sizeof(*wss));
        memset(wss, 0, sizeof(*wss));

        wss->svc.connected_evt = handle_connected_evt;
        wss->svc.read_req = handle_read_req;
        wss->svc.write_req = handle_write_req;
        wss->svc.event_sent = handle_event_sent_evt;
        wss->svc.cleanup = cleanup;
        wss->features = features;
        wss->cb = cb;

        // 2 characteristics and 1 descriptor
        num_attr = ble_service_get_num_attr(config, 2, 1);
        read_perm = ble_service_config_elevate_perm(ATT_PERM_READ, config);

        ble_uuid_create16(UUID_SERVICE_WSS, &uuid);
        ble_gatts_add_service(&uuid, config->service_type, num_attr);

        ble_service_config_add_includes(config);

        ble_uuid_create16(UUID_WEIGHT_SCALE_FEATURE, &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_READ, read_perm, 4, 0, NULL,
                                                                                &wss->features_h);

        ble_uuid_create16(UUID_WEIGHT_MEASUREMENT, &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_INDICATE, ATT_PERM_NONE, 15, 0, NULL,
                                                                                &wss->measurement_h);

        ble_uuid_create16(UUID_GATT_CLIENT_CHAR_CONFIGURATION, &uuid);
        ble_gatts_add_descriptor(&uuid, ATT_PERM_RW, 1, 0, &wss->wm_ccc_h);

        ble_gatts_register_service(&wss->svc.start_h, &wss->features_h, &wss->measurement_h,
                                                                                &wss->wm_ccc_h, 0);

        put_u32(value, features);
        ble_gatts_set_value(wss->features_h, 4, value);

        wss->svc.end_h = wss->svc.start_h + num_attr;

        ble_service_add(&wss->svc);

        return &wss->svc;
}

static uint16_t pack_measurement(ws_service_t *wss, const wss_weight_measurement_t *measurement,
                                                                                uint8_t value[])
{
        uint8_t *ptr = value;
        uint8_t flags = (measurement->unit == WSS_UNIT_SI) ? WSS_WM_FLAGS_UNITS_SI :
                                                                        WSS_WM_FLAGS_UNITS_IMPERIAL;

        if ((wss->features & WSS_FEAT_TIME_STAMP_SUPPORTED) && measurement->time_stamp_present) {
                flags |= WSS_WM_FLAGS_DATE_TIME_PRESENT;
        }

        if ((wss->features & WSS_FEAT_MULTI_USER_SUPPORTED)) {
                flags |= WSS_WM_FLAGS_USER_ID_PRESENT;
        }

        if ((wss->features & WSS_FEAT_BMI_SUPPORTED) && measurement->bmi > 0 &&
                                                                        measurement->height > 0) {
                flags |= WSS_WM_FLAGS_BMI_AND_HEIGHT_PRESENT;
        }

        put_u8_inc(&ptr, flags);
        put_u16_inc(&ptr, measurement->weight);

        if (flags & WSS_WM_FLAGS_DATE_TIME_PRESENT) {
                pack_date_time(&measurement->time_stamp, &ptr);
        }

        if (flags & WSS_WM_FLAGS_USER_ID_PRESENT) {
                put_u8_inc(&ptr, measurement->user_id);
        }

        if (flags & WSS_WM_FLAGS_BMI_AND_HEIGHT_PRESENT) {
                put_u16_inc(&ptr, measurement->bmi);
                put_u16_inc(&ptr, measurement->height);
        }

        return ptr - value;
}

static ble_error_t send_weight_indication(ws_service_t *wss, uint16_t conn_idx,
                                                const wss_weight_measurement_t *measurement)
{
        uint8_t value[15];
        uint16_t len = pack_measurement(wss, measurement, value);

        return ble_gatts_send_event(conn_idx, wss->measurement_h, GATT_EVENT_INDICATION, len, value);
}

ble_error_t wss_indicate_weight(ble_service_t *svc, uint16_t conn_idx,
                                                const wss_weight_measurement_t *measurement)
{
        ws_service_t *wss = (ws_service_t *) svc;

        if (!wss_is_indication_enabled(svc, conn_idx)) {
                return BLE_ERROR_NOT_ALLOWED;
        }

        return send_weight_indication(wss, conn_idx, measurement);
}

struct indicate_cb_ud {
        ws_service_t   *wss;
        const uint8_t  *value;
        uint16_t        value_len;
};

void wss_indicate_weight_all(ble_service_t *svc, const wss_weight_measurement_t *meas)
{
        uint8_t num_conn;
        uint16_t *conn_idx;

        ble_gap_get_connected(&num_conn, &conn_idx);

        while (num_conn--) {
                wss_indicate_weight(svc, conn_idx[num_conn], meas);
        }

        if (conn_idx) {
                OS_FREE(conn_idx);
        }
}

bool wss_is_indication_enabled(ble_service_t *svc, uint16_t conn_idx)
{
        ws_service_t *wss = (ws_service_t *) svc;
        uint16_t ccc_val = 0x0000;

        ble_storage_get_u16(conn_idx, wss->wm_ccc_h, &ccc_val);

        return (ccc_val & GATT_CCC_INDICATIONS);
}
