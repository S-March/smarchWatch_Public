/**
 ****************************************************************************************
 *
 * @file bas_client.h
 *
 * @brief Battery Service Client header file
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BAS_CLIENT_H
#define BAS_CLIENT_H

/**
 * Characteristics containing CCC descriptors - may be configured for notifications or
 * indications
 */
typedef enum {
        /** Battery level notifications */
        BAS_CLIENT_EVENT_BATTERY_LEVEL_NOTIFY = 0x01,
} bas_client_event_t;

/**
 * Capabilities (supported characteristics)
 */
typedef enum {
        /** Battery level characteristic supports notifications */
        BAS_CLIENT_CAP_BATTERY_LEVEL_NOTIFICATION = 0x01,
} bas_client_cap_t;

/**
 * \brief Read battery level completed callback
 *
 * It is called when read response is received from server.
 *
 * \param [in] bas_client       BAS Client instance
 * \param [in] status           ATT status of operation
 * \param [in] level            battery level
 */
typedef void (* bas_client_read_battery_level_completed_cb_t) (ble_client_t *bas_client,
                                                                att_error_t status, uint8_t level);

/**
 * \brief Set event state completed callback
 *
 * Called When particular characteristic's event state has been set
 *
 * \param [in] bas_client       BAS Client instance
 * \param [in] event            Event type
 * \param [in] status           ATT status of operation
 */
typedef void (* bas_client_set_event_state_completed_cb_t) (ble_client_t *bas_client,
                                                                        bas_client_event_t event,
                                                                        att_error_t status);

/**
 * \brief Get event state completed callback
 *
 * Called When particular characteristic's event state has been returned by server
 *
 * \param [in] bas_client       BAS Client instance
 * \param [in] event            Event type
 * \param [in] status           ATT status of operation
 * \param [in] enabled          State flag
 */
typedef void (* bas_client_get_event_state_completed_cb_t) (ble_client_t *bas_client,
                                                                bas_client_event_t event,
                                                                att_error_t status, bool enabled);

/**
 * Battery level notification callback
 *
 * Called when battery level notification has been received
 *
 * \param [in] bas_client       BAS Client instance
 * \param [in] level            battery level
 */
typedef void (* bas_client_battery_level_notif_cb_t) (ble_client_t *bas_client, uint8_t level);

/**
 * Application callbacks
 */
typedef struct {
        /** Called once client finished read battery level */
        bas_client_read_battery_level_completed_cb_t    read_battery_level_completed;
        /** Called once set event state completed */
        bas_client_set_event_state_completed_cb_t       set_event_state_completed;
        /** Called once get event state completed */
        bas_client_get_event_state_completed_cb_t       get_event_state_completed;
        /** Called once client received Battery Level notification */
        bas_client_battery_level_notif_cb_t             battery_level_notif;
} bas_client_callbacks_t;

/**
 * \brief Register BAS Client instance
 *
 * Function registers BAS Client
 *
 * \param [in] cb       application callbacks
 * \param [in] evt      browse svc event with Battery Service details
 *
 * \return client instance
 *
 */
ble_client_t *bas_client_init(const bas_client_callbacks_t *cb,
                                                        const ble_evt_gattc_browse_svc_t *evt);

/**
 * \brief Get BAS Client capabilities
 *
 * Function returns bit mask with BAS Client capabilities.
 *
 * \param [in] bas_client       client instance
 *
 * \return capabilities bitmask
 */
bas_client_cap_t bas_client_get_capabilities(ble_client_t *bas_client);

/**
 * \brief Read of battery level
 *
 * Function trigger read of battery level.
 *
 * \param [in] bas_client       client instance
 *
 * \return true if read request has been sent successfully, false otherwise.
 */
bool bas_client_read_battery_level(ble_client_t *bas_client);

/**
 * \brief Set event state
 *
 * Function set event state for given characteristic
 *
 * \param [in] bas_client       client instance
 * \param [in] event            event type
 * \param [in] enable           enable/disable flag
 *
 * \return true if write request to CCC descriptor has been sent successfully, false otherwise.
 */
bool bas_client_set_event_state(ble_client_t *bas_client, bas_client_event_t event, bool enable);

/**
 * \brief Get event state
 *
 * Functions reads CCC descriptor of given characteristic.
 *
 * \param [in] bas_client       client instance
 * \param [in] event            event type
 *
 * \return true if read request to CCC descriptor has been sent successfully, false otherwise.
 */
bool bas_client_get_event_state(ble_client_t *bas_client, bas_client_event_t event);

#endif /* BAS_CLIENT_H */
