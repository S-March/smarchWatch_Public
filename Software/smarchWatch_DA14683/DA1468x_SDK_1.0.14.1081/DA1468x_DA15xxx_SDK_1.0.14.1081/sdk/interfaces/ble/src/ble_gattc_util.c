/**
 ****************************************************************************************
 *
 * @file ble_gattc_util.c
 *
 * @brief BLE GATT Client utilities API
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdint.h>
#include <string.h>
#include "FreeRTOS.h"
#include "ble_bufops.h"
#include "ble_common.h"
#include "ble_gattc.h"
#include "ble_uuid.h"

struct find_item_state {
        uint16_t index;
        bool has_uuid;
        att_uuid_t uuid;
};

PRIVILEGED_DATA static struct {
        const ble_evt_gattc_browse_svc_t *evt;

        struct find_item_state c_state; /* state for 'find_characteristic' */
        struct find_item_state d_state; /* state for 'find_descriptor' */
} find_state;

static void reset_find_state(void)
{
        find_state.c_state.index = 0;
        find_state.c_state.has_uuid = false;
        find_state.d_state.index = 0;
        find_state.d_state.has_uuid = false;
}

void ble_gattc_util_find_init(const ble_evt_gattc_browse_svc_t *evt)
{
        find_state.evt = evt;

        reset_find_state();
}

const gattc_item_t *ble_gattc_util_find_characteristic(const att_uuid_t *uuid)
{
        if (!find_state.evt) {
                return NULL;
        }

        /* Reset state if parameter has changed */
        if (((uuid != NULL) != find_state.c_state.has_uuid) ||
                                        (uuid && !ble_uuid_equal(uuid, &find_state.c_state.uuid))) {
                reset_find_state();

                find_state.c_state.has_uuid = uuid;
                if (find_state.c_state.has_uuid) {
                        memcpy(&find_state.c_state.uuid, uuid, sizeof(att_uuid_t));
                }
        }

        for (; find_state.c_state.index < find_state.evt->num_items; find_state.c_state.index++) {
                const gattc_item_t *item = &find_state.evt->items[find_state.c_state.index];

                if (item->type != GATTC_ITEM_TYPE_CHARACTERISTIC) {
                        continue;
                }

                if (!uuid || ble_uuid_equal(uuid, &item->uuid)) {
                        /* Move to next item, we'll start next search from there */
                        find_state.c_state.index++;
                        find_state.d_state.index = find_state.c_state.index;

                        return item;
                }
        }

        return NULL;
}

const gattc_item_t *ble_gattc_util_find_descriptor(const att_uuid_t *uuid)
{
        if (!find_state.evt) {
                return NULL;
        }

        /* Reset state if parameter has changed */
        if (((uuid != NULL) != find_state.d_state.has_uuid) ||
                                        (uuid && !ble_uuid_equal(uuid, &find_state.d_state.uuid))) {
                /* we start for last found characteristic item index */
                find_state.d_state.index = find_state.c_state.index;

                find_state.d_state.has_uuid = uuid;
                if (find_state.d_state.has_uuid) {
                        memcpy(&find_state.d_state.uuid, uuid, sizeof(att_uuid_t));
                }
        }

        for (; find_state.d_state.index < find_state.evt->num_items; find_state.d_state.index++) {
                const gattc_item_t *item = &find_state.evt->items[find_state.d_state.index];

                if (item->type == GATTC_ITEM_TYPE_CHARACTERISTIC) {
                        /* characteristic found - no more descriptors! */
                        return NULL;
                }

                if (!uuid || ble_uuid_equal(uuid, &item->uuid)) {
                        /* Move to next item, we'll start next search from there */
                        find_state.d_state.index++;

                        return item;
                }
        }

        return NULL;
}
