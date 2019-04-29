/**
 ****************************************************************************************
 *
 * @file hids.c
 *
 * @brief HID Service sample implementation
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdbool.h>
#include <string.h>
#include "osal.h"
#include "ble_att.h"
#include "ble_bufops.h"
#include "ble_common.h"
#include "ble_gatts.h"
#include "ble_uuid.h"
#include "ble_storage.h"
#include "hids.h"

#define UUID_HID_SERVICE                 (0x1812)
#define UUID_PROTOCOL_MODE               (0x2A4E)
#define UUID_REPORT                      (0x2A4D)
#define UUID_REPORT_REFERENCE            (0x2908)
#define UUID_REPORT_MAP                  (0x2A4B)
#define UUID_BOOT_KEYBOARD_INPUT_REPORT  (0x2A22)
#define UUID_BOOT_KEYBOARD_OUTPUT_REPORT (0x2A32)
#define UUID_BOOT_MOUSE_INPUT_REPORT     (0x2A33)
#define UUID_HID_INFORMATION             (0x2A4A)
#define UUID_HID_CONTROL_POINT           (0x2A4C)

#define HID_INFO_VAL_SIZE                (4)
#define CCC_VAL_SIZE                     (2)
#define REPORT_REFERENCE_VAL_SIZE        (2)
#define BOOT_REPORT_SIZE                 (8)

typedef struct {
        hids_report_type_t type;
        uint8_t report_id;
        uint16_t length;
        uint16_t cur_length;
        uint8_t *value;
        uint16_t val_h;
        uint16_t ccc_h;
        uint16_t report_ref_h;
} report_t;

typedef struct {
        ble_service_t svc;

        const hids_callbacks_t *cb;

        uint8_t protocol_mode;

        uint16_t protocol_mode_val_h;
        uint16_t hid_cp_val_h;
        uint16_t report_map_h;
        uint16_t hid_info_h;

        uint8_t num_reports;
        report_t *reports;

        uint16_t boot_keyboard_input_val_h;
        uint16_t boot_keyboard_input_ccc_h;
        uint16_t boot_keyboard_output_val_h;
        uint16_t boot_mouse_input_val_h;
        uint16_t boot_mouse_input_ccc_h;

        uint16_t boot_keyboard_input_length;
        uint8_t *boot_keyboard_input_val;

        uint16_t boot_keyboard_output_length;
        uint8_t *boot_keyboard_output_val;

        uint16_t boot_mouse_input_length;
        uint8_t *boot_mouse_input_val;

        uint16_t conn_idx; // attached connection index

        bool is_busy; // notifications busy flag
} hid_service_t;

static report_t *find_report_by_handle(hid_service_t *hids, uint16_t handle)
{
        uint8_t i;

        for (i = 0; i < hids->num_reports; i++) {
                if (hids->reports[i].val_h == handle || hids->reports[i].ccc_h == handle) {
                        return &hids->reports[i];
                }
        }

        return NULL;
}

static att_error_t handle_protocol_mode_write(hid_service_t *hids, uint16_t conn_idx,
                                        uint16_t offset, uint16_t length, const uint8_t *value)
{
        if (offset) {
                return ATT_ERROR_ATTRIBUTE_NOT_LONG;
        }

        if (length != sizeof(uint8_t)) {
                return ATT_ERROR_INVALID_VALUE_LENGTH;
        }

        if (value[0] != HIDS_PROTOCOL_MODE_BOOT && value[0] != HIDS_PROTOCOL_MODE_REPORT) {
                return ATT_ERROR_APPLICATION_ERROR;
        }

        hids->protocol_mode = value[0];
        ble_gatts_set_value(hids->protocol_mode_val_h, sizeof(uint8_t), value);

        if (hids->cb && hids->cb->set_protocol_mode) {
                hids->cb->set_protocol_mode(&hids->svc, value[0]);
        }

        return ATT_ERROR_OK;
}

static att_error_t handle_control_point_write(hid_service_t *hids, uint16_t conn_idx,
                                        uint16_t offset, uint16_t length, const uint8_t *value)
{
        if (offset) {
                return ATT_ERROR_ATTRIBUTE_NOT_LONG;
        }

        if (length != sizeof(uint8_t)) {
                return ATT_ERROR_INVALID_VALUE_LENGTH;
        }

        if (value[0] != HIDS_CONTROL_POINT_SUSPEND &&
                                                value[0] != HIDS_CONTROL_POINT_EXIT_SUSPEND) {
                return ATT_ERROR_APPLICATION_ERROR;
        }

        if (hids->cb && hids->cb->control_point) {
                hids->cb->control_point(&hids->svc, value[0]);
        }

        return ATT_ERROR_OK;
}

static att_error_t handle_boot_keyboard_write(hid_service_t *hids, uint16_t conn_idx,
                                        uint16_t offset, uint16_t length, const uint8_t *value)
{
        if (offset) {
                return ATT_ERROR_ATTRIBUTE_NOT_LONG;
        }

        if (length > BOOT_REPORT_SIZE) {
                return ATT_ERROR_INVALID_VALUE_LENGTH;
        }

        memcpy(hids->boot_keyboard_output_val, value, length);
        hids->boot_keyboard_output_length = length;

        if (hids->cb && hids->cb->boot_keyboard_write) {
                hids->cb->boot_keyboard_write(&hids->svc, length, value);
        }

        return ATT_ERROR_OK;
}

static att_error_t handle_ccc_write(hid_service_t *hids, uint16_t conn_idx, uint16_t handle,
                                        uint16_t offset, uint16_t length, const uint8_t *value)
{
        uint16_t ccc;

        if (offset) {
                return ATT_ERROR_ATTRIBUTE_NOT_LONG;
        }

        if (length != sizeof(ccc)) {
                return ATT_ERROR_APPLICATION_ERROR;
        }

        ccc = get_u16(value);

        ble_storage_put_u32(conn_idx, handle, ccc, true);

        return ATT_ERROR_OK;
}

static att_error_t handle_report_write(hid_service_t *hids, report_t *report, uint16_t offset,
                                                        uint16_t length, const uint8_t *value)
{
        if (offset) {
                return ATT_ERROR_ATTRIBUTE_NOT_LONG;
        }

        if (length > report->length) {
                return ATT_ERROR_INVALID_VALUE_LENGTH;
        }

        memcpy(report->value, value, length);
        report->cur_length = length;

        if (hids->cb && hids->cb->report_write) {
                hids->cb->report_write(&hids->svc, report->type, report->report_id, length, value);
        }

        return ATT_ERROR_OK;
}

static void handle_write_req(ble_service_t *svc, const ble_evt_gatts_write_req_t *evt)
{
        hid_service_t *hids = (hid_service_t *) svc;
        att_error_t status = ATT_ERROR_WRITE_NOT_PERMITTED;
        uint16_t handle = evt->handle;
        report_t *report;

        if (evt->conn_idx != hids->conn_idx) {
                goto done;
        }

        if (handle == hids->protocol_mode_val_h) {
                status = handle_protocol_mode_write(hids, evt->conn_idx, evt->offset, evt->length,
                                                                                evt->value);
                goto done;
        }

        if (handle == hids->hid_cp_val_h) {
                status = handle_control_point_write(hids, evt->conn_idx, evt->offset, evt->length,
                                                                                evt->value);
                goto done;
        }

        if (handle == hids->boot_mouse_input_ccc_h || handle == hids->boot_keyboard_input_ccc_h) {
                status = handle_ccc_write(hids, evt->conn_idx, handle, evt->offset, evt->length,
                                                                                evt->value);
                goto done;
        }

        if (handle == hids->boot_keyboard_output_val_h) {
                status = handle_boot_keyboard_write(hids, evt->conn_idx, evt->offset, evt->length,
                                                                                        evt->value);
                goto done;
        }

        report = find_report_by_handle(hids, handle);
        if (report) {
                if (handle == report->val_h) {
                        status = handle_report_write(hids, report, evt->offset, evt->length,
                                                                                evt->value);
                        goto done;
                } else if (handle == report->ccc_h) {
                        status = handle_ccc_write(hids, evt->conn_idx, handle, evt->offset,
                                                                        evt->length, evt->value);
                        goto done;
                }
        }

done:
        ble_gatts_write_cfm(evt->conn_idx, evt->handle, status);
}

static void handle_report_read(hid_service_t *hids, uint16_t handle, uint16_t report_length,
                                        const uint8_t *report_value, uint16_t offset)
{
        att_error_t status = ATT_ERROR_OK;
        uint16_t length = 0;
        const uint8_t *data = NULL;

        if (offset > report_length) {
                status = ATT_ERROR_INVALID_OFFSET;
                goto done;
        }

        length = report_length - offset;
        if (length > 0) {
                data = &report_value[offset];
        }

done:
        ble_gatts_read_cfm(hids->conn_idx, handle, status, length, data);
}

static void handle_read_req(ble_service_t *svc, const ble_evt_gatts_read_req_t *evt)
{
        hid_service_t *hids = (hid_service_t *) svc;
        uint16_t handle = evt->handle;
        report_t *report = find_report_by_handle(hids, handle);

        if (evt->conn_idx != hids->conn_idx) {
                ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_READ_NOT_PERMITTED, 0, NULL);
                return;
        }

        if (handle == hids->boot_keyboard_input_ccc_h || handle == hids->boot_mouse_input_ccc_h ||
                                                        (report && report->ccc_h == handle)) {
                uint16_t ccc = 0x0000;

                ble_storage_get_u16(evt->conn_idx, handle, &ccc);

                ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_OK, sizeof(ccc), &ccc);
        } else if (handle == hids->boot_keyboard_input_val_h) {
                handle_report_read(hids, handle, hids->boot_keyboard_input_length,
                                                hids->boot_keyboard_input_val, evt->offset);
        } else if (handle == hids->boot_keyboard_output_val_h) {
                handle_report_read(hids, handle, hids->boot_keyboard_output_length,
                                                hids->boot_keyboard_output_val, evt->offset);
        } else if (handle == hids->boot_mouse_input_val_h) {
                handle_report_read(hids, handle, hids->boot_mouse_input_length,
                                                hids->boot_mouse_input_val, evt->offset);
        } else if (report && handle == report->val_h) {
                handle_report_read(hids, handle, report->cur_length, report->value, evt->offset);
        } else {
                ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_READ_NOT_PERMITTED, 0, NULL);
        }
}

static void handle_disconnected(ble_service_t *svc, const ble_evt_gap_disconnected_t *evt)
{
        hid_service_t *hids = (hid_service_t *) svc;

        hids->conn_idx = BLE_CONN_IDX_INVALID;
        hids->is_busy = false;
}

static void handle_event_sent(ble_service_t *svc, const ble_evt_gatts_event_sent_t *evt)
{
        hid_service_t *hids = (hid_service_t *) svc;
        uint16_t handle = evt->handle;

        /* Set IDLE flag here */
        hids->is_busy = false;

        if (!hids->cb) {
                return;
        }

        if (hids->cb->report_sent) {
                hids->cb->report_sent(&hids->svc);
        }

        if (handle == hids->boot_mouse_input_val_h) {
                if (hids->cb->notify_boot_mouse_input_report_completed) {
                        hids->cb->notify_boot_mouse_input_report_completed(svc, evt->status);
                }
        } else if (handle == hids->boot_keyboard_input_val_h) {
                if ( hids->cb->notify_boot_keyboard_input_report_completed) {
                        hids->cb->notify_boot_keyboard_input_report_completed(svc, evt->status);
                }
        } else if (hids->cb->notify_input_report_completed) {
                report_t *report = find_report_by_handle(hids, handle);

                if (report) {
                        hids->cb->notify_input_report_completed(svc, report->report_id,
                                                                                evt->status);
                }
        }
}

