/**
 ****************************************************************************************
 *
 * @file ancs_client.c
 *
 * @brief Apple Notification Center Service client implementation
 *
 * Copyright (C) 2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdarg.h>
#include <string.h>
#include "osal.h"
#include "ble_bufops.h"
#include "ble_gattc.h"
#include "ble_gattc_util.h"
#include "ble_uuid.h"
#include "ancs_client.h"

#define UUID_ANCS                 "7905F431-B5CE-4E99-A40F-4B1E122D00D0"
#define UUID_NOTIFICATION_SOURCE  "9FBF120D-6301-42D9-8C58-25E699A21DBD"
#define UUID_CONTROL_POINT        "69D1D8F3-45E1-49A8-9821-9BBDFDAAD9D9"
#define UUID_DATA_SOURCE          "22EAC6E9-24D6-4BB5-BE44-B36ACE7C7BFB"

enum ctrl_point {
        CTRL_POINT_GET_NOTIFICATION_ATTRIBUTES = 0,
        CTRL_POINT_GET_APP_ATTRIBUTES = 1,
        CTRL_POINT_PERFORM_NOTIFICATION_ACTION = 2,
        /* dummy entry to indicate no-op */
        CTRL_POINT_LAST,
};

typedef struct {
        uint8_t event_id;
        uint8_t event_flags;
        uint8_t category_id;
        uint8_t category_count;
        uint32_t notification_uid;
} __attribute__((packed)) notification_source_pdu_t;

typedef struct {
        bool in_progress : 1;   // operation in progress
        bool has_command : 1;   // has CommandID received
        bool has_id : 1;        // has object identifier received
        bool wait_write_cmp : 1;        // has write response been received

        // number of attributes to retrieve
        uint8_t attr_num;

        // object identifier for request (to reply in case of error)
        void *obj_id;

        // CommandID
        enum ctrl_point command;

        // object identifier state
        union {
                uint32_t uid;
                char *app_id;
        };
        uint16_t app_id_len;

        // attribute header state
        uint8_t hdr_len;
        union {
                uint8_t hdr[3];
                struct {
                        uint8_t attr;
                        uint16_t value_len;
                } __attribute__((packed));
        };

        // attribute value state
        uint16_t recv_len;
        char *value;
} data_src_state_t;

typedef struct {
        ble_client_t client;

        const ancs_client_callbacks_t *cb;

        ancs_client_cap_t caps;

        uint16_t notif_src_h;
        uint16_t notif_src_ccc_h;

        uint16_t ctrl_point_h;

        uint16_t data_src_h;
        uint16_t data_src_ccc_h;

        enum ctrl_point ctrl_point_state;
        data_src_state_t data_src_state;
} ancs_client_t;

static void cleanup(ble_client_t *client)
{
        ancs_client_t *ancs_client = (ancs_client_t *) client;

        OS_FREE(ancs_client);
}

static void dispatch_notification_source_event(ble_client_t *client, const ble_evt_gattc_notification_t *evt)
{
        ancs_client_t *ancs_client = (ancs_client_t *) client;
        notification_source_pdu_t *pdu = (notification_source_pdu_t *) evt->value;
        ancs_notification_data_t notif;

        if (evt->length < sizeof(notification_source_pdu_t)) {
                return;
        }

        notif.flags = pdu->event_flags;
        notif.category = pdu->category_id;
        notif.category_count = pdu->category_count;

        switch (pdu->event_id) {
        case 0x00:
                if (ancs_client->cb->notification_added) {
                        ancs_client->cb->notification_added(client, pdu->notification_uid, &notif);
                }
                break;
        case 0x01:
                if (ancs_client->cb->notification_modified) {
                        ancs_client->cb->notification_modified(client, pdu->notification_uid, &notif);
                }
                break;
        case 0x02:
                if (ancs_client->cb->notification_removed) {
                        ancs_client->cb->notification_removed(client, pdu->notification_uid);
                }
                break;
        }
}

static void *append_buf(void *dst, uint16_t *dst_len, const void *src, uint16_t src_len)
{
        uint8_t *tmp;

        tmp = OS_MALLOC(*dst_len + src_len);

        if (dst) {
                memcpy(tmp, dst, *dst_len);
        }

        memcpy(tmp + *dst_len, src, src_len);

        if (dst) {
                OS_FREE(dst);
        }

        *dst_len = *dst_len + src_len;

        return tmp;
}

