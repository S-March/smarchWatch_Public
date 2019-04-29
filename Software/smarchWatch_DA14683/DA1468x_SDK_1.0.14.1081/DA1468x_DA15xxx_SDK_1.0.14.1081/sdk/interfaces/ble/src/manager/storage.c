/**
 ****************************************************************************************
 *
 * @file storage.c
 *
 * @brief BLE Manager storage
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <string.h>
#include "FreeRTOS.h"
#include "osal.h"
#include "ble_mgr.h"
#include "storage.h"
#include "storage_flash.h"

enum {
        STATE_CLEAN       = 0x00,
        STATE_DIRTY       = 0x01,
        STATE_NEEDS_FLUSH = 0x02,
};

PRIVILEGED_DATA static OS_MUTEX lock;

PRIVILEGED_DATA static uint8_t state;

PRIVILEGED_DATA static queue_t device_list;

static void app_value_destroy(void *elem)
{
        app_value_t *appval = elem;

        /*
         * length is non-zero in case there's actual pointer stored in ptr which should be
         * freed when removing appval. otherwise ptr keeps scalar value and should not be
         * freed.
         */
        if (appval->length && appval->ptr) {
                if (appval->free_cb) {
                        appval->free_cb(appval->ptr);
                } else {
                        OS_FREE(appval->ptr);
                }
        }

        OS_FREE(appval);
}

static bool device_match(const void *elem, const void *ud)
{
        return elem == ud;
}

static bool device_addr_match(const void *elem, const void *ud)
{
        const device_t *dev = elem;
        const bd_address_t *addr = ud;

        return !memcmp(&dev->addr, addr, sizeof(*addr));
}

static bool device_conn_idx_match(const void *elem, const void *ud)
{
        const device_t *dev = elem;

        // matching by conn_idx makes sense only when device is connected
        return dev->connected && (dev->conn_idx == (uint32_t) ud);
}

struct device_find_data {
        device_match_cb_t cb;
        void *ud;
};

static bool device_custom_match(const void *elem, const void *ud)
{
        struct device_find_data *d = (struct device_find_data *) ud;
        const device_t *dev = elem;

        return d->cb(dev, d->ud);
}

static bool app_value_match(const void *elem, const void *ud)
{
        const app_value_t *appval = elem;
        ble_storage_key_t key = (uint32_t) ud;

        return appval->key == key;
}

static bool app_value_match_free_np(const void *elem, const void *ud)
{
        const app_value_t *appval = elem;

        return !appval->persistent;
}

static void device_free_pairing(device_t *dev)
{
        if (dev->ltk) {
                OS_FREE(dev->ltk);
                dev->ltk = NULL;
        }

        if (dev->remote_ltk) {
                OS_FREE(dev->remote_ltk);
                dev->remote_ltk = NULL;
        }

        if (dev->irk) {
                OS_FREE(dev->irk);
                dev->irk = NULL;
        }

        if (dev->csrk) {
                OS_FREE(dev->csrk);
                dev->csrk = NULL;
        }

        if (dev->remote_csrk) {
                OS_FREE(dev->remote_csrk);
                dev->remote_csrk = NULL;
        }
}

static void device_free(device_t *dev)
{
        device_free_pairing(dev);

        OS_FREE(dev);
}

void storage_init(void)
{
        queue_init(&device_list);

        if (!lock) {
                (void)OS_MUTEX_CREATE(lock);
        }

        OS_ASSERT(lock);

        storage_flash_init();
        storage_flash_load();
}

static void device_cleanup(void *data)
{
        device_t *dev = (device_t *) data;

        queue_remove_all(&dev->app_value, app_value_destroy);

        device_free(dev);
}

void storage_cleanup(void)
{
        storage_flash_save();

        queue_remove_all(&device_list, device_cleanup);
}

void storage_acquire(void)
{
        OS_MUTEX_GET(lock, OS_MUTEX_FOREVER);
}