static uint16_t get_num_attrs(const hids_config_t *config,
                                                const ble_service_config_t *service_config)
{
        /* protocol_mode + hid_info + hid_control + report_map */
        uint16_t chars = 4;
        uint16_t descs = 0;
        uint8_t i;

        chars += config->num_reports;

        for (i = 0; i < config->num_reports; i++) {
                if (config->reports[i].type == HIDS_REPORT_TYPE_INPUT) {
                        /* Report Reference Desc. + CCC Desc */
                        descs += 2;
                } else {
                        /* Report Reference Desc */
                        descs += 1;
                }
        }

        if (config->boot_device & HIDS_BOOT_DEVICE_MOUSE) {
                /* Boot Mouse Input Report Characteristic */
                chars += 1;
                /* CCC Desc */
                descs += 1;
        }

        if (config->boot_device & HIDS_BOOT_DEVICE_KEYBOARD) {
                /* Boot Keyboard Output Report + Boot Keyboard Input Characteristics */
                chars += 2;
                /* CCC Desc. */
                descs += 1;
        }

        return ble_service_get_num_attr(service_config, chars, descs);
}

static void set_values(hid_service_t *hids, const hids_config_t *config)
{
        uint8_t hid_info_data[4];
        int i;

        /* Protocol mode */
        hids->protocol_mode = HIDS_PROTOCOL_MODE_REPORT;
        ble_gatts_set_value(hids->protocol_mode_val_h, sizeof(uint8_t), &hids->protocol_mode);

        /* HID info */
        put_u16(hid_info_data, config->hids_info.bcd_hid);
        hid_info_data[2] = config->hids_info.country_code;
        hid_info_data[3] = config->hids_info.flags;
        ble_gatts_set_value(hids->hid_info_h, sizeof(hid_info_data), hid_info_data);

        /* Report map */
        ble_gatts_set_value(hids->report_map_h, config->report_map_length, config->report_map);

        /* Boot keyboard */
        if (hids->boot_keyboard_input_val_h) {
                memset(hids->boot_keyboard_input_val, 0, hids->boot_keyboard_input_length);
                memset(hids->boot_keyboard_output_val, 0, hids->boot_keyboard_output_length);
        }

        /* Boot mouse */
        if (hids->boot_mouse_input_val_h) {
                memset(hids->boot_mouse_input_val, 0, hids->boot_mouse_input_length);
        }

        /* Report chars */
        for (i = 0; i < hids->num_reports; i++) {
                report_t *report = &hids->reports[i];
                uint8_t report_ref_data[2];

                report_ref_data[0] = report->report_id;
                report_ref_data[1] = report->type;

                memset(report->value, 0, report->length);
                report->cur_length = 0;

                ble_gatts_set_value(report->report_ref_h, sizeof(report_ref_data),
                                                                                report_ref_data);
        }
}

