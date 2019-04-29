/**
 ****************************************************************************************
 *
 * @file ble_mgr_l2cap.c
 *
 * @brief BLE manager handlers for L2CAP API
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdlib.h>
#include <string.h>
#include "rwble_hl_error.h"
#include "ble_mgr.h"
#include "ble_mgr_gtl.h"
#include "ble_mgr_cmd.h"
#include "ble_mgr_common.h"
#include "ble_mgr_l2cap.h"
#include "ble_mgr_helper.h"
#include "ble_common.h"
#include "ble_l2cap.h"
#include "ble_storage.h"
#include "storage.h"
#include "util/queue.h"

#include "gapc_task.h"
#include "l2cc_task.h"
#include "l2cc_pdu.h"

#define SCID_BASE       (0x40)
#define SCID_MAX        (0x7F)
#define SCID_NUM        (SCID_MAX - SCID_BASE)

typedef struct {
        void *next;
        uint16_t conn_idx;
        uint16_t psm;
        uint16_t scid;
        uint16_t dcid;
        uint16_t local_credits;
        bool connecting : 1;
        bool defer_setup : 1;
} l2cap_chan_t;

/* Local COC list */
PRIVILEGED_DATA queue_t l2cap_chan;

/* Mask of allocated channels, by source CID */
PRIVILEGED_DATA uint64_t scid_mask;

/** \brief Allocate free Source CID */
static inline uint16_t alloc_scid(uint16_t conn_idx)
{
        int i;

        /*
         * For now we allocate source CID from "global" range instead of per-connection. This is ok,
         * but limits a bit number of connections possible, i.e. with 64 allowed source CIDs we can
         * have "only" 64 channels in total instead of 64 channels per-device - so probably not a
         * real limitation, we can remove this later if necessary.
         */

        for (i = 0; i < SCID_NUM; i++) {
                if ((scid_mask & (1 << i)) == 0) {
                        scid_mask |= (1 << i);
                        return SCID_BASE + i;
                }
        }

        return 0;
}

/** \brief Deallocate Source CID */
static inline void dealloc_scid(uint16_t scid)
{
        scid_mask &= ~(1 << (scid - SCID_BASE));
}

struct chan_match_data {
        uint16_t conn_idx;
        uint16_t value;
};

static bool chan_conn_idx_match(const void *data, const void *match_data)
{
        const l2cap_chan_t *chan = data;
        const uint16_t conn_idx = (uint32_t) match_data;

        return chan->conn_idx == conn_idx;
}

static bool chan_psm_match(const void *data, const void *match_data)
{
        const l2cap_chan_t *chan = data;
        const struct chan_match_data *md = match_data;

        return chan->conn_idx == md->conn_idx && chan->psm == md->value;
}

static bool chan_scid_match(const void *data, const void *match_data)
{
        const l2cap_chan_t *chan = data;
        const struct chan_match_data *md = match_data;

        /* Ignore conn_idx for now since scid is unique for all connections */

        return chan->scid == md->value;
}

static bool chan_dcid_match(const void *data, const void *match_data)
{
        const l2cap_chan_t *chan = data;
        const struct chan_match_data *md = match_data;

        return chan->conn_idx == md->conn_idx && chan->dcid == md->value;
}

static bool chan_connecting_match(const void *data, const void *match_data)
{
        const l2cap_chan_t *chan = data;
        const struct chan_match_data *md = match_data;

        return chan->conn_idx == md->conn_idx && chan->connecting;
}

static bool chan_match(const void *data, const void *match_data)
{
        return data == match_data;
}

static inline l2cap_chan_t *find_chan_by_psm(uint16_t conn_idx, uint16_t psm)
{
        struct chan_match_data md = {
                .conn_idx = conn_idx,
                .value  = psm,
        };

        return queue_find(&l2cap_chan, chan_psm_match, &md);
}

static inline l2cap_chan_t *find_chan_by_scid(uint16_t conn_idx, uint16_t scid)
{
        struct chan_match_data md = {
                .conn_idx = conn_idx,
                .value  = scid,
        };

        return queue_find(&l2cap_chan, chan_scid_match, &md);
}

static inline l2cap_chan_t *find_chan_by_dcid(uint16_t conn_idx, uint16_t scid)
{
        struct chan_match_data md = {
                .conn_idx = conn_idx,
                .value  = scid,
        };

        return queue_find(&l2cap_chan, chan_dcid_match, &md);
}

