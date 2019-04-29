/**
 ****************************************************************************************
 *
 * @file cts.c
 *
 * @brief Current Time Service sample implementation
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
#include "ble_storage.h"
#include "ble_uuid.h"
#include "svc_defines.h"
#include "svc_types.h"
#include "cts.h"

#define UUID_LOCAL_TIME_INFORMATION             (0x2A0F)
#define UUID_REFERENCE_TIME_INFORMATION         (0x2A14)
#define UUID_CURRENT_TIME                       (0x2A2B)

typedef struct {
        ble_service_t svc;

        // callbacks
        const cts_callbacks_t *cb;

        // handles
        uint16_t ct_val_h;      // Current Time
        uint16_t ct_ccc_h;      // Current Time CCC
        uint16_t lti_val_h;     // Local Time Information
        uint16_t rti_val_h;     // Reference Time Information
} ct_service_t;

static void pack_time(const cts_current_time_t *time, uint8_t value[10])
{
        uint8_t *ptr = value;

        // Prepare response
        pack_date_time(&time->date_time, &ptr);
        put_u8_inc(&ptr, time->day_of_week);
        put_u8_inc(&ptr, time->fractions_256);
        put_u8_inc(&ptr, time->adjust_reason);
}

static void do_ct_read(ct_service_t *cts, const ble_evt_gatts_read_req_t *evt)
{
        if (!cts->cb || !cts->cb->get_time) {
                ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_READ_NOT_PERMITTED, 0, NULL);
                return;
        }

        // Ask client for reference time information
        cts->cb->get_time(&cts->svc, evt->conn_idx);

        // callback executed properly, will be replied by cts_get_time_cfm call
}

static void do_rti_read(ct_service_t *cts, const ble_evt_gatts_read_req_t *evt)
{
        if (!cts->cb || !cts->cb->get_ref_time_info) {
                ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_READ_NOT_PERMITTED, 0, NULL);
                return;
        }

        // Ask client for reference time information
        cts->cb->get_ref_time_info(&cts->svc, evt->conn_idx);

        // callback executed properly, will be replied by cts_get_ref_time_info_cfm call
}

static att_error_t do_ct_write(ct_service_t *cts, uint16_t conn_idx, uint16_t offset,
                                                        uint16_t length, const uint8_t *value)
{
        cts_current_time_t ct;

        if (offset) {
                return ATT_ERROR_ATTRIBUTE_NOT_LONG;
        }

        if (length != 10) {
                return ATT_ERROR_INVALID_VALUE_LENGTH;
        }

        if (!cts->cb || !cts->cb->set_time) {
                return ATT_ERROR_WRITE_NOT_PERMITTED;
        }

        ct.date_time.year = get_u16_inc(&value);
        ct.date_time.month = get_u8_inc(&value);
        ct.date_time.day = get_u8_inc(&value);
        ct.date_time.hours = get_u8_inc(&value);
        ct.date_time.minutes = get_u8_inc(&value);
        ct.date_time.seconds = get_u8_inc(&value);
        ct.day_of_week = get_u8_inc(&value);
        ct.fractions_256 = get_u8_inc(&value);
        ct.adjust_reason = get_u8_inc(&value);

        cts->cb->set_time(&cts->svc, conn_idx, &ct);

        return ATT_ERROR_OK;
}

static att_error_t do_ct_ccc_write(ct_service_t *cts, uint16_t conn_idx, uint16_t offset,
                                                                uint16_t length, const uint8_t *value)
{
        uint16_t ccc;

        if (offset) {
                return ATT_ERROR_ATTRIBUTE_NOT_LONG;
        }

        if (length != sizeof(ccc)) {
                return ATT_ERROR_INVALID_VALUE_LENGTH;
        }

        ccc = get_u16(value);

        ble_storage_put_u32(conn_idx, cts->ct_ccc_h, ccc, true);

        return ATT_ERROR_OK;
}

static att_error_t do_lti_write(ct_service_t *cts, uint16_t conn_idx, uint16_t offset,
                                                        uint16_t length, const uint8_t *value)
{
        cts_local_time_info_t lti;

        if (offset) {
                return ATT_ERROR_ATTRIBUTE_NOT_LONG;
        }

        if (length != 2) {
                return ATT_ERROR_INVALID_VALUE_LENGTH;
        }

        if (!cts->cb || !cts->cb->set_local_time_info) {
                return ATT_ERROR_WRITE_NOT_PERMITTED;
        }

        lti.time_zone = get_u8(value);
        lti.dst = get_u8(value + 1);

        cts->cb->set_local_time_info(&cts->svc, conn_idx, &lti);

        return (att_error_t) -1;
}

static void handle_read_req(ble_service_t *svc, const ble_evt_gatts_read_req_t *evt)
{
        ct_service_t *cts = (ct_service_t *) svc;

        if (evt->handle == cts->ct_val_h) {             // Current Time
                do_ct_read(cts, evt);
        } else if (evt->handle == cts->rti_val_h) {         // Reference Time Information
                do_rti_read(cts, evt);
        } else if (evt->handle == cts->ct_ccc_h) {      // Current Time CCC
                uint16_t ccc = 0x0000;

                ble_storage_get_u16(evt->conn_idx, cts->ct_ccc_h, &ccc);

                // we're little-endian, ok to write directly from uint16_t
                ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_OK, sizeof(ccc), &ccc);
        } else {
                ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_READ_NOT_PERMITTED, 0, NULL);
        }
}

static void handle_write_req(ble_service_t *svc, const ble_evt_gatts_write_req_t *evt)
{
        ct_service_t *cts = (ct_service_t *) svc;
        att_error_t status = ATT_ERROR_WRITE_NOT_PERMITTED;

        if (evt->handle == cts->ct_val_h) {             // Current Time
                status = do_ct_write(cts, evt->conn_idx, evt->offset, evt->length, evt->value);
                goto done;
        } else if (evt->handle == cts->ct_ccc_h) {      // Current Time CCC
                status = do_ct_ccc_write(cts, evt->conn_idx, evt->offset, evt->length, evt->value);
                goto done;
        } else if (evt->handle == cts->lti_val_h) {         // Local Time Information
                status = do_lti_write(cts, evt->conn_idx, evt->offset, evt->length, evt->value);
                goto done;
        }

done:
        if (status == ((att_error_t) -1)) {
                // write handler executed properly, will be replied by cfm call
                return;
        }

        ble_gatts_write_cfm(evt->conn_idx, evt->handle, status);
}

static void cleanup(ble_service_t *svc)
{
        ct_service_t *cts = (ct_service_t *) svc;

        ble_storage_remove_all(cts->ct_ccc_h);

        OS_FREE(cts);
}

ble_service_t *cts_init(const cts_local_time_info_t *info, const cts_callbacks_t *cb)
{
        ct_service_t *cts;
        uint16_t num_attr;
        att_uuid_t uuid;

        /* Make sure get_time is defined */
        OS_ASSERT(cb->get_time);

        cts = OS_MALLOC(sizeof(*cts));
        memset(cts, 0, sizeof(*cts));

        cts->svc.read_req = handle_read_req;
        cts->svc.write_req = handle_write_req;
        cts->svc.cleanup = cleanup;
        cts->cb = cb;

        /*
         * current time - one characteristic and one descriptor
         * local time information - optionally one characteristic
         * reference time information - optionally one characteristic
         */
        num_attr = ble_gatts_get_num_attr(0, 1 + (info ? 1 : 0) +
                                                (cb->get_ref_time_info ? 1 : 0), 1);

        ble_uuid_create16(UUID_SERVICE_CTS, &uuid);
        ble_gatts_add_service(&uuid, GATT_SERVICE_PRIMARY, num_attr);

        // Write is optional, if set_time_cb is not NULL make characteristic READ/WRITE
        ble_uuid_create16(UUID_CURRENT_TIME, &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_READ | GATT_PROP_NOTIFY |
                                                 (cb->set_time ? GATT_PROP_WRITE : 0),
                                                 (cb->set_time ? ATT_PERM_RW : ATT_PERM_READ) , 10,
                                                 GATTS_FLAG_CHAR_READ_REQ, NULL, &cts->ct_val_h);

        ble_uuid_create16(UUID_GATT_CLIENT_CHAR_CONFIGURATION, &uuid);
        ble_gatts_add_descriptor(&uuid, ATT_PERM_RW, 2, 0, &cts->ct_ccc_h);

        /* Optional characteristic Local Time Information */
        if (info) {
                const bool rw = cb->set_local_time_info != NULL;

                ble_uuid_create16(UUID_LOCAL_TIME_INFORMATION, &uuid);
                ble_gatts_add_characteristic(&uuid,
                                                GATT_PROP_READ | (rw ? GATT_PROP_WRITE : 0),
                                                rw ? ATT_PERM_RW : ATT_PERM_READ, 2, 0, NULL,
                                                                                &cts->lti_val_h);
        }

        /* Optional characteristic Reference Time Information */
        if (cb->get_ref_time_info) {
                ble_uuid_create16(UUID_REFERENCE_TIME_INFORMATION, &uuid);
                ble_gatts_add_characteristic(&uuid, GATT_PROP_READ,
                                                ATT_PERM_READ, 4, GATTS_FLAG_CHAR_READ_REQ, NULL,
                                                                                &cts->rti_val_h);
        }

        ble_gatts_register_service(&cts->svc.start_h, &cts->ct_val_h, &cts->ct_ccc_h, &cts->lti_val_h,
                                                                                &cts->rti_val_h, 0);

        cts->svc.end_h = cts->svc.start_h + num_attr;

        /*
         * Set initial value of Local Time Information if characteristic is there
         */
        if (info) {
                ble_gatts_set_value(cts->lti_val_h, 2, info);
        }

        ble_service_add(&cts->svc);

        return &cts->svc;
}

