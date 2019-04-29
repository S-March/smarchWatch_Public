/**
 ****************************************************************************************
 *
 * @file ble_mgr_gattc.c
 *
 * @brief BLE manager handlers for GATTC API
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <string.h>
#include "ble_mgr.h"
#include "ble_mgr_gtl.h"
#include "ble_mgr_cmd.h"
#include "ble_mgr_common.h"
#include "ble_mgr_gattc.h"
#include "ble_mgr_helper.h"
#include "ble_common.h"
#include "ble_att.h"
#include "ble_gattc.h"
#include "ble_uuid.h"
#include "ble_storage.h"
#include "storage.h"

#include "gattc_task.h"

/** \brief Convert RW UUID to Dialog UUID */
static void uuid_rw2dg(uint8_t uuid_len, const uint8_t *uuid, att_uuid_t *uuid_out)
{
        if (uuid_len == 2) {
                ble_uuid_create16(uuid[1] << 8 | uuid[0], uuid_out);
        } else {
                ble_uuid_from_buf(uuid, uuid_out);
        }
}

/** \brief Convert Dialog UUID to RW UUID */
static void uuid_dg2rw(const att_uuid_t *uuid, uint8_t *uuid_out_len, uint8_t *uuid_out)
{
        if (uuid->type == ATT_UUID_16) {
                *uuid_out_len = 2;
                uuid_out[0] = uuid->uuid16;
                uuid_out[1] = uuid->uuid16 >> 8;
        } else {
                *uuid_out_len = 16;
                memcpy(uuid_out, uuid->uuid128, 16);
        }
}

