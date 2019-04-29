/**
 ****************************************************************************************
 *
 * @file ble_ipsp.h
 *
 * @brief Internet Protocol Support Profile API
 *
 * Copyright (C) 2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BLE_IPSP_H_
#define BLE_IPSP_H_

#include <stdint.h>
#include "osal.h"
#include "ble_common.h"

/** Maximum number of opened L2CAP transport channels */
#ifndef BLE_IPSP_MAX_OPENED_CHANNELS
#define BLE_IPSP_MAX_OPENED_CHANNELS            (1)
#endif

/** BLE IPSP event queue length */
#ifndef BLE_IPSP_EVT_QUEUE_LENGTH
#define BLE_IPSP_EVT_QUEUE_LENGTH               (3)
#endif

/** BLE IPSP role enum */
typedef enum {
        /** BLE IPSP node role */
        BLE_IPSP_ROLE_NODE = 0x01,
        /** BLE IPSP router role */
        BLE_IPSP_ROLE_ROUTER = 0x02,
} ble_ipsp_role_t;

/**
 * \brief BLE IPSP connected callback
 *
 * \param [in] conn_idx connection index
 *
 */
typedef void (* ble_ipsp_connected_t) (uint16_t conn_idx);

/**
 * \brief BLE IPSP connection failed callback
 *
 * \param [in] conn_idx connection index
 *
 */
typedef void (* ble_ipsp_connection_failed_t) (uint16_t conn_idx);

/**
 * \brief BLE IPSP disconnected callback
 *
 * \param [in] conn_idx connection index
 * \param [in] reason   disconnect reason
 *
 */
typedef void (* ble_ipsp_disconnected_t) (uint16_t conn_idx, uint16_t reason);

/**
 * \brief BLE IPSP data indication callback
 *
 * \param [in] conn_idx connection index
 * \param [in] length   data length
 * \param [in] data     data pointer
 *
 */
typedef void (* ble_ipsp_data_ind_t) (uint16_t conn_idx, uint16_t length, const uint8_t *data);

/**
 * \brief BLE IPSP sent callback
 *
 * \param [in] conn_idx connection index
 * \param [in] status   send operation status
 *
 */
typedef void (* ble_ipsp_sent_t) (uint16_t conn_idx, ble_error_t status);

/**
 * BLE IPSP callbacks
 */
typedef struct {
        /** Connected callback */
        ble_ipsp_connected_t            connected;
        /** Connection failed callback */
        ble_ipsp_connection_failed_t    connection_failed;
        /** Disconnected callback */
        ble_ipsp_disconnected_t         disconnected;
        /** Data indication callback */
        ble_ipsp_data_ind_t             data_ind;
        /** Sent callback */
        ble_ipsp_sent_t                 sent;
} ble_ipsp_callbacks_t;

/**
 * BLE IPSP config struct
 */
typedef struct {
        /** Bit mask with IPSP role */
        ble_ipsp_role_t role;
        /** BLE task notification mask */
        uint32_t notif_mask;
        /** Security level of L2CAP channel */
        gap_sec_level_t sec_level;
} ble_ipsp_config_t;

/**
 * \brief Register BLE IPSP
 *
 * Function registers BLE IPSP module.
 *
 * \param [in] config   BLE IPSP configuration structure
 *
 */
void ble_ipsp_init(const ble_ipsp_config_t *config);

/**
 * \brief Register BLE IPSP callbacks
 *
 * Function registers BLE IPSP callbacks. Callbacks should be registered right
 * after initialization.
 *
 * \param [in] cb       BLE IPSP callbacks structure
 *
 */
void ble_ipsp_register_callbacks(const ble_ipsp_callbacks_t *cb);

/**
 * \brief Connect IPSP channel
 *
 * Function connects IPSP channel. Valid only if role is ::BLE_IPSP_ROLE_ROUTER
 *
 * \param [in] conn_idx connection index
 *
 * \return True if command has been successfully sent to BLE task, otherwise false
 *
 */
bool ble_ipsp_connect(uint16_t conn_idx);

/**
 * \brief Send data to remote device
 *
 * Function sends data to remote device. If function returns true, sent callback
 * will be called with proper status.
 *
 * \param [in] conn_idx connection index
 * \param [in] length   data length
 * \param [in] data     data pointer
 *
 * \return True if command has been successfully sent to BLE task, otherwise false
 *
 */
bool ble_ipsp_send(uint16_t conn_idx, uint16_t length, const uint8_t *data);

/**
 * \brief Disconnect IPSP channel
 *
 * Function disconnects IPSP channel.
 *
 * \param [in] conn_idx connection index
 *
 * \return True if command has been successfully sent to BLE task, otherwise false
 *
 */
bool ble_ipsp_disconnect(uint16_t conn_idx);

/**
* \brief Handle BLE event
*
* Function handles BLE events and passes them to BLE IPSP.
*
* \param [in] evt       BLE event
*
*/
void ble_ipsp_handle_event(const ble_evt_hdr_t *evt);

/**
 * Handle notification from BLE IPSP
 *
 * Function shall be called when BLE application task is notified.
 *
 */
void ble_ipsp_handle_notified(void);

#endif /* BLE_IPSP_H_ */