static void update_handles(hid_service_t *hids, uint16_t start_h)
{
        int i;

        hids->hid_cp_val_h += start_h;
        hids->protocol_mode_val_h += start_h;
        hids->report_map_h += start_h;
        hids->hid_info_h += start_h;

        if (hids->boot_keyboard_input_val_h) {
                hids->boot_keyboard_input_val_h += start_h;
                hids->boot_keyboard_input_ccc_h += start_h;
                hids->boot_keyboard_output_val_h += start_h;
        }

        if (hids->boot_mouse_input_val_h) {
                hids->boot_mouse_input_val_h += start_h;
                hids->boot_mouse_input_ccc_h += start_h;
        }

        for (i = 0; i < hids->num_reports; i++) {
                report_t *report = &hids->reports[i];

                report->val_h += start_h;
                report->report_ref_h += start_h;

                if (report->type == HIDS_REPORT_TYPE_INPUT) {
                        report->ccc_h += start_h;
                }
        }
}

static void add_ccc(uint16_t *ccc_h)
{
        att_uuid_t uuid;

        ble_uuid_create16(UUID_GATT_CLIENT_CHAR_CONFIGURATION, &uuid);
        ble_gatts_add_descriptor(&uuid, ATT_PERM_RW, CCC_VAL_SIZE, 0, ccc_h);
}