static void complete_request(ble_client_t *client, att_error_t status)
{
        ancs_client_t *ancs_client = (ancs_client_t *) client;
        data_src_state_t *state = &ancs_client->data_src_state;

        state->in_progress = false;
        ancs_client->ctrl_point_state = CTRL_POINT_LAST;

        if (state->command == CTRL_POINT_GET_NOTIFICATION_ATTRIBUTES) {
                uint32_t uid;

                memcpy(&uid, state->obj_id, sizeof(uid));

                OS_FREE(state->obj_id);
                state->obj_id = NULL;

                ancs_client->cb->get_notification_attr_completed(client, uid, status);
        } else if (state->command == CTRL_POINT_GET_APP_ATTRIBUTES) {
                char *app_id = state->obj_id;

                if (state->app_id) {
                        OS_FREE(state->app_id);
                }

                state->obj_id = NULL;

                ancs_client->cb->get_application_attr_completed(client, app_id, status);

                OS_FREE(app_id);
        }
}

static void dispatch_data_source_event(ble_client_t *client, const ble_evt_gattc_notification_t *evt)
{
        ancs_client_t *ancs_client = (ancs_client_t *) client;
        data_src_state_t *state = &ancs_client->data_src_state;
        const uint8_t *p = evt->value;
        const uint8_t *end_p = evt->value + evt->length;

        /* Waiting for something? */
        if (!state->in_progress || !state->attr_num || state->wait_write_cmp) {
                return;
        }

        if (!state->has_command) {
                uint8_t cmd = get_u8_inc(&p);

                if (cmd != state->command) {
                        /* CommandID does not match, ignore and reset state to wait for another */
                        return;
                }

                state->hdr_len = 0;
                state->has_command = true;
        }

        if (!state->has_id) {
                if (state->command == CTRL_POINT_GET_NOTIFICATION_ATTRIBUTES) {
                        /*
                         * For CommandIDGetNotificationAttributes we just retrieve 4 bytes long
                         * NotificationUID which can always fit in 1st PDU
                         */

                        state->uid = get_u32_inc(&p);
                        state->has_id = true;

                        if (memcmp(&state->uid, state->obj_id, sizeof(state->uid))) {
                                /*
                                 * NotificationUID does not match, ignore and reset state to wait
                                 * for another
                                 */
                                state->has_id = false;
                                state->has_command = false;
                                return;
                        }
                } else if (state->command == CTRL_POINT_GET_APP_ATTRIBUTES) {
                        /*
                         * For CommandIDGetAppAttributes application identified is null-terminated
                         * string of unknown length. Usually it will probably fit in 1st PDU, but
                         * we need to also handle case when it's split across multiple PDUs (unlikely).
                         */

                        const uint8_t *f = memchr(p, 0, end_p - p);

                        if (!f) {
                                f = end_p;
                        } else {
                                f++;
                                state->has_id = true;
                        }

                        state->app_id = append_buf(state->app_id, &state->app_id_len, p, f - p);
                        p = f;

                        if (state->has_id && strcmp(state->app_id, state->obj_id)) {
                                /*
                                 * ApplicationID does not match, ignore and reset state to wait
                                 * for another
                                 */
                                state->has_id = false;
                                state->has_command = false;

                                /* Need to also clean received app_id, we start with empty one */
                                OS_FREE(state->app_id);
                                state->app_id = NULL;
                                return;
                        }
                } else {
                        goto protocol_error;
                }

                /* If still does not have complete object identifier, wait for next PDU */
                if (!state->has_id) {
                        return;
                }
        }

        while (p < end_p) {
                int data_avail;

                /*
                 * Header is only 3 bytes long but due to fragmentation done in ANCS it may be split
                 * between two PDUs. We need to reassemble it before can access rest of data so
                 * if header cannot be completed using data from current PDU, we'll just wait for
                 * next one.
                 */
                while ((state->hdr_len < sizeof(state->hdr)) && (p < end_p)) {
                        state->hdr[state->hdr_len++] = *p++;
                }

                if (state->hdr_len < sizeof(state->hdr)) {
                        return;
                }

                /*
                 * If value buffer is not allocated it means we've just got complete header and can
                 * allocate it now (we know size). In our case returned string will be always
                 * null-terminated.
                 */
                if (!state->value) {
                        /* Returned value length cannot be larger than our preconfigured maximum */
                        if (state->value_len > CFG_ANCS_ATTRIBUTE_MAXLEN) {
                                goto protocol_error;
                        }

                        state->recv_len = 0;
                        state->value = OS_MALLOC(state->value_len + 1);
                        state->value[state->value_len] = '\0';
                }

                /*
                 * Now just check how many data for current attribute can be retrieved from current
                 * PDU and copy it to output buffer. If we have complete attribute value in buffer
                 * we can fire callback and reset state to start looking for next attribute. Or just
                 * return in case there are no more attributes expected.
                 */
                data_avail = MIN(state->value_len - state->recv_len, end_p - p);

                memcpy(&state->value[state->recv_len], p, data_avail);

                state->recv_len += data_avail;
                p += data_avail;

                if (state->recv_len == state->value_len) {
                        char *value = state->value;
                        bool more = --state->attr_num;

                        /*
                         * Reset header length so we'll start looking for another one.
                         * This is done before firing callback since this will allow application to
                         * start another request from within callback handler in case this one will
                         * be last one.
                         */
                        state->hdr_len = 0;
                        state->value = NULL;
                        if (!more) {
                                OS_FREE(state->obj_id);
                                state->obj_id = NULL;
                                state->in_progress = false;
                        }

                        if (state->command == CTRL_POINT_GET_NOTIFICATION_ATTRIBUTES) {
                                ancs_client->cb->notification_attr(client, state->uid, state->attr, value);

                                if (!more) {
                                        ancs_client->ctrl_point_state = CTRL_POINT_LAST;
                                        ancs_client->cb->get_notification_attr_completed(client, state->uid, ATT_ERROR_OK);
                                }
                        } else if (state->command == CTRL_POINT_GET_APP_ATTRIBUTES) {
                                ancs_client->cb->application_attr(client, state->app_id, state->attr, value);

                                if (!more) {
                                        /*
                                         * We store app_id because can make another request from
                                         * callback which would overwrite app_id - this would cause
                                         * memory leak.
                                         */

                                        char *app_id = state->app_id;

                                        ancs_client->ctrl_point_state = CTRL_POINT_LAST;
                                        ancs_client->cb->get_application_attr_completed(client, app_id, ATT_ERROR_OK);

                                        OS_FREE(app_id);
                                }
                        }

                        if (!more) {
                                return;
                        }

                }
        }

        return;

protocol_error:
        complete_request(client, ATT_ERROR_UNLIKELY);
}

