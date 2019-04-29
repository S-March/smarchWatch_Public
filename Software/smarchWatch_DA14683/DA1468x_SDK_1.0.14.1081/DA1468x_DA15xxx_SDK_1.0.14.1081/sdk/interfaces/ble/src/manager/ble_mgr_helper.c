/**
 ****************************************************************************************
 *
 * @file ble_mgr_helper.c
 *
 * @brief BLE message creation and handling
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <string.h>
#include "ble_mgr.h"
#include "ble_mgr_config.h"
#include "ble_mgr_cmd.h"
#include "ble_mgr_helper.h"
#include "ble_common.h"

void *alloc_ble_msg(uint16_t op_code, uint16_t size)
{
        ble_mgr_msg_hdr_t *msg;

        /* Allocate at least the size needed for the base message */
        OS_ASSERT(size >= sizeof(ble_mgr_msg_hdr_t));

        msg = OS_MALLOC(size);
        memset(msg, 0, size);
        msg->op_code  = op_code;
        msg->msg_len = size - sizeof(ble_mgr_msg_hdr_t);

        return msg;
}

static void *alloc_evt(uint16_t evt_code, uint16_t size)
{
        ble_evt_hdr_t *evt;

        /* Allocate at least the size needed for the base message */
        OS_ASSERT(size >= sizeof(*evt));

        evt = OS_MALLOC(size);
        memset(evt, 0, size);
        evt->evt_code = evt_code;
        evt->length = size - sizeof(*evt);

        return evt;
}

void *ble_msg_init(uint16_t op_code, uint16_t size)
{
        /* Allocate at least the size needed for the base message */
        OS_ASSERT(size >= sizeof(ble_mgr_msg_hdr_t));

        return alloc_ble_msg(op_code, size);
}

void *ble_evt_init(uint16_t evt_code, uint16_t size)
{
        /* Allocate at least the size needed for the base message */
        OS_ASSERT(size >= sizeof(ble_evt_hdr_t));

        return alloc_evt(evt_code, size);
}

void ble_msg_free(void *msg)
{
        if (msg) {
                OS_FREE(msg);
        }
}


bool ble_cmd_execute(void *cmd, void **rsp, ble_mgr_cmd_handler_t handler)
{
        uint16_t op_code __attribute__((unused));

        /* Save opcode for response check */
        op_code = ((ble_mgr_msg_hdr_t *)cmd)->op_code;

        /* Acquire the BLE manager interface */
        ble_mgr_acquire();

#if (BLE_MGR_DIRECT_ACCESS == 1)
        /* Call BLE manager's handler and wait for response */
        handler(cmd);
#else
        /* Send to BLE manager's command queue and wait for response */
        ble_mgr_command_queue_send(&cmd, OS_QUEUE_FOREVER);
#endif /* (BLE_MGR_DIRECT_ACCESS == 1) */

        /* Block and wait for response on the response queue */
        ble_mgr_response_queue_get(rsp, OS_QUEUE_FOREVER);

        /* Release the BLE manager interface */
        ble_mgr_release();

        /* The response op code must be the same as the original command op code */
        OS_ASSERT(((ble_mgr_msg_hdr_t *) *rsp)->op_code == op_code);

        return true;
}
