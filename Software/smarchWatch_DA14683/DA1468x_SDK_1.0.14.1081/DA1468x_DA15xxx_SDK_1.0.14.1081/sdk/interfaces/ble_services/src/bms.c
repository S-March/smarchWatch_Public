/**
 ****************************************************************************************
 *
 * @file bms.c
 *
 * @brief Bond Management Service implementation
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include "osal.h"
#include <string.h>
#include "ble_att.h"
#include "ble_common.h"
#include "ble_gatts.h"
#include "ble_uuid.h"
#include "bms.h"

enum bms_error {
        BMS_ERROR_OPCODE_NOT_SUPPORTED = 0x80,
        BMS_ERROR_OPERATION_FAILED     = 0x81,
};

enum bmcp_opcode {
        BMCP_OPCODE_DELETE_BOND_REQ_DEV            = 0x03,
        BMCP_OPCODE_DELETE_BOND_ALL_DEV            = 0x06,
        BMCP_OPCODE_DELETE_BOND_ALL_EXCEPT_REQ_DEV = 0x09,
};

#define UUID_BOND_MANAGEMENT_CONTROL_POINT  (0x2AA4)
#define UUID_BOND_MANAGEMENT_FEATURE        (0x2AA5)
#define BMCP_SIZE                           (512)
#define BMF_SIZE                            (3)

typedef struct {
        ble_service_t svc;

        const bms_callbacks_t *cb;
        bms_delete_bond_op_t supported_delete_bond_op;

        uint16_t bmcp_val_h;
        uint16_t bmf_val_h;
} bm_service_t;

static void handle_bmcp_write(bm_service_t *bms, uint16_t conn_idx, uint16_t length,
                                                                        const uint8_t *data)
{
        att_error_t att_error = BMS_ERROR_OPERATION_FAILED;
        bms_delete_bond_op_t op;

        if (length < 1) {
                att_error = ATT_ERROR_INVALID_VALUE_LENGTH;
                goto failed;
        }

        switch (data[0]) {
        case BMCP_OPCODE_DELETE_BOND_REQ_DEV:
                op = length > 1 ? BMS_DELETE_BOND_REQ_DEV_AUTH : BMS_DELETE_BOND_REQ_DEV;
                break;
        case BMCP_OPCODE_DELETE_BOND_ALL_DEV:
                op = length > 1 ? BMS_DELETE_BOND_ALL_DEV_AUTH : BMS_DELETE_BOND_ALL_DEV;
                break;
        case BMCP_OPCODE_DELETE_BOND_ALL_EXCEPT_REQ_DEV:
                op = length > 1 ? BMS_DELETE_BOND_ALL_EXCEPT_REQ_DEV_AUTH :
                                                        BMS_DELETE_BOND_ALL_EXCEPT_REQ_DEV;
                break;
        default:
                att_error = BMS_ERROR_OPCODE_NOT_SUPPORTED;
                goto failed;
        }

        if (!(op & bms->supported_delete_bond_op)) {
                att_error = BMS_ERROR_OPCODE_NOT_SUPPORTED;
                goto failed;
        }

        if (bms->cb && bms->cb->delete_bond) {
                const uint8_t *auth_code = data + 1;
                length -= 1;

                bms->cb->delete_bond(op, conn_idx, length, auth_code);
                return;
        }

failed:
        ble_gatts_write_cfm(conn_idx, bms->bmcp_val_h, att_error);
}

static void handle_write_cb(ble_service_t *svc, const ble_evt_gatts_write_req_t *evt)
{
        bm_service_t *bms = (bm_service_t *) svc;

        if (evt->handle == bms->bmcp_val_h) {
                handle_bmcp_write(bms, evt->conn_idx, evt->length, evt->value);
        } else {
                ble_gatts_write_cfm(evt->conn_idx, evt->handle, ATT_ERROR_ATTRIBUTE_NOT_FOUND);
        }
}

static void handle_prepare_write_req(ble_service_t *svc,
                                                const ble_evt_gatts_prepare_write_req_t *evt)
{
        bm_service_t *bms = (bm_service_t *) svc;

        if (evt->handle == bms->bmcp_val_h) {
                ble_gatts_prepare_write_cfm(evt->conn_idx, evt->handle, BMCP_SIZE, ATT_ERROR_OK);
        } else {
                ble_gatts_prepare_write_cfm(evt->conn_idx, evt->handle, 0,
                                                                ATT_ERROR_REQUEST_NOT_SUPPORTED);
        }
}

static void set_bmf_value(bm_service_t *bms)
{
        bms_delete_bond_op_t features = bms->supported_delete_bond_op;
        uint8_t value[BMF_SIZE];

        memset(value, 0, sizeof(value));

        /* Delete bond of requesting device (LE transport only) */
        if (features & BMS_DELETE_BOND_REQ_DEV) {
                value[0] |= (1 << 4);
        }

        /* Delete bond of requesting device (LE transport only) with authorization code*/
        if (features & BMS_DELETE_BOND_REQ_DEV_AUTH) {
                value[0] |= (1 << 5);
        }

        /* Delete all bonds on server (LE transport only) */
        if (features & BMS_DELETE_BOND_ALL_DEV) {
                value[1] |= (1 << 2);
        }

        /* Delete all bonds on server (LE transport only) with authorization code */
        if (features & BMS_DELETE_BOND_ALL_DEV_AUTH) {
                value[1] |= (1 << 3);
        }

        /* Delete bond of all except the requesting device on the server (LE transport only) */
        if (features & BMS_DELETE_BOND_ALL_EXCEPT_REQ_DEV) {
                value[2] |= (1 << 0);
        }

        /*
         * Delete bond of all except the requesting device on the server (LE transport only)
         * with authorization code
         */
        if (features & BMS_DELETE_BOND_ALL_EXCEPT_REQ_DEV_AUTH) {
                value[2] |= (1 << 1);
        }

        ble_gatts_set_value(bms->bmf_val_h, sizeof(value), value);
}

