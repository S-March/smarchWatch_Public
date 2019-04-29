/**
 ****************************************************************************************
 *
 * @file hids_client.c
 *
 * @brief HID Service Client implementation
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include "osal.h"
#include "ble_bufops.h"
#include "ble_uuid.h"
#include "ble_gatt.h"
#include "ble_gattc.h"
#include "ble_gattc_util.h"
#include "util/queue.h"
#include "ble_client.h"
#include "hids_client.h"

#define UUID_REPORT_REFERENCE                           (0x2908)
#define UUID_PROTOCOL_MODE                              (0x2A4E)
#define UUID_BOOT_KEYBOARD_INPUT_REPORT                 (0x2A22)
#define UUID_BOOT_KEYBOARD_OUTPUT_REPORT                (0x2A32)
#define UUID_BOOT_MOUSE_INPUT_REPORT                    (0x2A33)
#define UUID_HID_INFORMATION                            (0x2A4A)
#define UUID_HID_CONTROL_POINT                          (0x2A4C)
#define UUID_REPORT_MAP                                 (0x2A4B)
#define UUID_REPORT                                     (0x2A4D)
#define UUID_EXTERNAL_REPORT_REFERENCE                  (0x2907)

typedef enum {
        READ_STATUS_NONE,
        READ_STATUS_TRIGGERED,
        READ_STATUS_DONE,
} read_status_t;

typedef struct {
        void *next;
        hids_client_report_type_t type;
        uint8_t report_id;
        uint8_t prop;
        uint16_t val_h;
        uint16_t ccc_h;
        uint16_t report_ref_h;
        /* Report reference read status */
        read_status_t read_status;
} report_t;

typedef struct {
        void *next;
        uint16_t handle;
        read_status_t read_status;
} external_report_t;

typedef struct {
        ble_client_t client;
        /* Application callbacks */
        const hids_client_callbacks_t *cb;
        /* Protocol Mode */
        hids_client_protocol_mode_t mode;
        /* Boot Keyboard input report writable */
        bool boot_keyboard_input_writable : 1;
        /* Protocol mode value handle */
        uint16_t protocol_mode_val_h;
        /* Control point value handle */
        uint16_t cp_val_h;
        /* Report Map value handle */
        uint16_t report_map_val_h;
        /* Hid Info value handle */
        uint16_t hid_info_val_h;
        /* Boot Keyboard input report value handle */
        uint16_t boot_keyboard_input_val_h;
        /* Boot Keyboard input report CCC descriptor handle */
        uint16_t boot_keyboard_input_ccc_h;
        /* Boot Keyboard output report value handle */
        uint16_t boot_keyboard_output_val_h;
        /* Boot Mouse input report value handle */
        uint16_t boot_mouse_input_val_h;
        /* Boot Mouse input report CCC descriptor handle */
        uint16_t boot_mouse_input_ccc_h;
        /* Report characteristics */
        queue_t reports;
        /* External report descriptors */
        queue_t external_reports;
} hids_client_t;

typedef struct {
        uint8_t type;
        uint8_t report_id;
        uint8_t prop;
        uint16_t val_h;
        uint16_t ccc_h;
        uint16_t report_ref_h;
} __attribute__((packed)) report_serialized_t;

/* Structure used for client serialization */
typedef struct {
        bool boot_keyboard_input_writable;
        uint16_t protocol_mode_val_h;
        uint16_t cp_val_h;
        uint16_t report_map_val_h;
        uint16_t hid_info_val_h;
        uint16_t boot_keyboard_input_val_h;
        uint16_t boot_keyboard_input_ccc_h;
        uint16_t boot_keyboard_output_val_h;
        uint16_t boot_mouse_input_val_h;
        uint16_t boot_mouse_input_ccc_h;
        uint16_t num_reports;
        uint16_t num_external_reports;
} __attribute__((packed)) hids_client_t_serialized_t;

static bool match_report_by_ref_h(const void *data, const void *match_data)
{
        const report_t *report = data;
        const uint16_t *report_ref_h = match_data;

        return report->report_ref_h == *report_ref_h;
}

static bool match_report_by_val_h(const void *data, const void *match_data)
{
        const report_t *report = data;
        const uint16_t *val_h = match_data;

        return report->val_h == *val_h;
}

static bool match_report_by_ccc(const void *data, const void *match_data)
{
        const report_t *report = data;
        const uint16_t *ccc_h = match_data;

        return report->ccc_h == *ccc_h;
}

static bool match_ext_report(const void *data, const void *match_data)
{
        const external_report_t *report = data;
        const uint16_t *handle = match_data;

        return report->handle == *handle;
}

static bool match_ext_report_in_progress(const void *data, const void *match_data)
{
        const external_report_t *report = data;

        return (report->read_status != READ_STATUS_DONE);
}

static bool match_report_in_progress(const void *data, const void *match_data)
{
        const report_t *report = data;

        return (report->read_status != READ_STATUS_DONE);
}
static bool enable_notifications(uint16_t conn_idx, uint16_t handle, bool enable)
{
        ble_error_t status;

        status = ble_gattc_util_write_ccc(conn_idx, handle, enable ? GATT_CCC_NOTIFICATIONS :
                                                                                GATT_CCC_NONE);

        return (status == BLE_STATUS_OK);
}
static void handle_boot_report_value(hids_client_t *hids_client, hids_client_boot_report_type type,
                                        att_error_t status, uint16_t length, const uint8_t *data)
{
        if (hids_client->mode != HIDS_CLIENT_PROTOCOL_MODE_BOOT) {
                return;
        }

        if (hids_client->cb->boot_report) {
                hids_client->cb->boot_report(&hids_client->client, type, status, length, data);
        }
}

