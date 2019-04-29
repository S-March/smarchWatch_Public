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
 * @file ble_attribdb.h
 *
 * @brief Helper to manage complex attributes database
 *
 * Copyright (C) 2015 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BLE_ATTRIBDB_H_
#define BLE_ATTRIBDB_H_

#include <stdint.h>

/**
 * NOTE:
 * THIS API IS NO LONGER SUPPORTED AND WILL BE REMOVED - use ble_storage.h instead
 */

typedef struct {
        uint16_t length;
        union {
                int  i32;
                void *ptr;
        };
} ble_attribdb_value_t;

typedef void (* ble_attribdb_foreach_cb_t) (uint16_t conn_idx, const ble_attribdb_value_t *val, void *ud);

void ble_attribdb_put_int(uint16_t conn_idx, uint16_t handle, int value);

void ble_attribdb_put_buffer(uint16_t conn_idx, uint16_t handle, uint16_t length, void *buffer);

int ble_attribdb_get_int(uint16_t conn_idx, uint16_t handle, int def_value);

void *ble_attribdb_get_buffer(uint16_t conn_idx, uint16_t handle, uint16_t *length);

void ble_attribdb_remove(uint16_t conn_idx, uint16_t handle, bool free);

void ble_attribdb_foreach_conn(uint16_t handle, ble_attribdb_foreach_cb_t cb, void *ud);

#endif /* BLE_ATTRIBDB_H_ */
/**
 \}
 \}
 \}
 */