static void dispatch_get_event_state_completed(ble_client_t *client, const ble_evt_gattc_read_completed_t *evt)
{
        ancs_client_t *ancs_client = (ancs_client_t *) client;
        ancs_client_evt_t event;
        uint16_t ccc = 0;

        if (!ancs_client->cb->get_event_state_completed) {
                return;
        }

        if (evt->handle == ancs_client->notif_src_ccc_h) {
                event = ANCS_CLIENT_EVT_NOTIFICATION_SOURCE_NOTIF;
        } else if (evt->handle == ancs_client->data_src_ccc_h) {
                event = ANCS_CLIENT_EVT_DATA_SOURCE_NOTIF;
        } else {
                return;
        }

        if (evt->length < sizeof(uint16_t)) {
                ancs_client->cb->get_event_state_completed(client, ATT_ERROR_UNLIKELY, event, 0);
                return;
        }

        if (evt->status == ATT_ERROR_OK) {
                ccc = get_u16(evt->value);
        }

        ancs_client->cb->get_event_state_completed(client, evt->status, event, ccc & GATT_CCC_NOTIFICATIONS);
}

static void dispatch_set_event_state_completed(ble_client_t *client, const ble_evt_gattc_write_completed_t *evt)
{
        ancs_client_t *ancs_client = (ancs_client_t *) client;
        ancs_client_evt_t event;

        if (!ancs_client->cb->set_event_state_completed) {
                return;
        }

        if (evt->handle == ancs_client->notif_src_ccc_h) {
                event = ANCS_CLIENT_EVT_NOTIFICATION_SOURCE_NOTIF;
        } else if (evt->handle == ancs_client->data_src_ccc_h) {
                event = ANCS_CLIENT_EVT_DATA_SOURCE_NOTIF;
        } else {
                return;
        }

        ancs_client->cb->set_event_state_completed(client, evt->status, event);
}

