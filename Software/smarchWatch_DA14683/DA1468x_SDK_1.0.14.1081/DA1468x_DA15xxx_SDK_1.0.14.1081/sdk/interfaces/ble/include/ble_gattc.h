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
 * @file ble_gattc.h
 *
 * @brief BLE GATT Client API
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BLE_GATTC_H_
#define BLE_GATTC_H_

#include <stdint.h>
#include "ble_att.h"
#include "ble_common.h"
#include "ble_gatt.h"

/** GATT Client events */
enum ble_evt_gattc {
        /** Service found during browsing procedure */
        BLE_EVT_GATTC_BROWSE_SVC = BLE_EVT_CAT_FIRST(BLE_EVT_CAT_GATTC),
        /** Browsing procedure completed */
        BLE_EVT_GATTC_BROWSE_COMPLETED,
        /** Service found during discovery */
        BLE_EVT_GATTC_DISCOVER_SVC,
        /** Included service found during discovery */
        BLE_EVT_GATTC_DISCOVER_INCLUDE,
        /** Characteristic found during discovery */
        BLE_EVT_GATTC_DISCOVER_CHAR,
        /** Characteristic descriptor found during discovery */
        BLE_EVT_GATTC_DISCOVER_DESC,
        /** Discovery completed */
        BLE_EVT_GATTC_DISCOVER_COMPLETED,
        /** Read attribute value completed */
        BLE_EVT_GATTC_READ_COMPLETED,
        /** Write attribute value completed */
        BLE_EVT_GATTC_WRITE_COMPLETED,
        /** Value notification received */
        BLE_EVT_GATTC_NOTIFICATION,
        /** value indication received */
        BLE_EVT_GATTC_INDICATION,
        /** MTU changes for peer */
        BLE_EVT_GATTC_MTU_CHANGED,
};

/** Service item type */
typedef enum {
        GATTC_ITEM_TYPE_NONE,           ///< invalid or unknown item
        GATTC_ITEM_TYPE_INCLUDE,        ///< included service
        GATTC_ITEM_TYPE_CHARACTERISTIC, ///< characteristic
        GATTC_ITEM_TYPE_DESCRIPTOR,     ///< characteristic description
} gattc_item_type_t;

/** Discovery type */
typedef enum {
        GATTC_DISCOVERY_TYPE_SVC,               ///< discovery services type
        GATTC_DISCOVERY_TYPE_INCLUDED,          ///< discovery included services type
        GATTC_DISCOVERY_TYPE_CHARACTERISTICS,   ///< discovery characteristics type
        GATTC_DISCOVERY_TYPE_DESCRIPTORS,       ///< discovery descriptors type
} gattc_discovery_type_t;

/** Service item definition */
typedef struct {
        att_uuid_t uuid;                ///< item UUID
        uint16_t handle;                ///< item handle
        gattc_item_type_t type;         ///< item type

        union {
                /** included service data (if type == ::GATTC_ITEM_TYPE_INCLUDE) */
                struct {
                        uint16_t start_h;       ///< included service start handle
                        uint16_t end_h;         ///< included service end handle
                } i;

                /** characteristic data (if type == ::GATTC_ITEM_TYPE_CHARACTERISTIC) */
                struct {
                        uint16_t value_handle;  ///< characteristic value handle
                        uint8_t properties;     ///< characteristic properties
                } c;

                /** characteristic descriptor data (if type == ::GATTC_ITEM_TYPE_DESCRIPTOR) */
                struct {
                        // nothing here
                } d;
        };
} gattc_item_t;

/** Structure for ::BLE_EVT_GATTC_BROWSE_SVC event */
typedef struct {
        ble_evt_hdr_t   hdr;            ///< event header
        uint16_t        conn_idx;       ///< connection index
        att_uuid_t      uuid;           ///< service uuid
        uint16_t        start_h;        ///< service start handle
        uint16_t        end_h;          ///< service end handle
        uint16_t        num_items;      ///< number of items in service
        gattc_item_t    items[];        ///< items found in service
} ble_evt_gattc_browse_svc_t;

/** Structure for ::BLE_EVT_GATTC_BROWSE_COMPLETED event */
typedef struct {
        ble_evt_hdr_t   hdr;            ///< event header
        uint16_t        conn_idx;       ///< connection index
        uint8_t         status;         ///< browsing status
} ble_evt_gattc_browse_completed_t;

