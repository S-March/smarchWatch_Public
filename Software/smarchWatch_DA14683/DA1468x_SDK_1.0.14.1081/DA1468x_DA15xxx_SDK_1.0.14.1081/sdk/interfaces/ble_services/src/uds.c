/**
 ****************************************************************************************
 *
 * @file uds.c
 *
 * @brief User Data Service sample implementation
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
#include "uds.h"

#define UUID_DATABASE_CHANGE_INCREMENT  (0x2A99)
#define UUID_USER_INDEX                 (0x2A9A)
#define UUID_USER_CONTROL_POINT         (0x2A9F)

// note: this should have the same order as uds_db_field_t
enum uds_db_field_idx {
        UDS_IDX_FIRST_NAME                      = 0,
        UDS_IDX_LAST_NAME                       = 1,
        UDS_IDX_EMAIL_ADDRESS                   = 2,
        UDS_IDX_AGE                             = 3,
        UDS_IDX_DATE_OF_BIRTH                   = 4,
        UDS_IDX_GENDER                          = 5,
        UDS_IDX_WEIGHT                          = 6,
        UDS_IDX_HEIGHT                          = 7,
        UDS_IDX_VO2_MAX                         = 8,
        UDS_IDX_HEART_RATE_MAX                  = 9,
        UDS_IDX_RESTING_HEART_RATE              = 10,
        UDS_IDX_MAX_RECOMMENDED_HEART_RATE      = 11,
        UDS_IDX_AEROBIC_THRESHOLD               = 12,
        UDS_IDX_ANAEROBIC_THRESHOLD             = 13,
        UDS_IDX_SPORT_TYPE                      = 14,
        UDS_IDX_DATE_OF_THRESHOLD_ASSESSMENT    = 15,
        UDS_IDX_WAIST_CIRCUMFERENCE             = 16,
        UDS_IDX_HIP_CIRCUMFERENCE               = 17,
        UDS_IDX_FAT_BURN_HEART_RATE_LOW_LIMIT   = 18,
        UDS_IDX_FAT_BURN_HEART_RATE_UP_LIMIT    = 19,
        UDS_IDX_AEROBIC_HEART_RATE_LOW_LIMIT    = 20,
        UDS_IDX_AEROBIC_HEART_RATE_UP_LIMIT     = 21,
        UDS_IDX_ANEROBIC_HEART_RATE_LOW_LIMIT   = 22,
        UDS_IDX_ANEROBIC_HEART_RATE_UP_LIMIT    = 23,
        UDS_IDX_FIVE_ZONE_HEART_RATE_LIMITS     = 24,
        UDS_IDX_THREE_ZONE_HEART_RATE_LIMITS    = 25,
        UDS_IDX_TWO_ZONE_HEART_RATE_LIMIT       = 26,
        UDS_IDX_LANGUAGE                        = 27,
        UDS_IDX_LAST,
};

enum uds_opcode {
        UDS_OPCODE_REGISTER_NEW_USER    = 0x01,
        UDS_OPCODE_CONSENT              = 0x02,
        UDS_OPCODE_DELETE_USER_DATA     = 0x03,
        UDS_OPCODE_RESPONSE_CODE        = 0x20,
};

typedef struct {
        ble_service_t svc;

        //callbacks
        const uds_callbacks_t *cb;

        // handles
        uint16_t uid_val_h;
        uint16_t dbi_val_h;
        uint16_t dbi_ccc_h;
        uint16_t ucp_val_h;
        uint16_t ucp_ccc_h;
        uint16_t db_val_h[UDS_IDX_LAST];
} ud_service_t;

static void handle_read_req(ble_service_t *svc, const ble_evt_gatts_read_req_t *evt);
static void handle_write_req(ble_service_t *svc, const ble_evt_gatts_write_req_t *evt);
static void handle_event_sent(ble_service_t *svc, const ble_evt_gatts_event_sent_t *evt);

struct db_field {
        uint16_t uuid16;
        uint16_t max_length;
};

static const struct db_field fields[UDS_IDX_LAST] = {
        { 0x2A8A,       50, },
        { 0x2A90,       50, },
        { 0x2A87,       50, },
        { 0x2A80,       1,  },
        { 0x2A85,       4,  },
        { 0x2A8C,       1,  },
        { 0x2A98,       2,  },
        { 0x2A8E,       2,  },
        { 0x2A96,       1,  },
        { 0x2A8D,       1,  },
        { 0x2A92,       1,  },
        { 0x2A91,       1,  },
        { 0x2A7F,       1,  },
        { 0x2A83,       1,  },
        { 0x2A93,       1,  },
        { 0x2A86,       4,  },
        { 0x2A97,       2,  },
        { 0x2A8F,       2,  },
        { 0x2A88,       1,  },
        { 0x2A89,       1,  },
        { 0x2A7E,       1,  },
        { 0x2A84,       1,  },
        { 0x2A81,       1,  },
        { 0x2A82,       1,  },
        { 0x2A8B,       4,  },
        { 0x2A94,       2,  },
        { 0x2A95,       1,  },
        { 0x2AA2,       2,  },
};

static int get_field_idx(uint32_t field)
{
        int idx;

        for (idx = 0; idx < UDS_IDX_LAST; idx++) {
                if (field & (1 << idx)) {
                        return idx;
                }
        }

        return -1;
}

void uds_cp_register_new_user_cfm(ble_service_t *svc, uint16_t conn_idx,
                                                        uds_cp_response_t status, uint8_t user_id)
{
        uint8_t data[4];
        uint8_t length = 3;
        ud_service_t *uds = (ud_service_t *) svc;
        uint16_t ucp_val = 0;

        ble_storage_get_u16(conn_idx, uds->ucp_val_h, &ucp_val);

        if (ucp_val != UDS_OPCODE_REGISTER_NEW_USER) {
                return;
        }

        data[0] = UDS_OPCODE_RESPONSE_CODE;
        data[1] = UDS_OPCODE_REGISTER_NEW_USER;
        data[2] = (uint8_t) status;
        if (status == UDS_CP_RESPONSE_SUCCESS) {
                data[3] = user_id;
                length = 4;
        }

        ble_gatts_send_event(conn_idx, uds->ucp_val_h, GATT_EVENT_INDICATION, length, data);
}

void uds_cp_consent_cfm(ble_service_t *svc, uint16_t conn_idx, uds_cp_response_t status)
{
        uint8_t data[3];
        ud_service_t *uds = (ud_service_t *) svc;
        uint16_t ucp_val = 0;

        ble_storage_get_u16(conn_idx, uds->ucp_val_h, &ucp_val);

        if (ucp_val != UDS_OPCODE_CONSENT) {
                return;
        }

        data[0] = UDS_OPCODE_RESPONSE_CODE;
        data[1] = UDS_OPCODE_CONSENT;
        data[2] = (uint8_t) status;

        ble_gatts_send_event(conn_idx, uds->ucp_val_h, GATT_EVENT_INDICATION, 3, data);
}

void uds_cp_delete_user_cfm(ble_service_t *svc, uint16_t conn_idx, uds_cp_response_t status)
{
        uint8_t data[3];
        ud_service_t *uds = (ud_service_t *) svc;
        uint16_t ucp_val = 0;

        ble_storage_get_u16(conn_idx, uds->ucp_val_h, &ucp_val);

        if (ucp_val != UDS_OPCODE_DELETE_USER_DATA) {
                return;
        }

        data[0] = UDS_OPCODE_RESPONSE_CODE;
        data[1] = UDS_OPCODE_DELETE_USER_DATA;
        data[2] = (uint8_t) status;

        ble_gatts_send_event(conn_idx, uds->ucp_val_h, GATT_EVENT_INDICATION, 3, data);
}

void uds_db_read_cfm(ble_service_t *svc, uint16_t conn_idx, uint32_t field, att_error_t status,
                                                        uint16_t length, const void *value)
{
        ud_service_t *uds = (ud_service_t *) svc;
        int idx;

        idx = get_field_idx(field);

        if (idx < 0) {
                return;
        }

        ble_gatts_read_cfm(conn_idx, uds->db_val_h[idx], status, length, value);
}

void uds_set_user_id(ble_service_t *svc, uint16_t conn_idx, uint8_t user_id)
{
        ud_service_t *uds = (ud_service_t *) svc;

        ble_storage_put_u32(conn_idx, uds->uid_val_h, user_id, false);
}

uint8_t uds_get_user_id(ble_service_t *svc, uint16_t conn_idx)
{
        ud_service_t *uds = (ud_service_t *) svc;
        uint8_t user_id = UDS_USER_ID_UNKNOWN;

        ble_storage_get_u8(conn_idx, uds->uid_val_h, &user_id);

        return user_id;
}

void uds_db_write_cfm(ble_service_t *svc, uint16_t conn_idx, uint32_t field, att_error_t status)
{
        ud_service_t *uds = (ud_service_t *) svc;
        int idx;

        idx = get_field_idx(field);

        if (idx < 0) {
                return;
        }

        ble_gatts_write_cfm(conn_idx, uds->db_val_h[idx], status);
}

static void database_increment(ble_service_t *svc, uint16_t conn_idx, uint8_t cur_user_id,
                                                                                uint32_t increment)
{
        uint8_t user_id = UDS_USER_ID_UNKNOWN;
        ud_service_t *uds = (ud_service_t *) svc;
        uint16_t ccc = 0x0000;

        /*
         * Get user id which is written in User Index characteristic
         */
        user_id = uds_get_user_id(svc, conn_idx);

        ble_storage_get_u16(conn_idx, uds->dbi_ccc_h, &ccc);

        if (!(ccc & GATT_CCC_NOTIFICATIONS) || user_id != cur_user_id) {
                return ;
        }

        ble_gatts_send_event(conn_idx, uds->dbi_val_h, GATT_EVENT_NOTIFICATION,
                                                                sizeof(increment), &increment);
}

