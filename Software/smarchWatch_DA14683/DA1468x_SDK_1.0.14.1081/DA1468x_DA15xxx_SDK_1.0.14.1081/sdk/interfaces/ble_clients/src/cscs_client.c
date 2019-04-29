/**
 ****************************************************************************************
 *
 * @file cscs_client.c
 *
 * @brief Cycling Speed and Cadence Service Client implementation
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include "osal.h"
#include <stdio.h>
#include "ble_uuid.h"
#include "ble_bufops.h"
#include "ble_gattc_util.h"
#include "ble_client.h"
#include "cscs_client.h"

#define UUID_CSC_MEASUREMENT    (0x2A5B)
#define UUID_CSC_FEATURE        (0x2A5C)
#define UUID_SENSOR_LOCATION    (0x2A5D)
#define UUID_SC_CONTROL_POINT   (0x2A55)

/* SC Control Point Operation Opcodes */
typedef enum {
        SC_CP_OPCODE_SET_CUMULATIVE_VALUE = 0x01,
        SC_CP_OPCODE_START_SENSOR_CALIBRATION = 0x02,
        SC_CP_OPCODE_UPDATE_SENSOR_LOCATION  = 0x03,
        SC_CP_OPCODE_REQUEST_SUPPORTED_SENSOR_LOCATIONS = 0x04,
        SC_CP_OPCODE_RESPONSE = 0x10,
} sc_cp_opcode_t;

typedef enum {
        MEASUREMENT_FLAG_WHEEL_REV_DATA_PRESENT = 0x01,
        MEASUREMENT_FLAG_CRANK_REV_DATA_PRESENT = 0x02,
} measurement_flags_t;

typedef struct {
        ble_client_t client;

        const cscs_client_callbacks_t *cb;

        uint16_t csc_feature_h;
        uint16_t sensor_location_h;

        uint16_t sc_control_point_h;
        uint16_t sc_control_point_ccc_h;

        uint16_t csc_measurement_h;
        uint16_t csc_measurement_ccc_h;

        uint8_t pending_operation;
} cscs_client_t;

/* Type used for client serialization */
typedef struct {
        uint16_t csc_feature_h;
        uint16_t sensor_location_h;

        uint16_t sc_control_point_h;
        uint16_t sc_control_point_ccc_h;

        uint16_t csc_measurement_h;
        uint16_t csc_measurement_ccc_h;
} __attribute__((packed)) cscs_client_serialized_t;

static void serialize(ble_client_t *client, void *data, size_t *length)
{
        cscs_client_t *cscs_client = (cscs_client_t *) client;
        cscs_client_serialized_t *s_data = data;
        *length = sizeof(cscs_client_serialized_t);

        if (!data) {
                return;
        }

        s_data->csc_feature_h          = cscs_client->csc_feature_h;
        s_data->sensor_location_h      = cscs_client->sensor_location_h;

        s_data->sc_control_point_h     = cscs_client->sc_control_point_h;
        s_data->sc_control_point_ccc_h = cscs_client->sc_control_point_ccc_h;

        s_data->csc_measurement_h      = cscs_client->csc_measurement_h;
        s_data->csc_measurement_ccc_h  = cscs_client->csc_measurement_ccc_h;
}

static void handle_disconnected_evt(ble_client_t *client, const ble_evt_gap_disconnected_t *evt)
{
        cscs_client_t *cscs_client = (cscs_client_t *) client;

        client->conn_idx = BLE_CONN_IDX_INVALID;
        cscs_client->pending_operation = 0x00;

        ble_client_remove(client);
}

static void handle_csc_measurement_ccc_write(cscs_client_t *cscs_client, att_error_t status)
{
        if (!cscs_client->cb->set_event_state_completed) {
                return;
        }

        cscs_client->cb->set_event_state_completed(&cscs_client->client,
                                                CSCS_CLIENT_EVENT_CSC_MEASUREMENT_NOTIF, status);
}

static void handle_sc_cp_ccc_write(cscs_client_t *cscs_client, att_error_t status)
{
        if (!cscs_client->cb->set_sc_control_point_state_completed) {
                return;
        }

        cscs_client->cb->set_sc_control_point_state_completed(&cscs_client->client, status);
}

static void update_sensor_location_cb(cscs_client_t *cscs_client, cscs_client_status_t status)
{
        if (cscs_client->cb->update_sensor_location_completed) {
                cscs_client->cb->update_sensor_location_completed(&cscs_client->client, status);
        }
}

