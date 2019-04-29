/**
 ****************************************************************************************
 *
 * @file hts.h
 *
 * @brief Health Thermometer Service API
 *
 * Copyright (C) 2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HTS_H_
#define HTS_H_

#include <stdint.h>
#include "ble_service.h"
#include "svc_types.h"

/**
 * \brief Callback function, called when Temperature Measurement indication configuration was
 * changed by client
 *
 * \param [in] conn_idx         connection index
 * \param [in] enabled          indication status
 *
 */
typedef void (* hts_temperature_meas_indication_changed_cb_t) (uint16_t conn_idx, bool enabled);

/**
 * \brief Callback function, called when confirmation to Temperature Measurement indication has been
 * received
 *
 * \param [in] conn_idx         connection index
 * \param [in] status           indication status
 *
 */
typedef void (* hts_temperature_meas_indication_sent_cb_t) (uint16_t conn_idx, bool status);

/**
 * \brief Callback function, called when Measurement Interval indication configuration was changed
 * by client
 *
 * \param [in] conn_idx         connection index
 * \param [in] enabled          indication status
 *
 */
typedef void (* hts_meas_interval_indication_changed_cb_t) (uint16_t conn_idx, bool enabled);

/**
 * \brief Callback function, called when confirmation to Measurement Interval indication has been
 * received
 *
 * \param [in] conn_idx         connection index
 * \param [in] status           indication status
 *
 */
typedef void (* hts_meas_interval_indication_sent_cb_t) (uint16_t conn_idx, bool status);

/**
 * \brief Callback function, called when Intermediate Temperature notification configuration was
 * changed by client
 *
 * \param [in] conn_idx         connection index
 * \param [in] enabled          notification status
 *
 */
typedef void (* hts_interm_temperature_notification_changed_cb_t) (uint16_t conn_idx, bool enabled);

/**
 * \brief Callback function, called when Intermediate Temperature notification has been sent to the
 * client
 *
 * \param [in] conn_idx         connection index
 * \param [in] status           indication status
 *
 */
typedef void (* hts_interm_temperature_notification_sent_cb_t) (uint16_t conn_idx, bool status);

/**
 * \brief Callback function, called when Measurement Interval value was set by client
 *
 * \p hts_set_meas_interval_cfm confirmation should be fired inside of this callback.
 *
 * \param [in] conn_idx         connection index
 * \param [in] interval         time between measurements
 *
 */
typedef void (* hts_meas_interval_set_cb_t) (uint16_t conn_idx, uint16_t interval);

/**
 * HTS application callbacks
 */
typedef struct {
        /** Indication status for Temperature Measurement changed by client */
        hts_temperature_meas_indication_changed_cb_t            temp_meas_indication_changed;
        /** Client response for Temperature Measurement indication */
        hts_temperature_meas_indication_sent_cb_t               temp_meas_indication_sent;
        /** Indication status for Measurement Interval changed by client */
        hts_meas_interval_indication_changed_cb_t               meas_interval_indication_changed;
        /** Client response for Measurement Interval indication */
        hts_meas_interval_indication_sent_cb_t                  meas_interval_indication_sent;
        /** Notification status for Intermediate Temperature changed by client */
        hts_interm_temperature_notification_changed_cb_t        interm_temp_notification_changed;
        /** Client response for Intermediate Temperature notification */
        hts_interm_temperature_notification_sent_cb_t           interm_temp_notification_sent;
        /** Measurement Interval was set by client */
        hts_meas_interval_set_cb_t                              meas_interval_set;
} hts_callbacks_t;

/**
 * \brief Temperature type
 *
 * Used to describe the type of temperature measurement with regards to the location on the human
 * body at which the temperature was measured.
 */
typedef enum {
        HTS_TEMP_TYPE_ARMPIT            = 0x01, /**< Armpit */
        HTS_TEMP_TYPE_BODY              = 0x02, /**< Body (general) */
        HTS_TEMP_TYPE_EAR               = 0x03, /**< Ear (usually ear lobe) */
        HTS_TEMP_TYPE_FINGER            = 0x04, /**< Finger */
        HTS_TEMP_TYPE_GASTRO_TRACT      = 0x05, /**< Gastro-intestinal Tract */
        HTS_TEMP_TYPE_MOUTH             = 0x06, /**< Mouth */
        HTS_TEMP_TYPE_RECTUM            = 0x07, /**< Rectum */
        HTS_TEMP_TYPE_TOE               = 0x08, /**< Toe */
        HTS_TEMP_TYPE_TYMPANUM          = 0x09, /**< Tympanum (ear drum) */
} hts_temp_type_t;

/**
 * \brief Temperature features
 */
