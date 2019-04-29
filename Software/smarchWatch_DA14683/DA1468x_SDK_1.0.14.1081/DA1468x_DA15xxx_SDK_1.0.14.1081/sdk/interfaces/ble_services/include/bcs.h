/**
 ****************************************************************************************
 *
 * @file bcs.h
 *
 * @brief Body Composition Service sample implementation API
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BCS_H_
#define BCS_H_

#include <stdbool.h>
#include <stdint.h>
#include "ble_service.h"
#include "svc_types.h"

typedef void (* bcs_indication_changed_cb_t) (uint16_t conn_idx, bool enabled);
typedef void (* bcs_indication_sent_cb_t) (uint16_t conn_idx, bool status);

/**
 * \struct bcs_callbacks_t
 *
 * \brief BCS application callbacks
 */
typedef struct {
        /** Indication status for Body Composition Measurement changed by client */
        bcs_indication_changed_cb_t    indication_changed;
        /** Response for sent indication */
        bcs_indication_sent_cb_t       indication_sent;
} bcs_callbacks_t;

/**
 * \enum bcs_unit_t
 *
 * \brief BCS unit types
 */
typedef enum {
        BCS_UNIT_SI,            /**< Weight and mass in kilograms and height in meters */
        BCS_UNIT_IMPERIAL,      /**< Weight and mass in pounds and height in inches */
} bcs_unit_t;

/**
 * \enum bcs_feat_t
 *
 * \brief BCS feature types
 */
typedef enum {
        BCS_FEAT_TIME_STAMP             = 0x000001,       /**< Time stamp supported */
        BCS_FEAT_MULTIPLE_USERS         = 0x000002,       /**< Multiple users supported */
        BCS_FEAT_BASAL_METABOLISM       = 0x000004,       /**< Basal metabolism supported */
        BCS_FEAT_MUSCLE_PERCENTAGE      = 0x000008,       /**< Muscle percentage supported */
        BCS_FEAT_MUSCLE_MASS            = 0x000010,       /**< Muscle mass supported */
        BCS_FEAT_FAT_FREE_MASS          = 0x000020,       /**< Fat free mass supported */
        BCS_FEAT_SOFT_LEAN_MASS         = 0x000040,       /**< Soft lean mass supported */
        BCS_FEAT_BODY_WATER_MASS        = 0x000080,       /**< Body water mass supported */
        BCS_FEAT_IMPEDANCE              = 0x000100,       /**< Impedance supported */
        BCS_FEAT_WEIGHT                 = 0x000200,       /**< Weight supported */
        BCS_FEAT_HEIGHT                 = 0x000400,       /**< Height supported */

        BCS_FEAT_MASS_RES_05_KG         = 0x000800,       /**< Resolution of 0.5 kg */
        BCS_FEAT_MASS_RES_02_KG         = 0x001000,       /**< Resolution of 0.2 kg */
        BCS_FEAT_MASS_RES_01_KG         = 0x001800,       /**< Resolution of 0.1 kg */
        BCS_FEAT_MASS_RES_005_KG        = 0x002000,       /**< Resolution of 0.05 kg */
        BCS_FEAT_MASS_RES_002_KG        = 0x002800,       /**< Resolution of 0.02 kg */
        BCS_FEAT_MASS_RES_001_KG        = 0x003000,       /**< Resolution of 0.01 kg */
        BCS_FEAT_MASS_RES_0005_KG       = 0x003800,       /**< Resolution of 0.005 kg */

        BCS_FEAT_MASS_RES_05_LB         = 0x000800,       /**< Resolution of 1 lb */
        BCS_FEAT_MASS_RES_02_LB         = 0x001000,       /**< Resolution of 0.5 lb */
        BCS_FEAT_MASS_RES_01_LB         = 0x001800,       /**< Resolution of 0.2 lb */
        BCS_FEAT_MASS_RES_005_LB        = 0x002000,       /**< Resolution of 0.1 lb */
        BCS_FEAT_MASS_RES_002_LB        = 0x002800,       /**< Resolution of 0.05 lb */
        BCS_FEAT_MASS_RES_001_LB        = 0x003000,       /**< Resolution of 0.02 lb */
        BCS_FEAT_MASS_RES_0005_LB       = 0x003800,       /**< Resolution of 0.01 lb */

        BCS_FEAT_HEIGHT_RES_001_M       = 0x008000,       /**< Resolution of 0.01 meter */
        BCS_FEAT_HEIGHT_RES_005_M       = 0x010000,       /**< Resolution of 0.005 meter */
        BCS_FEAT_HEIGHT_RES_0001_M      = 0x018000,       /**< Resolution of 0.001 meter */

        BCS_FEAT_HEIGHT_RES_1_INCH      = 0x008000,       /**< Resolution of 1 inch */
        BCS_FEAT_HEIGHT_RES_05_INCH     = 0x010000,       /**< Resolution of 0.5 inch */
        BCS_FEAT_HEIGHT_RES_01_INCH     = 0x018000,       /**< Resolution of 0.1 inch */
} bcs_feat_t;

