/**
 ****************************************************************************************
 *
 * @file ancs_task.c
 *
 * @brief Apple Notification Center Service task
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include "osal.h"
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "ble_bufops.h"
#include "ble_service.h"
#include "ble_uuid.h"
#include "ble_gattc.h"
#include "util/list.h"
#include "util/queue.h"
#include "svc_types.h"
#include "hw_wkup.h"
#include "sys_watchdog.h"
#include "ancs_client.h"
#include "gatt_client.h"
#include "ancs_config.h"
#include "miniDB.h"

#define UUID_ANCS                       "7905F431-B5CE-4E99-A40F-4B1E122D00D0"
#define UUID_SERVICE_CHANGED            0x2A05

/**
 * Button notify mask
 */
#define BUTTON_NOTIF              (1 << 1)

/* Request timeout notify mask */
#define REQ_TMO_NOTIF             (1 << 2)

/* Start browse notify mask */
#define BROWSE_NOTIF              (1 << 3)

/**
 * Application state enum
 */
typedef enum {
        APP_STATE_DISCONNECTED,
        APP_STATE_CONNECTING,
        APP_STATE_CONNECTED,
        APP_STATE_BROWSING,
        APP_STATE_BROWSE_COMPLETED,
} app_state_t;

typedef struct {
        void *next;

        uint32_t uid;

        ancs_notification_data_t data;

        char *app_id;
        char *date;
        char *title;
        char *message;
} notification_t;

typedef struct {
        void *next;

        char *app_id;
        char *display_name;
} application_t;

static const uint8_t adv_data[] = {
        0x11, GAP_DATA_TYPE_UUID128_SOLIC,
        0xD0, 0x00, 0x2D, 0x12, 0x1E, 0x4B, 0x0F, 0xA4, 0x99, 0x4E, 0xCE, 0xB5, 0x31, 0xF4, 0x05,
                                                                                        0x79,
        // 7905F431-B5CE-4E99-A40F-4B1E122D00D0 (ANCS UUID)
};

static const uint8_t scan_rsp[] = {
        0x0C, GAP_DATA_TYPE_LOCAL_NAME,
        's', 'm', 'a', 'r', 'c', 'h', 'W', 'a', 't', 'c', 'h'
};

static void set_event_state_completed_cb(ble_client_t *client, att_error_t status,
                                                                        ancs_client_evt_t event);

static void notification_added_cb(ble_client_t *client, uint32_t uid,
                                                        const ancs_notification_data_t *notif_data);
static void notification_modified_cb(ble_client_t *client, uint32_t uid,
                                                        const ancs_notification_data_t *notif);
static void notification_removed_cb(ble_client_t *client, uint32_t uid);

static void notification_attr_cb(ble_client_t *client, uint32_t uid,
                                                        ancs_notification_attr_t attr, char *value);
static void get_notification_attr_completed_cb(ble_client_t *client, uint32_t uid, att_error_t status);

static void application_attr_cb(ble_client_t *client, const char *app_id,
                                                        ancs_application_attr_t attr, char *value);
static void get_application_attr_completed_cb(ble_client_t *client,
                                                        const char *app_id, att_error_t status);

static void perform_notification_action_completed_cb(ble_client_t *client, att_error_t status);

static const ancs_client_callbacks_t ancs_cb = {
        .set_event_state_completed = set_event_state_completed_cb,

        .notification_added = notification_added_cb,
        .notification_modified = notification_modified_cb,
        .notification_removed = notification_removed_cb,

        .notification_attr = notification_attr_cb,
        .get_notification_attr_completed = get_notification_attr_completed_cb,

        .application_attr = application_attr_cb,
        .get_application_attr_completed = get_application_attr_completed_cb,

        .perform_notification_action_completed = perform_notification_action_completed_cb,
};

static void gatt_service_changed_cb(ble_client_t *gatt_client, uint16_t start_handle,
                                                                        uint16_t end_handle);

/* GATT Client callbacks */
static const gatt_client_callbacks_t gatt_cb = {
        .set_event_state_completed = NULL,
        .get_event_state_completed = NULL,
        .service_changed = gatt_service_changed_cb,
};

