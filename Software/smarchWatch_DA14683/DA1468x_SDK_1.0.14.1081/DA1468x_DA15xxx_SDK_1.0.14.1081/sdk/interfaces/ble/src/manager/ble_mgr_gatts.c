/**
 ****************************************************************************************
 *
 * @file ble_mgr_gatts.c
 *
 * @brief BLE manager handlers for GATTS API
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <string.h>
#include "co_version.h"
#include "ble_mgr.h"
#include "ble_mgr_gtl.h"
#include "ble_mgr_cmd.h"
#include "ble_mgr_common.h"
#include "ble_mgr_gatts.h"
#include "ble_mgr_helper.h"
#include "ble_common.h"
#include "ble_gatts.h"
#include "ble_uuid.h"
#include "storage.h"

#include "ke_msg.h"
#include "ke_task.h"
#include "gattm_task.h"
#include "gattc_task.h"
#include "rwip_config.h"

static void copy_uuid(const att_uuid_t *uuid1, uint8_t uuid2[16])
{
        switch (uuid1->type) {
        case ATT_UUID_16:
                uuid2[0] = uuid1->uuid16;
                uuid2[1] = uuid1->uuid16 >> 8;
                break;
        case ATT_UUID_128:
                memcpy(uuid2, uuid1->uuid128, sizeof(uuid1->uuid128));
                break;
        default:
                OS_ASSERT(0);
        }
}

/** \brief Map Dialog API properties to GTL API permissions */
static att_perm_type prop_to_rwperm(uint16_t prop)
{
        att_perm_type rwperm = 0;

        if (prop & GATT_PROP_BROADCAST) {
                rwperm |= (1 << PERM_POS_BROADCAST);
        }

// NOTE: READ property is set when proper READ permission is set in perm_to_perm
//        if (prop & GATTS_PROP_READ) {
//                perm_out |= (1 << PERM_POS_RD);
//        }

        if (prop & GATT_PROP_WRITE_NO_RESP) {
                rwperm |= (1 << PERM_POS_WRITE_COMMAND);
        }

        if (prop & GATT_PROP_WRITE) {
                rwperm |= (1 << PERM_POS_WRITE_REQ);
        }

        if (prop & GATT_PROP_NOTIFY) {
                rwperm |= (1 << PERM_POS_NTF);
        }

        if (prop & GATT_PROP_INDICATE) {
                rwperm |= (1 << PERM_POS_IND);
        }

        if (prop & GATT_PROP_WRITE_SIGNED) {
                rwperm |= (1 << PERM_POS_WRITE_SIGNED);
        }

        if (prop & GATT_PROP_EXTENDED) {
                rwperm |= (1 << PERM_POS_EXT);
        }

        return rwperm;
}

/** \brief Map Dialog API permissions to GTL API permissions */
static att_perm_type perm_to_rwperm(uint16_t perm)
{
        att_perm_type rwperm = 0;

        /* Translate read permissions */
        if (perm & ATT_PERM_READ_AUTH) {
                rwperm |= (PERM_RIGHT_AUTH << PERM_POS_RD);
        } else if (perm & ATT_PERM_READ_ENCRYPT) {
                rwperm |= (PERM_RIGHT_UNAUTH << PERM_POS_RD);
        } else if (perm & ATT_PERM_READ) {
                rwperm |= (PERM_RIGHT_ENABLE << PERM_POS_RD);
        }

        /* Translate write permissions */
        if (perm & ATT_PERM_WRITE_AUTH) {
                rwperm |= (PERM_RIGHT_AUTH << PERM_POS_WR);
        } else if (perm & ATT_PERM_WRITE_ENCRYPT) {
                rwperm |= (PERM_RIGHT_UNAUTH << PERM_POS_WR);
        } else if (perm & ATT_PERM_WRITE) {
                rwperm |= (PERM_RIGHT_ENABLE << PERM_POS_WR);
        }

        /* Translate keysize permissions */
        if (perm & ATT_PERM_KEYSIZE_16) {
                rwperm |= (PERM_RIGHT_ENABLE << PERM_POS_EKS);
        }

        return rwperm;
}

