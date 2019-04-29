/**
 ****************************************************************************************
 *
 * @file ancs_client.h
 *
 * @brief Apple Notification Center Service client interface
 *
 * Copyright (C) 2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef ANCS_CLIENT_H_
#define ANCS_CLIENT_H_

#include <stdint.h>
#include "ble_client.h"

/** Maximum allowed value for attribute length */
#ifndef CFG_ANCS_ATTRIBUTE_MAXLEN
#define CFG_ANCS_ATTRIBUTE_MAXLEN 128
#endif

/** Attribute ID element without maximum length */
#define ANCS_ATTR(ID) ((uint32_t) 0x80000000 | ((uint8_t) ID))

/** Attribute ID element with maximum length */
#define ANCS_ATTR_MAXLEN(ID, LEN) ((uint32_t) 0x80000000 | ((uint8_t) ID) | ((uint16_t) LEN << 8))

/** Notification attribute identifiers (as defined by specification) */
typedef enum {
        ANCS_NOTIFICATION_ATTR_APPLICATION_ID = 0,
        ANCS_NOTIFICATION_ATTR_TITLE = 1,       ///< specify maximum length when reading
        ANCS_NOTIFICATION_ATTR_SUBTITLE = 2,    ///< specify maximum length when reading
        ANCS_NOTIFICATION_ATTR_MESSAGE = 3,     ///< specify maximum length when reading
        ANCS_NOTIFICATION_ATTR_MESSAGE_SIZE = 4,
        ANCS_NOTIFICATION_ATTR_DATE = 5,
        ANCS_NOTIFICATION_ATTR_POSITIVE_ACTION_LABEL = 6,
        ANCS_NOTIFICATION_ATTR_NEGATIVE_ACTION_LABEL = 7,
} ancs_notification_attr_t;

/** Notification flags (as defined by specification) */
typedef enum {
        ANCS_NOTIFICATION_FLAG_SILENT = (1 << 0),
        ANCS_NOTIFICATION_FLAG_IMPORTANT = (1 << 1),
        ANCS_NOTIFICATION_FLAG_PREEXISTING = (1 << 2),
        ANCS_NOTIFICATION_FLAG_POSITIVE_ACTION = (1 << 3),
        ANCS_NOTIFICATION_FLAG_NEGATIVE_ACTION = (1 << 4),
} ancs_notification_flag_t;

/** Notification categories (as defined by specification */
typedef enum {
        ANCS_NOTIFICATION_CATEGORY_OTHER = 0,
        ANCS_NOTIFICATION_CATEGORY_INCOMING_CALL = 1,
        ANCS_NOTIFICATION_CATEGORY_MISSED_CALL = 2,
        ANCS_NOTIFICATION_CATEGORY_VOICEMAIL = 3,
        ANCS_NOTIFICATION_CATEGORY_SOCIAL = 4,
        ANCS_NOTIFICATION_CATEGORY_SCHEDULE = 5,
        ANCS_NOTIFICATION_CATEGORY_EMAIL = 6,
        ANCS_NOTIFICATION_CATEGORY_NEWS = 7,
        ANCS_NOTIFICATION_CATEGORY_HEALTH_AND_FITNESS = 8,
        ANCS_NOTIFICATION_CATEGORY_BUSINESS_AND_FINANCE = 9,
        ANCS_NOTIFICATION_CATEGORY_LOCATION = 10,
        ANCS_NOTIFICATION_CATEGORY_ENTERTAINMENT = 11,
} ancs_notification_category_t;

/** Notification data as received from Notification Source */
typedef struct {
        ancs_notification_flag_t flags;         ///< bitmask of flags
        ancs_notification_category_t category;  ///< category
        uint8_t category_count;                 ///< category count
} ancs_notification_data_t;

/** Application attribute identifiers (as defined by specification) */
typedef enum {
        ANCS_APPLICATION_ATTR_DISPLAY_NAME = 0,
} ancs_application_attr_t;

/** Notification actions (as defined by specification) */
typedef enum {
        ANCS_ACTION_POSITIVE = 0,
        ANCS_ACTION_NEGATIVE = 1,
} ancs_action_t;

/**
 * Client capabilities
 *
 * \note Only optional features are listed here. Mandatory features are assumed to be always supported
 * if client initialization was successful.
 */
