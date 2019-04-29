/**
 ****************************************************************************************
 *
 * @file ble_mgr_common.c
 *
 * @brief BLE manager handlers for common API
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <string.h>
#include "ad_ble_msg.h"
#include "ble_mgr.h"
#include "ble_mgr_ad_msg.h"
#include "ble_mgr_gtl.h"
#include "ble_mgr_cmd.h"
#include "ble_mgr_common.h"
#include "ble_mgr_helper.h"
#include "ble_common.h"
#include "storage.h"

#include "gapm_task.h"

void ble_mgr_common_stack_msg_handler(void *param)
{
        /* Send message directly to BLE adapter */
        ad_ble_command_queue_send(&param, OS_QUEUE_FOREVER);
}

void ble_mgr_common_register_cmd_handler(void *param)
{
        const ble_mgr_common_register_cmd_t *cmd = param;
        ble_mgr_common_register_rsp_t *rsp;

        ble_mgr_register_application(cmd->task);

        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_COMMON_REGISTER_CMD, sizeof(*rsp));

        rsp->status = BLE_STATUS_OK;

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_adapter_cmp_evt_init(ad_ble_msg_t *ad_msg, void *param)
{
        /* Event received from BLE adapter -- NOT GTL */
        ad_ble_cmp_evt_t *ad_evt = (void *) ad_msg;
        ble_dev_params_t *dev_params;
        ble_mgr_common_enable_rsp_t *rsp;
        ble_dev_params_t *devp;

        OS_ASSERT(ad_evt->status == AD_BLE_STATUS_NO_ERROR);

        dev_params = ble_mgr_dev_params_acquire();
        dev_params->status = BLE_IS_ENABLED;
        ble_mgr_dev_params_release();

        ble_msg_free(param);

        /* Create response */
        rsp = ble_msg_init(BLE_MGR_COMMON_ENABLE_CMD, sizeof(*rsp));
        rsp->status = (ad_evt->status == 0 ? BLE_STATUS_OK : BLE_ERROR_FAILED);

        /*
         * We now know that BLE adapter is up and running which means it already have proper address
         * set. Now it's a good time to update ble_dev_params - we always start with public static
         * address and application can change this after BLE is enabled.
         */
        devp = ble_mgr_dev_params_acquire();
        devp->own_addr.addr_type = PUBLIC_STATIC_ADDRESS;
        /* Update own public BD address with the one stored in NVPARAM */
        ad_ble_get_public_address(devp->own_addr.addr);
        /* Update own public IRK with the one stored in NVPARAM */
        ad_ble_get_irk(devp->irk.key);
        ble_mgr_dev_params_release();

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_common_enable_cmd_handler(void *param)
{
        ad_ble_msg_t *ad_cmd;

        storage_init();

        /* Allocate buffer for BLE adapter message */
        ad_cmd = ble_ad_msg_alloc(AD_BLE_OP_INIT_CMD, sizeof(ad_ble_msg_t));

        /* Add expected response on the waitqueue -- NOT GTL */
        ble_ad_msg_wqueue_add(AD_BLE_OP_CMP_EVT, 0, ble_adapter_cmp_evt_init, param);

        /* Send BLE adapter message -- NOT GTL */
        ble_ad_msg_send(ad_cmd);
}

static void gapm_reset_complete(ble_gtl_msg_t *gtl, void *param)
{
        struct gapm_cmp_evt *gevt = (void *) gtl->param;
        ble_dev_params_t *dev_params;
        ble_mgr_common_reset_rsp_t *rsp;

        /* Free command buffer */
        ble_msg_free(param);

        /* Create response */
        rsp = ble_msg_init(BLE_MGR_COMMON_RESET_CMD, sizeof(*rsp));

        if (gevt->status != GAP_ERR_NO_ERROR) {
                rsp->status = BLE_ERROR_FAILED;
                goto done;
        }

        rsp->status = BLE_STATUS_OK;

        /* Cleanup and initialize storage */
        storage_acquire();
        storage_cleanup();
        storage_init();
        storage_release();

        /* Set default device parameters */
        dev_params = ble_mgr_dev_params_acquire();
        ble_mgr_dev_params_set_default();
        /* Update own public BD address with the one stored in NVPARAM */
        ad_ble_get_public_address(dev_params->own_addr.addr);
        /* Update own IRK with the one stored in NVPARAM */
        ad_ble_get_irk(dev_params->irk.key);
        dev_params->status = BLE_IS_ENABLED;
        ble_mgr_dev_params_release();

done:
        ble_mgr_set_reset(false);
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_common_reset_cmd_handler(void *param)
{
        static ble_mgr_common_stack_msg_t *gmsg;
        struct gapm_reset_cmd *gcmd;
        void *pending;

        /* Remove elements from event queue */
        while (ble_mgr_event_queue_get(&pending, OS_QUEUE_NO_WAIT) == OS_QUEUE_OK) {
                ble_msg_free(pending);
        }

        ble_mgr_set_reset(true);

        gmsg = ble_gtl_alloc(GAPM_RESET_CMD, TASK_ID_GAPM, sizeof(*gcmd));
        gcmd = (struct gapm_reset_cmd *) gmsg->msg.gtl.param;
        gcmd->operation = GAPM_RESET;

        ble_gtl_waitqueue_add(BLE_CONN_IDX_INVALID, GAPM_CMP_EVT, GAPM_RESET, gapm_reset_complete,
                                                                                        param);
        ble_gtl_send(gmsg);
}

void ble_mgr_common_read_tx_power_cmd_handler(void *param)
{
        ble_mgr_common_read_tx_power_rsp_t *rsp;

        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_COMMON_READ_TX_POWER_CMD, sizeof(*rsp));

        rsp->tx_power_level = 0x00;
        rsp->status = BLE_STATUS_OK;

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}