void ble_mgr_gattc_browse_cmd_handler(void *param)
{
        const ble_mgr_gattc_browse_cmd_t *cmd = param;
        ble_mgr_gattc_browse_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gattc_sdp_svc_disc_cmd *gcmd;
        ble_error_t ret = BLE_ERROR_FAILED;
        device_t *dev;
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

        /* Setup GTL message */
        gmsg = ble_gtl_alloc_with_conn(GATTC_SDP_SVC_DISC_CMD, TASK_ID_GATTC, cmd->conn_idx, sizeof(struct gattc_sdp_svc_disc_cmd));
        gcmd = (struct gattc_sdp_svc_disc_cmd *) gmsg->msg.gtl.param;
        if (cmd->uuid) {
                gcmd->operation = GATTC_SDP_DISC_SVC;
                uuid_dg2rw(cmd->uuid, &gcmd->uuid_len, gcmd->uuid);
        } else {
                gcmd->operation = GATTC_SDP_DISC_SVC_ALL;
        }
        gcmd->seq_num = cmd->conn_idx; // use seq_num to hold conn_idx
        gcmd->start_hdl = 1;
        gcmd->end_hdl = 65535;

        ble_gtl_send(gmsg);

        ret = BLE_STATUS_OK;

done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GATTC_BROWSE_CMD, sizeof(*rsp));
        rsp->status = ret;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gattc_discover_svc_cmd_handler(void *param)
{
        const ble_mgr_gattc_discover_svc_cmd_t *cmd = param;
        ble_mgr_gattc_discover_svc_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gattc_disc_cmd *gcmd;
        ble_error_t ret = BLE_ERROR_FAILED;
        device_t *dev;
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

        /* Setup GTL message */
        gmsg = ble_gtl_alloc_with_conn(GATTC_DISC_CMD, TASK_ID_GATTC, cmd->conn_idx,
                                                sizeof(struct gattc_disc_cmd) + ATT_UUID_128_LEN);
        gcmd = (struct gattc_disc_cmd *) gmsg->msg.gtl.param;
        if (cmd->uuid) {
                gcmd->operation = GATTC_DISC_BY_UUID_SVC;
                uuid_dg2rw(cmd->uuid, &gcmd->uuid_len, gcmd->uuid);
        } else {
                gcmd->operation = GATTC_DISC_ALL_SVC;
                /* UUID has to be "set" to 0x0000 for this to work */
                gcmd->uuid_len = 2;
        }
        gcmd->seq_num = cmd->conn_idx; // use seq_num to hold conn_idx
        gcmd->start_hdl = 1;
        gcmd->end_hdl = 65535;

        ble_gtl_send(gmsg);

        ret = BLE_STATUS_OK;

done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GATTC_DISCOVER_SVC_CMD, sizeof(*rsp));
        rsp->status = ret;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gattc_discover_include_cmd_handler(void *param)
{
        const ble_mgr_gattc_discover_include_cmd_t *cmd = param;
        ble_mgr_gattc_discover_include_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gattc_disc_cmd *gcmd;
        ble_error_t ret = BLE_ERROR_FAILED;
        device_t *dev;
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

        /* Setup GTL message */
        gmsg = ble_gtl_alloc_with_conn(GATTC_DISC_CMD, TASK_ID_GATTC, cmd->conn_idx,
                                                                        sizeof(struct gattc_disc_cmd));
        gcmd = (struct gattc_disc_cmd *) gmsg->msg.gtl.param;
        gcmd->operation = GATTC_DISC_INCLUDED_SVC;
        gcmd->seq_num = cmd->conn_idx; // use seq_num to hold conn_idx
        gcmd->start_hdl = cmd->start_h;
        gcmd->end_hdl = cmd->end_h;

        ble_gtl_send(gmsg);

        ret = BLE_STATUS_OK;

done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GATTC_DISCOVER_INCLUDE_CMD, sizeof(*rsp));
        rsp->status = ret;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gattc_discover_char_cmd_handler(void *param)
{
        const ble_mgr_gattc_discover_char_cmd_t *cmd = param;
        ble_mgr_gattc_discover_char_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gattc_disc_cmd *gcmd;
        ble_error_t ret = BLE_ERROR_FAILED;
        device_t *dev;
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

        /* Setup GTL message */
        gmsg = ble_gtl_alloc_with_conn(GATTC_DISC_CMD, TASK_ID_GATTC, cmd->conn_idx,
                                                sizeof(struct gattc_disc_cmd) + ATT_UUID_128_LEN);
        gcmd = (struct gattc_disc_cmd *) gmsg->msg.gtl.param;
        if (cmd->uuid) {
                gcmd->operation = GATTC_DISC_BY_UUID_CHAR;
                uuid_dg2rw(cmd->uuid, &gcmd->uuid_len, gcmd->uuid);
        } else {
                /* UUID has to be "set" to 0x0000 for this to work */
                gcmd->uuid_len = 2;
                gcmd->operation = GATTC_DISC_ALL_CHAR;
        }
        gcmd->seq_num = cmd->conn_idx; // use seq_num to hold conn_idx
        gcmd->start_hdl = cmd->start_h;
        gcmd->end_hdl = cmd->end_h;

        ble_gtl_send(gmsg);

        ret = BLE_STATUS_OK;

done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GATTC_DISCOVER_CHAR_CMD, sizeof(*rsp));
        rsp->status = ret;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gattc_discover_desc_cmd_handler(void *param)
{
        const ble_mgr_gattc_discover_desc_cmd_t *cmd = param;
        ble_mgr_gattc_discover_desc_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gattc_disc_cmd *gcmd;
        ble_error_t ret = BLE_ERROR_FAILED;
        device_t *dev;
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

        /* setup GTL message */
        gmsg = ble_gtl_alloc_with_conn(GATTC_DISC_CMD, TASK_ID_GATTC, cmd->conn_idx, sizeof(struct gattc_disc_cmd));
        gcmd = (struct gattc_disc_cmd *) gmsg->msg.gtl.param;
        gcmd->operation = GATTC_DISC_DESC_CHAR;
        /* UUID has to be "set" to 0x0000 for this to work */
        gcmd->uuid_len = 2;
        gcmd->seq_num = cmd->conn_idx; // use seq_num to hold conn_idx
        gcmd->start_hdl = cmd->start_h;
        gcmd->end_hdl = cmd->end_h;

        ble_gtl_send(gmsg);

        ret = BLE_STATUS_OK;

done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GATTC_DISCOVER_DESC_CMD, sizeof(*rsp));
        rsp->status = ret;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gattc_read_cmd_handler(void *param)
{
        const ble_mgr_gattc_read_cmd_t *cmd = param;
        ble_mgr_gattc_read_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gattc_read_cmd *gcmd;
        ble_error_t ret = BLE_ERROR_FAILED;
        device_t *dev;
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

        /* Setup GTL message */
        gmsg = ble_gtl_alloc_with_conn(GATTC_READ_CMD, TASK_ID_GATTC, cmd->conn_idx, sizeof(*gcmd));
        gcmd = (typeof(gcmd)) gmsg->msg.gtl.param;
        gcmd->operation = GATTC_READ;
        gcmd->seq_num = cmd->handle; // use seq_num to hold attribute handle
        gcmd->req.simple.handle = cmd->handle;
        gcmd->req.simple.offset = cmd->offset;
        gcmd->req.simple.length = 0;

        ble_gtl_send(gmsg);

        ret = BLE_STATUS_OK;

done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GATTC_READ_CMD, sizeof(*rsp));
        rsp->status = ret;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gattc_write_generic_cmd_handler(void *param)
{
        const ble_mgr_gattc_write_generic_cmd_t *cmd = param;
        ble_mgr_gattc_write_generic_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gattc_write_cmd *gcmd;
        ble_error_t ret = BLE_ERROR_FAILED;
        device_t *dev;
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

        /* Setup GTL message */
        gmsg = ble_gtl_alloc_with_conn(GATTC_WRITE_CMD, TASK_ID_GATTC, cmd->conn_idx,
                                                                        sizeof(*gcmd) + cmd->length);
        gcmd = (typeof(gcmd)) gmsg->msg.gtl.param;
        if (cmd->no_response) {
                gcmd->operation = GATTC_WRITE_NO_RESPONSE;

                if (cmd->signed_write) {
                        device_t *dev;

                        storage_acquire();

                        dev = find_device_by_conn_idx(cmd->conn_idx);
                        if (!dev) {
                                storage_release();
                                goto done;
                        }

                        /* Use signed write if connection is not encrypted */
                        if (!dev->encrypted) {
                                gcmd->operation = GATTC_WRITE_SIGNED;
                        }

                        storage_release();
                }
        } else {
                gcmd->operation = GATTC_WRITE;
                gcmd->auto_execute = cmd->prepare ? 0 : 1;
                gcmd->offset = cmd->offset;
        }
        gcmd->seq_num = cmd->handle; // use seq_num to hold attribute handle
        gcmd->handle = cmd->handle;
        gcmd->length = cmd->length;
        memcpy(gcmd->value, cmd->value, cmd->length);

        ble_gtl_send(gmsg);

        ret = BLE_STATUS_OK;

done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GATTC_WRITE_GENERIC_CMD, sizeof(*rsp));
        rsp->status = ret;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gattc_write_execute_cmd_handler(void *param)
{
        const ble_mgr_gattc_write_execute_cmd_t *cmd = param;
        ble_mgr_gattc_write_execute_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gattc_execute_write_cmd *gcmd;
        ble_error_t ret = BLE_ERROR_FAILED;
        device_t *dev;
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

        /* Setup GTL message */
        gmsg = ble_gtl_alloc_with_conn(GATTC_EXECUTE_WRITE_CMD, TASK_ID_GATTC,
                                                                cmd->conn_idx, sizeof(*gcmd));
        gcmd = (typeof(gcmd)) gmsg->msg.gtl.param;
        gcmd->operation = GATTC_EXEC_WRITE;
        gcmd->execute = cmd->commit;
        gcmd->seq_num = 0; // make this '0' so we can handle in the same handler as other writes
                           // attribute handle '0' means 'execute write' there

        ble_gtl_send(gmsg);

        ret = BLE_STATUS_OK;

done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GATTC_WRITE_EXECUTE_CMD, sizeof(*rsp));
        rsp->status = ret;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gattc_exchange_mtu_cmd_handler(void *param)
{
        const ble_mgr_gattc_exchange_mtu_cmd_t *cmd = param;
        ble_mgr_gattc_exchange_mtu_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gattc_exc_mtu_cmd *gcmd;
        ble_error_t ret = BLE_ERROR_FAILED;
        device_t *dev;
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

        /* Setup GTL message */
        gmsg = ble_gtl_alloc_with_conn(GATTC_EXC_MTU_CMD, TASK_ID_GATTC, cmd->conn_idx, sizeof(*gcmd));
        gcmd = (typeof(gcmd)) gmsg->msg.gtl.param;
        gcmd->operation = GATTC_MTU_EXCH;
        gcmd->seq_num = 0x00;

        ble_gtl_send(gmsg);

        ret = BLE_STATUS_OK;

done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GATTC_EXCHANGE_MTU_CMD, sizeof(*rsp));
        rsp->status = ret;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gattc_mtu_changed_ind_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gattc_mtu_changed_ind *gevt = (void *) gtl->param;
        ble_evt_gattc_mtu_changed_t *evt;
        device_t *dev;

        storage_acquire();

        dev = find_device_by_conn_idx(TASK_2_CONNIDX(gtl->src_id));
        if (dev) {
                dev->mtu = gevt->mtu;
        }

        storage_release();

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_GATTC_MTU_CHANGED, sizeof(*evt));
        evt->conn_idx = TASK_2_CONNIDX(gtl->src_id);
        evt->mtu = gevt->mtu;

        /* Send to event queue */
        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
}