static void add_report(const hids_report_t *hids_report, report_t *report,
                                                                const ble_service_config_t *config)
{
        att_uuid_t uuid;
        uint8_t prop;

        report->report_id = hids_report->report_id;
        report->type = hids_report->type;
        report->length = hids_report->length;
        report->ccc_h = 0x0000;
        report->value = OS_MALLOC(report->length);

        prop = GATT_PROP_READ | GATT_PROP_WRITE;
        prop |= (report->type == HIDS_REPORT_TYPE_INPUT) ? GATT_PROP_NOTIFY : 0;
        prop |= (report->type == HIDS_REPORT_TYPE_OUTPUT) ? GATT_PROP_WRITE_NO_RESP : 0;

        ble_uuid_create16(UUID_REPORT, &uuid);
        ble_gatts_add_characteristic(&uuid, prop,
                                        ble_service_config_elevate_perm(ATT_PERM_RW, config),
                                        hids_report->length, GATTS_FLAG_CHAR_READ_REQ, NULL,
                                        &report->val_h);

        if (report->type == HIDS_REPORT_TYPE_INPUT) {
                add_ccc(&report->ccc_h);
        }

        ble_uuid_create16(UUID_REPORT_REFERENCE, &uuid);
        ble_gatts_add_descriptor(&uuid, ble_service_config_elevate_perm(ATT_PERM_READ, config),
                                        REPORT_REFERENCE_VAL_SIZE, 0, &report->report_ref_h);
}

