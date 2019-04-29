/**
 ****************************************************************************************
 *
 * @file ble_storage.c
 *
 * @brief BLE persistent storage API
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "osal.h"
#include "ble_storage.h"
#include "storage.h"

static ble_error_t generic_put_cmd(uint16_t conn_idx, ble_storage_key_t key, uint16_t length, void *ptr,
                                                ble_storage_free_cb_t free_cb, bool persistent)

{
        device_t *dev;
        ble_error_t ret = BLE_ERROR_FAILED;

        storage_acquire();

        dev = find_device_by_conn_idx(conn_idx);
        if (!dev) {
                ret = BLE_ERROR_NOT_CONNECTED;
                goto done;
        }

        app_value_put(dev, key, length, ptr, free_cb, persistent);

        ret = BLE_STATUS_OK;

done:
        storage_release();

        return ret;
}

static ble_error_t generic_get_cmd(uint16_t conn_idx, ble_storage_key_t key, uint16_t *length, void **ptr)
{
        device_t *dev;
        ble_error_t ret = BLE_ERROR_FAILED;

        storage_acquire();

        dev = find_device_by_conn_idx(conn_idx);
        if (!dev) {
                ret = BLE_ERROR_NOT_CONNECTED;
                goto done;
        }

        if (!app_value_get(dev, key, length, ptr)) {
                ret = BLE_ERROR_NOT_FOUND;
                goto done;
        }

        ret = BLE_STATUS_OK;

done:
        storage_release();

        return ret;
}

ble_error_t ble_storage_put_i32(uint16_t conn_idx, ble_storage_key_t key, int32_t value, bool persistent)
{
        return generic_put_cmd(conn_idx, key, 0, (void *) value, NULL, persistent);
}

ble_error_t ble_storage_put_u32(uint16_t conn_idx, ble_storage_key_t key, uint32_t value, bool persistent)
{
        return generic_put_cmd(conn_idx, key, 0, (void *) value, NULL, persistent);
}

ble_error_t ble_storage_put_buffer(uint16_t conn_idx, ble_storage_key_t key, uint16_t length, void *ptr,
                                                ble_storage_free_cb_t free_cb, bool persistent)
{
        if (!length) {
                return BLE_ERROR_FAILED;
        }

        return generic_put_cmd(conn_idx, key, length, ptr, free_cb, persistent);
}

uint8_t ble_storage_put_buffer_copy(uint16_t conn_idx, ble_storage_key_t key, uint16_t length, void *ptr,
                                                ble_storage_free_cb_t free_cb, bool persistent)
{
        void *new_ptr;

        if (!length) {
                return BLE_ERROR_FAILED;
        }

        new_ptr = OS_MALLOC(length);
        memcpy(new_ptr, ptr, length);

        return generic_put_cmd(conn_idx, key, length, new_ptr, free_cb, persistent);
}

ble_error_t ble_storage_get_i8(uint16_t conn_idx, ble_storage_key_t key, int8_t *value)
{
        ble_error_t ret;
        uint32_t val;

        ret = ble_storage_get_u32(conn_idx, key, &val);

        if (ret == BLE_STATUS_OK) {
                *value = (int8_t) val;
        }

        return ret;
}

ble_error_t ble_storage_get_u8(uint16_t conn_idx, ble_storage_key_t key, uint8_t *value)
{
        ble_error_t ret;
        uint32_t val;

        ret = ble_storage_get_u32(conn_idx, key, &val);

        if (ret == BLE_STATUS_OK) {
                *value = (uint8_t) val;
        }

        return ret;
}

ble_error_t ble_storage_get_i16(uint16_t conn_idx, ble_storage_key_t key, int16_t *value)
{
        ble_error_t ret;
        uint32_t val;

        ret = ble_storage_get_u32(conn_idx, key, &val);

        if (ret == BLE_STATUS_OK) {
                *value = (int16_t) val;
        }

        return ret;
}

ble_error_t ble_storage_get_u16(uint16_t conn_idx, ble_storage_key_t key, uint16_t *value)
{
        ble_error_t ret;
        uint32_t val;

        ret = ble_storage_get_u32(conn_idx, key, &val);

        if (ret == BLE_STATUS_OK) {
                *value = (uint16_t) val;
        }

        return ret;
}

ble_error_t ble_storage_get_i32(uint16_t conn_idx, ble_storage_key_t key, int32_t *value)
{
        ble_error_t ret;
        uint32_t val;

        ret = ble_storage_get_u32(conn_idx, key, &val);

        if (ret == BLE_STATUS_OK) {
                *value = (int32_t) val;
        }

        return ret;
}

ble_error_t ble_storage_get_u32(uint16_t conn_idx, ble_storage_key_t key, uint32_t *value)
{
        ble_error_t ret;
        uint16_t length;
        void *ptr;

        ret = generic_get_cmd(conn_idx, key, &length, &ptr);

        if (length) {
                // not an uint value (pointer is stored there)
                return BLE_ERROR_FAILED;
        }

        if (ret == BLE_STATUS_OK) {
                *value = (uint32_t) ptr;
        }

        return ret;
}

ble_error_t ble_storage_get_buffer(uint16_t conn_idx, ble_storage_key_t key, uint16_t *length, void **ptr)
{
        ble_error_t ret;
        uint16_t tmp_length;
        void *tmp_ptr;

        ret = generic_get_cmd(conn_idx, key, &tmp_length, &tmp_ptr);

        if (!tmp_length) {
                // not a buffer value (int/uint is stored there)
                return BLE_ERROR_FAILED;
        }

        if (ret == BLE_STATUS_OK) {
                *length = tmp_length;
                *ptr = tmp_ptr;
        }

        return ret;
}

ble_error_t ble_storage_remove(uint16_t conn_idx, ble_storage_key_t key)
{
        device_t *dev;
        ble_error_t ret = BLE_ERROR_FAILED;

        dev = find_device_by_conn_idx(conn_idx);
        if (!dev) {
                ret = BLE_ERROR_NOT_CONNECTED;
                goto done;
        }

        app_value_remove(dev, key);

        ret = BLE_STATUS_OK;
        ret = BLE_STATUS_OK;

done:
        storage_release();

        return ret;
}

static void remove_all(device_t *dev, void *ud)
{
        ble_storage_key_t key = (ble_storage_key_t) ud;

        app_value_remove(dev, key);
}

ble_error_t ble_storage_remove_all(ble_storage_key_t key)
{
        device_foreach(remove_all, (void *) key);

        return BLE_STATUS_OK;
}