/* Connected clients */
PRIVILEGED_DATA static ble_client_t *gatt_client;
PRIVILEGED_DATA static ble_client_t *ancs_client;
/* Current task handler */
PRIVILEGED_DATA static OS_TASK current_task;
/* Notifications queue (waiting to fetch attributes) */
PRIVILEGED_DATA static queue_t notif_q;
/* Application data cache */
PRIVILEGED_DATA static queue_t app_q;
/* Notification pending display and waiting to fetch application attributes (if any) */
PRIVILEGED_DATA static notification_t *pending_notif;
/* UID of last added notification */
PRIVILEGED_DATA static uint32_t last_notif_uid;
/* Timeout for requests */
PRIVILEGED_DATA static OS_TIMER req_tmo_timer;
/* Flag to set when request should time out */
PRIVILEGED_DATA static bool pending_tmo;
/* Timer to delay initial browse for ANCS */
PRIVILEGED_DATA static OS_TIMER browse_tmo_timer;
/* Connection index of active connected (there can be only one active connection) */
INITIALISED_PRIVILEGED_DATA static uint16_t active_conn_idx = BLE_CONN_IDX_INVALID;
/* Indicates if MTU echange procedure was performed */
PRIVILEGED_DATA static bool mtu_exchanged;
/* ANCS Client state */
INITIALISED_PRIVILEGED_DATA static app_state_t app_state = APP_STATE_DISCONNECTED;
/* Indicates if peer's services should be rediscovered */
PRIVILEGED_DATA static bool svc_changed;
/* ANCS action pending security */
PRIVILEGED_DATA static struct {
        bool is_event;
        ancs_client_evt_t event;

        bool is_notif;
} pending_sec_action;

void ancs_client_wkup_handler(void)
{
        OS_TASK_NOTIFY_FROM_ISR(current_task, BUTTON_NOTIF, OS_NOTIFY_SET_BITS);
}

static const char *notifcategory2str(ancs_notification_category_t category)
{
        switch (category) {
        case ANCS_NOTIFICATION_CATEGORY_OTHER:
                return "Other";
        case ANCS_NOTIFICATION_CATEGORY_INCOMING_CALL:
                return "Incoming call";
        case ANCS_NOTIFICATION_CATEGORY_MISSED_CALL:
                return "Missed call";
        case ANCS_NOTIFICATION_CATEGORY_VOICEMAIL:
                return "Voicemail";
        case ANCS_NOTIFICATION_CATEGORY_SOCIAL:
                return "Social";
        case ANCS_NOTIFICATION_CATEGORY_SCHEDULE:
                return "Schedule";
        case ANCS_NOTIFICATION_CATEGORY_EMAIL:
                return "E-mail";
        case ANCS_NOTIFICATION_CATEGORY_NEWS:
                return "News";
        case ANCS_NOTIFICATION_CATEGORY_HEALTH_AND_FITNESS:
                return "Health and Fitness";
        case ANCS_NOTIFICATION_CATEGORY_BUSINESS_AND_FINANCE:
                return "Business and Finance";
        case ANCS_NOTIFICATION_CATEGORY_LOCATION:
                return "Location";
        case ANCS_NOTIFICATION_CATEGORY_ENTERTAINMENT:
                return "Entertainment";
        }

        return "<unknown>";
}

static bool notification_match_uid(const void *data, const void *match_data)
{
        const notification_t *notif = data;
        uint32_t uid = (uint32_t) match_data;

        return notif->uid == uid;
}

static void free_notification(notification_t *notif)
{
        if (!notif) {
                return;
        }

        if (notif->app_id) {
                OS_FREE(notif->app_id);
        }

        if (notif->date) {
                OS_FREE(notif->date);
        }

        if (notif->title) {
                OS_FREE(notif->title);
        }

        if (notif->message) {
                OS_FREE(notif->message);
        }

        OS_FREE(notif);
}

static notification_t *add_notification(uint32_t uid, const ancs_notification_data_t *data)
{
        notification_t *notif;

        notif = OS_MALLOC(sizeof(notification_t));
        memset(notif, 0, sizeof(notification_t));
        notif->uid = uid;
        if (data) {
                notif->data = *data;
        }

#if CFG_NOTIF_QUEUE_MAX
        /*
         * If a maximum limit for temporarily saved notifications has been defined and reached,
         * remove the oldest pending notification.
         */
        if (queue_length(&notif_q) > CFG_NOTIF_QUEUE_MAX) {
                notification_t *oldest_notif = queue_pop_front(&notif_q);
                free_notification(oldest_notif);
        }
#endif
        queue_push_back(&notif_q, notif);

        return notif;
}