static void cleanup(ble_service_t *svc)
{
        OS_FREE(svc);
}

ble_service_t *bms_init(const ble_service_config_t *config, const bms_config_t *bms_config,
                                                                const bms_callbacks_t *callbacks)
{
        bm_service_t *bms;
        uint16_t num_attr;
        att_uuid_t uuid;
        att_perm_t perm;

        bms = OS_MALLOC(sizeof(*bms));
        memset(bms, 0, sizeof(*bms));

        /* Set mandatory feature */
        bms->supported_delete_bond_op = BMS_DELETE_BOND_REQ_DEV;
        bms->supported_delete_bond_op |= bms_config->supported_delete_bond_op;

        num_attr = ble_service_get_num_attr(config, 2, 1);

        /* Bond Management Service */
        ble_uuid_create16(UUID_SERVICE_BMS, &uuid);
        ble_gatts_add_service(&uuid, GATT_SERVICE_PRIMARY, num_attr);

        /* Include services */
        ble_service_config_add_includes(config);

        /* Bond Management Control Point */
        perm = (!config || config->sec_level <= GAP_SEC_LEVEL_2) ? ATT_PERM_WRITE_ENCRYPT : ATT_PERM_WRITE_AUTH;
        ble_uuid_create16(UUID_BOND_MANAGEMENT_CONTROL_POINT, &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_WRITE | GATT_PROP_EXTENDED |
                                                        GATT_PROP_EXTENDED_RELIABLE_WRITE, perm,
                                                        BMCP_SIZE, 0, NULL, &bms->bmcp_val_h);

        /* Extended Properties Descriptor */
        ble_uuid_create16(UUID_GATT_CHAR_EXT_PROPERTIES, &uuid);
        ble_gatts_add_descriptor(&uuid, ATT_PERM_READ, 0, 0, NULL);

        /* Bond Management Feature */
        perm = (!config || config->sec_level <= GAP_SEC_LEVEL_2) ? ATT_PERM_READ_ENCRYPT : ATT_PERM_READ_AUTH;
        ble_uuid_create16(UUID_BOND_MANAGEMENT_FEATURE, &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_READ, perm, BMF_SIZE, 0, NULL, &bms->bmf_val_h);

        ble_gatts_register_service(&bms->svc.start_h, &bms->bmcp_val_h, &bms->bmf_val_h, 0);
        bms->svc.end_h = num_attr + bms->svc.start_h;
        set_bmf_value(bms);

        bms->svc.write_req = handle_write_cb;
        bms->svc.prepare_write_req = handle_prepare_write_req;
        bms->svc.cleanup = cleanup;
        bms->cb = callbacks;

        ble_service_add(&bms->svc);

        return &bms->svc;
}

void bms_delete_bond_cfm(ble_service_t *svc, uint16_t conn_idx, bms_delete_bond_status_t status)
{
        bm_service_t *bms = (bm_service_t *) svc;
        att_error_t att_error = ATT_ERROR_OK;

        switch (status) {
        case BMS_DELETE_BOND_STATUS_OK:
                att_error = ATT_ERROR_OK;
                break;
        case BMS_DELETE_BOND_STATUS_FAILED:
                att_error = BMS_ERROR_OPERATION_FAILED;
                break;
        case BMS_DELETE_BOND_STATUS_INSUFFICIENT_AUTH:
                att_error = ATT_ERROR_INSUFFICIENT_AUTHORIZATION;
                break;
        case BMS_DELETE_BOND_STATUS_NOT_SUPPORTED:
        default:
                att_error = BMS_ERROR_OPCODE_NOT_SUPPORTED;
                break;
        }

        ble_gatts_write_cfm(conn_idx, bms->bmcp_val_h, att_error);
}
