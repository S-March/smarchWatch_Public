/**
 \addtogroup INTERFACES
 \{
 \addtogroup BLE
 \{
 \addtogroup API
 \{
 */

/**
 ****************************************************************************************
 *
 * @file ble_gatts.h
 *
 * @brief BLE GATT Server API
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BLE_GATTS_H_
#define BLE_GATTS_H_

#include <stdint.h>
#include "ble_att.h"
#include "ble_common.h"
#include "ble_gatt.h"

/** GATT Server flags */
typedef enum {
        GATTS_FLAG_CHAR_READ_REQ = 0x01,        ///< enable ::BLE_EVT_GATTS_READ_REQ for attribute
} gatts_flag_t;

/** GATT Server events */
enum ble_evt_gatts {
        /** Read request from peer */
        BLE_EVT_GATTS_READ_REQ = BLE_EVT_CAT_FIRST(BLE_EVT_CAT_GATTS),
        /** Write request from peer */
        BLE_EVT_GATTS_WRITE_REQ,
        /** Prepare write request from peer */
        BLE_EVT_GATTS_PREPARE_WRITE_REQ,
        /** Event (notification or indication) sent */
        BLE_EVT_GATTS_EVENT_SENT,
};

/** Structure for ::BLE_EVT_GATTS_READ_REQ */
typedef struct {
        ble_evt_hdr_t   hdr;            ///< event header
        uint16_t        conn_idx;       ///< connection index
        uint16_t        handle;         ///< attribute handle
        uint16_t        offset;         ///< attribute value offset
} ble_evt_gatts_read_req_t;

/** Structure for ::BLE_EVT_GATTS_WRITE_REQ */
typedef struct {
        ble_evt_hdr_t   hdr;            ///< event header
        uint16_t        conn_idx;       ///< connection index
        uint16_t        handle;         ///< attribute handle
        uint16_t        offset;         ///< attribute value offset
        uint16_t        length;         ///< attribute value length
        uint8_t         value[];        ///< attribute value
} ble_evt_gatts_write_req_t;

/** Structure for ::BLE_EVT_GATTS_PREPARE_WRITE_REQ */
typedef struct {
        ble_evt_hdr_t   hdr;            ///< event header
        uint16_t        conn_idx;       ///< connection index
        uint16_t        handle;         ///< attribute handle
} ble_evt_gatts_prepare_write_req_t;

/** Structure for ::BLE_EVT_GATTS_EVENT_SENT */
typedef struct {
        ble_evt_hdr_t   hdr;            ///< event header
        uint16_t        conn_idx;       ///< connection index
        uint16_t        handle;         ///< attribute handle
        gatt_event_t    type;           ///< event type
        bool            status;         ///< event status
} ble_evt_gatts_event_sent_t;

/**
 * \brief Add new GATT service
 *
 * This initiates the addition of a new service to ATT database. Subsequent calls to
 * ble_gatts_add_include(), ble_gatts_add_characteristic() and ble_gatts_add_descriptor() will add
 * attributes to the service added by this call. All of these calls should be in the same order as
 * attributes should be added to the database.
 *
 * The Service is added to the database and enabled once ble_gatts_register_service() is called
 * after all attributes have been added.
 *
 * \param [in]  uuid       service UUID
 * \param [in]  type       service type
 * \param [in]  num_attrs  number of attributes to be added
 *
 * \return result code
 *
 * \sa ble_gatts_add_include()
 * \sa ble_gatts_add_characteristic()
 * \sa ble_gatts_add_descriptor()
 * \sa ble_gatts_register_service()
 * \sa ble_gatts_get_num_attr()
 *
 */
ble_error_t ble_gatts_add_service(const att_uuid_t *uuid, const gatt_service_t type, uint16_t num_attrs);

/**
 * \brief Add included service to GATT service
 *
 * This adds an included service declaration to the service added by ble_gatts_add_service().
 *
 * \param [in]  handle    included service handle
 * \param [out] h_offset  attribute offset (relative to service handle)
 *
 * \return result code
 *
 */
ble_error_t ble_gatts_add_include(uint16_t handle, uint16_t *h_offset);

/**
 * \brief Add characteristic to GATT service
 *
 * This adds a characteristic declaration to the service added by ble_gatts_add_service().
 *
 * The application will receive a ::BLE_EVT_GATTS_WRITE_REQ event every time a value is written by
 * the peer to the attribute. In order for application to receive also a ::BLE_EVT_GATTS_READ_REQ
 * event every time the attribute's value is read by peer, \p flags shall be set to
 * ::GATTS_FLAG_CHAR_READ_REQ.
 *
 * \note If an Extended Characteristic Properties Descriptor is to be added to this characteristic,
 *       extended properties shall be added to \p prop. It will be used later to set the correct
 *       descriptor's value.
 *
 * \param [in]  uuid          characteristic UUID
 * \param [in]  prop          characteristic properties
 * \param [in]  perm          characteristic value attribute permissions
 * \param [in]  max_len       maximum length of characteristic value
 * \param [in]  flags         additional settings for characteristic
 * \param [out] h_offset      attribute offset (relative to service handle)
 * \param [out] h_val_offset  value attribute offset (relative to service handle)
 *
 * \return result code
 *
 */
