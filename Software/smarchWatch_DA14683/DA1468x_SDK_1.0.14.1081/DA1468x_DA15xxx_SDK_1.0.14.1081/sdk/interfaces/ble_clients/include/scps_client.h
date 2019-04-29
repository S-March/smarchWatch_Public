/**
 ****************************************************************************************
 *
 * @file scps_client.h
 *
 * @brief Scan Parameters Service Client header file
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SCPS_CLIENT_H
#define SCPS_CLIENT_H

/**
 * Capabilities (supported characteristics)
 */
typedef enum {
        /** Refresh notify */
        SCPS_CLIENT_EVENT_REFRESH_NOTIF = 0x01,
} scps_client_event_t;

/**
 * Supported characteristics bit mask
 */
typedef enum {
        /** Refresh characteristic */
        SCPS_CLIENT_CAP_REFRESH = 0x01,
} scps_client_cap_t;

/**
 * \brief Set event state completed callback
 *
 * Called When particular characteristic's event state has been set
 *
 * \param [in] scps_client      SCPS Client instance
 * \param [in] event            Event type
 * \param [in] status           ATT status of operation
 */
typedef void (* scps_client_set_event_state_completed_cb_t) (ble_client_t *scps_client,
                                                                        scps_client_event_t event,
                                                                        att_error_t status);

/**
 * \brief Get event state completed callback
 *
 * Called When particular characteristic's event state has been returned by server
 *
 * \param [in] scps_client      SCPS Client instance
 * \param [in] event            Event type
 * \param [in] status           ATT status of operation
 * \param [in] enabled          State flag
 */
typedef void (* scps_client_get_event_state_completed_cb_t) (ble_client_t *scps_client,
                                                                scps_client_event_t event,
                                                                att_error_t status, bool enabled);

/**
 * Refresh characteristic notification callback
 *
 * Called when refresh notification has been received
 *
 * \param [in] scps_client      SCPS Client instance
 */
typedef void (* scps_client_refresh_notif_cb_t) (ble_client_t *scps_client);

/**
 * SCPS Client callbacks
 */
typedef struct {
        /** Called when server requires the Scan Interval Window to be written */
        scps_client_refresh_notif_cb_t                  refresh_notif;
        /** Called once client enabled/disabled Refresh characteristic notifications */
        scps_client_set_event_state_completed_cb_t      set_event_state_completed;
        /** Called once client read CCC descriptor of Refresh characteristic */
        scps_client_get_event_state_completed_cb_t      get_event_state_completed;
} scps_client_callbacks_t;

/**
 * \brief Register SCPS Client instance
 *
 * Function registers SCPS Client
 *
 * \param [in] cb       application callbacks
 * \param [in] evt      browse svc event with Battery Service details
 *
 * \return client instance in case of success, otherwise NULL.
 *
 */
ble_client_t *scps_client_init(const scps_client_callbacks_t *cb,
                                                        const ble_evt_gattc_browse_svc_t *evt);

/**
 * \brief Get capabilites
 *
 * Function returns bit mask with supported capabilities
 *
 * \param [in] scps_client      client instance
 *
 * \return Bit mask with supported characteristics
 */
scps_client_cap_t scps_client_get_capabilities(ble_client_t *scps_client);

/**
 * \brief Get event state
 *
 * Function reads CCC descriptor. After successful operation get_event_state callback will be
 * called
 *
 * \param [in] scps_client      client instance
 * \param [in] event            event type
 *
 * \return true if read request has been sent successfully, false if server doesn't support
 * specified event.
 */
bool scps_client_get_event_state(ble_client_t *scps_client, scps_client_event_t event);

/**
 * \brief Set Refresh characteristic notification state
 *
 * Function writes to CCC descriptor. After successful operation set_notif_state callback will be
 * called
 *
 * \param [in] scps_client      client instance
 * \param [in] event            event type
 * \param [in] enable           state flag
 *
 * \return true if write request has been sent successfully, false if server doesn't support
 * specified event.
 */
bool scps_client_set_event_state(ble_client_t *scps_client, scps_client_event_t event,
                                                                                bool enable);

/**
 * \brief Write Scan Interval Window
 *
 * Function send Write Command to Scan Interval Window Characteristic
 *
 * \param [in] scps_client      client instance
 * \param [in] scan_interval    le scan interval
 * \param [in] scan_window      le scan window
 *
 * \return true if write command has been sent successfully, false otherwise.
 */
bool scps_client_write_scan_interval_window(ble_client_t *scps_client, uint16_t scan_interval,
                                                                        uint16_t scan_window);

#endif /* SCPS_CLIENT_H */