static void request_supported_sensor_locations_cb(cscs_client_t *cscs_client,
                                                        cscs_client_status_t status,
                                                        uint16_t length, const uint8_t *locations)
{
        if (cscs_client->cb->request_supported_sensor_locations_completed) {
                cscs_client->cb->request_supported_sensor_locations_completed(&cscs_client->client,
                                                                        status, length, locations);
        }
}

static void set_cumulative_value_cb(cscs_client_t *cscs_client, cscs_client_status_t status)
{
        if (cscs_client->cb->set_cumulative_value_completed) {
                cscs_client->cb->set_cumulative_value_completed(&cscs_client->client, status);
        }
}

static void handle_sc_cp_write(cscs_client_t *cscs_client, att_error_t status)
{
        cscs_client_status_t client_status = (cscs_client_status_t) status;
        uint8_t pending_operation;

        if (cscs_client->pending_operation == 0x00) {
                // Client did not triggered write operation
                return;
        }

        pending_operation = cscs_client->pending_operation;

        if (status == ATT_ERROR_OK) {
                // Wait for indication
                return;
        }

        cscs_client->pending_operation = 0x00;

        switch (client_status) {
        case CSCS_CLIENT_STATUS_OPERATION_IN_PROGRESS:
        case CSCS_CLIENT_STATUS_IMPROPERLY_CONFIGURED:
                break;
        default:
                client_status = CSCS_CLIENT_STATUS_OPERATION_FAILED;
                break;
        }

        switch (pending_operation) {
        case SC_CP_OPCODE_UPDATE_SENSOR_LOCATION:
                update_sensor_location_cb(cscs_client, client_status);
                break;
        case SC_CP_OPCODE_REQUEST_SUPPORTED_SENSOR_LOCATIONS:
                request_supported_sensor_locations_cb(cscs_client, client_status, 0, NULL);
                break;
        case SC_CP_OPCODE_SET_CUMULATIVE_VALUE:
                set_cumulative_value_cb(cscs_client, client_status);
                break;
        default:
                break;
        }
}

static void handle_write_completed_evt(ble_client_t *client,
                                                        const ble_evt_gattc_write_completed_t *evt)
{
        cscs_client_t *cscs_client = (cscs_client_t *) client;
        uint16_t handle = evt->handle;

        if (handle == cscs_client->csc_measurement_ccc_h) {
                handle_csc_measurement_ccc_write(cscs_client, evt->status);
        } else if (handle == cscs_client->sc_control_point_ccc_h) {
                handle_sc_cp_ccc_write(cscs_client, evt->status);
        } else if (handle == cscs_client->sc_control_point_h) {
                handle_sc_cp_write(cscs_client, evt->status);
        }
}

static void handle_csc_feature_value(cscs_client_t *cscs_client, att_error_t status,
                                                        uint16_t length, const uint8_t *value)
{
        uint16_t feat = 0;

        if (!cscs_client->cb->read_csc_features_completed) {
                return;
        }

        if (status != ATT_ERROR_OK) {
                goto done;
        }

        if (length != sizeof(uint16_t)) {
                status = ATT_ERROR_UNLIKELY;
                goto done;
        }

        feat = get_u16(value);

done:
        cscs_client->cb->read_csc_features_completed(&cscs_client->client, status, feat);
}

static void handle_sensor_location_value(cscs_client_t *cscs_client, att_error_t status,
                                                        uint16_t length, const uint8_t *value)
{
        cscs_client_sensor_location_t location = 0;

        if (status != ATT_ERROR_OK) {
                goto done;
        }

        if (length != sizeof(uint8_t)) {
                status = ATT_ERROR_UNLIKELY;
                goto done;
        }

        location = value[0];

done:
        if (cscs_client->cb->read_sensor_location_completed) {
                cscs_client->cb->read_sensor_location_completed(&cscs_client->client, status,
                                                                                        location);
        }
}

static void handle_csc_measurement_ccc_read(cscs_client_t *cscs_client, att_error_t status,
                                                        uint16_t length, const uint8_t *value)
{
        gatt_ccc_t ccc_val = GATT_CCC_NONE;

        if (!cscs_client->cb->get_event_state_completed) {
                return;
        }

        if (status == ATT_ERROR_OK && length < sizeof(uint16_t)) {
                status = ATT_ERROR_UNLIKELY;
                goto done;
        }

        ccc_val = (gatt_ccc_t) get_u16(value);

done:
        cscs_client->cb->get_event_state_completed(&cscs_client->client,
                CSCS_CLIENT_EVENT_CSC_MEASUREMENT_NOTIF, status, ccc_val == GATT_CCC_NOTIFICATIONS);
}