ble_error_t ble_gatts_add_characteristic(const att_uuid_t *uuid, gatt_prop_t prop, att_perm_t perm,
                                                        uint16_t max_len, gatts_flag_t flags,
                                                        uint16_t *h_offset, uint16_t *h_val_offset);

/**
 * \brief Add descriptor to GATT service
 *
 * This adds a characteristic declaration to the service added by ble_gatts_add_service().
 *
 * The application will receive a ::BLE_EVT_GATTS_WRITE_REQ event every time the attribute is
 * written by the peer. In order for application to receive also a ::BLE_EVT_GATTS_READ_REQ event
 * every time the attribute is read by the peer, \p flags shall be set to ::GATTS_FLAG_CHAR_READ_REQ.
 *
 * \note
 * For some descriptors the ::BLE_EVT_GATTS_READ_REQ event will be sent regardless of the
 * \p flags value. This applies e.g. to the Client Characteristic Configuration descriptor.
 *
 * \note
 * In case of the Extended Characteristic Properties Descriptor, the descriptor's value will be
 * updated based on the properties passed using ble_gatts_add_characteristic().
 *
 * \param [in]  uuid          descriptor UUID
 * \param [in]  perm          descriptor attribute permissions
 * \param [in]  max_len       maximum length of descriptor value
 * \param [in]  flags         additional settings for descriptor
 * \param [out] h_offset      attribute offset (relative to service handle)
 *
 * \return result code
 *
 */
ble_error_t ble_gatts_add_descriptor(const att_uuid_t *uuid, att_perm_t perm, uint16_t max_len,
                                                        gatts_flag_t flags, uint16_t *h_offset);

/**
 * \brief Register service in database
 *
 * This adds all attributes previously added to the service to the attribute database.
 *
 * \p handle can be used to calculate the actual handle values for offset values returned in
 * \p h_offset and \p h_val_offset parameters. A series of pointers to offset values can be
 * specified as arguments in order for this function to update them automatically.
 *
 * \param [out] handle  service handle
 * \param [out] ...     attributes offsets to be translated (shall end with 0)
 *
 * \return result code
 *
 */
ble_error_t ble_gatts_register_service(uint16_t *handle, ...);

/**
 * \brief Enable service in database
 *
 * This makes a service visible to clients. Once registered, services are always enabled by default.
 * Since this function may change the database structure, the application shall ensure that the
 * Service Changed characteristic is enabled in the database (bit 0x20 is set in
 * #defaultBLE_ATT_DB_CONFIGURATION) and indicated if necessary.
 *
 * \param [in] handle   service start handle
 *
 * \return result code
 *
 * \sa ble_gatts_service_changed_ind()
 *
 */
ble_error_t ble_gatts_enable_service(uint16_t handle);

/**
 * \brief Disable service in database
 *
 * This makes a service invisible to clients. Once registered, services are always enabled by
 * default. Since this function may change the database structure, application shall ensure that the
 * Service Changed characteristic is enabled in the database (bit 0x20 is set in
 * #defaultBLE_ATT_DB_CONFIGURATION) and indicated if necessary.
 *
 * \param [in] handle   service start handle
 *
 * \return result code
 *
 * \sa ble_gatts_service_changed_ind()
 *
 */
ble_error_t ble_gatts_disable_service(uint16_t handle);

/**
 * \brief Read current characteristic properties and permissions
 *
 * \param [in]  handle  characteristic handle
 * \param [out] prop    characteristic properties
 * \param [out] perm    characteristic permissions
 *
 * \return result code
 *
 * \sa ble_gatts_set_characteristic_prop()
 *
 */
ble_error_t ble_gatts_get_characteristic_prop(uint16_t handle, gatt_prop_t *prop, att_perm_t *perm);

/**
 * \brief Set characteristic properties and permissions
 *
 * As mandated by Core Specification version 4.1, characteristic declarations shall not change while
 * a bond with any device exists on the server. THe application shall ensure it does not use this
 * API to change the characteristic properties while such bond exists.
 *
 * \param [in] handle   characteristic handle
 * \param [in] prop     characteristic properties
 * \param [in] perm     characteristic permissions
 *
 * \return result code
 *
 * \sa ble_gatts_get_characteristic_prop()
 *
 */
ble_error_t ble_gatts_set_characteristic_prop(uint16_t handle, gatt_prop_t prop, att_perm_t perm);