static void notify_db_increment(ble_service_t *svc, uint16_t src_conn_idx, uint8_t user_id,
                                                                                uint32_t increment)
{
        uint8_t num_conn;
        uint16_t *conn_idx;

        ble_gap_get_connected(&num_conn, &conn_idx);

        while (num_conn--) {
                if (src_conn_idx != conn_idx[num_conn]) {
                        database_increment(svc, conn_idx[num_conn], user_id, increment);
                }
        }

        if (conn_idx) {
                OS_FREE(conn_idx);
        }
}

static void handle_read_req(ble_service_t *svc, const ble_evt_gatts_read_req_t *evt)
{
        int i;
        ud_service_t *uds = (ud_service_t *) svc;

        for (i = 0; i < UDS_IDX_LAST; i++) {
                if (evt->handle == uds->db_val_h[i]) {
                        if (uds->cb && uds->cb->db_read) {
                                uds->cb->db_read(svc, evt->conn_idx, 1 << i);
                        } else {
                                ble_gatts_read_cfm(evt->conn_idx, evt->handle,
                                                        ATT_ERROR_READ_NOT_PERMITTED, 0, NULL);
                        }
                        return;
                }
        }

        if (evt->handle == uds->dbi_val_h) {
                uint32_t dbi_val = 0;

                ble_storage_get_u32(evt->conn_idx, uds->dbi_val_h, &dbi_val);
                ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_OK, sizeof(uint32_t),
                                                                                        &dbi_val);
        } else if (evt->handle == uds->dbi_ccc_h) {
                uint16_t ccc_val = 0;

                ble_storage_get_u16(evt->conn_idx, uds->dbi_ccc_h, &ccc_val);

                // we're little-endian, ok to use value as-is
                ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_OK,
                                                                        sizeof(uint16_t), &ccc_val);
        } else if (evt->handle == uds->uid_val_h) {
                uint8_t uid_val = UDS_USER_ID_UNKNOWN;

                ble_storage_get_u8(evt->conn_idx, uds->uid_val_h, &uid_val);

                ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_OK,
                                                                        sizeof(uint8_t), &uid_val);
        } else if (evt->handle == uds->ucp_ccc_h) {
                uint16_t ccc_val = 0;

                ble_storage_get_u16(evt->conn_idx, uds->ucp_ccc_h, &ccc_val);

                // we're little-endian, ok to use value as-is
                ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_OK,
                                                                        sizeof(uint16_t), &ccc_val);
        } else {
                ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_READ_NOT_PERMITTED, 0, NULL);
        }
}