static notification_t *find_notification(uint32_t uid)
{
        return queue_find(&notif_q, notification_match_uid, (void *) uid);
}

static notification_t *remove_notification(uint32_t uid)
{
        return queue_remove(&notif_q, notification_match_uid, (void *) uid);
}

static bool application_match_id(const void *data, const void *match_data)
{
        const application_t *app = data;
        const char *app_id = match_data;

        return !strcmp(app->app_id, app_id);
}

static application_t *add_application(const char *app_id)
{
        application_t *app;

        app = OS_MALLOC(sizeof(application_t));
        memset(app, 0, sizeof(application_t));
        app->app_id = OS_MALLOC(strlen(app_id) + 1);
        strcpy(app->app_id, app_id);

        queue_push_back(&app_q, app);

        return app;
}

static application_t *find_application(const char *app_id)
{
        return queue_find(&app_q, application_match_id, app_id);
}

static void free_application(application_t *app)
{
        if (!app) {
                return;
        }

        if (app->app_id) {
                OS_FREE(app->app_id);
        }

        if (app->display_name) {
                OS_FREE(app->display_name);
        }

        OS_FREE(app);
}

static inline void fetch_next_notification(ble_client_t *client)
{
        notification_t *notif;

        notif = queue_peek_front(&notif_q);
        if (!notif) {
                return;
        }

        ancs_client_get_notification_attr(client, notif->uid,
                ANCS_ATTR(ANCS_NOTIFICATION_ATTR_APPLICATION_ID),
                ANCS_ATTR(ANCS_NOTIFICATION_ATTR_DATE),
                ANCS_ATTR_MAXLEN(ANCS_NOTIFICATION_ATTR_TITLE, CFG_TITLE_ATTRIBUTE_MAXLEN),
                ANCS_ATTR_MAXLEN(ANCS_NOTIFICATION_ATTR_MESSAGE, CFG_MESSAGE_ATTRIBUTE_MAXLEN),
                0);

        OS_TIMER_RESET(req_tmo_timer, OS_TIMER_FOREVER);
}

static inline void print_notification(const notification_t *notif, const application_t *app)
{
        const char *app_name = app ? app->display_name : "<unknown>";

        OS_TASK_NOTIFY(getDisplayTaskHandle(), BLE_APP_NOTIFY_MASK, eSetBits);
        printf("Application: %s (%s)\r\n", app_name, (app && notif->app_id) ? notif->app_id : "<unknown>");
        printf("Category:    %s\r\n", notifcategory2str(notif->data.category));
        printf("Date:        %s\r\n", notif->date);
        printf("Title:       %s\r\n", notif->title);
        setANCSTitle(notif->title);
        printf("Message:     %s\r\n", notif->message);
        setANCSMessage(notif->message);
        printf("\n");
}

static void set_event_state_completed_cb(ble_client_t *client, att_error_t status,
                                                                        ancs_client_evt_t event)
{
        if (status == ATT_ERROR_INSUFFICIENT_AUTHENTICATION) {
                pending_sec_action.is_event = true;
                pending_sec_action.event = event;

                ble_gap_set_sec_level(client->conn_idx, GAP_SEC_LEVEL_2);
                return;
        }

        pending_sec_action.is_event = false;

        /*
         * In case of other error, disconnect since we were not able to properly configure the
         * server properly and won't get notifications.
         */
        if (status != ATT_ERROR_OK) {
                ble_gap_disconnect(client->conn_idx, BLE_HCI_ERROR_REMOTE_USER_TERM_CON);
                return;
        }

        /* Data Source configured, now proceed with Notification Source */
        if (event == ANCS_CLIENT_EVT_DATA_SOURCE_NOTIF) {
                ancs_client_set_event_state(ancs_client,
                                                ANCS_CLIENT_EVT_NOTIFICATION_SOURCE_NOTIF, true);
        }
}

