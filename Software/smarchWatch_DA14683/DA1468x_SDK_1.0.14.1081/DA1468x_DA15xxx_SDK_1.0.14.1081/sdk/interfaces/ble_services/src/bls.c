/**
 ****************************************************************************************
 *
 * @file bls.c
 *
 * @brief Blood Pressure Service implementation
 *
 * Copyright (C) 2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdbool.h>
#include "osal.h"
#include "ble_storage.h"
#include "ble_uuid.h"
#include "bls.h"

#define UUID_BLOOD_PRESSURE_MEASUREMENT         (0x2A35)
#define UUID_INTERMEDIATE_CUFF_PRESSURE         (0x2A36)
#define UUID_BLOOD_PRESSURE_FEATURE             (0x2A49)

#define BLS_FEATURE_SIZE                        (2)
/**
 * Max length of measurement as sum of each fields:
 * - flags                      - 1 byte
 * - pressure systolic          - 2 bytes
 * - pressure diastolic         - 2 bytes
 * - pressure map               - 2 bytes
 * - time stamp                 - 7 bytes
 * - pulse rate                 - 2 bytes
 * - user ID                    - 1 byte
 * - measurement status         - 2 bytes
 */
#define BLS_MAX_MEASUREMENT_SIZE                (19)

#define MEASUREMENT_STATUS_BODY_MOVEMENT_BIT_POSITION    (0)
#define MEASUREMENT_STATUS_CUFF_FIT_BIT_POSITION         (1)
#define MEASUREMENT_STATUS_IRREGULAR_PULSE_BIT_POSITION  (2)
#define MEASUREMENT_STATUS_PULSE_RATE_RANGE_BIT_POSITION (3)
#define MEASUREMENT_STATUS_MEASUREMENT_POS_BIT_POSITION  (5)

/**
 * Blood Pressure Service Flag
 */
typedef enum {
        BLS_FLAG_UNIT_KPA                       = 0x01, /**< Pressure in kPa if set - otherwise mmHg */
        BLS_FLAG_TIME_STAMP_SUPPORTED           = 0x02, /**< Time Stamp is supported */
        BLS_FLAG_PULSE_RATE                     = 0x04, /**< Pulse Rate is supported */
        BLS_FLAG_USER_ID                        = 0x08, /**< User ID is supported */
        BLS_FLAG_MEASUREMENT_STATUS             = 0x10, /**< Measurement status is supported */
} bls_flag_t;

typedef struct {
        ble_service_t svc;

        // handles
        uint16_t blood_measurement_h;
        uint16_t blood_measurement_ccc_h;
        uint16_t interm_cuff_pressure_h;
        uint16_t interm_cuff_pressure_ccc_h;

        // callbacks
        const bls_callbacks_t   *cb;

        // Blood Pressure Feature support
        bls_feature_t bls_feature;
} bl_service_t;

static att_error_t do_blood_measure_ccc_write(bl_service_t *bls, uint16_t conn_idx, uint16_t offset,
                                                        uint16_t length, const uint8_t *value)
{
        uint16_t ccc_val;

        if (offset) {
                return ATT_ERROR_ATTRIBUTE_NOT_LONG;
        }

        if (length != sizeof(ccc_val)) {
                return ATT_ERROR_INVALID_VALUE_LENGTH;
        }

        ccc_val = get_u16(value);

        ble_storage_put_u32(conn_idx, bls->blood_measurement_ccc_h, ccc_val, true);

        if (bls->cb->meas_indication_changed) {
                bls->cb->meas_indication_changed(&bls->svc, conn_idx,
                                                                   ccc_val & GATT_CCC_INDICATIONS);
        }

        return ATT_ERROR_OK;
}

static att_error_t do_cuff_pressure_ccc_write(bl_service_t *bls, uint16_t conn_idx, uint16_t offset,
                                                        uint16_t length, const uint8_t *value)
{
        uint16_t ccc_val;

        if (offset) {
                return ATT_ERROR_ATTRIBUTE_NOT_LONG;
        }

        if (length != sizeof(ccc_val)) {
                return ATT_ERROR_INVALID_VALUE_LENGTH;
        }

        ccc_val = get_u16(value);

        ble_storage_put_u32(conn_idx, bls->interm_cuff_pressure_ccc_h, ccc_val, true);

        if (bls->cb->interm_cuff_pressure_notif_changed) {
                bls->cb->interm_cuff_pressure_notif_changed(&bls->svc, conn_idx,
                                                                 ccc_val & GATT_CCC_NOTIFICATIONS);
        }

        return ATT_ERROR_OK;
}