void storage_release(void)
{
        if (state & STATE_NEEDS_FLUSH) {
                /*
                 * If this is called from BLE Manager context, just write data immediately. If
                 * called from some other task we just notify manager to do the write. This is to
                 * avoid calling VES in application task context which requires larger stack and we
                 * don't want to put another requirement on application to take care of this.
                 */
                if (ble_mgr_is_own_task()) {
                        storage_flash_save();
                        state = STATE_CLEAN;
                } else {
                        ble_mgr_notify_commit_storage();
                }
        }

        OS_MUTEX_PUT(lock);
}

void storage_mark_dirty(bool flush_now)
{
        state |= STATE_DIRTY;

        if (flush_now) {
                state |= STATE_NEEDS_FLUSH;
        }
}

device_t *find_device_by_addr(const bd_address_t *addr, bool create)
{
        device_t *dev;

        dev = queue_find(&device_list, device_addr_match, addr);

        if (!dev && create) {
                dev = OS_MALLOC(sizeof(*dev));
                if (!dev) {
                        return NULL;
                }

                memset(dev, 0, sizeof(*dev));

                queue_init(&dev->app_value);

                memcpy(&dev->addr, addr, sizeof(*addr));
                dev->mtu = 23;

                queue_push_back(&device_list, dev);
        }

        return dev;
}

device_t *find_device_by_conn_idx(uint16_t conn_idx)
{
        return queue_find(&device_list, device_conn_idx_match, (void *) (uint32_t) conn_idx);
}

device_t *find_device(device_match_cb_t cb, void *ud)
{
        struct device_find_data d = {
                .cb = cb,
                .ud = ud,
        };

        return queue_find(&device_list, device_custom_match, &d);
}

static app_value_t *find_app_value(device_t *dev, ble_storage_key_t key, bool create)
{
        app_value_t *appval;

        appval = queue_find(&dev->app_value, app_value_match, (void *) (uint32_t) key);

        if (!appval && create) {
                appval = OS_MALLOC(sizeof(*appval));
                memset(appval, 0, sizeof(*appval));

                appval->key = key;

                queue_push_back(&dev->app_value, appval);
        }

        return appval;
}

void device_move_front(device_t *dev)
{
        dev = queue_remove(&device_list, device_match, dev);

        if (dev) {
                queue_push_front(&device_list, dev);
        }
}

struct device_cb_data {
        device_cb_t cb;
        void *ud;
};

static void device_cb(void *elem, void *ud)
{
        device_t *dev = elem;
        const struct device_cb_data *d = ud;

        d->cb(dev, d->ud);
}

void device_foreach(device_cb_t cb, void *ud)
{
        struct device_cb_data d = {
                .cb = cb,
                .ud = ud,
        };

        queue_foreach(&device_list, device_cb, &d);
}

void device_remove(device_t *dev)
{
        dev = queue_remove(&device_list, device_match, dev);

        if (!dev) {
                return; // should not happen! ;)
        }

        queue_remove_all(&dev->app_value, app_value_destroy);

        device_free(dev);

        storage_mark_dirty(true);
}

void device_remove_pairing(device_t *dev)
{
        dev->bonded = false;
        dev->paired = false;
        dev->mitm = false;

        device_free_pairing(dev);

        storage_mark_dirty(true);
}

void app_value_put(device_t *dev, ble_storage_key_t key, uint16_t length, void *ptr,
                                                ble_storage_free_cb_t free_cb, bool persistent)
{
        app_value_t *appval;

        app_value_remove(dev, key);

        appval = find_app_value(dev, key, true);
        appval->persistent = persistent;
        appval->length = length;
        appval->ptr = ptr;
        appval->free_cb = free_cb;

        storage_mark_dirty(true);
}

bool app_value_get(device_t *dev, ble_storage_key_t key, uint16_t *length, void **ptr)
{
        app_value_t *appval;


        appval = find_app_value(dev, key, false);
        if (!appval) {
                return false;
        }

        *length = appval->length;
        *ptr = appval->ptr;

        return true;
}

void app_value_remove(device_t *dev, ble_storage_key_t key)
{
        app_value_t *appval;

        appval = queue_remove(&dev->app_value, app_value_match, (void *) (uint32_t) key);
        if (appval) {
                app_value_destroy(appval);
        }
}

void app_value_remove_np(device_t *dev)
{
        queue_filter(&dev->app_value, app_value_match_free_np, NULL, app_value_destroy);
}
