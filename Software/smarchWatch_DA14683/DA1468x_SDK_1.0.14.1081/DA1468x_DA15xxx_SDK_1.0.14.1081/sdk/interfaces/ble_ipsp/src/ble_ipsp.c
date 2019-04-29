/**
 ****************************************************************************************
 *
 * @file ble_ipsp.c
 *
 * @brief Internet Protocol Support Profile
 *
 * Copyright (C) 2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include "osal.h"
#include "ble_gap.h"
#include "ble_l2cap.h"
#include "ble_ipsp.h"

#define IPSP_PSM                (0x0023)
#define IPSP_MTU                (1280)
#define IPSP_SDU_CREDITS        (((IPSP_MTU) / 23) + 1)
#define IPSP_CREDITS_WATERMARK  (IPSP_SDU_CREDITS)
#define IPSP_INITIAL_CREDITS    (2 * IPSP_SDU_CREDITS)

typedef enum {
        IPSP_STATE_UNUSED,
        IPSP_STATE_IDLE,
        IPSP_STATE_LISTEN,
        IPSP_STATE_ACCEPTED,
        IPSP_STATE_CONNECTING,
        IPSP_STATE_CONNECTED,
} ipsp_state_t;

typedef struct {
        uint16_t conn_idx;
        uint16_t scid;
        uint16_t pending_credits;
        ipsp_state_t state;
        bool flow_stop : 1;
        bool tx_in_progress : 1;
} ipsp_t;

typedef struct {
        uint32_t notif_mask;
        OS_TASK task;
        OS_QUEUE queue;
} ble_context_t;

typedef enum {
        BLE_IPSP_EVT_CONNECT,
        BLE_IPSP_EVT_DISCONNECT,
        BLE_IPSP_EVT_SEND,
} ble_ipsp_evt_code_t;

typedef struct {
        ble_ipsp_evt_code_t code;
        uint16_t conn_idx;
        uint16_t length;
        uint8_t data[0];
} ble_ipsp_evt_t;

__RETAINED static ble_context_t ble_context;
__RETAINED static ipsp_t channels[BLE_GAP_MAX_CONNECTED];
__RETAINED static const ble_ipsp_callbacks_t *callbacks;
__RETAINED static ble_ipsp_role_t ipsp_role;
__RETAINED static gap_sec_level_t sec_level;

static void ipsp_reset(ipsp_t *ipsp)
{
        ipsp->conn_idx = BLE_CONN_IDX_INVALID;
        ipsp->scid = 0;
        ipsp->state = IPSP_STATE_UNUSED;
        ipsp->flow_stop = false;
}

void ble_ipsp_init(const ble_ipsp_config_t *config)
{
        int i;

        ipsp_role = config->role;
        sec_level = config->sec_level;
        ble_context.notif_mask = config->notif_mask;
        ble_context.task = OS_GET_CURRENT_TASK();
        OS_QUEUE_CREATE(ble_context.queue, sizeof(ble_ipsp_evt_t *), BLE_IPSP_EVT_QUEUE_LENGTH);

        for (i = 0; i < BLE_GAP_MAX_CONNECTED; i++) {
                ipsp_reset(&channels[i]);
        }
}

void ble_ipsp_register_callbacks(const ble_ipsp_callbacks_t *cb)
{
        callbacks = cb;
}

static ipsp_t *find_channel(uint16_t conn_idx)
{
        int i;

        for (i = 0; i < BLE_GAP_MAX_CONNECTED; i++) {
                if (channels[i].conn_idx == conn_idx) {
                        return &channels[i];
                }
        }

        return NULL;
}

static ipsp_t *find_free_channel(void)
{
        return find_channel(BLE_CONN_IDX_INVALID);
}

static int connected_channels(void)
{
        int connected_channels = 0;
        int i;

        for (i = 0; i < BLE_GAP_MAX_CONNECTED; i++) {
                if (channels[i].state == IPSP_STATE_CONNECTED) {
                        connected_channels++;
                }
        }

        return connected_channels;
}

static void handle_gap_connected(const ble_evt_gap_connected_t *evt)
{
        ipsp_t *ipsp = find_free_channel();

        if (!ipsp) {
                return;
        }

        ipsp->conn_idx = evt->conn_idx;

        if (ipsp_role & BLE_IPSP_ROLE_NODE) {
                ipsp->state = IPSP_STATE_LISTEN;
                ble_l2cap_listen_defer_setup(ipsp->conn_idx, IPSP_PSM, sec_level,
                                                                IPSP_INITIAL_CREDITS, &ipsp->scid);
        } else {
                ipsp->state = IPSP_STATE_IDLE;
        }
}

static void handle_gap_disconnected(const ble_evt_gap_disconnected_t *evt)
{
        ipsp_t *ipsp = find_channel(evt->conn_idx);

        if (!ipsp) {
                return;
        }

        if (ipsp->state == IPSP_STATE_CONNECTED) {
                /* Notify upper layer */
                if (callbacks && callbacks->disconnected) {
                        callbacks->disconnected(ipsp->conn_idx, evt->reason);
                }
        }

        ipsp_reset(ipsp);
}