typedef enum {
        ANCS_CLIENT_CAP_CONTROL_POINT,  ///< Control Point is supported
        ANCS_CLIENT_CAP_DATA_SOURCE,    ///< Data Source is supported
} ancs_client_cap_t;

/** Configurable events (notifications and indications) */
typedef enum {
        ANCS_CLIENT_EVT_NOTIFICATION_SOURCE_NOTIF,      ///< Notification Source notifications
        ANCS_CLIENT_EVT_DATA_SOURCE_NOTIF,              ///< Data Source notifications
} ancs_client_evt_t;

/** Client callbacks */
typedef struct {
        /**
         * Get event state completed
         *
         * Called when ancs_client_get_event_state() action is completed.
         *
         * \param [in] client   client instance
         * \param [in] status   operation status
         * \param [in] event    requested event
         * \param [in] enabled  event state (true if enabled, false otherwise)
         *
         */
        void (* get_event_state_completed) (ble_client_t *client, att_error_t status,
                                                        ancs_client_evt_t event, bool enabled);

        /**
         * Set event state completed
         *
         * Called when ancs_client_set_event_state() action is completed.
         *
         * \param [in] client   client instance
         * \param [in] status   operation status
         * \param [in] event    requested event
         *
         */
        void (* set_event_state_completed) (ble_client_t *client, att_error_t status,
                                                                        ancs_client_evt_t event);

        /**
         * Notification Added event received
         *
         * ANCS_CLIENT_EVT_NOTIFICATION_SOURCE_NOTIF event has to be enabled in order for application
         * to receive this event.
         *
         * \param [in] client  client instance
         * \param [in] uid     notification UID
         * \param [in] notif   notification data
         *
         */
        void (* notification_added) (ble_client_t *client, uint32_t uid,
                                                        const ancs_notification_data_t *notif);

        /**
         * Notification Modified event received
         *
         * ANCS_CLIENT_EVT_NOTIFICATION_SOURCE_NOTIF event has to be enabled in order for application
         * to receive this event.
         *
         * \param [in] client  client instance
         * \param [in] uid     notification UID
         * \param [in] notif   notification data
         *
         */
        void (* notification_modified) (ble_client_t *client, uint32_t uid,
                                                        const ancs_notification_data_t *notif);

        /**
         * Notification Removed event received
         *
         * ANCS_CLIENT_EVT_NOTIFICATION_SOURCE_NOTIF event has to be enabled in order for application
         * to receive this event.
         *
         * \param [in] client  client instance
         * \param [in] uid     notification UID
         *
         */
        void (* notification_removed) (ble_client_t *client, uint32_t uid);

        /**
         * Notification attribute received
         *
         * Called as response for ancs_client_get_notification_attr() when requested attribute data
         * is received.
         *
         * \param [in] client  client instance
         * \param [in] uid     notification UID
         * \param [in] attr    attribute ID
         * \param [in] value   attribute value (null-terminated string)
         *
         */
        void (* notification_attr) (ble_client_t *client, uint32_t uid,
                                                        ancs_notification_attr_t attr, char *value);

        /**
         * Notification attributes request completed
         *
         * Called as response for ancs_client_get_notification_attr() when all requested attributes
         * data is received.
         *
         * \param [in] client  client instance
         * \param [in] uid     notification UID
         * \param [in] status  operation status
         *
         */
        void (* get_notification_attr_completed) (ble_client_t *client, uint32_t uid,
                                                                                att_error_t status);

        /**
         * Application attribute received
         *
         * Called as response for ancs_client_get_application_attr() when requested attribute data
         * is received.
         *
         * \param [in] client  client instance
         * \param [in] app_id  application ID
         * \param [in] attr    attribute ID
         * \param [in] value   attribute value (null-terminated string)
         *
         */
        void (* application_attr) (ble_client_t *client, const char *app_id,
                                                        ancs_application_attr_t attr, char *value);

        /**
         * Application attributes request completed
         *
         * Called as response for ancs_client_get_application_attr() when all requested attributes
         * data is received.
         *
         * \param [in] client  client instance
         * \param [in] app_id  application ID
         * \param [in] status  operation status
         *
         */
        void (* get_application_attr_completed) (ble_client_t *client, const char *app_id,
                                                                                att_error_t status);

        /**
         * Notification action completed
         *
         * Called as response for ancs_client_perform_notification_action() when action is
         * performed.
         *
         * \param [in] client  client instance
         * \param [in] status  operation status
         *
         */
        void (* perform_notification_action_completed) (ble_client_t *client, att_error_t status);
} ancs_client_callbacks_t;