void ble_mgr_gattc_sdp_svc_ind_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gattc_sdp_svc_ind *gevt = (void *) gtl->param;
        ble_evt_gattc_browse_svc_t *evt;
        int num_items = 0;
        int i;

        for (i = 0; i < gevt->end_hdl - gevt->start_hdl; i++) {
                uint8_t att_type = gevt->info[i].att_type;

                /* Value is not a separate item - it's part of the characteristic item */
                if (att_type == GATTC_SDP_NONE || att_type == GATTC_SDP_ATT_VAL) {
                        continue;
                }

                num_items++;
        }

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_GATTC_BROWSE_SVC, sizeof(*evt)
                                                                + num_items * sizeof(evt->items[0]));
        evt->conn_idx = TASK_2_CONNIDX(gtl->src_id);
        evt->start_h = gevt->start_hdl;
        evt->end_h = gevt->end_hdl;
        uuid_rw2dg(gevt->uuid_len, gevt->uuid, &evt->uuid);

        for (i = 0; i < gevt->end_hdl - gevt->start_hdl; i++) {
                union gattc_sdp_att_info *info = &gevt->info[i];
                gattc_item_t *item;

                if (info->att_type == GATTC_SDP_NONE || info->att_type == GATTC_SDP_ATT_VAL) {
                        continue;
                }

                item = &evt->items[evt->num_items++];
                item->handle = gevt->start_hdl + i + 1;

                switch (info->att_type) {
                case GATTC_SDP_INC_SVC:
                        item->type = GATTC_ITEM_TYPE_INCLUDE;
                        item->i.start_h = info->inc_svc.start_hdl;
                        item->i.end_h = info->inc_svc.end_hdl;
                        uuid_rw2dg(info->inc_svc.uuid_len, info->inc_svc.uuid, &item->uuid);
                        break;
                case GATTC_SDP_ATT_CHAR:
                        item->type = GATTC_ITEM_TYPE_CHARACTERISTIC;
                        item->c.value_handle = info->att_char.handle;
                        item->c.properties = info->att_char.prop;

                        /*
                         * Now we need to move to the value attribute to retrieve UUID from there
                         * later, value attribute will be ignored since it's no more useful.
                         */
                        if (info->att_char.handle > gevt->end_hdl) {
                                /* Should not happen (assuming data sent from RW are valid) */
                                goto failed;
                        }

                        info = &gevt->info[info->att_char.handle - gevt->start_hdl - 1];
                        if (info->att_type != GATTC_SDP_ATT_VAL) {
                                /* Should not happen (i.e. data sent from BLE stack are valid) */
                                goto failed;
                        }

                        uuid_rw2dg(info->att.uuid_len, info->att.uuid, &item->uuid);
                        break;
                case GATTC_SDP_ATT_DESC:
                        item->type = GATTC_ITEM_TYPE_DESCRIPTOR;
                        uuid_rw2dg(info->att.uuid_len, info->att.uuid, &item->uuid);
                        break;
                }
        }

        /* Send to event queue */
        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);

        return;

