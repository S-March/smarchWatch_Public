/**
 ****************************************************************************************
 *
 * @file ble_service.h
 *
 * @brief Services handling routines API
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BLE_SERVICE_H_
#define BLE_SERVICE_H_

#include <stdbool.h>
#include "ble_gap.h"
#include "ble_gatts.h"

typedef struct ble_service ble_service_t;

/**
* \brief Connected event callback
*
* Function to be called called when a new connection is established.
*
* \param [in] svc       service instance
* \param [in] evt       connected event
*
*/
typedef void (* connected_evt_t) (ble_service_t *svc, const ble_evt_gap_connected_t *evt);

/**
* \brief Dis event callback
*
* Function to be called when disconnected from a remote device.
*
* \param [in] svc       service instance
* \param [in] evt       disconnected event
*
*/
typedef void (* disconnected_evt_t) (ble_service_t *svc, const ble_evt_gap_disconnected_t *evt);

/**
* \brief Read request callback
*
* Function to be called when a read request has been received from a remote device.
*
* \param [in] svc       service instance
* \param [in] evt       read request event
*
*/
typedef void (* read_req_t) (ble_service_t *svc, const ble_evt_gatts_read_req_t *evt);

/**
* \brief Write request callback
*
* Function to be called when a write request has been received from a remote device.
*
* \param [in] svc       service instance
* \param [in] evt       write request event
*
*/
typedef void (* write_req_t) (ble_service_t *svc, const ble_evt_gatts_write_req_t *evt);

/**
* \brief Prepare write request callback
*
* Function to be called when a prepare write request has been received from a remote device.
*
* \param [in] svc       service instance
* \param [in] evt       prepare write request event
*
*/
typedef void (* prepare_write_req_t) (ble_service_t *svc,
                                                const ble_evt_gatts_prepare_write_req_t *evt);

/**
* \brief Event sent callback
*
* Function to be called when a notification has been sent or an indication has been confirmed.
*
* \param [in] svc       service instance
* \param [in] evt       event sent event
*
*/
typedef void (* event_sent_t) (ble_service_t *svc, const ble_evt_gatts_event_sent_t *evt);

/**
 * \brief Cleanup callback
 *
 * Cleanup callback. Called when application invokes ::ble_service_cleanup.
 *
 * \param [in] svc      service instance
 */
typedef void (* cleanup_t) (ble_service_t *svc);

/**
 * BLE Service struct
 *
 */
typedef struct ble_service {
        uint16_t        start_h;                /**< Service start handle */
        uint16_t        end_h;                  /**< Service end handle */

        connected_evt_t     connected_evt;      /**< Connected event callback */
        disconnected_evt_t  disconnected_evt;   /**< Disconnected event callback */
        read_req_t          read_req;           /**< Read request callback */
        write_req_t         write_req;          /**< Write request callback */
        prepare_write_req_t prepare_write_req;  /**< Prepare write request callback */
        event_sent_t        event_sent;         /**< Event sent callback */
        cleanup_t           cleanup;            /**< Cleanup callback */

} ble_service_t;

/**
 * BLE Service Config struct
 *
 */
typedef struct {
        gatt_service_t service_type;            /**< GATT service type */
        gap_sec_level_t sec_level;              /**< Service security requirements */

        uint8_t         num_includes;           /**< Number of included services */
        ble_service_t   **includes;             /**< Included services */
} ble_service_config_t;

/**
* \brief Add service
*
* This function adds a service to the internal database. It is required in order to receive service
* callbacks.
*
* \param [in] svc       service instance
*
*/
void ble_service_add(ble_service_t *svc);

/**
* \brief Remove service
*
* This function removes a service from the internal database.
*
* \param [in] svc       service instance
*
* \note This function does not remove service from attribute database. This should be called
* before ble_reset function which destroys attribute database.
*
*/
void ble_service_remove(ble_service_t *svc);

/**
* \brief Cleanup service
*
* This function frees all resources allocated by service.
*
* \param [in] svc       service instance
*
* \note Service should be removed from internal database first using ble_service_remove function.
*
*/
void ble_service_cleanup(ble_service_t *svc);

/**
* \brief Handle BLE event
*
* This function handles BLE events and passes them to services.
*
* \param [in] evt       BLE event
*
* \return True if event was handled, false otherwise.
*
*/
bool ble_service_handle_event(const ble_evt_hdr_t *evt);

/**
* \brief Elevate permissions
*
* Function elevates attribute permissions.
*
* \param [in] perm      Attribute permissions
* \param [in] config    Service configuration structure
*
* \return Elevated permissions.
*
*/
att_perm_t ble_service_config_elevate_perm(att_perm_t perm, const ble_service_config_t *config);

/**
* \brief Get number of attributes
*
* This function calculates number of attributes needed to register the service.
*
* \param [in] config    Service configuration structure
* \param [in] chars     Number of characters
* \param [in] descs     Number of descriptors
*
* \return Number of attributes
*
*/
static inline uint8_t ble_service_get_num_attr(const ble_service_config_t *config, uint16_t chars,
                                                                                uint16_t descs)
{
        return ble_gatts_get_num_attr(config ? config->num_includes : 0, chars, descs);
}

/**
* \brief Add included services
*
* Helper function to register included services.
*
* \param [in] config    Service configuration structure
*
*/
void ble_service_config_add_includes(const ble_service_config_t *config);

#endif /* BLE_SERVICE_H_ */