static void notification_added_cb(ble_client_t *client, uint32_t uid, const ancs_notification_data_t *notif_data)
{
#if CFG_VERBOSE_LOG
        printf("| Notification added (0x%08" PRIx32 ")\r\n", uid);
        printf("|\tflags=0x%02x\r\n", notif_data->flags);
        printf("|\tcategory=%d\r\n", notif_data->category);
        printf("|\tcategory_count=%d\r\n", notif_data->category_count);
        printf("\n");
#endif

        if ((!CFG_DROP_PREEXISTING_NOTIFICATIONS ||
                                        !(notif_data->flags & ANCS_NOTIFICATION_FLAG_PREEXISTING))
                && OS_GET_FREE_HEAP_SIZE() > CFG_DROP_ALL_NOTIF_THRESHOLD) {
                add_notification(uid, notif_data);
                last_notif_uid = uid;
        }

        if (!ancs_client_is_busy(client)) {
                fetch_next_notification(client);
        }
}

static void notification_modified_cb(ble_client_t *client, uint32_t uid, const ancs_notification_data_t *notif)
{
#if CFG_VERBOSE_LOG
        printf("| Notification modified (0x%08" PRIx32 ")\r\n", uid);
        printf("|\tflags=0x%02x\r\n", notif->flags);
        printf("|\tcategory=%d\r\n", notif->category);
        printf("|\tcategory_count=%d\r\n", notif->category_count);
        printf("\n");
#endif

}

static void notification_removed_cb(ble_client_t *client, uint32_t uid)
{
#if CFG_VERBOSE_LOG
        printf("| Notification removed (%08" PRIx32 ")\r\n", uid);
        printf("\n");
#endif
}

static void notification_attr_cb(ble_client_t *client, uint32_t uid, ancs_notification_attr_t attr, char *value)
{
        notification_t *notif;

#if CFG_VERBOSE_LOG
        printf("| Notification (%08" PRIx32 ") attribute (%d)\r\n", uid, attr);
        printf("|\t%s\r\n", value);
        printf("\n");
#endif

        notif = find_notification(uid);
        if (!notif) {
                OS_FREE(value);
                return;
        }

        switch (attr) {
        case ANCS_NOTIFICATION_ATTR_APPLICATION_ID:
                notif->app_id = value;
                break;
        case ANCS_NOTIFICATION_ATTR_DATE:
                notif->date = value;
                break;
        case ANCS_NOTIFICATION_ATTR_TITLE:
                notif->title = value;
                break;
        case ANCS_NOTIFICATION_ATTR_MESSAGE:
                notif->message = value;
                break;
        default:
                OS_FREE(value);
        }
}

static void ancs_task_cleanup()
{
        /*
         * Cleanup all queued notifications and applications cache - we don't need them since
         * session is now closed.
         */
        queue_remove_all(&notif_q, (queue_destroy_func_t) free_notification);
        queue_remove_all(&app_q, (queue_destroy_func_t) free_application);

        if(pending_notif) {
                // this has been removed from the queue, so if it exists free it separately
                free_notification(pending_notif);
                pending_notif = NULL;
        }
}

static void get_notification_attr_completed_cb(ble_client_t *client, uint32_t uid, att_error_t status)
{
        notification_t *notif;
        application_t *app = NULL;

        /* Make sure this request won't time out */
        OS_TIMER_STOP(req_tmo_timer, OS_TIMER_FOREVER);
        pending_tmo = false;

        if (status == ATT_ERROR_INSUFFICIENT_AUTHENTICATION) {
                pending_sec_action.is_notif = true;

                ble_gap_set_sec_level(client->conn_idx, GAP_SEC_LEVEL_2);
                return;
        }

        notif = remove_notification(uid);
        if (!notif) {
                /* ANCS client should not report this callback for notification that does not exist! */
                return;
        }

        if (status != ATT_ERROR_OK) {
#if CFG_VERBOSE_LOG
                printf("| FAILED to get attributes for 0x%08" PRIx32 "\r\n\n", uid);
#endif
                goto done;
        }

        if (notif->app_id) {
                app = find_application(notif->app_id);
                if (!app) {
                        pending_notif = notif;
                        ancs_client_get_application_attr(client, notif->app_id,
                                        ANCS_ATTR(ANCS_APPLICATION_ATTR_DISPLAY_NAME), 0);

                        OS_TIMER_RESET(req_tmo_timer, OS_TIMER_FOREVER);
                        return;
                }
        }

        print_notification(notif, app);

done:
        free_notification(notif);

        if (!ancs_client_is_busy(client)) {
                fetch_next_notification(client);
        }
}