/** \brief Map GTL API permissions to Dialog API properties */
static uint16_t rwperm_to_prop(att_perm_type rwperm)
{
        uint16_t prop = 0;

        if (rwperm & PERM_MASK_BROADCAST) {
                prop |= GATT_PROP_BROADCAST;
        }

        if (rwperm & PERM_MASK_RD) {
                prop |= GATT_PROP_READ;
        }

        if (rwperm & PERM_MASK_WRITE_COMMAND) {
                prop |= GATT_PROP_WRITE_NO_RESP;
        }

        if (rwperm & PERM_MASK_WRITE_REQ) {
                prop |= GATT_PROP_WRITE;
        }

        if (rwperm & PERM_MASK_NTF) {
                prop |= GATT_PROP_NOTIFY;
        }

        if (rwperm & PERM_MASK_IND) {
                prop |= GATT_PROP_INDICATE;
        }

        if (rwperm & PERM_MASK_WRITE_SIGNED) {
                prop |= GATT_PROP_WRITE_SIGNED;
        }

        if (rwperm & PERM_MASK_EXT) {
                prop |= GATT_PROP_EXTENDED;
        }

        return prop;
}

/** \brief Map GTL API permissions to Dialog API permissions */
static uint16_t rwperm_to_perm(att_perm_type rwperm)
{
        uint16_t perm = 0;

        switch (rwperm & PERM_MASK_RD) {
        case (PERM_RIGHT_ENABLE << PERM_POS_RD):
                perm |= ATT_PERM_READ;
                break;
        case (PERM_RIGHT_UNAUTH << PERM_POS_RD):
                perm |= ATT_PERM_READ_ENCRYPT;
                break;
        case (PERM_RIGHT_AUTH << PERM_POS_RD):
                perm |= ATT_PERM_READ_AUTH;
                break;
        }

        switch (rwperm & PERM_MASK_WR) {
        case (PERM_RIGHT_ENABLE << PERM_POS_WR):
                perm |= ATT_PERM_WRITE;
                break;
        case (PERM_RIGHT_UNAUTH << PERM_POS_WR):
                perm |= ATT_PERM_WRITE_ENCRYPT;
                break;
        case (PERM_RIGHT_AUTH << PERM_POS_WR):
                perm |= ATT_PERM_WRITE_AUTH;
                break;
        }

        if (rwperm & PERM_MASK_EKS) {
                perm |= ATT_PERM_KEYSIZE_16;
        }

        return perm;
}

PRIVILEGED_DATA static ble_mgr_common_stack_msg_t *gattm_add_svc_msg = NULL;
PRIVILEGED_DATA static uint16_t attr_idx = 0;
PRIVILEGED_DATA static uint16_t extended_prop = 0;

