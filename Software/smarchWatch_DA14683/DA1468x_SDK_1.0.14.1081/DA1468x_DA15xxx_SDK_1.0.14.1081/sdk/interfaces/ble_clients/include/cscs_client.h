/**
 ****************************************************************************************
 *
 * @file cscs_client.h
 *
 * @brief Cycling Speed and Cadence Service Client API
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef CSCS_CLIENT_H_
#define CSCS_CLIENT_H_

#include <string.h>
#include "ble_gattc.h"
#include "ble_client.h"

/**
 * CSC features
 */
typedef enum {
        /** Wheel Revolution Data Supported */
        CSCS_CLIENT_FEATURE_WHEEL_REVOLUTION_DATA = 0x01,
        /** Crank Revolution Data Supported */
        CSCS_CLIENT_FEATURE_CRANK_REVOLUTION_DATA = 0x02,
        /** Multiple Sensor Locations Supported */
        CSCS_CLIENT_FEATURE_MULTIPLE_SENSOR_LOCATIONS = 0x04,
} cscs_client_feature_t;

/**
 * Status
 */
typedef enum {
        /** Status success */
        CSCS_CLIENT_STATUS_SUCCESS = 0x01,
        /** Status opcode not supported */
        CSCS_CLIENT_STATUS_OPCODE_NOT_SUPPORTED = 0x02,
        /** Status invalid parameters */
        CSCS_CLIENT_STATUS_INVALID_PARAM = 0x03,
        /** Status operation failed */
        CSCS_CLIENT_STATUS_OPERATION_FAILED = 0x04,
        /** Status operation in progress */
        CSCS_CLIENT_STATUS_OPERATION_IN_PROGRESS = 0x80,
        /** Status CCC descriptor improperly configured */
        CSCS_CLIENT_STATUS_IMPROPERLY_CONFIGURED = 0x81,
        /** Status SC Control Point operation timeout */
        CSCS_CLIENT_STATUS_TIMEOUT = 0x100,
} cscs_client_status_t;

/**
 * Sensor locations
 */
typedef enum {
        /** Location Other */
        CSCS_CLIENT_SENSOR_LOCATION_OTHER = 0x00,
        /** Location Top of shoe */
        CSCS_CLIENT_SENSOR_LOCATION_TOP_OF_SHOE = 0x01,
        /** Location In shoe */
        CSCS_CLIENT_SENSOR_LOCATION_IN_SHOE = 0x02,
        /** Location Hip */
        CSCS_CLIENT_SENSOR_LOCATION_HIP = 0x03,
        /** Location Front wheel */
        CSCS_CLIENT_SENSOR_LOCATION_FRONT_WHEEL = 0x04,
        /** Location Left crank */
        CSCS_CLIENT_SENSOR_LOCATION_LEFT_CRANK = 0x05,
        /** Location Right crank */
        CSCS_CLIENT_SENSOR_LOCATION_RIGHT_CRANK = 0x06,
        /** Location Left pedal */
        CSCS_CLIENT_SENSOR_LOCATION_LEFT_PEDAL = 0x07,
        /** Location Right pedal */
        CSCS_CLIENT_SENSOR_LOCATION_RIGHT_PEDAL = 0x08,
        /** Location Front hub */
        CSCS_CLIENT_SENSOR_LOCATION_FRONT_HUB = 0x09,
        /** Location Rear dropout */
        CSCS_CLIENT_SENSOR_LOCATION_REAR_DROPOUT = 0x0a,
        /** Location Chainstay */
        CSCS_CLIENT_SENSOR_LOCATION_CHAINSTAY = 0x0b,
        /** Location Rear wheel */
        CSCS_CLIENT_SENSOR_LOCATION_REAR_WHEEL = 0x0c,
        /** Location Rear hub */
        CSCS_CLIENT_SENSOR_LOCATION_REAR_HUB = 0x0d,
        /** Location Chest */
        CSCS_CLIENT_SENSOR_LOCATION_CHEST = 0x0e,
} cscs_client_sensor_location_t;