static void application_attr_cb(ble_client_t *client, const char *app_id, ancs_application_attr_t attr, char *value)
{
        application_t *app;

#if CFG_VERBOSE_LOG
        printf("| Application (%s) attribute (%d)\r\n", app_id, attr);
        printf("|\t%s\r\n", value);
        printf("\n");
#endif

        app = find_application(app_id);
        if (!app) {
                app = add_application(app_id);
        }

        switch (attr) {
        case ANCS_APPLICATION_ATTR_DISPLAY_NAME:
                app->display_name = value;
                break;
        default:
                OS_FREE(value);
        }
}

static void get_application_attr_completed_cb(ble_client_t *client, const char *app_id, att_error_t status)
{
        /* Make sure this request won't time out */
        OS_TIMER_STOP(req_tmo_timer, OS_TIMER_FOREVER);
        pending_tmo = false;

        if (status != ATT_ERROR_OK) {
#if CFG_VERBOSE_LOG
                printf("| FAILED to get attributes for %s\r\n\n", app_id);
#endif
        }

        if (pending_notif) {
                application_t *app = find_application(app_id);

                print_notification(pending_notif, app);

                free_notification(pending_notif);
                pending_notif = NULL;
        }

        if (!ancs_client_is_busy(client)) {
                fetch_next_notification(client);
        }
}

static void perform_notification_action_completed_cb(ble_client_t *client, att_error_t status)
{
#if CFG_VERBOSE_LOG
        printf("| Perform notification action status: %d\r\n", status);
#endif
}

static void req_tmo_cb(OS_TIMER pxTime)
{
        pending_tmo = true;
        OS_TASK_NOTIFY(current_task, REQ_TMO_NOTIF, OS_NOTIFY_SET_BITS);
}

static void browse_tmo_cb(OS_TIMER pxTime)
{
        OS_TASK_NOTIFY(current_task, BROWSE_NOTIF, OS_NOTIFY_SET_BITS);
}


static void purge_clients(void)
{
        if (ancs_client) {
                ble_client_remove(ancs_client);
                ble_client_cleanup(ancs_client);
                ancs_client = NULL;
        }

        if (gatt_client) {
                ble_client_remove(gatt_client);
                ble_client_cleanup(gatt_client);
                gatt_client = NULL;
        }
}

static void gatt_service_changed_cb(ble_client_t *gatt_client, uint16_t start_handle,
                                                                        uint16_t end_handle)
{
#if CFG_VERBOSE_LOG
        printf("| Service changed notification: start_h: 0x%04x, end_h: 0x%04x\r\n", start_handle,
                                                                                        end_handle);
#endif

        /* Rediscover services once the initial browse is completed */
        if (app_state != APP_STATE_BROWSING) {
                purge_clients();

                printf("Services changed, browsing...\r\n");

                app_state = APP_STATE_BROWSING;
                ble_gattc_browse(gatt_client->conn_idx, NULL);
        } else {
                svc_changed = true;
        }
}

static void handle_evt_gap_connected(ble_evt_gap_connected_t *evt)
{
//        printf("Device connected\r\n");

        /* We can have only one active connection */
        if (active_conn_idx != BLE_CONN_IDX_INVALID) {
                OS_ASSERT(0);
                return;
        }

        app_state = APP_STATE_CONNECTED;
        active_conn_idx = evt->conn_idx;
        mtu_exchanged = false;

        ble_gattc_exchange_mtu(evt->conn_idx);
}

static void handle_evt_gap_disconnected(ble_evt_gap_disconnected_t *evt)
{
//        printf("Device disconnected\r\n");

        /* Make sure proper connection disconnected (just for sanity, we can have only one anyway) */
        if (evt->conn_idx != active_conn_idx) {
                OS_ASSERT(0);
                return;
        }

        active_conn_idx = BLE_CONN_IDX_INVALID;
        /* Make sure browse timer is stopped */
        OS_TIMER_STOP(browse_tmo_timer, OS_TIMER_FOREVER);

        /* Unregister ancs_client from clients framework and cleanup */
        if (ancs_client) {
                ble_client_remove(ancs_client);
                ble_client_cleanup(ancs_client);
                ancs_client = NULL;
        }

        ancs_task_cleanup();

        purge_clients();

        app_state = APP_STATE_DISCONNECTED;
        ble_gap_adv_start(GAP_CONN_MODE_UNDIRECTED);
}

