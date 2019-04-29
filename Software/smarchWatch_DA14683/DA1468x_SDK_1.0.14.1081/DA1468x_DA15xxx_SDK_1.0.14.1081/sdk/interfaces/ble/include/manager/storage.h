/**
 ****************************************************************************************
 *
 * @file storage.h
 *
 * @brief BLE Manager storage interface
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef STORAGE_H_
#define STORAGE_H_

#include <stdbool.h>
#include "ble_common.h"
#include "ble_gap.h"
#include "ble_storage.h"
#include "util/queue.h"

/*
 * Following are internal storage keys (for app values) to be used internally - the should be
 * outside ranges allowed to be set by application, so start them with 0xF0000000.
 */
enum {
        STORAGE_KEY_SVC_CHANGED_CCC = 0xF0000000,
};

typedef struct {
        void     *next;

        ble_storage_key_t key;

        bool     persistent;

        uint16_t length;
        void     *ptr;

        ble_storage_free_cb_t free_cb;
} app_value_t;

typedef struct {
        uint64_t        rand;
        uint16_t        ediv;
        uint8_t         key[16];
        uint8_t         key_size;
} key_ltk_t;

typedef struct {
        uint8_t         key[16];
} key_irk_t;

typedef struct {
        uint8_t         key[16];
        uint32_t        sign_cnt;
} key_csrk_t;

typedef struct {
        void            *next;

        bd_address_t    addr;
        uint16_t        conn_idx;

        // state flags
        bool            connecting:1;
        bool            connected:1;
        bool            master:1;
        bool            paired:1;
        bool            bonded:1;
        bool            encrypted:1;
        bool            mitm:1;
        bool            resolving:1;
#if (dg_configBLE_SECURE_CONNECTIONS == 1)
        bool            secure:1;
#endif /* (dg_configBLE_SECURE_CONNECTIONS == 1) */

        // parameters
        uint16_t        mtu;
        gap_sec_level_t sec_level;
        uint16_t        ce_len_min;
        uint16_t        ce_len_max;

        // pairing information
        key_ltk_t       *ltk;
        key_ltk_t       *remote_ltk;
        key_irk_t       *irk;
        key_csrk_t      *csrk;
        key_csrk_t      *remote_csrk;

        // custom values set from application
        queue_t         app_value;
} device_t;

typedef void (* device_cb_t) (device_t *dev, void *ud);

typedef bool (* device_match_cb_t) (const device_t *dev, void *ud);

device_t *find_device_by_addr(const bd_address_t *addr, bool create);

device_t *find_device_by_conn_idx(uint16_t conn_idx);

device_t *find_device(device_match_cb_t cb, void *ud);

void storage_init(void);

void storage_cleanup(void);

void storage_acquire(void);

void storage_release(void);

void storage_mark_dirty(bool flush_now);

void device_foreach(device_cb_t cb, void *ud);

void device_move_front(device_t *dev);

void device_remove(device_t *dev);

void device_remove_pairing(device_t *dev);

void app_value_put(device_t *dev, ble_storage_key_t key, uint16_t length, void *ptr,
                                                ble_storage_free_cb_t free_cb, bool persistent);

bool app_value_get(device_t *dev, ble_storage_key_t key, uint16_t *length, void **ptr);

void app_value_remove(device_t *dev, ble_storage_key_t key);

void app_value_remove_np(device_t *dev);

#endif /* STORAGE_H_ */