static void add_boot_mouse(hid_service_t *hids, const ble_service_config_t *config)
{
        att_uuid_t uuid;

        /* Boot mouse input */
        ble_uuid_create16(UUID_BOOT_MOUSE_INPUT_REPORT, &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_READ | GATT_PROP_NOTIFY,
                                        ble_service_config_elevate_perm(ATT_PERM_READ, config),
                                        BOOT_REPORT_SIZE, GATTS_FLAG_CHAR_READ_REQ, NULL,
                                        &hids->boot_mouse_input_val_h);

        add_ccc(&hids->boot_mouse_input_ccc_h);

        hids->boot_mouse_input_val = OS_MALLOC(BOOT_REPORT_SIZE);
        hids->boot_mouse_input_length = BOOT_REPORT_SIZE;
}

static void add_boot_keyboard(hid_service_t *hids, const ble_service_config_t *config)
{
        att_uuid_t uuid;

        /* Boot keyboard input */
        ble_uuid_create16(UUID_BOOT_KEYBOARD_INPUT_REPORT, &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_READ | GATT_PROP_NOTIFY,
                                        ble_service_config_elevate_perm(ATT_PERM_READ, config),
                                        BOOT_REPORT_SIZE, GATTS_FLAG_CHAR_READ_REQ, NULL,
                                        &hids->boot_keyboard_input_val_h);

        add_ccc(&hids->boot_keyboard_input_ccc_h);

        hids->boot_keyboard_input_val = OS_MALLOC(BOOT_REPORT_SIZE);
        hids->boot_keyboard_input_length = BOOT_REPORT_SIZE;

        /* Boot keyboard output */
        ble_uuid_create16(UUID_BOOT_KEYBOARD_OUTPUT_REPORT, &uuid);
        ble_gatts_add_characteristic(&uuid,
                                        GATT_PROP_READ | GATT_PROP_WRITE | GATT_PROP_WRITE_NO_RESP,
                                        ble_service_config_elevate_perm(ATT_PERM_RW, config),
                                        BOOT_REPORT_SIZE, GATTS_FLAG_CHAR_READ_REQ, NULL,
                                        &hids->boot_keyboard_output_val_h);

        hids->boot_keyboard_output_val = OS_MALLOC(BOOT_REPORT_SIZE);
        hids->boot_keyboard_output_length = BOOT_REPORT_SIZE;
}

static void cleanup(ble_service_t *svc)
{
        hid_service_t *hids = (hid_service_t *) svc;
        int i;

        if (hids->boot_keyboard_input_ccc_h) {
                ble_storage_remove_all(hids->boot_keyboard_input_ccc_h);
        }

        if (hids->boot_mouse_input_ccc_h) {
                ble_storage_remove_all(hids->boot_mouse_input_ccc_h);
        }

        for (i = 0; i < hids->num_reports; i++) {
                if (hids->reports[i].ccc_h) {
                        ble_storage_remove_all(hids->reports[i].ccc_h);
                }

                OS_FREE(hids->reports[i].value);
        }

        OS_FREE(hids->boot_keyboard_output_val);
        OS_FREE(hids->boot_keyboard_input_val);
        OS_FREE(hids->boot_mouse_input_val);
        OS_FREE(hids->reports);
        OS_FREE(hids);
}