static inline l2cap_chan_t *find_chan_by_connecting(uint16_t conn_idx)
{
        struct chan_match_data md = {
                .conn_idx = conn_idx,
        };

        return queue_find(&l2cap_chan, chan_connecting_match, &md);
}

/** \brief Add new channel with unique Source CID */
static inline l2cap_chan_t *add_chan(uint16_t conn_idx)
{
        l2cap_chan_t *chan;
        uint16_t scid;

        scid = alloc_scid(conn_idx);
        if (!scid) {
                return NULL;
        }

        chan = OS_MALLOC(sizeof(*chan));
        memset(chan, 0, sizeof(*chan));
        chan->conn_idx = conn_idx;
        chan->scid = scid;

        queue_push_back(&l2cap_chan, chan);

        return chan;
}

static inline void remove_chan(l2cap_chan_t *chan)
{
        OS_ASSERT(chan);

        if (!chan) {
                return;
        }

        queue_remove(&l2cap_chan, chan_match, chan);

        dealloc_scid(chan->scid);

        OS_FREE(chan);
}

static void l2cap_listen_rsp(ble_gtl_msg_t *gtl, void *param)
{
        struct gapc_cmp_evt *gevt = gtl ? (void *) gtl->param : NULL;
        ble_mgr_l2cap_listen_rsp_t *rsp = param;

        if (!gevt) {
                rsp->status = BLE_ERROR_FAILED;
                goto done;
        }

        switch (gevt->status) {
        case GAP_ERR_NO_ERROR:
                rsp->status = BLE_STATUS_OK;
                break;
        case L2C_ERR_NO_RES_AVAIL:
                rsp->status = BLE_ERROR_INS_RESOURCES;
                break;
        case GAP_ERR_INVALID_PARAM:
                rsp->status = BLE_ERROR_INVALID_PARAM;
                break;
        case GAP_ERR_COMMAND_DISALLOWED:
                rsp->status = BLE_ERROR_NOT_ALLOWED;
                break;
        default:
                rsp->status = BLE_ERROR_FAILED;
                break;
        }

done:
        /* Remove channel from local list if failed in stack */
        if (rsp->status != BLE_STATUS_OK) {
                l2cap_chan_t *chan = find_chan_by_scid(rsp->conn_idx, rsp->scid);
                remove_chan(chan);
        }

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}
void ble_mgr_l2cap_listen_cmd_handler(void *param)
{
        const ble_mgr_l2cap_listen_cmd_t *cmd = param;
        ble_mgr_l2cap_listen_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapc_lecb_create_cmd *gcmd;
        ble_error_t ret = BLE_ERROR_FAILED;
        device_t *dev;
        l2cap_chan_t *chan;
        uint16_t conn_idx = cmd->conn_idx;

        storage_acquire();

        dev = find_device_by_conn_idx(conn_idx);
        if (!dev) {
                /* No active connection corresponds to provided index */
                ret = BLE_ERROR_NOT_CONNECTED;
                storage_release();
                goto done;
        }

        storage_release();

        /*
         * Do not allow channel creation if another one is created or connected on the same PSM
         * since the stack does not allow to manage two channels with the same PSM in a reliable way.
         */
        chan = find_chan_by_psm(conn_idx, cmd->psm);
        if (chan) {
                ret = BLE_ERROR_ALREADY_DONE;
                goto done;
        }

        chan = add_chan(conn_idx);
        if (!chan) {
                /* Cannot find free source CID */
                ret = BLE_ERROR_INS_RESOURCES;
                goto done;
        }

        chan->psm = cmd->psm;
        chan->local_credits = cmd->initial_credits;
        chan->defer_setup = cmd->defer_setup;

        /* Setup GTL message */
        gmsg = ble_gtl_alloc_with_conn(GAPC_LECB_CREATE_CMD, TASK_ID_GAPC, conn_idx, sizeof(*gcmd));
        gcmd = (struct gapc_lecb_create_cmd *) gmsg->msg.gtl.param;
        gcmd->operation = GAPC_LE_CB_CREATE;

        switch (cmd->sec_level) {
        case GAP_SEC_LEVEL_1:
                gcmd->sec_lvl = 1;
                break;
        case GAP_SEC_LEVEL_2:
                gcmd->sec_lvl = 2;
                break;
        case GAP_SEC_LEVEL_3:
                gcmd->sec_lvl = 3;
                break;
        default:
                gcmd->sec_lvl = 0;
        }

        gcmd->le_psm = cmd->psm;
        gcmd->cid = chan->scid;
        gcmd->intial_credit = cmd->initial_credits;

        /* Free command buffer */
        ble_msg_free(param);

        /* Create response */
        rsp = ble_msg_init(BLE_MGR_L2CAP_LISTEN_CMD, sizeof(*rsp));
        rsp->scid = chan->scid;
        rsp->conn_idx = conn_idx;

        ble_gtl_waitqueue_add(conn_idx, GAPC_CMP_EVT, GAPC_LE_CB_CREATE,
                              l2cap_listen_rsp, (void *) rsp);
        ble_gtl_send(gmsg);

        return;

done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_L2CAP_LISTEN_CMD, sizeof(*rsp));
        rsp->status = ret;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

