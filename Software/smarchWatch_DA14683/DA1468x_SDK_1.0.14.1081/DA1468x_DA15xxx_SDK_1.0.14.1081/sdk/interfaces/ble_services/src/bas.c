/**
 ****************************************************************************************
 *
 * @file bas.c
 *
 * @brief Battery Service sample implementation
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
#include "bas.h"

#define UUID_BATTERY_LEVEL      (0x2A19)

typedef struct {
        ble_service_t svc;

        // handles
        uint16_t bl_val_h;
        uint16_t bl_ccc_h;
} bat_service_t;

static att_error_t do_bl_ccc_write(bat_service_t *bas, uint16_t conn_idx, uint16_t offset,
                                                                uint16_t length, const uint8_t *value)
{
        uint16_t ccc;

        if (offset) {
                return ATT_ERROR_ATTRIBUTE_NOT_LONG;
        }

        if (length != sizeof(ccc)) {
                return ATT_ERROR_APPLICATION_ERROR;
        }

        ccc = get_u16(value);

        ble_storage_put_u32(conn_idx, bas->bl_ccc_h, ccc, true);

        return ATT_ERROR_OK;
}

static void notify_level(ble_service_t *svc, uint16_t conn_idx, uint8_t level);

static void handle_connected_evt(ble_service_t *svc, const ble_evt_gap_connected_t *evt)
{
        bat_service_t *bas = (bat_service_t *) svc;
        uint8_t level = 0x00;
        uint16_t level_len = sizeof(level);
        uint8_t prev_level;
        ble_error_t err;

        ble_gatts_get_value(bas->bl_val_h, &level_len, &level);

        prev_level = level;
        err = ble_storage_get_u8(evt->conn_idx, bas->bl_val_h, &prev_level);

        if (BLE_STATUS_OK == err && prev_level != level) {
                notify_level(svc, evt->conn_idx, level);
        }

        ble_storage_put_u32(evt->conn_idx, bas->bl_val_h, level, true);
}

static void handle_read_req(ble_service_t *svc, const ble_evt_gatts_read_req_t *evt)
{
        bat_service_t *bas = (bat_service_t *) svc;

        if (evt->handle == bas->bl_ccc_h) {
                uint16_t ccc = 0x0000;

                ble_storage_get_u16(evt->conn_idx, bas->bl_ccc_h, &ccc);

                // we're little-endian, ok to write directly from uint16_t
                ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_OK, sizeof(ccc), &ccc);
        } else {
                ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_READ_NOT_PERMITTED, 0, NULL);
        }
}

static void handle_write_req(ble_service_t *svc, const ble_evt_gatts_write_req_t *evt)
{
        bat_service_t *bas = (bat_service_t *) svc;
        att_error_t status = ATT_ERROR_ATTRIBUTE_NOT_FOUND;

        if (evt->handle == bas->bl_ccc_h) {
                status = do_bl_ccc_write(bas, evt->conn_idx, evt->offset, evt->length, evt->value);
                goto done;
        }

done:
        ble_gatts_write_cfm(evt->conn_idx, evt->handle, status);
}

static void cleanup(ble_service_t *svc)
{
        bat_service_t *bas = (bat_service_t *) svc;

        ble_storage_remove_all(bas->bl_ccc_h);
        ble_storage_remove_all(bas->bl_val_h);

        OS_FREE(bas);
}

ble_service_t *bas_init(const ble_service_config_t *config, const bas_battery_info_t *info)
{
        uint16_t num_descr;
        uint16_t num_attr;
        uint16_t cpf_h = 0;
        bat_service_t *bas;
        att_uuid_t uuid;
        uint8_t level = 0;

        bas = OS_MALLOC(sizeof(*bas));
        memset(bas, 0, sizeof(*bas));

        bas->svc.connected_evt = handle_connected_evt;
        bas->svc.read_req = handle_read_req;
        bas->svc.write_req = handle_write_req;
        bas->svc.cleanup = cleanup;

        // Content Presentation Format descriptor present if 'info' is set
        num_descr = (info ? 2 : 1);
        num_attr = ble_service_get_num_attr(config, 1, num_descr);

        ble_uuid_create16(UUID_SERVICE_BAS, &uuid);
        ble_gatts_add_service(&uuid, GATT_SERVICE_PRIMARY, num_attr);

        ble_service_config_add_includes(config);

        ble_uuid_create16(UUID_BATTERY_LEVEL, &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_READ | GATT_PROP_NOTIFY,
                                        ble_service_config_elevate_perm(ATT_PERM_READ, config),
                                        1, 0, NULL, &bas->bl_val_h);

        ble_uuid_create16(UUID_GATT_CLIENT_CHAR_CONFIGURATION, &uuid);
        ble_gatts_add_descriptor(&uuid, ATT_PERM_RW, 1, 0, &bas->bl_ccc_h);

        if (info) {
                ble_uuid_create16(UUID_GATT_CHAR_PRESENTATION_FORMAT, &uuid);
                ble_gatts_add_descriptor(&uuid,
                                        ble_service_config_elevate_perm(ATT_PERM_READ, config),
                                        7, 0, &cpf_h);
        }

        ble_gatts_register_service(&bas->svc.start_h, &bas->bl_val_h, &bas->bl_ccc_h, &cpf_h, 0);

        /* Set initial value for battery level so we always have proper characteristic value set. */
        ble_gatts_set_value(bas->bl_val_h, sizeof(level), &level);

        if (info) {
                uint8_t cpf_val[7];
                uint8_t *p = cpf_val;

                put_u8_inc(&p, 0x04);     // Format=unsigned 8-bit integer
                put_u8_inc(&p, 0x00);     // Exponent=0
                put_u16_inc(&p, 0x27AD);  // Unit=percentage
                put_u8_inc(&p, info->namespace);
                put_u16_inc(&p, info->descriptor);

                // Content Presentation Format descriptor has static value
                ble_gatts_set_value(cpf_h, sizeof(cpf_val), cpf_val);
        }

        bas->svc.end_h = bas->svc.start_h + num_attr;

        ble_service_add(&bas->svc);

        return &bas->svc;
}