static void handle_sc_cp_ccc_read(cscs_client_t *cscs_client, att_error_t status,
                                                        uint16_t length, const uint8_t *value)
{
        gatt_ccc_t ccc_val = GATT_CCC_NONE;

        if (!cscs_client->cb->get_sc_control_point_state_completed) {
                return;
        }

        if (status == ATT_ERROR_OK && length < sizeof(uint16_t)) {
                status = ATT_ERROR_UNLIKELY;
                goto done;
        }

        ccc_val = (gatt_ccc_t) get_u16(value);

done:
        cscs_client->cb->get_sc_control_point_state_completed(&cscs_client->client, status,
                                                                ccc_val == GATT_CCC_INDICATIONS);
}

static void handle_read_completed_evt(ble_client_t *client,
                                                        const ble_evt_gattc_read_completed_t *evt)
{
        cscs_client_t *cscs_client = (cscs_client_t *) client;
        uint16_t handle = evt->handle;

        if (handle == cscs_client->csc_feature_h) {
                handle_csc_feature_value(cscs_client, evt->status, evt->length, evt->value);
        } else if (handle == cscs_client->sensor_location_h) {
                handle_sensor_location_value(cscs_client, evt->status, evt->length, evt->value);
        } else if (handle == cscs_client->csc_measurement_ccc_h) {
                handle_csc_measurement_ccc_read(cscs_client, evt->status, evt->length, evt->value);
        } else if (handle == cscs_client->sc_control_point_ccc_h) {
                handle_sc_cp_ccc_read(cscs_client, evt->status, evt->length, evt->value);
        }
}

static void handle_sc_cp_indication(cscs_client_t *cscs_client, uint16_t length,
                                                                        const uint8_t *value)
{
        cscs_client_status_t status;
        sc_cp_opcode_t operation;

        cscs_client->pending_operation = 0x00;

        /*
         * Indication should contain at least:
         * 1 byte response opcode
         * 1 byte operation opcode
         * 1 byte status field
         */
        if (length < 3) {
                return;
        }

        if (value[0] != SC_CP_OPCODE_RESPONSE) {
                return;
        }

        operation = value[1];
        status = value[2];

        switch (operation) {
        case SC_CP_OPCODE_UPDATE_SENSOR_LOCATION:
                update_sensor_location_cb(cscs_client, status);
                break;
        case SC_CP_OPCODE_REQUEST_SUPPORTED_SENSOR_LOCATIONS:
                request_supported_sensor_locations_cb(cscs_client, status, length - 3, value + 3);
                break;
        case SC_CP_OPCODE_SET_CUMULATIVE_VALUE:
                set_cumulative_value_cb(cscs_client, status);
                break;
        default:
                break;
        }
}

static void handle_indication_evt(ble_client_t *client, const ble_evt_gattc_indication_t *evt)
{
        cscs_client_t *cscs_client = (cscs_client_t *) client;
        uint16_t handle = evt->handle;

        if (handle == cscs_client->sc_control_point_h) {
                handle_sc_cp_indication(cscs_client, evt->length, evt->value);
        }
}

static void handle_csc_measurement(cscs_client_t *cscs_client, uint16_t length,
                                                                        const uint8_t *value)
{
        cscs_client_measurement_t measurement;
        const uint8_t *p;

        if (!cscs_client->cb->csc_measurement) {
                return;
        }

        if (length <= 1) {
                return;
        }

        memset(&measurement, 0, sizeof(measurement));

        p = &value[1];
        length--;

        if (value[0] & MEASUREMENT_FLAG_WHEEL_REV_DATA_PRESENT) {
                if (length < 6) {
                        // malformed packet
                        return;
                }

                measurement.wheel_revolution_data_present = true;
                measurement.cumulative_wheel_revolutions = get_u32_inc(&p);
                measurement.last_wheel_event_time = get_u16_inc(&p);
                length -= 6;
        }

        if (value[0] & MEASUREMENT_FLAG_CRANK_REV_DATA_PRESENT) {
                if (length < 4) {
                        // malformed packet
                        return;
                }

                measurement.crank_revolution_data_present = true;
                measurement.cumulative_crank_revolutions = get_u16_inc(&p);
                measurement.last_crank_event_time = get_u16_inc(&p);
        }

        cscs_client->cb->csc_measurement(&cscs_client->client, &measurement);
}