void ble_mgr_gatts_service_add_cmd_handler(void *param)
{
        const ble_mgr_gatts_service_add_cmd_t *cmd = param;
        ble_mgr_gatts_service_add_rsp_t *rsp;
        struct gattm_add_svc_req *gcmd;
        ble_error_t ret = BLE_ERROR_FAILED;

        /* Check if there's pending GTL message set (should NOT be) */
        if (gattm_add_svc_msg) {
                goto done;
        }

        /* Allocate GTL message, this will be filled by other calls and sent when service is committed */
        gattm_add_svc_msg = ble_gtl_alloc(GATTM_ADD_SVC_REQ, TASK_ID_GATTM,
                sizeof(struct gattm_add_svc_req) + sizeof(struct gattm_att_desc) * cmd->num_attrs);
        attr_idx = 0;

        /* For now, fill only service information */
        gcmd = (struct gattm_add_svc_req *) gattm_add_svc_msg->msg.gtl.param;
        gcmd->svc_desc.start_hdl = 0; // assign automatically
        gcmd->svc_desc.task_id = TASK_ID_GTL;
        gcmd->svc_desc.perm = (1 << 2) |                                     // enabled
                        (cmd->uuid.type == ATT_UUID_128 ? (2 << 5) : 0) |    // uuid128 vs uuid16
                        (cmd->type == GATT_SERVICE_PRIMARY ? (1 << 7) : 0);  // primary vs secondary
        gcmd->svc_desc.nb_att = cmd->num_attrs;
        copy_uuid(&cmd->uuid, gcmd->svc_desc.uuid);

        ret = BLE_STATUS_OK;

done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GATTS_SERVICE_ADD_CMD, sizeof(*rsp));
        rsp->status = ret;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gatts_service_add_include_cmd_handler(void *param)
{
        static const att_uuid_t uuid_incl = { .type = ATT_UUID_16, .uuid16 = 0x2802 };
        const ble_mgr_gatts_service_add_include_cmd_t *cmd = param;
        ble_mgr_gatts_service_add_include_rsp_t *rsp;
        struct gattm_add_svc_req *gcmd;
        uint16_t h_offset = 0;
        ble_error_t ret = BLE_ERROR_FAILED;

        /* Check if there's pending GTL message set (should be) */
        if (!gattm_add_svc_msg) {
                goto done;
        }

        gcmd = (struct gattm_add_svc_req *) gattm_add_svc_msg->msg.gtl.param;

        /* Check if there are enough free attributes left */
        if (gcmd->svc_desc.nb_att - attr_idx < 1) {
                goto done;
        }

        copy_uuid(&uuid_incl, gcmd->svc_desc.atts[attr_idx].uuid);
        gcmd->svc_desc.atts[attr_idx].perm = 0; // don't care
        gcmd->svc_desc.atts[attr_idx].max_len = cmd->handle;
        h_offset = ++attr_idx;

        ret = BLE_STATUS_OK;

done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GATTS_SERVICE_INCLUDE_ADD_CMD, sizeof(*rsp));
        rsp->status = ret;
        rsp->h_offset = h_offset;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gatts_service_add_characteristic_cmd_handler(void *param)
{
        static const att_uuid_t uuid_char = { .type = ATT_UUID_16, .uuid16 = 0x2803 };
        const ble_mgr_gatts_service_add_characteristic_cmd_t *cmd = param;
        ble_mgr_gatts_service_add_characteristic_rsp_t *rsp;
        struct gattm_add_svc_req *gcmd;
        uint16_t max_len;
        uint16_t h_offset = 0;
        uint16_t h_val_offset = 0;
        ble_error_t ret = BLE_ERROR_FAILED;

        /* Check if there's pending GTL message set (should be) */
        if (!gattm_add_svc_msg) {
                goto done;
        }

        gcmd = (struct gattm_add_svc_req *) gattm_add_svc_msg->msg.gtl.param;

        /* Check if there are enough free attributes left */
        if (gcmd->svc_desc.nb_att - attr_idx < 2) {
                goto done;
        }

        max_len = cmd->max_len & 0x7FFF;
        if (cmd->flags & GATTS_FLAG_CHAR_READ_REQ) {
                max_len |= 0x8000;
        }

        /* Store value of extended properties descriptor */
        extended_prop = cmd->prop & (GATT_PROP_EXTENDED_RELIABLE_WRITE |
                                                        GATT_PROP_EXTENDED_WRITABLE_AUXILIARIES);

        /* Characteristic attribute */
        copy_uuid(&uuid_char, gcmd->svc_desc.atts[attr_idx].uuid);
        gcmd->svc_desc.atts[attr_idx].perm = 0; // don't care
        gcmd->svc_desc.atts[attr_idx].max_len = 0;
        h_offset = ++attr_idx;

        /* Characteristic value attribute */
        copy_uuid(&cmd->uuid, gcmd->svc_desc.atts[attr_idx].uuid);
        /* For characteristics, BLE stack permissions are combination of properties and permissions */
        gcmd->svc_desc.atts[attr_idx].perm = prop_to_rwperm(cmd->prop) | perm_to_rwperm(cmd->perm) |
                                        ((cmd->uuid.type == ATT_UUID_128 ? 2 : 0) << PERM_POS_UUID_LEN);
        gcmd->svc_desc.atts[attr_idx].max_len = max_len;
        h_val_offset = ++attr_idx;

        ret = BLE_STATUS_OK;

done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GATTS_SERVICE_CHARACTERISTIC_ADD_CMD, sizeof(*rsp));
        rsp->status = ret;
        rsp->h_offset = h_offset;
        rsp->h_val_offset = h_val_offset;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gatts_service_add_descriptor_cmd_handler(void *param)
{
        const ble_mgr_gatts_service_add_descriptor_cmd_t *cmd = param;
        ble_mgr_gatts_service_add_descriptor_rsp_t *rsp;
        struct gattm_add_svc_req *gcmd;
        uint16_t max_len;
        uint16_t h_offset = 0;
        ble_error_t ret = BLE_ERROR_FAILED;
        att_uuid_t uuid;

        /* Check if there's pending GTL message set (should be) */
        if (!gattm_add_svc_msg) {
                goto done;
        }

        gcmd = (struct gattm_add_svc_req *) gattm_add_svc_msg->msg.gtl.param;

        /* Check if there are enough free attributes left */
        if (gcmd->svc_desc.nb_att - attr_idx < 1) {
                goto done;
        }

        max_len = cmd->max_len & 0x7FFF;
        if (cmd->flags & GATTS_FLAG_CHAR_READ_REQ) {
                max_len |= 0x8000;
        }

        /* Check if it is Extended properties descriptor and set it's value */
        ble_uuid_create16(UUID_GATT_CHAR_EXT_PROPERTIES, &uuid);
        if (ble_uuid_equal(&uuid, &cmd->uuid)) {
                max_len = 0;

                if (extended_prop & GATT_PROP_EXTENDED_RELIABLE_WRITE) {
                        max_len |= 0x0001;
                }

                if (extended_prop & GATT_PROP_EXTENDED_WRITABLE_AUXILIARIES) {
                        max_len |= 0x0002;
                }
        }

        copy_uuid(&cmd->uuid, gcmd->svc_desc.atts[attr_idx].uuid);
        gcmd->svc_desc.atts[attr_idx].perm = perm_to_rwperm(cmd->perm) |
                                        ((cmd->uuid.type == ATT_UUID_128 ? 2 : 0) << PERM_POS_UUID_LEN);

        /* Support write requests if any write permissions declared */
        if (cmd->perm & (ATT_PERM_WRITE_ENCRYPT | ATT_PERM_WRITE_AUTH | ATT_PERM_WRITE)) {
                gcmd->svc_desc.atts[attr_idx].perm |= prop_to_rwperm(GATT_PROP_WRITE |
                                                                        GATT_PROP_WRITE_NO_RESP);
        }

        gcmd->svc_desc.atts[attr_idx].max_len = max_len;
        h_offset = ++attr_idx;

        ret = BLE_STATUS_OK;

done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GATTS_SERVICE_DESCRIPTOR_ADD_CMD, sizeof(*rsp));
        rsp->status = ret;
        rsp->h_offset = h_offset;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