/**
 * CSC measurement
 *
 * It corresponds to CSC measurement characteristic value
 */
typedef struct {
        /** Indicates if wheel revolution data is present */
        bool wheel_revolution_data_present : 1;
        /** Indicates if crank revolution data is present */
        bool crank_revolution_data_present : 1;
        /** Cumulative wheel revolutions count. Field exists if wheel revolution data present */
        uint32_t cumulative_wheel_revolutions;
        /** Last wheel event time. Unit: 1/1024 s. Field exists if wheel revolution data present */
        uint16_t last_wheel_event_time;
        /** Cumulative crank revolutions count. Field exists if crank revolution data present */
        uint16_t cumulative_crank_revolutions;
        /** Last crank event time. Unit: 1/1024 s. Field exists if crank revolution data present */
        uint16_t last_crank_event_time;
} cscs_client_measurement_t;

/**
 * Event Characteristics (those which have indications/notifications as property)
 */
typedef enum {
        /** Cycling Speed and Cadence Measurement Characteristic */
        CSCS_CLIENT_EVENT_CSC_MEASUREMENT_NOTIF = 0x01,
} cscs_client_event_t;

/**
 * Capabilities (supported characteristic) bit mask
 */
typedef enum {
        /** Sensor Location Characteristic */
        CSCS_CLIENT_CAP_SENSOR_LOCATION      = 0x01,
} cscs_client_cap_t;

/**
* \brief Read feature completed callback
*
* \param [in] client                    client instance
* \param [in] status                    operation status
* \param [in] features                  supported features
*
*/
typedef void (* cscs_client_read_csc_features_completed_cb_t) (ble_client_t *client,
                                                        att_error_t status, uint16_t features);

/**
* \brief Get event state completed callback
*
* \param [in] client                    client instance
* \param [in] event                     indication/notification characteristic
* \param [in] status                    operation status
* \param [in] enabled                   state flag
*
*/
typedef void (* cscs_client_get_event_state_completed_cb_t) (ble_client_t *client,
                                cscs_client_event_t event, att_error_t status, bool enabled);

/**
* \brief Set event state completed callback
*
* \param [in] client                    client instance
* \param [in] event                     indication/notification characteristic
* \param [in] status                    operation status
*
*/
typedef void (* cscs_client_set_event_state_completed_cb_t) (ble_client_t *client,
                                                cscs_client_event_t event, att_error_t status);

/**
* \brief Get control point state completed callback
*
* \param [in] client                    client instance
* \param [in] status                    operation status
* \param [in] enabled                   state flag
*
*/
typedef void (* cscs_client_get_sc_control_point_state_completed_cb_t) (ble_client_t *client,
                                                                att_error_t status, bool enabled);

/**
* \brief Set control point state completed callback
*
* \param [in] client                    client instance
* \param [in] status                    operation status
*
*/
typedef void (* cscs_client_set_sc_control_point_state_completed_cb_t) (ble_client_t *client,
                                                                                att_error_t status);

/**
 * \brief Update sensor location callback
 *
 * Indicates status of update sensor location procedure.
 *
 * \param       [in]    client          CSCS Client instance
 * \param       [in]    status          operation status
 */
typedef void (* cscs_client_update_sensor_location_completed_cb_t) (ble_client_t *client,
                                                                cscs_client_status_t status);

/**
 * \brief Read sensor location callback
 *
 * Indicates current sensor location
 *
 * \param       [in]    client          CSCS Client instance
 * \param       [in]    status          operation status
 * \param       [in]    location        sensor location
 */
typedef void (* cscs_client_read_sensor_location_completed_cb_t) (ble_client_t *client,
                                                        att_error_t status,
                                                        cscs_client_sensor_location_t location);

/**
 * \brief Set cumulative value callback
 *
 * Indicates status of set cumulative value procedure.
 *
 * \param       [in]    client          CSCS Client instance
 * \param       [in]    status          operation status
 */
