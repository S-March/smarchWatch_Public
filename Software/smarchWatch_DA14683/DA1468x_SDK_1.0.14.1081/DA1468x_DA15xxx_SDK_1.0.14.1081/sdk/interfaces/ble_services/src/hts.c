/**
 ****************************************************************************************
 *
 * @file hts.c
 *
 * @brief Health Thermometer Service implementation
 *
 * Copyright (C) 2017 Dialog Semiconductor.
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
#include "ble_storage.h"
#include "ble_bufops.h"
#include "ble_common.h"
#include "ble_gatt.h"
#include "ble_gatts.h"
#include "ble_uuid.h"
#include "hts.h"

#define UUID_TEMPERATURE_MEASUREMENT    (0x2A1C)
#define UUID_TEMPERATURE_TYPE           (0x2A1D)
#define UUID_INTERMEDIATE_TEMPERATURE   (0x2A1E)
#define UUID_MEASUREMENT_INTERVAL       (0x2A21)
/* Characteristic Valid Range descriptor */
#define UUID_VALID_RANGE                (0x2906)

#define CHAR_MAX_LEN_TEMPERATURE_MEASUREMENT    (13)
#define CHAR_MAX_LEN_TEMPERATURE_TYPE           (1)
#define CHAR_MAX_LEN_INTERMEDIATE_TEMPERATURE   (13)
#define CHAR_MAX_LEN_MEASUREMENT_INTERVAL       (2)

enum {
        HTS_ERROR_OUT_OF_RANGE = ATT_ERROR_APPLICATION_ERROR,
};

enum measurement_flags {
        MEAS_FLAG_TEMP_UNIT_FAHRENHEIT  = 0x01, /**< Temperature in Fahrenheits */
        MEAS_FLAG_TIME_STAMP_SUPPORTED  = 0x02, /**< Time Stamp is supported */
        MEAS_FLAG_TEMP_TYPE_SUPPORTED   = 0x04, /**< Temperature type is supported */
};

typedef struct {
        ble_service_t svc;

        // handles
        uint16_t meas_h;
        uint16_t meas_ccc_h;
        uint16_t inter_temp_h;
        uint16_t inter_temp_ccc_h;
        uint16_t meas_interval_h;
        uint16_t meas_interval_ccc_h;

        // callbacks
        const hts_callbacks_t *cb;

        // service features
        hts_feature_t features;

        struct {
                uint16_t low_bound;
                uint16_t high_bound;
        } __attribute__((__packed__)) range;
} ht_service_t;

static att_error_t do_meas_ccc_write(ht_service_t *hts, uint16_t conn_idx, uint16_t offset,
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

        ble_storage_put_u32(conn_idx, hts->meas_ccc_h, ccc_val, true);

        if (hts->cb->temp_meas_indication_changed) {
                hts->cb->temp_meas_indication_changed(conn_idx, ccc_val & GATT_CCC_INDICATIONS);
        }

        return ATT_ERROR_OK;
}

static att_error_t do_inter_temp_ccc_write(ht_service_t *hts, uint16_t conn_idx, uint16_t offset,
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

        ble_storage_put_u32(conn_idx, hts->inter_temp_ccc_h, ccc_val, true);

        if (hts->cb->interm_temp_notification_changed) {
                hts->cb->interm_temp_notification_changed(conn_idx, ccc_val & GATT_CCC_NOTIFICATIONS);
        }

        return ATT_ERROR_OK;
}

static att_error_t do_meas_interval_ccc_write(ht_service_t *hts, uint16_t conn_idx, uint16_t offset,
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

        ble_storage_put_u32(conn_idx, hts->meas_interval_ccc_h, ccc_val, true);

        if (hts->cb->meas_interval_indication_changed) {
                hts->cb->meas_interval_indication_changed(conn_idx, ccc_val & GATT_CCC_INDICATIONS);
        }

        return ATT_ERROR_OK;
}

static att_error_t do_meas_interval_write(ht_service_t *hts, uint16_t conn_idx, uint16_t offset,
                                                        uint16_t length, const uint8_t *value)
{
        uint16_t meas_inter_val;

        if (offset) {
                return ATT_ERROR_ATTRIBUTE_NOT_LONG;
        }

        if (length != sizeof(meas_inter_val)) {
                return ATT_ERROR_INVALID_VALUE_LENGTH;
        }

        meas_inter_val = get_u16(value);

        if ((meas_inter_val < hts->range.low_bound || meas_inter_val > hts->range.high_bound) &&
                                                                        meas_inter_val != 0) {
                return HTS_ERROR_OUT_OF_RANGE;
        }

        hts->cb->meas_interval_set(conn_idx, meas_inter_val);

        return ATT_ERROR_OK;
}