static void dispatch_ctrl_point_write_completed(ble_client_t *client, const ble_evt_gattc_write_completed_t *evt)
{
        ancs_client_t *ancs_client = (ancs_client_t *) client;
        data_src_state_t *state = &ancs_client->data_src_state;
        void *obj_id;

        state->wait_write_cmp = false;

        if (ancs_client->ctrl_point_state == CTRL_POINT_PERFORM_NOTIFICATION_ACTION) {
                ancs_client->ctrl_point_state = CTRL_POINT_LAST;
                ancs_client->cb->perform_notification_action_completed(client, evt->status);
                return;
        }

        /* We have some special handling below for commands which use Data Source */
        if ((ancs_client->ctrl_point_state != CTRL_POINT_GET_NOTIFICATION_ATTRIBUTES) &&
                                (ancs_client->ctrl_point_state != CTRL_POINT_GET_APP_ATTRIBUTES)) {
                return;
        }

        /* In case of error we'll receive reply via Data Source notification */
        if (evt->status == ATT_ERROR_OK) {
                return;
        }

        /*
         * We need to reset state here (and also keep copy of obj_id) since callbacks fired below
         * may want to start another ANCS request and this is only possible with 'clear' state.
         */
        state->in_progress = false;
        ancs_client->ctrl_point_state = CTRL_POINT_LAST;
        obj_id = state->obj_id;
        state->obj_id = NULL;

        /*
         * In this case state->command and ancs_client->ctrl_point_state should both have the same
         * value, but we've just cleared ctrl_point_state to make Control Point free - this way
         * application can start another request directly from callback.
         */
        if (state->command == CTRL_POINT_GET_NOTIFICATION_ATTRIBUTES) {
                uint32_t uid;

                memcpy(&uid, obj_id, sizeof(uid));

                ancs_client->cb->get_notification_attr_completed(client, uid, evt->status);
        } else {
                ancs_client->cb->get_application_attr_completed(client, obj_id, evt->status);
        }

        OS_FREE(obj_id);
}

static void handle_disconnect_evt(ble_client_t *client, const ble_evt_gap_disconnected_t *evt)
{
        client->conn_idx = BLE_CONN_IDX_INVALID;

        ble_client_remove(client);
}

static void handle_notification_evt(ble_client_t *client, const ble_evt_gattc_notification_t *evt)
{
        ancs_client_t *ancs_client = (ancs_client_t *) client;

        if (evt->handle == ancs_client->notif_src_h) {
                dispatch_notification_source_event(client, evt);
        } else if (evt->handle == ancs_client->data_src_h) {
                dispatch_data_source_event(client, evt);
        }
}

static void handle_read_completed_evt(ble_client_t *client, const ble_evt_gattc_read_completed_t *evt)
{
        ancs_client_t *ancs_client = (ancs_client_t *) client;

        if (evt->handle == ancs_client->notif_src_ccc_h) {
                dispatch_get_event_state_completed(client, evt);
        } else if (evt->handle == ancs_client->data_src_ccc_h) {
                dispatch_get_event_state_completed(client, evt);
        }
}

static void handle_write_completed_evt(ble_client_t *client, const ble_evt_gattc_write_completed_t *evt)
{
        ancs_client_t *ancs_client = (ancs_client_t *) client;

        if (evt->handle == ancs_client->notif_src_ccc_h) {
                dispatch_set_event_state_completed(client, evt);
        } else if (evt->handle == ancs_client->data_src_ccc_h) {
                dispatch_set_event_state_completed(client, evt);
        } else if (evt->handle == ancs_client->ctrl_point_h) {
                dispatch_ctrl_point_write_completed(client, evt);
        }
}

