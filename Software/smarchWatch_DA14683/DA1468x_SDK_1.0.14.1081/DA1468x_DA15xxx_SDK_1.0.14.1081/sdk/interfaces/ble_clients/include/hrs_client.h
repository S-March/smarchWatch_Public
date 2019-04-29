/**
 ****************************************************************************************
 *
 * @file hrs_client.h
 *
 * @brief Heart Rate Service Client header file
 *
 * Copyright (C) 2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HRS_CLIENT_H
#define HRS_CLIENT_H

#include "ble_gattc.h"
#include "ble_client.h"

/**
 * \brief Body Sensor Location
 *
 * As defined by HRS specification.
 *
 */
typedef enum {
        HRS_CLIENT_BODY_SENSOR_LOC_OTHER        = 0, /**< Other */
        HRS_CLIENT_BODY_SENSOR_LOC_CHEST        = 1, /**< Chest */
        HRS_CLIENT_BODY_SENSOR_LOC_WRIST        = 2, /**< Wrist */
        HRS_CLIENT_BODY_SENSOR_LOC_FINGER       = 3, /**< Finger */
        HRS_CLIENT_BODY_SENSOR_LOC_HAND         = 4, /**< Hand */
        HRS_CLIENT_BODY_SENSOR_LOC_EAR_LOBE     = 5, /**< Ear Lobe */
        HRS_CLIENT_BODY_SENSOR_LOC_FOOT         = 6, /**< Foot */
} hrs_client_body_sensor_location_t;

/**
 * \brief Heart rate measurement information
 *
 * This corresponds to contents of Heart Rate Measurement characteristic value.
 *
 */
typedef struct {
        uint16_t        bpm;                    /**< Beats Per Minute value */
        bool            contact_supported;      /**< True if Sensor Contact feature is supported */
        bool            contact_detected;       /**< True if Sensor Contact is detected */
        bool            has_energy_expended;    /**< True if Energy Expended value is present */
        uint16_t        energy_expended;        /**< Energy Expended value */
        uint8_t         rr_num;                 /**< Number of RR-Interval values present */
        uint16_t        rr[];                   /**< RR-Interval values */
} hrs_client_measurement_t;

/**
 * \brief Client capabilities bit mask
 *
 */
typedef enum {
        /** Body Sensor Location Characteristic */
        HRS_CLIENT_CAP_BODY_SENSOR_LOCATION          = 0x01,
        /** Heart Rate Control Point Characteristic */
        HRS_CLIENT_CAP_HEART_RATE_CONTROL_POINT      = 0x02,
} hrs_client_cap_t;


/**
 * Event Characteristics (those which have indications/notifications as property)
 */
typedef enum {
        /** Heart Rate Measurement Characteristic */
        HRS_CLIENT_EVENT_HEART_RATE_MEASUREMENT_NOTIF  = 0x01,
} hrs_client_event_t;

/**
 * \brief Heart Rate Measurement notification callback
 *
 * Called when Client received Heart Rate Measurement notification from server.
 * In this callback OS_FREE(measurement) must be done to prevent leaking of memory caused by
 * OS_MALLOC for measurement with variable length of rr values while data receiving.
 *
 * \param [in] client           client instance
 * \param [in] measurement      heart rate measurements
 *
 */
typedef void (* hrs_client_heart_rate_measurement_notif_cb_t) (ble_client_t *client,
                                                        hrs_client_measurement_t *measurement);

/**
 * \brief Get event state completed callback
 *
 * \param [in] client           client instance
 * \param [in] event            indication/notification characteristic
 * \param [in] status           operation status
 * \param [in] enabled          state flag
 *
 */
typedef void (* hrs_client_get_event_state_completed_cb_t) (ble_client_t *client,
                                                                hrs_client_event_t event,
                                                                att_error_t status, bool enabled);

/**
 * \brief Set event state completed callback
 *
 * \param [in] client           client instance
 * \param [in] event            indication/notification characteristic
 * \param [in] status           operation status
 *
 */
typedef void (* hrs_client_set_event_state_completed_cb_t) (ble_client_t *client,
                                                                hrs_client_event_t event,
                                                                att_error_t status);

/**
 * \brief Read value of Body Sensor Location callback
 *
 * Indicates status of read Body Sensor Location value procedure.
 *
 * \param [in] client           client instance
 * \param [in] status           operation status
 * \param [in] location         body sensor location
 *
 */
typedef void (* hrs_client_read_body_sensor_location_cb_t) (ble_client_t *client, att_error_t status,
                                                        hrs_client_body_sensor_location_t location);

