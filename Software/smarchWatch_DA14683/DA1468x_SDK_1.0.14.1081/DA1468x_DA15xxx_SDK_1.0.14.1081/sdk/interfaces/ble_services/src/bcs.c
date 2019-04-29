/**
 ****************************************************************************************
 *
 * @file bcs.c
 *
 * @brief Body Composition Service sample implementation
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
#include "ble_gattc.h"
#include "ble_gatts.h"
#include "ble_storage.h"
#include "ble_uuid.h"
#include "svc_defines.h"
#include "bcs.h"

#define UUID_BODY_COMPOSITION_FEATURE           (0x2A9B)
#define UUID_BODY_COMPOSITION_MEASUREMENT       (0x2A9C)

#define PACKET_NORMAL 0
#define PACKET_MULTIPLE 1
#define PAYLOAD_HEADER_LEN 3

enum measurement_flags {
        BCM_FLAG_UNIT_IMPERIAL          = 0x0001,
        BCM_FLAG_TIME_STAMP             = 0x0002,
        BCM_FLAG_USER_ID                = 0x0004,
        BCM_FLAG_BASAL_METABOLISM       = 0x0008,
        BCM_FLAG_MUSCLE_PERCENTAGE      = 0x0010,
        BCM_FLAG_MUSCLE_MASS            = 0x0020,
        BCM_FLAG_FAT_FREE_MASS          = 0x0040,
        BCM_FLAG_SOFT_LEAN_MASS         = 0x0080,
        BCM_FLAG_BODY_WATER_MASS        = 0x0100,
        BCM_FLAG_IMPEDANCE              = 0x0200,
        BCM_FLAG_WEIGHT                 = 0x0400,
        BCM_FLAG_HEIGHT                 = 0x0800,
        BCM_FLAG_MULTIPLE_PACKET        = 0x1000,
};

typedef struct {
        ble_service_t svc;
        bcs_feat_t feat;

        //callbacks
        const bcs_callbacks_t *cb;

        //handles
        uint16_t bcf_val_h;
        uint16_t bcm_val_h;
        uint16_t bcs_ccc_h;
} bc_service_t;

typedef struct {
        uint8_t buf[30]; /* max bcs indication data is set to 30 */
        uint8_t buf_len;
        uint16_t flags;
} packet_t;

static void handle_connected_evt(ble_service_t *svc, const ble_evt_gap_connected_t *evt);
static void handle_read_req(ble_service_t *svc, const ble_evt_gatts_read_req_t *evt);
static void handle_write_req(ble_service_t *svc, const ble_evt_gatts_write_req_t *evt);
static void handle_event_sent_evt(ble_service_t *svc, const ble_evt_gatts_event_sent_t *evt);

static att_error_t do_bcm_ccc_write(bc_service_t *bcs, uint16_t conn_idx, uint16_t offset,
                                                        uint16_t length, const uint8_t *value)
{
        uint16_t ccc_val;

        if (offset) {
                return ATT_ERROR_ATTRIBUTE_NOT_LONG;
        }

        if (length != sizeof(ccc_val)) {
                return ATT_ERROR_APPLICATION_ERROR;
        }

        ccc_val = get_u16(value);

        ble_storage_put_u32(conn_idx, bcs->bcs_ccc_h, ccc_val, true);

        if (bcs->cb && bcs->cb->indication_changed) {
                bcs->cb->indication_changed(conn_idx, ccc_val & GATT_CCC_INDICATIONS);
        }

        return ATT_ERROR_OK;
}