ble_service_t *hids_init(const ble_service_config_t *service_config, const hids_config_t *config,
                                                                const hids_callbacks_t *callbacks)
{
        hid_service_t *hids;
        uint16_t num_attr, start_h;
        att_uuid_t uuid;
        uint8_t i;

        hids = OS_MALLOC(sizeof(*hids));
        memset(hids, 0, sizeof(*hids));

        num_attr = get_num_attrs(config, service_config);

        ble_uuid_create16(UUID_HID_SERVICE, &uuid);
        ble_gatts_add_service(&uuid, GATT_SERVICE_PRIMARY, num_attr);

        ble_service_config_add_includes(service_config);

        /* Protocol Mode */
        ble_uuid_create16(UUID_PROTOCOL_MODE, &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_READ | GATT_PROP_WRITE_NO_RESP,
                                        ble_service_config_elevate_perm(ATT_PERM_RW, service_config),
                                        sizeof(uint8_t), 0, NULL, &hids->protocol_mode_val_h);

        /* Report map */
        ble_uuid_create16(UUID_REPORT_MAP, &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_READ,
                                        ble_service_config_elevate_perm(ATT_PERM_READ, service_config),
                                        config->report_map_length, 0, NULL, &hids->report_map_h);

        /* HID Information */
        ble_uuid_create16(UUID_HID_INFORMATION, &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_READ,
                                        ble_service_config_elevate_perm(ATT_PERM_READ, service_config),
                                        HID_INFO_VAL_SIZE, 0, NULL, &hids->hid_info_h);

        /* HID Control Point */
        ble_uuid_create16(UUID_HID_CONTROL_POINT, &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_WRITE_NO_RESP,
                                        ble_service_config_elevate_perm(ATT_PERM_WRITE, service_config),
                                        sizeof(uint8_t), 0, NULL, &hids->hid_cp_val_h);

        /* Report characteristics */
        if (config->num_reports > 0) {
                hids->num_reports = config->num_reports;
                hids->reports = OS_MALLOC(sizeof(report_t) * hids->num_reports);

                for (i = 0; i < hids->num_reports; i++) {
                        add_report(&config->reports[i], &hids->reports[i], service_config);
                }
        }

        /* Boot Mouse Characteristic */
        if (config->boot_device & HIDS_BOOT_DEVICE_MOUSE) {
                add_boot_mouse(hids, service_config);
        }

        /* Boot Keyboard Characteristic */
        if (config->boot_device & HIDS_BOOT_DEVICE_KEYBOARD) {
                add_boot_keyboard(hids, service_config);
        }

        ble_gatts_register_service(&start_h, 0);

        hids->svc.end_h = start_h + num_attr;
        hids->svc.start_h = start_h;

        /* Update rest of handles */
        update_handles(hids, start_h);

        /* Set default values */
        set_values(hids, config);

        hids->cb = callbacks;
        hids->svc.write_req = handle_write_req;
        hids->svc.read_req = handle_read_req;
        hids->svc.disconnected_evt = handle_disconnected;
        hids->svc.event_sent = handle_event_sent;
        hids->svc.cleanup = cleanup;

        hids->conn_idx = BLE_CONN_IDX_INVALID;

        ble_service_add(&hids->svc);

        return &hids->svc;
}

bool hids_attach_connection(ble_service_t *svc, uint16_t conn_idx)
{
        hid_service_t *hids = (hid_service_t *) svc;

        if (hids->conn_idx != BLE_CONN_IDX_INVALID) {
                return false;
        }

        hids->conn_idx = conn_idx;

        hids->protocol_mode = HIDS_PROTOCOL_MODE_REPORT;
        ble_gatts_set_value(hids->protocol_mode_val_h, sizeof(uint8_t), &hids->protocol_mode);

        if (hids->cb && hids->cb->set_protocol_mode) {
                hids->cb->set_protocol_mode(svc, hids->protocol_mode);
        }

        return true;
}