static void handle_read_req(ble_service_t *svc, const ble_evt_gatts_read_req_t *evt)
{
        ht_service_t *hts = (ht_service_t *) svc;

        if (evt->handle == hts->meas_ccc_h || evt->handle == hts->inter_temp_ccc_h ||
                                                        evt->handle == hts->meas_interval_ccc_h) {
                uint16_t ccc_val = GATT_CCC_NONE;

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
        ht_service_t *hts = (ht_service_t *) svc;
        att_error_t status = ATT_ERROR_ATTRIBUTE_NOT_FOUND;

        if (evt->handle == hts->meas_ccc_h) {
                status = do_meas_ccc_write(hts, evt->conn_idx, evt->offset, evt->length, evt->value);
                goto done;
        }

        if (evt->handle == hts->inter_temp_ccc_h) {
                status = do_inter_temp_ccc_write(hts, evt->conn_idx, evt->offset, evt->length,
                                                                                        evt->value);
                goto done;
        }

        if (evt->handle == hts->meas_interval_h) {
                status = do_meas_interval_write(hts, evt->conn_idx, evt->offset, evt->length,
                                                                                        evt->value);
                if (status == ATT_ERROR_OK) {
                        return;
                }

                goto done;
        }

        if (evt->handle == hts->meas_interval_ccc_h) {
                status = do_meas_interval_ccc_write(hts, evt->conn_idx, evt->offset, evt->length,
                                                                                        evt->value);
                goto done;
        }

done:
        ble_gatts_write_cfm(evt->conn_idx, evt->handle, status);
}

static void handle_event_sent_evt(ble_service_t *svc, const ble_evt_gatts_event_sent_t *evt)
{
        ht_service_t *hts = (ht_service_t *) svc;

        if (evt->handle == hts->meas_h) {
                if (hts->cb->temp_meas_indication_sent) {
                        hts->cb->temp_meas_indication_sent(evt->conn_idx, evt->status);
                }
                return;
        }

        if (evt->handle == hts->inter_temp_h) {
                if (hts->cb->interm_temp_notification_sent) {
                        hts->cb->interm_temp_notification_sent(evt->conn_idx, evt->status);
                }
                return;
        }

        if (evt->handle == hts->meas_interval_h) {
                if (hts->cb->meas_interval_indication_sent) {
                        hts->cb->meas_interval_indication_sent(evt->conn_idx, evt->status);
                }
                return;
        }
}

static void handle_connected_evt(ble_service_t *svc, const ble_evt_gap_connected_t *evt)
{
        uint16_t ccc_val;
        ht_service_t *hts = (ht_service_t *) svc;

        if (hts->cb->temp_meas_indication_changed) {
                ccc_val = GATT_CCC_NONE;
                ble_storage_get_u16(evt->conn_idx, hts->meas_ccc_h, &ccc_val);
                hts->cb->temp_meas_indication_changed(evt->conn_idx, ccc_val & GATT_CCC_INDICATIONS);
        }

        if (hts->cb->interm_temp_notification_changed && hts->inter_temp_ccc_h) {
                ccc_val = GATT_CCC_NONE;
                ble_storage_get_u16(evt->conn_idx, hts->inter_temp_ccc_h, &ccc_val);
                hts->cb->interm_temp_notification_changed(evt->conn_idx,
                                                                ccc_val & GATT_CCC_NOTIFICATIONS);
        }

        if (hts->cb->meas_interval_indication_changed && hts->meas_interval_ccc_h) {
                ccc_val = GATT_CCC_NONE;
                ble_storage_get_u16(evt->conn_idx, hts->meas_interval_ccc_h, &ccc_val);
                hts->cb->meas_interval_indication_changed(evt->conn_idx,
                                                                ccc_val & GATT_CCC_INDICATIONS);
        }
}

static void handle_disconnected_evt(ble_service_t *svc, const ble_evt_gap_disconnected_t *evt)
{
        ht_service_t *hts = (ht_service_t *) svc;

        if (hts->cb->temp_meas_indication_changed) {
                hts->cb->temp_meas_indication_changed(evt->conn_idx, false);
        }

        if (hts->cb->interm_temp_notification_changed && hts->inter_temp_ccc_h) {
                hts->cb->interm_temp_notification_changed(evt->conn_idx, false);
        }

        if (hts->cb->meas_interval_indication_changed && hts->meas_interval_ccc_h) {
                hts->cb->meas_interval_indication_changed(evt->conn_idx, false);
        }
}

static uint8_t get_num_attr(const ble_service_config_t *config,
                                        const hts_config_t *hts_config, const hts_callbacks_t *cb)
{
        /*
         * Temperature Measurement characteristic and its CCC are mandatory.
         */
        uint16_t num_chars = 1;
        uint16_t num_descs = 1;

        /*
         * If Temperature Type values are static then Temperature Type characteristic should be
         * supported in the service.
         */
        if (hts_config->features & HTS_FEATURE_TEMPERATURE_TYPE) {
                num_chars++;
        }

        /*
         * If user decided to receive Intermediate Temperature values then this characteristic with
         * appropriate descriptors should be supported in service.
         */
        if (hts_config->features & HTS_FEATURE_INTERMEDIATE_TEMP) {
                num_chars++;
                num_descs++;
        }

        /*
         * Measurement Interval characteristic is needed when user wants to control the interval
         * between consecutive temperature measurements, otherwise no periodic measurement shall be
         * made.
         */
        if (hts_config->features & HTS_FEATURE_MEASUREMENT_INTERVAL) {
                num_chars++;

                /*
                 * CCC descriptor should be only used when characteristic has indications enabled.
                 */
                if (hts_config->features & HTS_FEATURE_MEASUREMENT_INTERVAL_INDICATIONS) {
                        num_descs++;
                }
                /*
                 * Valid Range descriptor should be only used when characteristic is writable.
                 */
                if (hts_config->features & HTS_FEATURE_MEASUREMENT_INTERVAL_WRITABLE) {
                        num_descs++;
                }
        }

        return ble_service_get_num_attr(config, num_chars, num_descs);
}

static void cleanup(ble_service_t *svc)
{
        ht_service_t *hts = (ht_service_t *) svc;

        ble_storage_remove_all(hts->meas_ccc_h);
        ble_storage_remove_all(hts->meas_interval_ccc_h);
        ble_storage_remove_all(hts->inter_temp_ccc_h);

        OS_FREE(hts);
}

ble_service_t *hts_init(const ble_service_config_t *config, const hts_config_t *hts_config,
                                                                        const hts_callbacks_t *cb)
{
        att_uuid_t uuid;
        att_uuid_t ccc_uuid;
        att_perm_t read_perm;
        uint16_t num_attr;
        uint16_t temp_type_h = 0;
        uint16_t mi_valid_range_h = 0;
        ht_service_t *hts;

        if (!cb) {
                return NULL;
        }

        if (hts_config->init_interval != 0 && cb->meas_interval_set) {
                if (hts_config->init_interval < hts_config->interval_bound_low ||
                                hts_config->init_interval > hts_config->interval_bound_high) {
                        return NULL;
                }

                if (hts_config->interval_bound_low > hts_config->interval_bound_high) {
                        return NULL;
                }
        }

        hts = OS_MALLOC(sizeof(*hts));
        memset(hts, 0, sizeof(*hts));

        hts->svc.read_req = handle_read_req;
        hts->svc.write_req = handle_write_req;
        hts->svc.event_sent = handle_event_sent_evt;
        hts->svc.connected_evt = handle_connected_evt;
        hts->svc.disconnected_evt = handle_disconnected_evt;
        hts->svc.cleanup = cleanup;

        num_attr = get_num_attr(config, hts_config, cb);
        read_perm = ble_service_config_elevate_perm(ATT_PERM_READ, config);

        ble_uuid_create16(UUID_SERVICE_HTS, &uuid);
        ble_gatts_add_service(&uuid, config->service_type, num_attr);

        ble_service_config_add_includes(config);

        ble_uuid_create16(UUID_TEMPERATURE_MEASUREMENT, &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_INDICATE, ATT_PERM_NONE,
                                        CHAR_MAX_LEN_TEMPERATURE_MEASUREMENT, 0, NULL, &hts->meas_h);

        ble_uuid_create16(UUID_GATT_CLIENT_CHAR_CONFIGURATION, &ccc_uuid);
        ble_gatts_add_descriptor(&ccc_uuid, ATT_PERM_RW, sizeof(uint16_t), 0, &hts->meas_ccc_h);

        /*
         * Temperature Type characteristic should be used only when the value is static while in
         * a connection.
         */
        if (hts_config->features & HTS_FEATURE_TEMPERATURE_TYPE) {
                ble_uuid_create16(UUID_TEMPERATURE_TYPE, &uuid);
                ble_gatts_add_characteristic(&uuid, GATT_PROP_READ, read_perm,
                                                CHAR_MAX_LEN_TEMPERATURE_TYPE, 0, NULL, &temp_type_h);
        }

        /*
         * Intermediate Temperature is optional so user can decide if this measurements are needed
         * or not for him.
         */
        if (hts_config->features & HTS_FEATURE_INTERMEDIATE_TEMP) {
                ble_uuid_create16(UUID_INTERMEDIATE_TEMPERATURE, &uuid);
                ble_gatts_add_characteristic(&uuid, GATT_PROP_NOTIFY, ATT_PERM_NONE,
                                                CHAR_MAX_LEN_INTERMEDIATE_TEMPERATURE, 0, NULL,
                                                                                &hts->inter_temp_h);

                ble_gatts_add_descriptor(&ccc_uuid, ATT_PERM_RW, sizeof(uint16_t), 0,
                                                                        &hts->inter_temp_ccc_h);
        }

        /*
         * Measurement Interval characteristic is supported only when user wants to use periodic
         * measurements, setting HTS_FEAT_PERIODIC_MEAS_SUPP.
         */
        if (hts_config->features & HTS_FEATURE_MEASUREMENT_INTERVAL) {
                att_perm_t meas_inter_perm;
                gatt_prop_t meas_inter_prop;

                meas_inter_prop = GATT_PROP_READ;
                meas_inter_perm = read_perm;

                if (hts_config->features & HTS_FEATURE_MEASUREMENT_INTERVAL_INDICATIONS) {
                        meas_inter_prop |= GATT_PROP_INDICATE;
                }

                if (hts_config->features & HTS_FEATURE_MEASUREMENT_INTERVAL_WRITABLE) {
                        meas_inter_prop |= GATT_PROP_WRITE;
                        meas_inter_perm |= ble_service_config_elevate_perm(ATT_PERM_WRITE_ENCRYPT,
                                                                                        config);
                }

                ble_uuid_create16(UUID_MEASUREMENT_INTERVAL, &uuid);
                ble_gatts_add_characteristic(&uuid, meas_inter_prop, meas_inter_perm,
                                                CHAR_MAX_LEN_MEASUREMENT_INTERVAL, 0, NULL,
                                                                        &hts->meas_interval_h);

                if (hts_config->features & HTS_FEATURE_MEASUREMENT_INTERVAL_INDICATIONS) {
                        ble_gatts_add_descriptor(&ccc_uuid, ATT_PERM_RW, sizeof(uint16_t), 0,
                                                                        &hts->meas_interval_ccc_h);
                }

                if (meas_inter_prop & GATT_PROP_WRITE) {
                        ble_uuid_create16(UUID_VALID_RANGE, &uuid);
                        ble_gatts_add_descriptor(&uuid, ATT_PERM_READ, sizeof(uint32_t), 0,
                                                                                &mi_valid_range_h);
                }
        }

        ble_gatts_register_service(&hts->svc.start_h, &hts->meas_h, &hts->meas_ccc_h, 0);

        if (temp_type_h) {
                temp_type_h += hts->svc.start_h;
        }

        if (hts->inter_temp_h) {
                hts->inter_temp_h += hts->svc.start_h;
                hts->inter_temp_ccc_h += hts->svc.start_h;
        }

        if (hts->meas_interval_h) {
                hts->meas_interval_h += hts->svc.start_h;
                if (hts->meas_interval_ccc_h) {
                        hts->meas_interval_ccc_h += hts->svc.start_h;
                }
                if (mi_valid_range_h) {
                        mi_valid_range_h += hts->svc.start_h;
                }
        }

        if (temp_type_h) {
                uint8_t temp_type = hts_config->type;
                ble_gatts_set_value(temp_type_h, sizeof(temp_type), &temp_type);
        }

        if (hts->meas_interval_h) {
                ble_gatts_set_value(hts->meas_interval_h, sizeof(hts_config->init_interval),
                                                                        &hts_config->init_interval);

                if (mi_valid_range_h) {
                        // Set interval valid range
                        hts->range.low_bound = hts_config->interval_bound_low;
                        hts->range.high_bound = hts_config->interval_bound_high;
                        ble_gatts_set_value(mi_valid_range_h, sizeof(hts->range), &hts->range);
                }
        }

        hts->svc.end_h = hts->svc.start_h + num_attr;

        hts->cb = cb;

        ble_service_add(&hts->svc);

        return &hts->svc;
}

static uint8_t pack_measurement(const hts_temp_measurement_t *meas, uint8_t *value)
{
        uint8_t *ptr = &value[1];
        uint8_t flags = 0;
        uint32_t temp_val;

        temp_val = pack_ieee11703_float(&meas->temperature);
        put_u32_inc(&ptr, temp_val);

        if (meas->unit == HTS_TEMP_UNIT_FAHRENHEIT) {
                flags |= MEAS_FLAG_TEMP_UNIT_FAHRENHEIT;
        }

        if (meas->has_time_stamp) {
                flags |= MEAS_FLAG_TIME_STAMP_SUPPORTED;
                pack_date_time(&meas->time_stamp, &ptr);
        }

        if (meas->has_temp_type) {
                flags |= MEAS_FLAG_TEMP_TYPE_SUPPORTED;
                put_u8_inc(&ptr, meas->temp_type);
        }

        put_u8(value, flags);

        return ptr - value;
}

bool hts_indicate_temperature(ble_service_t *svc, uint16_t conn_idx,
                                                        const hts_temp_measurement_t *measurement)
{
        ht_service_t *hts = (ht_service_t *) svc;
        uint8_t value[CHAR_MAX_LEN_TEMPERATURE_MEASUREMENT];
        uint8_t value_len;
        ble_error_t ret;
        uint16_t ccc_val = GATT_CCC_NONE;

        ble_storage_get_u16(conn_idx, hts->meas_ccc_h, &ccc_val);

        if (!(ccc_val & GATT_CCC_INDICATIONS)) {
                // indication disabled by client
                return false;
        }

        value_len = pack_measurement(measurement, value);

        ret = ble_gatts_send_event(conn_idx, hts->meas_h, GATT_EVENT_INDICATION, value_len, value);

        return ret == BLE_STATUS_OK;
}

bool hts_set_measurement_interval(ble_service_t *svc, uint16_t interval)
{
        att_error_t ret;
        ht_service_t *hts = (ht_service_t *) svc;

        ret = ble_gatts_set_value(hts->meas_interval_h, sizeof(interval), &interval);

        return ret == ATT_ERROR_OK;
}

bool hts_indicate_measurement_interval(ble_service_t *svc, uint16_t conn_idx)
{
        ht_service_t *hts = (ht_service_t *) svc;
        uint16_t ccc_val = GATT_CCC_NONE;
        uint16_t interval;
        uint16_t interval_len = sizeof(interval);
        ble_error_t ret;

        if (!hts->meas_interval_h) {
                // measurement interval is not supported
                return false;
        }

        ble_storage_get_u16(conn_idx, hts->meas_interval_ccc_h, &ccc_val);

        if (!(ccc_val & GATT_CCC_INDICATIONS)) {
                // indication disabled by client
                return false;
        }

        ble_gatts_get_value(hts->meas_interval_ccc_h, &interval_len, &interval);

        ret = ble_gatts_send_event(conn_idx, hts->meas_interval_h, GATT_EVENT_INDICATION,
                                                                        sizeof(interval), &interval);

        return ret == BLE_STATUS_OK;
}

bool hts_notify_interm_temperature(ble_service_t *svc, uint16_t conn_idx,
                                                        const hts_temp_measurement_t *measurement)
{
        ht_service_t *hts = (ht_service_t *) svc;
        uint16_t ccc_val = GATT_CCC_NONE;
        uint8_t value[CHAR_MAX_LEN_INTERMEDIATE_TEMPERATURE];
        uint8_t value_len;
        ble_error_t ret;

        if (!hts->inter_temp_h) {
                // intermediate temperature is not supported
                return false;
        }

        ble_storage_get_u16(conn_idx, hts->inter_temp_ccc_h, &ccc_val);

        if (!(ccc_val & GATT_CCC_NOTIFICATIONS)) {
                // notification disabled by client
                return false;
        }

        value_len = pack_measurement(measurement, value);

        ret = ble_gatts_send_event(conn_idx, hts->inter_temp_h, GATT_EVENT_NOTIFICATION, value_len,
                                                                                        value);

        return ret == BLE_STATUS_OK;
}

void hts_set_meas_interval_cfm(ble_service_t *svc, uint16_t conn_idx, att_error_t status)
{
        ht_service_t *hts = (ht_service_t *) svc;

        if (!hts->meas_interval_h) {
                return;
        }

        ble_gatts_write_cfm(conn_idx, hts->meas_interval_h, status);
}