void cts_notify_time(ble_service_t *svc, uint16_t conn_idx, const cts_current_time_t *time)
{
        ct_service_t *cts = (ct_service_t *) svc;
        uint16_t ccc = 0x0000;
        uint8_t pdu[10];

        ble_storage_get_u16(conn_idx, cts->ct_ccc_h, &ccc);

        if (!(ccc & GATT_CCC_NOTIFICATIONS)) {
                return;
        }

        pack_time(time, pdu);

        ble_gatts_send_event(conn_idx, cts->ct_val_h, GATT_EVENT_NOTIFICATION, sizeof(pdu), pdu);
}

void cts_notify_time_all(ble_service_t *svc, const cts_current_time_t *time)
{
        uint8_t num_conn;
        uint16_t *conn_idx;

        ble_gap_get_connected(&num_conn, &conn_idx);

        while (num_conn--) {
                cts_notify_time(svc, conn_idx[num_conn], time);
        }

        if (conn_idx) {
                OS_FREE(conn_idx);
        }
}

void cts_set_local_time_info(ble_service_t *svc, const cts_local_time_info_t *local_time_info)
{
        ct_service_t *cts = (ct_service_t *) svc;

        ble_gatts_set_value(cts->lti_val_h, 2, local_time_info);
}

void cts_get_time_cfm(ble_service_t *svc, uint16_t conn_idx, att_error_t status,
                                                                const cts_current_time_t *time)
{
        ct_service_t *cts = (ct_service_t *) svc;
        uint8_t pdu[10];

        pack_time(time, pdu);

        ble_gatts_read_cfm(conn_idx, cts->ct_val_h, ATT_ERROR_OK, sizeof(pdu), &pdu);
}

void cts_set_time_cfm(ble_service_t *svc, uint16_t conn_idx, att_error_t status)
{
        ct_service_t *cts = (ct_service_t *) svc;

        ble_gatts_write_cfm(conn_idx, cts->ct_val_h, status);
}

void cts_set_local_time_info_cfm(ble_service_t *svc, uint16_t conn_idx, att_error_t status)
{
        ct_service_t *cts = (ct_service_t *) svc;

        ble_gatts_write_cfm(conn_idx, cts->lti_val_h, status);
}

void cts_get_ref_time_info_cfm(ble_service_t *svc, uint16_t conn_idx, att_error_t status,
                                                                const cts_ref_time_info_t *info)
{
        ct_service_t *cts = (ct_service_t *) svc;
        uint8_t pdu[4];

        pdu[0] = info->source;
        pdu[1] = info->accuracy;
        pdu[2] = info->days_since_update;
        pdu[3] = info->hours_since_update;

        //
        ble_gatts_read_cfm(conn_idx, cts->rti_val_h, ATT_ERROR_OK, sizeof(pdu), pdu);
}