static bool send_notifications(hid_service_t *hids, uint16_t val_h, uint16_t ccc_h,
                                                        uint16_t length, const uint8_t *data)
{
        uint16_t ccc = 0x0000;
        ble_error_t status;

        if (hids->is_busy) {
                return false;
        }

        if (hids->conn_idx == BLE_CONN_IDX_INVALID) {
                return true;
        }

        ble_storage_get_u16(hids->conn_idx, ccc_h, &ccc);
        if (!(ccc & GATT_CCC_NOTIFICATIONS)) {
                return true;
        }

        status = ble_gatts_send_event(hids->conn_idx, val_h, GATT_EVENT_NOTIFICATION, length, data);
        hids->is_busy = (status == BLE_STATUS_OK) ? true : false;

        return hids->is_busy;
}

bool hids_set_report_value(ble_service_t *svc, hids_report_type_t type, uint8_t report_id,
                                                        uint16_t length, const uint8_t *data)
{
        hid_service_t *hids = (hid_service_t *) svc;
        report_t *report = NULL;
        int i;

        for (i = 0; i < hids->num_reports; i++) {
                if (hids->reports[i].type == type && hids->reports[i].report_id == report_id) {
                        report = &hids->reports[i];
                        break;
                }
        }

        if (!report) {
                return false;
        }

        if (length > report->length) {
                return false;
        }

        if (hids->protocol_mode == HIDS_PROTOCOL_MODE_REPORT &&
                                                        report->type == HIDS_REPORT_TYPE_INPUT) {
                if (!send_notifications(hids, report->val_h, report->ccc_h, length, data)) {
                        return false;
                }
        }

        memcpy(report->value, data, length);
        report->cur_length = length;

        return true;
}

bool hids_set_boot_keyboard_input_value(ble_service_t *svc, uint16_t length, const uint8_t *data)
{
        hid_service_t *hids = (hid_service_t *) svc;

        if (hids->boot_keyboard_input_val_h == 0x0000) {
                return false;
        }

        if (length > BOOT_REPORT_SIZE) {
                return false;
        }

        if (hids->protocol_mode == HIDS_PROTOCOL_MODE_BOOT) {
                if (!send_notifications(hids, hids->boot_keyboard_input_val_h,
                                                hids->boot_keyboard_input_ccc_h, length, data)) {
                        return false;
                }
        }

        memcpy(hids->boot_keyboard_input_val, data, length);
        hids->boot_keyboard_input_length = length;

        return true;
}

bool hids_set_boot_mouse_input_value(ble_service_t *svc, uint16_t length, const uint8_t *data)
{
        hid_service_t *hids = (hid_service_t *) svc;

        if (hids->boot_mouse_input_val_h == 0x0000) {
                return false;
        }

        if (length > BOOT_REPORT_SIZE) {
                return false;
        }

        if (hids->protocol_mode == HIDS_PROTOCOL_MODE_BOOT) {
                if (!send_notifications(hids, hids->boot_mouse_input_val_h,
                                                hids->boot_mouse_input_ccc_h, length, data)) {
                        return false;
                }
        }

        memcpy(hids->boot_mouse_input_val, data, length);
        hids->boot_mouse_input_length = length;

        return true;
}

bool hids_set_boot_mouse_input_report(ble_service_t *svc, uint16_t length, const uint8_t *data)
{
        hid_service_t *hids = (hid_service_t *) svc;

        if (!hids->boot_mouse_input_val_h) {
                return false;
        }

        if (length > BOOT_REPORT_SIZE) {
                return false;
        }

        memcpy(hids->boot_mouse_input_val, data, length);
        hids->boot_keyboard_input_length = length;

        return true;
}