static att_error_t do_ucp_val_write(ud_service_t *uds, uint16_t conn_idx, uint16_t offset,
                                                        uint16_t length, const uint8_t *value)
{
        uint8_t user_id = 0;
        uint8_t data[3];
        uint16_t ucp_ccc = 0x0000;
        uint16_t ucp_val = 0;
        uint16_t opcode;
        uint16_t consent;

        if (offset) {
                return ATT_ERROR_ATTRIBUTE_NOT_LONG;
        }

        if (length > 19) {
                return ATT_ERROR_APPLICATION_ERROR;
        }

        ble_storage_get_u16(conn_idx, uds->ucp_ccc_h, &ucp_ccc);

        if (!(ucp_ccc & GATT_CCC_INDICATIONS)) {
                // indication disabled by client
                return ATT_ERROR_CCC_DESCRIPTOR_IMPROPERLY_CONFIGURED;
        }

        ble_storage_get_u16(conn_idx, uds->ucp_val_h, &ucp_val);

        if (ucp_val) {
                // current request cannot be serviced because the previous one is still in progress
                return ATT_ERROR_PROCEDURE_ALREADY_IN_PROGRESS;
        }

        opcode = get_u8_inc(&value);
        ble_storage_put_u32(conn_idx, uds->ucp_val_h, opcode, false);
        ble_gatts_write_cfm(conn_idx, uds->ucp_val_h, ATT_ERROR_OK);

        switch (opcode) {
        case UDS_OPCODE_REGISTER_NEW_USER:
                if (!uds->cb || !uds->cb->cp_register_new_user) {
                        goto failed;
                }

                if (length != 3) {
                        goto register_failed;
                }

                consent = get_u16(value);

                if (consent > 9999) {
                        goto register_failed;
                } else {
                        uds->cb->cp_register_new_user(&uds->svc, conn_idx, consent);
                }
                break;

        case UDS_OPCODE_CONSENT:
                if (!uds->cb || !uds->cb->cp_consent) {
                        goto failed;
                }
                user_id = get_u8_inc(&value);

                if (length != 4) {
                        goto consent_failed;
                }

                consent = get_u16(value);

                if (consent > 9999 || user_id == UDS_USER_ID_UNKNOWN) {
                        goto consent_failed;
                } else {
                        uds->cb->cp_consent(&uds->svc, conn_idx, user_id, consent);
                }

                break;

        case UDS_OPCODE_DELETE_USER_DATA:
                if (!uds->cb || !uds->cb->cp_delete_user_data) {
                        goto failed;
                }
                user_id = uds_get_user_id(&uds->svc, conn_idx);

                if (user_id == UDS_USER_ID_UNKNOWN) {
                        uds_cp_delete_user_cfm(&uds->svc, conn_idx, UDS_CP_RESPONSE_NOT_AUTHORIZED);
                } else {
                        uds->cb->cp_delete_user_data(&uds->svc, conn_idx);
                }
                break;

        default:
                ble_storage_remove(conn_idx, uds->ucp_val_h);
                goto failed;
        }

        return ATT_ERROR_OK;

failed:
        data[0] = UDS_OPCODE_RESPONSE_CODE;
        data[1] = opcode;
        data[2] = UDS_CP_RESPONSE_OPCODE_NOT_SUPPORTED;

        ble_gatts_send_event(conn_idx, uds->ucp_val_h, GATT_EVENT_INDICATION, sizeof(data), data);
        return ATT_ERROR_OK;

register_failed:
        uds_cp_register_new_user_cfm(&uds->svc, conn_idx, UDS_CP_RESPONSE_INVALID_PARAM, 0);
        return ATT_ERROR_OK;

consent_failed:
        uds_cp_consent_cfm(&uds->svc, conn_idx, UDS_CP_RESPONSE_INVALID_PARAM);
        return ATT_ERROR_OK;
}