failed:
        OS_FREE(evt);

        /* Silently discard */
}

void ble_mgr_gattc_cmp__browse_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gattc_cmp_evt *gevt = (void *) gtl->param;
        ble_evt_gattc_browse_completed_t *evt;

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_GATTC_BROWSE_COMPLETED, sizeof(*evt));
        evt->conn_idx = TASK_2_CONNIDX(gtl->src_id);
        evt->status = gevt->status == ATT_ERR_NO_ERROR
                        || gevt->status == ATT_ERR_ATTRIBUTE_NOT_FOUND ? BLE_STATUS_OK : BLE_ERROR_FAILED;

        /* Send to event queue */
        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
}

void ble_mgr_gattc_disc_svc_ind_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gattc_disc_svc_ind *gevt = (void *) gtl->param;
        ble_evt_gattc_discover_svc_t *evt;

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_GATTC_DISCOVER_SVC, sizeof(*evt));
        evt->conn_idx = TASK_2_CONNIDX(gtl->src_id);
        evt->start_h = gevt->start_hdl;
        evt->end_h = gevt->end_hdl;
        uuid_rw2dg(gevt->uuid_len, gevt->uuid, &evt->uuid);

        /* Send to event queue */
        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
}

void ble_mgr_gattc_disc_svc_incl_ind_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gattc_disc_svc_incl_ind *gevt = (void *) gtl->param;
        ble_evt_gattc_discover_include_t *evt;

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_GATTC_DISCOVER_INCLUDE, sizeof(*evt));
        evt->conn_idx = TASK_2_CONNIDX(gtl->src_id);
        evt->handle = gevt->attr_hdl;
        evt->start_h = gevt->start_hdl;
        evt->end_h = gevt->end_hdl;
        uuid_rw2dg(gevt->uuid_len, gevt->uuid, &evt->uuid);

        /* Send to event queue */
        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
}