/**
 * \brief Reset the value of the Energy Expended callback
 *
 * Indicates status of reset Energy Expended value procedure.
 *
 * \param [in] client           client instance
 * \param [in] status           operation status
 *
 */
typedef void (* hrs_client_reset_energy_expended_completed_cb_t) (ble_client_t *client,
                                                                                att_error_t status);

/**
 * \brief HRS application callbacks
 *
 */
typedef struct {
        /** Heart Rate Measurement notification callback - triggered by notification */
        hrs_client_heart_rate_measurement_notif_cb_t    heart_rate_measurement_notif;
        /** Called once client read CCC descriptor of event characteristic notifications/indications */
        hrs_client_get_event_state_completed_cb_t       get_event_state_completed;
        /** Called once client enabled/disabled event characteristic notifications/indications */
        hrs_client_set_event_state_completed_cb_t       set_event_state_completed;
        /** Callback called when read of Body Sensor Location is completed */
        hrs_client_read_body_sensor_location_cb_t       read_body_sensor_location_completed;
        /** Callback called when reset energy expended is completed */
        hrs_client_reset_energy_expended_completed_cb_t reset_energy_expended_completed;
} hrs_client_callbacks_t;

/**
 * \brief Register Heart Rate Client instance
 *
 * Function registers new Heart Rate Client instance.
 *
 * \param [in] cb               client application callbacks
 * \param [in] evt              browse svc event with HR svc details
 *
 * \return client instance if success, otherwise NULL
 *
 */
ble_client_t *hrs_client_init(const hrs_client_callbacks_t *cb,
                                                        const ble_evt_gattc_browse_svc_t *evt);

/**
 * \brief Get client capabilities
 *
 * Function returns client capabilities enum
 *
 * \param [in] client           client instance
 *
 * \return Bit mask with client capabilities
 */
hrs_client_cap_t hrs_client_get_capabilities(ble_client_t *client);

/**
 * \brief Get event characteristic indication/notification state
 *
 * Function reads CCC descriptor of selected \p event. After successful operation
 * get_event_state callback will be called.
 *
 * \param [in] client           client instance
 * \param [in] event            selected notification/indication characteristic
 *
 * \return true if read request has been sent successfully, false if server doesn't support
 *         selected event characteristic or if operation is already in progress.
 *
 */
bool hrs_client_get_event_state(ble_client_t *client, hrs_client_event_t event);

/**
 * \brief Set event characteristic indication/notification state
 *
 * Function writes CCC descriptor of selected \p event. After successful operation
 * set_event_state callback will be called.
 *
 * \param [in] client           client instance
 * \param [in] event            selected notification/indication characteristic
 * \param [in] enable           state flag
 *
 * \return true if write request has been sent successfully, false if server doesn't support
 *         selected event characteristic or if operation is already in progress.
 *
 */
bool hrs_client_set_event_state(ble_client_t *client, hrs_client_event_t event, bool enable);

/**
 * \brief Read body sensor location
 *
 * Function triggers read Body Sensor Location characteristic. Once operation is completed, callback
 * hrs_client_read_body_sensor_location_cb_t will be called.
 *
 * \param [in] client           client instance
 *
 * \return true if request has been sent successfully, otherwise false
 *
 */
bool hrs_client_read_body_sensor_location(ble_client_t *client);

/**
 * \brief Reset Energy Expended value to 0
 *
 * Function reset Energy Expended field in HR Measurement characteristic to 0 by writing 0x01 value
 * to heart rate control point. Once operation is completed, callback
 * hrs_client_reset_energy_expended_completed_cb_t will be called.
 *
 * \param [in] client           client instance
 *
 * \return true if request has been sent successfully, otherwise false
 *
 */
bool hrs_client_reset_energy_expended(ble_client_t *client);

/**
 * \brief Initialize HRS Client instance from buffered (cached) data and register application
 * callbacks
 *
 * Function sematics is very similar to hrs_client_init() but internal data is initialized by
 * buffered context and the client is automatically added to active clients collection.
 *
 * \param [in] conn_idx         connection index
 * \param [in] cb               application callbacks
 * \param [in] data             buffered data
 * \param [in] length           buffer's length
 *
 * \return client instance when initialized properly, NULL otherwise
 */
ble_client_t *hrs_client_init_from_data(uint16_t conn_idx, const hrs_client_callbacks_t *cb,
                                                                const void *data, size_t length);

#endif /* HRS_CLIENT_H */