/** Structure for ::BLE_EVT_GATTC_DISCOVER_SVC event */
typedef struct {
        ble_evt_hdr_t   hdr;            ///< event header
        uint16_t        conn_idx;       ///< connection index
        att_uuid_t      uuid;           ///< service UUID
        uint16_t        start_h;        ///< service start handle
        uint16_t        end_h;          ///< service end handle
} ble_evt_gattc_discover_svc_t;

/** Structure for ::BLE_EVT_GATTC_DISCOVER_INCLUDE event */
typedef struct {
        ble_evt_hdr_t   hdr;            ///< event header
        uint16_t        conn_idx;       ///< connection index
        uint16_t        handle;         ///< include attribute handle
        att_uuid_t      uuid;           ///< included service UUID
        uint16_t        start_h;        ///< included service start handle
        uint16_t        end_h;          ///< included service end handle
} ble_evt_gattc_discover_include_t;

/** Structure for ::BLE_EVT_GATTC_DISCOVER_CHAR event */
typedef struct {
        ble_evt_hdr_t   hdr;            ///< event header
        uint16_t        conn_idx;       ///< connection index
        att_uuid_t      uuid;           ///< characteristic UUID
        uint16_t        handle;         ///< characteristic handle
        uint16_t        value_handle;   ///< characteristic value handle
        uint8_t         properties;     ///< characteristic properties
} ble_evt_gattc_discover_char_t;

/** Structure for ::BLE_EVT_GATTC_DISCOVER_DESC event */
typedef struct {
        ble_evt_hdr_t   hdr;            ///< event header
        uint16_t        conn_idx;       ///< connection index
        att_uuid_t      uuid;           ///< characteristic descriptor UUID
        uint16_t        handle;         ///< characteristic descriptor handle
} ble_evt_gattc_discover_desc_t;

/** Structure for ::BLE_EVT_GATTC_DISCOVER_COMPLETED event */
typedef struct {
        ble_evt_hdr_t           hdr;            ///< event header
        uint16_t                conn_idx;       ///< connection index
        gattc_discovery_type_t  type;           ///< discovery type
        uint8_t                 status;         ///< discovery status
} ble_evt_gattc_discover_completed_t;

/** Structure for ::BLE_EVT_GATTC_READ_COMPLETED event */
typedef struct {
        ble_evt_hdr_t   hdr;            ///< event header
        uint16_t        conn_idx;       ///< connection index
        uint16_t        handle;         ///< attribute handle
        att_error_t     status;         ///< operation status
        uint16_t        offset;         ///< value offset
        uint16_t        length;         ///< value length
        uint8_t         value[];        ///< value data
} ble_evt_gattc_read_completed_t;

/** Structure for ::BLE_EVT_GATTC_WRITE_COMPLETED event */
typedef struct {
        ble_evt_hdr_t   hdr;            ///< event header
        uint16_t        conn_idx;       ///< connection index
        uint16_t        handle;         ///< attribute handle (will be 0 for ble_gatts_write_execute())
        att_error_t     status;         ///< operation status
} ble_evt_gattc_write_completed_t;

/** Structure for ::BLE_EVT_GATTC_NOTIFICATION event */
typedef struct {
        ble_evt_hdr_t   hdr;            ///< event header
        uint16_t        conn_idx;       ///< connection index
        uint16_t        handle;         ///< attribute handle
        uint16_t        length;         ///< value length
        uint8_t         value[];        ///< value data
} ble_evt_gattc_notification_t;

/** Structure for ::BLE_EVT_GATTC_INDICATION event */
typedef struct {
        ble_evt_hdr_t   hdr;            ///< event header
        uint16_t        conn_idx;       ///< connection index
        uint16_t        handle;         ///< attribute handle
        uint16_t        length;         ///< value length
        uint8_t         value[];        ///< value data
} ble_evt_gattc_indication_t;

/** Structure for ::BLE_EVT_GATTC_MTU_CHANGED event */
typedef struct {
        ble_evt_hdr_t   hdr;            ///< event header
        uint16_t        conn_idx;       ///< connection index
        uint16_t        mtu;            ///< current MTU
} ble_evt_gattc_mtu_changed_t;

