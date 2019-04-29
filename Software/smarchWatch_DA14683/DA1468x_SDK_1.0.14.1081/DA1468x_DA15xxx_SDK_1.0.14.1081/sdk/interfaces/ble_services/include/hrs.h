/**
 ****************************************************************************************
 *
 * @file hrs.h
 *
 * @brief Heart Rate Service sample implementation API
 *
 * Copyright (C) 2015 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HRS_H_
#define HRS_H_

#include <stdbool.h>
#include <stdint.h>
#include "ble_service.h"

/**
 * Body Sensor Location
 *
 * As defined by HRS specification.
 *
 */
typedef enum {
        HRS_SENSOR_LOC_OTHER    = 0,
        HRS_SENSOR_LOC_CHEST    = 1,
        HRS_SENSOR_LOC_WRIST    = 2,
        HRS_SENSOR_LOC_FINGER   = 3,
        HRS_SENSOR_LOC_HAND     = 4,
        HRS_SENSOR_LOC_EAR_LOBE = 5,
        HRS_SENSOR_LOC_FOOT     = 6,
} hrs_body_sensor_location_t;

typedef void (* hrs_ee_reset_cb_t) (uint16_t conn_idx);

typedef void (* hrs_notification_changed_cb_t) (uint16_t conn_idx, bool enabled);

/**
 * HRS application callbacks
 */
typedef struct {
        /** Energy Expended reset requested from client */
        hrs_ee_reset_cb_t ee_reset;

        /** Notification status for Heart Rate Measurement changed by client */
        hrs_notification_changed_cb_t notif_changed;
} hrs_callbacks_t;

/**
 * Heart rate measurement information
 *
 * This corresponds to contents of Heart Rate Measurement characteristic value.
 *
 */
typedef struct {
        uint16_t        bpm;                    /**< BPM value */
        bool            contact_supported;      /**< true if Sensor Contact feature is supported */
        bool            contact_detected;       /**< true if sensor contact is detected */
        bool            has_energy_expended;    /**< true if Energy Expended value is present */
        uint16_t        energy_expended;        /**< Energy Expended value */
        uint8_t         rr_num;                 /**< number of RR-Interval values present */
        uint16_t        rr[];                   /**< RR-Interval values */
} hrs_measurement_t;

/**
 * Register Heart Rate Service instance
 *
 * \param [in] location  sensor location
 * \param [in] cb        application callbacks
 *
 * \return service instance
 *
 */
ble_service_t *hrs_init(hrs_body_sensor_location_t location, const hrs_callbacks_t *cb);

/**
 * Notify heart rate measurement to client
 *
 * Notification will only be sent if given client enabled notifications before.
 *
 * \param [in] svc       service instance
 * \param [in] conn_idx  connection index
 * \param [in] meas      heart rate measurement
 *
 */
void hrs_notify_measurement(ble_service_t *svc, uint16_t conn_idx, hrs_measurement_t *meas);

/**
 * Notify heart rate measurement to all connected clients
 *
 * This is equivalent for calling hrs_notify_measurement() for each connected client.
 *
 * \param [in] svc   service instance
 * \param [in] meas  heart rate measurement
 *
 */
void hrs_notify_measurement_all(ble_service_t *svc, hrs_measurement_t *meas);

#endif /* HRS_H_ */