static void gatts_service_register_rsp(ble_gtl_msg_t *gtl, void *param)
{
        struct gattm_add_svc_rsp *grsp = (void *) gtl->param;
        ble_mgr_gatts_service_register_rsp_t *rsp = param;

        rsp->handle = grsp->start_hdl;
        rsp->status = (grsp->status == 0 ? BLE_STATUS_OK : BLE_ERROR_FAILED);

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gatts_service_register_cmd_handler(void *param)
{
        ble_mgr_gatts_service_register_rsp_t *rsp;

        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GATTS_SERVICE_REGISTER_CMD, sizeof(*rsp));

        if (gattm_add_svc_msg) {
                ble_gtl_waitqueue_add(BLE_CONN_IDX_INVALID, GATTM_ADD_SVC_RSP, 0,
                                                        gatts_service_register_rsp, (void *) rsp);
                ble_gtl_send(gattm_add_svc_msg);

                gattm_add_svc_msg = NULL;
        } else {
                rsp->status = BLE_ERROR_FAILED;
                ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
        }
}

static void service_enable_reply(void *param, ble_status_t status)
{
        ble_mgr_msg_hdr_t *hdr = param;

        if (hdr->op_code == BLE_MGR_GATTS_SERVICE_ENABLE_CMD) {
                ble_mgr_gatts_service_enable_rsp_t *rsp;

                rsp = ble_msg_init(BLE_MGR_GATTS_SERVICE_ENABLE_CMD, sizeof(*rsp));
                rsp->status = status;

                ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
        } else {
                ble_mgr_gatts_service_disable_rsp_t *rsp;

                rsp = ble_msg_init(BLE_MGR_GATTS_SERVICE_DISABLE_CMD, sizeof(*rsp));
                rsp->status = status;

                ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
        }

        ble_msg_free(param);
}

static void gatts_svc_set_permission_rsp(ble_gtl_msg_t *gtl, void *param)
{
        struct gattm_svc_set_permission_rsp *grsp = (void *) gtl->param;

        service_enable_reply(param, grsp->status == 0 ? BLE_STATUS_OK : BLE_ERROR_FAILED);
}