static void handle_l2cap_connected(const ble_evt_l2cap_connected_t *evt)
{
        ipsp_t *ipsp = find_channel(evt->conn_idx);

        if (!ipsp) {
                return;
        }

        /* Check if we are in valid state */
        if (ipsp->state != IPSP_STATE_CONNECTING && ipsp->state != IPSP_STATE_ACCEPTED) {
                return;
        }

        ipsp->state = IPSP_STATE_CONNECTED;
        ipsp->pending_credits = 0;

        if (!evt->remote_credits) {
               ipsp->flow_stop = true;
        } else {
               ipsp->flow_stop = false;
        }

        if (callbacks && callbacks->connected) {
                callbacks->connected(ipsp->conn_idx);
        }
}

static void handle_l2cap_connection_req(const ble_evt_l2cap_connection_req_t *evt)
{
        ipsp_t *ipsp = find_channel(evt->conn_idx);

        if (ipsp->state != IPSP_STATE_LISTEN) {
                return;
        }

        if (connected_channels() >= BLE_IPSP_MAX_OPENED_CHANNELS) {
                ble_l2cap_connection_cfm(evt->conn_idx, evt->scid,
                                        BLE_L2CAP_CONNECTION_REFUSED_NO_RESOURCES_AVAILABLE);
                return;
        }

        ble_l2cap_connection_cfm(evt->conn_idx, evt->scid, BLE_L2CAP_CONNECTION_SUCCESSFUL);

        ipsp->state = IPSP_STATE_ACCEPTED;
}

static void handle_l2cap_disconnected(const ble_evt_l2cap_disconnected_t *evt)
{
        ipsp_t *ipsp = find_channel(evt->conn_idx);

        if (!ipsp || ipsp->scid != evt->scid || ipsp->state != IPSP_STATE_CONNECTED) {
                return;
        }

        ipsp->state = IPSP_STATE_IDLE;
        ipsp->scid = 0;

        if (ipsp->tx_in_progress) {
                ipsp->tx_in_progress = false;

                if (callbacks && callbacks->sent) {
                        callbacks->sent(ipsp->conn_idx, BLE_ERROR_FAILED);
                }
        }

        if (callbacks && callbacks->disconnected) {
                callbacks->disconnected(ipsp->conn_idx, evt->reason);
        }

        if (ipsp_role & BLE_IPSP_ROLE_NODE) {
                ble_l2cap_listen_defer_setup(ipsp->conn_idx, IPSP_PSM, sec_level,
                                                                IPSP_INITIAL_CREDITS, &ipsp->scid);
                ipsp->state = IPSP_STATE_LISTEN;
        } else {
                ipsp->state = IPSP_STATE_IDLE;
        }
}

static void handle_l2cap_data_ind(const ble_evt_l2cap_data_ind_t *evt)
{
        ipsp_t *ipsp = find_channel(evt->conn_idx);

        if (!ipsp || ipsp->scid != evt->scid || ipsp->state != IPSP_STATE_CONNECTED) {
                return;
        }

        ipsp->pending_credits += evt->local_credits_consumed;

        if (ipsp->pending_credits >= IPSP_CREDITS_WATERMARK) {
                /* Give back credits immediately, we don't process data */
                ble_l2cap_add_credits(evt->conn_idx, evt->scid, ipsp->pending_credits);
                ipsp->pending_credits = 0;
        }

        if (callbacks && callbacks->data_ind) {
                callbacks->data_ind(ipsp->conn_idx, evt->length, evt->data);
        }
}

static void handle_l2cap_credits_changed(const ble_evt_l2cap_credit_changed_t *evt)
{
        ipsp_t *ipsp = find_channel(evt->conn_idx);

        if (!ipsp || ipsp->scid != evt->scid  || ipsp->state != IPSP_STATE_CONNECTED) {
                return;
        }

        if (!evt->remote_credits || !ipsp->flow_stop) {
                return;
        }

        ipsp->flow_stop = false;
}