static att_error_t do_ccc_write(uint16_t ccc_h, uint16_t conn_idx, uint16_t offset, uint16_t length,
                                                                        const uint8_t *value)
{
        uint16_t ccc_val;

        if (offset) {
                return ATT_ERROR_ATTRIBUTE_NOT_LONG;
        }

        if (length != sizeof(uint16_t)) {
                return ATT_ERROR_APPLICATION_ERROR;
        }

        ccc_val = get_u16(value);

        ble_storage_put_u32(conn_idx, ccc_h, ccc_val, true);

        return ATT_ERROR_OK;
}

static void handle_write_req(ble_service_t *svc, const ble_evt_gatts_write_req_t *evt)
{
        int i;
        ud_service_t *uds = (ud_service_t *) svc;
        att_error_t status = ATT_ERROR_ATTRIBUTE_NOT_FOUND;

        if (evt->handle == uds->dbi_val_h) {
                uint32_t dbi_val;
                uint8_t user_id;

                user_id = uds_get_user_id(svc, evt->conn_idx);
                status = ATT_ERROR_WRITE_NOT_PERMITTED;

                if (user_id == UDS_USER_ID_UNKNOWN) {
                        status = UDS_ERROR_ACCESS_NOT_PERMITTED;
                        goto done;
                }

                dbi_val = get_u32(evt->value);

                if (uds->cb && uds->cb->db_increment_changed) {
                        bool update_dbi;
                        update_dbi = uds->cb->db_increment_changed(svc, evt->conn_idx, dbi_val);

                        if (update_dbi) {
                                ble_storage_put_u32(evt->conn_idx, uds->dbi_val_h, dbi_val, true);
                                status = ATT_ERROR_OK;
                                notify_db_increment(svc, evt->conn_idx, user_id, dbi_val);
                        }
                }

                goto done;
        }

        if (evt->handle == uds->dbi_ccc_h) {
                status = do_ccc_write(uds->dbi_ccc_h, evt->conn_idx, evt->offset, evt->length,
                                                                                        evt->value);
                goto done;
        }

        if (evt->handle == uds->ucp_val_h) {
                status = do_ucp_val_write(uds, evt->conn_idx, evt->offset, evt->length, evt->value);
                if (status == ATT_ERROR_OK) {
                        return;
                }
                goto done;
        }

        if (evt->handle == uds->ucp_ccc_h) {
                status = do_ccc_write(uds->ucp_ccc_h, evt->conn_idx, evt->offset, evt->length,
                                                                                        evt->value);
                goto done;
        }

        for (i = 0; i < UDS_IDX_LAST; i++) {
                if (evt->handle == uds->db_val_h[i]) {
                        if (uds->cb && uds->cb->db_write) {
                                uds->cb->db_write(svc, evt->conn_idx, 1 << i, evt->offset,
                                                                        evt->length, evt->value);
                        } else {
                                ble_gatts_write_cfm(evt->conn_idx, evt->handle,
                                                                ATT_ERROR_WRITE_NOT_PERMITTED);
                        }
                        return;
                }
        }

done:
        ble_gatts_write_cfm(evt->conn_idx, evt->handle, status);
}

