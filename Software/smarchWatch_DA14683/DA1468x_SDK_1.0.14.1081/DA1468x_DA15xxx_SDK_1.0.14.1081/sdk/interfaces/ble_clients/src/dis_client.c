/**
 ****************************************************************************************
 *
 * @file dis_client.c
 *
 * @brief Device Information Service Client implementation
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
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
#include "dis_client.h"

#define UUID_MANUFACTURER_NAME  (0x2A29)
#define UUID_MODEL_NUMBER       (0x2A24)
#define UUID_SERIAL_NUMBER      (0x2A25)
#define UUID_HARDWARE_REVISION  (0x2A27)
#define UUID_FIRMWARE_REVISION  (0x2A26)
#define UUID_SOFTWARE_REVISION  (0x2A28)
#define UUID_SYSTEM_ID          (0x2A23)
#define UUID_REG_CERT_DATA_LIST (0x2A2A)
#define UUID_PNP_ID             (0x2A50)

typedef struct {
        ble_client_t client;

        const dis_client_callbacks_t *cb;

        uint16_t manufacturer_name_h;
        uint16_t model_number_h;
        uint16_t serial_number_h;
        uint16_t hardware_revision_h;
        uint16_t firmware_revision_h;
        uint16_t software_revision_h;
        uint16_t system_id_h;
        uint16_t reg_cert_data_list_h;
        uint16_t pnp_id_h;
} dis_client_t;

/* Type used for client serialization */
typedef struct {
        uint16_t manufacturer_name_h;
        uint16_t model_number_h;
        uint16_t serial_number_h;
        uint16_t hardware_revision_h;
        uint16_t firmware_revision_h;
        uint16_t software_revision_h;
        uint16_t system_id_h;
        uint16_t reg_cert_data_list_h;
        uint16_t pnp_id_h;
} __attribute__((packed)) dis_client_serialized_t;

static void handle_disconnected_evt(ble_client_t *client, const ble_evt_gap_disconnected_t *evt)
{
        client->conn_idx = BLE_CONN_IDX_INVALID;

        ble_client_remove(client);
}

static dis_client_cap_t get_char_for_handle(dis_client_t *dis_client, uint16_t handle)
{
        if (handle == 0) {
                return 0;
        }

        if (handle == dis_client->manufacturer_name_h) {
                return DIS_CLIENT_CAP_MANUFACTURER_NAME;
        }

        if (handle == dis_client->model_number_h) {
                return DIS_CLIENT_CAP_MODEL_NUMBER;
        }

        if (handle == dis_client->serial_number_h) {
                return DIS_CLIENT_CAP_SERIAL_NUMBER;
        }

        if (handle == dis_client->hardware_revision_h) {
                return DIS_CLIENT_CAP_HARDWARE_REVISION;
        }

        if (handle == dis_client->firmware_revision_h) {
                return DIS_CLIENT_CAP_FIRMWARE_REVISION;
        }

        if (handle == dis_client->software_revision_h) {
                return DIS_CLIENT_CAP_SOFTWARE_REVISION;
        }

        if (handle == dis_client->system_id_h) {
                return DIS_CLIENT_CAP_SYSTEM_ID;
        }

        if (handle == dis_client->reg_cert_data_list_h) {
                return DIS_CLIENT_CAP_REG_CERT;
        }

        if (handle == dis_client->pnp_id_h) {
                return DIS_CLIENT_CAP_PNP_ID;
        }

        return 0;
}

static uint16_t get_handle_for_char(dis_client_t *dis_client, dis_client_cap_t capability)
{
        uint16_t handle;

        switch (capability) {
        case DIS_CLIENT_CAP_MANUFACTURER_NAME:
                handle = dis_client->manufacturer_name_h;
                break;
        case DIS_CLIENT_CAP_MODEL_NUMBER:
                handle = dis_client->model_number_h;
                break;
        case DIS_CLIENT_CAP_SERIAL_NUMBER:
                handle = dis_client->serial_number_h;
                break;
        case DIS_CLIENT_CAP_HARDWARE_REVISION:
                handle = dis_client->hardware_revision_h;
                break;
        case DIS_CLIENT_CAP_FIRMWARE_REVISION:
                handle = dis_client->firmware_revision_h;
                break;
        case DIS_CLIENT_CAP_SOFTWARE_REVISION:
                handle = dis_client->software_revision_h;
                break;
        case DIS_CLIENT_CAP_SYSTEM_ID:
                handle = dis_client->system_id_h;
                break;
        case DIS_CLIENT_CAP_REG_CERT:
                handle = dis_client->reg_cert_data_list_h;
                break;
        case DIS_CLIENT_CAP_PNP_ID:
                handle = dis_client->pnp_id_h;
                break;
        default:
                handle = 0;
                break;
        }

        return handle;
}