static void notify_level(ble_service_t *svc, uint16_t conn_idx, uint8_t level)
{
        bat_service_t *bas = (bat_service_t *) svc;
        uint16_t ccc = 0x0000;

        ble_storage_get_u16(conn_idx, bas->bl_ccc_h, &ccc);

        if (!(ccc & GATT_CCC_NOTIFICATIONS)) {
                return;
        }

        ble_gatts_send_event(conn_idx, bas->bl_val_h, GATT_EVENT_NOTIFICATION, sizeof(level), &level);
}

void bas_notify_level(ble_service_t *svc, uint16_t conn_idx)
{
        bat_service_t *bas = (bat_service_t *) svc;
        uint8_t level = 0x00;
        uint16_t level_len = sizeof(level);

        ble_gatts_get_value(bas->bl_val_h, &level_len, &level);

        notify_level(svc, conn_idx, level);

        ble_storage_put_u32(conn_idx, bas->bl_val_h, level, true);
}

void bas_set_level(ble_service_t *svc, uint8_t level, bool notify)
{
        bat_service_t *bas = (bat_service_t *) svc;
        uint8_t prev_level = 0x00;
        uint16_t prev_level_len = sizeof(prev_level);
        uint8_t num_conn;
        uint16_t *conn_idx;

        if (level > 100) {
                return;
        }

        ble_gatts_get_value(bas->bl_val_h, &prev_level_len, &prev_level);

        if (level == prev_level) {
                return;
        }

        ble_gatts_set_value(bas->bl_val_h, sizeof(level), &level);

        /*
         * for each connected device we need to:
         * - notify new value, if requested by caller
         * - put new value to storage to use when device is reconnected
         */

        ble_gap_get_connected(&num_conn, &conn_idx);

        while (num_conn--) {
                if (notify) {
                        notify_level(svc, conn_idx[num_conn], level);
                }

                ble_storage_put_u32(conn_idx[num_conn], bas->bl_val_h, level, true);
        }

        if (conn_idx) {
                OS_FREE(conn_idx);
        }
}
