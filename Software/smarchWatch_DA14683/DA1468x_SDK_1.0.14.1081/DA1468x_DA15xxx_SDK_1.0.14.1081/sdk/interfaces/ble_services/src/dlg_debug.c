/**
 ****************************************************************************************
 *
 * @file dlg_debug.c
 *
 * @brief Debug service implementation
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "osal.h"
#include "util/list.h"
#include "ble_att.h"
#include "ble_bufops.h"
#include "ble_gattc.h"
#include "ble_gatts.h"
#include "ble_storage.h"
#include "ble_uuid.h"
#include "ble_service.h"
#include "dlg_debug.h"

#ifndef CONFIG_BLE_DLGDEBUG_MAX_CP_LEN
#define CONFIG_BLE_DLGDEBUG_MAX_CP_LEN (64)
#endif

#define UUID_DLGDEBUG        "6b559111-c4df-4660-818e-234f9e17b290"
#define UUID_DLGDEBUG_CP     "6b559111-c4df-4660-818e-234f9e17b291"

struct handler {
        void *next;

        dlgdebug_call_cb_t cb;  // callback
        void *ud;               // user-data

        /*
         * Instead of wasting space to malloc 'cat' and 'cmd' separately and store pointers here,
         * we combine them into single 'id' which is placed at the end of struct and has following
         * format: <cat>\0<cmd>\0
         */
        char id[];              // id (cat + cmd)
};

typedef struct {
        ble_service_t svc;

        // handles
        uint16_t cp_val_h;
        uint16_t cp_ccc_h;

        void *handlers;
} dlgdebug_service_t;

static void handle_prepare_write_req(ble_service_t *svc,
                                                const ble_evt_gatts_prepare_write_req_t *evt)
{
        dlgdebug_service_t *dbgs = (dlgdebug_service_t *) svc;

        if (evt->handle == dbgs->cp_val_h) {
                ble_gatts_prepare_write_cfm(evt->conn_idx, evt->handle,
                                                CONFIG_BLE_DLGDEBUG_MAX_CP_LEN, ATT_ERROR_OK);
        } else {
                ble_gatts_prepare_write_cfm(evt->conn_idx, evt->handle, 0,
                                                                ATT_ERROR_REQUEST_NOT_SUPPORTED);
        }
}

static void handle_read_req(ble_service_t *svc, const ble_evt_gatts_read_req_t *evt)
{
        dlgdebug_service_t *dbgs = (dlgdebug_service_t *) svc;

        if (evt->handle == dbgs->cp_ccc_h) {
                uint16_t ccc_val = 0;

                ble_storage_get_u16(evt->conn_idx, dbgs->cp_ccc_h, &ccc_val);

                // we're little-endian, ok to use value as-is
                ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_OK,
                                                                        sizeof(uint16_t), &ccc_val);
        } else {
                ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_READ_NOT_PERMITTED, 0, NULL);
        }
}

static bool match_handler_cb(const void *elem, const void *ud)
{
        const struct handler *hdlr = elem;
        const char **argv = (const char **) ud;
        const char *cmd;

        if (strcmp(hdlr->id, argv[0])) {
                return false;
        }

        cmd = hdlr->id + strlen(hdlr->id) + 1;

        return !strcmp(cmd, argv[1]);
}

static att_error_t handle_cp_val_write(dlgdebug_service_t *dbgs, const ble_evt_gatts_write_req_t *evt)
{
        char *str = NULL;
        char *p;
        int argc = 0;
        char *argv[8];
        int i;
        struct handler *hdlr;

        if (evt->offset > 0) {
                return ATT_ERROR_INVALID_OFFSET;
        }

        str = OS_MALLOC_NORET(evt->length + 1);
        memcpy(str, evt->value, evt->length);
        str[evt->length] = '\0';

        // tokenize string into argv
        p = strtok(str, " ");
        while (p && argc < (sizeof(argv) / sizeof(argv[0]))) {
                argv[argc++] = p;
                p = strtok(NULL, " ");
        }

        // should be service designator and command
        if (argc < 2) {
                goto done;
        }

        // make all arguments lowercase (to simplify code)
        for (i = 0; i < argc; i++) {
                p = argv[i];
                while (*p) {
                        if (*p >= 'A' && *p <= 'Z') {
                                *p = *p - 'A' + 'a';
                        }
                        p++;
                }
        }

        // handle command
        hdlr = list_find(dbgs->handlers, match_handler_cb, argv);
        if (hdlr && hdlr->cb) {
                hdlr->cb(evt->conn_idx, argc - 2, &argv[2], hdlr->ud);
        }

done:
        OS_FREE_NORET(str);

        return ATT_ERROR_OK;
}

static void handle_write_req(ble_service_t *svc, const ble_evt_gatts_write_req_t *evt)
{
        dlgdebug_service_t *dbgs = (dlgdebug_service_t *) svc;
        att_error_t status = ATT_ERROR_WRITE_NOT_PERMITTED;

        if (evt->handle == dbgs->cp_ccc_h) {
                uint16_t ccc_val;

                if (evt->offset) {
                        status = ATT_ERROR_ATTRIBUTE_NOT_LONG;
                        goto done;
                }

                if (evt->length != sizeof(uint16_t)) {
                        status = ATT_ERROR_APPLICATION_ERROR;
                        goto done;
                }

                ccc_val = get_u16(evt->value);

                ble_storage_put_u32(evt->conn_idx, dbgs->cp_ccc_h, ccc_val, true);

                status = ATT_ERROR_OK;

                goto done;
        }

        if (evt->handle == dbgs->cp_val_h) {
                status = handle_cp_val_write(dbgs, evt);
        }

done:
        ble_gatts_write_cfm(evt->conn_idx, evt->handle, status);
}