static void handle_boot_report_ccc_value(hids_client_t *hids_client,
                                        hids_client_boot_report_type type, att_error_t status,
                                        uint16_t length, const uint8_t *data)
{
        uint16_t ccc;
        bool enabled = false;

        if (status != ATT_ERROR_OK) {
                goto done;
        }

        if (length != sizeof(ccc)) {
                status = ATT_ERROR_INVALID_VALUE_LENGTH;
                goto done;
        }

        ccc = get_u16(data);
        enabled = ccc & GATT_CCC_NOTIFICATIONS;

done:
        if (hids_client->cb->boot_report_get_notif_state) {
                hids_client->cb->boot_report_get_notif_state(&hids_client->client, type, status, enabled);
        }
}

static void handle_hid_info_value(hids_client_t *hids_client, att_error_t status, uint16_t length,
                                                                        const uint8_t *data)
{
        hids_client_hid_info_t hid_info;

        if (!hids_client->cb->hid_info) {
                return;
        }

        memset(&hid_info, 0, sizeof(hid_info));

        if (status != ATT_ERROR_OK) {
                goto done;
        }

        /* HID Information should contain 4 bytes */
        if (length != 4) {
                status = ATT_ERROR_INVALID_VALUE_LENGTH;
                goto done;
        }

        hid_info.bcd_hid = get_u16(data);
        hid_info.country_code = get_u8(&data[2]);
        hid_info.flags = get_u8(&data[3]);

done:
        hids_client->cb->hid_info(&hids_client->client, &hid_info);
}

static void handle_report_map_value(hids_client_t *hids_client, att_error_t status,
                                                        uint16_t length, const uint8_t *data)
{
        if (hids_client->cb->report_map) {
                hids_client->cb->report_map(&hids_client->client, status, length, data);
        }
}

static void handle_report_value(hids_client_t *hids_client, report_t *report, att_error_t status,
                                                        uint16_t length, const uint8_t *data)
{
        if (hids_client->mode != HIDS_CLIENT_PROTOCOL_MODE_REPORT) {
                return;
        }

        if (hids_client->cb->report) {
                hids_client->cb->report(&hids_client->client, report->type, report->report_id,
                                                                        status,length, data);
        }
}

static void set_report_read_status(void *elem, void *ud)
{
        report_t *report = elem;
        read_status_t read_status = (read_status_t) ud;

        report->read_status = read_status;
}

static void check_reports_discovery_complete(hids_client_t *hids_client)
{
        if (!hids_client->cb->discover_reports_complete) {
                return;
        }

        if (!queue_find(&hids_client->reports, match_report_in_progress, NULL)) {
                hids_client->cb->discover_reports_complete(&hids_client->client);
                queue_foreach(&hids_client->reports, set_report_read_status,
                                                                        (void *) READ_STATUS_NONE);
        }
}

static void handle_report_ref_value(hids_client_t *hids_client, report_t *report,
                                        att_error_t status, uint16_t length, const uint8_t *data)
{
        if (report->read_status == READ_STATUS_TRIGGERED) {
                report->read_status = READ_STATUS_DONE;
        }

        if (status != ATT_ERROR_OK) {
                goto done;
        }

        /* Report reference descriptor length shall be 2 bytes */
        if (length != 2) {
                status = ATT_ERROR_INVALID_VALUE_LENGTH;
                goto done;
        }

        report->report_id = data[0];
        report->type = data[1];

done:
        if (hids_client->cb->report_found) {
                hids_client->cb->report_found(&hids_client->client, status, report->type,
                                                                        report->report_id);
        }

        check_reports_discovery_complete(hids_client);
}

static void set_external_report_read_status(void *elem, void *ud)
{
        external_report_t *report = elem;
        read_status_t read_status = (read_status_t) ud;

        report->read_status = read_status;
}

static void check_external_reports_discovery_complete(hids_client_t *hids_client)
{
        if (!hids_client->cb->discover_external_reports_complete) {
                return;
        }

        if (!queue_find(&hids_client->external_reports, match_ext_report_in_progress, NULL)) {
                hids_client->cb->discover_external_reports_complete(&hids_client->client);
                queue_foreach(&hids_client->external_reports, set_external_report_read_status,
                                                                        (void *) READ_STATUS_NONE);
        }
}

static void handle_external_report_ref_value(hids_client_t *hids_client, external_report_t *report,
                                        att_error_t status, uint16_t length, const uint8_t *data)
{
        att_uuid_t uuid;
        uint16_t uuid16;

        if (report->read_status == READ_STATUS_TRIGGERED) {
                report->read_status = READ_STATUS_DONE;
        }

        if (!hids_client->cb->external_report_found) {
                check_external_reports_discovery_complete(hids_client);
                return;
        }

        memset(&uuid, 0, sizeof(uuid));

        if (status != ATT_ERROR_OK) {
                goto done;
        }

        if (length == sizeof(uuid16)) {
                uuid16 = get_u16(data);
                ble_uuid_create16(uuid16, &uuid);
        } else if (length == 16) {
                ble_uuid_from_buf(data, &uuid);
        } else {
                status = ATT_ERROR_INVALID_VALUE_LENGTH;
                goto done;
        }

done:
        hids_client->cb->external_report_found(&hids_client->client, status, &uuid);
        check_external_reports_discovery_complete(hids_client);
}

static void handle_report_ccc_value(hids_client_t *hids_client, report_t *report,
                                        att_error_t status, uint16_t length, const uint8_t *data)
{
        uint16_t ccc;
        bool enabled = false;

        if (!hids_client->cb->input_report_get_notif_state) {
                return;
        }

        if (status != ATT_ERROR_OK) {
                goto done;
        }

        if (length != sizeof(ccc)) {
                status = ATT_ERROR_INVALID_VALUE_LENGTH;
                goto done;
        }

        ccc = get_u16(data);
        enabled = ccc & GATT_CCC_NOTIFICATIONS;

done:
        hids_client->cb->input_report_get_notif_state(&hids_client->client, report->report_id, status,
                                                                                        enabled);
}