static void handle_evt_gap_pair_req(ble_evt_gap_pair_req_t *evt)
{
        ble_gap_pair_reply(evt->conn_idx, true, evt->bond);
}

static void handle_evt_gap_sec_level_changed(ble_evt_gap_sec_level_changed_t *evt)
{
        //printf("Security level changed (level: %d)\r\n", evt->level + 1);

        if (pending_sec_action.is_event) {
                pending_sec_action.is_event = false;

                ancs_client_set_event_state(ancs_client, pending_sec_action.event, true);
        }

        if (pending_sec_action.is_notif) {
                pending_sec_action.is_notif = false;

                fetch_next_notification(ancs_client);
        }
}

static void handle_evt_gattc_browse_svc(ble_evt_gattc_browse_svc_t *evt)
{
        att_uuid_t uuid;

        /* If there was Service Changed indication in the meantime, we'll need to browse again */
        if (svc_changed) {
                return;
        }
        ble_uuid_from_string(UUID_ANCS, &uuid);
        if (ble_uuid_equal(&uuid, &evt->uuid)) {
                ancs_client = ancs_client_init(&ancs_cb, evt);
                if (!ancs_client) {
                        return;
                }

                ble_client_add(ancs_client);

                /*
                 * Enable Data Source notification first, if they succeed then we'll enable
                 * Notification Source as well which will start producing notification and Data
                 * Source will be already set properly.
                 */
                ancs_client_set_event_state(ancs_client, ANCS_CLIENT_EVT_DATA_SOURCE_NOTIF, true);

                return;
        }


        ble_uuid_create16(UUID_SERVICE_GATT, &uuid);
        if (ble_uuid_equal(&uuid, &evt->uuid)) {
                gatt_client = gatt_client_init(&gatt_cb, evt);
                if (!gatt_client) {
                        return;
                }

                ble_client_add(gatt_client);

                gatt_client_set_event_state(gatt_client, GATT_CLIENT_EVENT_SERVICE_CHANGED_INDICATE, true);
        }
}

static void handle_evt_gattc_browse_completed(ble_evt_gattc_browse_completed_t *evt)
{
        if (app_state == APP_STATE_BROWSING) {
                app_state = APP_STATE_BROWSE_COMPLETED;

//                printf("Browse completed\r\n");
//                printf("\tANCS: %s\r\n", ancs_client ? "found" : "not found");
//                printf("\tGATT: %s\r\n", gatt_client ? "found" : "not found");
//                printf("\r\n");
        }

        /* If there was Service Changed indication in the meantime, need to browse again */
        if (svc_changed) {
                svc_changed = false;
                purge_clients();

                printf("Services changed, browsing...\r\n");

                app_state = APP_STATE_BROWSING;
                ble_gattc_browse(evt->conn_idx, NULL);
        }
}

static void handle_evt_gattc_mtu_changed(ble_evt_gattc_mtu_changed_t *evt)
{
        if (mtu_exchanged) {
                return;
        }

        mtu_exchanged = true;

        /*
         * Start delay before triggering proper browse request to make sure everything is up and
         * running on iOS
         */
        OS_TIMER_START(browse_tmo_timer, OS_TIMER_FOREVER);

}