typedef enum {
        /** Temperature Type characteristic */
        HTS_FEATURE_TEMPERATURE_TYPE                    = 0x01,
        /** Intermediate Temperature characteristic */
        HTS_FEATURE_INTERMEDIATE_TEMP                   = 0x02,
        /** Measurement Interval */
        HTS_FEATURE_MEASUREMENT_INTERVAL                = 0x04,
        /** Measurement Interval Write */
        HTS_FEATURE_MEASUREMENT_INTERVAL_WRITABLE       = 0x08,
        /** Measurement Interval Indicate */
        HTS_FEATURE_MEASUREMENT_INTERVAL_INDICATIONS    = 0x10,
} hts_feature_t;

/**
 * \brief Temperature unit
 */
typedef enum {
        HTS_TEMP_UNIT_CELSIUS           = 0x00, /**< Temperature in Celsius */
        HTS_TEMP_UNIT_FAHRENHEIT        = 0x01, /**< Temperature in Fahrenheit */
} hts_temp_unit_t;

typedef struct {
        /** Temperature Unit (Celsius or Fahrenheit) */
        hts_temp_unit_t         unit;
        /** Temperature value */
        svc_ieee11073_float_t   temperature;
        /** Time stamp included in measurements */
        bool                    has_time_stamp : 1;
        /** Temperature Type included in measurements. Valid only for intermediate temperature */
        bool                    has_temp_type : 1;
        /** Time of measurement */
        svc_date_time_t         time_stamp;
        /** Temperature type - location on the human body where temperature was measured */
        hts_temp_type_t         temp_type;
} hts_temp_measurement_t;

/**
 * Health Temperature Service config used during initialization of service
 */
typedef struct {
        /** Service features */
        hts_feature_t features;
        /** Temperature type - valid if static feature is set */
        hts_temp_type_t type;
        /**
         * Measurements interval (time between measurements) - valid if temperature type feature
         * is set
         */
        uint16_t init_interval;
        /**
         * Lower inclusive value of the interval range - measurement interval must be writable to
         * use it
         */
        uint16_t interval_bound_low;
        /**
         * Higher inclusive value of the interval range - measurement interval must be writable to
         * use it
         */
        uint16_t interval_bound_high;
} hts_config_t;

/**
 * \brief Initialize Health Thermometer Service instance
 *
 * This function initializes Health Thermometer Service with a given set of flags (temperature unit,
 * whether time stamp and temperature type are supported).
 *
 * \param [in] config           service configuration
 * \param [in] hts_config       Health Temperature Service specific config
 * \param [in] cb               application callbacks
 *
 * \return service instance
 *
 */
ble_service_t *hts_init(const ble_service_config_t *config, const hts_config_t *hts_config,
                                                                        const hts_callbacks_t *cb);

/**
 * \brief Send temperature indication to client
 *
 * Send indication of the Temperature Measurement characteristic value to a client (if such
 * indications have been previously enabled).
 *
 * \param [in] svc              service instance
 * \param [in] conn_idx         connection index
 * \param [in] measurement      temperature measurements
 *
 * \return true if indication was sent properly, false otherwise
 *
 */
bool hts_indicate_temperature(ble_service_t *svc, uint16_t conn_idx,
                                                        const hts_temp_measurement_t *measurement);

/**
 * \brief Send measurement interval indication to client
 *
 * Send indication of the Measurement Interval characteristic value to a client (if such indications
 * have been previously enabled).
 *
 * \param [in] svc              service instance
 * \param [in] conn_idx         connection index
 *
 * \return true if indication was sent properly, false otherwise
 *
 */
bool hts_indicate_measurement_interval(ble_service_t *svc, uint16_t conn_idx);

/**
 * \brief Set measurement interval value
 *
 * \param [in] svc              service instance
 * \param [in] interval         time between measurements
 *
 * \return true if interval was set properly, false otherwise
 *
 */
bool hts_set_measurement_interval(ble_service_t *svc, uint16_t interval);

/**
 * \brief Notify intermediate temperature measurement to client
 *
 * Send notification of the Intermediate Temperature characteristic value to a client (if such
 * notifications have been previously enabled).
 *
 * \param [in] svc              service instance
 * \param [in] conn_idx         connection index
 * \param [in] measurement      intermediate measurement
 *
 * \return true if notification was sent properly, false otherwise
 *
 */
bool hts_notify_interm_temperature(ble_service_t *svc, uint16_t conn_idx,
                                                        const hts_temp_measurement_t *measurement);

/**
 * \brief Confirmation function for \p meas_interval_set callback
 *
 * Should be called by \p meas_interval_set callback when a client writes the value of the
 * Measurement Interval characteristic.
 *
 * \param [in] svc              service instance
 * \param [in] conn_idx         connection index
 * \param [in] status           ATT error
 *
 */
void hts_set_meas_interval_cfm(ble_service_t *svc, uint16_t conn_idx, att_error_t status);

#endif /* HTS_H_ */