/**
 * \brief Browse services on remote GATT server
 *
 * This will automatically discover all characteristics and descriptors of a service. To discover
 * services only, use ble_gattc_discover_svc() instead.
 *
 * ::BLE_EVT_GATTC_BROWSE_SVC will be sent for each service found. Once completed
 * ::BLE_EVT_GATTC_BROWSE_COMPLETED will be sent.
 *
 * If \p uuid is NULL, all services are returned.
 *
 * \param [in] conn_idx  connection index
 * \param [in] uuid      optional service UUID
 *
 * \return result code
 *
 * \sa ble_gattc_discover_svc()
 *
 */
ble_error_t ble_gattc_browse(uint16_t conn_idx, const att_uuid_t *uuid);

/**
 * \brief Discover services on remote GATT server
 *
 * ::BLE_EVT_GATTC_DISCOVER_SVC will be sent for each service found. Once completed
 * ::BLE_EVT_GATTC_DISCOVER_COMPLETED will be sent.

 * If \p uuid is NULL, all services are returned.
 *
 * \param [in] conn_idx  connection index
 * \param [in] uuid      optional service UUID
 *
 * \return result code
 *
 * \sa ble_gattc_browse()
 * \sa ble_gattc_discover_char()
 * \sa ble_gattc_discover_desc()
 *
 */
ble_error_t ble_gattc_discover_svc(uint16_t conn_idx, const att_uuid_t *uuid);

/**
 * \brief Discover included services on remote GATT server
 *
 * ::BLE_EVT_GATTC_DISCOVER_INCLUDE will be sent for each included service found. Once completed
 * ::BLE_EVT_GATTC_DISCOVER_COMPLETED will be sent.
 *
 * \param [in] conn_idx  connection index
 * \param [in] start_h   start handle of service to discover
 * \param [in] end_h     end handle of service to discover
 *
 * \return result code
 *
 */
ble_error_t ble_gattc_discover_include(uint16_t conn_idx, uint16_t start_h, uint16_t end_h);

/**
 * \brief Discover characteristics on remote GATT server
 *
 * ::BLE_EVT_GATTC_DISCOVER_CHAR will be sent for each characteristic found. Once completed
 * ::BLE_EVT_GATTC_DISCOVER_COMPLETED will be sent.
 *
 * If \p uuid is NULL, all characteristics are returned.
 *
 * \param [in] conn_idx  connection index
 * \param [in] start_h   start handle of service to discover
 * \param [in] end_h     end handle of service to discover
 * \param [in] uuid      optional characteristic UUID
 *
 * \return result code
 *
 * \sa ble_gattc_discover_desc()
 *
 */
ble_error_t ble_gattc_discover_char(uint16_t conn_idx, uint16_t start_h, uint16_t end_h,
                                                                        const att_uuid_t *uuid);

/**
 * \brief Discover descriptors on remote GATT server
 *
 * ::BLE_EVT_GATTC_DISCOVER_DESC will be sent for each descriptor found. Once completed
 * ::BLE_EVT_GATTC_DISCOVER_COMPLETED will be sent.
 *
 * \param [in] conn_idx  connection index
 * \param [in] start_h   start handle of characteristic to discover
 * \param [in] end_h     end handle of characteristic to discover
 *
 * \return result code
 *
 */
ble_error_t ble_gattc_discover_desc(uint16_t conn_idx, uint16_t start_h, uint16_t end_h);

/**
 * \brief Read attribute from remote GATT server
 *
 * This uses either the "Read Characteristic Value" procedure or the "Read Characteristic Descriptor"
 * procedure, depending on the attribute pointed by \p handle. If \p offset is non-zero or the
 * attribute length is larger than the MTU, the "Read Long Characteristic Value" procedure or the
 * "Read Long Characteristic Descriptor" procedure will be used respectively. The complete attribute
 * value will be returned in the ::BLE_EVT_GATTC_READ_COMPLETED event.
 *
 * \param [in]     conn_idx  connection index
 * \param [in]     handle    attribute handle
 * \param [in]     offset    value offset to start with
 *
 * \return result code
 *
 */
ble_error_t ble_gattc_read(uint16_t conn_idx, uint16_t handle, uint16_t offset);