static void gatts_svc_get_permission_rsp(ble_gtl_msg_t *gtl, void *param)
{
        ble_mgr_msg_hdr_t *hdr = param;
        struct gattm_svc_get_permission_rsp *grsp = (void *) gtl->param;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gattm_svc_set_permission_req *gcmd;

        if (grsp->status != 0) {
                service_enable_reply(param, BLE_ERROR_FAILED);
                return;
        }

        /* Setup GTL message */
        gmsg = ble_gtl_alloc(GATTM_SVC_SET_PERMISSION_REQ, TASK_ID_GATTM,
                                                        sizeof(struct gattm_svc_set_permission_req));
        gcmd = (struct gattm_svc_set_permission_req *) gmsg->msg.gtl.param;
        gcmd->start_hdl = grsp->start_hdl;

        if (hdr->op_code == BLE_MGR_GATTS_SERVICE_ENABLE_CMD) {
                /* Clear current auth setting and set it to 'enabled' */
                gcmd->perm = grsp->perm & ~PERM_MASK_SVC_AUTH;
                gcmd->perm |= (1 << PERM_POS_SVC_AUTH);
        } else {
                /* Clear current auth setting and leave it on 'disabled' */
                gcmd->perm = grsp->perm & ~PERM_MASK_SVC_AUTH;
        }

        ble_gtl_waitqueue_add(BLE_CONN_IDX_INVALID, GATTM_SVC_SET_PERMISSION_RSP, 0,
                                                        gatts_svc_set_permission_rsp, param);
        ble_gtl_send(gmsg);
}

void ble_mgr_gatts_service_enable_cmd_handler(void *param)
{
        const ble_mgr_gatts_service_enable_cmd_t *cmd = param;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gattm_svc_get_permission_req *gcmd;

        /* Setup GTL message */
        gmsg = ble_gtl_alloc(GATTM_SVC_GET_PERMISSION_REQ, TASK_ID_GATTM,
                                                        sizeof(struct gattm_svc_get_permission_req));
        gcmd = (struct gattm_svc_get_permission_req *) gmsg->msg.gtl.param;
        gcmd->start_hdl = cmd->handle;

        /* Don't free command buffer now - it will be used when creating reply */

        ble_gtl_waitqueue_add(BLE_CONN_IDX_INVALID, GATTM_SVC_GET_PERMISSION_RSP, 0,
                              gatts_svc_get_permission_rsp, (void *) cmd);
        ble_gtl_send(gmsg);
}

void ble_mgr_gatts_service_disable_cmd_handler(void *param)
{
        const ble_mgr_gatts_service_disable_cmd_t *cmd = param;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gattm_svc_get_permission_req *gcmd;

        /* Setup GTL message */
        gmsg = ble_gtl_alloc(GATTM_SVC_GET_PERMISSION_REQ, TASK_ID_GATTM,
                                                        sizeof(struct gattm_svc_get_permission_req));
        gcmd = (struct gattm_svc_get_permission_req *) gmsg->msg.gtl.param;
        gcmd->start_hdl = cmd->handle;

        /* Don't free command buffer now - it will be used when creating reply */

        ble_gtl_waitqueue_add(BLE_CONN_IDX_INVALID, GATTM_SVC_GET_PERMISSION_RSP, 0,
                              gatts_svc_get_permission_rsp, (void *) cmd);
        ble_gtl_send(gmsg);
}

static void gatts_att_get_permission_rsp(ble_gtl_msg_t *gtl, void *param)
{
        struct gattm_att_get_permission_rsp *grsp = (void *) gtl->param;
        ble_mgr_gatts_service_characteristic_get_prop_rsp_t *rsp = param;

        rsp->status = (grsp->status == 0 ? BLE_STATUS_OK : BLE_ERROR_FAILED);

        if (rsp->status == BLE_STATUS_OK) {
                rsp->perm = rwperm_to_perm(grsp->perm);
                rsp->prop = rwperm_to_prop(grsp->perm);
        }

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gatts_service_characteristic_get_prop_cmd_handler(void *param)
{
        const ble_mgr_gatts_service_characteristic_get_prop_cmd_t *cmd = param;
        ble_mgr_gatts_service_characteristic_get_prop_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gattm_att_get_permission_req *gcmd;

        /* Setup GTL message */
        gmsg = ble_gtl_alloc(GATTM_ATT_GET_PERMISSION_REQ, TASK_ID_GATTM,
                                                        sizeof(struct gattm_att_get_permission_req));
        gcmd = (struct gattm_att_get_permission_req *) gmsg->msg.gtl.param;

        /*
         * API takes characteristic handle, but for the stack we need to read permissions of
         * attribute value handle.
         */
        gcmd->handle = cmd->handle + 1;

        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GATTS_SERVICE_CHARACTERISTIC_GET_PROP_CMD, sizeof(*rsp));

        ble_gtl_waitqueue_add(BLE_CONN_IDX_INVALID, GATTM_ATT_GET_PERMISSION_RSP, 0,
                              gatts_att_get_permission_rsp, (void *) rsp);
        ble_gtl_send(gmsg);
}