static void handle_disconnected_evt(ble_client_t *client, const ble_evt_gap_disconnected_t *evt)
{
        hids_client_t *hids_client = (hids_client_t *) client;

        queue_foreach(&hids_client->external_reports, set_external_report_read_status,
                                                                        (void *) READ_STATUS_NONE);
        queue_foreach(&hids_client->reports, set_report_read_status, (void *) READ_STATUS_NONE);

        client->conn_idx = BLE_CONN_IDX_INVALID;

        ble_client_remove(client);
}

static void handle_notification_evt(ble_client_t *client, const ble_evt_gattc_notification_t *evt)
{
        hids_client_t *hids_client = (hids_client_t *) client;
        uint16_t handle = evt->handle;

        if (handle == hids_client->boot_keyboard_input_val_h) {
                handle_boot_report_value(hids_client, HIDS_CLIENT_BOOT_KEYBOARD_INPUT,
                                                        ATT_ERROR_OK, evt->length, evt->value);
        } else if (handle == hids_client->boot_mouse_input_val_h) {
                handle_boot_report_value(hids_client, HIDS_CLIENT_BOOT_MOUSE_INPUT, ATT_ERROR_OK,
                                                                        evt->length, evt->value);
        } else {
                report_t *report;

                report = queue_find(&hids_client->reports, match_report_by_val_h, &handle);
                if (report) {
                        handle_report_value(hids_client, report, ATT_ERROR_OK, evt->length,
                                                                                evt->value);
                }
        }
}

static void handle_protocol_mode_value(hids_client_t *hids_client, att_error_t status,
                                                        uint16_t length, const uint8_t *data)
{
        hids_client_protocol_mode_t mode = HIDS_CLIENT_PROTOCOL_MODE_BOOT;

        if (status != ATT_ERROR_OK) {
                goto done;
        }

        if (length != sizeof(uint8_t)) {
                status = ATT_ERROR_INVALID_VALUE_LENGTH;
                goto done;
        }

        mode = get_u8(data);

done:
        if (hids_client->cb->get_protocol_mode) {
                hids_client->cb->get_protocol_mode(&hids_client->client, status, mode);
        }
}

static void handle_read_completed_evt(ble_client_t *client,
                                                        const ble_evt_gattc_read_completed_t *evt)
{
        hids_client_t *hids_client = (hids_client_t *) client;
        uint16_t handle = evt->handle;

        if (handle == hids_client->protocol_mode_val_h) {
                handle_protocol_mode_value(hids_client, evt->status, evt->length, evt->value);
        } else if (handle == hids_client->boot_keyboard_input_val_h) {
                handle_boot_report_value(hids_client, HIDS_CLIENT_BOOT_KEYBOARD_INPUT, evt->status,
                                                                        evt->length, evt->value);
        } else if (handle == hids_client->boot_keyboard_input_ccc_h) {
                handle_boot_report_ccc_value(hids_client, HIDS_CLIENT_BOOT_KEYBOARD_INPUT, evt->status,
                                                                        evt->length, evt->value);
        } else if (handle == hids_client->boot_keyboard_output_val_h) {
                handle_boot_report_value(hids_client, HIDS_CLIENT_BOOT_KEYBOARD_OUTPUT,
                                                        evt->status, evt->length, evt->value);
        } else if (handle == hids_client->boot_mouse_input_val_h) {
                handle_boot_report_value(hids_client, HIDS_CLIENT_BOOT_MOUSE_INPUT, evt->status,
                                                                        evt->length, evt->value);
        } else if (handle == hids_client->boot_mouse_input_ccc_h) {
                handle_boot_report_ccc_value(hids_client, HIDS_CLIENT_BOOT_MOUSE_INPUT, evt->status,
                                                                        evt->length, evt->value);
        } else if (handle == hids_client->hid_info_val_h) {
                handle_hid_info_value(hids_client, evt->status, evt->length, evt->value);
        } else if (handle == hids_client->report_map_val_h) {
                handle_report_map_value(hids_client, evt->status, evt->length, evt->value);
        } else {
                external_report_t *external_report;
                report_t *report;

                report = queue_find(&hids_client->reports, match_report_by_ref_h, &handle);
                if (report) {
                        handle_report_ref_value(hids_client, report, evt->status, evt->length,
                                                                                evt->value);
                        return;
                }

                report = queue_find(&hids_client->reports, match_report_by_val_h, &handle);
                if (report) {
                        handle_report_value(hids_client, report, evt->status, evt->length, evt->value);
                        return;
                }

                report = queue_find(&hids_client->reports, match_report_by_ccc, &handle);
                if (report) {
                        handle_report_ccc_value(hids_client, report, evt->status, evt->length, evt->value);
                        return;
                }

                external_report = queue_find(&hids_client->external_reports, match_ext_report,
                                                                                        &handle);
                if (external_report) {
                        handle_external_report_ref_value(hids_client, external_report, evt->status,
                                                                        evt->length, evt->value);
                        return;
                }
        }
}

static void handle_write_completed_evt(ble_client_t *client,
                                                        const ble_evt_gattc_write_completed_t *evt)
{
        hids_client_t *hids_client = (hids_client_t *) client;
        att_error_t status = evt->status;
        uint16_t handle = evt->handle;

        if (handle == hids_client->protocol_mode_val_h) {
                if (hids_client->cb->set_protocol_mode) {
                        hids_client->cb->set_protocol_mode(client, status);
                }
        }

        if (handle == hids_client->boot_keyboard_input_ccc_h) {
                if (hids_client->cb->boot_report_set_notif_state) {
                        hids_client->cb->boot_report_set_notif_state(client,
                                                        HIDS_CLIENT_BOOT_KEYBOARD_INPUT, status);
                }
        } else if (handle == hids_client->boot_mouse_input_ccc_h) {
                if (hids_client->cb->boot_report_set_notif_state) {
                        hids_client->cb->boot_report_set_notif_state(client,
                                                        HIDS_CLIENT_BOOT_MOUSE_INPUT, status);
                }
        } else {
                report_t *report;

                report = queue_find(&hids_client->reports, match_report_by_ccc, &handle);
                if (report) {
                        if (hids_client->cb->input_report_set_notif_state) {
                                hids_client->cb->input_report_set_notif_state(client,
                                                                                report->report_id,
                                                                                status);
                        }
                        return;
                }

                report = queue_find(&hids_client->reports, match_report_by_val_h, &handle);
                if (report) {
                        if (hids_client->cb->report_write_complete) {
                                hids_client->cb->report_write_complete(client, report->type,
                                                                        report->report_id, status);
                        }
                        return;
                }
        }
}