static void handle_read_req(ble_service_t *svc, const ble_evt_gatts_read_req_t *evt)
{
        bl_service_t *bls = (bl_service_t *) svc;
        uint16_t ccc_val = GATT_CCC_NONE;

        if (evt->handle == bls->blood_measurement_ccc_h ||
                                                  evt->handle == bls->interm_cuff_pressure_ccc_h) {
                ble_storage_get_u16(evt->conn_idx, evt->handle, &ccc_val);

                // we're little-endian, ok to write directly from uint16_t
                ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_OK,
                                                                        sizeof(ccc_val), &ccc_val);
        } else {
                ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_READ_NOT_PERMITTED, 0,
                                                                                             NULL);
        }
}

static void handle_write_req(ble_service_t *svc, const ble_evt_gatts_write_req_t *evt)
{
        bl_service_t *bls = (bl_service_t *) svc;
        att_error_t status = ATT_ERROR_WRITE_NOT_PERMITTED;

        if (evt->handle == bls->blood_measurement_ccc_h) {
                status = do_blood_measure_ccc_write(bls, evt->conn_idx, evt->offset, evt->length,
                                                                                       evt->value);
                goto done;
        }

        if (evt->handle == bls->interm_cuff_pressure_ccc_h) {
                status = do_cuff_pressure_ccc_write(bls, evt->conn_idx, evt->offset, evt->length,
                                                                                       evt->value);
                goto done;
        }

done:
        ble_gatts_write_cfm(evt->conn_idx, evt->handle, status);
}

static void handle_connected_evt(ble_service_t *svc, const ble_evt_gap_connected_t *evt)
{
        bl_service_t *bls = (bl_service_t *) svc;
        uint16_t ccc_val = GATT_CCC_NONE;

        if (bls->cb->meas_indication_changed) {
                ble_storage_get_u16(evt->conn_idx, bls->blood_measurement_ccc_h, &ccc_val);

                bls->cb->meas_indication_changed(&bls->svc, evt->conn_idx,
                                                                   ccc_val & GATT_CCC_INDICATIONS);
        }

        if (bls->cb->interm_cuff_pressure_notif_changed && bls->interm_cuff_pressure_ccc_h) {
                ble_storage_get_u16(evt->conn_idx, bls->interm_cuff_pressure_ccc_h, &ccc_val);

                bls->cb->interm_cuff_pressure_notif_changed(&bls->svc, evt->conn_idx,
                                                                 ccc_val & GATT_CCC_NOTIFICATIONS);
        }
}

static void handle_disconnected_evt(ble_service_t *svc, const ble_evt_gap_disconnected_t *evt)
{
        bl_service_t *bls = (bl_service_t *) svc;

        if (bls->cb->meas_indication_changed) {
                bls->cb->meas_indication_changed(&bls->svc, evt->conn_idx, false);
        }

        if (bls->cb->interm_cuff_pressure_notif_changed && bls->interm_cuff_pressure_h) {
                bls->cb->interm_cuff_pressure_notif_changed(&bls->svc, evt->conn_idx, false);
        }
}

static void handle_cleanup(ble_service_t *svc)
{
        bl_service_t *bls = (bl_service_t *) svc;

        ble_storage_remove_all(bls->blood_measurement_ccc_h);
        ble_storage_remove_all(bls->interm_cuff_pressure_ccc_h);

        OS_FREE(bls);
}

static void handle_event_sent_evt(ble_service_t *svc, const ble_evt_gatts_event_sent_t *evt)
{
        bl_service_t *bls = (bl_service_t *) svc;

        if (evt->handle == bls->blood_measurement_h && bls->cb->meas_indication_sent) {
                bls->cb->meas_indication_sent(evt->conn_idx, evt->status);
        }
}