bool hids_notify_boot_mouse_input_report(ble_service_t *svc, uint16_t length, const uint8_t *data)
{
        hid_service_t *hids = (hid_service_t *) svc;
        uint16_t ccc = 0x0000;
        ble_error_t status;

        if (hids->conn_idx == BLE_CONN_IDX_INVALID) {
                return false;
        }

        if (hids->protocol_mode != HIDS_PROTOCOL_MODE_BOOT) {
                return false;
        }

        if (!hids->boot_mouse_input_val_h) {
                return false;
        }

        if (length > BOOT_REPORT_SIZE) {
                return false;
        }

        ble_storage_get_u16(hids->conn_idx, hids->boot_mouse_input_ccc_h, &ccc);
        if (!(ccc & GATT_CCC_NOTIFICATIONS)) {
                return false;
        }

        status = ble_gatts_send_event(hids->conn_idx, hids->boot_mouse_input_val_h,
                                                        GATT_EVENT_NOTIFICATION, length, data);

        return status == BLE_STATUS_OK;
}

bool hids_set_boot_keyboard_input_report(ble_service_t *svc, uint16_t length, const uint8_t *data)
{
        hid_service_t *hids = (hid_service_t *) svc;

        if (!hids->boot_keyboard_input_val_h) {
                return false;
        }

        if (length > BOOT_REPORT_SIZE) {
                return false;
        }

        memcpy(hids->boot_keyboard_input_val, data, length);
        hids->boot_keyboard_input_length = length;

        return true;
}

bool hids_notify_boot_keyboard_input_report(ble_service_t *svc, uint16_t length,
                                                                        const uint8_t *data)
{
        hid_service_t *hids = (hid_service_t *) svc;
        uint16_t ccc = 0x0000;
        ble_error_t status;

        if (hids->conn_idx == BLE_CONN_IDX_INVALID) {
                return false;
        }

        if (hids->protocol_mode != HIDS_PROTOCOL_MODE_BOOT) {
                return false;
        }

        if (!hids->boot_keyboard_input_val_h) {
                return false;
        }

        if (length > BOOT_REPORT_SIZE) {
                return false;
        }

        ble_storage_get_u16(hids->conn_idx, hids->boot_keyboard_input_ccc_h, &ccc);
        if (!(ccc & GATT_CCC_NOTIFICATIONS)) {
                return false;
        }

        status = ble_gatts_send_event(hids->conn_idx, hids->boot_keyboard_input_val_h,
                                                        GATT_EVENT_NOTIFICATION, length, data);

        return status == BLE_STATUS_OK;
}

static report_t *get_report(hid_service_t *hids, hids_report_type_t type, uint8_t report_id)
{
        int i;

        for (i = 0; i < hids->num_reports; i++) {
                if (hids->reports[i].type == type && hids->reports[i].report_id == report_id) {
                        return &hids->reports[i];
                }
        }

        return NULL;
}

bool hids_set_report(ble_service_t *svc, hids_report_type_t type, uint8_t report_id,
                                                        uint16_t length, const uint8_t *data)
{
        hid_service_t *hids = (hid_service_t *) svc;
        report_t *report = NULL;

        report = get_report(hids, type, report_id);
        if (!report) {
                return false;
        }

        if (length > report->length) {
                return false;
        }

        memcpy(report->value, data, length);
        report->cur_length = length;

        return true;
}

bool hids_notify_input_report(ble_service_t *svc, uint8_t report_id, uint16_t length,
                                                                        const uint8_t *data)
{
        hid_service_t *hids = (hid_service_t *) svc;
        report_t *report = NULL;
        uint16_t ccc = 0x0000;
        ble_error_t status;

        if (hids->conn_idx == BLE_CONN_IDX_INVALID) {
                return false;
        }

        if (hids->protocol_mode != HIDS_PROTOCOL_MODE_REPORT) {
                return false;
        }

        report = get_report(hids, HIDS_REPORT_TYPE_INPUT, report_id);
        if (!report) {
                return false;
        }

        ble_storage_get_u16(hids->conn_idx, report->ccc_h, &ccc);
        if (!(ccc & GATT_CCC_NOTIFICATIONS)) {
                return false;
        }

        status = ble_gatts_send_event(hids->conn_idx, report->val_h, GATT_EVENT_NOTIFICATION,
                                                                                length, data);

        return status == BLE_STATUS_OK;
}