/**
 * Initialize client instance
 *
 * This should be called by application once it received browse event with ANCS service. Client
 * instance will be created and initialized as a result which will identify client in all other API
 * calls.
 *
 * After this call application should usually enable events on remote device using
 * ancs_client_set_event_state() calls.
 *
 * \note This function will fail if any of mandatory characteristics in found ANCS service are
 * missing or have incorrect properties.
 *
 * \param [in] cb   client callbacks
 * \param [in] evt  browse event instance
 *
 * \return client instance or NULL if failed
 *
 */
ble_client_t *ancs_client_init(const ancs_client_callbacks_t *cb,
                                                        const ble_evt_gattc_browse_svc_t *evt);

/**
 * Get client event state
 *
 * \p get_event_state_completed callback will be called when this operation completes.
 *
 * \param [in] client  client instance
 * \param [in] event   requested event
 *
 * \return true if operation was executed successfully, false otherwise
 *
 */
bool ancs_client_get_event_state(ble_client_t *client, ancs_client_evt_t event);

/**
 * Set client event state
 *
 * \p set_event_state_completed callback will be called when this operation completes.
 *
 * \param [in] client   client instance
 * \param [in] event    requested event
 * \param [in] enabled  event state (true if enabled, false otherwise)
 *
 * \return true if operation was executed successfully, false otherwise
 *
 */
bool ancs_client_set_event_state(ble_client_t *client, ancs_client_evt_t event, bool enabled);

/**
 * Check if client is busy with Control Point operation
 *
 * This allows to check whether operation stated by either ancs_client_get_notification_attr() or
 * ancs_client_get_application_attr() is in progress since these operations can be only executed
 * sequentially.
 *
 * \param [in] client  client instance
 *
 * \return true if client is busy, false otherwise
 *
 */
bool ancs_client_is_busy(ble_client_t *client);

/**
 * Cancel ongoing request, if any
 *
 * This can be used to cancel request started by either ancs_client_get_notification_attr() or
 * ancs_client_get_application_attr() if they are e.g. taking too long to complete.
 *
 * \param [in] client  client instance
 *
 * \return true if request is cancelled, false if there was no event to cancel
 *
 */
bool ancs_client_cancel_request(ble_client_t *client);

/**
 * Get notification attributes
 *
 * \p ... shall be series of Attribute ID elements constructed using \p ANCS_ATTR and
 * \p ANCS_ATTR_MAXLEN macros. It's caller responsibility to use proper macro depending on requested
 * attribute.
 *
 * For each attribute there will be \p notification_attr callback fired when attribute value is
 * received.
 *
 * After all attributes are received, \p get_notification_attr_completed callback will be fired.
 *
 * \param [in] client     client instance
 * \param [in] notif_uid  notification UID
 * \param [in] ...        requested attributes (ends with 0)
 *
 * \return true if operation was executed successfully, false otherwise
 *
 */
bool ancs_client_get_notification_attr(ble_client_t *client, uint32_t notif_uid, ...);

/**
 * Get application attributes
 *
 * \p ... shall be series of Attribute ID elements constructed using \p ANCS_ATTR and
 * \p ANCS_ATTR_MAXLEN macros. It's caller responsibility to use proper macro depending on requested
 * attribute.
 *
 * For each attribute there will be \p application_attr callback fired when attribute value is
 * received.
 *
 * After all attributes are received, \p get_application_attr_completed callback will be fired.
 *
 * \param [in] client  client instance
 * \param [in] app_id  application ID
 * \param [in] ...     requested attributes (ends with 0)
 *
 * \return true if operation was executed successfully, false otherwise
 *
 */
bool ancs_client_get_application_attr(ble_client_t *client, const char *app_id, ...);

/**
 * Performs action on notification
 *
 * Once successfully executed, \p perform_notification_action_callback will be fired once action is
 * completed.
 *
 * \param [in] client     client instance
 * \param [in] notif_uid  notification UID
 * \param [in] action     requested action
 *
 * \return true if operation was executed successfully, false otherwise
 *
 */
bool ancs_client_perform_notification_action(ble_client_t *client, uint32_t notif_uid,
                                                                        ancs_action_t action);

#endif /* ANCS_CLIENT_H_ */