static void handle_notification_evt(ble_client_t *client, const ble_evt_gattc_notification_t *evt)
{
        cscs_client_t *cscs_client = (cscs_client_t *) client;
        uint16_t handle = evt->handle;

        if (handle == cscs_client->csc_measurement_h) {
                handle_csc_measurement(cscs_client, evt->length, evt->value);
        }
}

static void cleanup(ble_client_t *client)
{
        cscs_client_t *cscs_client = (cscs_client_t *) client;

        OS_FREE(cscs_client);
}

static cscs_client_t *init(uint16_t conn_idx, const cscs_client_callbacks_t *cb)
{
        cscs_client_t *cscs_client;

        cscs_client = OS_MALLOC(sizeof(*cscs_client));
        memset(cscs_client, 0, sizeof(*cscs_client));

        cscs_client->cb                         = cb;
        cscs_client->client.notification_evt    = handle_notification_evt;
        cscs_client->client.indication_evt      = handle_indication_evt;
        cscs_client->client.write_completed_evt = handle_write_completed_evt;
        cscs_client->client.read_completed_evt  = handle_read_completed_evt;
        cscs_client->client.disconnected_evt    = handle_disconnected_evt;
        cscs_client->client.cleanup             = cleanup;
        cscs_client->client.serialize           = serialize;
        cscs_client->client.conn_idx            = conn_idx;

        return cscs_client;
}

ble_client_t *cscs_client_init(const cscs_client_callbacks_t *cb,
                                                        const ble_evt_gattc_browse_svc_t *evt)
{
        cscs_client_t *cscs_client;
        att_uuid_t uuid;
        const gattc_item_t *item;

        if (!cb) {
                return NULL;
        }

        ble_uuid_create16(UUID_SERVICE_CSCS, &uuid);
        if (!ble_uuid_equal(&uuid, &evt->uuid)) {
                return NULL;
        }

        cscs_client = init(evt->conn_idx, cb);

        /* Find CSC Service's characteristics and descriptors */
        ble_gattc_util_find_init(evt);

        /* CSC Measurement */
        ble_uuid_create16(UUID_CSC_MEASUREMENT, &uuid);
        item = ble_gattc_util_find_characteristic(&uuid);

        if (!item || !(item->c.properties & GATT_PROP_NOTIFY)) {
                /* Characteristic does not exist or does not have mandatory property */
                goto failed;
        }

        cscs_client->csc_measurement_h = item->c.value_handle;

        ble_uuid_create16(UUID_GATT_CLIENT_CHAR_CONFIGURATION, &uuid);
        item = ble_gattc_util_find_descriptor(&uuid);

        if (!item) {
                goto failed;
        }

        cscs_client->csc_measurement_ccc_h = item->handle;

        /* CSC Feature */
        ble_uuid_create16(UUID_CSC_FEATURE, &uuid);
        item = ble_gattc_util_find_characteristic(&uuid);

        if (!item || !(item->c.properties & GATT_PROP_READ)) {
                /* Characteristic does not exist or does not have mandatory property */
                goto failed;
        }

        cscs_client->csc_feature_h = item->c.value_handle;

        /* Sensor Location */
        ble_uuid_create16(UUID_SENSOR_LOCATION, &uuid);
        item = ble_gattc_util_find_characteristic(&uuid);

        if (item) {
                if ((item->c.properties & GATT_PROP_READ)) {
                        /* Characteristic exists and has mandatory property - save handle for it */
                        cscs_client->sensor_location_h = item->c.value_handle;
                }
        }

        /* SC Control Point */
        ble_uuid_create16(UUID_SC_CONTROL_POINT, &uuid);
        item = ble_gattc_util_find_characteristic(&uuid);

        if (item) {
                if (!(item->c.properties & GATT_PROP_WRITE) || !(item->c.properties &
                                                                        GATT_PROP_INDICATE)) {
                        /* Characteristic exists but does not have not mandatory properties */
                        goto done;
                }

                cscs_client->sc_control_point_h = item->c.value_handle;
                ble_uuid_create16(UUID_GATT_CLIENT_CHAR_CONFIGURATION, &uuid);
                item = ble_gattc_util_find_descriptor(&uuid);

                /* This characteristic has mandatory indicate property - must have CCC descriptor */
                if (!item) {
                        /* Remove SC Control Point handle */
                        cscs_client->sc_control_point_h = 0;
                        goto done;
                }

                cscs_client->sc_control_point_ccc_h = item->handle;
        }

done:

        return &cscs_client->client;

failed:

        cleanup(&cscs_client->client);
        return NULL;
}