static bool verify_hids_client(hids_client_t *hids_client)
{
        if (!hids_client->report_map_val_h) {
                return false;
        }

        if (hids_client->mode == HIDS_CLIENT_PROTOCOL_MODE_BOOT) {
                if (!hids_client->protocol_mode_val_h) {
                        return false;
                }
        } else {
                if (!hids_client->cp_val_h) {
                        return false;
                }

                if (!hids_client->hid_info_val_h) {
                        return false;
                }
        }

        return true;
}

static const gattc_item_t *find_char(uint16_t uuid16)
{
        att_uuid_t uuid;

        ble_uuid_create16(uuid16, &uuid);

        return ble_gattc_util_find_characteristic(&uuid);
}

static const gattc_item_t *find_desc(uint16_t uuid16)
{
        att_uuid_t uuid;

        ble_uuid_create16(uuid16, &uuid);

        return ble_gattc_util_find_descriptor(&uuid);
}

static void cleanup(ble_client_t *client)
{
        hids_client_t *hids_client = (hids_client_t *) client;

        queue_remove_all(&hids_client->reports, OS_FREE_FUNC);
        queue_remove_all(&hids_client->external_reports, OS_FREE_FUNC);
        OS_FREE(hids_client);
}


static void serialize(ble_client_t *client, void *data, size_t *length)
{
        hids_client_t *hids_client = (hids_client_t *) client;
        hids_client_t_serialized_t *s_data = data;
        report_t *rep;
        external_report_t *ext_rep;
        report_serialized_t *s_rep;
        uint16_t *s_ext_rep;
        int i;

        *length = sizeof(hids_client_t_serialized_t) + hids_client->reports.size *
                sizeof(report_serialized_t) + hids_client->external_reports.size * sizeof(uint16_t);

        if (!data) {
                return;
        }

        s_data->boot_keyboard_input_writable = hids_client->boot_keyboard_input_writable;
        s_data->protocol_mode_val_h = hids_client->protocol_mode_val_h;
        s_data->cp_val_h = hids_client->cp_val_h;
        s_data->report_map_val_h = hids_client->report_map_val_h;
        s_data->hid_info_val_h = hids_client->hid_info_val_h;
        s_data->boot_keyboard_input_val_h = hids_client->boot_keyboard_input_val_h;
        s_data->boot_keyboard_input_ccc_h = hids_client->boot_keyboard_input_ccc_h;
        s_data->boot_keyboard_output_val_h = hids_client->boot_keyboard_output_val_h;
        s_data->boot_mouse_input_val_h = hids_client->boot_mouse_input_val_h;
        s_data->boot_mouse_input_ccc_h = hids_client->boot_mouse_input_ccc_h;
        s_data->num_reports = hids_client->reports.size;
        s_data->num_external_reports = hids_client->external_reports.size;

        /* Serialize reports */
        s_rep = (report_serialized_t *) (((uint8_t *) data) + sizeof(hids_client_t_serialized_t));
        rep = (report_t *) hids_client->reports.head;

        for (i = 0; i < s_data->num_reports; i++) {
                if (rep == NULL) {
                        break;
                }

                s_rep->type = rep->type;
                s_rep->report_id = rep->report_id;
                s_rep->prop = rep->prop;
                s_rep->val_h = rep->val_h;
                s_rep->ccc_h = rep->ccc_h;
                s_rep->report_ref_h = rep->report_ref_h;
                rep = (report_t *) rep->next;
                s_rep++;
        }

        /* Serialize external reports */
        s_ext_rep = (uint16_t *) s_rep;
        ext_rep = (external_report_t *) hids_client->external_reports.head;

        for (i = 0; i < s_data->num_external_reports; i++) {
                if (rep == NULL) {
                        break;
                }

                *s_ext_rep = ext_rep->handle;
                ext_rep = (external_report_t *) ext_rep->next;
                s_ext_rep++;
        }
}