static uint8_t get_num_attr(const ble_service_config_t *config, const bls_config_t *bls_config)
{
        /*
         * Mandatory characteristics:
         * - Blood Pressure Measurement
         * - Blood Pressure Feature
         *
         * Mandatory descriptors:
         * - Blood Pressure Measurement
         */
        uint16_t num_chars = 2;
        uint16_t num_descs = 1;

        /*
         * If user decided to receive Intermediate Cuff Pressure then this characteristic with its
         * mandatory descriptor should be supported in service.
         */
        if (bls_config->supported_char & BLS_SUPPORTED_CHAR_INTERM_CUFF_PRESSURE) {
                num_chars++;
                num_descs++;
        }

        return ble_service_get_num_attr(config, num_chars, num_descs);
}

ble_service_t *bls_init(const ble_service_config_t *config, const bls_config_t *bls_config,
                                                                        const bls_callbacks_t *cb)
{
        bl_service_t *bls;
        att_uuid_t uuid;
        uint16_t num_attr;
        att_perm_t perm_read;
        uint8_t value[2];
        uint16_t bls_feature_h;

        if (!cb) {
                return NULL;
        }

        bls = OS_MALLOC(sizeof(*bls));
        memset(bls, 0, sizeof(*bls));

        bls->svc.read_req = handle_read_req;
        bls->svc.write_req = handle_write_req;
        bls->svc.connected_evt = handle_connected_evt;
        bls->svc.disconnected_evt = handle_disconnected_evt;
        bls->svc.cleanup = handle_cleanup;
        bls->svc.event_sent = handle_event_sent_evt;

        perm_read = ble_service_config_elevate_perm(ATT_PERM_READ, config);

        num_attr = get_num_attr(config, bls_config);

        ble_service_config_add_includes(config);

        // Add service
        ble_uuid_create16(UUID_SERVICE_BLS, &uuid);
        ble_gatts_add_service(&uuid, config->service_type, num_attr);

        // Add characteristics and descriptors
        ble_uuid_create16(UUID_BLOOD_PRESSURE_MEASUREMENT, &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_INDICATE, ATT_PERM_NONE, 0, 0, NULL,
                                                                        &bls->blood_measurement_h);

        ble_uuid_create16(UUID_GATT_CLIENT_CHAR_CONFIGURATION, &uuid);
        ble_gatts_add_descriptor(&uuid, ATT_PERM_RW, sizeof(uint16_t), 0,
                                                                    &bls->blood_measurement_ccc_h);

        // Add optional Intermediate Cuff Pressure characteristic if supported
        if (bls_config->supported_char & BLS_SUPPORTED_CHAR_INTERM_CUFF_PRESSURE) {
                ble_uuid_create16(UUID_INTERMEDIATE_CUFF_PRESSURE, &uuid);
                ble_gatts_add_characteristic(&uuid, GATT_PROP_NOTIFY, ATT_PERM_NONE, 0, 0, NULL,
                                                                     &bls->interm_cuff_pressure_h);

                ble_uuid_create16(UUID_GATT_CLIENT_CHAR_CONFIGURATION, &uuid);
                ble_gatts_add_descriptor(&uuid, ATT_PERM_RW, sizeof(uint16_t), 0,
                                                                &bls->interm_cuff_pressure_ccc_h);
        }

        ble_uuid_create16(UUID_BLOOD_PRESSURE_FEATURE, &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_READ, perm_read, BLS_FEATURE_SIZE,
                                                                          0, NULL, &bls_feature_h);

        // Register service
        ble_gatts_register_service(&bls->svc.start_h, &bls->blood_measurement_h,
                                                 &bls->blood_measurement_ccc_h, &bls_feature_h, 0);

        if (bls->interm_cuff_pressure_h) {
                bls->interm_cuff_pressure_h += bls->svc.start_h;
                bls->interm_cuff_pressure_ccc_h += bls->svc.start_h;
        }

        put_u16(value, bls_config->feature_supp);
        ble_gatts_set_value(bls_feature_h, sizeof(value), value);

        bls->svc.end_h = bls->svc.start_h + num_attr;

        bls->cb = cb;
        bls->bls_feature = bls_config->feature_supp;

        // Add service in service framework
        ble_service_add(&bls->svc);

        return &bls->svc;
}