static void handle_read_req(ble_service_t *svc, const ble_evt_gatts_read_req_t *evt)
{
        bc_service_t *bcs = (bc_service_t *) svc;

        if (evt->handle == bcs->bcs_ccc_h) {
                uint16_t ccc_val = 0x0000;

                ble_storage_get_u16(evt->conn_idx, bcs->bcs_ccc_h, &ccc_val);

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
        bc_service_t *bcs = (bc_service_t *) svc;
        att_error_t status = ATT_ERROR_ATTRIBUTE_NOT_FOUND;

        if (evt->handle == bcs->bcs_ccc_h) {
                status = do_bcm_ccc_write(bcs, evt->conn_idx, evt->offset, evt->length, evt->value);
        }

        ble_gatts_write_cfm(evt->conn_idx, evt->handle, status);
}

static void handle_connected_evt(ble_service_t *svc, const ble_evt_gap_connected_t *evt)
{
        bc_service_t *bcs = (bc_service_t *) svc;

        if (bcs->cb && bcs->cb->indication_changed) {
                uint16_t ccc_val = 0x0000;

                ble_storage_get_u16(evt->conn_idx, bcs->bcs_ccc_h, &ccc_val);

                bcs->cb->indication_changed(evt->conn_idx, ccc_val & GATT_CCC_INDICATIONS);
        }
}

static void handle_event_sent_evt(ble_service_t *svc, const ble_evt_gatts_event_sent_t *evt)
{
        bc_service_t *bcs = (bc_service_t *) svc;

        if (bcs->cb && bcs->cb->indication_sent) {
                bcs->cb->indication_sent(evt->conn_idx, evt->status);
        }
}

static void cleanup(ble_service_t *svc)
{
        bc_service_t *bcs = (bc_service_t *) svc;

        ble_storage_remove_all(bcs->bcs_ccc_h);

        OS_FREE(bcs);
}

ble_service_t *bcs_init(const ble_service_config_t *config, bcs_feat_t feat,
                                                                        const bcs_callbacks_t *cb)
{
        bc_service_t *bcs;
        uint16_t num_attr;
        uint32_t bcf;
        att_uuid_t uuid;
        att_perm_t read_perm;

        bcs = OS_MALLOC(sizeof(*bcs));
        memset(bcs, 0, sizeof(*bcs));

        num_attr = ble_service_get_num_attr(config, 2, 1);
        read_perm = ble_service_config_elevate_perm(ATT_PERM_READ, config);

        ble_uuid_create16(UUID_SERVICE_BCS, &uuid);
        ble_gatts_add_service(&uuid, config->service_type, num_attr);

        ble_service_config_add_includes(config);

        ble_uuid_create16(UUID_BODY_COMPOSITION_FEATURE, &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_READ, read_perm, 4, 0, NULL, &bcs->bcf_val_h);

        ble_uuid_create16(UUID_BODY_COMPOSITION_MEASUREMENT, &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_INDICATE, ATT_PERM_NONE, 30, 0, NULL, &bcs->bcm_val_h);

        ble_uuid_create16(UUID_GATT_CLIENT_CHAR_CONFIGURATION, &uuid);
        ble_gatts_add_descriptor(&uuid, ATT_PERM_RW, 2, 0, &bcs->bcs_ccc_h);

        ble_gatts_register_service(&bcs->svc.start_h, &bcs->bcf_val_h, &bcs->bcm_val_h,
                                                                                &bcs->bcs_ccc_h, 0);

        bcf = feat;
        ble_gatts_set_value(bcs->bcf_val_h, sizeof(bcf), &bcf);

        bcs->svc.end_h = bcs->svc.start_h + num_attr;

        bcs->cb = cb;
        bcs->feat = feat;

        bcs->svc.read_req = handle_read_req;
        bcs->svc.write_req = handle_write_req;
        bcs->svc.connected_evt = handle_connected_evt;
        bcs->svc.event_sent = handle_event_sent_evt;
        bcs->svc.cleanup = cleanup;

        ble_service_add(&bcs->svc);

        return &bcs->svc;
}

static void measurement_init(const bcs_body_measurement_t *meas, uint16_t *flags, uint8_t **buf)
{
        if (meas->measurement_unit == BCS_UNIT_IMPERIAL) {
                *flags |= BCM_FLAG_UNIT_IMPERIAL;
        }

        put_u16_inc(buf, meas->body_fat_percentage);
}

static void add_field_to_packet(uint16_t field, uint8_t field_len, uint16_t max_mtu,
                                        const bcs_body_measurement_t *meas, uint8_t **cur_buf,
                                                                                packet_t *d_list)
{
        uint8_t cur_size = 0;
        packet_t *data;

        /* case MULTIPLE PACKET */
        if (d_list[PACKET_MULTIPLE].buf_len) {
                data = &d_list[PACKET_MULTIPLE];

                cur_size = *cur_buf - data->buf;
        } else {
                /* case NORMAL PACKET */
                data = &d_list[PACKET_NORMAL];
                cur_size = *cur_buf - data->buf;

                if (cur_size + field_len > max_mtu) {
                        /* The Multiple Packet bit is setting in flags variable for first packet. */
                        data->flags |= BCM_FLAG_MULTIPLE_PACKET;
                        put_u16(data->buf, data->flags);
                        data->buf_len = cur_size;

                        /* multiple packet buffer is initiated. */
                        data = &d_list[PACKET_MULTIPLE];
                        /* Switching packet buffer */
                        *cur_buf = &data->buf[2];
                        measurement_init(meas, &data->flags, cur_buf);
                        data->flags = BCM_FLAG_MULTIPLE_PACKET | field;
                        /* Storing flags and data size for multiple packet buffer */
                        put_u16(data->buf, data->flags);
                        data->buf_len = field_len;

                        return;
                }
        }

        data->flags |= field;
        put_u16(data->buf, data->flags);
        data->buf_len = cur_size + field_len;
}

static void pack_indicate_value(bcs_feat_t bcf, const bcs_body_measurement_t *meas, uint16_t max_mtu,
                                                                                packet_t *d_list)
{
        packet_t * data ;
        uint16_t *flags;
        uint8_t *ptr;

        data = &d_list[PACKET_NORMAL];
        flags = &data->flags;
        ptr = &data->buf[2];

        measurement_init(meas, &data->flags, &ptr);

        if ((bcf & BCS_FEAT_TIME_STAMP) && meas->time_stamp_present) {
                *flags |= BCM_FLAG_TIME_STAMP;
                pack_date_time(&meas->time_stamp, &ptr);
        }

        if (bcf & BCS_FEAT_MULTIPLE_USERS) {
                *flags |= BCM_FLAG_USER_ID;
                put_u8_inc(&ptr, meas->user_id);
        }

        if ((bcf & BCS_FEAT_BASAL_METABOLISM) && meas->basal_metabolism > 0) {
                *flags |= BCM_FLAG_BASAL_METABOLISM;
                put_u16_inc(&ptr, meas->basal_metabolism);
        }

        if ((bcf & BCS_FEAT_MUSCLE_PERCENTAGE) && meas->muscle_percentage > 0) {
                *flags |= BCM_FLAG_MUSCLE_PERCENTAGE;
                put_u16_inc(&ptr, meas->muscle_percentage);
        }

        if ((bcf & BCS_FEAT_MUSCLE_MASS) && meas->muscle_mass > 0) {
                *flags |= BCM_FLAG_MUSCLE_MASS;
                put_u16_inc(&ptr, meas->muscle_mass);
        }

        if ((bcf & BCS_FEAT_FAT_FREE_MASS) && meas->fat_free_mass > 0) {
                *flags |= BCM_FLAG_FAT_FREE_MASS;
                put_u16_inc(&ptr, meas->fat_free_mass);
        }

        put_u16(data->buf, *flags);
        data->buf_len = ptr - data->buf;

        /*
         * Each above parameter can be sent in one packet (20 bytes), but for each below parameter
         * current packet size shall be check and MULTIPLE_PACKET feature shall be used if needed
         */
        if ((bcf & BCS_FEAT_SOFT_LEAN_MASS) && meas->soft_lean_mass > 0) {
                add_field_to_packet(BCM_FLAG_SOFT_LEAN_MASS, 2, max_mtu, meas, &ptr, d_list);
                put_u16_inc(&ptr, meas->soft_lean_mass);
        }

        if ((bcf & BCS_FEAT_BODY_WATER_MASS) && meas->body_water_mass > 0) {
                add_field_to_packet(BCM_FLAG_BODY_WATER_MASS, 2, max_mtu, meas,&ptr, d_list);
                put_u16_inc(&ptr, meas->body_water_mass);
        }

        if ((bcf & BCS_FEAT_IMPEDANCE) && meas->impedance > 0) {
                add_field_to_packet(BCM_FLAG_IMPEDANCE, 2, max_mtu, meas, &ptr, d_list);
                put_u16_inc(&ptr, meas->impedance);
        }

        if ((bcf & BCS_FEAT_WEIGHT) && meas->weight > 0) {
                add_field_to_packet(BCM_FLAG_WEIGHT, 2, max_mtu, meas, &ptr, d_list);
                put_u16_inc(&ptr, meas->weight);
        }

        if ((bcf & BCS_FEAT_HEIGHT) && meas->height > 0) {
                add_field_to_packet(BCM_FLAG_HEIGHT, 2, max_mtu, meas, &ptr, d_list);
                put_u16_inc(&ptr, meas->height);
        }
}

ble_error_t bcs_indicate(ble_service_t *svc, uint16_t conn_idx, bcs_body_measurement_t *meas)
{
        bc_service_t *bcs = (bc_service_t *) svc;
        uint16_t ccc_val = 0x0000;
        uint16_t max_mtu;
        packet_t data_list[2];
        ble_error_t err;

        memset(data_list, 0x00, sizeof(packet_t) * 2);

        ble_storage_get_u16(conn_idx, bcs->bcs_ccc_h, &ccc_val);

        if (!(ccc_val & GATT_CCC_INDICATIONS)) {
                // indication disabled by client
                return BLE_ERROR_NOT_ALLOWED;
        }

        ble_gattc_get_mtu(conn_idx, &max_mtu);

        pack_indicate_value(bcs->feat, meas, max_mtu - PAYLOAD_HEADER_LEN, data_list);

        err = ble_gatts_send_event(conn_idx, bcs->bcm_val_h, GATT_EVENT_INDICATION,
                                data_list[PACKET_NORMAL].buf_len, data_list[PACKET_NORMAL].buf);
        /*
         * If MULTIPLE_PACKET will be detected remaining BCS data will be send in second
         * packet transmission called "continuation packet".
         */
        if (data_list[PACKET_MULTIPLE].buf_len) {
                err = ble_gatts_send_event(conn_idx, bcs->bcm_val_h, GATT_EVENT_INDICATION,
                                data_list[PACKET_MULTIPLE].buf_len, data_list[PACKET_MULTIPLE].buf);
        }

        return err;
}

void bcs_indicate_all(ble_service_t *svc, bcs_body_measurement_t *meas)
{
        uint8_t num_conn;
        uint16_t *conn_idx;

        ble_gap_get_connected(&num_conn, &conn_idx);

        while (num_conn--) {
                bcs_indicate(svc, conn_idx[num_conn], meas);
        }

        if (conn_idx) {
                OS_FREE(conn_idx);
        }
}

bool bcs_is_indication_enabled(ble_service_t *svc, uint16_t conn_idx)
{
        bc_service_t *bcs = (bc_service_t *) svc;
        uint16_t ccc_val = 0x0000;

        ble_storage_get_u16(conn_idx, bcs->bcs_ccc_h, &ccc_val);

        return (ccc_val & GATT_CCC_INDICATIONS);
}