/**
 * \brief Write attribute to remote GATT server
 *
 * This uses either the "Write Characteristic Value" procedure or the "Write Characteristic
 * Descriptor" procedure, depending on the attribute pointed by \p handle. If \p offset is non-zero
 * or the attribute length is larger than the MTU, the "Write Long Characteristic Value" procedure
 * or the "Write Long Characteristic Descriptor" procedure will be used respectively.
 *
 * The application will receive a ::BLE_EVT_GATTC_WRITE_COMPLETED event when the write operation is
 * completed.
 *
 * \param [in]     conn_idx  connection index
 * \param [in]     handle    attribute handle
 * \param [in]     offset    value offset to start with
 * \param [in]     length    value length
 * \param [in]     value     value data
 *
 * \return result code
 *
 */
ble_error_t ble_gattc_write(uint16_t conn_idx, uint16_t handle, uint16_t offset, uint16_t length,
                                                                        const uint8_t *value);

/**
 * \brief Write attribute to remote GATT server (without response)
 *
 * If \p signed_write is set to false, the "Write Without Response" procedure will be used.
 * If \p signed_write is set to true, the "Signed Write Without Response" procedure will be used on
 * a link which is not encrypted or will fall back to the "Write Without Response" procedure on a
 * link that is already encrypted.
 *
 * The application will receive a ::BLE_EVT_GATTC_WRITE_COMPLETED event when the write operation is
 * performed.
 *
 * \param [in]     conn_idx      connection index
 * \param [in]     handle        attribute handle
 * \param [in]     signed_write  true if signed write should be used if possible/applicable
 * \param [in]     length        value length
 * \param [in]     value         value data
 *
 * \return result code
 *
 */
ble_error_t ble_gattc_write_no_resp(uint16_t conn_idx, uint16_t handle, bool signed_write,
                                                        uint16_t length, const uint8_t *value);

/**
 * \brief Prepare long/reliable write to remote GATT server
 *
 * The application will receive a ::BLE_EVT_GATTC_WRITE_COMPLETED event when the write operation is
 * queued.
 *
 * \param [in]     conn_idx  connection index
 * \param [in]     handle    attribute handle
 * \param [in]     offset    value offset
 * \param [in]     length    value length
 * \param [in]     value     value data
 *
 * \return result code
 *
 */
ble_error_t ble_gattc_write_prepare(uint16_t conn_idx, uint16_t handle, uint16_t offset,
                                                        uint16_t length, const uint8_t *value);

/**
 * \brief Execute reliable/long write to remote GATT server
 *
 * In order to cancel prepared requests, \p commit shall be set to \p false.
 *
 * The application will receive a ::BLE_EVT_GATTC_WRITE_COMPLETED event when write queue is
 * executed. The \p handle parameter of this event will be set to 0.
 *
 * \param [in]     conn_idx  connection index
 * \param [in]     commit    true if data shall be written, false otherwise
 *
 * \return result code
 *
 */
ble_error_t ble_gattc_write_execute(uint16_t conn_idx, bool commit);

/**
 * \brief Send confirmation for received indication
 *
 * \param [in]     conn_idx  connection index
 * \param [in]     handle    attribute handle
 *
 * \return result code
 *
 * \deprecated
 * This function is deprecated because all indications are confirmed by the BLE framework
 * immediately after reception. To avoid sending wrong confirmations calling this function has no
 * effect.
 *
 */
ble_error_t ble_gattc_indication_cfm(uint16_t conn_idx, uint16_t handle) __attribute__ ((deprecated));

/**
 * \brief Get current TX MTU for peer
 *
 * \param [in]     conn_idx  connection index
 * \param [out]    mtu       current MTU
 *
 * \return result code
 *
 */
ble_error_t ble_gattc_get_mtu(uint16_t conn_idx, uint16_t *mtu);

/**
 * \brief Exchange MTU
 *
 * This call will start an MTU exchange procedure with the MTU previously set using
 * ble_gap_mtu_size_set(). If the MTU has been changed during the negotiation, a
 * ::BLE_EVT_GATTC_MTU_CHANGED event will be sent to the application.
 *
 * \param [in]     conn_idx  connection index
 *
 * \return result code
 *
 */
ble_error_t ble_gattc_exchange_mtu(uint16_t conn_idx);

#endif /* BLE_GATTC_H_ */
/**
 \}
 \}
 \}
 */
