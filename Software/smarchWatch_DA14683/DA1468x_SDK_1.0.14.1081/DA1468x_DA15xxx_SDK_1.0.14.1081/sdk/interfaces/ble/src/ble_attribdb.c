/**
 ****************************************************************************************
 *
 * @file ble_attribdb.c
 *
 * @brief Helper to manage complex attributes database
 *
 * Copyright (C) 2015 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdbool.h>
#include <stddef.h>
#include "osal.h"
#include "ble_attribdb.h"
#include "util/list.h"

struct elem_c;
struct attrib;

struct attrib {
        struct attrib           *next;
        uint16_t                handle;
        ble_attribdb_value_t    val;
};

struct conn {
        struct conn     *next;
        void            *attrib;
        uint16_t        conn_idx;
};

static void *conn_list;

static bool attrib_match(const void *elem, const void *ud)
{
        const struct attrib *attrib = elem;

        return attrib->handle == (uint32_t) ud;
}

static bool conn_match(const void *elem, const void *ud)
{
        const struct conn *conn = elem;

        return conn->conn_idx == (uint32_t) ud;
}

static struct attrib *find_attrib(uint16_t conn_idx, uint16_t handle, bool can_create)
{
        struct conn *conn;
        struct attrib *attrib = NULL;

        conn = list_find(conn_list, conn_match, (void *) (uint32_t) conn_idx);
        if (!conn) {
                if (!can_create) {
                        goto done;
                }

                conn = OS_MALLOC(sizeof(*conn));
                conn->attrib = NULL;
                conn->conn_idx = conn_idx;

                list_add(&conn_list, conn);
        }

        attrib = list_find(conn->attrib, attrib_match, (void *) (uint32_t) handle);
        if (!attrib) {
                if (!can_create) {
                        goto done;
                }

                attrib = OS_MALLOC(sizeof(*attrib));
                attrib->handle = handle;
                attrib->val.length = 0;
                attrib->val.ptr = NULL;

                list_add(&conn->attrib, attrib);
        }

done:
        return attrib;
}

void ble_attribdb_put_int(uint16_t conn_idx, uint16_t handle, int value)
{
        struct attrib *attrib = find_attrib(conn_idx, handle, true);

        /*
         * 'int' can be stored without allocating buffer, just indicate that buffer has no length
         * so it can't be freed when removing element
         */

        attrib->val.length = 0;
        attrib->val.i32 = value;
}

void ble_attribdb_put_buffer(uint16_t conn_idx, uint16_t handle, uint16_t length, void *buffer)
{
        struct attrib *attrib = find_attrib(conn_idx, handle, true);

        attrib->val.length = length;
        /*
         * do not assign pointer if no length given - this is to avoid confusion with non-buffer
         * value stored which also has length=0 assigned
         */
        attrib->val.ptr = length ? buffer : NULL;
}

int ble_attribdb_get_int(uint16_t conn_idx, uint16_t handle, int def_value)
{
        struct attrib *attrib = find_attrib(conn_idx, handle, false);

        if (!attrib) {
                return def_value;
        }

        return attrib->val.i32;
}

void *ble_attribdb_get_buffer(uint16_t conn_idx, uint16_t handle, uint16_t *length)
{
        struct attrib *attrib = find_attrib(conn_idx, handle, false);

        if (!attrib) {
                return NULL;
        }

        if (length) {
                *length = attrib->val.length;
        }

        return attrib->val.ptr;
}

void ble_attribdb_remove(uint16_t conn_idx, uint16_t handle, bool free)
{
        struct conn *conn;

        conn = list_find(conn_list, conn_match, (void *) (uint32_t) conn_idx);
        if (!conn) {
                return;
        }

        list_remove(&conn->attrib, attrib_match, (void *) (uint32_t) handle);

        if (!conn->attrib) {
                list_remove(&conn_list, conn_match, (void *) (uint32_t) conn_idx);
        }
}

void ble_attribdb_foreach_conn(uint16_t handle, ble_attribdb_foreach_cb_t cb, void *ud)
{
        struct conn *conn = conn_list;

        while (conn) {
                struct attrib *attrib = list_find(conn->attrib, attrib_match, (void *) (uint32_t) handle);

                cb(conn->conn_idx, &attrib->val, ud);

                conn = conn->next;
        }
}