ble_client_t *hids_client_init(const hids_client_config_t *config,
                                                        const hids_client_callbacks_t *cb,
                                                        const ble_evt_gattc_browse_svc_t *evt)
{
        hids_client_t *hids_client;
        const gattc_item_t *item;
        att_uuid_t uuid;

        if (!cb) {
                return NULL;
        }

        ble_uuid_create16(UUID_SERVICE_HIDS, &uuid);
        if (!ble_uuid_equal(&uuid, &evt->uuid)) {
                return NULL;
        }

        hids_client = OS_MALLOC(sizeof(*hids_client));
        memset(hids_client, 0, sizeof(*hids_client));

        queue_init(&hids_client->reports);
        queue_init(&hids_client->external_reports);

        ble_gattc_util_find_init(evt);

        /* Protocol Mode Characteristic */
        item = find_char(UUID_PROTOCOL_MODE);
        if (item && (item->c.properties & GATT_PROP_READ) &&
                                                (item->c.properties & GATT_PROP_WRITE_NO_RESP)) {
                hids_client->protocol_mode_val_h = item->c.value_handle;
        }

        /* Control Point Characteristic */
        item = find_char(UUID_HID_CONTROL_POINT);
        if (item && (item->c.properties & GATT_PROP_WRITE_NO_RESP)) {
                hids_client->cp_val_h = item->c.value_handle;
        }

        /* HID Info Characteristic */
        item = find_char(UUID_HID_INFORMATION);
        if (item && (item->c.properties & GATT_PROP_READ)) {
                hids_client->hid_info_val_h = item->c.value_handle;
        }

        /* Report Map */
        item = find_char(UUID_REPORT_MAP);
        if (item && (item->c.properties & GATT_PROP_READ)) {
                hids_client->report_map_val_h = item->c.value_handle;

                /* Discover External Report Descriptors */
                do {
                        item = find_desc(UUID_EXTERNAL_REPORT_REFERENCE);

                        if (item) {
                                external_report_t *report;

                                report = OS_MALLOC(sizeof(*report));
                                report->handle = item->handle;

                                queue_push_back(&hids_client->external_reports, report);
                        }
                } while (item);
        }

        /* Boot Mouse Input Report */
        item = find_char(UUID_BOOT_MOUSE_INPUT_REPORT);
        if (item && (item->c.properties & GATT_PROP_READ) &&
                                                        (item->c.properties & GATT_PROP_NOTIFY)) {
                hids_client->boot_mouse_input_val_h = item->c.value_handle;

                item = find_desc(UUID_GATT_CLIENT_CHAR_CONFIGURATION);
                if (item) {
                        hids_client->boot_mouse_input_ccc_h = item->handle;
                }
        }

        /* Boot Keyboard Input Report */
        item = find_char(UUID_BOOT_KEYBOARD_INPUT_REPORT);
        if (item && (item->c.properties & GATT_PROP_READ) &&
                                                        (item->c.properties & GATT_PROP_NOTIFY)) {
                hids_client->boot_keyboard_input_val_h = item->c.value_handle;

                if (item->c.properties & GATT_PROP_WRITE) {
                        hids_client->boot_keyboard_input_writable = true;
                }

                item = find_desc(UUID_GATT_CLIENT_CHAR_CONFIGURATION);
                if (item) {
                        hids_client->boot_keyboard_input_ccc_h = item->handle;
                }
        }

        /* Boot Keyboard Output Report */
        item = find_char(UUID_BOOT_KEYBOARD_OUTPUT_REPORT);
        if (item && (item->c.properties & GATT_PROP_READ) &&
                                        (item->c.properties & GATT_PROP_WRITE_NO_RESP) &&
                                        (item->c.properties & GATT_PROP_WRITE)) {
                hids_client->boot_keyboard_output_val_h = item->c.value_handle;
        }

        /* Report Characteristic */
        for (item = find_char(UUID_REPORT); item; item = find_char(UUID_REPORT)) {
                report_t *report = OS_MALLOC(sizeof(*report));

                report->val_h = item->c.value_handle;
                report->prop = item->c.properties;

                item = find_desc(UUID_GATT_CLIENT_CHAR_CONFIGURATION);
                if (item) {
                        report->ccc_h = item->handle;
                }

                item = find_desc(UUID_REPORT_REFERENCE);
                if (item) {
                        report->report_ref_h = item->handle;
                }

                queue_push_back(&hids_client->reports, report);
        }

        hids_client->client.conn_idx = evt->conn_idx;
        hids_client->client.write_completed_evt = handle_write_completed_evt;
        hids_client->client.read_completed_evt = handle_read_completed_evt;
        hids_client->client.notification_evt = handle_notification_evt;
        hids_client->client.disconnected_evt = handle_disconnected_evt;
        hids_client->client.cleanup = cleanup;
        hids_client->client.serialize = serialize;
        hids_client->mode = config->mode;
        hids_client->cb = cb;

        /* Check if mandatory characteristics were found */
        if (!verify_hids_client(hids_client)) {
                cleanup(&hids_client->client);
                return NULL;
        }

        return &hids_client->client;
}

ble_client_t *hids_client_init_from_data(uint16_t conn_idx, const hids_client_config_t *config,
                                const hids_client_callbacks_t *cb,const void *data, size_t length)
{
        int i;
        hids_client_t *hids_client;
        const hids_client_t_serialized_t *s_data = data;
        report_serialized_t *s_rep;
        uint16_t *s_ext_rep;

        if (!data || (length < sizeof(hids_client_t_serialized_t))) {
                return NULL;
        }

        if (length < sizeof(hids_client_t_serialized_t) + s_data->num_reports *
                sizeof(report_serialized_t) + s_data->num_external_reports * sizeof(uint16_t)) {
                return NULL;
        }

        hids_client = OS_MALLOC(sizeof(*hids_client));
        memset(hids_client, 0, sizeof(*hids_client));

        queue_init(&hids_client->reports);
        queue_init(&hids_client->external_reports);

        hids_client->boot_keyboard_input_writable = s_data->boot_keyboard_input_writable;
        hids_client->protocol_mode_val_h = s_data->protocol_mode_val_h;
        hids_client->cp_val_h = s_data->cp_val_h;
        hids_client->report_map_val_h = s_data->report_map_val_h;
        hids_client->hid_info_val_h = s_data->hid_info_val_h;
        hids_client->boot_keyboard_input_val_h = s_data->boot_keyboard_input_val_h;
        hids_client->boot_keyboard_input_ccc_h = s_data->boot_keyboard_input_ccc_h;
        hids_client->boot_keyboard_output_val_h = s_data->boot_keyboard_output_val_h;
        hids_client->boot_mouse_input_val_h = s_data->boot_mouse_input_val_h;
        hids_client->boot_mouse_input_ccc_h = s_data->boot_mouse_input_ccc_h;

        s_rep = (report_serialized_t *) (((uint8_t *) data) + sizeof(hids_client_t_serialized_t));

        for (i = 0; i < s_data->num_reports; i++) {
                report_t *report = OS_MALLOC(sizeof(*report));

                report->type = s_rep->type;
                report->report_id = s_rep->report_id;
                report->prop = s_rep->prop;
                report->val_h = s_rep->val_h;
                report->ccc_h = s_rep->ccc_h;
                report->report_ref_h = s_rep->report_ref_h;
                report->read_status = READ_STATUS_NONE;
                queue_push_back(&hids_client->reports, report);

                ++s_rep;
        }

        s_ext_rep = (uint16_t *) s_rep;

        for (i = 0; i < s_data->num_external_reports; i++) {
                external_report_t *report = OS_MALLOC(sizeof(*report));

                report->handle = *s_ext_rep;
                report->read_status = READ_STATUS_NONE;
                queue_push_back(&hids_client->external_reports, report);

                ++s_ext_rep;
        }

        hids_client->client.conn_idx = conn_idx;
        hids_client->client.write_completed_evt = handle_write_completed_evt;
        hids_client->client.read_completed_evt = handle_read_completed_evt;
        hids_client->client.notification_evt = handle_notification_evt;
        hids_client->client.disconnected_evt = handle_disconnected_evt;
        hids_client->client.cleanup = cleanup;
        hids_client->client.serialize = serialize;
        hids_client->mode = config->mode;
        hids_client->cb = cb;

        ble_client_attach(&hids_client->client, conn_idx);

        return &hids_client->client;
}

