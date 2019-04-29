/**
 ****************************************************************************************
 *
 * @file dgtl_msg.c
 *
 * @brief DGTL message
 *
 * Copyright (C) 2016-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stddef.h>
#include <string.h>
#include <osal.h>
#include "dgtl.h"
#include "dgtl_msg.h"
#include "dgtl_pkt.h"
#include "ble_mgr_common.h"

static inline size_t get_ext_len(uint8_t pkt_type)
{
        if ((pkt_type >= DGTL_PKT_TYPE_HCI_CMD) && (pkt_type <= DGTL_PKT_TYPE_GTL)) {
                /*
                 * For HCI and GTL packets we return the offset of packet contents from the start
                 * of stack message structure. We also need to include the fact that packet contents
                 * there *does not* include packet type indicator.
                 */
                return offsetof(ble_mgr_common_stack_msg_t, msg) - 1;

        }

        return 0;
}

static inline void *msg2ptr(dgtl_msg_t *msg)
{
        /* Need to remove const from pkt_type pointer! */
        return (void *) (&msg->pkt_type - get_ext_len(msg->pkt_type));
}

static inline dgtl_msg_t *ptr2msg(void *ptr, uint8_t pkt_type)
{
        dgtl_msg_t *msg;

        msg = ptr + get_ext_len(pkt_type);

        /* If this does not match then cast is invalid */
        if (msg->pkt_type != pkt_type) {
                OS_ASSERT(0);
                return NULL;
        }

        return msg;
}

static inline size_t get_pkt_header_length(const dgtl_msg_t *msg)
{
        return dgtl_pkt_get_header_length((const dgtl_pkt_t *) msg);
}

static inline size_t get_pkt_param_length(const dgtl_msg_t *msg)
{
        return dgtl_pkt_get_param_length((const dgtl_pkt_t *) msg);
}

dgtl_msg_t *dgtl_msg_alloc(uint8_t pkt_type, size_t length)
{
        uint8_t *buf;
        size_t ext_len;

        ext_len = get_ext_len(pkt_type);

        buf = OS_MALLOC(length + ext_len);
        buf[ext_len] = pkt_type;

        return ptr2msg(buf, pkt_type);
}

void dgtl_msg_free(dgtl_msg_t *msg)
{
        void *ptr = msg2ptr(msg);
        OS_FREE(ptr);
}

void *dgtl_msg_get_param_ptr(dgtl_msg_t *msg, size_t *length)
{
        size_t header_len;
        uint8_t *data;

        header_len = get_pkt_header_length(msg);

        /* Unknown packet, we don't know where parameters start */
        if (header_len == 0) {
                return NULL;
        }

        if (length) {
                *length = get_pkt_param_length(msg);
        }

        return &msg->data[header_len];
}

void *dgtl_msg_get_ext_ptr(dgtl_msg_t *msg, size_t *length)
{
        if (length) {
                *length = get_ext_len(msg->pkt_type);
        }

        return msg2ptr(msg);
}

dgtl_msg_t *dgtl_msg_prepare_hci_cmd(dgtl_msg_t *msg, uint16_t opcode, uint8_t param_len, void *param)
{
        dgtl_pkt_hci_cmd_t *pkt;

        if (!msg) {
                msg = dgtl_msg_alloc(DGTL_PKT_TYPE_HCI_CMD, sizeof(dgtl_pkt_hci_cmd_t) + param_len);
                if (!msg) {
                        return NULL;
                }
        }

        OS_ASSERT(msg->pkt_type == DGTL_PKT_TYPE_HCI_CMD);

        pkt = (dgtl_pkt_hci_cmd_t *) msg;
        pkt->opcode = opcode;
        pkt->length = param_len;

        if (param) {
                memcpy(pkt->parameters, param, param_len);
        }

        return msg;
}

dgtl_msg_t *dgtl_msg_prepare_hci_evt(dgtl_msg_t *msg, uint8_t code, uint8_t param_len, void *param)
{
        dgtl_pkt_hci_evt_t *pkt;

        if (!msg) {
                msg = dgtl_msg_alloc(DGTL_PKT_TYPE_HCI_EVT, sizeof(dgtl_pkt_hci_evt_t) + param_len);
                if (!msg) {
                        return NULL;
                }
        }

        OS_ASSERT(msg->pkt_type == DGTL_PKT_TYPE_HCI_EVT);

        pkt = (dgtl_pkt_hci_evt_t *) msg;
        pkt->code = code;
        pkt->length = param_len;

        if (param) {
                memcpy(pkt->parameters, param, param_len);
        }

        return msg;
}

void *dgtl_msg_to_raw_ptr(dgtl_msg_t *msg)
{
        return (void *) msg - get_ext_len(msg->pkt_type);

}

dgtl_msg_t *dgtl_msg_from_raw_ptr(void *ptr, uint8_t pkt_type)
{
        dgtl_msg_t *msg = ptr + get_ext_len(pkt_type);

        /* Make sure packet type is set properly - app would need to do this anyway */
        msg->data[0] = pkt_type;

        return msg;
}