static void gatts_att_set_permission_rsp(ble_gtl_msg_t *gtl, void *param)
{
        struct gattm_att_set_permission_rsp *grsp = (void *) gtl->param;
        ble_mgr_gatts_service_characteristic_set_prop_rsp_t *rsp = param;

        rsp->status = (grsp->status == 0 ? BLE_STATUS_OK : BLE_ERROR_FAILED);

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gatts_service_characteristic_set_prop_cmd_handler(void *param)
{
        const ble_mgr_gatts_service_characteristic_set_prop_cmd_t *cmd = param;
        ble_mgr_gatts_service_characteristic_set_prop_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gattm_att_set_permission_req *gcmd;

        /* Setup GTL message */
        gmsg = ble_gtl_alloc(GATTM_ATT_SET_PERMISSION_REQ, TASK_ID_GATTM,
                                                        sizeof(struct gattm_att_set_permission_req));
        gcmd = (struct gattm_att_set_permission_req *) gmsg->msg.gtl.param;

        /*
         * API takes characteristic handle, but for the stack we need to read permissions of
         * attribute value handle.
         */
        gcmd->handle = cmd->handle + 1;
        gcmd->perm = prop_to_rwperm(cmd->prop) | perm_to_rwperm(cmd->perm);

        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GATTS_SERVICE_CHARACTERISTIC_SET_PROP_CMD, sizeof(*rsp));

        ble_gtl_waitqueue_add(BLE_CONN_IDX_INVALID, GATTM_ATT_SET_PERMISSION_RSP, 0,
                              gatts_att_set_permission_rsp, (void *) rsp);
        ble_gtl_send(gmsg);
}

