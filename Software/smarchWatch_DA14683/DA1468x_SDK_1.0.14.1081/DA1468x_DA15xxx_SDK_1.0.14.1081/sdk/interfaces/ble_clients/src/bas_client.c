/**
 ****************************************************************************************
 *
 * @file bas_client.c
 *
 * @brief Battery Service Client implementation
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
#include "bas_client.h"

#define UUID_BATTERY_LEVEL      (0x2A19)

typedef struct {
        ble_client_t client;
        const bas_client_callbacks_t *cb;

        uint16_t battery_level_h;
        uint16_t battery_level_ccc_h;
} bas_client_t;

static void handle_disconnected_evt(ble_client_t *client, const ble_evt_gap_disconnected_t *evt)
{
        client->conn_idx = BLE_CONN_IDX_INVALID;

        ble_client_remove(client);
}

static void handle_notification_evt(ble_client_t *client, const ble_evt_gattc_notification_t *evt)
{
        bas_client_t *bas_client = (bas_client_t *) client;

        if (evt->handle == bas_client->battery_level_h) {
                if (!bas_client->cb->battery_level_notif) {
                        return;
                }

                if (evt->length != sizeof(uint8_t)) {
                        return;
                }

                bas_client->cb->battery_level_notif(client, evt->value[0]);
        }
}

static void handle_write_completed_evt(ble_client_t *client,
                                        const ble_evt_gattc_write_completed_t *evt)
{
        bas_client_t *bas_client = (bas_client_t *) client;
        uint16_t handle = evt->handle;

        if (handle == bas_client->battery_level_ccc_h) {
                if (!bas_client->cb->set_event_state_completed) {
                        return;
                }

                bas_client->cb->set_event_state_completed(client,
                                                        BAS_CLIENT_EVENT_BATTERY_LEVEL_NOTIFY,
                                                        evt->status);
        }
}

static void handle_battery_level_value(bas_client_t *bas_client, att_error_t status,
                                                uint16_t length, const uint8_t *value)
{
        uint8_t level = 0;

        if (!bas_client->cb->read_battery_level_completed) {
                return;
        }

        if (status != ATT_ERROR_OK) {
                goto done;
        }

        if (length != sizeof(level)) {
                status = ATT_ERROR_INVALID_VALUE_LENGTH;
                goto done;
        }

        level = value[0];

done:
        bas_client->cb->read_battery_level_completed(&bas_client->client, status, level);
}

static void handle_battery_level_ccc_value(bas_client_t *bas_client, att_error_t status,
                                                uint16_t length, const uint8_t *value)
{
        uint16_t ccc = 0;

        if (!bas_client->cb->get_event_state_completed) {
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
        bas_client->cb->get_event_state_completed(&bas_client->client,
                                                        BAS_CLIENT_EVENT_BATTERY_LEVEL_NOTIFY,
                                                        status, ccc & GATT_CCC_NOTIFICATIONS);
}

static void handle_read_completed_evt(ble_client_t *client,
                                                        const ble_evt_gattc_read_completed_t *evt)
{
        bas_client_t *bas_client = (bas_client_t *) client;
        uint16_t handle = evt->handle;

        if (handle == bas_client->battery_level_h) {
                handle_battery_level_value(bas_client, evt->status, evt->length, evt->value);
        } else if (handle == bas_client->battery_level_ccc_h) {
                handle_battery_level_ccc_value(bas_client, evt->status, evt->length, evt->value);
        }
}

static void cleanup(ble_client_t *bas_client)
{
        OS_FREE(bas_client);
}

ble_client_t *bas_client_init(const bas_client_callbacks_t *cb,
                                                        const ble_evt_gattc_browse_svc_t *evt)
{
        bas_client_t *bas_client;
        const gattc_item_t *item;
        att_uuid_t uuid;

        if (!cb) {
                return NULL;
        }

        ble_uuid_create16(UUID_SERVICE_BAS, &uuid);
        if (!ble_uuid_equal(&uuid, &evt->uuid)) {
                return NULL;
        }

        bas_client = OS_MALLOC(sizeof(*bas_client));
        memset(bas_client, 0, sizeof(*bas_client));
        bas_client->client.conn_idx = evt->conn_idx;
        bas_client->client.read_completed_evt = handle_read_completed_evt;
        bas_client->client.write_completed_evt = handle_write_completed_evt;
        bas_client->client.notification_evt = handle_notification_evt;
        bas_client->client.disconnected_evt = handle_disconnected_evt;
        bas_client->client.cleanup = cleanup;
        bas_client->cb = cb;

        ble_gattc_util_find_init(evt);

        ble_uuid_create16(UUID_BATTERY_LEVEL, &uuid);
        item = ble_gattc_util_find_characteristic(&uuid);
        if (item && (item->c.properties & GATT_PROP_READ)) {
                bas_client->battery_level_h = item->c.value_handle;

                /* Look for CCC descriptor if NOTIFY prop is supported */
                if (item->c.properties & GATT_PROP_NOTIFY) {
                        ble_uuid_create16(UUID_GATT_CLIENT_CHAR_CONFIGURATION, &uuid);
                        item = ble_gattc_util_find_descriptor(&uuid);
                        bas_client->battery_level_ccc_h = item ? item->handle : 0;
                }
        }

        /* Check if mandatory characteristic was found */
        if (!bas_client->battery_level_h) {
                cleanup(&bas_client->client);
                return NULL;
        }

        return &bas_client->client;
}

bas_client_cap_t bas_client_get_capabilities(ble_client_t *client)
{
        bas_client_t *bas_client = (bas_client_t *) client;
        bas_client_cap_t cap = 0;

        if (bas_client->battery_level_ccc_h) {
                cap |= BAS_CLIENT_CAP_BATTERY_LEVEL_NOTIFICATION;
        }

        return cap;
}

bool bas_client_read_battery_level(ble_client_t *client)
{
        bas_client_t *bas_client = (bas_client_t *) client;
        ble_error_t status;

        status = ble_gattc_read(bas_client->client.conn_idx, bas_client->battery_level_h, 0);
        if (status != BLE_STATUS_OK) {
                return false;
        }

        return true;
}

bool bas_client_set_event_state(ble_client_t *client, bas_client_event_t event, bool enable)
{
        bas_client_t *bas_client = (bas_client_t *) client;
        ble_error_t status;
        uint16_t handle;

        switch (event) {
        case BAS_CLIENT_EVENT_BATTERY_LEVEL_NOTIFY:
                handle = bas_client->battery_level_ccc_h;
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

bool bas_client_get_event_state(ble_client_t *client, bas_client_event_t event)
{
        bas_client_t *bas_client = (bas_client_t *) client;
        ble_error_t status;
        uint16_t handle;

        switch (event) {
        case BAS_CLIENT_EVENT_BATTERY_LEVEL_NOTIFY:
                handle = bas_client->battery_level_ccc_h;
                break;
        default:
                handle = 0;
                break;
        }

        if (!handle) {
                return false;
        }

        status = ble_gattc_read(bas_client->client.conn_idx, handle, 0);
        if (status != BLE_STATUS_OK) {
                return false;
        }

        return true;
}
