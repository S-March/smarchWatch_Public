/**
 ****************************************************************************************
 *
 * @file wss.h
 *
 * @brief Weight Scale Service sample implementation API
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef WSS_H_
#define WSS_H_

#include <stdint.h>
#include "ble_service.h"
#include "svc_types.h"

typedef void (* wss_indication_changed_cb_t) (uint16_t conn_idx, bool enabled);
typedef void (* wss_indication_sent_cb_t) (uint16_t conn_idx, bool status);

/**
 * \struct wss_callbacks_t
 *
 * \brief WSS application callbacks
 */
typedef struct {
        /** Indication status for Weight Measurement changed by client */
        wss_indication_changed_cb_t    indication_changed;
        /** Response for sent indication */
        wss_indication_sent_cb_t       indication_sent;
} wss_callbacks_t;

/**
 * \enum wss_unit_t
 *
 * \brief WSS unit types
 */
typedef enum {
        WSS_UNIT_SI,                /**< Weight and mass in kilograms and height in meters */
        WSS_UNIT_IMPERIAL,          /**< Weight and mass in pounds and height in inches */
} wss_unit_t;

/**
 * \struct wss_weight_measurement_t
 *
 * \brief WSS weight measurement data
 */
typedef struct {
        wss_unit_t      unit;                  /**< Measurement Unit (mass, weight and height
                                                    values) */
        uint16_t        weight;                /**< fixed point weight value defined in WSS spec */
        bool            time_stamp_present;    /**< flag indicates if time stamp is present */
        svc_date_time_t time_stamp;            /**< time of measurement, NULL if not present */
        uint8_t         user_id;               /**< user id or 0xFF for unknown user */
        uint16_t        bmi;                   /**< fixed point BMI value as defined in WSS spec */
        uint16_t        height;                /**< fixed point height value as defined in WSS
                                                    spec, if 0 bmi and height will not be
                                                    reported */
} wss_weight_measurement_t;

/**
 * \enum wss_feature_t
 *
 * \brief Weight Feature characteristic bit values
 */
typedef enum
{
        WSS_FEAT_TIME_STAMP_SUPPORTED  = 0x0001, /**< Time Stamp */

        WSS_FEAT_MULTI_USER_SUPPORTED  = 0x0002, /**< Multiple Users */

        WSS_FEAT_BMI_SUPPORTED         = 0x0004, /**< BMI supported */

        /* Weight Resolution */
        WSS_FEAT_WT_DISPLAY_500G_ACC   = 0x0008, /**< Resolution of 0.5kg   or 1lb */
        WSS_FEAT_WT_DISPLAY_200G_ACC   = 0x0010, /**< Resolution of 0.2kg   or 0.5lb */
        WSS_FEAT_WT_DISPLAY_100G_ACC   = 0x0018, /**< Resolution of 0.1kg   or 0.2lb */
        WSS_FEAT_WT_DISPLAY_50G_ACC    = 0x0020, /**< Resolution of 0.05kg   or 0.1lb */
        WSS_FEAT_WT_DISPLAY_20G_ACC    = 0x0028, /**< Resolution of 0.02kg   or 0.05lb */
        WSS_FEAT_WT_DISPLAY_10G_ACC    = 0x0030, /**< Resolution of 0.01kg   or 0.02lb */
        WSS_FEAT_WT_DISPLAY_5G_ACC     = 0x0038, /**< Resolution of 0.005kg   or 0.01lb */

        /* Height Resolution */
        WSS_FEAT_HT_DISPLAY_10MM_ACC   = 0x0080, /**< Resolution of 0.01m  or 1in */
        WSS_FEAT_HT_DISPLAY_5MM_ACC    = 0x0100, /**< Resolution of 0.005m  or 0.5in */
        WSS_FEAT_HT_DISPLAY_1MM_ACC    = 0x0180, /**< Resolution of 0.001m  or 0.1in */
} wss_feature_t;

/**
 * \brief Register Weight Scale Service instance
 *
 * Function registers Weight Scale Service with give set of features.
 *
 * \param [in] config   service configuration
 * \param [in] features bit mask of supported features
 * \param [in] cb       application callbacks
 *
 * \return service instance
 *
 */
ble_service_t *wss_init(const ble_service_config_t *config, wss_feature_t features,
                                                                        const wss_callbacks_t *cb);

/**
 * \brief Send weight indication to client
 *
 * Function sends indication to selected client.
 *
 * \param [in] svc service instance
 * \param [in] conn_idx connection index to sent indication to
 * \param [in] measurement characteristic value to send
 *
 * \return 0 (BLE_STATUS_OK) if indication is sent successfully, ble_error_t if error occured
 *
 */
ble_error_t wss_indicate_weight(ble_service_t *svc, uint16_t conn_idx,
                                                const wss_weight_measurement_t *measurement);

/**
 * \brief Send weight indication to all interested clients
 *
 * \param [in] svc service instance
 * \param [in] measurement characteristic value to send
 *
 * \return service instance
 *
 */
void wss_indicate_weight_all(ble_service_t *svc, const wss_weight_measurement_t *measurement);

/**
 * \brief Check if indication is enabled
 *
 * \param [in] svc service instance
 * \param [in] conn_idx connection index to sent indication to
 *
 * \return true if indication is enabled, false otherwise
 *
 */
bool wss_is_indication_enabled(ble_service_t *svc, uint16_t conn_idx);

#endif /* WSS_H_ */
