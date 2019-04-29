/**
 ****************************************************************************************
 *
 * @file uds.h
 *
 * @brief User Data Service sample implementation API
 *
 * Copyright (C) 2015 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef UDS_H_
#define UDS_H_

#include <stdbool.h>
#include <stdint.h>
#include "ble_service.h"

/**
 * \brief CCC for Database Change Increment written callback
 *
 * \param [in] svc              service instance
 * \param [in] conn_idx         connection index
 * \param [in] value            value to change
 *
 */
typedef void (* uds_ccc_changed_cb_t) (ble_service_t *svc, uint16_t conn_idx, uint16_t value);

/**
 * \brief Database Change Increment written callback
 *
 * \param [in] svc              service instance
 * \param [in] conn_idx         connection index
 * \param [in] increment        increment
 *
 * \return if database was changed
 *
 */
typedef bool (* uds_db_increment_changed_cb_t) (ble_service_t *svc,
                                                        uint16_t conn_idx, uint32_t increment);

/**
 * \brief Register New User control point written callback
 *
 * \param [in] svc              service instance
 * \param [in] conn_idx         connection index
 * \param [in] consent          consent
 *
 */
typedef void (* uds_cp_register_new_user_cb_t) (ble_service_t *svc,
                                                        uint16_t conn_idx, uint16_t consent);

/**
 * \brief Consent control point written callback
 *
 * \param [in] svc              service instance
 * \param [in] conn_idx         connection index
 * \param [in] user_id          user ID
 * \param [in] consent          consent
 *
 */
typedef void (* uds_cp_consent_cb_t) (ble_service_t *svc, uint16_t conn_idx, uint8_t user_id,
                                                                                uint16_t consent);

/**
 * \brief Delete user data control point written callback
 *
 * \param [in] svc              service instance
 * \param [in] conn_idx         connection index
 *
 */
typedef void (* uds_cp_delete_user_data_cb_t) (ble_service_t *svc, uint16_t conn_idx);

/**
 * \brief Client attempt to read database value callback
 *
 * \param [in] svc              service instance
 * \param [in] conn_idx         connection index
 * \param [in] field            field
 *
 */
typedef void (* uds_db_read_cb_t) (ble_service_t *svc, uint16_t conn_idx, uint32_t field);

/**
 * \brief Client attempt to write database value callback
 *
 * \param [in] svc              service instance
 * \param [in] conn_idx         connection index
 * \param [in] field            field
 * \param [in] offset           offset
 * \param [in] length           length of value
 * \param [in] value            written value
 *
 */
typedef void (* uds_db_write_cb_t) (ble_service_t *svc, uint16_t conn_idx, uint32_t field,
                                        uint16_t offset , uint16_t length, const void *value);

/** UDS application callbacks */
typedef struct {
        /** CCC for Database Change Increment written */
        uds_ccc_changed_cb_t            db_increment_ccc_changed;

        /** Database Change Increment written */
        uds_db_increment_changed_cb_t   db_increment_changed;

        /** Register New User control point written */
        uds_cp_register_new_user_cb_t   cp_register_new_user;

        /** Consent control point written */
        uds_cp_consent_cb_t             cp_consent;

        /** Delete User Data control point written */
        uds_cp_delete_user_data_cb_t    cp_delete_user_data;

        /** Client attempt to read database value */
        uds_db_read_cb_t                db_read;

        /** Client attempt to write database value */
        uds_db_write_cb_t               db_write;
} uds_callbacks_t;

/** User ID */
typedef enum {
        /** User id unknown */
        UDS_USER_ID_UNKNOWN = 0xFF,
} uds_user_id_t;

/** UDS specific error */
typedef enum {
        /** Access not permitted */
        UDS_ERROR_ACCESS_NOT_PERMITTED = ATT_ERROR_APPLICATION_ERROR,
} uds_error_t;