ble_client_t *cscs_client_init_from_data(uint16_t conn_idx, const cscs_client_callbacks_t *cb,
                                                                    const void *data, size_t length)
{
        cscs_client_t *cscs_client;
        const cscs_client_serialized_t *d = data;

        if (!data || (length < sizeof(cscs_client_serialized_t))) {
                return NULL;
        }

        cscs_client = init(conn_idx, cb);

        cscs_client->csc_feature_h          = d->csc_feature_h;
        cscs_client->sensor_location_h      = d->sensor_location_h;
        cscs_client->sc_control_point_h     = d->sc_control_point_h;
        cscs_client->sc_control_point_ccc_h = d->sc_control_point_ccc_h;
        cscs_client->csc_measurement_h      = d->csc_measurement_h;
        cscs_client->csc_measurement_ccc_h  = d->csc_measurement_ccc_h;

        ble_client_attach(&cscs_client->client, conn_idx);

        return &cscs_client->client;
}

bool cscs_client_read_csc_features(ble_client_t *client)
{
        cscs_client_t *cscs_client = (cscs_client_t *) client;

        /*
         * CSC Feature handle must be non-zero value, otherwise CSCS Client has not been created -
         * no need to check it here.
         */
        return ble_gattc_read(cscs_client->client.conn_idx, cscs_client->csc_feature_h, 0)
                                                                                == BLE_STATUS_OK;
}

cscs_client_cap_t cscs_client_get_capabilities(ble_client_t *client)
{
        cscs_client_t *cscs_client = (cscs_client_t *) client;
        cscs_client_cap_t cap = 0;

        if (cscs_client->sensor_location_h) {
                cap |= CSCS_CLIENT_CAP_SENSOR_LOCATION;
        }

        return cap;
}

bool cscs_client_set_event_state(ble_client_t *client, cscs_client_event_t event, bool enable)
{
        cscs_client_t *cscs_client = (cscs_client_t *) client;
        gatt_ccc_t ccc;

        if (event != CSCS_CLIENT_EVENT_CSC_MEASUREMENT_NOTIF) {
                return false;
        }

        ccc = enable ? GATT_CCC_NOTIFICATIONS : GATT_CCC_NONE;

        /*
         *  CSC Measurement CCC descriptor handle is non zero value, otherwise CSCS Client has not
         *  been created.
         */
        return ble_gattc_util_write_ccc(client->conn_idx, cscs_client->csc_measurement_ccc_h, ccc)
                                                                                == BLE_STATUS_OK;
}

bool cscs_client_set_sc_control_point_ind_state(ble_client_t *client, bool enable)
{
        cscs_client_t *cscs_client = (cscs_client_t *) client;
        gatt_ccc_t ccc;

        if (!cscs_client->sc_control_point_ccc_h) {
                /* No handle for SC Control Point CCC descriptor */
                return false;
        }

        ccc = enable ? GATT_CCC_INDICATIONS : GATT_CCC_NONE;

        return ble_gattc_util_write_ccc(client->conn_idx, cscs_client->sc_control_point_ccc_h, ccc)
                                                                                == BLE_STATUS_OK;
}

bool cscs_client_get_event_state(ble_client_t *client, cscs_client_event_t event)
{
        cscs_client_t *cscs_client = (cscs_client_t *) client;

        if (event != CSCS_CLIENT_EVENT_CSC_MEASUREMENT_NOTIF) {
                return false;
        }

        /*
         *  CSC Measurement CCC descriptor handle is non zero value, otherwise CSCS Client has not
         *  been created.
         */
        return ble_gattc_read(client->conn_idx, cscs_client->csc_measurement_ccc_h, 0)
                                                                                == BLE_STATUS_OK;
}

