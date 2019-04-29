/**
 ****************************************************************************************
 *
 * @file hrs_client.c
 *
 * @brief Heart Rate Service Client implementation
 *
 * Copyright (C) 2016 Dialog Semiconductor.
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
#include "hrs_client.h"

#define UUID_HEART_RATE_MEASUREMENT     (0x2A37)
#define UUID_BODY_SENSOR_LOCATION       (0x2A38)
#define UUID_HEART_RATE_CONTROL_POINT   (0x2A39)

typedef struct {
        ble_client_t client;

        // handles
        uint16_t meas_h;
        uint16_t meas_ccc_h;
        uint16_t body_sensor_location_h;
        uint16_t control_point_h;

        // callbacks
        const hrs_client_callbacks_t *cb;

        // client capabilities
        hrs_client_cap_t capabilities;
} hrs_client_t;

enum measurement_flags {
        HRM_FLAG_VAL_16BIT                      = 0x01, // 16bit format for Heart Rate Value
        HRM_FLAG_SENSOR_CONTACT_DETECTED        = 0x02, // Sensor Contact feature is supported and contact is detected
        HRM_FLAG_SENSOR_CONTACT_SUPPORTED       = 0x04, // Sensor Contact feature is supported but contact is not detected
        HRM_FLAG_ENERGY_EXPENDED_PRESENT        = 0x08, // Energy Expended field is present
        HRM_FLAG_RR_INTERVAL_PRESENT            = 0x10, // One or more RR-Interval values are present
};

/* Type used for client serialization */
typedef struct {
        uint16_t meas_h;
        uint16_t meas_ccc_h;

        uint16_t body_sensor_location_h;
        uint16_t control_point_h;

        uint8_t capabilities;
} __attribute__((packed)) hrs_client_serialized_t;

static void serialize(ble_client_t *client, void *data, size_t *length)
{
        hrs_client_t *hrs_client = (hrs_client_t *) client;
        hrs_client_serialized_t *s_data = data;
        *length = sizeof(hrs_client_serialized_t);

        if (!data) {
                return;
        }

        s_data->meas_h                  = hrs_client->meas_h;
        s_data->meas_ccc_h              = hrs_client->meas_ccc_h;
        s_data->body_sensor_location_h  = hrs_client->body_sensor_location_h;
        s_data->control_point_h         = hrs_client->control_point_h;
        s_data->capabilities            = hrs_client->capabilities;
}

hrs_client_cap_t hrs_client_get_capabilities(ble_client_t *client)
{
        hrs_client_t *hrs_client = (hrs_client_t *) client;

        return hrs_client->capabilities;
}

bool hrs_client_read_body_sensor_location(ble_client_t *client)
{
        hrs_client_t *hrs_client = (hrs_client_t *) client;
        ble_error_t ret = BLE_ERROR_FAILED;

        if (hrs_client->body_sensor_location_h) {
                ret = ble_gattc_read(hrs_client->client.conn_idx, hrs_client->body_sensor_location_h, 0);
        }

        return ret == BLE_STATUS_OK;
}

bool hrs_client_reset_energy_expended(ble_client_t *client)
{
        hrs_client_t *hrs_client = (hrs_client_t *) client;
        ble_error_t ret = BLE_ERROR_FAILED;
        const uint8_t cp_val_reset_ee = 0x01;

        if (hrs_client->capabilities & HRS_CLIENT_CAP_HEART_RATE_CONTROL_POINT) {
                ret = ble_gattc_write(hrs_client->client.conn_idx, hrs_client->control_point_h, 0,
                                                                sizeof(uint8_t), &cp_val_reset_ee);
        }

        return ret == BLE_STATUS_OK;
}