static void handle_event_sent(ble_service_t *svc, const ble_evt_gatts_event_sent_t *evt)
{
        ud_service_t *uds = (ud_service_t *) svc;

        if (evt->handle == uds->ucp_val_h) {
                ble_storage_remove(evt->conn_idx, uds->ucp_val_h);
        }
}

static void set_db_increment(ble_service_t *svc, uint16_t conn_idx, uint8_t cur_user_id,
                                                                                uint32_t increment)
{
        uint8_t user_id = UDS_USER_ID_UNKNOWN;
        ud_service_t *uds = (ud_service_t *) svc;

        /*
         * Get user id which is written in User Index characteristic
         */
        user_id = uds_get_user_id(svc, conn_idx);

        /*
         * If user id from User Index is not equal with current user id user should not be
         * "incremented"
         */
        if (user_id != cur_user_id) {
                return ;
        }

        ble_storage_put_u32(conn_idx, uds->dbi_val_h, increment, true);
}

void uds_set_db_increment(ble_service_t *svc, uint16_t src_conn_idx, uint32_t increment, bool notify)
{
        uint8_t num_conn;
        uint16_t *conn_idx;
        uint8_t user_id;

        user_id = uds_get_user_id(svc, src_conn_idx);
        ble_gap_get_connected(&num_conn, &conn_idx);

        while (num_conn--) {
                set_db_increment(svc, conn_idx[num_conn], user_id, increment);
        }

        if (conn_idx) {
                OS_FREE(conn_idx);
        }

        if (!notify) {
                return;
        }

        notify_db_increment(svc, src_conn_idx, user_id, increment);
}

static void cleanup(ble_service_t *svc)
{
        ud_service_t *uds = (ud_service_t *) svc;

        ble_storage_remove_all(uds->dbi_val_h);
        ble_storage_remove_all(uds->dbi_ccc_h);
        ble_storage_remove_all(uds->ucp_val_h);
        ble_storage_remove_all(uds->ucp_ccc_h);
        ble_storage_remove_all(uds->uid_val_h);

        OS_FREE(uds);
}