/**
 * Body composition measurement information
 *
 * This corresponds to contents of Body Composition Measurement characteristic value.
 * All fixed point values have precision as defined by BCS specification.
 */
typedef struct {
        bcs_unit_t              measurement_unit;       /**< Measurement Unit (applies to all mass, weight and height values) */
        bool                    time_stamp_present;     /**< Flag indicates if time stamp is present */
        svc_date_time_t         time_stamp;             /**< Time Stamp */
        uint8_t                 user_id;                /**< User ID from 0 to 255 (special value - unknown user) */
        uint16_t                body_fat_percentage;    /**< Body Fat Percentage (fixed point value) */
        uint16_t                basal_metabolism;       /**< Basal Metabolism in kilo Joules (fixed point value) */
        uint16_t                muscle_percentage;      /**< Muscle Percentage (fixed point value) */
        uint16_t                muscle_mass;            /**< Muscle Mass (fixed point value) */
        uint16_t                fat_free_mass;          /**< Fat Free Mass (fixed point value) */
        uint16_t                soft_lean_mass;         /**< Soft Lean Mass (fixed point value) */
        uint16_t                body_water_mass;        /**< Body Water Mass (fixed point value) */
        uint16_t                impedance;              /**< Impedance in Ohms (fixed point value) */
        uint16_t                weight;                 /**< Weight (fixed point value) */
        uint16_t                height;                 /**< Height (fixed point value) */
} bcs_body_measurement_t;

/**
 * Register Body Composition Service instance
 *
 * \param [in] config   service configuration
 * \param [in] cb       application callbacks
 * \param [in] feat     body composition features
 *
 * \return BCS instance
 *
 */
ble_service_t *bcs_init(const ble_service_config_t *config, bcs_feat_t feat,
                                                                        const bcs_callbacks_t *cb);

/**
 * Indicate body composition measurement to client
 *
 * Indicate will only be sent if given client enabled indications before.
 *
 * \param [in] svc       BCS instance
 * \param [in] conn_idx  connection index
 * \param [in] meas      body composition measurement
 *
 * \return 0 (BLE_STATUS_OK) if indication is sent successfully, ble_error_t if error occured
 *
 */
ble_error_t bcs_indicate(ble_service_t *svc, uint16_t conn_idx, bcs_body_measurement_t *meas);

/**
 * Indicate body composition measurement to all connected clients
 *
 * This is roughly equivalent for calling bcs_indicate() for each connected client.
 *
 * \param [in] svc   BCS instance
 * \param [in] meas  body composition measurement
 *
 */
void bcs_indicate_all(ble_service_t *svc, bcs_body_measurement_t *meas);

/**
 * \brief Check if indication is enabled
 *
 * \param [in] svc service instance
 * \param [in] conn_idx connection index to sent indication to
 *
 * \return true if indication is enabled, false otherwise
 *
 */
bool bcs_is_indication_enabled(ble_service_t *svc, uint16_t conn_idx);

#endif /* BCS_H_ */