bool hids_client_boot_report_read(ble_client_t *client, hids_client_boot_report_type type)
{
        hids_client_t *hids_client = (hids_client_t *) client;
        ble_error_t status;
        uint16_t handle;

        if (hids_client->mode != HIDS_CLIENT_PROTOCOL_MODE_BOOT) {
                return false;
        }

        switch (type) {
        case HIDS_CLIENT_BOOT_MOUSE_INPUT:
                handle = hids_client->boot_mouse_input_val_h;
                break;
        case HIDS_CLIENT_BOOT_KEYBOARD_INPUT:
                handle = hids_client->boot_keyboard_input_val_h;
                break;
        case HIDS_CLIENT_BOOT_KEYBOARD_OUTPUT:
                handle = hids_client->boot_keyboard_output_val_h;
                break;
        default:
                handle = 0;
        }

        if (handle == 0) {
                return false;
        }

        status = ble_gattc_read(hids_client->client.conn_idx, handle, 0);

        return (status == BLE_STATUS_OK);
}

bool hids_client_boot_report_write(ble_client_t *client, hids_client_boot_report_type type,
                                        bool response, uint16_t length, const uint8_t *data)
{
        hids_client_t *hids_client = (hids_client_t *) client;
        ble_error_t status;
        uint16_t handle;

        if (hids_client->mode != HIDS_CLIENT_PROTOCOL_MODE_BOOT) {
                return false;
        }

        switch (type) {
        case HIDS_CLIENT_BOOT_MOUSE_INPUT:
                handle = hids_client->boot_mouse_input_val_h;
                response = true;
                break;
        case HIDS_CLIENT_BOOT_KEYBOARD_INPUT:
                if (hids_client->boot_keyboard_input_writable) {
                        handle = hids_client->boot_keyboard_input_val_h;
                        response = true;
                } else {
                        handle = 0;
                }
                break;
        case HIDS_CLIENT_BOOT_KEYBOARD_OUTPUT:
                handle = hids_client->boot_keyboard_output_val_h;
                break;
        default:
                handle = 0;
        }

        if (handle == 0) {
                return false;
        }

        if (response) {
                status = ble_gattc_write(hids_client->client.conn_idx, handle, 0, length, data);
        } else {
                status = ble_gattc_write_no_resp(hids_client->client.conn_idx, handle, false,
                                                                                length, data);
        }

        return (status == BLE_STATUS_OK);
}

static bool match_report_by_type_id(const void *data, const void *match_data)
{
        const report_t *report = data;
        const report_t *match_report = match_data;

        if (report->report_id != match_report->report_id) {
                return false;
        }

        if (report->type != match_report->type) {
                return false;
        }

        return true;
}

static report_t *find_report(hids_client_t *hids_client, hids_client_report_type_t type,
                                                                        uint8_t report_id)
{
        report_t report;

        report.type = type;
        report.report_id = report_id;

        return queue_find(&hids_client->reports, match_report_by_type_id, &report);
}

bool hids_client_report_write(ble_client_t *client, hids_client_report_type_t type,
                                        uint8_t report_id, bool response, uint16_t length,
                                        const uint8_t *data)
{
        hids_client_t *hids_client = (hids_client_t *) client;
        ble_error_t status;
        report_t *report;
        uint16_t handle;

        if (hids_client->mode != HIDS_CLIENT_PROTOCOL_MODE_REPORT) {
                return false;
        }

        report = find_report(hids_client, type, report_id);
        if (!report) {
                return false;
        }

        switch (type) {
        case HIDS_CLIENT_REPORT_TYPE_INPUT:
                if (report->prop & GATT_PROP_WRITE) {
                        handle = report->val_h;
                        response = true;
                } else {
                        handle = 0;
                }
                break;
        case HIDS_CLIENT_REPORT_TYPE_FEATURE:
                handle = report->val_h;
                response = true;
                break;
        case HIDS_CLIENT_REPORT_TYPE_OUTPUT:
                handle = report->val_h;
                break;
        default:
                handle = 0;
        }

        if (handle == 0) {
                return false;
        }

        if (response) {
                status = ble_gattc_write(hids_client->client.conn_idx, handle, 0, length, data);
        } else {
                status = ble_gattc_write_no_resp(hids_client->client.conn_idx, handle, false,
                                                                                length, data);
        }

        return (status == BLE_STATUS_OK);
}

bool hids_client_report_read(ble_client_t *client, hids_client_report_type_t type,
                                                                                uint8_t report_id)
{
        hids_client_t *hids_client = (hids_client_t *) client;
        ble_error_t status;
        report_t *report;

        if (hids_client->mode != HIDS_CLIENT_PROTOCOL_MODE_REPORT) {
                return false;
        }

        report = find_report(hids_client, type, report_id);
        if (!report) {
                return false;
        }

        status = ble_gattc_read(hids_client->client.conn_idx, report->val_h, 0);

        return (status == BLE_STATUS_OK);
}