static void l2cap_stop_listen_rsp(ble_gtl_msg_t *gtl, void *param)
{
        struct gapc_cmp_evt *gevt = gtl ? (void *) gtl->param : NULL;
        ble_mgr_l2cap_stop_listen_rsp_t *rsp = param;
        bool remove = true;

        if (!gevt) {
                rsp->status = BLE_ERROR_FAILED;
                goto done;
        }

        switch (gevt->status) {
        case GAP_ERR_NO_ERROR:
                rsp->status = BLE_STATUS_OK;
                break;
        case GAP_ERR_COMMAND_DISALLOWED:
                rsp->status = BLE_ERROR_NOT_ALLOWED;
                /* Do not remove channel - it was not removed in stack! */
                remove = false;
                break;
        case GAP_ERR_NOT_FOUND:
                rsp->status = BLE_ERROR_NOT_FOUND;
                break;
        default:
                rsp->status = BLE_ERROR_FAILED;
                break;
        }

done:
        if (remove) {
                l2cap_chan_t *chan = find_chan_by_scid(rsp->conn_idx, rsp->scid);
                remove_chan(chan);
        }

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_l2cap_stop_listen_cmd_handler(void *param)
{
        const ble_mgr_l2cap_stop_listen_cmd_t *cmd = param;
        ble_mgr_l2cap_stop_listen_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapc_lecb_destroy_cmd *gcmd;
        ble_error_t ret = BLE_ERROR_FAILED;
        device_t *dev;
        l2cap_chan_t *chan;
        uint16_t conn_idx = cmd->conn_idx;

        storage_acquire();

        dev = find_device_by_conn_idx(conn_idx);
        if (!dev) {
                /* No active connection corresponds to provided index */
                ret = BLE_ERROR_NOT_CONNECTED;
                storage_release();
                goto done;
        }

        storage_release();

        chan = find_chan_by_scid(conn_idx, cmd->scid);
        if (!chan) {
                ret = BLE_ERROR_NOT_FOUND;
                goto done;
        }

        /* Setup GTL message */
        gmsg = ble_gtl_alloc_with_conn(GAPC_LECB_DESTROY_CMD, TASK_ID_GAPC, conn_idx, sizeof(*gcmd));
        gcmd = (struct gapc_lecb_destroy_cmd *) gmsg->msg.gtl.param;
        gcmd->operation = GAPC_LE_CB_DESTROY;
        gcmd->le_psm = chan->psm;

        /* Free command buffer */
        ble_msg_free(param);

        /* Create response */
        rsp = ble_msg_init(BLE_MGR_L2CAP_STOP_LISTEN_CMD, sizeof(*rsp));
        rsp->scid = chan->scid;
        rsp->conn_idx = conn_idx;

        ble_gtl_waitqueue_add(conn_idx, GAPC_CMP_EVT, GAPC_LE_CB_DESTROY,
                              l2cap_stop_listen_rsp, (void *) rsp);
        ble_gtl_send(gmsg);

        return;
done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_L2CAP_STOP_LISTEN_CMD, sizeof(*rsp));
        rsp->status = ret;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_l2cap_connection_cfm_cmd_handler(void *param)
{
        const ble_mgr_l2cap_connection_cfm_cmd_t *cmd = param;
        ble_mgr_l2cap_connection_cfm_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapc_lecb_connect_cfm *gcmd;
        ble_error_t ret = BLE_STATUS_OK;
        device_t *dev;
        l2cap_chan_t *chan;
        uint16_t conn_idx = cmd->conn_idx;
        uint16_t status;

        storage_acquire();

        dev = find_device_by_conn_idx(conn_idx);
        if (!dev) {
                /* No active connection corresponds to provided index */
                ret = BLE_ERROR_NOT_CONNECTED;
                storage_release();
                goto done;
        }

        storage_release();

        chan = find_chan_by_scid(conn_idx, cmd->scid);
        if (!chan) {
                ret = BLE_ERROR_NOT_FOUND;
                goto done;
        }

        if (!chan->defer_setup) {
                ret = BLE_ERROR_NOT_ALLOWED;
                goto done;
        }

        switch (cmd->status) {
        case BLE_L2CAP_CONNECTION_SUCCESSFUL:
                status = L2C_CB_CON_SUCCESS;
                break;
        case BLE_L2CAP_CONNECTION_REFUSED_LE_PSM_NOT_SUPPORTED:
                status = L2C_CB_CON_LEPSM_NOT_SUPP;
                break;
        case BLE_L2CAP_CONNECTION_REFUSED_NO_RESOURCES_AVAILABLE:
                status = L2C_CB_CON_NO_RES_AVAIL;
                break;
        case BLE_L2CAP_CONNECTION_REFUSED_INSUFFICIENT_AUTHENTICATION:
                status = L2C_CB_CON_INS_AUTH;
                break;
        case BLE_L2CAP_CONNECTION_REFUSED_INSUFFICIENT_AUTHORIZATION:
                status = L2C_CB_CON_INS_AUTHOR;
                break;
        case BLE_L2CAP_CONNECTION_REFUSED_INSUFFICIENT_ENCRYPTION_KEY_SIZE:
                status = L2C_CB_CON_INS_EKS;
                break;
        case BLE_L2CAP_CONNECTION_REFUSED_INSUFFICIENT_ENCRYPTION:
                status = L2C_CB_CON_INS_ENCRYPTION;
                break;
        default:
                ret = BLE_ERROR_INVALID_PARAM;
                goto done;
                break;
        }

        gmsg = ble_gtl_alloc_with_conn(GAPC_LECB_CONNECT_CFM, TASK_ID_GAPC, conn_idx,
                                                                                sizeof(*gcmd));
        gcmd = (struct gapc_lecb_connect_cfm *) gmsg->msg.gtl.param;
        gcmd->le_psm = chan->psm;
        gcmd->status = status;

        ble_gtl_send(gmsg);

done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_L2CAP_CONNECTION_CFM_CMD, sizeof(*rsp));
        rsp->status = ret;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_l2cap_connect_cmd_handler(void *param)
{
        const ble_mgr_l2cap_connect_cmd_t *cmd = param;
        ble_mgr_l2cap_connect_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapc_lecb_connect_cmd *gcmd;
        ble_error_t ret = BLE_ERROR_FAILED;
        device_t *dev;
        l2cap_chan_t *chan = NULL;

        storage_acquire();

        dev = find_device_by_conn_idx(cmd->conn_idx);
        if (!dev) {
                /* No active connection corresponds to provided index */
                ret = BLE_ERROR_NOT_CONNECTED;
                storage_release();
                goto done;
        }

        storage_release();

        /*
         * Do not allow channel connection if another is being connected on the same link.
         * This is because we won't be able to distinguish between complete events in case two are
         * sent at the same time.
         */
        chan = find_chan_by_connecting(cmd->conn_idx);
        if (chan) {
                ret = BLE_ERROR_IN_PROGRESS;
                goto done;
        }

        /*
         * Do not allow channel connection if another is created or connected on the same PSM since
         * the stack does not allow to manage two channels with the same PSM in a reliable way.
         */
        chan = find_chan_by_psm(cmd->conn_idx, cmd->psm);
        if (chan) {
                ret = BLE_ERROR_ALREADY_DONE;
                goto done;
        }

        chan = add_chan(cmd->conn_idx);
        if (!chan) {
                /* Cannot find free source CID */
                ret = BLE_ERROR_INS_RESOURCES;
                goto done;
        }

        chan->psm = cmd->psm;
        chan->local_credits = cmd->initial_credits;
        chan->connecting = true;

        /* Setup GTL message */
        gmsg = ble_gtl_alloc_with_conn(GAPC_LECB_CONNECT_CMD, TASK_ID_GAPC, cmd->conn_idx,
                                                                                     sizeof(*gcmd));
        gcmd = (struct gapc_lecb_connect_cmd *) gmsg->msg.gtl.param;
        gcmd->operation = GAPC_LE_CB_CONNECTION;
        gcmd->le_psm = cmd->psm;
        gcmd->cid = chan->scid;
        gcmd->credit = cmd->initial_credits;

        ble_gtl_send(gmsg);
        ret = BLE_STATUS_OK;

done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_L2CAP_CONNECT_CMD, sizeof(*rsp));
        rsp->status = ret;
        rsp->scid = chan ? chan->scid : 0;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

static void l2cap_disconnect_rsp(ble_gtl_msg_t *gtl, void *param)
{
        struct gapc_cmp_evt *gevt = gtl ? (void *) gtl->param : NULL;
        ble_mgr_l2cap_disconnect_rsp_t *rsp = param;

        if (!gevt) {
                rsp->status = BLE_ERROR_FAILED;
                goto done;
        }

        switch (gevt->status) {
        case GAP_ERR_NO_ERROR:
                rsp->status = BLE_STATUS_OK;
                break;
        case L2C_ERR_INVALID_CID:
        case GAP_ERR_INVALID_PARAM:
                rsp->status = BLE_ERROR_INVALID_PARAM;
                break;
        case GAP_ERR_NOT_FOUND:
                rsp->status = BLE_ERROR_NOT_FOUND;
                break;
        default:
                rsp->status = BLE_ERROR_FAILED;
                break;
        }

done:
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_l2cap_disconnect_cmd_handler(void *param)
{
        const ble_mgr_l2cap_disconnect_cmd_t *cmd = param;
        ble_mgr_l2cap_disconnect_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapc_lecb_disconnect_cmd *gcmd;
        ble_error_t ret = BLE_ERROR_FAILED;
        device_t *dev;
        l2cap_chan_t *chan;
        uint16_t conn_idx = cmd->conn_idx;

        storage_acquire();

        dev = find_device_by_conn_idx(conn_idx);
        if (!dev) {
                /* No active connection corresponds to provided index */
                ret = BLE_ERROR_NOT_CONNECTED;
                storage_release();
                goto done;
        }

        storage_release();

        chan = find_chan_by_scid(conn_idx, cmd->scid);
        if (!chan) {
                ret = BLE_ERROR_NOT_FOUND;
                goto done;
        }

        /* Setup GTL message */
        gmsg = ble_gtl_alloc_with_conn(GAPC_LECB_DISCONNECT_CMD, TASK_ID_GAPC, conn_idx,
                                                                                     sizeof(*gcmd));
        gcmd = (struct gapc_lecb_disconnect_cmd *) gmsg->msg.gtl.param;
        gcmd->operation = GAPC_LE_CB_DISCONNECTION;
        gcmd->le_psm = chan->psm;

        /* Free command buffer */
        ble_msg_free(param);

        /* Create response */
        rsp = ble_msg_init(BLE_MGR_L2CAP_DISCONNECT_CMD, sizeof(*rsp));
        rsp->scid = chan->scid;

        ble_gtl_waitqueue_add(conn_idx, GAPC_CMP_EVT, GAPC_LE_CB_DISCONNECTION,
                              l2cap_disconnect_rsp, (void *) rsp);
        ble_gtl_send(gmsg);

        return;
done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_L2CAP_DISCONNECT_CMD, sizeof(*rsp));
        rsp->status = ret;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

static void l2cap_add_credits_rsp(ble_gtl_msg_t *gtl, void *param)
{
        struct gapc_cmp_evt *gevt = gtl ? (void *) gtl->param : NULL;
        ble_mgr_l2cap_add_credits_rsp_t *rsp = param;
        l2cap_chan_t *chan = find_chan_by_scid(rsp->conn_idx, rsp->scid);

        if (!gevt) {
                rsp->status = BLE_ERROR_FAILED;
                goto done;
        }

        if (!chan) {
                gevt->status = GAP_ERR_NOT_FOUND;
        } else {
                if (gevt->status == GAP_ERR_NO_ERROR) {
                        chan->local_credits += rsp->credits;
                }
        }

        switch (gevt->status) {
        case GAP_ERR_NO_ERROR:
                rsp->status = BLE_STATUS_OK;
                break;
        case GAP_ERR_INVALID_PARAM:
                rsp->status = BLE_ERROR_INVALID_PARAM;
                break;
        case GAP_ERR_NOT_FOUND:
                rsp->status = BLE_ERROR_NOT_FOUND;
                break;
        default:
                rsp->status = BLE_ERROR_FAILED;
                break;
        }

done:
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_l2cap_add_credits_cmd_handler(void *param)
{
        const ble_mgr_l2cap_add_credits_cmd_t *cmd = param;
        ble_mgr_l2cap_add_credits_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapc_lecb_add_cmd *gcmd;
        ble_error_t ret = BLE_ERROR_FAILED;
        device_t *dev;
        l2cap_chan_t *chan;
        uint16_t conn_idx = cmd->conn_idx;
        uint16_t scid = cmd->scid;

        storage_acquire();

        dev = find_device_by_conn_idx(conn_idx);
        if (!dev) {
                /* No active connection corresponds to provided index */
                ret = BLE_ERROR_NOT_CONNECTED;
                storage_release();
                goto done;
        }

        storage_release();

        chan = find_chan_by_scid(conn_idx, scid);
        if (!chan) {
                ret = BLE_ERROR_NOT_FOUND;
                goto done;
        }

        /* Setup GTL message */
        gmsg = ble_gtl_alloc_with_conn(GAPC_LECB_ADD_CMD, TASK_ID_GAPC, conn_idx, sizeof(*gcmd));
        gcmd = (struct gapc_lecb_add_cmd *) gmsg->msg.gtl.param;
        gcmd->operation = GAPC_LE_CB_ADDITION;
        gcmd->le_psm = chan->psm;
        gcmd->credit = cmd->credits;

        /* Free command buffer */
        ble_msg_free(param);

        /* Create response */
        rsp = ble_msg_init(BLE_MGR_L2CAP_ADD_CREDITS_CMD, sizeof(*rsp));

        rsp->conn_idx = conn_idx;
        rsp->scid = scid;
        rsp->credits = gcmd->credit;

        ble_gtl_waitqueue_add(conn_idx, GAPC_CMP_EVT, GAPC_LE_CB_ADDITION,
                              l2cap_add_credits_rsp, (void *) rsp);
        ble_gtl_send(gmsg);

        return;
done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_L2CAP_ADD_CREDITS_CMD, sizeof(*rsp));
        rsp->status = ret;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_l2cap_send_cmd_handler(void *param)
{
        const ble_mgr_l2cap_send_cmd_t *cmd = param;
        ble_mgr_l2cap_send_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct l2cc_pdu_send_req *gcmd;
        ble_error_t ret = BLE_ERROR_FAILED;
        device_t *dev;
        l2cap_chan_t *chan;
        size_t size;

        storage_acquire();

        dev = find_device_by_conn_idx(cmd->conn_idx);
        if (!dev) {
                /* No active connection corresponds to provided index */
                ret = BLE_ERROR_NOT_CONNECTED;
                storage_release();
                goto done;
        }

        storage_release();

        chan = find_chan_by_scid(cmd->conn_idx, cmd->scid);
        if (!chan) {
                ret = BLE_ERROR_NOT_FOUND;
                goto done;
        }

        /*
         * Calculate the length of the GTL message. We cannot simply use sizeof() since structure
         * has union as one of its members and it does not give accurate value (allocated memory
         * would be larger that what we need and we don't want to waste memory) - so just add the
         * sizes of all relevant fields of the structure.
         */
        size = sizeof(uint16_t) + // offset
                sizeof(uint16_t) + // payld_len
                sizeof(uint16_t) + // chan_id
                sizeof(uint16_t) + // code - this is uint8_t but structure is not packed so it's
                                   //        aligned as uint16_t
                sizeof(uint16_t) + // sdu_data_len
                cmd->length;

        /* Setup GTL message */
        gmsg = ble_gtl_alloc_with_conn(L2CC_PDU_SEND_REQ, TASK_ID_L2CC, cmd->conn_idx, size);
        gcmd = (struct l2cc_pdu_send_req *) gmsg->msg.gtl.param;
        gcmd->offset = 0;
        gcmd->pdu.payld_len = 0;
        gcmd->pdu.chan_id = chan->dcid;
        gcmd->pdu.data.send_lecb_data_req.code = 0;
        gcmd->pdu.data.send_lecb_data_req.sdu_data_len = cmd->length;
        memcpy(gcmd->pdu.data.send_lecb_data_req.sdu_data, cmd->data, cmd->length);

        ret = BLE_STATUS_OK;

        /* Send response immediately, we need to send event once data are sent anyway */
        ble_gtl_send(gmsg);

done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_L2CAP_SEND_CMD, sizeof(*rsp));
        rsp->status = ret;

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_l2cap_connect_ind_evt_handler(ble_gtl_msg_t *gtl)
{
        ble_evt_l2cap_connected_t *evt;
        struct gapc_lecb_connect_ind *gevt = (void *) gtl->param;
        uint16_t conn_idx;
        l2cap_chan_t *chan;

        conn_idx = TASK_2_CONNIDX(gtl->src_id);

        chan = find_chan_by_psm(conn_idx, gevt->le_psm);
        if (!chan) {
                return;
        }

        /* Update channel with destination CID */
        chan->dcid = gevt->dest_cid;
        chan->connecting = false;

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_L2CAP_CONNECTED, sizeof(*evt));
        evt->conn_idx = conn_idx;
        evt->psm = gevt->le_psm;
        evt->scid = chan->scid;
        evt->dcid = chan->dcid;
        evt->local_credits = chan->local_credits;
        evt->remote_credits = gevt->dest_credit;
        evt->mtu = gevt->max_sdu;

        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
}

void ble_mgr_l2cap_disconnect_ind_evt_handler(ble_gtl_msg_t *gtl)
{
        ble_evt_l2cap_disconnected_t *evt;
        struct gapc_lecb_disconnect_ind *gevt = (void *) gtl->param;
        uint16_t conn_idx;
        l2cap_chan_t *chan;

        conn_idx = TASK_2_CONNIDX(gtl->src_id);

        chan = find_chan_by_psm(conn_idx, gevt->le_psm);
        if (!chan) {
                return;
        }

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_L2CAP_DISCONNECTED, sizeof(*evt));
        evt->conn_idx = conn_idx;
        evt->scid = chan->scid;

        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);

        remove_chan(chan);
}