ble_client_t *ancs_client_init(const ancs_client_callbacks_t *cb, const ble_evt_gattc_browse_svc_t *evt)
{
        ancs_client_t *ancs_client;
        att_uuid_t uuid, uuid_ccc;
        const gattc_item_t *item;

        if (!cb || !evt) {
                return NULL;
        }

        ble_uuid_from_string(UUID_ANCS, &uuid);
        if (!ble_uuid_equal(&uuid, &evt->uuid)) {
                return NULL;
        }

        ble_uuid_create16(UUID_GATT_CLIENT_CHAR_CONFIGURATION, &uuid_ccc);

        ancs_client = OS_MALLOC(sizeof(ancs_client_t));
        memset(ancs_client, 0, sizeof(ancs_client_t));
        ancs_client->client.conn_idx = evt->conn_idx;
        ancs_client->client.cleanup = cleanup;
        ancs_client->client.disconnected_evt = handle_disconnect_evt;
        ancs_client->client.notification_evt = handle_notification_evt;
        ancs_client->client.read_completed_evt = handle_read_completed_evt;
        ancs_client->client.write_completed_evt = handle_write_completed_evt;
        ancs_client->cb = cb;
        ancs_client->ctrl_point_state = CTRL_POINT_LAST;

        ble_gattc_util_find_init(evt);

        /* Notification Source characteristic */
        ble_uuid_from_string(UUID_NOTIFICATION_SOURCE, &uuid);
        item = ble_gattc_util_find_characteristic(&uuid);
        if (!item || !(item->c.properties & GATT_PROP_NOTIFY)) {
                goto failed;
        }
        ancs_client->notif_src_h = item->c.value_handle;

        /* Notification Source CCC descriptor */
        item = ble_gattc_util_find_descriptor(&uuid_ccc);
        if (!item) {
                goto failed;
        }
        ancs_client->notif_src_ccc_h = item->handle;

        /* Control Point characteristic */
        ble_uuid_from_string(UUID_CONTROL_POINT, &uuid);
        item = ble_gattc_util_find_characteristic(&uuid);
        if (item && (item->c.properties & GATT_PROP_WRITE)) {
                ancs_client->ctrl_point_h = item->c.value_handle;
                ancs_client->caps |= ANCS_CLIENT_CAP_CONTROL_POINT;
        }

        /* Data Source characteristic */
        ble_uuid_from_string(UUID_DATA_SOURCE, &uuid);
        item = ble_gattc_util_find_characteristic(&uuid);
        if (item && (item->c.properties & GATT_PROP_NOTIFY)) {
                ancs_client->data_src_h = item->c.value_handle;

                /* Data Source CCC descriptor */
                item = ble_gattc_util_find_descriptor(&uuid_ccc);
                if (item) {
                        ancs_client->data_src_ccc_h = item->handle;
                        ancs_client->caps |= ANCS_CLIENT_CAP_DATA_SOURCE;
                }
        }

        return &ancs_client->client;

failed:
        cleanup(&ancs_client->client);
        return NULL;
}

bool ancs_client_get_event_state(ble_client_t *client, ancs_client_evt_t event)
{
        ancs_client_t *ancs_client = (ancs_client_t *) client;
        ble_error_t err;
        bool ret = false;

        switch (event) {
        case ANCS_CLIENT_EVT_NOTIFICATION_SOURCE_NOTIF:
                err = ble_gattc_read(client->conn_idx, ancs_client->notif_src_ccc_h, 0);
                ret = (err == BLE_STATUS_OK);
                break;

        case ANCS_CLIENT_EVT_DATA_SOURCE_NOTIF:
                if (ancs_client->caps & ANCS_CLIENT_CAP_DATA_SOURCE) {
                        err = ble_gattc_read(client->conn_idx, ancs_client->data_src_ccc_h, 0);
                        ret = (err == BLE_STATUS_OK);
                }
                break;
        }

        return ret;
}

bool ancs_client_set_event_state(ble_client_t *client, ancs_client_evt_t event, bool enabled)
{
        ancs_client_t *ancs_client = (ancs_client_t *) client;
        gatt_ccc_t ccc = enabled ? GATT_CCC_NOTIFICATIONS : 0;
        bool ret = false;

        switch (event) {
        case ANCS_CLIENT_EVT_NOTIFICATION_SOURCE_NOTIF:
                ret = (ble_gattc_util_write_ccc(client->conn_idx, ancs_client->notif_src_ccc_h,
                                                                        ccc) == BLE_STATUS_OK);
                break;

        case ANCS_CLIENT_EVT_DATA_SOURCE_NOTIF:
                if (ancs_client->caps & ANCS_CLIENT_CAP_DATA_SOURCE) {
                        ret = (ble_gattc_util_write_ccc(client->conn_idx, ancs_client->data_src_ccc_h,
                                                                        ccc) == BLE_STATUS_OK);
                }
                break;
        }

        return ret;
}