void ancs_task(void *params)
{
        int8_t wdog_id;

        /* register ancs task to be monitored by watchdog */
        wdog_id = sys_watchdog_register(false);

        ble_peripheral_start();
        ble_gap_mtu_size_set(128);
        ble_register_app();

        current_task = OS_GET_CURRENT_TASK();
//        printf("\r\nANCS Current Task: %d\r\n",(int)current_task);
//        printf("\r\nDisplay Idle Task: %d\r\n",(int)getDisplayTaskHandle());

        /*
         * Set device name and appearance to be discoverable by iOS devices
         */
        ble_gap_device_name_set("smarchWatch", ATT_PERM_READ);
        ble_gap_appearance_set(BLE_GAP_APPEARANCE_GENERIC_WATCH, ATT_PERM_READ);

        /*
         * Create timer which will be used to timeout requests which take too long
         */
        req_tmo_timer = OS_TIMER_CREATE("tmo", OS_MS_2_TICKS(CFG_REQUEST_TIMEOUT_MS), OS_TIMER_FAIL,
                                                                                NULL, req_tmo_cb);

        /*
         * Create timer which will be used to make short delay before starting browse for ANCS
         */
        browse_tmo_timer = OS_TIMER_CREATE("browse", OS_MS_2_TICKS(CFG_BROWSE_DELAY_MS), OS_TIMER_FAIL,
                                                                                NULL, browse_tmo_cb);

        ble_gap_adv_data_set(sizeof(adv_data), adv_data, sizeof(scan_rsp), scan_rsp);
        ble_gap_adv_start(GAP_CONN_MODE_UNDIRECTED);
//        printf("Start advertising...\r\n");

        for (;;) {
                OS_BASE_TYPE ret;
                uint32_t notif;

                /* Notify watchdog on each loop */
                sys_watchdog_notify(wdog_id);

                /* Suspend watchdog while blocking on OS_TASK_NOTIFY_WAIT() */
                sys_watchdog_suspend(wdog_id);

                /*
                 * Wait on any of the notification bits, then clear them all
                 */
                ret = OS_TASK_NOTIFY_WAIT(0, OS_TASK_NOTIFY_ALL_BITS, &notif, OS_TASK_NOTIFY_FOREVER);
                OS_ASSERT(ret == OS_OK);

                /* Resume watchdog */
                sys_watchdog_notify_and_resume(wdog_id);

                /* Notified from BLE manager, can get event */
                if (notif & BLE_APP_NOTIFY_MASK) {
                        ble_evt_hdr_t *hdr;

                        hdr = ble_get_event(false);
                        if (!hdr) {
                                goto no_event;
                        }

                        ble_client_handle_event(hdr);

                        if (!ble_service_handle_event(hdr)) {
                                switch (hdr->evt_code) {
                                case BLE_EVT_GAP_CONNECTED:
                                        handle_evt_gap_connected((ble_evt_gap_connected_t *) hdr);
                                        break;
                                case BLE_EVT_GAP_DISCONNECTED:
                                        handle_evt_gap_disconnected((ble_evt_gap_disconnected_t *) hdr);
                                        break;
                                case BLE_EVT_GAP_PAIR_REQ:
                                        handle_evt_gap_pair_req((ble_evt_gap_pair_req_t *) hdr);
                                        break;
                                case BLE_EVT_GAP_SEC_LEVEL_CHANGED:
                                        handle_evt_gap_sec_level_changed(
                                                        (ble_evt_gap_sec_level_changed_t *) hdr);
                                        break;
                                case BLE_EVT_GATTC_BROWSE_SVC:
                                        handle_evt_gattc_browse_svc(
                                                        (ble_evt_gattc_browse_svc_t *) hdr);
                                        break;
                                case BLE_EVT_GATTC_BROWSE_COMPLETED:
                                        handle_evt_gattc_browse_completed(
                                                        (ble_evt_gattc_browse_completed_t *) hdr);
                                        break;
                                case BLE_EVT_GATTC_MTU_CHANGED:
                                        handle_evt_gattc_mtu_changed(
                                                        (ble_evt_gattc_mtu_changed_t *) hdr);
                                        break;
                                default:
                                        ble_handle_event_default(hdr);
                                        break;
                                }
                        }

                        OS_FREE(hdr);

no_event:
                        /* Notify again if there are more events to process in queue */
                        if (ble_has_event()) {
                                OS_TASK_NOTIFY(OS_GET_CURRENT_TASK(), BLE_APP_NOTIFY_MASK, eSetBits);
                        }
                }

                if (notif & BUTTON_NOTIF) {
                        ancs_client_perform_notification_action(ancs_client, last_notif_uid,
                                                                                ANCS_ACTION_NEGATIVE);
                }

                if (notif & REQ_TMO_NOTIF) {
                        /*
                         * Even though we have notification, this flag can be reset by code above
                         * when request is completed at the same time as it supposed to time out.
                         */
                        if (pending_tmo) {
                                ancs_client_cancel_request(ancs_client);
                                pending_tmo = false;
                        }
                }

                /* Ignore browse request if we don't have connection (i.e. already disconnected) */
                if ((notif & BROWSE_NOTIF) && (active_conn_idx != BLE_CONN_IDX_INVALID)) {
                        //printf("Browsing...\r\n");

                        app_state = APP_STATE_BROWSING;
                        ble_gattc_browse(active_conn_idx, NULL);
                }
        }
}