static void gatts_get_value_rsp(ble_gtl_msg_t *gtl, void *param)
{
        struct gattm_att_get_value_rsp *grsp = (void *) gtl->param;
        ble_mgr_gatts_get_value_cmd_t *cmd = param;
        ble_mgr_gatts_get_value_rsp_t *rsp;
        uint16_t length;

        length = MIN(cmd->max_len, grsp->length);

        /* Free command buffer */
        ble_msg_free(cmd);

        /* Create response */
        rsp = ble_msg_init(BLE_MGR_GATTS_GET_VALUE_CMD, sizeof(*rsp) + length);

        if (grsp->status == ATT_ERR_NO_ERROR) {
                rsp->length = grsp->length;
                memcpy(rsp->value, grsp->value, length);
        }

        rsp->status = (grsp->status == 0 ? BLE_STATUS_OK : BLE_ERROR_FAILED);

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gatts_get_value_cmd_handler(void *param)
{
        const ble_mgr_gatts_get_value_cmd_t *cmd = param;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gattm_att_get_value_req *gcmd;

        /* Setup GTL message */
        gmsg = ble_gtl_alloc(GATTM_ATT_GET_VALUE_REQ, TASK_ID_GATTM, sizeof(struct gattm_att_get_value_req));
        gcmd = (struct gattm_att_get_value_req *) gmsg->msg.gtl.param;
        gcmd->handle = cmd->handle;

        /* 
         * Keep param buffer, we'll need it when creating response.
         * The response message will be allocated in gatts_get_value_rsp since its length depends on
         * GTL response.
         */

        ble_gtl_waitqueue_add(BLE_CONN_IDX_INVALID, GATTM_ATT_GET_VALUE_RSP, 0,
                                                        gatts_get_value_rsp, (void *) cmd);
        ble_gtl_send(gmsg);
}

static void gatts_set_value_rsp(ble_gtl_msg_t *gtl, void *param)
{
        struct gattm_att_set_value_rsp *grsp = (void *) gtl->param;
        ble_mgr_gatts_set_value_rsp_t *rsp = param;

        rsp->status = (grsp->status == 0 ? BLE_STATUS_OK : BLE_ERROR_FAILED);

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gatts_set_value_cmd_handler(void *param)
{
        const ble_mgr_gatts_set_value_cmd_t *cmd = param;
        ble_mgr_gatts_set_value_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gattm_att_set_value_req *gcmd;

        /* Setup GTL message */
        gmsg = ble_gtl_alloc(GATTM_ATT_SET_VALUE_REQ, TASK_ID_GATTM, sizeof(struct gattm_att_set_value_req) + cmd->length);
        gcmd = (struct gattm_att_set_value_req *) gmsg->msg.gtl.param;
        gcmd->handle = cmd->handle;
        gcmd->length = cmd->length;
        memcpy(gcmd->value, cmd->value, cmd->length);

        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GATTS_SET_VALUE_CMD, sizeof(*rsp));

        ble_gtl_waitqueue_add(BLE_CONN_IDX_INVALID, GATTM_ATT_SET_VALUE_RSP, 0,
                                                                gatts_set_value_rsp, (void *) rsp);
        ble_gtl_send(gmsg);
}

void ble_mgr_gatts_read_cfm_cmd_handler(void *param)
{
        const ble_mgr_gatts_read_cfm_cmd_t *cmd = param;
        ble_mgr_gatts_read_cfm_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gattc_read_cfm *gcmd;
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
        gmsg = ble_gtl_alloc_with_conn(GATTC_READ_CFM, TASK_ID_GATTC, cmd->conn_idx, sizeof(*gcmd) + cmd->length);
        gcmd = (struct gattc_read_cfm *) gmsg->msg.gtl.param;
        gcmd->handle = cmd->handle;
        gcmd->length = cmd->length;
        gcmd->status = cmd->status;
        if (cmd->value) {
                memcpy(gcmd->value, cmd->value, cmd->length);
        }

        ble_gtl_send(gmsg);

        ret = BLE_STATUS_OK;
        /* GATTC_READ_CFM does not return anything so just free command buffer and create response */
done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GATTS_READ_CFM_CMD, sizeof(*rsp));
        rsp->status = ret;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gatts_write_cfm_cmd_handler(void *param)
{
        const ble_mgr_gatts_write_cfm_cmd_t *cmd = param;
        ble_mgr_gatts_write_cfm_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gattc_write_cfm *gcmd;
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
        gmsg = ble_gtl_alloc_with_conn(GATTC_WRITE_CFM, TASK_ID_GATTC, cmd->conn_idx, sizeof(*gcmd));
        gcmd = (struct gattc_write_cfm *) gmsg->msg.gtl.param;
        gcmd->handle = cmd->handle;
        gcmd->status = cmd->status;

        ble_gtl_send(gmsg);

        ret = BLE_STATUS_OK;
        /* GATTC_READ_CFM does not return anything so just free command buffer and create response */
done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GATTS_WRITE_CFM_CMD, sizeof(*rsp));
        rsp->status = ret;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gatts_prepare_write_cfm_cmd_handler(void *param)
{
        const ble_mgr_gatts_prepare_write_cfm_cmd_t *cmd = param;
        ble_mgr_gatts_prepare_write_cfm_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gattc_att_info_cfm *gcmd;
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
        gmsg = ble_gtl_alloc_with_conn(GATTC_ATT_INFO_CFM, TASK_ID_GATTC, cmd->conn_idx, sizeof(*gcmd));
        gcmd = (struct gattc_att_info_cfm *) gmsg->msg.gtl.param;
        gcmd->handle = cmd->handle;
        gcmd->length = cmd->length;
        gcmd->status = cmd->status;

        ble_gtl_send(gmsg);

        ret = BLE_STATUS_OK;
        /* GATTC_ATT_INFO_CFM does not return anything so just free command buffer and create response */
done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GATTS_PREPARE_WRITE_CFM_CMD, sizeof(*rsp));
        rsp->status = ret;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gatts_send_event_cmd_handler(void *param)
{
        const ble_mgr_gatts_send_event_cmd_t *cmd = param;
        ble_mgr_gatts_send_event_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gattc_send_evt_cmd *gcmd;
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

        /* Check if GATT controller is busy */
        if ( ke_state_get(KE_BUILD_ID(TASK_GATTC, cmd->conn_idx)) & GATTC_SERVER_BUSY ) {
                ret = BLE_ERROR_BUSY;
                goto done;
        }

        /* Setup GTL message */
        gmsg = ble_gtl_alloc_with_conn(GATTC_SEND_EVT_CMD, TASK_ID_GATTC, cmd->conn_idx, sizeof(*gcmd) + cmd->length);
        gcmd = (struct gattc_send_evt_cmd *) gmsg->msg.gtl.param;
        gcmd->handle = cmd->handle;
        gcmd->length = cmd->length;
        gcmd->operation = cmd->type == GATT_EVENT_NOTIFICATION ? GATTC_NOTIFY : GATTC_INDICATE;
        /* We use sequence number to store info about handle. (Handle is not present in gattc_cmp_evt) */
        gcmd->seq_num = cmd->handle;
        memcpy(gcmd->value, cmd->value, cmd->length);

        ble_gtl_send(gmsg);

        ret = BLE_STATUS_OK;
        /* Do not wait for GATTC_CMP_EVT, it will be handled async to avoid infinite wait */
done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GATTS_SEND_EVENT_CMD, sizeof(*rsp));
        rsp->status = ret;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gatts_service_changed_ind_cmd_handler(void *param)
{
        const ble_mgr_gatts_service_changed_ind_cmd_t *cmd = param;
        ble_mgr_gatts_service_changed_ind_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gattc_send_svc_changed_cmd *gcmd;
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
        gmsg = ble_gtl_alloc_with_conn(GATTC_SEND_SVC_CHANGED_CMD, TASK_ID_GATTC, cmd->conn_idx, sizeof(*gcmd));
        gcmd = (struct gattc_send_svc_changed_cmd *) gmsg->msg.gtl.param;
        gcmd->operation = GATTC_SVC_CHANGED;
        gcmd->seq_num = 0; // not used
        gcmd->svc_shdl = cmd->start_handle;
        gcmd->svc_ehdl = cmd->end_handle;

        ble_gtl_send(gmsg);
        ret = BLE_STATUS_OK;

done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GATTS_SERVICE_CHANGED_IND_CMD, sizeof(*rsp));
        rsp->status = ret;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gatts_read_value_req_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gattc_read_req_ind *gevt = (void *) gtl->param;
        ble_evt_gatts_read_req_t *evt;

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_GATTS_READ_REQ, sizeof(*evt));
        evt->conn_idx = TASK_2_CONNIDX(gtl->src_id);
        evt->handle = gevt->handle;
        evt->offset = 0;    // stack always requires full value

        /* Send to event queue */
        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
}