ble_service_t *uds_init(const ble_service_config_t *config, uds_db_field_t db_fields,
                                                                        const uds_callbacks_t *cb)
{
        ud_service_t *uds;
        uint16_t num_field = 0;
        uint16_t num_attr;
        uint32_t f;
        att_uuid_t uuid;

        // count bits set in db_fields
        for (f = db_fields; f; num_field++) {
                f &= f - 1;
        }

        // at least one field should be present
        if (!num_field) {
                return NULL;
        }

        uds = OS_MALLOC(sizeof(*uds));
        memset(uds, 0, sizeof(*uds));

        // User Index, Database Change Increment (+CCC), User Control Point (+CCC) are always present
        num_attr = ble_service_get_num_attr(config, 3 + num_field, 2);

        ble_uuid_create16(UUID_SERVICE_UDS, &uuid);
        ble_gatts_add_service(&uuid, GATT_SERVICE_PRIMARY, num_attr);

        ble_service_config_add_includes(config);

        // add mandatory characteristics
        ble_uuid_create16(UUID_DATABASE_CHANGE_INCREMENT, &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_READ | GATT_PROP_WRITE | GATT_PROP_NOTIFY,
                                        ble_service_config_elevate_perm(ATT_PERM_READ |
                                                                ATT_PERM_WRITE_ENCRYPT, config),
                                        sizeof(uint32_t), GATTS_FLAG_CHAR_READ_REQ, NULL,
                                        &uds->dbi_val_h);

        ble_uuid_create16(UUID_GATT_CLIENT_CHAR_CONFIGURATION, &uuid);
        ble_gatts_add_descriptor(&uuid, ATT_PERM_RW, sizeof(uint16_t), 0, &uds->dbi_ccc_h);

        ble_uuid_create16(UUID_USER_INDEX, &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_READ, ble_service_config_elevate_perm(
                                                                ATT_PERM_READ_ENCRYPT, config),
                                        sizeof(uint8_t), GATTS_FLAG_CHAR_READ_REQ, NULL,
                                        &uds->uid_val_h);

        ble_uuid_create16(UUID_USER_CONTROL_POINT, &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_WRITE | GATT_PROP_INDICATE,
                                        ble_service_config_elevate_perm(ATT_PERM_WRITE_ENCRYPT,
                                                                                        config),
                                        19, 0, NULL, &uds->ucp_val_h);

        ble_uuid_create16(UUID_GATT_CLIENT_CHAR_CONFIGURATION, &uuid);
        ble_gatts_add_descriptor(&uuid, ATT_PERM_RW, sizeof(uint16_t), 0, &uds->ucp_ccc_h);

        // add optional characteristics
        for (f = 0; f < UDS_IDX_LAST; f++) {
                const struct db_field *field = &fields[f];

                uds->db_val_h[f] = 0;

                if (!(db_fields & (1 << f))) {
                        continue;
                }

                ble_uuid_create16(field->uuid16, &uuid);
                ble_gatts_add_characteristic(&uuid, GATT_PROP_READ | GATT_PROP_WRITE,
                                                ble_service_config_elevate_perm(ATT_PERM_RW_ENCRYPT,
                                                                                        config),
                                                field->max_length, GATTS_FLAG_CHAR_READ_REQ, NULL,
                                                &uds->db_val_h[f]);
        }

        ble_gatts_register_service(&uds->svc.start_h, &uds->dbi_val_h, &uds->dbi_ccc_h,
                                                &uds->uid_val_h, &uds->ucp_val_h, &uds->ucp_ccc_h, 0);

        // translate handles for optional characteristics
        for (f = 0; f < UDS_IDX_LAST; f++) {
                if (uds->db_val_h[f]) {
                        uds->db_val_h[f] += uds->svc.start_h;
                }
        }

        uds->svc.end_h = uds->svc.start_h + num_attr;
        uds->svc.read_req = handle_read_req;
        uds->svc.write_req = handle_write_req;
        uds->svc.event_sent = handle_event_sent;
        uds->svc.cleanup = cleanup;
        uds->cb = cb;

        ble_service_add(&uds->svc);

        return &uds->svc;
}