/**
 * \brief Get attribute value
 *
 * This retrieves an attribute's value from the database. Up to \p size bytes are copied to \p val
 * buffer. The value returned in \p size is the total attribute value length and may be larger than
 * the supplied size. This information can be used by the application to allocate a larger buffer
 * that fits the complete attribute value.
 *
 * \param [in]     handle attribute handle
 * \param [in,out] length input buffer size or attribute value length
 * \param [out]    value  buffer to store attribute value
 *
 * \return result code
 *
 */
ble_error_t ble_gatts_get_value(uint16_t handle, uint16_t *length, void *value);

/**
 * \brief Set attribute value
 *
 * This sets an attribute's value in the internal database. Any read request from any peer will have
 * this value returned. In order for an attribute to have different values for each peer, the
 * application should store them locally and use ::BLE_EVT_GATTS_READ_REQ to handle read requests
 * for a given attribute.
 *
 * \param [in] handle attribute handle
 * \param [in] length attribute value length
 * \param [in] value  attribute value
 *
 * \return result code
 *
 */
ble_error_t ble_gatts_set_value(uint16_t handle, uint16_t length, const void *value);

/**
 * \brief Respond to an attribute read request
 *
 * The application should use this function to respond to a ::BLE_EVT_GATTS_READ_REQ event.
 *
 * \param [in] conn_idx connection index
 * \param [in] handle   attribute handle
 * \param [in] status   operation status
 * \param [in] length   attribute value length
 * \param [in] value    attribute value
 *
 * \return result code
 *
 */
ble_error_t ble_gatts_read_cfm(uint16_t conn_idx, uint16_t handle, att_error_t status, uint16_t length, const void *value);


/**
 * \brief Respond to an attribute write request
 *
 * The application should use this function to respond to a ::BLE_EVT_GATTS_WRITE_REQ event.
 *
 * \note The application shall also use this to confirm write requests for characteristics with
 *       the "Write Without Response" property set.
 *
 * \param [in] conn_idx connection index
 * \param [in] handle   attribute handle
 * \param [in] status   operation status
 *
 * \return result code
 *
 */
ble_error_t ble_gatts_write_cfm(uint16_t conn_idx, uint16_t handle, att_error_t status);

/**
 * \brief Respond to an attribute prepare write request
 *
 * The application should use this function to respond to a ::BLE_EVT_GATTS_PREPARE_WRITE_REQ event.
 *
 * \param [in] conn_idx connection index
 * \param [in] handle   attribute handle
 * \param [in] length   attribute value length
 * \param [in] status   operation status
 *
 * \return result code
 *
 */
ble_error_t ble_gatts_prepare_write_cfm(uint16_t conn_idx, uint16_t handle, uint16_t length, att_error_t status);

/**
 * \brief Send a characteristic value notification or indication
 *
 * Send an indication or a notification of an attribute's value to a connected peer.
 *
 * The application will receive a ::BLE_EVT_GATTS_EVENT_SENT event when the notification is
 * successfully sent over the air.
 *
 * \note The characteristic must have either the GATT_PROP_NOTIFY or the GATT_PROP_INDICATE property.
 *
 * \note If a disconnection happens after calling this function or the notification cannot be
 *       successfully sent over the air due to e.g. a bad connection, the ::BLE_EVT_GATTS_EVENT_SENT
 *       event may not be received.
 *
 * \param [in] conn_idx connection index
 * \param [in] handle   characteristic value handle
 * \param [in] type     indication or notification
 * \param [in] length   characteristic value length
 * \param [in] value    characteristic value
 *
 * \return result code
 *
 */
ble_error_t ble_gatts_send_event(uint16_t conn_idx, uint16_t handle, gatt_event_t type,
                                                                uint16_t length, const void *value);

/**
 * \brief Send indication of the Service Changed Characteristic
 *
 * This should be called only if the Service Changed Characteristic is enabled in the attribute
 * database configuration (bit 0x20 is set in #defaultBLE_ATT_DB_CONFIGURATION).
 *
 * \param [in] conn_idx         connection index
 * \param [in] start_handle     start handle of affected database region
 * \param [in] end_handle       end handle of affected database region
 *
 * \return result code
 *
 */
ble_error_t ble_gatts_service_changed_ind(uint16_t conn_idx, uint16_t start_handle,
                                                                        uint16_t end_handle);

/**
 * \brief Calculate number of attributes required for service
 *
 * \param [in] include         number of included services
 * \param [in] characteristic  number of characteristics
 * \param [in] descriptor      number of descriptors
 *
 * \return number of attributes required
 *
 */
static inline uint16_t ble_gatts_get_num_attr(uint16_t include, uint16_t characteristic,
                                                                                uint16_t descriptor)
{
        return 1 * include + 2 * characteristic + 1 * descriptor;
}

#endif /* BLE_GATTS_H_ */
/**
 \}
 \}
 \}
 */