void ble_mgr_gatts_write_value_req_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gattc_write_req_ind *gevt = (void *) gtl->param;
        ble_evt_gatts_write_req_t *evt;

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_GATTS_WRITE_REQ, sizeof(*evt) + gevt->length);
        evt->conn_idx = TASK_2_CONNIDX(gtl->src_id);
        evt->handle = gevt->handle;
        evt->offset = gevt->offset;
        evt->length = gevt->length;
        memcpy(evt->value, gevt->value, gevt->length);

        /* Send to event queue */
        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
}

void ble_mgr_gatts_prepare_write_req_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gattc_att_info_req_ind *gevt = (void *) gtl->param;
        ble_evt_gatts_prepare_write_req_t *evt;

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_GATTS_PREPARE_WRITE_REQ, sizeof(*evt));
        evt->conn_idx = TASK_2_CONNIDX(gtl->src_id);
        evt->handle = gevt->handle;

        /* Send to event queue */
        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
}

void ble_mgr_gatts_event_sent_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gattc_cmp_evt *gevt = (void *) gtl->param;
        ble_evt_gatts_event_sent_t *evt;

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_GATTS_EVENT_SENT, sizeof(*evt));
        evt->conn_idx = TASK_2_CONNIDX(gtl->src_id);
        /* We used sequence number in ble_gatts_send_event_cmd_handler() to store info about handle */
        evt->handle = gevt->seq_num;
        evt->type = gevt->operation == GATTC_NOTIFY ? GATT_EVENT_NOTIFICATION : GATT_EVENT_INDICATION;
        evt->status = gevt->status == 0;

        /* Send to event queue */
        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
}