bool cscs_client_get_sc_control_point_ind_state(ble_client_t *client)
{
        cscs_client_t *cscs_client = (cscs_client_t *) client;

        if (!cscs_client->sc_control_point_ccc_h) {
                /* CCC descriptor handle not found */
                return false;
        }

        return ble_gattc_read(client->conn_idx, cscs_client->sc_control_point_ccc_h, 0)
                                                                                == BLE_STATUS_OK;
}

bool cscs_client_update_sensor_location(ble_client_t *client,
                                                        cscs_client_sensor_location_t location)
{
        cscs_client_t *cscs_client = (cscs_client_t *) client;
        uint8_t buffer[2];// two byte buffer for opcode and new location
        ble_error_t status;

        if (!cscs_client->sc_control_point_h) {
                return false;
        }

        if (cscs_client->pending_operation != 0x00) {
                return false;
        }

        buffer[0] = SC_CP_OPCODE_UPDATE_SENSOR_LOCATION;
        buffer[1] = location;

        status = ble_gattc_write(client->conn_idx, cscs_client->sc_control_point_h, 0,
                                                                        sizeof(buffer), buffer);

        if (status != BLE_STATUS_OK) {
                return false;
        }

        cscs_client->pending_operation = SC_CP_OPCODE_UPDATE_SENSOR_LOCATION;

        return true;
}

bool cscs_client_request_supported_sensor_locations(ble_client_t *client)
{
        cscs_client_t *cscs_client = (cscs_client_t *) client;
        uint8_t opcode = SC_CP_OPCODE_REQUEST_SUPPORTED_SENSOR_LOCATIONS;
        ble_error_t status;

        if (!cscs_client->sc_control_point_h) {
                return false;
        }

        if (cscs_client->pending_operation != 0x00) {
                return false;
        }

        status = ble_gattc_write(client->conn_idx, cscs_client->sc_control_point_h, 0,
                                                                        sizeof(opcode), &opcode);

        if (status != BLE_STATUS_OK) {
                return false;
        }

        cscs_client->pending_operation = SC_CP_OPCODE_REQUEST_SUPPORTED_SENSOR_LOCATIONS;

        return true;
}

bool cscs_client_set_cumulative_value(ble_client_t *client, uint32_t value)
{
        cscs_client_t *cscs_client = (cscs_client_t *) client;
        uint8_t buffer[5];//1 byte for opcode + 4 bytes for cumulative value
        ble_error_t status;

        if (!cscs_client->sc_control_point_h) {
                return false;
        }

        if (cscs_client->pending_operation != 0x00) {
                return false;
        }

        buffer[0] = SC_CP_OPCODE_SET_CUMULATIVE_VALUE;
        put_u32(&buffer[1], value);

        status = ble_gattc_write(client->conn_idx, cscs_client->sc_control_point_h, 0,
                                                                        sizeof(buffer), buffer);

        if (status != BLE_STATUS_OK) {
                return false;
        }

        cscs_client->pending_operation = SC_CP_OPCODE_SET_CUMULATIVE_VALUE;

        return true;
}

bool cscs_client_read_sensor_location(ble_client_t *client)
{
        cscs_client_t *cscs_client = (cscs_client_t *) client;

        if (!cscs_client->sensor_location_h) {
                return false;
        }

        return ble_gattc_read(client->conn_idx, cscs_client->sensor_location_h, 0) == BLE_STATUS_OK;
}

void cscs_client_sc_control_point_timeout(ble_client_t *client)
{
        cscs_client_t *cscs_client = (cscs_client_t *) client;
        uint8_t pending_operation;

        if (cscs_client->pending_operation == 0x00) {
                /* Client did not trigger write operation */
                return;
        }

        pending_operation = cscs_client->pending_operation;
        cscs_client->pending_operation = 0;

        switch (pending_operation) {
        case SC_CP_OPCODE_UPDATE_SENSOR_LOCATION:
                update_sensor_location_cb(cscs_client, CSCS_CLIENT_STATUS_TIMEOUT);
                break;
        case SC_CP_OPCODE_REQUEST_SUPPORTED_SENSOR_LOCATIONS:
                request_supported_sensor_locations_cb(cscs_client, CSCS_CLIENT_STATUS_TIMEOUT, 0,
                                                                                        NULL);
                break;
        case SC_CP_OPCODE_SET_CUMULATIVE_VALUE:
                set_cumulative_value_cb(cscs_client, CSCS_CLIENT_STATUS_TIMEOUT);
                break;
        default:
                break;
        }
}