bool hids_client_cp_command(ble_client_t *client, hids_client_cp_command_t command)
{
        hids_client_t *hids_client = (hids_client_t *) client;
        ble_error_t status;
        uint8_t value;

        if (hids_client->mode != HIDS_CLIENT_PROTOCOL_MODE_REPORT) {
                return false;
        }

        if (command != HIDS_CLIENT_CONTROL_POINT_SUSPEND &&
                                        command != HIDS_CLIENT_CONTROL_POINT_EXIT_SUSPEND) {
                return false;
        }

        value = command;

        status = ble_gattc_write_no_resp(hids_client->client.conn_idx, hids_client->cp_val_h,
                                                                false, sizeof(value), &value);

        return (status == BLE_STATUS_OK);
}

bool hids_client_input_report_set_notif_state(ble_client_t *client, uint8_t report_id, bool enable)
{
        hids_client_t *hids_client = (hids_client_t *) client;
        report_t *report;

        if (hids_client->mode != HIDS_CLIENT_PROTOCOL_MODE_REPORT) {
                return false;
        }

        report = find_report(hids_client, HIDS_CLIENT_REPORT_TYPE_INPUT, report_id);
        if (!report) {
                return false;
        }

        return enable_notifications(client->conn_idx, report->ccc_h, enable);
}

bool hids_client_input_report_get_notif_state(ble_client_t *client, uint8_t report_id)
{
        hids_client_t *hids_client = (hids_client_t *) client;
        ble_error_t status;
        report_t *report;

        if (hids_client->mode != HIDS_CLIENT_PROTOCOL_MODE_REPORT) {
                return false;
        }

        report = find_report(hids_client, HIDS_CLIENT_REPORT_TYPE_INPUT, report_id);
        if (!report) {
                return false;
        }

        status = ble_gattc_read(client->conn_idx, report->ccc_h, 0x00);

        return (status == BLE_STATUS_OK);
}

bool hids_client_boot_report_set_notif_state(ble_client_t *client,
                                                hids_client_boot_report_type type, bool enable)
{
        hids_client_t *hids_client = (hids_client_t *) client;
        uint16_t handle;

        if (hids_client->mode != HIDS_CLIENT_PROTOCOL_MODE_BOOT) {
                return false;
        }

        switch (type) {
        case HIDS_CLIENT_BOOT_MOUSE_INPUT:
                handle = hids_client->boot_mouse_input_ccc_h;
                break;
        case HIDS_CLIENT_BOOT_KEYBOARD_INPUT:
                handle = hids_client->boot_keyboard_input_ccc_h;
                break;
        case HIDS_CLIENT_BOOT_KEYBOARD_OUTPUT:
        default:
                handle = 0;
        }

        if (handle == 0) {
                return false;
        }

        return enable_notifications(client->conn_idx, handle, enable);
}

bool hids_client_boot_report_get_notif_state(ble_client_t *client,
                                                                hids_client_boot_report_type type)
{
        hids_client_t *hids_client = (hids_client_t *) client;
        ble_error_t status;
        uint16_t handle;

        if (hids_client->mode != HIDS_CLIENT_PROTOCOL_MODE_BOOT) {
                return false;
        }

        switch (type) {
        case HIDS_CLIENT_BOOT_MOUSE_INPUT:
                handle = hids_client->boot_mouse_input_ccc_h;
                break;
        case HIDS_CLIENT_BOOT_KEYBOARD_INPUT:
                handle = hids_client->boot_keyboard_input_ccc_h;
                break;
        case HIDS_CLIENT_BOOT_KEYBOARD_OUTPUT:
        default:
                handle = 0;
        }

        if (handle == 0) {
                return false;
        }

        status = ble_gattc_read(client->conn_idx, handle, 0x00);

        return (status == BLE_STATUS_OK);
}

bool hids_client_get_protocol_mode(ble_client_t *client)
{
        hids_client_t *hids_client = (hids_client_t *) client;
        ble_error_t status;

        if (hids_client->protocol_mode_val_h == 0) {
                return false;
        }

        status = ble_gattc_read(client->conn_idx, hids_client->protocol_mode_val_h, 0x00);

        return (status == BLE_STATUS_OK);
}

bool hids_client_read_hid_info(ble_client_t *client)
{
        hids_client_t *hids_client = (hids_client_t *) client;
        ble_error_t status;

        if (hids_client->mode != HIDS_CLIENT_PROTOCOL_MODE_REPORT) {
                return false;
        }

        if (hids_client->hid_info_val_h == 0) {
                return false;
        }

        status = ble_gattc_read(client->conn_idx, hids_client->hid_info_val_h, 0x00);

        return (status == BLE_STATUS_OK);
}

bool hids_client_read_report_map(ble_client_t *client)
{
        hids_client_t *hids_client = (hids_client_t *) client;
        ble_error_t status;

        if (hids_client->mode != HIDS_CLIENT_PROTOCOL_MODE_REPORT) {
                return false;
        }

        if (hids_client->report_map_val_h == 0) {
                return false;
        }

        status = ble_gattc_read(client->conn_idx, hids_client->report_map_val_h, 0x00);

        return (status == BLE_STATUS_OK);
}

struct read_data {
        hids_client_t *hids_client;
        ble_error_t status;
};

static void read_external_report(void *elem, void *ud)
{
        struct read_data *read_data = ud;
        hids_client_t *hids_client = read_data->hids_client;
        external_report_t *report = elem;

        /* Stop reading if previous read failed */
        if (read_data->status != BLE_STATUS_OK) {
                return;
        }

        read_data->status = ble_gattc_read(hids_client->client.conn_idx, report->handle, 0);
        if (read_data->status == BLE_STATUS_OK) {
                report->read_status = READ_STATUS_TRIGGERED;
        }
}