static void handle_l2cap_sent(const ble_evt_l2cap_sent_t *evt)
{
        ipsp_t *ipsp = find_channel(evt->conn_idx);

        if (!ipsp || ipsp->scid != evt->scid  || ipsp->state != IPSP_STATE_CONNECTED) {
                return;
        }

        ipsp->tx_in_progress = false;

        if (evt->status != BLE_STATUS_OK) {
                goto done;
        }

        if (!evt->remote_credits) {
               ipsp->flow_stop = true;
        }

done:
        if (callbacks && callbacks->sent) {
                callbacks->sent(ipsp->conn_idx, evt->status);
        }
}

static void handle_l2cap_connection_failed(const ble_evt_l2cap_connection_failed_t *evt)
{
        ipsp_t *ipsp = find_channel(evt->conn_idx);

        if (!ipsp || ipsp->scid != evt->scid  || ipsp->state != IPSP_STATE_CONNECTING) {
                return;
        }

        if (ipsp_role & BLE_IPSP_ROLE_NODE) {
                ble_l2cap_listen_defer_setup(ipsp->conn_idx, IPSP_PSM, sec_level,
                                                                IPSP_SDU_CREDITS, &ipsp->scid);
                ipsp->state = IPSP_STATE_LISTEN;
        } else {
                ipsp->state = IPSP_STATE_IDLE;
        }

        if (callbacks && callbacks->connection_failed) {
                callbacks->connection_failed(evt->conn_idx);
        }
}

static void ipsp_disconnect(uint16_t conn_idx)
{
        ipsp_t *ipsp = find_channel(conn_idx);

        if (!ipsp) {
                return;
        }

        if (ipsp->state != IPSP_STATE_CONNECTED) {
                return;
        }

        ble_l2cap_disconnect(ipsp->conn_idx, ipsp->scid);
}

static void destroy_ble_ipsp_evt(ble_ipsp_evt_t *evt)
{
        OS_FREE(evt);
}

static bool ipsp_send(uint16_t conn_idx, uint16_t length, const uint8_t *data)
{
        ipsp_t *ipsp = find_channel(conn_idx);
        ble_error_t err;

        if (!ipsp || ipsp->state != IPSP_STATE_CONNECTED || ipsp->tx_in_progress) {
                return false;
        }

        if (ipsp->flow_stop) {
                return false;
        }

        err = ble_l2cap_send(ipsp->conn_idx, ipsp->scid, length, data);

        if (!err) {
                ipsp->tx_in_progress = true;
        }

        return err == BLE_STATUS_OK;
}

static bool ipsp_connect(uint16_t conn_idx)
{
        ble_error_t status;
        ipsp_t *ipsp;

        if (!(ipsp_role & BLE_IPSP_ROLE_ROUTER)) {
                return false;
        }

        if (connected_channels() >= BLE_IPSP_MAX_OPENED_CHANNELS) {
                return false;
        }

        ipsp = find_channel(conn_idx);
        if (!ipsp || ipsp->state == IPSP_STATE_CONNECTED || ipsp->state == IPSP_STATE_CONNECTING) {
                return false;
        }

        if (ipsp->state == IPSP_STATE_LISTEN) {
                ble_l2cap_stop_listen(ipsp->conn_idx, ipsp->scid);
                ipsp->state = IPSP_STATE_IDLE;
                ipsp->scid = 0;
        }

        status = ble_l2cap_connect(conn_idx, IPSP_PSM, IPSP_INITIAL_CREDITS, &ipsp->scid);
        if (status != BLE_STATUS_OK) {
                return false;
        }

        ipsp->state = IPSP_STATE_CONNECTING;
        return true;
}


static ble_ipsp_evt_t *create_ble_ipsp_evt(ble_ipsp_evt_code_t code, uint16_t conn_idx,
                                                                                uint16_t data_len)
{
        ble_ipsp_evt_t *evt = OS_MALLOC(sizeof(ble_ipsp_evt_t) + data_len);

        memset(evt, 0, sizeof(*evt) + data_len);
        evt->conn_idx = conn_idx;
        evt->code = code;
        evt->length = data_len;

        return evt;
}

