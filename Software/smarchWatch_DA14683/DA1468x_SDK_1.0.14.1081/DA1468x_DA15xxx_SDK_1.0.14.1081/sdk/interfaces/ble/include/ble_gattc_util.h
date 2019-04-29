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
 * @file ble_gattc_util.h
 *
 * @brief BLE GATT Client Utilities API
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BLE_GATTC_UTIL_H_
#define BLE_GATTC_UTIL_H_

#include "ble_gattc.h"
#include "ble_uuid.h"

/**
 * \brief Initialize browse event iterators
 *
 * This call will initialize internal structures to iterate over ::BLE_EVT_GATTC_BROWSE_SVC. After
 * this call the application can use ble_gattc_util_find_characteristic() to get a characteristic
 * from the event. \p evt instance should be valid as long as the iterator is used since only weak
 * reference is stored internally.
 *
 * \param [in] evt  instance of event
 *
 */
void ble_gattc_util_find_init(const ble_evt_gattc_browse_svc_t *evt);

/**
 * \brief Find characteristic in browse event
 *
 * This call will return the first characteristic from the event which matches the given UUID (in
 * case \p uuid is \p NULL, the first characteristic will be returned). Subsequent calls with the
 * same given \p uuid will return subsequent characteristics matching the same criteria or \p NULL
 * if no more matching characteristics are found.
 *
 * ble_gattc_util_find_init() must be called prior to calling this function.
 *
 * Subsequent calls with different \p uuid will restart searching from the first characteristic.
 *
 * The returned item will always have the ::GATTC_ITEM_TYPE_CHARACTERISTIC type.
 *
 * \param [in] uuid  optional UUID of characteristic
 *
 * \return found item
 *
 * \sa ble_gattc_util_find_init()
 *
 */
const gattc_item_t *ble_gattc_util_find_characteristic(const att_uuid_t *uuid);

/**
 * \brief Find descriptor in browse event
 *
 * This call will return the first descriptor from the event which matches the given UUID (if
 * \p uuid is \p NULL, the first descriptor will be returned). Subsequent calls with the same
 * given \p uuid will return subsequent descriptors matching the same criteria or \p NULL if no more
 * matching descriptors are found.
 *
 * ble_gattc_util_find_characteristic() must be called and the specified characteristic must be
 * found prior to calling this function.
 *
 * Subsequent calls with different \p uuid will restart searching from the first descriptor.
 *
 * Returned item will always have the ::GATTC_ITEM_TYPE_DESCRIPTOR type.
 *
 * \param [in] uuid  optional UUID of descriptor
 *
 * \return found item
 *
 * \sa ble_gattc_util_find_characteristic()
 *
 */
const gattc_item_t *ble_gattc_util_find_descriptor(const att_uuid_t *uuid);

/**
 * \brief Write value to CCC descriptor
 *
 * This function writes a Client Characteristic Configuration Descriptor value to a given handle.
 *
 * \param [in]  conn_idx        connection index
 * \param [in]  handle          CCC descriptor handle
 * \param [in]  ccc             value to be written
 *
 * \return status of GATT write operation
 */

static inline ble_error_t ble_gattc_util_write_ccc(uint16_t conn_idx, uint16_t handle,
                                                                                gatt_ccc_t ccc)
{
        uint16_t value = ccc;

        return ble_gattc_write(conn_idx, handle, 0, sizeof(value), (uint8_t *) &value);
}

#endif /* BLE_GATTC_UTIL_H_ */
/**
 \}
 \}
 \}
 */
