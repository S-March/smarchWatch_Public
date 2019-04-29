/**
 ****************************************************************************************
 *
 * @file hrs.c
 *
 * @brief Heart Rate Service sample implementation
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
#include "svc_defines.h"
#include "hrs.h"

#define UUID_HEART_RATE_MEASUREMENT     (0x2A37)
#define UUID_BODY_SENSOR_LOCATION       (0x2A38)
#define UUID_HEART_RATE_CONTROL_POINT   (0x2A39)

enum hrs_error {
        HRS_ERROR_CONTROL_POINT_NOT_SUPPORTED = ATT_ERROR_APPLICATION_ERROR,
};

enum measurement_flags {
        HRM_FLAG_VAL_16BIT                      = 0x01,
        HRM_FLAG_SENSOR_CONTACT_DETECTED        = 0x02,
        HRM_FLAG_SENSOR_CONTACT_SUPPORTED       = 0x04,
        HRM_FLAG_ENERGY_EXPENDED_PRESENT        = 0x08,
        HRM_FLAG_RR_INTERVAL_PRESENT            = 0x10,
};

typedef struct {
        ble_service_t svc;

        // callbacks
        const hrs_callbacks_t *cb;

        // handles
        uint16_t hrm_val_h;
        uint16_t hrm_ccc_h;
        uint16_t hrcp_val_h;
} hr_service_t;

static att_error_t do_hrm_ccc_write(hr_service_t *hrs, uint16_t conn_idx, uint16_t offset,
                                                                uint16_t length, const uint8_t *value)
{
        uint16_t ccc_val;

        if (offset) {
                return ATT_ERROR_ATTRIBUTE_NOT_LONG;
        }

        if (length != sizeof(uint16_t)) {
                return ATT_ERROR_APPLICATION_ERROR;
        }

        ccc_val = get_u16(value);

        ble_storage_put_u32(conn_idx, hrs->hrm_ccc_h, ccc_val, true);

        if (hrs->cb && hrs->cb->notif_changed) {
                hrs->cb->notif_changed(conn_idx, ccc_val & GATT_CCC_NOTIFICATIONS);
        }

        return ATT_ERROR_OK;
}

static att_error_t do_hrcp_val_write(hr_service_t *hrs, uint16_t conn_idx, uint16_t offset,
                                                                uint16_t length, const uint8_t *value)
{
        uint8_t op;

        if (offset) {
                return ATT_ERROR_ATTRIBUTE_NOT_LONG;
        }

        if (length < 1) {
                return ATT_ERROR_OK;
        }

        op = get_u8(value);

        if (op != 1) {
                return HRS_ERROR_CONTROL_POINT_NOT_SUPPORTED;
        }

        if (hrs->cb && hrs->cb->ee_reset) {
                hrs->cb->ee_reset(conn_idx);
        }

        return ATT_ERROR_OK;
}

static void handle_read_req(ble_service_t *svc, const ble_evt_gatts_read_req_t *evt)
{
        hr_service_t *hrs = (hr_service_t *) svc;

        if (evt->handle == hrs->hrm_ccc_h) {
                uint16_t ccc_val = 0;

                ble_storage_get_u16(evt->conn_idx, hrs->hrm_ccc_h, &ccc_val);

                // we're little-endian, ok to use value as-is
                ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_OK,
                                                                        sizeof(uint16_t), &ccc_val);
        } else {
                ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_READ_NOT_PERMITTED, 0, NULL);
        }
}

static void handle_write_req(ble_service_t *svc, const ble_evt_gatts_write_req_t *evt)
{
        hr_service_t *hrs = (hr_service_t *) svc;
        att_error_t status = ATT_ERROR_ATTRIBUTE_NOT_FOUND;

        if (evt->handle == hrs->hrm_ccc_h) {
                status = do_hrm_ccc_write(hrs, evt->conn_idx, evt->offset, evt->length, evt->value);
                goto done;
        }

        if (evt->handle == hrs->hrcp_val_h) {
                status = do_hrcp_val_write(hrs, evt->conn_idx, evt->offset, evt->length, evt->value);
                goto done;
        }

done:
        ble_gatts_write_cfm(evt->conn_idx, evt->handle, status);
}

static void cleanup(ble_service_t *svc)
{
        hr_service_t *hrs = (hr_service_t *) svc;

        ble_storage_remove_all(hrs->hrm_ccc_h);

        OS_FREE(svc);
}

ble_service_t *hrs_init(hrs_body_sensor_location_t location, const hrs_callbacks_t *cb)
{
        hr_service_t *hrs;
        uint16_t num_attr;
        uint8_t loc = location;
        uint16_t bsl_val_h;
        att_uuid_t uuid;

        hrs = OS_MALLOC(sizeof(*hrs));
        memset(hrs, 0, sizeof(*hrs));

        hrs->svc.read_req = handle_read_req;
        hrs->svc.write_req = handle_write_req;
        hrs->svc.cleanup = cleanup;
        hrs->cb = cb;

        num_attr = ble_gatts_get_num_attr(0, 3, 1);

        ble_uuid_create16(UUID_SERVICE_HRS, &uuid);
        ble_gatts_add_service(&uuid, GATT_SERVICE_PRIMARY, num_attr);

        ble_uuid_create16(UUID_HEART_RATE_MEASUREMENT, &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_NOTIFY, ATT_PERM_NONE,
                                                                7, 0, NULL, &hrs->hrm_val_h);

        ble_uuid_create16(UUID_GATT_CLIENT_CHAR_CONFIGURATION, &uuid);
        ble_gatts_add_descriptor(&uuid, ATT_PERM_RW, 1, 0, &hrs->hrm_ccc_h);

        ble_uuid_create16(UUID_BODY_SENSOR_LOCATION, &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_READ, ATT_PERM_READ, 1, 0, NULL, &bsl_val_h);

        ble_uuid_create16(UUID_HEART_RATE_CONTROL_POINT, &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_WRITE, ATT_PERM_WRITE, 1, 0, NULL,
                                                                                &hrs->hrcp_val_h);

        ble_gatts_register_service(&hrs->svc.start_h, &hrs->hrm_val_h, &hrs->hrm_ccc_h,
                                                                &bsl_val_h, &hrs->hrcp_val_h, 0);

        // Body Sensor Location is static
        ble_gatts_set_value(bsl_val_h, sizeof(uint8_t), &loc);

        hrs->svc.end_h = hrs->svc.start_h + num_attr;

        ble_service_add(&hrs->svc);

        return &hrs->svc;
}

static uint8_t pack_notify_value(const hrs_measurement_t *meas, uint16_t length, uint8_t *value)
{
        uint8_t *ptr = &value[1];
        uint8_t flags = 0;

        if (meas->bpm > 255) {
                flags |= HRM_FLAG_VAL_16BIT;
                put_u16_inc(&ptr, meas->bpm);
        } else {
                put_u8_inc(&ptr, meas->bpm);
        }

        if (meas->has_energy_expended) {
                flags |= HRM_FLAG_ENERGY_EXPENDED_PRESENT;
                put_u16_inc(&ptr, meas->energy_expended);
        }

        if (meas->rr_num) {
                int idx = 0;
                flags |= HRM_FLAG_RR_INTERVAL_PRESENT;
                while (idx < meas->rr_num && (length - (ptr - value)) >= sizeof(uint16_t)) {
                        put_u16_inc(&ptr, meas->rr[idx++]);
                }
        }

        flags |= (meas->contact_supported ? HRM_FLAG_SENSOR_CONTACT_SUPPORTED : 0);
        flags |= (meas->contact_detected ? HRM_FLAG_SENSOR_CONTACT_DETECTED : 0);
        put_u8(value, flags);

        return ptr - value;
}

void hrs_notify_measurement(ble_service_t *svc, uint16_t conn_idx,  hrs_measurement_t *meas)
{
        hr_service_t *hrs = (hr_service_t *) svc;
        uint16_t ccc_val = 0;
        uint8_t value[20]; // default ATT_MTU-3
        uint8_t value_len;

        ble_storage_get_u16(conn_idx, hrs->hrm_ccc_h, &ccc_val);

        if (!(ccc_val & GATT_CCC_NOTIFICATIONS)) {
                return;
        }

        value_len = pack_notify_value(meas, sizeof(value), value);

        ble_gatts_send_event(conn_idx, hrs->hrm_val_h, GATT_EVENT_NOTIFICATION, value_len, value);
}

void hrs_notify_measurement_all(ble_service_t *svc, hrs_measurement_t *meas)
{
        uint8_t num_conn;
        uint16_t *conn_idx;

        ble_gap_get_connected(&num_conn, &conn_idx);

        while (num_conn--) {
                hrs_notify_measurement(svc, conn_idx[num_conn], meas);
        }

        if (conn_idx) {
                OS_FREE(conn_idx);
        }
}