bool ble_ipsp_disconnect(uint16_t conn_idx)
{
        ble_ipsp_evt_t *evt = create_ble_ipsp_evt(BLE_IPSP_EVT_DISCONNECT, conn_idx, 0);

        if (OS_QUEUE_PUT(ble_context.queue, &evt, OS_QUEUE_NO_WAIT) != OS_QUEUE_OK) {
                destroy_ble_ipsp_evt(evt);
                return false;
        }

        OS_TASK_NOTIFY(ble_context.task, ble_context.notif_mask, OS_NOTIFY_SET_BITS);

        return true;
}

bool ble_ipsp_send(uint16_t conn_idx, uint16_t length, const uint8_t *data)
{
        ble_ipsp_evt_t *evt;

        if (length == 0 || !data) {
                return false;
        }

        evt = create_ble_ipsp_evt(BLE_IPSP_EVT_SEND, conn_idx, length);
        memcpy(evt->data, data, length);

        if (OS_QUEUE_PUT(ble_context.queue, &evt, OS_QUEUE_NO_WAIT) != OS_QUEUE_OK) {
                destroy_ble_ipsp_evt(evt);
                return false;
        }

        OS_TASK_NOTIFY(ble_context.task, ble_context.notif_mask, OS_NOTIFY_SET_BITS);

        return true;
}

bool ble_ipsp_connect(uint16_t conn_idx)
{
        ble_ipsp_evt_t *evt = create_ble_ipsp_evt(BLE_IPSP_EVT_CONNECT, conn_idx, 0);

        if (OS_QUEUE_PUT(ble_context.queue, &evt, OS_QUEUE_NO_WAIT) != OS_QUEUE_OK) {
                destroy_ble_ipsp_evt(evt);
                return false;
        }

        OS_TASK_NOTIFY(ble_context.task, ble_context.notif_mask, OS_NOTIFY_SET_BITS);

        return true;
}

void ble_ipsp_handle_notified(void)
{
        ble_ipsp_evt_t *evt;

        if (OS_QUEUE_GET(ble_context.queue, &evt, OS_QUEUE_NO_WAIT) != OS_QUEUE_OK) {
                return;
        }

        switch (evt->code) {
        case BLE_IPSP_EVT_CONNECT:
                if (!ipsp_connect(evt->conn_idx) && callbacks && callbacks->connection_failed) {
                        callbacks->connection_failed(evt->conn_idx);
                }
                break;
        case BLE_IPSP_EVT_DISCONNECT:
                ipsp_disconnect(evt->conn_idx);
                break;
        case BLE_IPSP_EVT_SEND:
                if (!ipsp_send(evt->conn_idx, evt->length, evt->data) && callbacks &&
                                                                                callbacks->sent)
                        callbacks->sent(evt->conn_idx, BLE_ERROR_FAILED);
                break;
        }

        destroy_ble_ipsp_evt(evt);
}

void ble_ipsp_handle_event(const ble_evt_hdr_t *evt)
{
        switch (evt->evt_code) {
        case BLE_EVT_GAP_CONNECTED:
                handle_gap_connected((const ble_evt_gap_connected_t *) evt);
                break;
        case BLE_EVT_GAP_DISCONNECTED:
                handle_gap_disconnected((const ble_evt_gap_disconnected_t *) evt);
                break;
        case BLE_EVT_L2CAP_CONNECTED:
                handle_l2cap_connected((const ble_evt_l2cap_connected_t *) evt);
                break;
        case BLE_EVT_L2CAP_CONNECTION_FAILED:
                handle_l2cap_connection_failed((const ble_evt_l2cap_connection_failed_t *) evt);
                break;
        case BLE_EVT_L2CAP_CONNECTION_REQ:
                handle_l2cap_connection_req((const ble_evt_l2cap_connection_req_t *) evt);
                break;
        case BLE_EVT_L2CAP_DISCONNECTED:
                handle_l2cap_disconnected((const ble_evt_l2cap_disconnected_t *) evt);
                break;
        case BLE_EVT_L2CAP_DATA_IND:
                handle_l2cap_data_ind((const ble_evt_l2cap_data_ind_t *) evt);
                break;
        case BLE_EVT_L2CAP_REMOTE_CREDITS_CHANGED:
                handle_l2cap_credits_changed((const ble_evt_l2cap_credit_changed_t *) evt);
                break;
        case BLE_EVT_L2CAP_SENT:
                handle_l2cap_sent((const ble_evt_l2cap_sent_t *) evt);
                break;
        }
}