static uint16_t calculate_measurement_status(const bls_measurement_status_t *measurement_status,
                                                                            bls_feature_t features)
{
        uint16_t status = 0;

        status |= (measurement_status->body_movement)
                                          << MEASUREMENT_STATUS_BODY_MOVEMENT_BIT_POSITION;
        status |= (measurement_status->cuff_fit)
                                          << MEASUREMENT_STATUS_CUFF_FIT_BIT_POSITION;
        status |= (measurement_status->irregular_pulse)
                                          << MEASUREMENT_STATUS_IRREGULAR_PULSE_BIT_POSITION;
        status |= (measurement_status->pulse_rate_range)
                                          << MEASUREMENT_STATUS_PULSE_RATE_RANGE_BIT_POSITION;
        status |= (measurement_status->measurement_pos)
                                          << MEASUREMENT_STATUS_MEASUREMENT_POS_BIT_POSITION;

        return status;
}

static uint8_t pack_send_value(bls_feature_t blfs, const bls_measurement_t *measurement,
                                                                                    uint8_t *value)
{
        uint8_t *ptr = &value[1];
        uint8_t flags = 0;

        if (measurement->unit == BLS_PRESSURE_UNIT_KPA) {
                flags |= BLS_FLAG_UNIT_KPA;
        }

        put_u16_inc(&ptr, pack_ieee11703_sfloat(&measurement->pressure_systolic));
        put_u16_inc(&ptr, pack_ieee11703_sfloat(&measurement->pressure_diastolic));
        put_u16_inc(&ptr, pack_ieee11703_sfloat(&measurement->pressure_map));

        if (measurement->time_stamp_present) {
                flags |= BLS_FLAG_TIME_STAMP_SUPPORTED;
                pack_date_time(&measurement->time_stamp, &ptr);
        }

        if (measurement->pulse_rate_present) {
                flags |= BLS_FLAG_PULSE_RATE;
                put_u16_inc(&ptr, pack_ieee11703_sfloat(&measurement->pulse_rate));
        }

        if (measurement->user_id_present) {
                flags |= BLS_FLAG_USER_ID;
                put_u8_inc(&ptr, measurement->user_id);
        }

        if (measurement->measurement_status_present) {
                flags |= BLS_FLAG_MEASUREMENT_STATUS;
                put_u16_inc(&ptr, calculate_measurement_status(&measurement->measurement_status,
                                                                                            blfs));
        }

        put_u8(value, flags);

        return ptr - value;
}

bool bls_indicate_pressure_measurement(ble_service_t *svc, uint16_t conn_idx,
                                                              const bls_measurement_t *measurement)
{
        ble_error_t status;
        uint16_t ccc_val = GATT_CCC_NONE;
        bl_service_t *bls = (bl_service_t *) svc;
        uint8_t value[BLS_MAX_MEASUREMENT_SIZE];
        uint8_t value_len;

        ble_storage_get_u16(conn_idx, bls->blood_measurement_ccc_h, &ccc_val);

        if (!(ccc_val & GATT_CCC_INDICATIONS)) {
                // indication disabled by client
                return false;
        }

        value_len = pack_send_value(bls->bls_feature, measurement, value);

        status = ble_gatts_send_event(conn_idx, bls->blood_measurement_h, GATT_EVENT_INDICATION,
                                                                                 value_len, value);

        return status == BLE_STATUS_OK;
}

bool bls_notify_intermediate_cuff_pressure(ble_service_t *svc, uint16_t conn_idx,
                                                              const bls_measurement_t *measurement)
{
        ble_error_t status;
        uint16_t ccc_val = GATT_CCC_NONE;
        bl_service_t *bls = (bl_service_t *) svc;
        uint8_t value[BLS_MAX_MEASUREMENT_SIZE];
        uint8_t value_len;

        ble_storage_get_u16(conn_idx, bls->interm_cuff_pressure_ccc_h, &ccc_val);

        if (!(ccc_val & GATT_CCC_NOTIFICATIONS)) {
                // notification disabled by client
                return false;
        }

        value_len = pack_send_value(bls->bls_feature, measurement, value);

        status = ble_gatts_send_event(conn_idx, bls->interm_cuff_pressure_h,
                                                        GATT_EVENT_NOTIFICATION, value_len, value);

        return status == BLE_STATUS_OK;
}