void ble_mgr_l2cap_connect_req_ind_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gapc_lecb_connect_req_ind *gevt = (void *) gtl->param;
        uint16_t status;
        uint16_t conn_idx;
        l2cap_chan_t *chan;

        conn_idx = TASK_2_CONNIDX(gtl->src_id);

        chan = find_chan_by_psm(conn_idx, gevt->le_psm);
        if (!chan) {
                status = L2C_CB_CON_LEPSM_NOT_SUPP;
        } else {
                chan->dcid = gevt->dest_cid;

                status = L2C_CB_CON_SUCCESS;
        }

        if ((status == L2C_CB_CON_SUCCESS) && chan->defer_setup) {
                ble_evt_l2cap_connection_req_t *evt;

                evt = ble_evt_init(BLE_EVT_L2CAP_CONNECTION_REQ, sizeof(*evt));
                evt->conn_idx = conn_idx;
                evt->psm = chan->psm;
                evt->scid = chan->scid;
                evt->dcid = chan->dcid;
                evt->remote_credits = gevt->dest_credit;
                evt->mtu = gevt->max_sdu;

                ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
        } else {
                ble_mgr_common_stack_msg_t *gmsg;
                struct gapc_lecb_connect_cfm *gcmd;

                gmsg = ble_gtl_alloc_with_conn(GAPC_LECB_CONNECT_CFM, TASK_ID_GAPC,
                                                TASK_2_CONNIDX(gtl->src_id), sizeof(*gcmd));
                gcmd = (struct gapc_lecb_connect_cfm *) gmsg->msg.gtl.param;
                /* For now, accept all connection requests */
                gcmd->le_psm = gevt->le_psm;
                gcmd->status = status;

                ble_gtl_send(gmsg);

                /* GAPC_LECB_CONNECT_CFM does not have a response message */
        }
}