bool hids_client_discover_external_reports(ble_client_t *client)
{
        hids_client_t *hids_client = (hids_client_t *) client;
        struct read_data read_data = {
                .hids_client = hids_client,
                .status = BLE_STATUS_OK,
        };

        if (hids_client->mode != HIDS_CLIENT_PROTOCOL_MODE_REPORT) {
                return false;
        }

        /* Read external report reference descriptors */
        queue_foreach(&hids_client->external_reports, read_external_report, &read_data);

        if (read_data.status == BLE_STATUS_OK) {
                check_external_reports_discovery_complete(hids_client);
                return true;
        }

        queue_foreach(&hids_client->external_reports, set_external_report_read_status,
                                                                        (void *) READ_STATUS_NONE);

        return false;
}

static void read_report_reference_match(void *elem, void *ud)
{
        struct read_data *read_data = ud;
        hids_client_t *hids_client = read_data->hids_client;
        report_t *report = elem;

        /* Stop reading if previous read failed */
        if (read_data->status != BLE_STATUS_OK) {
                return;
        }

        if (!report->report_ref_h) {
                return;
        }

        read_data->status = ble_gattc_read(hids_client->client.conn_idx, report->report_ref_h, 0);
        if (read_data->status == BLE_STATUS_OK) {
                report->read_status = READ_STATUS_TRIGGERED;
        }
}

bool hids_client_discover_reports(ble_client_t *client)
{
        hids_client_t *hids_client = (hids_client_t *) client;
        struct read_data read_data = {
                .hids_client = hids_client,
                .status = BLE_STATUS_OK,
        };

        if (hids_client->mode != HIDS_CLIENT_PROTOCOL_MODE_REPORT) {
                return false;
        }

        queue_foreach(&hids_client->reports, read_report_reference_match, &read_data);

        if (read_data.status == BLE_STATUS_OK) {
                check_reports_discovery_complete(hids_client);
                return true;
        }

        queue_foreach(&hids_client->external_reports, set_report_read_status,
                                                                        (void *) READ_STATUS_NONE);

        return false;
}

hids_client_cap_t hids_client_get_capabilities(ble_client_t *client)
{
        hids_client_t *hids_client = (hids_client_t *) client;

        hids_client_cap_t capabilities = 0;

        if (hids_client->boot_keyboard_input_val_h) {
                capabilities |= HIDS_CLIENT_CAP_BOOT_KEYBOARD_INPUT;
        }

        if (hids_client->boot_keyboard_output_val_h) {
                capabilities |= HIDS_CLIENT_CAP_BOOT_KEYBOARD_OUTPUT;
        }

        if (hids_client->boot_mouse_input_val_h) {
                capabilities |= HIDS_CLIENT_CAP_BOOT_MOUSE_INPUT;
        }

        if (hids_client->protocol_mode_val_h) {
                capabilities |= HIDS_CLIENT_CAP_PROTOCOL_MODE;
        }

        if (hids_client->hid_info_val_h) {
                capabilities |= HIDS_CLIENT_CAP_HID_INFO;
        }

        if (hids_client->cp_val_h) {
                capabilities |= HIDS_CLIENT_CAP_HID_CONTROL_POINT;
        }

        if (hids_client->report_map_val_h) {
                capabilities |= HIDS_CLIENT_CAP_REPORT_MAP;
        }

        return capabilities;
}

struct dump_report_data {
        hids_client_dump_service_data_cb_t cb;
        ble_client_t *client;
};

static const char *report_type_to_str(hids_client_report_type_t type)
{
        switch (type) {
        case HIDS_CLIENT_REPORT_TYPE_INPUT:
                return "REPORT TYPE INPUT";
        case HIDS_CLIENT_REPORT_TYPE_FEATURE:
                return "REPORT TYPE FEATURE";
        case HIDS_CLIENT_REPORT_TYPE_OUTPUT:
                return "REPORT TYPE OUTPUT";
        default:
                break;
        }

        return "UNKNOWN REPORT TYPE";
}

static void dump_report(void *elem, void *ud)
{
        struct dump_report_data *report_data = ud;
        report_t *report = elem;

        report_data->cb(report_data->client, "Report type:%s, Report id:0x%02X, properties:0x%02X,"
                        "value handle:0x%04X, report reference handle:0x%04X, ccc_h:0x%04X",
                        report_type_to_str(report->type), report->report_id, report->prop,
                        report->val_h, report->report_ref_h, report->ccc_h);

}

void hids_client_dump_dervice_data(ble_client_t *client, hids_client_dump_service_data_cb_t cb)
{
        hids_client_t *hids_client = (hids_client_t *) client;
        struct dump_report_data report_data = {
                .cb = cb,
                .client = client,
        };

        if (!cb) {
                return;
        }

        if (hids_client->report_map_val_h) {
                cb(client, "Report Map Characteristic value handle:0x%04X",
                                                                hids_client->report_map_val_h);
        }

        if (hids_client->hid_info_val_h) {
                cb(client, "HID Information CHaracteristic value handle:0x%04X",
                                                                hids_client->hid_info_val_h);
        }

        if (hids_client->cp_val_h) {
                cb(client, "HID Control Point Characteristic value handle:0x%04X",
                                                                        hids_client->cp_val_h);
        }

        if (hids_client->protocol_mode_val_h) {
                cb(client, "Protocol Mode Characteristic value handle:0x%04X",
                                                                hids_client->protocol_mode_val_h);
        }

        queue_foreach(&hids_client->reports, dump_report, &report_data);
}

bool hids_client_set_protocol_mode(ble_client_t *client)
{
        hids_client_t *hids_client = (hids_client_t *) client;
        uint8_t protocol_mode = hids_client->mode;
        ble_error_t status;

        /* Write protocol mode */
        if (!hids_client->protocol_mode_val_h) {
                return false;
        }

        status = ble_gattc_write_no_resp(hids_client->client.conn_idx,
                                        hids_client->protocol_mode_val_h, false,
                                        sizeof(protocol_mode), &protocol_mode);

        return (status == BLE_STATUS_OK);
}