void ble_mgr_gattc_disc_char_ind_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gattc_disc_char_ind *gevt = (void *) gtl->param;
        ble_evt_gattc_discover_char_t *evt;

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_GATTC_DISCOVER_CHAR, sizeof(*evt));
        evt->conn_idx = TASK_2_CONNIDX(gtl->src_id);
        uuid_rw2dg(gevt->uuid_len, gevt->uuid, &evt->uuid);
        evt->handle = gevt->attr_hdl;
        evt->value_handle = gevt->pointer_hdl;
        evt->properties = gevt->prop;

        /* Send to event queue */
        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
}

void ble_mgr_gattc_disc_char_desc_ind_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gattc_disc_char_desc_ind *gevt = (void *) gtl->param;
        ble_evt_gattc_discover_desc_t *evt;

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_GATTC_DISCOVER_DESC, sizeof(*evt));
        evt->conn_idx = TASK_2_CONNIDX(gtl->src_id);
        uuid_rw2dg(gevt->uuid_len, gevt->uuid, &evt->uuid);
        evt->handle = gevt->attr_hdl;

        /* Send to event queue */
        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
}

void ble_mgr_gattc_cmp__discovery_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gattc_cmp_evt *gevt = (void *) gtl->param;
        ble_evt_gattc_discover_completed_t *evt;

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_GATTC_DISCOVER_COMPLETED, sizeof(*evt));
        evt->conn_idx = TASK_2_CONNIDX(gtl->src_id);
        evt->status = gevt->status == ATT_ERR_NO_ERROR
                        || gevt->status == ATT_ERR_ATTRIBUTE_NOT_FOUND ? BLE_STATUS_OK : BLE_ERROR_FAILED;

        switch (gevt->operation) {
        case GATTC_DISC_ALL_SVC:
        case GATTC_DISC_BY_UUID_SVC:
                evt->type = GATTC_DISCOVERY_TYPE_SVC;
                break;
        case GATTC_DISC_INCLUDED_SVC:
                evt->type = GATTC_DISCOVERY_TYPE_INCLUDED;
                break;
        case GATTC_DISC_ALL_CHAR:
        case GATTC_DISC_BY_UUID_CHAR:
                evt->type = GATTC_DISCOVERY_TYPE_CHARACTERISTICS;
                break;
        case GATTC_DISC_DESC_CHAR:
                evt->type = GATTC_DISCOVERY_TYPE_DESCRIPTORS;
                break;
        }

        /* Send to event queue */
        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
}