typedef void (* cscs_client_set_cumulative_value_completed_cb_t) (ble_client_t *client,
                                                                cscs_client_status_t status);

/**
 * \brief Request supported sensor locations callback
 *
 * Indicates supported sensor locations of CSC Sensor. Locations array is valid in case of success.
 *
 * \param       [in]    client          CSCS Client instance
 * \param       [in]    status          operation status
 * \param       [in]    locations_count number of supported locations
 * \param       [in]    locations       array of supported locations
 */
typedef void (* cscs_client_request_supported_sensor_locations_completed_cb_t)
                                        (ble_client_t *client, cscs_client_status_t status,
                                        uint8_t locations_count, const uint8_t *locations);

/**
 * \brief CSC measurement callback
 *
 * Callback with CSC measurement
 *
 * \param       [in]    client          CSCS Client instance
 * \param       [in[    measurement     CSC measurement
 */
typedef void (* cscs_client_csc_measurement_cb_t) (ble_client_t *client,
                                                const cscs_client_measurement_t *measurement);

/**
 * Application callbacks
 */
typedef struct {
        /** Callback with features */
        cscs_client_read_csc_features_completed_cb_t read_csc_features_completed;
        /** Callback with state of event's CCC descriptor */
        cscs_client_get_event_state_completed_cb_t get_event_state_completed;
        /** Callback with status of enabling/disabling event state */
        cscs_client_set_event_state_completed_cb_t set_event_state_completed;
        /** Callback with state of SC Control Point's CCC descriptor */
        cscs_client_get_sc_control_point_state_completed_cb_t get_sc_control_point_state_completed;
        /** Callback with status of enabling/disabling SC Control Point state */
        cscs_client_set_sc_control_point_state_completed_cb_t set_sc_control_point_state_completed;
        /** Callback with current sensor location */
        cscs_client_read_sensor_location_completed_cb_t read_sensor_location_completed;
        /** Callback with status of update sensor location procedure */
        cscs_client_update_sensor_location_completed_cb_t update_sensor_location_completed;
        /** Callback with status of set cumulative value procedure */
        cscs_client_set_cumulative_value_completed_cb_t set_cumulative_value_completed;
        /** Callback with status of request supported sensor locations procedure */
        cscs_client_request_supported_sensor_locations_completed_cb_t
                                                request_supported_sensor_locations_completed;
        /** Callback with CSC measurement */
        cscs_client_csc_measurement_cb_t csc_measurement;
} cscs_client_callbacks_t;

/**
 * \brief Register CSC Client instance
 *
 * Funtion registers new CSC Client instance
 *
 * \param       [in]    cb              application callbacks
 * \param       [in]    evt             browse svc event with HID svc details
 *
 * \return client instance if success, otherwise NULL
 */
ble_client_t *cscs_client_init(const cscs_client_callbacks_t *cb,
                                                        const ble_evt_gattc_browse_svc_t *evt);

/**
 * \brief Read CSC features
 *
 * Function reads mandatory CSC Feature characteristic. After successful operation
 * read_features_completed callback will be called.
 *
 * \param [in] client           client instance
 *
 * \return true if request sent successfully, otherwise false
 */
bool cscs_client_read_csc_features(ble_client_t *client);

/**
 * \brief Get capabilities
 *
 * Function returns capabilities enum
 *
 * \param [in] client           client instance
 *
 * \return Bit mask with capabilities
 */
cscs_client_cap_t cscs_client_get_capabilities(ble_client_t *client);

/**
 * \brief Get event state
 *
 * Function reads CCC descriptor of specified characteristic. After successful operation
 * get_event_state_completed callback will be called.
 *
 * \param [in] client           client instance
 * \param [in] event            indication/notification characteristic
 *
 * \return true if read request has been sent successfully, otherwise false
 *
 */
bool cscs_client_get_event_state(ble_client_t *client, cscs_client_event_t event);

