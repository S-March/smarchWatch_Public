/**
 ****************************************************************************************
 *
 * @file dis.c
 *
 * @brief Device Information Service sample implementation
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
#include "ble_gatts.h"
#include "ble_uuid.h"
#include "dis.h"

#define UUID_MANUFACTURER_NAME_STRING   (0x2A29)
#define UUID_MODEL_NUMBER_STRING        (0x2A24)
#define UUID_SERIAL_NUMBER_STRING       (0x2A25)
#define UUID_HARDWARE_REVISION_STRING   (0x2A27)
#define UUID_FIRMWARE_REVISION_STRING   (0x2A26)
#define UUID_SOFTWARE_REVISION_STRING   (0x2A28)
#define UUID_SYSTEM_ID                  (0x2A23)
#define UUID_IEEE_REGULATORY_CERT_LIST  (0x2A2A)
#define UUID_PNP_ID                     (0x2A50)

static uint8_t get_num_chars(const dis_device_info_t *info)
{
        uint8_t ret = 0;

        if (info->manufacturer != NULL) {
                ret++;
        }

        if (info->model_number != NULL) {
                ret++;
        }

        if (info->serial_number != NULL) {
                ret++;
        }

        if (info->hw_revision != NULL) {
                ret++;
        }

        if (info->fw_revision != NULL) {
                ret++;
        }

        if (info->sw_revision != NULL) {
                ret++;
        }

        if (info->system_id != NULL) {
                ret++;
        }

        if (info->reg_cert != NULL && info->reg_cert_length != 0) {
                ret++;
        }

        if (info->pnp_id != NULL) {
                ret++;
        }

        return ret;
}

static void cleanup(ble_service_t *svc)
{
        OS_FREE(svc);
}

ble_service_t *dis_init(const ble_service_config_t *config, const dis_device_info_t *info)
{
        ble_service_t *dis;
        uint16_t num_attr;
        uint16_t manufacturer_name_val_h;
        uint16_t model_number_val_h;
        uint16_t serial_no_val_h;
        uint16_t hw_rev_val_h;
        uint16_t fw_rev_val_h;
        uint16_t sw_rev_val_h;
        uint16_t sys_id_val_h;
        uint16_t cert_val_h;
        uint16_t pnp_id_val_h;
        uint8_t num_chars;
        att_perm_t read_perm;
        att_uuid_t uuid;

        if (!info) {
                return NULL;
        }

        num_chars = get_num_chars(info);

        if (num_chars == 0) {
                return NULL;
        }

        dis = OS_MALLOC(sizeof(*dis));
        memset(dis, 0, sizeof(*dis));

        num_attr = ble_service_get_num_attr(config, num_chars, 0);
        read_perm = ble_service_config_elevate_perm(ATT_PERM_READ, config);

        ble_uuid_create16(UUID_SERVICE_DIS, &uuid);
        ble_gatts_add_service(&uuid, GATT_SERVICE_PRIMARY, num_attr);

        ble_service_config_add_includes(config);

        if (info->manufacturer != NULL) {
                ble_uuid_create16(UUID_MANUFACTURER_NAME_STRING, &uuid);
                ble_gatts_add_characteristic(&uuid, GATT_PROP_READ, read_perm,
                                                strlen(info->manufacturer), 0, NULL,
                                                                        &manufacturer_name_val_h);
        }

        if (info->model_number != NULL) {
                ble_uuid_create16(UUID_MODEL_NUMBER_STRING, &uuid);
                ble_gatts_add_characteristic(&uuid, GATT_PROP_READ, read_perm,
                                                strlen(info->model_number), 0, NULL, &model_number_val_h);
        }

        if (info->serial_number != NULL) {
                ble_uuid_create16(UUID_SERIAL_NUMBER_STRING, &uuid);
                ble_gatts_add_characteristic(&uuid, GATT_PROP_READ, read_perm,
                                                strlen(info->serial_number), 0, NULL, &serial_no_val_h);
        }

        if (info->hw_revision != NULL) {
                ble_uuid_create16(UUID_HARDWARE_REVISION_STRING, &uuid);
                ble_gatts_add_characteristic(&uuid, GATT_PROP_READ, read_perm,
                                                strlen(info->hw_revision), 0, NULL, &hw_rev_val_h);
        }

        if (info->fw_revision != NULL) {
                ble_uuid_create16(UUID_FIRMWARE_REVISION_STRING, &uuid);
                ble_gatts_add_characteristic(&uuid, GATT_PROP_READ, read_perm,
                                                strlen(info->fw_revision), 0, NULL, &fw_rev_val_h);
        }

        if (info->sw_revision != NULL) {
                ble_uuid_create16(UUID_SOFTWARE_REVISION_STRING, &uuid);
                ble_gatts_add_characteristic(&uuid, GATT_PROP_READ, read_perm,
                                                strlen(info->sw_revision), 0, NULL, &sw_rev_val_h);
        }

        if (info->system_id != NULL) {
                ble_uuid_create16(UUID_SYSTEM_ID, &uuid);
                ble_gatts_add_characteristic(&uuid, GATT_PROP_READ, read_perm, 8, 0,
                                                                        NULL, &sys_id_val_h);
        }

        if (info->reg_cert != NULL && info->reg_cert_length != 0) {
                ble_uuid_create16(UUID_IEEE_REGULATORY_CERT_LIST, &uuid);
                ble_gatts_add_characteristic(&uuid, GATT_PROP_READ, read_perm,
                                                info->reg_cert_length, 0, NULL, &cert_val_h);
        }

        if (info->pnp_id != NULL) {
                ble_uuid_create16(UUID_PNP_ID, &uuid);
                ble_gatts_add_characteristic(&uuid, GATT_PROP_READ, read_perm, 8, 0,
                                                                        NULL, &pnp_id_val_h);
        }

        ble_gatts_register_service(&dis->start_h, &manufacturer_name_val_h, &model_number_val_h,
                                                &serial_no_val_h, &hw_rev_val_h, &fw_rev_val_h,
                                                        &sw_rev_val_h, &sys_id_val_h, &cert_val_h,
                                                                                &pnp_id_val_h, 0);

        if (info->manufacturer != NULL) {
                ble_gatts_set_value(manufacturer_name_val_h, strlen(info->manufacturer),
                                                                                info->manufacturer);
        }

        if (info->model_number != NULL) {
                ble_gatts_set_value(model_number_val_h, strlen(info->model_number),
                                                                                info->model_number);
        }

        if (info->serial_number != NULL) {
                ble_gatts_set_value(serial_no_val_h, strlen(info->serial_number),
                                                                        info->serial_number);
        }

        if (info->hw_revision != NULL) {
                ble_gatts_set_value(hw_rev_val_h, strlen(info->hw_revision), info->hw_revision);
        }

        if (info->fw_revision != NULL) {
                ble_gatts_set_value(fw_rev_val_h, strlen(info->fw_revision), info->fw_revision);
        }

        if (info->sw_revision != NULL) {
                ble_gatts_set_value(sw_rev_val_h, strlen(info->sw_revision), info->sw_revision);
        }

        if (info->system_id != NULL) {
                uint8_t buf_sys[8];
                memcpy(buf_sys, info->system_id->manufacturer, 5);
                memcpy(&buf_sys[5], info->system_id->oui, 3);

                ble_gatts_set_value(sys_id_val_h, 8, &buf_sys);
        }

        if (info->reg_cert != NULL && info->reg_cert_length != 0) {
                ble_gatts_set_value(cert_val_h, info->reg_cert_length, info->reg_cert);
        }

        if (info->pnp_id != NULL) {
                uint8_t buf_pnp[7];
                uint8_t *p = buf_pnp;

                put_u8_inc(&p, info->pnp_id->vid_source);
                put_u16_inc(&p, info->pnp_id->vid);
                put_u16_inc(&p, info->pnp_id->pid);
                put_u16_inc(&p, info->pnp_id->version);

                ble_gatts_set_value(pnp_id_val_h, 7, buf_pnp);
        }

        dis->end_h = dis->start_h + num_attr;
        dis->cleanup = cleanup;

        ble_service_add(dis);

        return dis;
}