void ble_mgr_gattc_read_ind_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gattc_read_ind *gevt = (void *) gtl->param;
        ble_evt_gattc_read_completed_t *evt;

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_GATTC_READ_COMPLETED, sizeof(*evt) + gevt->length);
        evt->conn_idx = TASK_2_CONNIDX(gtl->src_id);
        evt->handle = gevt->handle;
        evt->status = ATT_ERROR_OK;
        evt->offset = gevt->offset;
        evt->length = gevt->length;
        memcpy(evt->value, gevt->value, gevt->length);

        /* Send to event queue */
        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
}

void ble_mgr_gattc_cmp__read_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gattc_cmp_evt *gevt = (void *) gtl->param;
        ble_evt_gattc_read_completed_t *evt;

        if (gevt->status == ATT_ERR_NO_ERROR) {
                /* Nothing to do, we replied in GATTC_READ_IND */
                return;
        }

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_GATTC_READ_COMPLETED, sizeof(*evt));
        evt->conn_idx = TASK_2_CONNIDX(gtl->src_id);
        evt->handle = gevt->seq_num;
        evt->status = gevt->status; // ATT error

        /* Send to event queue */
        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
}

void ble_mgr_gattc_cmp__write_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gattc_cmp_evt *gevt = (void *) gtl->param;
        ble_evt_gattc_write_completed_t *evt;

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_GATTC_WRITE_COMPLETED, sizeof(*evt));
        evt->conn_idx = TASK_2_CONNIDX(gtl->src_id);
        evt->handle = gevt->seq_num;
        evt->status = gevt->status; // ATT error

        /* Send to event queue */
        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
}

void ble_mgr_gattc_event_ind_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gattc_event_ind *gevt = (void *) gtl->param;
        ble_evt_gattc_notification_t *evt;

        if (gevt->type != GATTC_NOTIFY) {
                return;
        }

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_GATTC_NOTIFICATION, sizeof(*evt) + gevt->length);
        evt->conn_idx = TASK_2_CONNIDX(gtl->src_id);
        evt->handle = gevt->handle;
        evt->length = gevt->length;
        memcpy(evt->value, gevt->value, gevt->length);

        /* Send to event queue */
        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
}

void ble_mgr_gattc_event_req_ind_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gattc_event_ind *gevt = (void *) gtl->param;
        ble_evt_gattc_indication_t *evt;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gattc_event_cfm *gcmd;

        if (gevt->type != GATTC_INDICATE) {
                return;
        }

        /* Setup GTL message */
        gmsg = ble_gtl_alloc_with_conn(GATTC_EVENT_CFM, TASK_ID_GATTC, TASK_2_CONNIDX(gtl->src_id),
                                                                                sizeof(*gcmd));
        gcmd = (typeof(gcmd)) gmsg->msg.gtl.param;
        gcmd->handle = gevt->handle;

        ble_gtl_send(gmsg);

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_GATTC_INDICATION, sizeof(*evt) + gevt->length);
        evt->conn_idx = TASK_2_CONNIDX(gtl->src_id);
        evt->handle = gevt->handle;
        evt->length = gevt->length;
        memcpy(evt->value, gevt->value, gevt->length);

        /* Send to event queue */
        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
}

void ble_mgr_gattc_svc_changed_cfg_ind_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gattc_svc_changed_cfg *gevt = (void *) gtl->param;

        /*
         * Put this to persistent storage - it will be retrieved upon reconnection.
         */
        ble_storage_put_u32(TASK_2_CONNIDX(gtl->src_id), STORAGE_KEY_SVC_CHANGED_CCC,
                                                                        gevt->ind_cfg, true);
}
