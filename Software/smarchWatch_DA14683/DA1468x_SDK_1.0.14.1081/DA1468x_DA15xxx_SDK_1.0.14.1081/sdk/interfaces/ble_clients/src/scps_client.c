/**
 ****************************************************************************************
 *
 * @file scps_client.c
 *
 * @brief Scan Parameters Service Client implementation
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include "osal.h"
#include "ble_bufops.h"
#include "ble_uuid.h"
#include "ble_gatt.h"
#include "ble_gattc.h"
#include "ble_gattc_util.h"
#include "ble_client.h"
#include "scps_client.h"

#define UUID_SCAN_INTERVAL_WINDOW       (0x2A4F)
#define UUID_SCAN_REFRESH               (0x2A31)

#define SERVER_REQUIRES_REFRESH         (0x00)

typedef struct {
        ble_client_t client;
        const scps_client_callbacks_t *cb;

        uint16_t scan_interval_window_h;
        uint16_t scan_refresh_h;
        uint16_t scan_refresh_ccc_h;
} scps_client_t;

static void handle_notification_evt(ble_client_t *client, const ble_evt_gattc_notification_t *evt)
{
        scps_client_t *scps_client = (scps_client_t *) client;

        if (evt->handle == scps_client->scan_refresh_h) {
                if (!scps_client->cb->refresh_notif) {
                        return;
                }

                if ((evt->length != sizeof(uint8_t)) || (evt->value[0] != SERVER_REQUIRES_REFRESH)) {
                        return;
                }

                scps_client->cb->refresh_notif(client);
        }
}

static void handle_write_completed_evt(ble_client_t *client,
                                        const ble_evt_gattc_write_completed_t *evt)
{
        scps_client_t *scps_client = (scps_client_t *) client;
        uint16_t handle = evt->handle;

        if (handle == scps_client->scan_refresh_ccc_h) {
                if (!scps_client->cb->set_event_state_completed) {
                        return;
                }

                scps_client->cb->set_event_state_completed(client,
                                                                SCPS_CLIENT_EVENT_REFRESH_NOTIF,
                                                                evt->status);
        }
}

static void handle_refresh_ccc_value(scps_client_t *scps_client, att_error_t status,
                                                uint16_t length, const uint8_t *value)
{
        uint16_t ccc = 0;

        if (!scps_client->cb->get_event_state_completed) {
                return;
        }

        if (status != ATT_ERROR_OK) {
                goto done;
        }

        if (length != sizeof(ccc)) {
                status = ATT_ERROR_INVALID_VALUE_LENGTH;
                goto done;
        }

        ccc = get_u16(value);

done:
        scps_client->cb->get_event_state_completed(&scps_client->client,
                                                        SCPS_CLIENT_EVENT_REFRESH_NOTIF, status,
                                                        ccc & GATT_CCC_NOTIFICATIONS);
}

static void handle_read_completed_evt(ble_client_t *client,
                                                        const ble_evt_gattc_read_completed_t *evt)
{
        scps_client_t *scps_client = (scps_client_t *) client;
        uint16_t handle = evt->handle;

        if (handle == scps_client->scan_refresh_ccc_h) {
                handle_refresh_ccc_value(scps_client, evt->status, evt->length, evt->value);
        }
}

static void cleanup(ble_client_t *scps_client)
{
        OS_FREE(scps_client);
}

ble_client_t *scps_client_init(const scps_client_callbacks_t *cb,
                                                        const ble_evt_gattc_browse_svc_t *evt)
{
        scps_client_t *scps_client;
        const gattc_item_t *item;
        att_uuid_t uuid;

        if (!cb) {
                return NULL;
        }

        ble_uuid_create16(UUID_SERVICE_SCPS, &uuid);
        if (!ble_uuid_equal(&uuid, &evt->uuid)) {
                return NULL;
        }

        scps_client = OS_MALLOC(sizeof(*scps_client));
        memset(scps_client, 0, sizeof(*scps_client));
        scps_client->client.conn_idx = evt->conn_idx;
        scps_client->client.read_completed_evt = handle_read_completed_evt;
        scps_client->client.write_completed_evt = handle_write_completed_evt;
        scps_client->client.notification_evt = handle_notification_evt;
        scps_client->client.cleanup = cleanup;
        scps_client->cb = cb;

        ble_gattc_util_find_init(evt);

        ble_uuid_create16(UUID_SCAN_INTERVAL_WINDOW, &uuid);
        item = ble_gattc_util_find_characteristic(&uuid);
        if (item && (item->c.properties & GATT_PROP_WRITE_NO_RESP)) {
                scps_client->scan_interval_window_h = item->c.value_handle;
        }

        ble_uuid_create16(UUID_SCAN_REFRESH, &uuid);
        item = ble_gattc_util_find_characteristic(&uuid);
        if (item && (item->c.properties & GATT_PROP_NOTIFY)) {
                scps_client->scan_refresh_h = item->c.value_handle;

                ble_uuid_create16(UUID_GATT_CLIENT_CHAR_CONFIGURATION, &uuid);
                item = ble_gattc_util_find_descriptor(&uuid);
                scps_client->scan_refresh_ccc_h = item ? item->handle : 0;
        }

        /* Check if mandatory characteristics were found */
        if (!scps_client->scan_interval_window_h ||
                        (scps_client->scan_refresh_h && !scps_client->scan_refresh_ccc_h)) {
                cleanup(&scps_client->client);
                return NULL;
        }

        return &scps_client->client;
}

scps_client_cap_t scps_client_get_capabilities(ble_client_t *client)
{
        scps_client_t *scps_client = (scps_client_t *) client;
        scps_client_cap_t cap = 0;

        if (scps_client->scan_refresh_h) {
                cap |= SCPS_CLIENT_CAP_REFRESH;
        }

        return cap;
}

bool scps_client_get_event_state(ble_client_t *client, scps_client_event_t event)
{
        scps_client_t *scps_client = (scps_client_t *) client;
        ble_error_t status;
        uint16_t handle;

        switch (event) {
        case SCPS_CLIENT_EVENT_REFRESH_NOTIF:
                handle = scps_client->scan_refresh_ccc_h;
                break;
        default:
                handle = 0;
                break;
        }

        if (!handle) {
                return false;
        }

        status = ble_gattc_read(client->conn_idx, handle, 0);
        if (status != BLE_STATUS_OK) {
                return false;
        }

        return true;
}

bool scps_client_set_event_state(ble_client_t *client, scps_client_event_t event, bool enable)
{
        scps_client_t *scps_client = (scps_client_t *) client;
        ble_error_t status;
        uint16_t handle;

        switch (event) {
        case SCPS_CLIENT_EVENT_REFRESH_NOTIF:
                handle = scps_client->scan_refresh_ccc_h;
                break;
        default:
                handle = 0;
                break;
        }

        if (!handle) {
                return false;
        }

        status = ble_gattc_util_write_ccc(client->conn_idx, handle,
                                                enable ? GATT_CCC_NOTIFICATIONS : GATT_CCC_NONE);
        if (status != BLE_STATUS_OK) {
                return false;
        }

        return true;
}

bool scps_client_write_scan_interval_window(ble_client_t *client, uint16_t scan_interval,
                                                                        uint16_t scan_window)
{
        scps_client_t *scps_client = (scps_client_t *) client;
        ble_error_t status;
        uint8_t value[4];

        put_u16(value, scan_interval);
        put_u16(&value[2], scan_window);

        status = ble_gattc_write_no_resp(scps_client->client.conn_idx,
                                scps_client->scan_interval_window_h, false, sizeof(value), value);

        return (status == BLE_STATUS_OK);
}