/** UDS database fields (as UDS characteristics defined by User Data Service specification */
typedef enum {
        /** First name */
        UDS_DB_FIELD_FIRST_NAME                         = 0x00000001,
        /** Last name */
        UDS_DB_FIELD_LAST_NAME                          = 0x00000002,
        /** E-mail address */
        UDS_DB_FIELD_EMAIL_ADDRESS                      = 0x00000004,
        /** Age */
        UDS_DB_FIELD_AGE                                = 0x00000008,
        /** Date of birth */
        UDS_DB_FIELD_DATE_OF_BIRTH                      = 0x00000010,
        /** Gender */
        UDS_DB_FIELD_GENDER                             = 0x00000020,
        /** Weight */
        UDS_DB_FIELD_WEIGHT                             = 0x00000040,
        /** Height */
        UDS_DB_FIELD_HEIGHT                             = 0x00000080,
        /** V02 max */
        UDS_DB_FIELD_VO2_MAX                            = 0x00000100,
        /** Heart rate max */
        UDS_DB_FIELD_HEART_RATE_MAX                     = 0x00000200,
        /** Resting heart rate */
        UDS_DB_FIELD_RESTING_HEART_RATE                 = 0x00000400,
        /** Max recommended heart rate */
        UDS_DB_FIELD_MAX_RECOMMENDED_HEART_RATE         = 0x00000800,
        /** Aerobic threshold */
        UDS_DB_FIELD_AEROBIC_THRESHOLD                  = 0x00001000,
        /** Anaerobic threshold */
        UDS_DB_FIELD_ANAEROBIC_THRESHOLD                = 0x00002000,
        /** Sport type */
        UDS_DB_FIELD_SPORT_TYPE                         = 0x00004000,
        /** Date of threshold assessment */
        UDS_DB_FIELD_DATE_OF_THRESHOLD_ASSESSMENT       = 0x00008000,
        /** Waist circumference */
        UDS_DB_FIELD_WAIST_CIRCUMFERENCE                = 0x00010000,
        /** Hip circumference */
        UDS_DB_FIELD_HIP_CIRCUMFERENCE                  = 0x00020000,
        /** Fat burn heart rate low limit */
        UDS_DB_FIELD_FAT_BURN_HEART_RATE_LOW_LIMIT      = 0x00040000,
        /** Fat burn heart rate up limit */
        UDS_DB_FIELD_FAT_BURN_HEART_RATE_UP_LIMIT       = 0x00080000,
        /** Aerobic heart rate low limit */
        UDS_DB_FIELD_AEROBIC_HEART_RATE_LOW_LIMIT       = 0x00100000,
        /** Aerobic heart rate up limit */
        UDS_DB_FIELD_AEROBIC_HEART_RATE_UP_LIMIT        = 0x00200000,
        /** Anaerobic heart rate low limit */
        UDS_DB_FIELD_ANEROBIC_HEART_RATE_LOW_LIMIT      = 0x00400000,
        /** Anaerobic heart rate up limit */
        UDS_DB_FIELD_ANEROBIC_HEART_RATE_UP_LIMIT       = 0x00800000,
        /** Five zone heart rate limits */
        UDS_DB_FIELD_FIVE_ZONE_HEART_RATE_LIMITS        = 0x01000000,
        /** Tree zone heart rate limits */
        UDS_DB_FIELD_THREE_ZONE_HEART_RATE_LIMITS       = 0x02000000,
        /** Two zone heart rate limits */
        UDS_DB_FIELD_TWO_ZONE_HEART_RATE_LIMIT          = 0x04000000,
        /** Language */
        UDS_DB_FIELD_LANGUAGE                           = 0x08000000,
} uds_db_field_t;

/** Response status for UDS Control Point operation */
typedef enum {
        /** Response success */
        UDS_CP_RESPONSE_SUCCESS                 = 0x01,
        /** Opcode not supported */
        UDS_CP_RESPONSE_OPCODE_NOT_SUPPORTED    = 0x02,
        /** Invalid parameter */
        UDS_CP_RESPONSE_INVALID_PARAM           = 0x03,
        /** Response failed */
        UDS_CP_RESPONSE_FAILED                  = 0x04,
        /** Response not authorized */
        UDS_CP_RESPONSE_NOT_AUTHORIZED          = 0x05,
} uds_cp_response_t;