static void cleanup(ble_service_t *svc)
{
        dlgdebug_service_t *dbgs = (dlgdebug_service_t *) svc;

        list_free(&dbgs->handlers, NULL, NULL);
        ble_storage_remove_all(dbgs->cp_ccc_h);

        OS_FREE(dbgs);
}

ble_service_t *dlgdebug_init(const ble_service_config_t *cfg)
{
        dlgdebug_service_t *dbgs;
        uint16_t num_attr;
        att_uuid_t uuid;
        uint16_t cp_cpf_h;
        uint8_t cp_cpf_val[7];
        uint8_t *p = cp_cpf_val;

        dbgs = OS_MALLOC(sizeof(*dbgs));
        memset(dbgs, 0, sizeof(*dbgs));

        dbgs->svc.read_req = handle_read_req;
        dbgs->svc.write_req = handle_write_req;
        dbgs->svc.prepare_write_req = handle_prepare_write_req;
        dbgs->svc.cleanup = cleanup;

        num_attr = ble_gatts_get_num_attr(0, 1, 3);

        ble_uuid_from_string(UUID_DLGDEBUG, &uuid);
        ble_gatts_add_service(&uuid, GATT_SERVICE_PRIMARY, num_attr);

        ble_uuid_from_string(UUID_DLGDEBUG_CP, &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_WRITE | GATT_PROP_NOTIFY |
                                        GATT_PROP_EXTENDED | GATT_PROP_EXTENDED_RELIABLE_WRITE,
                                        ATT_PERM_WRITE, CONFIG_BLE_DLGDEBUG_MAX_CP_LEN, 0, NULL,
                                        &dbgs->cp_val_h);

        ble_uuid_create16(UUID_GATT_CHAR_EXT_PROPERTIES, &uuid);
        ble_gatts_add_descriptor(&uuid, ATT_PERM_READ, 0, 0, NULL);

        ble_uuid_create16(UUID_GATT_CLIENT_CHAR_CONFIGURATION, &uuid);
        ble_gatts_add_descriptor(&uuid, ATT_PERM_RW, sizeof(uint16_t), 0, &dbgs->cp_ccc_h);

        ble_uuid_create16(UUID_GATT_CHAR_PRESENTATION_FORMAT, &uuid);
        ble_gatts_add_descriptor(&uuid, ATT_PERM_READ, sizeof(cp_cpf_val), 0, &cp_cpf_h);

        ble_gatts_register_service(&dbgs->svc.start_h, &dbgs->cp_val_h, &dbgs->cp_ccc_h,
                                                                                &cp_cpf_h, 0);

        dbgs->svc.end_h = dbgs->svc.start_h + num_attr;

        /* Content Presentation Format for CP */
        put_u8_inc(&p, 25);      // Format=UTF-8 string
        put_u8_inc(&p, 0);       // Exponent=n/a
        put_u16_inc(&p, 0x2700); // Unit=unitless
        put_u8_inc(&p, 1);       // namespace
        put_u16_inc(&p, 0);      // descriptor
        ble_gatts_set_value(cp_cpf_h, sizeof(cp_cpf_val), cp_cpf_val);

        ble_service_add(&dbgs->svc);

        return &dbgs->svc;
}

void dlgdebug_register_handler(ble_service_t *svc, const char *cat, const char *cmd,
                                                                dlgdebug_call_cb_t cb, void *ud)
{
        dlgdebug_service_t *dbgs = (dlgdebug_service_t *) svc;
        size_t cat_len = strlen(cat) + 1;
        size_t cmd_len = strlen(cmd) + 1;
        size_t id_len = cat_len + cmd_len;
        struct handler *hdl;

        if (!dbgs) {
                return;
        }

        hdl = OS_MALLOC(sizeof(*hdl) + id_len);
        strcpy(hdl->id, cat);
        strcpy(hdl->id + cat_len, cmd);
        hdl->cb = cb;
        hdl->ud = ud;

        list_add(&dbgs->handlers, hdl);
}

void dlgdebug_register_handlers(ble_service_t *svc, size_t len, const dlgdebug_handler_t *handlers)
{
        int i;

        if (!handlers || !svc || len == 0) {
                return;
        }

        for (i = 0; i < len; i++) {
                dlgdebug_register_handler(svc, handlers[i].cat, handlers[i].cmd, handlers[i].cb,
                                                                                handlers[i].ud);
        }
}

void dlgdebug_notify_str(ble_service_t *svc, uint16_t conn_idx, const char *fmt, ...)
{
        dlgdebug_service_t *dbgs = (dlgdebug_service_t *) svc;
        uint16_t mtu;
        char *pdu;
        va_list ap;

        if (!dbgs) {
                return;
        }

        if (ble_gattc_get_mtu(conn_idx, &mtu) != BLE_STATUS_OK) {
                return;
        }

        pdu = OS_MALLOC_NORET(mtu - 3);

        va_start(ap, fmt);
        vsnprintf(pdu, mtu - 3, fmt, ap);
        va_end(ap);

        ble_gatts_send_event(conn_idx, dbgs->cp_val_h, GATT_EVENT_NOTIFICATION, strlen(pdu), pdu);

        OS_FREE_NORET(pdu);
}