static void handle_heart_rate_measurement_ccc_read(hrs_client_t *hrs_client, att_error_t status,
                                                                uint16_t length, const uint8_t *value)
{
        uint16_t ccc = 0;

        if (!hrs_client->cb->get_event_state_completed) {
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
        hrs_client->cb->get_event_state_completed(&hrs_client->client,
                                                        HRS_CLIENT_EVENT_HEART_RATE_MEASUREMENT_NOTIF,
                                                        status, ccc & GATT_CCC_NOTIFICATIONS);
}

static void handle_body_sensor_location_read(hrs_client_t *hrs_client, att_error_t status,
                                                        uint16_t length, const uint8_t *value)
{
        if (!hrs_client->cb->read_body_sensor_location_completed) {
                return;
        }

        if (status != ATT_ERROR_OK) {
                goto done;
        }

        if (length < 1) {
                status = ATT_ERROR_INVALID_VALUE_LENGTH;
        }

done:
        hrs_client->cb->read_body_sensor_location_completed(&hrs_client->client, status, value[0]);
}

static void handle_read_completed_evt(ble_client_t *client,
                                                        const ble_evt_gattc_read_completed_t *evt)
{
        hrs_client_t *hrs_client = (hrs_client_t *) client;
        uint16_t handle = evt->handle;

        if (handle == hrs_client->meas_ccc_h) {
                handle_heart_rate_measurement_ccc_read(hrs_client, evt->status, evt->length,
                                                                                        evt->value);
        } else if (handle == hrs_client->body_sensor_location_h) {
                handle_body_sensor_location_read(hrs_client, evt->status, evt->length, evt->value);
        }
}

static void handle_heart_rate_measurement_ccc_write(hrs_client_t *hrs_client, att_error_t status)
{
        if (!hrs_client->cb->set_event_state_completed) {
                return;
        }

        hrs_client->cb->set_event_state_completed(&hrs_client->client,
                                                        HRS_CLIENT_EVENT_HEART_RATE_MEASUREMENT_NOTIF,
                                                        status);
}

static void handle_heart_rate_control_point_write(hrs_client_t *hrs_client, att_error_t status)
{
        if (!hrs_client->cb->reset_energy_expended_completed) {
                return;
        }

        hrs_client->cb->reset_energy_expended_completed(&hrs_client->client, status);
}

static void handle_write_completed_evt(ble_client_t *client,
                                                        const ble_evt_gattc_write_completed_t *evt)
{
        hrs_client_t *hrs_client = (hrs_client_t *) client;
        uint16_t handle = evt->handle;

        if (handle == hrs_client->meas_ccc_h) {
                handle_heart_rate_measurement_ccc_write(hrs_client, evt->status);
        } else if (handle == hrs_client->control_point_h) {
                handle_heart_rate_control_point_write(hrs_client, evt->status);
        }
}

static bool check_pdu_length(uint16_t *length, size_t field_length)
{
        if (*length < field_length) {
                return false;
        }

        *length -= field_length;

        return true;
}

static void unpack_measurement(const uint8_t *value, uint16_t length,
                                                        hrs_client_measurement_t *measurement)
{
        uint8_t flags;

        memset(measurement, 0, sizeof(*measurement));

        if (!check_pdu_length(&length, 1)) {
                return;
        }

        flags = get_u8_inc(&value);

        if (flags & HRM_FLAG_VAL_16BIT) {
                if (!check_pdu_length(&length, 2)) {
                        return;
                }

                measurement->bpm = get_u16_inc(&value);
        } else {
                if (!check_pdu_length(&length, 1)) {
                        return;
                }

                measurement->bpm = get_u8_inc(&value);
        }

        if (flags & HRM_FLAG_SENSOR_CONTACT_DETECTED) {
                measurement->contact_detected = true;
        }

        if (flags & HRM_FLAG_SENSOR_CONTACT_SUPPORTED) {
                measurement->contact_supported = true;
        }

        if (flags & HRM_FLAG_ENERGY_EXPENDED_PRESENT) {
                if (!check_pdu_length(&length, 2)) {
                        return;
                }

                measurement->has_energy_expended = true;
                measurement->energy_expended = get_u16_inc(&value);
        }

        if (flags & HRM_FLAG_RR_INTERVAL_PRESENT) {
                measurement->rr_num = length / 2;
                memcpy(measurement->rr, value, length);
        }
}

static void handle_heart_rate_measurement(hrs_client_t *hrs_client, uint16_t length,
                                                                                const uint8_t *value)
{
        if (!hrs_client->cb->heart_rate_measurement_notif) {
                return;
        }

        /*
         * We have to allocate some more data than only hrs_client_measurement_t because of variable
         * length of rr[] values. We know that Flags and Heart Rate Measurement Value fields are
         * mandatory so we have to subtract 2 bytes for these data. The rest is enough space for rr
         * values. The space can be grater than number of rr values but for sure will be not too
         * small for them.
         *
         * With this solution application must take care of freeing memory of measurement.
         */
        hrs_client_measurement_t *measurement = OS_MALLOC(sizeof(*measurement) + length - 2);

        unpack_measurement(value, length, measurement);

        hrs_client->cb->heart_rate_measurement_notif(&hrs_client->client, measurement);
}

static void handle_notification_evt(ble_client_t *client, const ble_evt_gattc_notification_t *evt)
{
        hrs_client_t *hrs_client = (hrs_client_t *) client;
        uint16_t handle = evt->handle;

        if (handle == hrs_client->meas_h) {
                handle_heart_rate_measurement(hrs_client, evt->length, evt->value);
        }
}

static void handle_disconnected_evt(ble_client_t *client, const ble_evt_gap_disconnected_t *evt)
{
        client->conn_idx = BLE_CONN_IDX_INVALID;

        ble_client_remove(client);
}

static void cleanup(ble_client_t *client)
{
        hrs_client_t *hrs_client = (hrs_client_t *) client;

        OS_FREE(hrs_client);
}

static hrs_client_t *init(uint16_t conn_idx, const hrs_client_callbacks_t *cb)
{
        hrs_client_t *hrs_client;

        hrs_client = OS_MALLOC(sizeof(*hrs_client));
        memset(hrs_client, 0, sizeof(*hrs_client));

        hrs_client->cb = cb;
        hrs_client->client.conn_idx = conn_idx;
        hrs_client->client.read_completed_evt = handle_read_completed_evt;
        hrs_client->client.write_completed_evt = handle_write_completed_evt;
        hrs_client->client.notification_evt = handle_notification_evt;
        hrs_client->client.disconnected_evt = handle_disconnected_evt;
        hrs_client->client.cleanup = cleanup;
        hrs_client->client.serialize = serialize;

        return hrs_client;
}

ble_client_t *hrs_client_init(const hrs_client_callbacks_t *cb,
                                                        const ble_evt_gattc_browse_svc_t *evt)
{
        hrs_client_t *hrs_client;
        const gattc_item_t *item;
        att_uuid_t uuid;

        if (!cb) {
                return NULL;
        }

        ble_uuid_create16(UUID_SERVICE_HRS, &uuid);
        if (!ble_uuid_equal(&uuid, &evt->uuid)) {
                return NULL;
        }

        hrs_client = init(evt->conn_idx, cb);

        ble_gattc_util_find_init(evt);

        /* HR Heart Rate Measurement Characteristic */
        ble_uuid_create16(UUID_HEART_RATE_MEASUREMENT, &uuid);
        item = ble_gattc_util_find_characteristic(&uuid);
        if (!item) {
                goto failed;
        }
        hrs_client->meas_h = item->c.value_handle;

        /* Look for Heart Rate Measurement CCC descriptor */
        ble_uuid_create16(UUID_GATT_CLIENT_CHAR_CONFIGURATION, &uuid);
        item = ble_gattc_util_find_descriptor(&uuid);
        if (!item) {
                goto failed;
        }
        hrs_client->meas_ccc_h = item->handle;

        /* HR Body Sensor Location Characteristic */
        ble_uuid_create16(UUID_BODY_SENSOR_LOCATION, &uuid);
        item = ble_gattc_util_find_characteristic(&uuid);
        if (item && (item->c.properties & GATT_PROP_READ)) {
                hrs_client->body_sensor_location_h = item->c.value_handle;
                hrs_client->capabilities |= HRS_CLIENT_CAP_BODY_SENSOR_LOCATION;
        }

        /* HR Heart Rate Control Point Characteristic */
        ble_uuid_create16(UUID_HEART_RATE_CONTROL_POINT, &uuid);
        item = ble_gattc_util_find_characteristic(&uuid);
        if (item && (item->c.properties & GATT_PROP_WRITE)) {
                hrs_client->control_point_h = item->c.value_handle;
                hrs_client->capabilities |= HRS_CLIENT_CAP_HEART_RATE_CONTROL_POINT;
        }

        return &hrs_client->client;

failed:
        cleanup(&hrs_client->client);
        return NULL;
}

static bool handle_get_heart_rate_measurement_notif_state(hrs_client_t *hrs_client)
{
        ble_error_t status;

        status = ble_gattc_read(hrs_client->client.conn_idx, hrs_client->meas_ccc_h, 0);

        return status == BLE_STATUS_OK;
}

bool hrs_client_get_event_state(ble_client_t *client, hrs_client_event_t event)
{
        hrs_client_t *hrs_client = (hrs_client_t *) client;


        switch (event) {
        case HRS_CLIENT_EVENT_HEART_RATE_MEASUREMENT_NOTIF:
                return handle_get_heart_rate_measurement_notif_state(hrs_client);
        }

        return false;
}

static bool handle_set_heart_rate_measurement_notif_state(hrs_client_t *hrs_client, bool enable)
{
        ble_error_t status;

        status = ble_gattc_util_write_ccc(hrs_client->client.conn_idx, hrs_client->meas_ccc_h,
                                                                enable & GATT_CCC_NOTIFICATIONS);

        return status == BLE_STATUS_OK;
}

bool hrs_client_set_event_state(ble_client_t *client, hrs_client_event_t event, bool enable)
{
        hrs_client_t *hrs_client = (hrs_client_t *) client;

        switch (event) {
        case HRS_CLIENT_EVENT_HEART_RATE_MEASUREMENT_NOTIF:
                return handle_set_heart_rate_measurement_notif_state(hrs_client, enable);
        }

        return false;
}

ble_client_t *hrs_client_init_from_data(uint16_t conn_idx, const hrs_client_callbacks_t *cb,
                                                                const void *data, size_t length)
{
        hrs_client_t *hrs_client;
        const hrs_client_serialized_t *s_data = data;

        if (!data || (length < sizeof(hrs_client_serialized_t))) {
                return NULL;
        }

        hrs_client = init(conn_idx, cb);

        hrs_client->meas_h                      = s_data->meas_h;
        hrs_client->meas_ccc_h                  = s_data->meas_ccc_h;
        hrs_client->body_sensor_location_h      = s_data->body_sensor_location_h;
        hrs_client->control_point_h             = s_data->control_point_h;
        hrs_client->capabilities                = s_data->capabilities;

        ble_client_attach(&hrs_client->client, conn_idx);

        return &hrs_client->client;
}