void ble_mgr_l2cap_add_ind_evt_handler(ble_gtl_msg_t *gtl)
{
        ble_evt_l2cap_credit_changed_t *evt;
        struct gapc_lecb_add_ind *gevt = (void *) gtl->param;
        uint16_t conn_idx;
        l2cap_chan_t *chan;

        conn_idx = TASK_2_CONNIDX(gtl->src_id);

        chan = find_chan_by_psm(conn_idx, gevt->le_psm);
        if (!chan) {
                return;
        }

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_L2CAP_REMOTE_CREDITS_CHANGED, sizeof(*evt));
        evt->conn_idx = conn_idx;
        evt->scid = chan->scid;
        evt->remote_credits = gevt->dest_credit;

        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
}

void ble_mgr_l2cap_pdu_send_rsp_evt_handler(ble_gtl_msg_t *gtl)
{
        ble_evt_l2cap_sent_t *evt;
        struct l2cc_data_send_rsp *gevt = (void *) gtl->param;
        uint16_t conn_idx;
        l2cap_chan_t *chan;

        conn_idx = TASK_2_CONNIDX(gtl->src_id);

        chan = find_chan_by_dcid(conn_idx, gevt->dest_cid);
        if (!chan) {
                return;
        }

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_L2CAP_SENT, sizeof(*evt));
        evt->conn_idx = conn_idx;
        evt->scid = chan->scid;
        evt->remote_credits = gevt->dest_credit;

        switch (gevt->status) {
        case GAP_ERR_NO_ERROR:
                evt->status = BLE_STATUS_OK;
                break;
        case L2C_ERR_INSUFF_CREDIT:
                evt->status = BLE_ERROR_L2CAP_NO_CREDITS;
                break;
        case L2C_ERR_INVALID_MTU_EXCEED:
                evt->status = BLE_ERROR_L2CAP_MTU_EXCEEDED;
                break;
        default:
                evt->status = BLE_ERROR_FAILED;
                break;
        }

        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
}