/**
 * \brief Register User Data Service instance
 *
 * Function registers User Data Service with given set of database fields.
 *
 * \param [in] config           UDS specific configuration
 * \param [in] db_fields        UDS database fields
 * \param [in] cb               application callbacks
 *
 * \return service instance
 *
 */
ble_service_t *uds_init(const ble_service_config_t *config, uds_db_field_t db_fields,
                                                                        const uds_callbacks_t *cb);

/**
 * \brief Control Point register new user confirm
 *
 * Confirmation of registering the new user. This confirmation contains:
 *  - constant response code
 *  - op code
 *  - status
 *
 * \param [in] svc              service instance
 * \param [in] conn_idx         connection index
 * \param [in] status           status of response
 * \param [in] user_id          user ID
 *
 */
void uds_cp_register_new_user_cfm(ble_service_t *svc, uint16_t conn_idx,
                                                        uds_cp_response_t status, uint8_t user_id);

/**
 * \brief Set database increment
 *
 * Function sets value of database increment determined by the client to synchronize with
 * the server.
 *
 * \param [in] svc              service instance
 * \param [in] conn_idx         connection index
 * \param [in] increment        increment value
 * \param [in] notify           notify database increment
 *
 */
void uds_set_db_increment(ble_service_t *svc, uint16_t conn_idx, uint32_t increment, bool notify);

/**
 * \brief Control Point consent confirmation
 *
 * Confirmation of connecting the client to registered user.
 *
 * \param [in] svc              service instance
 * \param [in] conn_idx         connection index
 * \param [in] status           status of response
 *
 */
void uds_cp_consent_cfm(ble_service_t *svc, uint16_t conn_idx, uds_cp_response_t status);

/**
 * \brief Control Point delete user confirmation
 *
 * Send confirmation that the current user was deleted or not.
 *
 * \param [in] svc              service instance
 * \param [in] conn_idx         connection index
 * \param [in] status           status of response
 *
 */
void uds_cp_delete_user_cfm(ble_service_t *svc, uint16_t conn_idx, uds_cp_response_t status);

/**
 * \brief Database read confirmation
 *
 * This function is responds to the callback uds_db_read_cb_t
 *
 * \param [in] svc              service instance
 * \param [in] conn_idx         connection index
 * \param [in] field            field from which read the value
 * \param [in] status           error status
 * \param [in] length           value length
 * \param [out] value           read value
 *
 */
void uds_db_read_cfm(ble_service_t *svc, uint16_t conn_idx, uint32_t field, att_error_t status,
                                                                uint16_t length, const void *value);

/**
 * \brief Database write confirmation
 *
 * This function responds to the callback uds_db_write_cb_t
 *
 * \param [in] svc              service instance
 * \param [in] conn_idx         connection index
 * \param [in] field            field to which write the value
 * \param [in] status           error status
 *
 */
void uds_db_write_cfm(ble_service_t *svc, uint16_t conn_idx, uint32_t field, att_error_t status);

/**
 * \brief Set user ID
 *
 * Set user ID in User Index characteristic.
 *
 * \param [in] svc              service instance
 * \param [in] conn_idx         connection index
 * \param [in] user_id          user ID
 *
 */
void uds_set_user_id(ble_service_t *svc, uint16_t conn_idx, uint8_t user_id);

/**
 * \brief Get user ID
 *
 * Get user ID from User Index characteristic.
 *
 * \param [in] svc              service instance
 * \param [in] conn_idx         connection index
 *
 * \return user ID
 *
 */
uint8_t uds_get_user_id(ble_service_t *svc, uint16_t conn_idx);

#endif /* UDS_H_ */