static void handle_read_completed_evt(ble_client_t *client,
                                                        const ble_evt_gattc_read_completed_t *evt)
{
        dis_client_t *dis_client = (dis_client_t *) client;
        dis_client_cap_t capability;

        capability = get_char_for_handle(dis_client, evt->handle);
        if (!capability) {
                return;
        }

        if (!dis_client->cb->read_completed) {
                return;
        }

        dis_client->cb->read_completed(client, evt->status, capability, evt->length,
                                                                                evt->value);
}

static void cleanup(ble_client_t *client)
{
        OS_FREE(client);
}

static void serialize(ble_client_t *client, void *data, size_t *length)
{
        dis_client_t *dis_client = (dis_client_t *) client;
        dis_client_serialized_t *s_data = data;
        *length = sizeof(dis_client_serialized_t);

        if (!data) {
                return;
        }

        s_data->manufacturer_name_h  = dis_client->manufacturer_name_h;
        s_data->model_number_h       = dis_client->model_number_h;
        s_data->serial_number_h      = dis_client->serial_number_h;
        s_data->hardware_revision_h  = dis_client->hardware_revision_h;
        s_data->firmware_revision_h  = dis_client->firmware_revision_h;
        s_data->software_revision_h  = dis_client->software_revision_h;
        s_data->system_id_h          = dis_client->system_id_h;
        s_data->reg_cert_data_list_h = dis_client->reg_cert_data_list_h;
        s_data->pnp_id_h             = dis_client->pnp_id_h;
}

static dis_client_t *init(uint16_t conn_idx, const dis_client_callbacks_t *cb)
{
        dis_client_t *dis_client;

        dis_client = OS_MALLOC(sizeof(*dis_client));
        memset(dis_client, 0, sizeof(*dis_client));

        dis_client->cb                        = cb;
        dis_client->client.disconnected_evt   = handle_disconnected_evt;
        dis_client->client.read_completed_evt = handle_read_completed_evt;
        dis_client->client.cleanup            = cleanup;
        dis_client->client.serialize          = serialize;
        dis_client->client.conn_idx           = conn_idx;

        return dis_client;
}