void ble_mgr_l2cap_lecnx_data_recv_ind_evt_handler(ble_gtl_msg_t *gtl)
{
        ble_evt_l2cap_data_ind_t *evt;
        struct l2cc_lecnx_data_recv_ind *gevt = (void *) gtl->param;
        uint16_t conn_idx;
        l2cap_chan_t *chan;

        conn_idx = TASK_2_CONNIDX(gtl->src_id);

        chan = find_chan_by_scid(conn_idx, gevt->src_cid);
        if (!chan) {
                return;
        }

        OS_ASSERT(chan->local_credits >= gevt->src_credit);

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_L2CAP_DATA_IND, sizeof(*evt) + gevt->len);
        evt->conn_idx = conn_idx;
        evt->scid = chan->scid;
        evt->local_credits_consumed = chan->local_credits - gevt->src_credit;
        evt->length = gevt->len;
        memcpy(evt->data, gevt->data, gevt->len);

        chan->local_credits = gevt->src_credit;

        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
}

void ble_mgr_gapc_cmp__le_cb_connection_evt_handler(ble_gtl_msg_t *gtl)
{
        ble_evt_l2cap_connection_failed_t *evt;
        struct gapc_cmp_evt *gevt = (void *) gtl->param;
        uint16_t conn_idx = TASK_2_CONNIDX(gtl->src_id);
        l2cap_chan_t *chan;

        if (gevt->status == GAP_ERR_NO_ERROR) {
                /*
                 * Ignore this since there will be a connection indication and we will send (or have
                 * already sent) BLE_EVT_L2CAP_CONNECTED
                 */
                return;
        }

        chan = find_chan_by_connecting(conn_idx);
        if (!chan) {
                /* There should be channel in connecting state if request failed with an error */
                OS_ASSERT(0);
                return;
        }

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_L2CAP_CONNECTION_FAILED, sizeof(*evt));
        evt->conn_idx = conn_idx;
        evt->scid = chan->scid;

        remove_chan(chan);

        switch (gevt->status) {
        case GAP_ERR_INVALID_PARAM:
                evt->status = BLE_ERROR_INVALID_PARAM;
                break;
        case GAP_ERR_COMMAND_DISALLOWED:
                evt->status = BLE_ERROR_NOT_ALLOWED;
                break;
        case L2C_ERR_LEPSM_NOT_SUPP:
                evt->status = BLE_ERROR_NOT_SUPPORTED;
                break;
        default:
                evt->status = BLE_ERROR_FAILED;
                break;
        }

        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);

}

static void chan_destroy_func(void *data)
{
        l2cap_chan_t *chan = data;

        dealloc_scid(chan->scid);

        OS_FREE(chan);
}

void ble_mgr_l2cap_disconnect_ind(uint16_t conn_idx)
{
        queue_filter(&l2cap_chan, chan_conn_idx_match, (void *) (uint32_t) conn_idx,
                                                                                chan_destroy_func);
}
