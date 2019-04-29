/**
 ****************************************************************************************
 *
 * @file gatt_client.c
 *
 * @brief Generic Attribute Service Client implementation
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
#include "gatt_client.h"

#define UUID_SERVICE_CHANGED      (0x2A05)

typedef struct {
        ble_client_t client;

        const gatt_client_callbacks_t *cb;

        uint16_t service_changed_h;
        uint16_t service_changed_ccc_h;
} gatt_client_t;

static void cleanup(ble_client_t *client)
{
        OS_FREE(client);
}

static void handle_service_changed_ccc_value(gatt_client_t *gatt_client, att_error_t status,
                                                        uint16_t length, const uint8_t *value)
{
        uint16_t ccc = 0;

        if (!gatt_client->cb->get_event_state_completed) {
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
        gatt_client->cb->get_event_state_completed(&gatt_client->client,
                                                        GATT_CLIENT_EVENT_SERVICE_CHANGED_INDICATE,
                                                        status, ccc & GATT_CCC_INDICATIONS);
}

static void handle_read_completed_evt(ble_client_t *client,
                                        const ble_evt_gattc_read_completed_t *evt)
{
        gatt_client_t *gatt_client = (gatt_client_t *) client;
        uint16_t handle = evt->handle;

        if (handle == gatt_client->service_changed_ccc_h) {
                handle_service_changed_ccc_value(gatt_client, evt->status, evt->length,
                                                                                evt->value);
        }
}

static void handle_write_completed_evt(ble_client_t *client,
                                        const ble_evt_gattc_write_completed_t *evt)
{
        gatt_client_t *gatt_client = (gatt_client_t *) client;
        uint16_t handle = evt->handle;

        if (handle == gatt_client->service_changed_ccc_h) {
                if (!gatt_client->cb->set_event_state_completed) {
                        return;
                }

                gatt_client->cb->set_event_state_completed(client,
                                                GATT_CLIENT_EVENT_SERVICE_CHANGED_INDICATE,
                                                evt->status);
        }
}

static void handle_service_changed_value(gatt_client_t *gatt_client, uint16_t length,
                                                                        const uint8_t *value)
{
        uint16_t start_h, end_h;

        if (!gatt_client->cb->service_changed) {
                return;
        }

        if (length != (sizeof(uint16_t) * 2)) {
                return;
        }

        start_h = get_u16_inc(&value);
        end_h = get_u16(value);

        gatt_client->cb->service_changed(&gatt_client->client, start_h, end_h);
}

static void handle_indication_evt(ble_client_t *client, const ble_evt_gattc_indication_t *evt)
{
        gatt_client_t *gatt_client = (gatt_client_t *) client;
        uint16_t handle = evt->handle;

        if (handle == gatt_client->service_changed_h) {
                handle_service_changed_value(gatt_client, evt->length, evt->value);
        }
}

static void handle_disconnected_evt(ble_client_t *client, const ble_evt_gap_disconnected_t *evt)
{
        client->conn_idx = BLE_CONN_IDX_INVALID;

        ble_client_remove(client);
}

ble_client_t *gatt_client_init(const gatt_client_callbacks_t *cb,
                                                        const ble_evt_gattc_browse_svc_t *evt)
{
        gatt_client_t *gatt_client;
        const gattc_item_t *item;
        att_uuid_t uuid;

        if (!cb) {
                return NULL;
        }

        ble_uuid_create16(UUID_SERVICE_GATT, &uuid);
        if (!ble_uuid_equal(&uuid, &evt->uuid)) {
                return NULL;
        }

        gatt_client = OS_MALLOC(sizeof(*gatt_client));
        memset(gatt_client, 0, sizeof(*gatt_client));
        gatt_client->client.conn_idx = evt->conn_idx;
        gatt_client->client.cleanup = cleanup;
        gatt_client->client.write_completed_evt = handle_write_completed_evt;
        gatt_client->client.read_completed_evt = handle_read_completed_evt;
        gatt_client->client.indication_evt = handle_indication_evt;
        gatt_client->client.disconnected_evt = handle_disconnected_evt;
        gatt_client->cb = cb;

        ble_gattc_util_find_init(evt);

        ble_uuid_create16(UUID_SERVICE_CHANGED, &uuid);
        item = ble_gattc_util_find_characteristic(&uuid);
        if (item) {
                gatt_client->service_changed_h = item->c.value_handle;

                ble_uuid_create16(UUID_GATT_CLIENT_CHAR_CONFIGURATION, &uuid);
                item = ble_gattc_util_find_descriptor(&uuid);
                gatt_client->service_changed_ccc_h = item ? item->handle : 0;
        }

        /* Check if CCC descriptor is provided to Service Changed */
        if (gatt_client->service_changed_h && !gatt_client->service_changed_ccc_h) {
                cleanup(&gatt_client->client);
                return NULL;
        }

        return &gatt_client->client;
}

gatt_client_cap_t gatt_client_get_capabilites(ble_client_t *client)
{
        gatt_client_t *gatt_client = (gatt_client_t *) client;
        gatt_client_cap_t cap = 0;

        if (gatt_client->service_changed_h != 0) {
                cap |= GATT_CLIENT_CAP_SERVICE_CHANGED;
        }

        return cap;
}

bool gatt_client_set_event_state(ble_client_t *client, gatt_client_event_t event, bool enable)
{
        gatt_client_t *gatt_client = (gatt_client_t *) client;
        ble_error_t status;
        uint16_t handle;

        switch (event) {
        case GATT_CLIENT_EVENT_SERVICE_CHANGED_INDICATE:
                handle = gatt_client->service_changed_ccc_h;
                break;
        default:
                handle = 0;
                break;
        }

        if (handle == 0) {
                return false;
        }

        status = ble_gattc_util_write_ccc(client->conn_idx, handle,
                                                enable ? GATT_CCC_INDICATIONS : GATT_CCC_NONE);
        if (status != BLE_STATUS_OK) {
                return false;
        }

        return true;
}

bool gatt_client_get_event_state(ble_client_t *client, gatt_client_event_t event)
{
        gatt_client_t *gatt_client = (gatt_client_t *) client;
        bool status;
        uint16_t handle;

        switch (event) {
        case GATT_CLIENT_EVENT_SERVICE_CHANGED_INDICATE:
                handle = gatt_client->service_changed_ccc_h;
                break;
        default:
                handle = 0;
                break;
        }

        if (handle == 0) {
                return false;
        }

        status = ble_gattc_read(gatt_client->client.conn_idx, handle, 0);
        if (status != BLE_STATUS_OK) {
                return false;
        }

        return true;
}