ble_client_t *dis_client_init(const dis_client_callbacks_t *cb,
                                                        const ble_evt_gattc_browse_svc_t *evt)
{
        dis_client_t *dis_client;
        const gattc_item_t *item;
        att_uuid_t uuid;

        if (!cb) {
                return NULL;
        }

        ble_uuid_create16(UUID_SERVICE_DIS, &uuid);
        if (!ble_uuid_equal(&uuid, &evt->uuid)) {
                return NULL;
        }

        dis_client = init(evt->conn_idx, cb);

        ble_gattc_util_find_init(evt);

        /* Manufacturer Name Characeristic */
        ble_uuid_create16(UUID_MANUFACTURER_NAME, &uuid);
        item = ble_gattc_util_find_characteristic(&uuid);
        if (item && (item->c.properties & GATT_PROP_READ)) {
                dis_client->manufacturer_name_h = item->c.value_handle;
        }

        /* Model Number Characteristic */
        ble_uuid_create16(UUID_MODEL_NUMBER, &uuid);
        item = ble_gattc_util_find_characteristic(&uuid);
        if (item && (item->c.properties & GATT_PROP_READ)) {
                dis_client->model_number_h = item->c.value_handle;
        }

        /* Serial Number Characteristic */
        ble_uuid_create16(UUID_SERIAL_NUMBER, &uuid);
        item = ble_gattc_util_find_characteristic(&uuid);
        if (item && (item->c.properties & GATT_PROP_READ)) {
                dis_client->serial_number_h = item->c.value_handle;
        }

        /* Hardware Revision Characteristic */
        ble_uuid_create16(UUID_HARDWARE_REVISION, &uuid);
        item = ble_gattc_util_find_characteristic(&uuid);
        if (item && (item->c.properties & GATT_PROP_READ)) {
                dis_client->hardware_revision_h = item->c.value_handle;
        }

        /* Firmware Revision Characteristic */
        ble_uuid_create16(UUID_FIRMWARE_REVISION, &uuid);
        item = ble_gattc_util_find_characteristic(&uuid);
        if (item && (item->c.properties & GATT_PROP_READ)) {
                dis_client->firmware_revision_h = item->c.value_handle;
        }

        /* Software Revision Characteristic */
        ble_uuid_create16(UUID_SOFTWARE_REVISION, &uuid);
        item = ble_gattc_util_find_characteristic(&uuid);
        if (item && (item->c.properties & GATT_PROP_READ)) {
                dis_client->software_revision_h = item->c.value_handle;
        }

        /* System ID Characteristic */
        ble_uuid_create16(UUID_SYSTEM_ID, &uuid);
        item = ble_gattc_util_find_characteristic(&uuid);
        if (item && (item->c.properties & GATT_PROP_READ)) {
                dis_client->system_id_h = item->c.value_handle;
        }

        /* IEEE 11073-20601 Regulatory Certification Data List Characteristic */
        ble_uuid_create16(UUID_REG_CERT_DATA_LIST, &uuid);
        item = ble_gattc_util_find_characteristic(&uuid);
        if (item && (item->c.properties & GATT_PROP_READ)) {
                dis_client->reg_cert_data_list_h = item->c.value_handle;
        }

        /* PNP ID Characteristic */
        ble_uuid_create16(UUID_PNP_ID, &uuid);
        item = ble_gattc_util_find_characteristic(&uuid);
        if (item && (item->c.properties & GATT_PROP_READ)) {
                dis_client->pnp_id_h = item->c.value_handle;
        }

        return &dis_client->client;
}

ble_client_t *dis_client_init_from_data(uint16_t conn_idx, const dis_client_callbacks_t *cb,
                                                                    const void *data, size_t length)
{
        dis_client_t *dis_client;
        const dis_client_serialized_t *d = data;

        if (!data || (length < sizeof(dis_client_serialized_t))) {
                return NULL;
        }

        dis_client = init(conn_idx, cb);

        dis_client->manufacturer_name_h  = d->manufacturer_name_h;
        dis_client->model_number_h       = d->model_number_h;
        dis_client->serial_number_h      = d->serial_number_h;
        dis_client->hardware_revision_h  = d->hardware_revision_h;
        dis_client->firmware_revision_h  = d->firmware_revision_h;
        dis_client->software_revision_h  = d->software_revision_h;
        dis_client->system_id_h          = d->system_id_h;
        dis_client->reg_cert_data_list_h = d->reg_cert_data_list_h;
        dis_client->pnp_id_h             = d->pnp_id_h;

        ble_client_attach(&dis_client->client, conn_idx);

        return &dis_client->client;
}

dis_client_cap_t dis_client_get_capabilities(ble_client_t *client)
{
        dis_client_t *dis_client = (dis_client_t *) client;
        dis_client_cap_t supported_char = 0;

        supported_char |= get_char_for_handle(dis_client, dis_client->manufacturer_name_h);
        supported_char |= get_char_for_handle(dis_client, dis_client->model_number_h);
        supported_char |= get_char_for_handle(dis_client, dis_client->serial_number_h);
        supported_char |= get_char_for_handle(dis_client, dis_client->hardware_revision_h);
        supported_char |= get_char_for_handle(dis_client, dis_client->firmware_revision_h);
        supported_char |= get_char_for_handle(dis_client, dis_client->software_revision_h);
        supported_char |= get_char_for_handle(dis_client, dis_client->system_id_h);
        supported_char |= get_char_for_handle(dis_client, dis_client->reg_cert_data_list_h);
        supported_char |= get_char_for_handle(dis_client, dis_client->pnp_id_h);

        return supported_char;
}

bool dis_client_read(ble_client_t *client, dis_client_cap_t capability)
{
        dis_client_t *dis_client = (dis_client_t *) client;
        ble_error_t status;
        uint16_t handle;

        handle = get_handle_for_char(dis_client, capability);
        if (!handle) {
                return false;
        }

        status = ble_gattc_read(dis_client->client.conn_idx, handle, 0);
        if (status != BLE_STATUS_OK) {
                return false;
        }

        return true;
}