/**
 * \brief Set event state
 *
 * Function writes CCC descriptor of specified characteristic. After successful operation
 * set_event_state_completed callback will be called.
 *
 * \param [in] client           client instance
 * \param [in] event            indication/notification characteristic
 * \param [in] enable           state flag
 *
 * \return true if write request has been sent successfully, otherwise false
 *
 */
bool cscs_client_set_event_state(ble_client_t *client, cscs_client_event_t event, bool enable);

/**
 * \brief Get control point state
 *
 * Function reads CCC descriptor of SC Control Point characteristic. After successful
 * operation get_sc_control_point_state_completed callback will be called.
 *
 * \param [in] client           client instance
 *
 * \return true if read request has been sent successfully, false otherwise
 *
 */
bool cscs_client_get_sc_control_point_ind_state(ble_client_t *client);

/**
 * \brief Set control point state
 *
 * Function writes CCC descriptor of SC Control Point characteristic. After successful
 * operation set_sc_control_point_state_completed callback will be called.
 *
 * \param [in] client           client instance
 * \param [in] enable           state flag
 *
 * \return true if write request has been sent successfully, false otherwise.
 *
 */
bool cscs_client_set_sc_control_point_ind_state(ble_client_t *client, bool enable);

/**
 * \brief Read sensor location
 *
 * Function triggers read sensor location characteristic
 *
 * \param       [in]    client          CSC Client instance
 *
 * \return true if request has been send successfully, otherwise false
 */
bool cscs_client_read_sensor_location(ble_client_t *client);

/**
 * \brief Update sensor location
 *
 * Function triggers sensor location update using SC Control Point Characteristic.
 * Once operation is completed, callback update_sensor_location will be called with proper status.
 *
 * \param       [in]    client          CSC Client instance
 * \param       [in]    location        requested location
 *
 * \return true if request has been send successfully, otherwise false
 */
bool cscs_client_update_sensor_location(ble_client_t *client,
                                                        cscs_client_sensor_location_t location);

/**
 * \brief Request supported sensor locations
 *
 * Function requests supported sensor location using SC Control Point Characteristic.
 * Once operation is completed, callback request_supported_sensor_locations wil be called with
 * proper status.
 *
 * \param       [in]    client          CSC Client instance
 *
 * \return true if request has been send successfully, otherwise false
 */
bool cscs_client_request_supported_sensor_locations(ble_client_t *client);

/**
 * \brief Set cumulative value
 *
 * Function sets wheel cumulative value of CSC Sensor using SC Control Point Characteristic.
 * Once operation is completed, callback set_cumulative_value wil be called.
 *
 * \param       [in]    client          CSC Client instance
 * \param       [in]    value           cumulative value
 *
 * \return true if request has been send successfully, otherwise false
 */
bool cscs_client_set_cumulative_value(ble_client_t *client, uint32_t value);

/**
 * \brief Control point operation timeout
 *
 * Function should be called when the timer related with operation timeout of specific CSCS Client's
 * SC Control Point is expired. This function is necessary for proper handling of SC Control Point
 * indication timeout.
 *
 * \param       [in]    client          client instance
 *
 */
void cscs_client_sc_control_point_timeout(ble_client_t *client);

/**
 * \brief Initialize CSCP Client instance from buffered (cached) data and register application
 * callbacks
 *
 * Function sematics is very similar to cscs_client_init() but internal data is initialized by
 * buffered context and the client is automatically added to active clients collection.
 *
 * \param [in] conn_idx         connection index
 * \param [in] cb               application callbacks
 * \param [in] data             buffered data
 * \param [in] length           buffer's length
 *
 * \return client instance when initialized properly, NULL otherwise
 */
ble_client_t *cscs_client_init_from_data(uint16_t conn_idx, const cscs_client_callbacks_t *cb,
                                                                   const void *data, size_t length);
#endif /* CSCS_CLIENT_H_ */