static bool send_attributes_req(ble_client_t *client, enum ctrl_point command,
                                                        size_t id_len, const void *id, va_list ap)
{
        ancs_client_t *ancs_client = (ancs_client_t *) client;
        data_src_state_t *state = &ancs_client->data_src_state;
        uint8_t *pdu, *p;
        uint32_t attr;
        uint16_t size;
        uint8_t attr_num;
        va_list ap2;

        if (ancs_client->ctrl_point_state < CTRL_POINT_LAST|| state->in_progress) {
                return false;
        }

        /* obj_id should be NULL here since there is no request in progress */
        if (state->obj_id != NULL) {
                OS_ASSERT(0);
                return false;
        }

        /*
         * We need 1st pass to calculate number of attributes passed and size required to store
         * them in PDU. Then we can allocate proper buffer for PDU and do 2nd pass to fill it.
         */

        va_copy(ap2, ap);

        size = 1 + id_len; // for CommandID and <element>ID
        attr_num = 0;
        for (attr = va_arg(ap2, uint32_t); attr; attr = va_arg(ap2, uint32_t)) {
                size++;
                attr_num++;

                if (command != CTRL_POINT_GET_NOTIFICATION_ATTRIBUTES) {
                        continue;
                }

                if ((uint8_t) attr == ANCS_NOTIFICATION_ATTR_TITLE
                                        || (uint8_t) attr == ANCS_NOTIFICATION_ATTR_SUBTITLE
                                        || (uint8_t) attr == ANCS_NOTIFICATION_ATTR_MESSAGE) {
                        size += sizeof(uint16_t);
                }
        }

        va_end(ap2);

        pdu = OS_MALLOC_NORET(size);
        p = pdu;

        put_u8_inc(&p, command);
        memcpy(p, id, id_len);
        p += id_len;

        for (attr = va_arg(ap, uint32_t); attr; attr = va_arg(ap, uint32_t)) {
                put_u8_inc(&p, attr);

                if (command != CTRL_POINT_GET_NOTIFICATION_ATTRIBUTES) {
                        continue;
                }

                if ((uint8_t) attr == ANCS_NOTIFICATION_ATTR_TITLE
                                        || (uint8_t) attr == ANCS_NOTIFICATION_ATTR_SUBTITLE
                                        || (uint8_t) attr == ANCS_NOTIFICATION_ATTR_MESSAGE) {
                        uint16_t len =  attr >> 8;

                        if (len > CFG_ANCS_ATTRIBUTE_MAXLEN) {
                                len = CFG_ANCS_ATTRIBUTE_MAXLEN;
                        }

                        put_u16_inc(&p, len);
                }
        }

        if (ble_gattc_write(client->conn_idx, ancs_client->ctrl_point_h, 0, p - pdu, pdu)) {
                OS_FREE_NORET(pdu);
                return false;
        }

        ancs_client->ctrl_point_state = command;

        memset(state, 0, sizeof(data_src_state_t));
        state->in_progress = true;
        state->command = command;
        state->attr_num = attr_num;
        state->obj_id = OS_MALLOC(id_len);
        memcpy(state->obj_id, id, id_len);

        state->wait_write_cmp = true;

        OS_FREE_NORET(pdu);
        return true;
}

bool ancs_client_get_notification_attr(ble_client_t *client, uint32_t notif_uid, ...)
{
        va_list ap;
        bool ret;


        va_start(ap, notif_uid);
        ret = send_attributes_req(client, CTRL_POINT_GET_NOTIFICATION_ATTRIBUTES,
                                                                sizeof(notif_uid), &notif_uid, ap);
        va_end(ap);

        return ret;
}

bool ancs_client_get_application_attr(ble_client_t *client, const char *app_id, ...)
{
        va_list ap;
        bool ret;


        va_start(ap, app_id);
        ret = send_attributes_req(client, CTRL_POINT_GET_APP_ATTRIBUTES,
                                                                strlen(app_id) + 1, app_id, ap);
        va_end(ap);

        return ret;
}

bool ancs_client_is_busy(ble_client_t *client)
{
        ancs_client_t *ancs_client = (ancs_client_t *) client;
        data_src_state_t *state = &ancs_client->data_src_state;

        return state->in_progress;
}

bool ancs_client_cancel_request(ble_client_t *client)
{
        if (!ancs_client_is_busy(client)) {
                return false;
        }

        complete_request(client, ATT_ERROR_APPLICATION_ERROR);

        return true;
}

bool ancs_client_perform_notification_action(ble_client_t *client, uint32_t notif_uid,
                                                                        ancs_action_t action)
{
        ancs_client_t *ancs_client = (ancs_client_t *) client;
        uint8_t pdu[6];
        uint8_t *p = pdu;

        if (ancs_client->ctrl_point_state < CTRL_POINT_LAST) {
                return false;
        }

        put_u8_inc(&p, CTRL_POINT_PERFORM_NOTIFICATION_ACTION);
        put_u32_inc(&p, notif_uid);
        put_u8_inc(&p, action);

        if (ble_gattc_write(client->conn_idx, ancs_client->ctrl_point_h, 0, sizeof(pdu), pdu)) {
                return false;
        }

        ancs_client->ctrl_point_state = CTRL_POINT_PERFORM_NOTIFICATION_ACTION;

        return true;
}
