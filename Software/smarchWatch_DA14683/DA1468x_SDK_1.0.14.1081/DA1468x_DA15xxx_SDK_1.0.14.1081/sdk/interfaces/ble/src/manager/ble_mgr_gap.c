/**
 ****************************************************************************************
 *
 * @file ble_mgr_gap.c
 *
 * @brief BLE manager handlers for GAP API
 *
 * Copyright (C) 2015-2018 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <string.h>
#include <stdlib.h>
#include "co_bt.h"
#include "co_version.h"
#include "rwble_hl_error.h"
#include "smp_common.h"
#include "ble_config.h"
#include "ble_mgr.h"
#include "ble_mgr_gtl.h"
#include "ble_mgr_cmd.h"
#include "ble_mgr_common.h"
#include "ble_mgr_gap.h"
#include "ble_mgr_l2cap.h"
#include "ble_mgr_helper.h"
#include "ble_common.h"
#include "ble_gap.h"
#include "ble_gatt.h"
#include "ble_storage.h"
#include "storage.h"

#include "gapc.h"
#include "gapc_task.h"
#include "gapm_task.h"

#if (dg_configBLE_PRIVACY_1_2 == 1)
static void ble_mgr_gap_ral_sync(ble_mgr_cmd_handler_t handler, void *param);
#endif /* (dg_configBLE_PRIVACY_1_2 == 1) */

/** \brief Map Dialog API GAP device name write permissions to GTL API write permissions */
static uint8_t devname_perm_to_perm(uint16_t perm_in)
{
        uint16_t perm_out = 0;

        /* Translate write permissions */
        if (perm_in & ATT_PERM_WRITE_AUTH) {
                perm_out = (PERM_RIGHT_AUTH << GAPM_POS_ATT_NAME_PERM);
        } else if (perm_in & ATT_PERM_WRITE_ENCRYPT) {
                perm_out = (PERM_RIGHT_UNAUTH << GAPM_POS_ATT_NAME_PERM);
        } else if (perm_in & ATT_PERM_WRITE) {
                perm_out = (PERM_RIGHT_ENABLE << GAPM_POS_ATT_NAME_PERM);
        }

        return perm_out;
}

/** \brief Map Dialog API GAP appearance write permissions to GTL API write permissions */
static uint8_t appearance_perm_to_perm(uint16_t perm_in)
{
        uint16_t perm_out = 0;

        /* Translate write permissions */
        if (perm_in & ATT_PERM_WRITE_AUTH) {
                perm_out = (PERM_RIGHT_AUTH << GAPM_POS_ATT_APPEARENCE_PERM);
        } else if (perm_in & ATT_PERM_WRITE_ENCRYPT) {
                perm_out = (PERM_RIGHT_UNAUTH << GAPM_POS_ATT_APPEARENCE_PERM);
        } else if (perm_in & ATT_PERM_WRITE) {
                perm_out = (PERM_RIGHT_ENABLE << GAPM_POS_ATT_APPEARENCE_PERM);
        }

        return perm_out;
}

/** \brief Send a GTL GAPM cancel command */
static void send_gapm_cancel_cmd(void)
{
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapm_cancel_cmd *gcmd;

        /* NOTE: This cancels *any* ongoing air operation */
        gmsg = ble_gtl_alloc(GAPM_CANCEL_CMD, TASK_ID_GAPM, sizeof(*gcmd));
        gcmd = (struct gapm_cancel_cmd *) gmsg->msg.gtl.param;
        gcmd->operation = GAPM_CANCEL;

        ble_gtl_send(gmsg);
}

/** \brief Map gap_auth bitmask to security level */
__STATIC_INLINE gap_sec_level_t auth_2_sec_level(uint8_t auth)
{
        return (auth & GAP_AUTH_MITM) ?
                ((auth & GAP_AUTH_SEC) ? GAP_SEC_LEVEL_4 : GAP_SEC_LEVEL_3) :
                GAP_SEC_LEVEL_2;
}

/** \brief Translate GTL API role to Dialog API role */
static gap_role_t dlg_role_from_gtl_role(uint8_t gtl_role)
{
        gap_role_t dlg_role = GAP_NO_ROLE;

#if (dg_configBLE_CENTRAL == 1)
        if (gtl_role & GAP_ROLE_CENTRAL) {
                dlg_role |= GAP_CENTRAL_ROLE;
        }
#endif /* (dg_configBLE_CENTRAL == 1) */
#if (dg_configBLE_PERIPHERAL == 1)
        if (gtl_role & GAP_ROLE_PERIPHERAL) {
                dlg_role |= GAP_PERIPHERAL_ROLE;
        }
#endif /* (dg_configBLE_PERIPHERAL == 1) */
#if (dg_configBLE_BROADCASTER == 1)
        if (gtl_role & GAP_ROLE_BROADCASTER) {
                dlg_role |= GAP_BROADCASTER_ROLE;
        }
#endif /* (dg_configBLE_BROADCASTER == 1) */
#if (dg_configBLE_OBSERVER == 1)
        if (gtl_role & GAP_ROLE_OBSERVER) {
                dlg_role |= GAP_OBSERVER_ROLE;
        }
#endif /* (dg_configBLE_OBSERVER == 1) */

        return dlg_role;
}

/** \brief Translate Dialog API role to GTL API role */
static uint8_t dlg_role_to_gtl_role(gap_role_t dlg_role)
{
        gap_role_t gtl_role = GAP_ROLE_NONE;

#if (dg_configBLE_CENTRAL == 1)
        if (dlg_role & GAP_CENTRAL_ROLE) {
                gtl_role |= GAP_ROLE_CENTRAL;
        }
#endif /* (dg_configBLE_CENTRAL == 1) */
#if (dg_configBLE_PERIPHERAL == 1)
        if (dlg_role & GAP_PERIPHERAL_ROLE) {
                gtl_role |= GAP_ROLE_PERIPHERAL;
        }
#endif /* (dg_configBLE_PERIPHERAL == 1) */
#if (dg_configBLE_BROADCASTER == 1)
        if (dlg_role & GAP_BROADCASTER_ROLE) {
                gtl_role |= GAP_ROLE_BROADCASTER;
        }
#endif /* (dg_configBLE_BROADCASTER == 1) */
#if (dg_configBLE_OBSERVER == 1)
        if (dlg_role & GAP_OBSERVER_ROLE) {
                gtl_role |= GAP_ROLE_OBSERVER;
        }
#endif /* (dg_configBLE_OBSERVER == 1) */

        return gtl_role;
}

/** \brief Create GTL system configuration command using current ble_dev_params */
static ble_mgr_common_stack_msg_t *ble_gap_dev_params_to_gtl(ble_dev_params_t *ble_dev_params)
{
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapm_set_dev_config_cmd *gcmd;

        gmsg = ble_gtl_alloc(GAPM_SET_DEV_CONFIG_CMD,TASK_ID_GAPM,
                       sizeof(struct gapm_set_dev_config_cmd));
        gcmd = (struct gapm_set_dev_config_cmd *) gmsg->msg.gtl.param;

        gcmd->operation = GAPM_SET_DEV_CONFIG;
        gcmd->role      = dlg_role_to_gtl_role(ble_dev_params->role);
        gcmd->renew_dur = ble_dev_params->addr_renew_duration;
        gcmd->att_cfg   = ble_dev_params->att_db_cfg;
        gcmd->max_mtu   = ble_dev_params->mtu_size;
        gcmd->max_mps   = ble_dev_params->mtu_size;

        memcpy(&gcmd->addr, &ble_dev_params->own_addr.addr, BD_ADDR_LEN);
        switch (ble_dev_params->own_addr.addr_type) {
        case PUBLIC_STATIC_ADDRESS:
                gcmd->addr_type = GAPM_CFG_ADDR_PUBLIC;
                break;
        case PRIVATE_STATIC_ADDRESS:
                gcmd->addr_type = GAPM_CFG_ADDR_PRIVATE;
                break;
        case PRIVATE_RANDOM_RESOLVABLE_ADDRESS:
        case PRIVATE_RANDOM_NONRESOLVABLE_ADDRESS:
                gcmd->addr_type = GAPM_CFG_ADDR_PRIVACY;
                break;
#if (dg_configBLE_PRIVACY_1_2 == 1)
        case PRIVATE_CNTL:
                gcmd->addr_type = GAPM_CFG_ADDR_PRIVACY_CNTL;
                break;
#endif /* (dg_configBLE_PRIVACY_1_2 == 1) */
        default:
                gcmd->addr_type = GAPM_CFG_ADDR_PUBLIC;
                break;
        }

#if (dg_configBLE_PRIVACY_1_2 == 1)
        ble_dev_params->prev_privacy_operation = BLE_MGR_RAL_OP_NONE;
#endif /* (dg_configBLE_PRIVACY_1_2 == 1) */

        memcpy(&gcmd->irk, &ble_dev_params->irk, sizeof(gap_sec_key_t));

        /* Set max TX octets and time according to the defined maximum TX data length */
        gcmd->max_txoctets = dg_configBLE_DATA_LENGTH_TX_MAX;
        gcmd->max_txtime = (dg_configBLE_DATA_LENGTH_TX_MAX + 11 + 3) * 8;  // Conversion from llm.h

        return gmsg;
}

void ble_mgr_gap_dev_bdaddr_ind_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gapm_dev_bdaddr_ind *gevt = (void *) gtl->param;
        ble_dev_params_t *ble_dev_params = ble_mgr_dev_params_acquire();

        /* Update device BD address */
        memcpy(ble_dev_params->own_addr.addr, gevt->addr.addr.addr, BD_ADDR_LEN);

        ble_mgr_dev_params_release();
}

void ble_mgr_gap_adv_report_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gapm_adv_report_ind *gevt = (void *) gtl->param;
        ble_evt_gap_adv_report_t *evt;

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_GAP_ADV_REPORT, sizeof(*evt) + gevt->report.data_len);
        evt->type = gevt->report.evt_type;
        evt->rssi = gevt->report.rssi;
#if (dg_configBLE_PRIVACY_1_2 == 1)
        /* Mask the flag indicating that the address was resolved by the controller */
        evt->address.addr_type = gevt->report.adv_addr_type & 0x01;
#else
        evt->address.addr_type = gevt->report.adv_addr_type;
#endif /* (dg_configBLE_PRIVACY_1_2 == 1) */
        memcpy(evt->address.addr, gevt->report.adv_addr.addr, sizeof(evt->address.addr));
        evt->length = gevt->report.data_len;
        memcpy(evt->data, gevt->report.data, gevt->report.data_len);

        /* Send to event queue */
        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
}

static void gapm_address_resolve_complete(ble_gtl_msg_t *gtl, void *param)
{
        struct gapm_cmp_evt *gevt = (void *) gtl->param;
        ble_evt_gap_connected_t *evt = param;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapc_connection_cfm *gcmd;
        device_t *dev;
        uint16_t svc_chg_ccc = 0x0000;

        gmsg = ble_gtl_alloc_with_conn(GAPC_CONNECTION_CFM, TASK_ID_GAPC, evt->conn_idx, sizeof(*gcmd));
        gcmd = (struct gapc_connection_cfm *) gmsg->msg.gtl.param;

        storage_acquire();

        dev = find_device_by_conn_idx(evt->conn_idx);
        if (!dev) {
                /* Free BLE_EVT_GAP_CONNECTED event */
                ble_msg_free(evt);

                storage_release();
                return;
        }

        gcmd->auth = dev->bonded ? GAP_AUTH_BOND : 0;
        gcmd->auth |= dev->mitm ? GAP_AUTH_MITM : 0;
#if (dg_configBLE_SECURE_CONNECTIONS == 1)
        gcmd->auth |= dev->secure ? GAP_AUTH_SEC : 0;
#endif /* (dg_configBLE_SECURE_CONNECTIONS == 1) */
#if (RWBLE_SW_VERSION_MINOR >= 1)
        gcmd->auth |= dev->remote_ltk ? GAPC_LTK_MASK : 0;
#endif /* (RWBLE_SW_VERSION_MINOR >= 1) */

        /* Check if device was resolved and change address */
        if (gevt->status == GAP_ERR_NO_ERROR && dev->addr.addr_type !=
                                                                evt->peer_address.addr_type) {
                evt->peer_address.addr_type = dev->addr.addr_type;
                memcpy(evt->peer_address.addr, dev->addr.addr, sizeof(evt->peer_address.addr));
        }

        if (dev->csrk) {
                gcmd->lsign_counter = dev->csrk->sign_cnt;
                memcpy(&gcmd->lcsrk.key, dev->csrk->key, sizeof(gcmd->lcsrk.key));
        }

        if (dev->remote_csrk) {
                gcmd->rsign_counter = dev->remote_csrk->sign_cnt;
                memcpy(&gcmd->rcsrk.key, dev->remote_csrk->key, sizeof(gcmd->rcsrk.key));
        }

        dev->resolving = false;

        storage_release();

        /* Retrieve value for Service Changed Characteristic CCC value */
        ble_storage_get_u16(evt->conn_idx, STORAGE_KEY_SVC_CHANGED_CCC, &svc_chg_ccc);
        gcmd->svc_changed_ind_enable = !!(svc_chg_ccc & GATT_CCC_INDICATIONS);

        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);

        ble_gtl_send(gmsg);
}

static bool device_match_irk(const device_t *dev, void *ud)
{
        struct gap_sec_key *irk = (struct gap_sec_key *) ud;

        if (!dev->irk) {
                return false;
        }

        return !memcmp(irk->key, dev->irk->key, sizeof(dev->irk->key));
}

void ble_mgr_gap_addr_solved_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gapm_addr_solved_ind *gevt = (void *) gtl->param;
        ble_dev_params_t *ble_dev_params;
        uint16_t addr_resolv_req_pending;
        device_t *temp_dev, *dev;
        bd_address_t address;

        memcpy(&address.addr, gevt->addr.addr, sizeof(address.addr));
        address.addr_type = PRIVATE_ADDRESS;

        storage_acquire();
        ble_dev_params = ble_mgr_dev_params_acquire();
        addr_resolv_req_pending = ble_dev_params->addr_resolv_req_pending;
        ble_mgr_dev_params_release();

        /* Send event to application if address resolution was requested */
        if (addr_resolv_req_pending) {
                ble_evt_gap_address_resolved_t *evt;

                /* Find the device associated with the IRK that resolved the address */
                dev = find_device(device_match_irk, &gevt->irk);

                if (dev) {
                        evt = ble_evt_init(BLE_EVT_GAP_ADDRESS_RESOLVED, sizeof(*evt));

                        memcpy(&evt->resolved_address, &dev->addr, sizeof(dev->addr));
                        memcpy(&evt->address, &address, sizeof(address));

                        if (dev->connected) {
                                /* Resolved address corresponds to a connected device */
                                evt->conn_idx = dev->conn_idx;
                        } else {
                                /* Resolved address does not correspond to a connected device */
                                evt->conn_idx = BLE_CONN_IDX_INVALID;
                        }

                        /* Send to event queue */
                        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
                }
        } else {
                temp_dev = find_device_by_addr(&address, false);
                if (!temp_dev || !temp_dev->connected) {
                        goto done;
                }

                dev = find_device(device_match_irk, &gevt->irk);
                if (!dev) {
                        goto done;
                }

                dev->conn_idx = temp_dev->conn_idx;
                dev->master = temp_dev->master;
                dev->connected = true;

                device_remove(temp_dev);
        }

done:
        storage_release();
}

static void irk_count_cb(device_t *dev, void *ud)
{
        uint8_t *irk_count = (uint8_t *) ud;

        if (dev->irk) {
                (*irk_count)++;
        }
}

struct irk_copy_data {
        uint8_t index;
        struct gap_sec_key *array;
};

static void irk_copy_cb(device_t *dev, void *ud)
{
        struct irk_copy_data *copy_data = (struct irk_copy_data *) ud;

        if (dev->irk) {
                memcpy(&copy_data->array[copy_data->index++], dev->irk->key, sizeof(dev->irk->key));
        }
}

static bool resolve_address_from_connected_evt(const struct gapc_connection_req_ind *evt, void *param)
{
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapm_resolv_addr_cmd *gcmd;
        struct irk_copy_data copy_data;
        uint8_t irk_count = 0;

        /* Check if peer's address is random */
        if (evt->peer_addr_type != PRIVATE_ADDRESS) {
                return false;
        }

        /* Check if peer's address is resolvable */
        if ((evt->peer_addr.addr[5] & 0xc0) != 0x40) {
                return false;
        }

        device_foreach(irk_count_cb, &irk_count);
        if (irk_count == 0) {
                return false;
        }

        gmsg = ble_gtl_alloc(GAPM_RESOLV_ADDR_CMD, TASK_ID_GAPM, sizeof(*gcmd) +
                                                        (sizeof(struct gap_sec_key) * irk_count));
        gcmd = (struct gapm_resolv_addr_cmd *) gmsg->msg.gtl.param;
        memcpy(&gcmd->addr, &evt->peer_addr, sizeof(gcmd->addr));
        gcmd->operation = GAPM_RESOLV_ADDR;
        gcmd->nb_key = irk_count;

        /* Copy IRKs */
        copy_data.array = gcmd->irk;
        copy_data.index = 0;
        device_foreach(irk_copy_cb, &copy_data);

        ble_gtl_waitqueue_add(BLE_CONN_IDX_INVALID, GAPM_CMP_EVT, GAPM_RESOLV_ADDR,
                              gapm_address_resolve_complete, param);
        ble_gtl_send(gmsg);

        return true;
}

#if (dg_configBLE_CENTRAL == 1)
static void get_peer_features(uint16_t conn_idx)
{
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapc_get_info_cmd *gcmd;

        gmsg = ble_gtl_alloc_with_conn(GAPC_GET_INFO_CMD, TASK_ID_GAPC, conn_idx, sizeof(*gcmd));
        gcmd = (struct gapc_get_info_cmd *) gmsg->msg.gtl.param;
        gcmd->operation = GAPC_GET_PEER_FEATURES;

        ble_gtl_send(gmsg);
}
#endif/* (dg_configBLE_CENTRAL == 1) */

static void change_conn_data_length(uint16_t conn_idx, uint16_t tx_length, uint16_t tx_time)
{
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapc_set_le_pkt_size_cmd *gcmd;

        gmsg = ble_gtl_alloc_with_conn(GAPC_SET_LE_PKT_SIZE_CMD, TASK_ID_GAPC, conn_idx, sizeof(*gcmd));
        gcmd = (struct gapc_set_le_pkt_size_cmd *) gmsg->msg.gtl.param;
        gcmd->operation = GAPC_SET_LE_PKT_SIZE;
        gcmd->tx_octets = tx_length;
        gcmd->tx_time = tx_time;

        ble_gtl_send(gmsg);
}

void ble_mgr_gap_peer_features_ind_evt_handler(ble_gtl_msg_t *gtl)
{
#if (dg_configBLE_CENTRAL == 1)
#if ((dg_configBLE_DATA_LENGTH_RX_MAX > GAPM_LE_LENGTH_EXT_OCTETS_MIN) \
                     || (dg_configBLE_DATA_LENGTH_TX_MAX > GAPM_LE_LENGTH_EXT_OCTETS_MIN))
        struct gapc_peer_features_ind *gevt = (void *) gtl->param;
        device_t *dev;

        /* Check if the peer supports LE Data Packet Length Extension feature */
        if (gevt->features[0] & BLE_LE_LENGTH_FEATURE) {
                storage_acquire();

                dev = find_device_by_conn_idx(TASK_2_CONNIDX(gtl->src_id));

                /* If we are the master of the connection initiate a Data Length Update procedure */
                if (dev && dev->master) {
                        /* Set TX data length for connection to the maximum supported */
                        change_conn_data_length(TASK_2_CONNIDX(gtl->src_id),
                                dg_configBLE_DATA_LENGTH_TX_MAX,
                                BLE_DATA_LENGTH_TO_TIME(dg_configBLE_DATA_LENGTH_TX_MAX));
                }

                storage_release();
        }
#endif
#endif /* (dg_configBLE_CENTRAL == 1) */
}

#if (dg_configBLE_CENTRAL == 1)
static void get_peer_version(uint16_t conn_idx)
{
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapc_get_info_cmd *gcmd;

        gmsg = ble_gtl_alloc_with_conn(GAPC_GET_INFO_CMD, TASK_ID_GAPC, conn_idx, sizeof(*gcmd));
        gcmd = (struct gapc_get_info_cmd *) gmsg->msg.gtl.param;
        gcmd->operation = GAPC_GET_PEER_VERSION;

        ble_gtl_send(gmsg);
}
#endif /* (dg_configBLE_CENTRAL == 1) */

void ble_mgr_gap_peer_version_ind_evt_handler(ble_gtl_msg_t *gtl)
{
#if (dg_configBLE_CENTRAL == 1)
        device_t *dev;

        storage_acquire();

        dev = find_device_by_conn_idx(TASK_2_CONNIDX(gtl->src_id));

        if (dev && dev->master) {
                /* Initiate a Feature Exchange procedure */
                get_peer_features(TASK_2_CONNIDX(gtl->src_id));
        }

        storage_release();
#endif /* (dg_configBLE_CENTRAL == 1) */
}

void ble_mgr_gap_connected_evt_handler(ble_gtl_msg_t *gtl)
{
        ble_evt_gap_connected_t *evt;
        struct gapc_connection_req_ind *gevt = (void *) gtl->param;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapc_connection_cfm *gcmd;
        ble_dev_params_t *ble_dev_params = ble_mgr_dev_params_acquire();
        device_t *dev;
        uint16_t svc_chg_ccc = 0x0000;

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_GAP_CONNECTED, sizeof(*evt));
        evt->conn_idx                  = TASK_2_CONNIDX(gtl->src_id);
        evt->own_addr.addr_type        = ble_dev_params->own_addr.addr_type;
        memcpy(evt->own_addr.addr, ble_dev_params->own_addr.addr, sizeof(evt->own_addr.addr));
#if (dg_configBLE_PRIVACY_1_2 == 1)
        /* Mask the flag indicating that the address was resolved by the controller */
        evt->peer_address.addr_type    = gevt->peer_addr_type & 0x01;
#else
        evt->peer_address.addr_type    = gevt->peer_addr_type;
#endif /* (dg_configBLE_PRIVACY_1_2 == 1) */
        memcpy(evt->peer_address.addr, gevt->peer_addr.addr, sizeof(evt->peer_address.addr));
        evt->conn_params.interval_min  = gevt->con_interval;
        evt->conn_params.interval_max  = gevt->con_interval;
        evt->conn_params.slave_latency = gevt->con_latency;
        evt->conn_params.sup_timeout   = gevt->sup_to;

        //evt->clk_accuracy             = gevt->clk_accuracy;
#if (dg_configBLE_SKIP_LATENCY_API == 1)
        ble_mgr_skip_latency_set(evt->conn_idx, false);
#endif /* (dg_configBLE_SKIP_LATENCY_API == 1) */

        storage_acquire();

        dev = find_device_by_addr(&evt->peer_address, true);
        dev->conn_idx = evt->conn_idx;
        dev->connected = true;
        dev->mtu = ATT_DEFAULT_MTU;

        if (dev->connecting) {
                dev->master = true;
                dev->connecting = false;
        } else {
                dev->master = false;
        }

#if (dg_configBLE_CENTRAL == 1)
        if (dev->master) {
                /* Initiate a Version Exchange */
                get_peer_version(evt->conn_idx);
        }
#endif /* (dg_configBLE_CENTRAL == 1) */

#if (dg_configBLE_PRIVACY_1_2 == 1)
        if (ble_dev_params->own_addr.addr_type != PRIVATE_CNTL)
#endif /* (dg_configBLE_PRIVACY_1_2 == 1) */
        {
                if (resolve_address_from_connected_evt(gevt, evt)) {
                        dev->resolving = true;
                        goto done;
                }
        }

        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);

        gmsg = ble_gtl_alloc_with_conn(GAPC_CONNECTION_CFM, TASK_ID_GAPC, evt->conn_idx, sizeof(*gcmd));
        gcmd = (struct gapc_connection_cfm *) gmsg->msg.gtl.param;
        gcmd->auth = dev->bonded ? GAP_AUTH_BOND : 0;
        gcmd->auth |= dev->mitm ? GAP_AUTH_MITM : 0;
#if (dg_configBLE_SECURE_CONNECTIONS == 1)
        gcmd->auth |= dev->secure ? GAP_AUTH_SEC : 0;
#endif /* (dg_configBLE_SECURE_CONNECTIONS == 1) */
#if (RWBLE_SW_VERSION_MINOR >= 1)
        gcmd->auth |= dev->remote_ltk ? GAPC_LTK_MASK : 0;
#endif /* (RWBLE_SW_VERSION_MINOR >= 1) */

        if (dev->csrk) {
                gcmd->lsign_counter = dev->csrk->sign_cnt;
                memcpy(&gcmd->lcsrk.key, dev->csrk->key, sizeof(gcmd->lcsrk.key));
        }

        if (dev->remote_csrk) {
                gcmd->rsign_counter = dev->remote_csrk->sign_cnt;
                memcpy(&gcmd->rcsrk.key, dev->remote_csrk->key, sizeof(gcmd->rcsrk.key));
        }

        /* Retrieve value for Service Changed Characteristic CCC value */
        ble_storage_get_u16(evt->conn_idx, STORAGE_KEY_SVC_CHANGED_CCC, &svc_chg_ccc);
        gcmd->svc_changed_ind_enable = !!(svc_chg_ccc & GATT_CCC_INDICATIONS);

        ble_gtl_send(gmsg);

done:
        storage_release();
        ble_mgr_dev_params_release();
}

static void gapm_address_set_rsp(ble_gtl_msg_t *gtl, void *param)
{
        struct gapm_cmp_evt *gevt = (void *) gtl->param;
        ble_mgr_gap_address_set_cmd_t *cmd = param;
        ble_mgr_gap_address_set_rsp_t *rsp;

        if (gevt->status == GAP_ERR_NO_ERROR) {
                ble_dev_params_t *ble_dev_params = ble_mgr_dev_params_acquire();

                /* Update ble_dev_params with the new values for address and address_type */
                ble_dev_params->own_addr.addr_type = cmd->address->addr_type;

                switch (cmd->address->addr_type) {
                case PUBLIC_STATIC_ADDRESS:
                        ad_ble_get_public_address(ble_dev_params->own_addr.addr);
                        break;
                case PRIVATE_STATIC_ADDRESS:
                        memcpy(ble_dev_params->own_addr.addr, cmd->address->addr, BD_ADDR_LEN);
                        break;
#if (dg_configBLE_PRIVACY_1_2 == 1)
                case PRIVATE_CNTL:
                        /* The actual address depends on the air operation and whether the
                         * relevant peer is included in the RAL or not.
                         */
                        memset(ble_dev_params->own_addr.addr, 0, BD_ADDR_LEN);
                        ble_dev_params->addr_renew_duration = 0;
                        break;
#endif /* (dg_configBLE_PRIVACY_1_2 == 1) */
                default:
                        /*
                         * Assume it's either private random non-resolvable or resolvable address.
                         * We clear addr field to avoid confusion in application - only address type
                         * matters here. Proper address will be written when GAPM_DEV_BDADDR_IND
                         * is received.
                         */
                        memset(ble_dev_params->own_addr.addr, 0, BD_ADDR_LEN);

                        ble_dev_params->addr_renew_duration = cmd->renew_dur;
                        break;
                }

                ble_mgr_dev_params_release();
        }

        /* Free command buffer */
        ble_msg_free(cmd);

        /* Create response */
        rsp = ble_msg_init(BLE_MGR_GAP_ADDRESS_SET_CMD, sizeof(*rsp));
        rsp->status = (gevt->status == 0 ? BLE_STATUS_OK : BLE_ERROR_FAILED);

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gap_address_set_cmd_handler(void *param)
{
        const ble_mgr_gap_address_set_cmd_t *cmd = param;
        ble_mgr_gap_address_set_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapm_set_dev_config_cmd *gcmd;
        ble_dev_params_t *ble_dev_params = ble_mgr_dev_params_acquire();
        ble_error_t ret = BLE_ERROR_FAILED;

        /* Check if an air operation is in progress (GAPM_SET_DEV_CONFIG_CMD cannot be sent now) */
        if ((ble_dev_params->advertising == true) || (ble_dev_params->scanning == true)) {
                goto done;
        }

        /* Setup GTL message to update device configuration (for addr_type and addr) */
        gmsg = ble_gap_dev_params_to_gtl(ble_dev_params);
        gcmd = (struct gapm_set_dev_config_cmd *) gmsg->msg.gtl.param;
        switch (cmd->address->addr_type) {
        case PUBLIC_STATIC_ADDRESS:
                gcmd->addr_type = GAPM_CFG_ADDR_PUBLIC;
                break;
        case PRIVATE_STATIC_ADDRESS:
                gcmd->addr_type = GAPM_CFG_ADDR_PRIVATE;
                memcpy(gcmd->addr.addr, cmd->address->addr, BD_ADDR_LEN);
                break;
        case PRIVATE_RANDOM_RESOLVABLE_ADDRESS:
        case PRIVATE_RANDOM_NONRESOLVABLE_ADDRESS:
                gcmd->renew_dur = cmd->renew_dur;
                gcmd->addr_type = GAPM_CFG_ADDR_PRIVACY;
                break;
#if (dg_configBLE_PRIVACY_1_2 == 1)
        case PRIVATE_CNTL:
                gcmd->renew_dur = cmd->renew_dur;
                gcmd->addr_type = GAPM_CFG_ADDR_PRIVACY_CNTL;
                gcmd->priv1_2 = 0;
                break;
#endif /* (dg_configBLE_PRIVACY_1_2 == 1) */
        default:
                gcmd->addr_type = GAPM_CFG_ADDR_PUBLIC;
                break;
        }

        /* Keep param buffer, we'll need it when creating response */
        ble_gtl_waitqueue_add(BLE_CONN_IDX_INVALID, GAPM_CMP_EVT, GAPM_SET_DEV_CONFIG,
                              gapm_address_set_rsp, (void *) cmd);

        ble_gtl_send(gmsg);

        ble_mgr_dev_params_release();
        return;

done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GAP_ADDRESS_SET_CMD, sizeof(*rsp));
        rsp->status = ret;

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);

        ble_mgr_dev_params_release();
}

static void gapm_att_db_cfg_devname_perm_set_rsp(ble_gtl_msg_t *gtl, void *param)
{
        struct gapm_cmp_evt *gevt = (void *) gtl->param;
        ble_mgr_gap_device_name_set_cmd_t *cmd = param;
        ble_mgr_gap_device_name_set_rsp_t *rsp;

        if (gevt->status == GAP_ERR_NO_ERROR) {
                ble_dev_params_t *ble_dev_params = ble_mgr_dev_params_acquire();

                /* Update ble_dev_params with the new values for the device name and att_db_cfg */
                strcpy(ble_dev_params->dev_name, cmd->name);
                ble_dev_params->att_db_cfg = (ble_dev_params->att_db_cfg & ~GAPM_MASK_ATT_NAME_PERM)
                                             | devname_perm_to_perm(cmd->perm);

                ble_mgr_dev_params_release();
        }

        /* Free command buffer */
        ble_msg_free(cmd);

        /* Create response */
        rsp = ble_msg_init(BLE_MGR_GAP_DEVICE_NAME_SET_CMD, sizeof(*rsp));
        rsp->status = (gevt->status == 0 ? BLE_STATUS_OK : BLE_ERROR_FAILED);

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gap_device_name_set_cmd_handler(void *param)
{
        const ble_mgr_gap_device_name_set_cmd_t *cmd = param;
        ble_mgr_gap_device_name_set_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapm_set_dev_config_cmd *gcmd;
        ble_dev_params_t *ble_dev_params = ble_mgr_dev_params_acquire();
        ble_error_t ret = BLE_ERROR_FAILED;

        /* Check if the provided name is longer than the defined max size */
        if ( (strlen(cmd->name) > BLE_GAP_DEVNAME_LEN_MAX) ) {
                goto done;
        }

        /* Check if the attribute database configuration bit flag needs updating */
        if ((ble_dev_params->att_db_cfg & GAPM_MASK_ATT_NAME_PERM) != devname_perm_to_perm(cmd->perm)) {
                /* att_db_cfg has to be updated */

                /* Check if an air operation is in progress (att_db_cfg cannot be updated now) */
                if ((ble_dev_params->advertising == true) || (ble_dev_params->scanning == true)) {
                        goto done;
                }

                /* Setup GTL message to update device configuration (for att_db_cfg) */
                gmsg = ble_gap_dev_params_to_gtl(ble_dev_params);
                gcmd = (struct gapm_set_dev_config_cmd *) gmsg->msg.gtl.param;
                gcmd->att_cfg = (ble_dev_params->att_db_cfg & ~GAPM_MASK_ATT_NAME_PERM) |
                        devname_perm_to_perm(cmd->perm);

                /* Keep param buffer, we'll need it when creating response */
                ble_gtl_waitqueue_add(BLE_CONN_IDX_INVALID, GAPM_CMP_EVT, GAPM_SET_DEV_CONFIG,
                                      gapm_att_db_cfg_devname_perm_set_rsp, (void *) cmd);

                ble_gtl_send(gmsg);

                ble_mgr_dev_params_release();
                return;
        }

        /* No att_db_cfg update needed */

        /* Update ble_dev_params with the new value for device name */
        strcpy(ble_dev_params->dev_name, cmd->name);
        ret = BLE_STATUS_OK;

done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GAP_DEVICE_NAME_SET_CMD, sizeof(*rsp));
        rsp->status = ret;

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);

        ble_mgr_dev_params_release();
}

static void gapm_att_db_cfg_appearance_perm_set_rsp(ble_gtl_msg_t *gtl, void *param)
{
        struct gapm_cmp_evt *gevt = (void *) gtl->param;
        ble_mgr_gap_appearance_set_cmd_t *cmd = param;
        ble_mgr_gap_appearance_set_rsp_t *rsp;

        if (gevt->status == GAP_ERR_NO_ERROR) {
                ble_dev_params_t *ble_dev_params = ble_mgr_dev_params_acquire();

                /* Update appearance member and attribute DB configuration in ble_dev_params */
                ble_dev_params->appearance = cmd->appearance;
                ble_dev_params->att_db_cfg = (ble_dev_params->att_db_cfg &
                        ~GAPM_MASK_ATT_APPEARENCE_PERM) | appearance_perm_to_perm(cmd->perm);

                ble_mgr_dev_params_release();
        }

        /* Free command buffer */
        ble_msg_free(cmd);

        /* Create response */
        rsp = ble_msg_init(BLE_MGR_GAP_APPEARANCE_SET_CMD, sizeof(*rsp));
        rsp->status = (gevt->status == 0 ? BLE_STATUS_OK : BLE_ERROR_FAILED);

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gap_appearance_set_cmd_handler(void *param)
{
        const ble_mgr_gap_appearance_set_cmd_t *cmd = param;
        ble_mgr_gap_appearance_set_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapm_set_dev_config_cmd *gcmd;
        ble_dev_params_t *ble_dev_params = ble_mgr_dev_params_acquire();
        ble_error_t ret = BLE_ERROR_FAILED;

        /* Check if the attribute database configuration bit flag needs updating */
        if ((ble_dev_params->att_db_cfg & GAPM_MASK_ATT_APPEARENCE_PERM) !=
                appearance_perm_to_perm(cmd->perm)) {
                /* att_db_cfg has to be updated */

                /* Check if an air operation is in progress (att_db_cfg cannot be updated now) */
                if ((ble_dev_params->advertising == true) || (ble_dev_params->scanning == true)) {
                        goto done;
                }

                /* Setup GTL message to update att_db_cfg in device configuration */
                gmsg = ble_gap_dev_params_to_gtl(ble_dev_params);
                gcmd = (struct gapm_set_dev_config_cmd *) gmsg->msg.gtl.param;
                gcmd->att_cfg = (ble_dev_params->att_db_cfg & ~GAPM_MASK_ATT_APPEARENCE_PERM) |
                        appearance_perm_to_perm(cmd->perm);

                /* Keep param buffer, we'll need it when creating response */
                ble_gtl_waitqueue_add(BLE_CONN_IDX_INVALID, GAPM_CMP_EVT, GAPM_SET_DEV_CONFIG,
                                      gapm_att_db_cfg_appearance_perm_set_rsp, (void *) cmd);
                ble_gtl_send(gmsg);

                ble_mgr_dev_params_release();
                return;
        }

        /* No att_db_cfg update needed */

        /* Just update appearance member in ble_dev_params */
        ble_dev_params->appearance = cmd->appearance;

        ret = BLE_STATUS_OK;

done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GAP_APPEARANCE_SET_CMD, sizeof(*rsp));
        rsp->status = ret;

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);

        ble_mgr_dev_params_release();
}

static void gapm_att_db_cfg_ppcp_en_rsp(ble_gtl_msg_t *gtl, void *param)
{
        struct gapm_cmp_evt *gevt = (void *) gtl->param;
        const ble_mgr_gap_ppcp_set_cmd_t *cmd = param;
        ble_mgr_gap_ppcp_set_rsp_t *rsp;

        if (gevt->status == GAP_ERR_NO_ERROR) {
                ble_dev_params_t *ble_dev_params = ble_mgr_dev_params_acquire();

                /* Update PPCP member in ble_dev_params */
                memcpy(&ble_dev_params->gap_ppcp, cmd->gap_ppcp, sizeof(ble_dev_params->gap_ppcp));
                /* Update the PPCP present bit in att_db_cfg */
                ble_dev_params->att_db_cfg |= GAPM_MASK_ATT_SLV_PREF_CON_PAR_EN;

                ble_mgr_dev_params_release();
        }

        /* Free command buffer */
        ble_msg_free(param);

        /* Create response */
        rsp = ble_msg_init(BLE_MGR_GAP_PPCP_SET_CMD, sizeof(*rsp));
        rsp->status = (gevt->status == 0 ? BLE_STATUS_OK : BLE_ERROR_FAILED);

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gap_ppcp_set_cmd_handler(void *param)
{
        const ble_mgr_gap_ppcp_set_cmd_t *cmd = param;
        ble_mgr_gap_ppcp_set_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapm_set_dev_config_cmd *gcmd;
        ble_dev_params_t *ble_dev_params = ble_mgr_dev_params_acquire();
        ble_error_t ret = BLE_ERROR_FAILED;

        /* Check if the attribute database configuration bit flag needs updating */
        if ((ble_dev_params->att_db_cfg & GAPM_MASK_ATT_SLV_PREF_CON_PAR_EN) == 0x00) {
                /* att_db_cfg has to be updated */

                /* Check if an air operation is in progress (att_db_cfg cannot be updated now) */
                if ((ble_dev_params->advertising == true) || (ble_dev_params->scanning == true)) {
                        goto done;
                }

                /* Setup GTL message to update att_db_cfg in device configuration */
                gmsg = ble_gap_dev_params_to_gtl(ble_dev_params);
                gcmd = (struct gapm_set_dev_config_cmd *) gmsg->msg.gtl.param;

                /* Enable PPCP present bit in att_db_cfg */
                gcmd->att_cfg = ble_dev_params->att_db_cfg | GAPM_MASK_ATT_SLV_PREF_CON_PAR_EN;

                ble_gtl_waitqueue_add(BLE_CONN_IDX_INVALID, GAPM_CMP_EVT, GAPM_SET_DEV_CONFIG,
                                      gapm_att_db_cfg_ppcp_en_rsp, (void *) cmd);
                ble_gtl_send(gmsg);

                ble_mgr_dev_params_release();

                return;
        }

        /* No att_db_cfg update needed */

        /* Just update PPCP member in ble_dev_params */
        memcpy(&ble_dev_params->gap_ppcp, cmd->gap_ppcp, sizeof(ble_dev_params->gap_ppcp));

        ret = BLE_STATUS_OK;

done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GAP_PPCP_SET_CMD, sizeof(*rsp));
        rsp->status = ret;

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);

        ble_mgr_dev_params_release();
}

void ble_mgr_gap_adv_start_cmd_exec(void *param)
{
        const ble_mgr_gap_adv_start_cmd_t *cmd = param;
        ble_mgr_gap_adv_start_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapm_start_advertise_cmd *gcmd;
        ble_dev_params_t *ble_dev_params = ble_mgr_dev_params_acquire();
        ble_error_t ret = BLE_ERROR_FAILED;

        /* Check if an advertising operation is already in progress */
        if (ble_dev_params->advertising == true) {
                ret = BLE_ERROR_IN_PROGRESS;
                goto done;
        }

        /*
         * Check if length of advertising data is within limits
         */
        if (((cmd->adv_type == GAP_CONN_MODE_NON_CONN) && (ble_dev_params->adv_data_length > BLE_NON_CONN_ADV_DATA_LEN_MAX)) ||
                ((cmd->adv_type == GAP_CONN_MODE_UNDIRECTED) && (ble_dev_params->adv_data_length > BLE_ADV_DATA_LEN_MAX))) {
                        ret = BLE_ERROR_INVALID_PARAM;
                        goto done;
        }

        /* Update BLE device advertising parameters */
        ble_dev_params->adv_type = cmd->adv_type;

        gmsg = ble_gtl_alloc(GAPM_START_ADVERTISE_CMD, TASK_ID_GAPM, sizeof(struct gapm_start_advertise_cmd));
        gcmd = (struct gapm_start_advertise_cmd *) gmsg->msg.gtl.param;

        /* Translate Dialog API advertising type to BLE stack operation code */
        switch (cmd->adv_type) {
        case GAP_CONN_MODE_NON_CONN:
                gcmd->op.code = GAPM_ADV_NON_CONN;
                break;
        case GAP_CONN_MODE_UNDIRECTED:
                gcmd->op.code = GAPM_ADV_UNDIRECT;
                break;
        case GAP_CONN_MODE_DIRECTED:
                gcmd->op.code = GAPM_ADV_DIRECT;
                break;
        case GAP_CONN_MODE_DIRECTED_LDC:
                gcmd->op.code = GAPM_ADV_DIRECT_LDC;
                break;
        }

        switch (ble_dev_params->own_addr.addr_type) {
        case PUBLIC_STATIC_ADDRESS:
        case PRIVATE_STATIC_ADDRESS:
                gcmd->op.addr_src = GAPM_STATIC_ADDR;
                break;
        case PRIVATE_RANDOM_RESOLVABLE_ADDRESS:
                gcmd->op.addr_src = GAPM_GEN_RSLV_ADDR;
                break;
        case PRIVATE_RANDOM_NONRESOLVABLE_ADDRESS:
                gcmd->op.addr_src = GAPM_GEN_NON_RSLV_ADDR;
                break;
#if (dg_configBLE_PRIVACY_1_2 == 1)
        case PRIVATE_CNTL:
                /* Generate AdvA using local IRK */
                gcmd->op.addr_src = GAPM_GEN_RSLV_ADDR;
                ad_ble_get_public_address(gcmd->info.host.peer_info.addr.addr);
                gcmd->info.host.peer_info.addr_type = ADDR_PUBLIC;
                break;
#endif /* (dg_configBLE_PRIVACY_1_2 == 1) */
        default:
                gcmd->op.addr_src = GAPM_STATIC_ADDR;
                break;
        }

        gcmd->intv_min                    = ble_dev_params->adv_intv_min;
        gcmd->intv_max                    = ble_dev_params->adv_intv_max;
        gcmd->channel_map                 = ble_dev_params->adv_channel_map;
        if (cmd->adv_type < GAP_CONN_MODE_DIRECTED) {
                /* Fill info for undirected or broadcaster mode advertising */
                gcmd->info.host.mode              = ble_dev_params->adv_mode;
                gcmd->info.host.adv_filt_policy   = ble_dev_params->adv_filter_policy;
                gcmd->info.host.adv_data_len      = ble_dev_params->adv_data_length;
                memcpy(gcmd->info.host.adv_data, ble_dev_params->adv_data,
                        ble_dev_params->adv_data_length);
                gcmd->info.host.scan_rsp_data_len = ble_dev_params->scan_rsp_data_length;
                memcpy(gcmd->info.host.scan_rsp_data, ble_dev_params->scan_rsp_data,
                        ble_dev_params->scan_rsp_data_length);
        }
        else {
                /* Fill info for directed advertising */
                gcmd->info.direct.addr_type = ble_dev_params->adv_direct_address.addr_type;
                memcpy(gcmd->info.direct.addr.addr, ble_dev_params->adv_direct_address.addr,
                        BD_ADDR_LEN);
        }

        /* Set advertising flag to true */
        ble_dev_params->advertising = true;

        ble_gtl_send(gmsg);

        ret = BLE_STATUS_OK;
done:
        ble_msg_free(param);

        /* Create response */
        rsp = ble_msg_init(BLE_MGR_GAP_ADV_START_CMD, sizeof(*rsp));

        rsp->status = ret;

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);

        ble_mgr_dev_params_release();
}

void ble_mgr_gap_adv_start_cmd_handler(void *param)
{
#if (dg_configBLE_PRIVACY_1_2 == 1)
        ble_mgr_gap_ral_sync(ble_mgr_gap_adv_start_cmd_exec, param);
#else
        ble_mgr_gap_adv_start_cmd_exec(param);
#endif /* (dg_configBLE_PRIVACY_1_2 == 1) */
}


void ble_mgr_gapm_adv_cmp_evt_handler(ble_gtl_msg_t *gtl)
{
        ble_evt_gap_adv_completed_t *evt;
        struct gapm_cmp_evt *gevt = (void *) gtl->param;
        ble_dev_params_t *ble_dev_params = ble_mgr_dev_params_acquire();

        /* Set advertising flag to false */
        ble_dev_params->advertising = false;

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_GAP_ADV_COMPLETED, sizeof(*evt));

        /* Translate stack advertising operation to Dialog API advertising type */
        switch (gevt->operation) {
        case GAPM_ADV_NON_CONN:
                evt->adv_type = GAP_CONN_MODE_NON_CONN;
                break;
        case GAPM_ADV_UNDIRECT:
                evt->adv_type = GAP_CONN_MODE_UNDIRECTED;
                break;
        case GAPM_ADV_DIRECT:
                evt->adv_type = GAP_CONN_MODE_DIRECTED;
                break;
        case GAPM_ADV_DIRECT_LDC:
                evt->adv_type = GAP_CONN_MODE_DIRECTED_LDC;
                break;
        }

        /* Translate stack status */
        switch (gevt->status) {
        case GAP_ERR_NO_ERROR:
                evt->status = BLE_STATUS_OK;
                break;
        case GAP_ERR_CANCELED:
                evt->status = BLE_ERROR_CANCELED;
                break;
        case GAP_ERR_COMMAND_DISALLOWED:
                evt->status = BLE_ERROR_NOT_ALLOWED;
                break;
        case GAP_ERR_INVALID_PARAM:
        case GAP_ERR_ADV_DATA_INVALID:
        case LL_ERR_PARAM_OUT_OF_MAND_RANGE:
                evt->status = BLE_ERROR_INVALID_PARAM;
                break;
        case GAP_ERR_NOT_SUPPORTED:
        case GAP_ERR_PRIVACY_CFG_PB:
                evt->status = BLE_ERROR_NOT_SUPPORTED;
                break;
        case GAP_ERR_TIMEOUT:
                evt->status = BLE_ERROR_TIMEOUT;
                break;
        default:
                evt->status   = gevt->status;
                break;
        }

        /* Send to event queue */
        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);

        ble_mgr_dev_params_release();
}

void ble_mgr_gap_adv_stop_cmd_handler(void *param)
{
        ble_mgr_gap_adv_stop_rsp_t *rsp;
        ble_dev_params_t *ble_dev_params = ble_mgr_dev_params_acquire();
        ble_error_t ret = BLE_ERROR_FAILED;

        if (!ble_dev_params->advertising) {
                ret = BLE_ERROR_NOT_ALLOWED;
                goto done;
        }

        /*
         * This assumes we will cancel this properly (as we check whether we're actually
         * advertising) and the event will be generated to application when GAPM_CMP_EVT is received
         * with proper operation for advertising.
         * In case something failed, GAPM_CMD_EVT will have operation set to GAPM_CANCEL and we will
         * just discard it silently - not much we can do there anyway.
         */
        send_gapm_cancel_cmd();
        ret = BLE_STATUS_OK;

done:
        ble_mgr_dev_params_release();
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GAP_ADV_STOP_CMD, sizeof(*rsp));
        rsp->status = ret;

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

static void gapm_adv_data_update_cmd_rsp(ble_gtl_msg_t *gtl, void *param)
{
        struct gapm_cmp_evt *gevt = (void *) gtl->param;
        ble_mgr_gap_adv_data_set_cmd_t *cmd = param;
        ble_mgr_gap_adv_data_set_rsp_t *rsp;

        if (gevt->status == GAP_ERR_NO_ERROR) {
                ble_dev_params_t *ble_dev_params = ble_mgr_dev_params_acquire();

                ble_dev_params->adv_data_length = cmd->adv_data_len;
                memcpy(ble_dev_params->adv_data, cmd->adv_data, cmd->adv_data_len);
                ble_dev_params->scan_rsp_data_length = cmd->scan_rsp_data_len;
                memcpy(ble_dev_params->scan_rsp_data, cmd->scan_rsp_data, cmd->scan_rsp_data_len);

                ble_mgr_dev_params_release();
        }

        /* Free command buffer */
        ble_msg_free(cmd);

        /* Create response */
        rsp = ble_msg_init(BLE_MGR_GAP_ADV_DATA_SET_CMD, sizeof(*rsp));

        rsp->status = (gevt->status == 0 ? BLE_STATUS_OK : BLE_ERROR_FAILED);

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gap_adv_data_set_cmd_handler(void *param)
{
        const ble_mgr_gap_adv_data_set_cmd_t *cmd = param;
        ble_dev_params_t *ble_dev_params = ble_mgr_dev_params_acquire();

        if (ble_dev_params->advertising == true) {
                ble_mgr_common_stack_msg_t *gmsg;
                struct gapm_update_advertise_data_cmd *gcmd;

                /*
                 * Check if length of advertising data is within limits
                 */
                if (((ble_dev_params->adv_type == GAP_CONN_MODE_NON_CONN) && (cmd->adv_data_len > BLE_NON_CONN_ADV_DATA_LEN_MAX)) ||
                        ((ble_dev_params->adv_type == GAP_CONN_MODE_UNDIRECTED) && (cmd->adv_data_len > BLE_ADV_DATA_LEN_MAX))) {
                        ble_mgr_gap_adv_data_set_rsp_t *rsp;

                        /* Free command buffer */
                        ble_msg_free(param);

                        /* Create response */
                        rsp = ble_msg_init(BLE_MGR_GAP_ADV_DATA_SET_CMD, sizeof(*rsp));

                        rsp->status = BLE_ERROR_INVALID_PARAM;

                        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
                } else {
                        /* Setup GTL message */
                        gmsg = ble_gtl_alloc(GAPM_UPDATE_ADVERTISE_DATA_CMD, TASK_ID_GAPM, sizeof(*gcmd));
                        gcmd = (struct gapm_update_advertise_data_cmd *) gmsg->msg.gtl.param;
                        gcmd->operation = GAPM_UPDATE_ADVERTISE_DATA;
                        gcmd->adv_data_len = cmd->adv_data_len;
                        memcpy(gcmd->adv_data, cmd->adv_data, cmd->adv_data_len);
                        gcmd->scan_rsp_data_len = cmd->scan_rsp_data_len;
                        memcpy(gcmd->scan_rsp_data, cmd->scan_rsp_data, cmd->scan_rsp_data_len);

                        ble_gtl_waitqueue_add(BLE_CONN_IDX_INVALID, GAPM_CMP_EVT,
                                                GAPM_UPDATE_ADVERTISE_DATA,
                                                gapm_adv_data_update_cmd_rsp, (void *) cmd);
                        ble_gtl_send(gmsg);
                }
        }
        else {
                ble_mgr_gap_adv_data_set_rsp_t *rsp;

                ble_dev_params->adv_data_length = cmd->adv_data_len;
                memcpy(ble_dev_params->adv_data, cmd->adv_data, cmd->adv_data_len);
                ble_dev_params->scan_rsp_data_length = cmd->scan_rsp_data_len;
                memcpy(ble_dev_params->scan_rsp_data, cmd->scan_rsp_data, cmd->scan_rsp_data_len);

                /* Free command buffer */
                ble_msg_free(param);

                /* Create response */
                rsp = ble_msg_init(BLE_MGR_GAP_ADV_DATA_SET_CMD, sizeof(*rsp));

                rsp->status = BLE_STATUS_OK;

                ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
        }

        ble_mgr_dev_params_release();
}

void ble_mgr_gap_scan_start_cmd_exec(void *param)
{
        const ble_mgr_gap_scan_start_cmd_t *cmd = param;
        ble_mgr_gap_scan_start_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapm_start_scan_cmd *gcmd;
        ble_dev_params_t *ble_dev_params = ble_mgr_dev_params_acquire();

        rsp = ble_msg_init(BLE_MGR_GAP_SCAN_START_CMD, sizeof(*rsp));

        /* Check if a scanning operation is already in progress */
        if (ble_dev_params->scanning == true) {
                rsp->status = BLE_ERROR_IN_PROGRESS;
                goto done;
        }

        gmsg = ble_gtl_alloc(GAPM_START_SCAN_CMD, TASK_ID_GAPM, sizeof(struct gapm_start_scan_cmd));
        gcmd = (struct gapm_start_scan_cmd *) gmsg->msg.gtl.param;

        /* Translate Dialog API scan type to BLE stack operation code */
        switch (cmd->type) {
        case GAP_SCAN_ACTIVE:
                gcmd->op.code = GAPM_SCAN_ACTIVE;
                break;
        case GAP_SCAN_PASSIVE:
                gcmd->op.code = GAPM_SCAN_PASSIVE;
                break;
        }

        switch (ble_dev_params->own_addr.addr_type) {
        case PUBLIC_STATIC_ADDRESS:
        case PRIVATE_STATIC_ADDRESS:
                gcmd->op.addr_src = GAPM_STATIC_ADDR;
                break;
        case PRIVATE_RANDOM_RESOLVABLE_ADDRESS:
                gcmd->op.addr_src = GAPM_GEN_RSLV_ADDR;
                break;
        case PRIVATE_RANDOM_NONRESOLVABLE_ADDRESS:
                gcmd->op.addr_src = GAPM_GEN_NON_RSLV_ADDR;
                break;
#if (dg_configBLE_PRIVACY_1_2 == 1)
        case PRIVATE_CNTL:
                gcmd->op.addr_src = GAPM_GEN_RSLV_ADDR;
                break;
#endif /* (dg_configBLE_PRIVACY_1_2 == 1) */
        default:
                gcmd->op.addr_src = GAPM_STATIC_ADDR;
                break;
        }

        gcmd->interval      = cmd->interval;
        gcmd->window        = cmd->window;
        gcmd->mode          = cmd->mode;
        gcmd->filt_policy   = cmd->filt_wlist ? SCAN_ALLOW_ADV_WLST : SCAN_ALLOW_ADV_ALL;
        gcmd->filter_duplic = cmd->filt_dupl ? SCAN_FILT_DUPLIC_EN : SCAN_FILT_DUPLIC_DIS;

        /* Set advertising flag to true */
        ble_dev_params->scanning = true;

        ble_gtl_send(gmsg);

        rsp->status = BLE_STATUS_OK;
done:
        ble_msg_free(param);

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);

        ble_mgr_dev_params_release();
}

void ble_mgr_gap_scan_start_cmd_handler(void *param)
{
#if (dg_configBLE_PRIVACY_1_2 == 1)
        ble_mgr_gap_ral_sync(ble_mgr_gap_scan_start_cmd_exec, param);
#else
        ble_mgr_gap_scan_start_cmd_exec(param);
#endif /* (dg_configBLE_PRIVACY_1_2 == 1) */
}

void ble_mgr_gapm_scan_cmp_evt_handler(ble_gtl_msg_t *gtl)
{
        ble_evt_gap_scan_completed_t *evt;
        struct gapm_cmp_evt *gevt = (void *) gtl->param;
        ble_dev_params_t *ble_dev_params = ble_mgr_dev_params_acquire();

        /* Set advertising flag to false */
        ble_dev_params->scanning = false;

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_GAP_SCAN_COMPLETED, sizeof(*evt));

        switch (gevt->operation) {
        case GAPM_SCAN_ACTIVE:
                evt->scan_type = GAP_SCAN_ACTIVE;
                break;
        case GAPM_SCAN_PASSIVE:
                evt->scan_type = GAP_SCAN_PASSIVE;
                break;
        }

        /* Translate stack status */
        switch (gevt->status) {
        case GAP_ERR_NO_ERROR:
                evt->status = BLE_STATUS_OK;
                break;
        case GAP_ERR_CANCELED:
                evt->status = BLE_ERROR_CANCELED;
                break;
        case GAP_ERR_INVALID_PARAM:
                evt->status = BLE_ERROR_INVALID_PARAM;
                break;
        case GAP_ERR_NOT_SUPPORTED:
        case GAP_ERR_PRIVACY_CFG_PB:
                evt->status = BLE_ERROR_NOT_SUPPORTED;
                break;
        case GAP_ERR_TIMEOUT:
                evt->status = BLE_ERROR_TIMEOUT;
                break;
        case GAP_ERR_COMMAND_DISALLOWED:
                evt->status = BLE_ERROR_NOT_ALLOWED;
                break;
        default:
                evt->status   = gevt->status;
                break;
        }

        /* Send to event queue */
        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);

        ble_mgr_dev_params_release();
}

void ble_mgr_gap_scan_stop_cmd_handler(void *param)
{
        ble_dev_params_t *ble_dev_params = ble_mgr_dev_params_acquire();
        ble_error_t ret = BLE_ERROR_FAILED;
        ble_mgr_gap_scan_stop_rsp_t *rsp;

        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GAP_SCAN_STOP_CMD, sizeof(*rsp));

        /* Check if a scanning operation is in progress */
        if (!ble_dev_params->scanning) {
                ret = BLE_ERROR_NOT_ALLOWED;
                goto done;
        }

        send_gapm_cancel_cmd();
        ret = BLE_STATUS_OK;
done:
        ble_mgr_dev_params_release();
        rsp->status = ret;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

static bool match_connecting_dev(const device_t *dev, void *ud)
{
        return dev->connecting;
}

void ble_mgr_gapm_connect_cmp_evt_handler(ble_gtl_msg_t *gtl)
{
        ble_evt_gap_connection_completed_t *evt;
        struct gapm_cmp_evt *gevt = (void *) gtl->param;
        ble_dev_params_t *ble_dev_params = ble_mgr_dev_params_acquire();
        device_t *dev;

        /* Reset connecting flag in ble_dev_params */
        ble_dev_params->connecting = false;

        ble_mgr_dev_params_release();

        /* Remove temporary device which was created in connect command */
        if (gevt->status != GAP_ERR_NO_ERROR) {
                storage_acquire();

                dev = find_device(match_connecting_dev, NULL);
                if (dev && !dev->bonded) {
                        device_remove(dev);
                }

                storage_release();
        }

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_GAP_CONNECTION_COMPLETED, sizeof(*evt));

        /* Translate stack status */
        switch (gevt->status) {
        case GAP_ERR_NO_ERROR:
                evt->status = BLE_STATUS_OK;
                break;
        case GAP_ERR_CANCELED:
                evt->status = BLE_ERROR_CANCELED;
                break;
        case GAP_ERR_COMMAND_DISALLOWED:
                evt->status = BLE_ERROR_NOT_ALLOWED;
                break;
        case GAP_ERR_INVALID_PARAM:
                evt->status = BLE_ERROR_INVALID_PARAM;
                break;
        case GAP_ERR_NOT_SUPPORTED:
        case GAP_ERR_PRIVACY_CFG_PB:
                evt->status = BLE_ERROR_NOT_SUPPORTED;
                break;
        case LL_ERR_UNSPECIFIED_ERROR:
                evt->status = BLE_ERROR_INS_BANDWIDTH;
                break;
        default:
                evt->status = gevt->status;
                break;
        }

        /* Send to event queue */
        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
}

void ble_mgr_gap_connect_cmd_exec(void *param)
{
        const ble_mgr_gap_connect_cmd_t *cmd = param;
        ble_mgr_gap_connect_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapm_start_connection_cmd *gcmd;
        ble_dev_params_t *ble_dev_params = ble_mgr_dev_params_acquire();
        device_t *dev;
        ble_error_t ret = BLE_ERROR_FAILED;

        storage_acquire();

        dev = find_device(match_connecting_dev, NULL);
        if (dev) {
                ret = BLE_ERROR_BUSY;
                storage_release();
                goto done;
        }

        /* Check if we are already connected */
        dev = find_device_by_addr(cmd->peer_addr, true);
        if (!dev) {
                storage_release();
                goto done;
        }

        if (dev->connected) {
                ret = BLE_ERROR_ALREADY_DONE;
                storage_release();
                goto done;
        }

        /* Setup GTL message */
        gmsg = ble_gtl_alloc(GAPM_START_CONNECTION_CMD, TASK_ID_GAPM, sizeof(*gcmd) + sizeof(struct gap_bdaddr));
        gcmd = (struct gapm_start_connection_cmd *) gmsg->msg.gtl.param;
        gcmd->op.code = GAPM_CONNECTION_DIRECT;
        switch (ble_dev_params->own_addr.addr_type) {
        case PUBLIC_STATIC_ADDRESS:
        case PRIVATE_STATIC_ADDRESS:
                gcmd->op.addr_src = GAPM_STATIC_ADDR;
                break;
        case PRIVATE_RANDOM_RESOLVABLE_ADDRESS:
                gcmd->op.addr_src = GAPM_GEN_RSLV_ADDR;
                break;
        case PRIVATE_RANDOM_NONRESOLVABLE_ADDRESS:
                gcmd->op.addr_src = GAPM_GEN_NON_RSLV_ADDR;
                break;
#if (dg_configBLE_PRIVACY_1_2 == 1)
        case PRIVATE_CNTL:
                gcmd->op.addr_src = GAPM_GEN_RSLV_ADDR;
                break;
#endif /* (dg_configBLE_PRIVACY_1_2 == 1) */
        }
        gcmd->scan_interval = ble_dev_params->scan_params.interval;
        gcmd->scan_window = ble_dev_params->scan_params.window;
        gcmd->con_intv_min = cmd->conn_params->interval_min;
        gcmd->con_intv_max = cmd->conn_params->interval_max;
        gcmd->con_latency = cmd->conn_params->slave_latency;
        gcmd->superv_to = cmd->conn_params->sup_timeout;
        gcmd->ce_len_min = cmd->ce_len_min ? cmd->ce_len_min : dg_configBLE_CONN_EVENT_LENGTH_MIN;
        gcmd->ce_len_max = cmd->ce_len_max ? cmd->ce_len_max : dg_configBLE_CONN_EVENT_LENGTH_MAX;
        gcmd->nb_peers = 1;
        gcmd->peers[0].addr_type = cmd->peer_addr->addr_type;
        memcpy(&gcmd->peers[0].addr.addr, cmd->peer_addr->addr, sizeof(gcmd->peers[0].addr.addr));

        /* Set connecting flag in ble_dev_params */
        ble_dev_params->connecting = true;
        dev->connecting = true;

        /* Set minimum and maximum connection event length values for connected device */
        dev->ce_len_min = gcmd->ce_len_min;
        dev->ce_len_max = gcmd->ce_len_max;

        storage_release();

        ble_gtl_send(gmsg);

        ret = BLE_STATUS_OK;
done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GAP_CONNECT_CMD, sizeof(*rsp));

        rsp->status = ret;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);

        ble_mgr_dev_params_release();
}

void ble_mgr_gap_connect_cmd_handler(void *param)
{
#if (dg_configBLE_PRIVACY_1_2 == 1)
        ble_mgr_gap_ral_sync(ble_mgr_gap_connect_cmd_exec, param);
#else
        ble_mgr_gap_connect_cmd_exec(param);
#endif /* (dg_configBLE_PRIVACY_1_2 == 1) */
}

void ble_mgr_gap_connect_cancel_cmd_handler(void *param)
{
        ble_mgr_gap_connect_cancel_rsp_t *rsp;
        ble_dev_params_t *ble_dev_params = ble_mgr_dev_params_acquire();
        ble_error_t ret = BLE_ERROR_FAILED;

        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GAP_CONNECT_CANCEL_CMD, sizeof(*rsp));

        /* Check if a connection is in progress */
        if (!ble_dev_params->connecting) {
                ret = BLE_ERROR_NOT_ALLOWED;
                goto done;
        }

        send_gapm_cancel_cmd();
        ret = BLE_STATUS_OK;
done:
        ble_mgr_dev_params_release();
        rsp->status = ret;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gapc_cmp__disconnect_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gapc_cmp_evt *gevt = (void *) gtl->param;
        ble_evt_gap_disconnect_failed_t *evt;

        if (gevt->status == GAP_ERR_NO_ERROR) {
                /* Do nothing; event should be sent only when error occurred */
                return;
        }

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_GAP_DISCONNECT_FAILED, sizeof(*evt));
        evt->conn_idx = TASK_2_CONNIDX(gtl->src_id);

        /* Translate error code */
        switch (gevt->status) {
        case GAP_ERR_INVALID_PARAM:
        case LL_ERR_INVALID_HCI_PARAM:
                evt->status = BLE_ERROR_INVALID_PARAM;
                break;
        case LL_ERR_COMMAND_DISALLOWED:
                evt->status = BLE_ERROR_NOT_ALLOWED;
                break;
        default:
                evt->status = BLE_ERROR_FAILED;
                break;
        }

        /* Send to event queue */
        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
}

static void send_gapc_disconnect_cmd(uint16_t conn_idx, uint8_t reason)
{
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapc_disconnect_cmd *gcmd;

        gmsg = ble_gtl_alloc_with_conn(GAPC_DISCONNECT_CMD, TASK_ID_GAPC, conn_idx, sizeof(*gcmd));
        gcmd = (struct gapc_disconnect_cmd *) gmsg->msg.gtl.param;
        gcmd->operation = GAPC_DISCONNECT;
        gcmd->reason = reason;

        ble_gtl_send(gmsg);
}

void ble_mgr_gap_disconnect_cmd_handler(void *param)
{
        const ble_mgr_gap_disconnect_cmd_t *cmd = param;
        ble_mgr_gap_disconnect_rsp_t *rsp;
        device_t *dev;
        ble_error_t ret = BLE_ERROR_FAILED;

        storage_acquire();

        /* Check if the connection exists */
        dev = find_device_by_conn_idx(cmd->conn_idx);

        storage_release();

        if (!dev) {
                ret = BLE_ERROR_NOT_CONNECTED;
        } else {
                send_gapc_disconnect_cmd(cmd->conn_idx, cmd->reason);
                ret = BLE_STATUS_OK;
        }

        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GAP_DISCONNECT_CMD, sizeof(*rsp));

        rsp->status = ret;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gap_disconnected_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gapc_disconnect_ind *gevt = (void *) gtl->param;
        ble_evt_gap_disconnected_t *evt;
        device_t *dev;
        ble_dev_params_t *ble_dev_params = ble_mgr_dev_params_acquire();


        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_GAP_DISCONNECTED, sizeof(*evt));
        evt->conn_idx = TASK_2_CONNIDX(gtl->src_id);
        evt->reason   = gevt->reason;

        /* Need to notify L2CAP handler so it can 'deallocate' all channels for given conn_idx */
        ble_mgr_l2cap_disconnect_ind(evt->conn_idx);

        storage_acquire();

        dev = find_device_by_conn_idx(evt->conn_idx);
        if (!dev) {
                /* Device not found in storage - ignore event */
                storage_release();
                ble_mgr_dev_params_release();
                OS_FREE(evt);
                return;
        }

        /* Copy peer address to event */
        memcpy(&evt->address, &dev->addr, sizeof(evt->address));

        /*
         * For bonded device we only need to remove non-persistent appvals and mark device as not
         * connected; otherwise remove device from storage
         */
        if (dev->bonded) {
                dev->connected = false;
                dev->encrypted = false;
                dev->sec_level = GAP_SEC_LEVEL_1;
                app_value_remove_np(dev);
        } else {
                device_remove(dev);
        }

        storage_release();

        /* Flush any waitqueue elements corresponding to this connection */
        ble_gtl_waitqueue_flush(evt->conn_idx);

        /* Clear updating flag */
        ble_dev_params->updating = false;

#if (dg_configBLE_SKIP_LATENCY_API == 1)
        ble_mgr_skip_latency_set(evt->conn_idx, false);
#endif /* (dg_configBLE_SKIP_LATENCY_API == 1) */

        /* Send to event queue */
        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);

        ble_mgr_dev_params_release();
}

static void gap_get_con_rssi_rsp(ble_gtl_msg_t *gtl, void *param)
{
        ble_mgr_gap_conn_rssi_get_rsp_t *rsp = param;

        if (gtl != NULL) {
                struct gapc_con_rssi_ind *grsp = (void *) gtl->param;

                rsp->conn_rssi = (int8_t) grsp->rssi;

                rsp->status = BLE_STATUS_OK;
        }
        else {
                rsp->status = BLE_ERROR_NOT_CONNECTED;
        }

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gap_conn_rssi_get_cmd_handler(void *param)
{
        const ble_mgr_gap_conn_rssi_get_cmd_t *cmd = param;
        ble_mgr_gap_conn_rssi_get_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapc_get_info_cmd *gcmd;
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
        gmsg = ble_gtl_alloc_with_conn(GAPC_GET_INFO_CMD, TASK_ID_GAPC, conn_idx, sizeof(*gcmd));
        gcmd = (typeof(gcmd)) gmsg->msg.gtl.param;
        gcmd->operation = GAPC_GET_CON_RSSI;

        ble_msg_free(param);

        /* Create response */
        rsp = ble_msg_init(BLE_MGR_GAP_CONN_RSSI_GET_CMD, sizeof(*rsp));

        ble_gtl_waitqueue_add(conn_idx, GAPC_CON_RSSI_IND, 0,
                              gap_get_con_rssi_rsp, (void *) rsp);

        ble_gtl_send(gmsg);

        return;
done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GAP_CONN_RSSI_GET_CMD, sizeof(*rsp));
        rsp->status = ret;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gap_get_device_info_req_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gapc_get_dev_info_req_ind *gevt = (void *) gtl->param;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapc_get_dev_info_cfm *gcmd;
        ble_dev_params_t *ble_dev_params = ble_mgr_dev_params_acquire();

        /* Send confirmation message to stack */
        gmsg = ble_gtl_alloc(GAPC_GET_DEV_INFO_CFM, gtl->src_id,
                sizeof(*gcmd) + sizeof(ble_dev_params->dev_name));
        gcmd = (struct gapc_get_dev_info_cfm *) gmsg->msg.gtl.param;

        gcmd->req = gevt->req;

        switch (gevt->req) {
        case GAPC_DEV_NAME:
                gcmd->info.name.length = strlen(ble_dev_params->dev_name);
                memcpy(&gcmd->info.name.value, &ble_dev_params->dev_name,
                        strlen(ble_dev_params->dev_name));
                break;
        case GAPC_DEV_APPEARANCE:
                gcmd->info.appearance = ble_dev_params->appearance;
                break;
        case GAPC_DEV_SLV_PREF_PARAMS:
                gcmd->info.slv_params.con_intv_min  = ble_dev_params->gap_ppcp.interval_min;
                gcmd->info.slv_params.con_intv_max  = ble_dev_params->gap_ppcp.interval_max;
                gcmd->info.slv_params.slave_latency = ble_dev_params->gap_ppcp.slave_latency;
                gcmd->info.slv_params.conn_timeout  = ble_dev_params->gap_ppcp.sup_timeout;
                break;
        default:
                /* Do nothing */
                break;
        }

        ble_gtl_send(gmsg);

        /* GAPC_GET_DEV_INFO_CFM does not have a response message, so just send the message */

        ble_mgr_dev_params_release();
}

void ble_mgr_gap_set_device_info_req_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gapc_set_dev_info_req_ind *gevt = (void *) gtl->param;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapc_set_dev_info_cfm *gcmd;
        ble_dev_params_t *ble_dev_params = ble_mgr_dev_params_acquire();

        /* Send confirmation message to stack */
        gmsg = ble_gtl_alloc(GAPC_SET_DEV_INFO_CFM, gtl->src_id, sizeof(*gcmd));
        gcmd = (struct gapc_set_dev_info_cfm *) gmsg->msg.gtl.param;

        gcmd->req = gevt->req;

        switch (gevt->req) {
        case GAPC_DEV_NAME:
                if (gevt->info.name.length > BLE_GAP_DEVNAME_LEN_MAX) {
                        gcmd->status = GAP_ERR_INSUFF_RESOURCES;
                        goto done;
                }
                /* Copy name written by peer device to ble_dev_params */
                memcpy(ble_dev_params->dev_name, &gevt->info.name.value, gevt->info.name.length);
                /* Insert NULL character */
                ble_dev_params->dev_name[gevt->info.name.length] = '\0';
                gcmd->status = GAP_ERR_NO_ERROR;
done:
                ble_gtl_send(gmsg);
                /* GAPC_SET_DEV_INFO_CFM does not have a response message, so just send the message */
                break;
        case GAPC_DEV_APPEARANCE:
                /* Update appearance value in ble_dev_params */
                ble_dev_params->appearance = gevt->info.appearance;
                break;
        default:
                /* Do nothing */
                break;
        }

        ble_mgr_dev_params_release();
}

void ble_mgr_gap_conn_param_update_req_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gapc_param_update_req_ind *gevt = (void *) gtl->param;
        ble_evt_gap_conn_param_update_req_t *evt;
        ble_dev_params_t *params = ble_mgr_dev_params_acquire();

        /* Set updating flag */
        params->updating = true;

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_GAP_CONN_PARAM_UPDATE_REQ, sizeof(*evt));
        evt->conn_idx                  = TASK_2_CONNIDX(gtl->src_id);
        evt->conn_params.interval_min  = gevt->intv_min;
        evt->conn_params.interval_max  = gevt->intv_max;
        evt->conn_params.slave_latency = gevt->latency;
        evt->conn_params.sup_timeout   = gevt->time_out;

        /* Send to event queue */
        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);

        ble_mgr_dev_params_release();
}

void ble_mgr_gap_conn_param_updated_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gapc_param_updated_ind *gevt = (void *) gtl->param;
        ble_evt_gap_conn_param_updated_t *evt;

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_GAP_CONN_PARAM_UPDATED, sizeof(*evt));
        evt->conn_idx                  = TASK_2_CONNIDX(gtl->src_id);
        evt->conn_params.interval_min  = gevt->con_interval;
        evt->conn_params.interval_max  = gevt->con_interval;
        evt->conn_params.slave_latency = gevt->con_latency;
        evt->conn_params.sup_timeout   = gevt->sup_to;

        /* Send to event queue */
        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
}

static void gapm_set_role_rsp(ble_gtl_msg_t *gtl, void *param)
{
        struct gapm_cmp_evt *gevt = (void *) gtl->param;
        ble_mgr_gap_role_set_rsp_t *rsp = param;
        ble_dev_params_t *ble_dev_params = ble_mgr_dev_params_acquire();

        if (gevt->status == GAP_ERR_NO_ERROR) {
                ble_dev_params->role = rsp->new_role;
        }

        switch (gevt->status) {
        case GAP_ERR_NO_ERROR:
                rsp->status = BLE_STATUS_OK;
                break;
        case GAP_ERR_INVALID_PARAM:
                rsp->status = BLE_ERROR_INVALID_PARAM;
                break;
        case GAP_ERR_NOT_SUPPORTED:
                rsp->status = BLE_ERROR_NOT_SUPPORTED;
                break;
        case GAP_ERR_COMMAND_DISALLOWED:
                rsp->status = BLE_ERROR_NOT_ALLOWED;
                break;
        default:
                rsp->status = gevt->status;
                break;
        }

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);

        ble_mgr_dev_params_release();
}

void ble_mgr_gap_role_set_cmd_handler(void *param)
{
        const ble_mgr_gap_role_set_cmd_t *cmd = param;
        ble_mgr_gap_role_set_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapm_set_dev_config_cmd *gcmd;
        ble_dev_params_t *ble_dev_params = ble_mgr_dev_params_acquire();

        /* Setup GTL message */
        gmsg = ble_gap_dev_params_to_gtl(ble_dev_params);
        gcmd = (struct gapm_set_dev_config_cmd *) gmsg->msg.gtl.param;

        /* Translate Dialog API role to stack role */
        gcmd->role = dlg_role_to_gtl_role(cmd->role);

        ble_msg_free(param);

        /* Create response */
        rsp = ble_msg_init(BLE_MGR_GAP_ROLE_SET_CMD, sizeof(*rsp));

        rsp->previous_role = ble_dev_params->role;
        rsp->new_role = dlg_role_from_gtl_role(gcmd->role);

        ble_gtl_waitqueue_add(BLE_CONN_IDX_INVALID, GAPM_CMP_EVT, GAPM_SET_DEV_CONFIG,
                                                                gapm_set_role_rsp, (void *) rsp);
        ble_gtl_send(gmsg);

        ble_mgr_dev_params_release();
}

static void gapm_set_mtu_size_rsp(ble_gtl_msg_t *gtl, void *param)
{
        struct gapm_cmp_evt *gevt = (void *) gtl->param;
        ble_mgr_gap_mtu_size_set_rsp_t *rsp = param;
        ble_dev_params_t *ble_dev_params = ble_mgr_dev_params_acquire();

        if (gevt->status == GAP_ERR_NO_ERROR) {
                ble_dev_params->mtu_size = rsp->new_mtu_size;
        }

        switch (gevt->status) {
        case GAP_ERR_NO_ERROR:
                rsp->status = BLE_STATUS_OK;
                break;
        case GAP_ERR_INVALID_PARAM:
                rsp->status = BLE_ERROR_INVALID_PARAM;
                break;
        case GAP_ERR_NOT_SUPPORTED:
                rsp->status = BLE_ERROR_NOT_SUPPORTED;
                break;
        case GAP_ERR_COMMAND_DISALLOWED:
                rsp->status = BLE_ERROR_NOT_ALLOWED;
                break;
        default:
                rsp->status = gevt->status;
                break;
        }

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);

        ble_mgr_dev_params_release();
}

void ble_mgr_gap_mtu_size_set_cmd_handler(void *param)
{
        const ble_mgr_gap_mtu_size_set_cmd_t *cmd = param;
        ble_mgr_gap_mtu_size_set_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapm_set_dev_config_cmd *gcmd;
        ble_dev_params_t *ble_dev_params = ble_mgr_dev_params_acquire();

        /* Setup GTL message */
        gmsg = ble_gap_dev_params_to_gtl(ble_dev_params);
        gcmd = (struct gapm_set_dev_config_cmd *) gmsg->msg.gtl.param;
        gcmd->max_mtu = cmd->mtu_size;
        gcmd->max_mps = cmd->mtu_size;

        /* Free command buffer */
        ble_msg_free(param);

        /* Create response */
        rsp = ble_msg_init(BLE_MGR_GAP_MTU_SIZE_SET_CMD, sizeof(*rsp));
        rsp->previous_mtu_size = ble_dev_params->mtu_size;
        rsp->new_mtu_size      = gcmd->max_mtu;

        ble_gtl_waitqueue_add(0, GAPM_CMP_EVT, GAPM_SET_DEV_CONFIG,
                              gapm_set_mtu_size_rsp, (void *) rsp);
        ble_gtl_send(gmsg);

        ble_mgr_dev_params_release();
}

static void gapm_set_channel_map_rsp(ble_gtl_msg_t *gtl, void *param)
{
        struct gapm_cmp_evt *gevt = (void *) gtl->param;
        ble_mgr_gap_channel_map_set_cmd_t *cmd = param;
        ble_mgr_gap_channel_map_set_rsp_t *rsp;
        ble_dev_params_t *ble_dev_params = ble_mgr_dev_params_acquire();

        if (gevt->status == GAP_ERR_NO_ERROR) {
                ble_dev_params->channel_map.map[0] = (*cmd->chnl_map);
                ble_dev_params->channel_map.map[1] = (*cmd->chnl_map) >> 8;
                ble_dev_params->channel_map.map[2] = (*cmd->chnl_map) >> 16;
                ble_dev_params->channel_map.map[3] = (*cmd->chnl_map) >> 24;
                ble_dev_params->channel_map.map[4] = (*cmd->chnl_map) >> 32;
        }

        /* Free command buffer */
        ble_msg_free(cmd);

        /* Create response */
        rsp = ble_msg_init(BLE_MGR_GAP_CHANNEL_MAP_SET_CMD, sizeof(*rsp));

        switch (gevt->status) {
        case GAP_ERR_NO_ERROR:
                rsp->status = BLE_STATUS_OK;
                break;
        case GAP_ERR_INVALID_PARAM:
                rsp->status = BLE_ERROR_INVALID_PARAM;
                break;
        case GAP_ERR_NOT_SUPPORTED:
                rsp->status = BLE_ERROR_NOT_SUPPORTED;
                break;
        case GAP_ERR_COMMAND_DISALLOWED:
                rsp->status = BLE_ERROR_NOT_ALLOWED;
                break;
        default:
                rsp->status = gevt->status;
                break;
        }

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);

        ble_mgr_dev_params_release();
}

void ble_mgr_gap_channel_map_set_cmd_handler(void *param)
{
        const ble_mgr_gap_channel_map_set_cmd_t *cmd = param;
        ble_mgr_gap_channel_map_set_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapm_set_channel_map_cmd *gcmd;
        ble_dev_params_t *ble_dev_params = ble_mgr_dev_params_acquire();
        ble_error_t ret = BLE_ERROR_FAILED;

        /* Check if device is central */
        if (!(ble_dev_params->role & GAP_CENTRAL_ROLE)) {
                goto done;
        }

        /* Setup GTL message */
        gmsg = ble_gtl_alloc(GAPM_SET_CHANNEL_MAP_CMD, TASK_ID_GAPM, sizeof(*gcmd));
        gcmd = (struct gapm_set_channel_map_cmd *) gmsg->msg.gtl.param;
        gcmd->operation = GAPM_SET_CHANNEL_MAP;
        gcmd->chmap.map[0] = (*cmd->chnl_map);
        gcmd->chmap.map[1] = (*cmd->chnl_map) >> 8;
        gcmd->chmap.map[2] = (*cmd->chnl_map) >> 16;
        gcmd->chmap.map[3] = (*cmd->chnl_map) >> 24;
        gcmd->chmap.map[4] = (*cmd->chnl_map) >> 32;

        /* 
         * Keep param buffer, we'll need it when creating response.
         * Response message will be allocated in gap_set_channel_map_rsp.
         */
        ble_gtl_waitqueue_add(BLE_CONN_IDX_INVALID, GAPM_CMP_EVT, GAPM_SET_CHANNEL_MAP,
                              gapm_set_channel_map_rsp, (void *) cmd);
        ble_gtl_send(gmsg);

        ble_mgr_dev_params_release();
        return;

done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GAP_CHANNEL_MAP_SET_CMD, sizeof(*rsp));
        rsp->status = ret;

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);

        ble_mgr_dev_params_release();
}

void ble_mgr_gap_conn_param_update_cmd_handler(void *param)
{
        const ble_mgr_gap_conn_param_update_cmd_t *cmd = param;
        ble_mgr_gap_conn_param_update_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapc_param_update_cmd *gcmd;
        ble_error_t ret = BLE_ERROR_FAILED;
        device_t *dev;
        ble_dev_params_t *params = ble_mgr_dev_params_acquire();

        if (params->updating) {
                ret = BLE_ERROR_IN_PROGRESS;
                goto done;
        }
        storage_acquire();

        dev = find_device_by_conn_idx(cmd->conn_idx);
        if (!dev) {
                /* No active connection corresponds to provided index */
                ret = BLE_ERROR_NOT_CONNECTED;
                storage_release();
                goto done;
        }

        /* Setup GTL message */
        gmsg = ble_gtl_alloc_with_conn(GAPC_PARAM_UPDATE_CMD, TASK_ID_GAPC, cmd->conn_idx,
                                                                                     sizeof(*gcmd));
        gcmd = (struct gapc_param_update_cmd *) gmsg->msg.gtl.param;
        gcmd->operation = GAPC_UPDATE_PARAMS;
        gcmd->intv_min = cmd->conn_params->interval_min;
        gcmd->intv_max = cmd->conn_params->interval_max;
        gcmd->latency = cmd->conn_params->slave_latency;
        gcmd->time_out = cmd->conn_params->sup_timeout;

        /* Use currently set minimum and maximum connection event length parameters when master */
        if (dev->master) {
                gcmd->ce_len_min = dev->ce_len_min;
                gcmd->ce_len_max = dev->ce_len_max;
        }

        storage_release();

        /* Set updating flag to prevent starting another update when one is ongoing */
        params->updating = true;

        ble_gtl_send(gmsg);

        ret = BLE_STATUS_OK;

done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GAP_CONN_PARAM_UPDATE_CMD, sizeof(*rsp));
        rsp->status = ret;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
        ble_mgr_dev_params_release();
}

void ble_mgr_gap_conn_param_update_reply_cmd_handler(void *param)
{
        const ble_mgr_gap_conn_param_update_reply_cmd_t *cmd = param;
        ble_mgr_gap_conn_param_update_reply_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapc_param_update_cfm *gcmd;
        ble_error_t ret = BLE_ERROR_FAILED;
        device_t *dev;
        ble_dev_params_t *ble_dev_params = ble_mgr_dev_params_acquire();

        storage_acquire();

        dev = find_device_by_conn_idx(cmd->conn_idx);
        if (!dev) {
                /* No active connection corresponds to provided index */
                ret = BLE_ERROR_NOT_CONNECTED;
                storage_release();
                goto done;
        }

        gmsg = ble_gtl_alloc_with_conn(GAPC_PARAM_UPDATE_CFM, TASK_ID_GAPC, cmd->conn_idx,
                                                                                     sizeof(*gcmd));
        gcmd = (struct gapc_param_update_cfm *) gmsg->msg.gtl.param;

        gcmd->accept = cmd->accept;

        /* Use currently set minimum and maximum connection event length parameters when master */
        if (cmd->accept && dev->master) {
                gcmd->ce_len_min = dev->ce_len_min;
                gcmd->ce_len_max = dev->ce_len_max;
        }

        storage_release();

        ble_gtl_send(gmsg);

        ble_dev_params->updating = false;

        ret = BLE_STATUS_OK;

done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GAP_CONN_PARAM_UPDATE_REPLY_CMD, sizeof(*rsp));
        rsp->status = ret;

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
        ble_mgr_dev_params_release();
}

static uint8_t translate_io_cap(gap_io_cap_t io_cap)
{
        switch (io_cap) {
        case GAP_IO_CAP_DISP_ONLY:
                return GAP_IO_CAP_DISPLAY_ONLY;
        case GAP_IO_CAP_DISP_YES_NO:
                return GAP_IO_CAP_DISPLAY_YES_NO;
        case GAP_IO_CAP_KEYBOARD_ONLY:
                return GAP_IO_CAP_KB_ONLY;
        case GAP_IO_CAP_NO_INPUT_OUTPUT:
                return GAP_IO_CAP_NO_INPUT_NO_OUTPUT;
        case GAP_IO_CAP_KEYBOARD_DISP:
                return GAP_IO_CAP_KB_DISPLAY;
        default:
                return GAP_IO_CAP_NO_INPUT_NO_OUTPUT;
        }
}

#if (dg_configBLE_CENTRAL == 1)
static void send_bond_cmd(uint16_t conn_idx, gap_io_cap_t io_cap, bool bond, bool mitm, bool secure)
{
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapc_bond_cmd *gcmd;

        gmsg = ble_gtl_alloc_with_conn(GAPC_BOND_CMD, TASK_ID_GAPC, conn_idx, sizeof(*gcmd));
        gcmd = (struct gapc_bond_cmd *) gmsg->msg.gtl.param;
        gcmd->operation = GAPC_BOND;
        gcmd->pairing.iocap = translate_io_cap(io_cap);
        gcmd->pairing.oob = GAP_OOB_AUTH_DATA_NOT_PRESENT;
        gcmd->pairing.auth = bond ? GAP_AUTH_BOND : 0;
        gcmd->pairing.auth |= mitm ? GAP_AUTH_MITM : 0;
        gcmd->pairing.auth |= secure ? GAP_AUTH_SEC : 0;
        gcmd->pairing.key_size = 16;
        gcmd->pairing.ikey_dist = dg_configBLE_PAIR_INIT_KEY_DIST;
        gcmd->pairing.rkey_dist = dg_configBLE_PAIR_RESP_KEY_DIST;
        gcmd->pairing.sec_req = GAP_NO_SEC;

        ble_gtl_send(gmsg);
}
#endif /* (dg_configBLE_CENTRAL == 1) */

#if (dg_configBLE_PERIPHERAL == 1)
static void send_security_req(uint16_t conn_idx, bool bond, bool mitm, bool secure)
{
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapc_security_cmd *gcmd;

        gmsg = ble_gtl_alloc_with_conn(GAPC_SECURITY_CMD, TASK_ID_GAPC, conn_idx, sizeof(*gcmd));
        gcmd = (struct gapc_security_cmd *) gmsg->msg.gtl.param;
        gcmd->operation = GAPC_SECURITY_REQ;

        gcmd->auth = bond ? GAP_AUTH_BOND : 0;
        gcmd->auth |= mitm ? GAP_AUTH_MITM : 0;
        gcmd->auth |= secure ? GAP_AUTH_SEC: 0;

        ble_gtl_send(gmsg);
}
#endif /* (dg_configBLE_PERIPHERAL == 1) */

static gap_io_cap_t get_local_io_cap(void)
{
        ble_dev_params_t *ble_dev_params;
        gap_io_cap_t io_cap;

        ble_dev_params = ble_mgr_dev_params_acquire();
        io_cap = ble_dev_params->io_capabilities;
        ble_mgr_dev_params_release();

        return io_cap;
}

static void count_bonded_cb(device_t *dev, void *ud)
{
        int *bonded_count = ud;

        if (dev->bonded) {
                (*bonded_count)++;
        }
}

static int count_bonded(void)
{
        int bonded_count = 0;

        device_foreach(count_bonded_cb, &bonded_count);

        return bonded_count;
}

void ble_mgr_gap_pair_cmd_handler(void *param)
{
        const ble_mgr_gap_pair_cmd_t *cmd = param;
        ble_mgr_gap_pair_rsp_t *rsp;
        ble_error_t status = BLE_ERROR_FAILED;
        gap_io_cap_t io_cap;
        bool master, bonded, paired;
#if (dg_configBLE_SECURE_CONNECTIONS == 1)
        bool secure = true;
#else
        bool secure = false;
#endif /* (dg_configBLE_SECURE_CONNECTIONS == 1) */
        bool bond = cmd->bond;
        uint16_t conn_idx = cmd->conn_idx;
        device_t *dev;

        /* Free command message */
        ble_msg_free(param);

        /* Create response */
        rsp = ble_msg_init(BLE_MGR_GAP_PAIR_CMD, sizeof(*rsp));

        storage_acquire();
        dev = find_device_by_conn_idx(conn_idx);
        if (!dev) {
                storage_release();
                goto done;
        }
        master = dev->master;
        bonded = dev->bonded;
        paired = dev->paired;
        storage_release();

        /* Get local IO capabilities */
        io_cap = get_local_io_cap();

        /* We allow to overwrite old keys with new bonding */
        if ((!bond && (paired || bonded))) {
                status = BLE_ERROR_ALREADY_DONE;
                goto done;
        }

        /* Don't exceed the max bonded devices threshold */
        if (bond && !bonded && (count_bonded() >= BLE_GAP_MAX_BONDED)) {
                status = BLE_ERROR_INS_RESOURCES;
                goto done;
        }

        if (master) {
#if (dg_configBLE_CENTRAL == 1)
                send_bond_cmd(conn_idx, io_cap, bond,
                                       io_cap == GAP_IO_CAP_NO_INPUT_OUTPUT ? false : true, secure);
                status = BLE_STATUS_OK;
                goto done;
#endif /* (dg_configBLE_CENTRAL == 1) */
        } else {
#if (dg_configBLE_PERIPHERAL == 1)
                send_security_req(conn_idx, bond,
                                       io_cap == GAP_IO_CAP_NO_INPUT_OUTPUT ? false : true, secure);
                status = BLE_STATUS_OK;
                goto done;
#endif /* (dg_configBLE_PERIPHERAL == 1) */
        }

done:
        rsp->status = status;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gap_bond_req_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gapc_bond_req_ind *ind = (void *) &gtl->param;
        device_t *dev;

        switch (ind->request) {
        case GAPC_PAIRING_REQ:
        {
                ble_evt_gap_pair_req_t *evt;

                evt = ble_evt_init(BLE_EVT_GAP_PAIR_REQ, sizeof(*evt));
                evt->conn_idx = TASK_2_CONNIDX(gtl->src_id);
                evt->bond = (ind->data.auth_req & GAP_AUTH_BOND);

#if (dg_configBLE_SECURE_CONNECTIONS == 1)
                if (ind->data.auth_req & GAP_AUTH_SEC) {
                        storage_acquire();
                        dev = find_device_by_conn_idx(TASK_2_CONNIDX(gtl->src_id));
                        if (dev) {
                                dev->secure = true;
                        }
                        storage_release();
                }
#endif /* (dg_configBLE_SECURE_CONNECTIONS == 1) */

                /* Send to event queue */
                ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);

                break;
        }
        case GAPC_LTK_EXCH:
        {
                ble_mgr_common_stack_msg_t *gmsg;
                struct gapc_bond_cfm *gcmd;
                int i;

                gmsg = ble_gtl_alloc(GAPC_BOND_CFM, gtl->src_id, sizeof(*gcmd));
                gcmd = (struct gapc_bond_cfm *) gmsg->msg.gtl.param;

                gcmd->accept = 0x01;
                gcmd->request = GAPC_LTK_EXCH;

                gcmd->data.ltk.ediv = rand();
                gcmd->data.ltk.key_size = ind->data.key_size;

                for (i = 0; i < RAND_NB_LEN; i++) {
                    gcmd->data.ltk.randnb.nb[i] = rand();
                }

                for (i = 0; i < ind->data.key_size; i++) {
                    gcmd->data.ltk.ltk.key[i] = rand();
                }

                storage_acquire();
                dev = find_device_by_conn_idx(TASK_2_CONNIDX(gtl->src_id));
                if (dev) {
                        struct gapc_ltk *gltk = &gcmd->data.ltk;
                        key_ltk_t *ltk = dev->ltk;

                        if (!ltk) {
                                ltk = OS_MALLOC(sizeof(*ltk));
                                dev->ltk = ltk;
                        }

                        ltk->key_size = gltk->key_size;
                        memcpy(&ltk->rand, gltk->randnb.nb, sizeof(ltk->rand));
                        ltk->ediv = gltk->ediv;
                        memcpy(ltk->key, gltk->ltk.key, sizeof(ltk->key));


                        /* Storage will be written back to flash on pairing complete */
                        storage_mark_dirty(false);
                }
                storage_release();

                ble_gtl_send(gmsg);

                break;
        }
        case GAPC_CSRK_EXCH:
        {
                ble_mgr_common_stack_msg_t *gmsg;
                struct gapc_bond_cfm *gcmd;
                int i;

                gmsg = ble_gtl_alloc(GAPC_BOND_CFM, gtl->src_id, sizeof(*gcmd));
                gcmd = (struct gapc_bond_cfm *) gmsg->msg.gtl.param;

                gcmd->accept = 0x01;
                gcmd->request = GAPC_CSRK_EXCH;

                for (i = 0; i < KEY_LEN; i++) {
                        gcmd->data.csrk.key[i] = rand();
                }

                storage_acquire();
                dev = find_device_by_conn_idx(TASK_2_CONNIDX(gtl->src_id));
                if (dev) {
                        key_csrk_t *csrk = dev->csrk;

                        if (!csrk) {
                                csrk = OS_MALLOC(sizeof(*csrk));
                                dev->csrk = csrk;
                        }

                        memcpy(csrk->key, gcmd->data.csrk.key, sizeof(csrk->key));
                        csrk->sign_cnt = 0;


                        /* Storage will be written back to flash on pairing complete */
                        storage_mark_dirty(false);
                }
                storage_release();

                ble_gtl_send(gmsg);

                break;
        }
        case GAPC_TK_EXCH:
        {
                if (ind->data.tk_type == GAP_TK_DISPLAY) {
                        ble_mgr_common_stack_msg_t *gmsg;
                        struct gapc_bond_cfm *gcmd;
                        ble_evt_gap_passkey_notify_t *evt;
                        uint32_t passkey;

                        gmsg = ble_gtl_alloc(GAPC_BOND_CFM, gtl->src_id, sizeof(*gcmd));
                        gcmd = (struct gapc_bond_cfm *) gmsg->msg.gtl.param;

                        gcmd->accept = 0x01;
                        gcmd->request = GAPC_TK_EXCH;

                        /* Generate passkey */
                        passkey = rand() % 1000000;

                        gcmd->data.tk.key[0] = passkey;
                        gcmd->data.tk.key[1] = passkey >> 8;
                        gcmd->data.tk.key[2] = passkey >> 16;
                        gcmd->data.tk.key[3] = passkey >> 24;

                        /* Send confirmation to stack */
                        ble_gtl_send(gmsg);

                        evt = ble_evt_init(BLE_EVT_GAP_PASSKEY_NOTIFY, sizeof(*evt));
                        evt->conn_idx = TASK_2_CONNIDX(gtl->src_id);
                        evt->passkey = passkey;

                        /* Send notification to app */
                        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
                } else if (ind->data.tk_type == GAP_TK_KEY_ENTRY) {
                        ble_evt_gap_passkey_request_t *evt;

                        evt = ble_evt_init(BLE_EVT_GAP_PASSKEY_REQUEST, sizeof(*evt));
                        evt->conn_idx = TASK_2_CONNIDX(gtl->src_id);

                        /* Send request to app */
                        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
#if (dg_configBLE_SECURE_CONNECTIONS == 1)
                } else if (ind->data.tk_type == GAP_TK_KEY_CONFIRM) {
                        ble_evt_gap_numeric_request_t *evt;

                        evt = ble_evt_init(BLE_EVT_GAP_NUMERIC_REQUEST, sizeof(*evt));
                        evt->conn_idx = TASK_2_CONNIDX(gtl->src_id);
                        evt->num_key = ind->tk.key[0];
                        evt->num_key += ind->tk.key[1] << 8;
                        evt->num_key += ind->tk.key[2] << 16;
                        evt->num_key += ind->tk.key[3] << 24;
                        evt->num_key %= 1000000;

                        /* Send request to app */
                        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
#endif /* (dg_configBLE_SECURE_CONNECTIONS == 1) */
                }

                break;
        }
        default:
                return;
        }
}

void ble_mgr_gap_pair_reply_cmd_handler(void *param)
{
        const ble_mgr_gap_pair_reply_cmd_t *cmd = param;
        ble_mgr_gap_pair_reply_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapc_bond_cfm *gcmd;
        gap_io_cap_t io_cap;
        device_t *dev;
        bool bonded;
        ble_error_t status = BLE_ERROR_FAILED;

        storage_acquire();
        dev = find_device_by_conn_idx(cmd->conn_idx);
        if (!dev) {
                storage_release();
                status = BLE_ERROR_NOT_CONNECTED;
                goto reply;
        }

        bonded = dev->bonded;
        storage_release();

        /* Don't exceed the max bonded devices threshold */
        if (cmd->bond && cmd->accept && !bonded && (count_bonded() >= BLE_GAP_MAX_BONDED)) {
                status = BLE_ERROR_INS_RESOURCES;
                goto reply;
        }

        /* Get local IO capabilities */
        io_cap = get_local_io_cap();

        gmsg = ble_gtl_alloc_with_conn(GAPC_BOND_CFM, TASK_ID_GAPC, cmd->conn_idx, sizeof(*gcmd));
        gcmd = (struct gapc_bond_cfm *) gmsg->msg.gtl.param;

        gcmd->request = GAPC_PAIRING_RSP;
        gcmd->accept = cmd->accept;

        if (!cmd->accept) {
                goto done;
        }

        gcmd->data.pairing_feat.auth = cmd->bond ? GAP_AUTH_BOND : 0;
        gcmd->data.pairing_feat.auth |= (io_cap != GAP_IO_CAP_NO_INPUT_OUTPUT) ? GAP_AUTH_MITM : 0;
#if (dg_configBLE_SECURE_CONNECTIONS == 1)
        gcmd->data.pairing_feat.auth |= GAP_AUTH_SEC;
#endif /* (dg_configBLE_SECURE_CONNECTIONS == 1) */

        gcmd->data.pairing_feat.oob = GAP_OOB_AUTH_DATA_NOT_PRESENT;
        gcmd->data.pairing_feat.key_size = KEY_LEN;
        gcmd->data.pairing_feat.iocap = translate_io_cap(io_cap);
        gcmd->data.pairing_feat.ikey_dist = dg_configBLE_PAIR_INIT_KEY_DIST;
        gcmd->data.pairing_feat.rkey_dist = dg_configBLE_PAIR_RESP_KEY_DIST;
        gcmd->data.pairing_feat.sec_req = GAP_NO_SEC;

done:
        ble_gtl_send(gmsg);
        status = BLE_STATUS_OK;

reply:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GAP_PAIR_REPLY_CMD, sizeof(*rsp));
        rsp->status = status;

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gap_passkey_reply_cmd_handler(void *param)
{
        const ble_mgr_gap_passkey_reply_cmd_t *cmd = param;
        ble_mgr_gap_passkey_reply_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapc_bond_cfm *gcmd;

        gmsg = ble_gtl_alloc_with_conn(GAPC_BOND_CFM, TASK_ID_GAPC, cmd->conn_idx, sizeof(*gcmd));
        gcmd = (struct gapc_bond_cfm *) gmsg->msg.gtl.param;

        gcmd->request = GAPC_TK_EXCH;
        gcmd->accept = cmd->accept;

        if (!cmd->accept) {
                goto done;
        }

        gcmd->data.tk.key[0] = cmd->passkey;
        gcmd->data.tk.key[1] = cmd->passkey >> 8;
        gcmd->data.tk.key[2] = cmd->passkey >> 16;
        gcmd->data.tk.key[3] = cmd->passkey >> 24;

done:
        ble_gtl_send(gmsg);

        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GAP_PASSKEY_REPLY_CMD, sizeof(*rsp));
        rsp->status = BLE_STATUS_OK;

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

#if (dg_configBLE_SECURE_CONNECTIONS == 1)
void ble_mgr_gap_numeric_reply_cmd_handler(void *param)
{
        const ble_mgr_gap_numeric_reply_cmd_t *cmd = param;
        ble_mgr_gap_numeric_reply_rsp_t *rsp;
        static ble_mgr_common_stack_msg_t *gmsg;
        struct gapc_bond_cfm *gcmd;

        gmsg = ble_gtl_alloc_with_conn(GAPC_BOND_CFM, TASK_ID_GAPC, cmd->conn_idx, sizeof(*gcmd));
        gcmd = (struct gapc_bond_cfm *) gmsg->msg.gtl.param;

        gcmd->request = GAPC_TK_EXCH;
        gcmd->accept = cmd->accept;

        ble_gtl_send(gmsg);

        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GAP_NUMERIC_REPLY_CMD, sizeof(*rsp));
        rsp->status = BLE_STATUS_OK;

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}
#endif /* (dg_configBLE_SECURE_CONNECTIONS == 1) */

static void send_sec_level_changed_evt(uint16_t conn_idx, gap_sec_level_t level)
{
        ble_evt_gap_sec_level_changed_t *evt;

        evt = ble_evt_init(BLE_EVT_GAP_SEC_LEVEL_CHANGED, sizeof(*evt));
        evt->conn_idx = conn_idx;
        evt->level = level;

        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
}

#if ( (dg_configBLE_SECURE_CONNECTIONS == 1) && (RWBLE_SW_VERSION_MINOR >= 1) )
static void public_key_renew(void)
{
        static ble_mgr_common_stack_msg_t *gmsg;
        struct gapm_reset_cmd *gcmd;

        /* Send GAPM_RESET_CMD command with GAPM_KEY_RENEW OP code to the BLE stack */
        gmsg = ble_gtl_alloc(GAPM_RESET_CMD, TASK_ID_GAPM, sizeof(*gcmd));
        gcmd = (struct gapm_reset_cmd *) gmsg->msg.gtl.param;
        gcmd->operation = GAPM_KEY_RENEW;

        ble_gtl_send(gmsg);
}
#endif /* (dg_configBLE_SECURE_CONNECTIONS == 1) */

void ble_mgr_gap_bond_ind_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gapc_bond_ind *ind = (void *) &gtl->param;
        device_t *dev;

        switch (ind->info) {
        case GAPC_PAIRING_SUCCEED:
        {
                ble_evt_gap_pair_completed_t *evt;

#if ( (dg_configBLE_SECURE_CONNECTIONS == 1) && (RWBLE_SW_VERSION_MINOR >= 1) )
                ble_dev_params_t *params = ble_mgr_dev_params_acquire();

                /* Increase successful pairings counter */
                params->pairing_successes++;
                if (params->pairing_successes == dg_configBLE_PUB_KEY_SUCCESS_THR) {
                        /* Renew public key and reset counters */
                        public_key_renew();
                        params->pairing_successes = 0;
                        params->pairing_failures = 0;
                }
                ble_mgr_dev_params_release();
#endif /* ( (dg_configBLE_SECURE_CONNECTIONS == 1) && (RWBLE_SW_VERSION_MINOR >= 1) ) */

                evt = ble_evt_init(BLE_EVT_GAP_PAIR_COMPLETED, sizeof(*evt));
                evt->conn_idx = TASK_2_CONNIDX(gtl->src_id);
                evt->status = BLE_STATUS_OK;
                evt->bond = ind->data.auth & GAP_AUTH_BOND;
                evt->mitm = ind->data.auth & GAP_AUTH_MITM;

                /* Store authentication requirements in pairing data */
                storage_acquire();
                dev = find_device_by_conn_idx(TASK_2_CONNIDX(gtl->src_id));
                if (dev) {
                        gap_sec_level_t sec_level = auth_2_sec_level(ind->data.auth);
                        dev->paired = true;
                        dev->bonded = evt->bond;
                        dev->encrypted = true;
                        dev->mitm = evt->mitm;
#if (dg_configBLE_SECURE_CONNECTIONS == 1)
                        dev->secure = ind->data.auth & GAP_AUTH_SEC ? true : false;
#endif /* (dg_configBLE_SECURE_CONNECTIONS == 1) */

                        /* Check if the security level has changed */
                        if (dev->sec_level != sec_level) {
                                /* Update device security level */
                                dev->sec_level = sec_level;

                                /* Send security level changed event */
                                send_sec_level_changed_evt(evt->conn_idx, sec_level);
                        }

                        if (dev->bonded) {
                                /* Move device to the front of the connected devices list */
                                device_move_front(dev);
                        }

#if (dg_configBLE_PRIVACY_1_2 == 1)
                        ble_dev_params_t *ble_dev_params = ble_mgr_dev_params_acquire();
                        if (ble_dev_params->own_addr.addr_type == PRIVATE_CNTL) {
                                ble_dev_params->prev_privacy_operation = BLE_MGR_RAL_OP_NONE;
                        }
                        ble_mgr_dev_params_release();
#endif /* (dg_configBLE_PRIVACY_1_2 == 1) */
                }

                /* Write storage back to flash immediately */
                storage_mark_dirty(true);

                storage_release();

                /* Send to event queue */
                ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);

                break;
        }
        case GAPC_PAIRING_FAILED:
        {
                ble_evt_gap_pair_completed_t *evt;

#if ( (dg_configBLE_SECURE_CONNECTIONS == 1) && (RWBLE_SW_VERSION_MINOR >= 1) )
                ble_dev_params_t *params = ble_mgr_dev_params_acquire();

                /* Increase failed pairings counter */
                params->pairing_failures++;
                if (params->pairing_failures == dg_configBLE_PUB_KEY_FAILURE_THR) {
                        /* Renew public key and reset counters */
                        public_key_renew();
                        params->pairing_successes = 0;
                        params->pairing_failures = 0;
                }
                ble_mgr_dev_params_release();

                storage_acquire();
                dev = find_device_by_conn_idx(TASK_2_CONNIDX(gtl->src_id));
                if (dev) {
                        /* Reset secure flag */
                        dev->secure = false;
                }

                /* Write storage back to flash immediately */
                storage_mark_dirty(true);

                storage_release();
#endif /* ( (dg_configBLE_SECURE_CONNECTIONS == 1) && (RWBLE_SW_VERSION_MINOR >= 1) ) */

                evt = ble_evt_init(BLE_EVT_GAP_PAIR_COMPLETED, sizeof(*evt));
                evt->conn_idx = TASK_2_CONNIDX(gtl->src_id);

                switch (ind->data.reason) {
                case SMP_ERROR_REM_PAIRING_NOT_SUPP:
                        evt->status = BLE_ERROR_NOT_SUPPORTED_BY_PEER;
                        break;
                default:
                        evt->status = BLE_ERROR_FAILED;
                        break;
                };
                evt->bond = false;
                evt->mitm = false;

                /* Send to event queue */
                ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);

                break;
        }
        case GAPC_LTK_EXCH:
                storage_acquire();
                dev = find_device_by_conn_idx(TASK_2_CONNIDX(gtl->src_id));
                if (dev) {
                        struct gapc_ltk *gltk = &ind->data.ltk;
                        key_ltk_t *ltk = dev->remote_ltk;

                        if (!ltk) {
                                ltk = OS_MALLOC(sizeof(*ltk));
                                dev->remote_ltk = ltk;
                        }

                        ltk->key_size = gltk->key_size;
                        memcpy(&ltk->rand, gltk->randnb.nb, sizeof(ltk->rand));
                        ltk->ediv = gltk->ediv;
                        memcpy(ltk->key, gltk->ltk.key, sizeof(ltk->key));


                        /* Storage will be written back to flash on pairing complete */
                        storage_mark_dirty(false);
                }
                storage_release();
                break;
        case GAPC_CSRK_EXCH:
                storage_acquire();
                dev = find_device_by_conn_idx(TASK_2_CONNIDX(gtl->src_id));
                if (dev) {
                        key_csrk_t *csrk = dev->remote_csrk;

                        if (!csrk) {
                                csrk = OS_MALLOC(sizeof(*csrk));
                                dev->remote_csrk = csrk;
                        }

                        memcpy(csrk->key, ind->data.csrk.key, sizeof(csrk->key));
                        csrk->sign_cnt = 0;


                        /* Storage will be written back to flash on pairing complete */
                        storage_mark_dirty(false);
                }
                storage_release();
                break;
        case GAPC_IRK_EXCH:
        {
                ble_evt_gap_address_resolved_t *evt;

                storage_acquire();
                dev = find_device_by_conn_idx(TASK_2_CONNIDX(gtl->src_id));
                if (dev) {
                        key_irk_t *irk = dev->irk;

                        bd_address_t addr;
                        device_t *old_dev;

                        /* Remove any other device record with the same address but an older IRK */
                        addr.addr_type = ind->data.irk.addr.addr_type;
                        memcpy(addr.addr, ind->data.irk.addr.addr.addr, sizeof(addr.addr));
                        while (((old_dev = find_device_by_addr(&addr, false)) != NULL) &&
                                (old_dev != dev)) {
                                device_remove(old_dev);
                        }

                        evt = ble_evt_init(BLE_EVT_GAP_ADDRESS_RESOLVED, sizeof(*evt));

                        if (!irk) {
                                irk = OS_MALLOC(sizeof(*irk));
                                dev->irk = irk;
                        }

                        memcpy(irk->key, ind->data.irk.irk.key, sizeof(irk->key));

                        memcpy(&evt->address, &dev->addr, sizeof(evt->address));
                        dev->addr.addr_type = ind->data.irk.addr.addr_type;
                        memcpy(&dev->addr, &addr, sizeof(dev->addr));
                        memcpy(&evt->resolved_address, &dev->addr, sizeof(evt->resolved_address));
                        evt->conn_idx = TASK_2_CONNIDX(gtl->src_id);

                        /* Send to event queue */
                        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);

                        /* Storage will be written back to flash on pairing complete */
                        storage_mark_dirty(false);
                }
                storage_release();
                break;
        }
        default:
                break;
        }
}

static void gapc_encrypt_complete(ble_gtl_msg_t *gtl, void *param)
{
        struct gapc_cmp_evt *gevt = gtl ? (void *) gtl->param : NULL;
        ble_evt_hdr_t *msg = param;

        /* Check current operation */
        if (msg->evt_code == BLE_EVT_GAP_SET_SEC_LEVEL_FAILED) {
                ble_evt_gap_set_sec_level_failed_t *evt =
                        (ble_evt_gap_set_sec_level_failed_t *) msg;

                /* Check if callback was called by ble_gtl_waitqueue_flush() */
                if (gtl != NULL) {
                        switch (gevt->status) {
                        case GAP_ERR_NO_ERROR:
                                /*
                                 * Encryption was successful, BLE_EVT_GAP_SET_SEC_LEVEL_FAILED event
                                 * should be freed and should not be sent to application.
                                 * BLE_EVT_GAP_SEC_LEVEL_CHANGED event will be sent instead upon
                                 * reception of GAPC_ENCRYPT_IND message.
                                 */
                                ble_msg_free(param);
                                return;
                        /* Translate error codes */
                        case SMP_ERROR_REM_ENC_KEY_MISSING:
                        case SMP_ERROR_ENC_KEY_MISSING:
                                evt->status = BLE_ERROR_ENC_KEY_MISSING;
                                break;
                        default:
                                evt->status = BLE_ERROR_FAILED;
                                break;
                        }
                } else {
                        /* Called by ble_gtl_waitqueue_flush(), link was disconnected */
                        evt->status = BLE_ERROR_NOT_CONNECTED;
                }
        } else if (msg->evt_code == BLE_EVT_GAP_SECURITY_REQUEST) {
                if ((gtl == NULL) || (gevt->status == GAP_ERR_NO_ERROR)) {
                        /*
                         * Either link was disconnected or encryption was successful. In any case,
                         * security request has been handled internally, so there's no need to
                         * notify the application. Free event buffer and return.
                         */
                        ble_msg_free(param);
                        return;
                }
        } else {
                /* No other event code is expected */
                ASSERT_ERROR(0);
        }

        /* Encryption has failed, send event to application */
        ble_mgr_event_queue_send(&param, OS_QUEUE_FOREVER);
}

static bool encrypt_conn_using_ltk(uint16_t conn_idx, uint8_t auth, void *param)
{
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapc_encrypt_cmd *gcmd;
        device_t *dev;

        storage_acquire();
        dev = find_device_by_conn_idx(conn_idx);
        if (!dev || !dev->remote_ltk) {
                storage_release();
                return false;
        }

        if ((auth & GAP_AUTH_MITM) && !dev->mitm) {
                storage_release();
                return false;
        }

        gmsg = ble_gtl_alloc_with_conn(GAPC_ENCRYPT_CMD, TASK_ID_GAPC, conn_idx, sizeof(*gcmd));
        gcmd = (struct gapc_encrypt_cmd *) gmsg->msg.gtl.param;

        gcmd->operation = GAPC_ENCRYPT;
        gcmd->ltk.ediv = dev->remote_ltk->ediv;
        gcmd->ltk.key_size = dev->remote_ltk->key_size;
        memcpy(&gcmd->ltk.ltk.key, dev->remote_ltk->key, sizeof(gcmd->ltk.ltk.key));
        memcpy(&gcmd->ltk.randnb.nb, &dev->remote_ltk->rand, sizeof(gcmd->ltk.randnb.nb));

        storage_release();

        ble_gtl_waitqueue_add(conn_idx, GAPC_CMP_EVT, GAPC_ENCRYPT, gapc_encrypt_complete, param);
        ble_gtl_send(gmsg);

        return true;
}

void ble_mgr_gap_security_ind_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gapc_security_ind *ind = (void *) &gtl->param;
        uint16_t conn_idx = TASK_2_CONNIDX(gtl->src_id);
        ble_evt_gap_security_request_t *evt;

        evt = ble_evt_init(BLE_EVT_GAP_SECURITY_REQUEST, sizeof(*evt));
        evt->conn_idx = conn_idx;
        evt->bond = ind->auth & GAP_AUTH_BOND;
        evt->mitm = ind->auth & GAP_AUTH_MITM;

        if (!encrypt_conn_using_ltk(conn_idx, ind->auth, evt)) {
                /* Send to event queue */
                ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
        }
}

void ble_mgr_gap_sign_counter_ind_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gapc_sign_counter_ind *ind = (void *) &gtl->param;
        device_t *dev;

        storage_acquire();
        dev = find_device_by_conn_idx(TASK_2_CONNIDX(gtl->src_id));
        if (dev) {
                /* Local and remote CSRKs should exist for this device */
                OS_ASSERT(dev->csrk);
                OS_ASSERT(dev->remote_csrk);

                dev->csrk->sign_cnt = ind->local_sign_counter;
                dev->remote_csrk->sign_cnt = ind->peer_sign_counter;
        }
        storage_release();
}

static void send_bonding_info_miss_evt(uint16_t conn_idx)
{
        ble_evt_gap_ltk_missing_t *evt;

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_GAP_LTK_MISSING, sizeof(*evt));
        evt->conn_idx = conn_idx;

        /* Send to event queue */
        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
}

void ble_mgr_gap_encrypt_req_ind_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gapc_encrypt_req_ind *ind = (void *) &gtl->param;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapc_encrypt_cfm *gcmd;
        device_t *dev;

        gmsg = ble_gtl_alloc(GAPC_ENCRYPT_CFM, gtl->src_id, sizeof(*gcmd));
        gcmd = (struct gapc_encrypt_cfm *) gmsg->msg.gtl.param;
        gcmd->found = 0x00;

        storage_acquire();

        dev = find_device_by_conn_idx(TASK_2_CONNIDX(gtl->src_id));

        if (!dev) {
                goto done;
        }
#if (dg_configBLE_SECURE_CONNECTIONS == 1)
        if ((!dev->bonded) || (!ind->ediv)) {
                if (!dev->remote_ltk) {
                        goto done;
                }

                memcpy(&gcmd->ltk.key, dev->remote_ltk->key, sizeof(gcmd->ltk.key));
        } else
#endif /* (dg_configBLE_SECURE_CONNECTIONS == 1) */
        {
                if (!dev->ltk) {
                        goto done;
                }

                if (dev->ltk->ediv != ind->ediv) {
                        goto done;
                }

                // Our Rand is stored in the same endianess as RW
                if (memcmp(&dev->ltk->rand, &ind->rand_nb.nb, sizeof(dev->ltk->rand))) {
                        goto done;
                }

                memcpy(&gcmd->ltk.key, dev->ltk->key, sizeof(gcmd->ltk.key));
        }

        gcmd->found = 0x01;
        gcmd->key_size = dev->ltk->key_size;

done:
        /* Send key missing event to application */
        if ( dev && ( gcmd->found == 0x00 ) ) {
                send_bonding_info_miss_evt(dev->conn_idx);
        }

        storage_release();
        ble_gtl_send(gmsg);
}

void ble_mgr_gap_encrypt_ind_evt_handler(ble_gtl_msg_t *gtl)
{
        device_t *dev;

        storage_acquire();

        dev = find_device_by_conn_idx(TASK_2_CONNIDX(gtl->src_id));
        if (dev) {
                struct gapc_encrypt_ind *ind = (void *) &gtl->param;

                dev->encrypted = true;

                /* Check if the security level has changed (if 0x00, wait for pairing completed) */
                if (dev->paired && (dev->sec_level != auth_2_sec_level(ind->auth))
                        && (ind->auth != 0x00)) {
                        /* Update device security level */
                        dev->sec_level = auth_2_sec_level(ind->auth);

                        /* Send security level changed event */
                        send_sec_level_changed_evt(TASK_2_CONNIDX(gtl->src_id),
                                                                       auth_2_sec_level(ind->auth));
                }
        }

        storage_release();
}

void ble_mgr_gapc_cmp__update_params_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gapc_cmp_evt *gevt = (void *) gtl->param;
        ble_evt_gap_conn_param_update_completed_t *evt;
        ble_dev_params_t *params = ble_mgr_dev_params_acquire();

        /* Reset updating flag, update is completed */
        params->updating = false;

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_GAP_CONN_PARAM_UPDATE_COMPLETED, sizeof(*evt));
        evt->conn_idx = TASK_2_CONNIDX(gtl->src_id);

        switch (gevt->status) {
        case GAP_ERR_NO_ERROR:
                evt->status = BLE_STATUS_OK;
                break;
        case GAP_ERR_INVALID_PARAM:
        case LL_ERR_INVALID_HCI_PARAM:
                evt->status = BLE_ERROR_INVALID_PARAM;
                break;
        case GAP_ERR_TIMEOUT:
                evt->status = BLE_ERROR_TIMEOUT;
                break;
        case GAP_ERR_REJECTED:
                evt->status = BLE_ERROR_NOT_ACCEPTED;
                break;
        case LL_ERR_COMMAND_DISALLOWED:
                evt->status = BLE_ERROR_NOT_ALLOWED;
                break;
        case LL_ERR_UNKNOWN_HCI_COMMAND:
        case LL_ERR_UNSUPPORTED:
        case LL_ERR_UNKNOWN_LMP_PDU:
        case LL_ERR_UNSUPPORTED_LMP_PARAM_VALUE:
                evt->status = BLE_ERROR_NOT_SUPPORTED;
                break;
        case LL_ERR_UNSUPPORTED_REMOTE_FEATURE:
                evt->status = BLE_ERROR_NOT_SUPPORTED_BY_PEER;
                break;
        default:
                evt->status = BLE_ERROR_FAILED;
                break;
        }

        /* Send to event queue */
        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);

        ble_mgr_dev_params_release();
}

void ble_mgr_gapc_cmp__bond_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gapm_cmp_evt *gevt = (void *) gtl->param;
        ble_evt_gap_pair_completed_t *evt;

        if (gevt->status == GAP_ERR_NO_ERROR) {
                /* Nothing to do, we replied in GAPC_BOND_IND handler */
                return;
        }

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_GAP_PAIR_COMPLETED, sizeof(*evt));
        evt->conn_idx = TASK_2_CONNIDX(gtl->src_id);
        evt->status = gevt->status;
        evt->bond = false;
        evt->mitm = false;

        /* Send to event queue */
        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
}

void ble_mgr_gap_unpair_cmd_handler(void *param)
{
        const ble_mgr_gap_unpair_cmd_t *cmd = param;
        uint8_t status = BLE_ERROR_FAILED;
        ble_mgr_gap_unpair_rsp_t *rsp;
        device_t *dev;

        /* Remove pairing info if we are not bonded */
        storage_acquire();

        dev = find_device_by_addr(&cmd->addr, false);
        if (!dev) {
                goto done;
        }

        device_remove_pairing(dev);
        status = BLE_STATUS_OK;

#if (dg_configBLE_PRIVACY_1_2 == 1)
        ble_dev_params_t *ble_dev_params = ble_mgr_dev_params_acquire();
        if (ble_dev_params->own_addr.addr_type == PRIVATE_CNTL) {
                ble_dev_params->prev_privacy_operation = BLE_MGR_RAL_OP_NONE;
        }
        ble_mgr_dev_params_release();
#endif /* (dg_configBLE_PRIVACY_1_2 == 1) */

        if (!dev->connected) {
                device_remove(dev);
                goto done;
        }

        send_gapc_disconnect_cmd(dev->conn_idx, BLE_HCI_ERROR_REMOTE_USER_TERM_CON);

done:
        storage_release();
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GAP_UNPAIR_CMD, sizeof(*rsp));
        rsp->status = status;

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gap_set_sec_level_cmd_handler(void *param)
{
        const ble_mgr_gap_set_sec_level_cmd_t *cmd = param;
        ble_mgr_gap_set_sec_level_rsp_t *rsp;
        gap_sec_level_t level = cmd->level;
        bool mitm = level > GAP_SEC_LEVEL_2 ? true : false;
        uint16_t conn_idx = cmd->conn_idx;
        ble_dev_params_t *ble_dev_params;
        gap_io_cap_t io_cap;
        ble_error_t status = BLE_ERROR_FAILED;
        device_t *dev;
        bool bonded, master;
        bool secure = false;

        (void) io_cap;

        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GAP_SET_SEC_LEVEL_CMD, sizeof(*rsp));

        if (level == GAP_SEC_LEVEL_4) {
#if (dg_configBLE_SECURE_CONNECTIONS == 1)
                secure = true;
#else
                status = BLE_ERROR_NOT_SUPPORTED;
                goto done;
#endif /* (dg_configBLE_SECURE_CONNECTIONS == 1) */
        }

        ble_dev_params = ble_mgr_dev_params_acquire();
        io_cap = ble_dev_params->io_capabilities;
        ble_mgr_dev_params_release();

        storage_acquire();
        dev = find_device_by_conn_idx(conn_idx);
        if (!dev) {
                storage_release();
                status = BLE_ERROR_NOT_CONNECTED;
                goto done;
        }

        bonded = dev->bonded;
        master = dev->master;
        storage_release();

        if (master) {
#if (dg_configBLE_CENTRAL == 1)
                ble_evt_gap_set_sec_level_failed_t *evt;

                /* Create new event and fill it */
                evt = ble_evt_init(BLE_EVT_GAP_SET_SEC_LEVEL_FAILED, sizeof(*evt));
                evt->conn_idx = conn_idx;

                if (!encrypt_conn_using_ltk(conn_idx, mitm ? GAP_AUTH_MITM : 0, evt)) {
                        /* Free not needed event buffer */
                        ble_msg_free(evt);

                        /* Initiate pairing */
                        send_bond_cmd(conn_idx, io_cap, bonded, mitm, secure);
                }
                status = BLE_STATUS_OK;
                goto done;
#endif /* (dg_configBLE_CENTRAL == 1) */
        } else {
#if (dg_configBLE_PERIPHERAL == 1)
                send_security_req(conn_idx, bonded, mitm, secure);
                status = BLE_STATUS_OK;
                goto done;
#endif
        }

done:
        rsp->status = status;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

#if (dg_configBLE_SKIP_LATENCY_API == 1)
void ble_mgr_gap_skip_latency_cmd_handler(void *param)
{
        const ble_mgr_gap_skip_latency_cmd_t *cmd = param;
        ble_mgr_gap_skip_latency_rsp_t *rsp;
        uint16_t conn_idx = cmd->conn_idx;
        bool enable = cmd->enable;
        ble_error_t status = BLE_ERROR_FAILED;
        device_t *dev;

        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GAP_SKIP_LATENCY_CMD, sizeof(*rsp));

        storage_acquire();
        dev = find_device_by_conn_idx(conn_idx);
        if (!dev) {
                status = BLE_ERROR_NOT_CONNECTED;
        } else if (dev->master) {
                status = BLE_ERROR_NOT_ALLOWED;
        } else {
                ble_mgr_skip_latency_set(conn_idx, enable);
                status = BLE_STATUS_OK;
        }
        storage_release();

        rsp->status = status;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}
#endif /* (dg_configBLE_SKIP_LATENCY_API == 1) */

void ble_mgr_gap_le_pkt_size_ind_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gapc_le_pkt_size_ind *gevt = (void *) gtl->param;
        ble_evt_gap_data_length_changed_t *evt;

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_GAP_DATA_LENGTH_CHANGED, sizeof(*evt));
        evt->conn_idx      = TASK_2_CONNIDX(gtl->src_id);
        evt->max_rx_length = gevt->max_rx_octets;
        evt->max_rx_time   = gevt->max_rx_time;
        evt->max_tx_length = gevt->max_tx_octets;
        evt->max_tx_time   = gevt->max_tx_time;

        /* Send to event queue */
        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
}

void ble_mgr_gap_data_length_set_cmd_handler(void *param)
{
        const ble_mgr_gap_data_length_set_cmd_t *cmd = param;
        ble_mgr_gap_data_length_set_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        ble_error_t status = BLE_ERROR_FAILED;
        device_t *dev;

        /* Check if data length and time values provided are within the supported range */
        if ((cmd->tx_length < GAPM_LE_LENGTH_EXT_OCTETS_MIN) ||
            (cmd->tx_length > GAPM_LE_LENGTH_EXT_OCTETS_MAX) ||
            (cmd->tx_time && cmd->tx_time != BLE_DATA_LENGTH_TO_TIME(cmd->tx_length))) {
                status = BLE_ERROR_INVALID_PARAM;
                goto done;
        }

        if (cmd->conn_idx == BLE_CONN_IDX_INVALID) {
                /* Set preferred data length for future connections */
                ble_dev_params_t *ble_dev_params;
                struct gapm_set_dev_config_cmd *gcmd;

                gmsg = ble_gtl_alloc(GAPM_SET_DEV_CONFIG_CMD, TASK_ID_GAPM,
                                                            sizeof(struct gapm_set_dev_config_cmd));
                gcmd = (struct gapm_set_dev_config_cmd *) gmsg->msg.gtl.param;

                ble_dev_params = ble_mgr_dev_params_acquire();

                gcmd->operation = GAPM_SET_SUGGESTED_DFLT_LE_DATA_LEN;
                gcmd->max_txoctets = cmd->tx_length;
                gcmd->max_txtime = cmd->tx_time;
                gcmd->max_mps = ble_dev_params->mtu_size;

                ble_mgr_dev_params_release();

                ble_gtl_send(gmsg);
        } else {
                /* Set data length for specified connection */
                storage_acquire();

                dev = find_device_by_conn_idx(cmd->conn_idx);
                if (!dev) {
                        storage_release();
                        status = BLE_ERROR_NOT_CONNECTED;
                        goto done;
                }

                storage_release();

                change_conn_data_length(cmd->conn_idx, cmd->tx_length, cmd->tx_time);
        }

        status = BLE_STATUS_OK;

done:
        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_GAP_DATA_LENGTH_SET_CMD, sizeof(*rsp));
        rsp->status = status;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gap_cmp__data_length_set_evt_handler(ble_gtl_msg_t *gtl)
{
        /*
         * This handler will handle both GAPM_CMP_EVT and GAPC_CMP_EVT events. These event
         * structures are identical so there is no need to differentiate.
         */
        struct gapm_cmp_evt *gevt = (void *) gtl->param;
        ble_evt_gap_data_length_set_failed_t *evt;

        if (gevt->status == GAP_ERR_NO_ERROR) {
                /* Nothing to do, event should be sent only when error occurred */
                return;
        }

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_GAP_DATA_LENGTH_SET_FAILED, sizeof(*evt));
        evt->conn_idx = TASK_2_CONNIDX(gtl->src_id);

        /* Translate error code */
        switch (gevt->status) {
        case CO_ERROR_COMMAND_DISALLOWED:
                evt->status = BLE_ERROR_NOT_ALLOWED;
                break;
        case CO_ERROR_UNSUPPORTED_REMOTE_FEATURE:
                evt->status = BLE_ERROR_NOT_SUPPORTED;
                break;
        case CO_ERROR_INVALID_HCI_PARAM:
                evt->status = BLE_ERROR_INVALID_PARAM;
                break;
        default:
                evt->status = BLE_ERROR_FAILED;
                break;
        }

        /* Send to event queue */
        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
}

void ble_mgr_gap_address_resolve_cmd_handler(void *param)
{
        const ble_mgr_gap_address_resolve_cmd_t *cmd = param;
        ble_mgr_gap_address_resolve_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapm_resolv_addr_cmd *gcmd;
        struct irk_copy_data copy_data;
        uint8_t irk_count = 0;
        ble_error_t status = BLE_ERROR_FAILED;
        ble_dev_params_t *ble_dev_params;

        /* Check if peer's address is random */
        if (cmd->address.addr_type != PRIVATE_ADDRESS) {
                status = BLE_ERROR_INVALID_PARAM;
                goto done;
        }

        /* Check if peer's address is resolvable */
        if ((cmd->address.addr[BD_ADDR_LEN-1] & 0xc0) != GAP_RSLV_ADDR) {
                status = BLE_ERROR_INVALID_PARAM;
                goto done;
        }

        device_foreach(irk_count_cb, &irk_count);
        if (irk_count == 0) {
                status = BLE_ERROR_NOT_FOUND;
                goto done;
        }

        gmsg = ble_gtl_alloc(GAPM_RESOLV_ADDR_CMD, TASK_ID_GAPM, sizeof(*gcmd) +
                                                        (sizeof(struct gap_sec_key) * irk_count));
        gcmd = (struct gapm_resolv_addr_cmd *) gmsg->msg.gtl.param;
        memcpy(&gcmd->addr, &cmd->address.addr, sizeof(gcmd->addr));
        gcmd->operation = GAPM_RESOLV_ADDR;
        gcmd->nb_key = irk_count;

        /* Copy IRKs */
        copy_data.array = gcmd->irk;
        copy_data.index = 0;
        device_foreach(irk_copy_cb, &copy_data);

        ble_dev_params = ble_mgr_dev_params_acquire();
        /* Increase request count */
        ble_dev_params->addr_resolv_req_pending++;
        ble_mgr_dev_params_release();

        ble_gtl_send(gmsg);

        status = BLE_STATUS_OK;

done:
        rsp = ble_msg_init(BLE_MGR_GAP_ADDRESS_RESOLVE_CMD, sizeof(*rsp));
        rsp->status = status;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_gapm_cmp__address_resolve_evt_handler(ble_gtl_msg_t *gtl)
{
        struct gapm_cmp_evt *gevt = (void *) gtl->param;
        ble_evt_gap_address_resolution_failed_t *evt;
        ble_dev_params_t *ble_dev_params = ble_mgr_dev_params_acquire();

        /* Decrement request count */
        ble_dev_params->addr_resolv_req_pending--;

        ble_mgr_dev_params_release();

        if (gevt->status == GAP_ERR_NO_ERROR) {
                /* Nothing to do, event should be sent only when error occurred */
                return;
        }

        /* Create new event and fill it */
        evt = ble_evt_init(BLE_EVT_GAP_ADDRESS_RESOLUTION_FAILED, sizeof(*evt));

        /* Translate error code */
        switch (gevt->status) {
        case GAP_ERR_COMMAND_DISALLOWED:
                evt->status = BLE_ERROR_NOT_ALLOWED;
                break;
        case GAP_ERR_INVALID_PARAM:
                evt->status = BLE_ERROR_INVALID_PARAM;
                break;
        case GAP_ERR_NOT_FOUND:
                evt->status = BLE_ERROR_NOT_FOUND;
                break;
        default:
                evt->status = BLE_ERROR_FAILED;
                break;
        }

        /* Send to event queue */
        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
}

#if (dg_configBLE_PRIVACY_1_2 == 1)
typedef struct {
        ble_mgr_cmd_handler_t handler;
        void *param;
        uint8_t dev_info_count;
        uint8_t dev_info_index;
        struct gap_ral_dev_info dev_info[0];
} ble_mgr_gap_ral_sync_param_t;

static void ral_copy_cb(device_t *dev, void *ud)
{
        ble_mgr_gap_ral_sync_param_t *ble_mgr_gap_ral_sync_param = (ble_mgr_gap_ral_sync_param_t *) ud;
        struct gap_ral_dev_info* dev_info;
        uint8_t local_irk[KEY_LEN];

        if (dev->irk) {
                dev_info = &ble_mgr_gap_ral_sync_param->dev_info[ble_mgr_gap_ral_sync_param->dev_info_index];

                ad_ble_get_irk(local_irk);
                memcpy(&dev_info->addr, dev->addr.addr, BD_ADDR_LEN);
                dev_info->addr_type = dev->addr.addr_type;
                memcpy(dev_info->local_irk, local_irk, KEY_LEN);
                memcpy(dev_info->peer_irk, dev->irk->key, KEY_LEN);

                ble_mgr_gap_ral_sync_param->dev_info_index++;
        }
}

static void ble_mgr_gap_ral_sync_complete(ble_gtl_msg_t *gtl, void *param)
{
        struct gapm_cmp_evt *gevt = (void *) gtl->param;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapm_rslv_list_mgt_cmd *gcmd;
        ble_mgr_gap_ral_sync_param_t* ral_sync_param = param;

        if ((gevt->status == LL_ERR_COMMAND_DISALLOWED) || (gevt->status == LL_ERR_MEMORY_CAPA_EXCEED)) {
                /* Resolving list cannot be updated due to ongoing air operation
                * or because RAL is full */
                ble_dev_params_t * ble_dev_params = ble_mgr_dev_params_acquire();
                ble_dev_params->prev_privacy_operation = BLE_MGR_RAL_OP_NONE;
                ble_mgr_dev_params_release();

                if (ral_sync_param->handler == ble_mgr_gap_adv_start_cmd_exec) {
                        ble_mgr_gap_adv_start_rsp_t *rsp;
                        ble_msg_free(ral_sync_param->param);
                        rsp = ble_msg_init(BLE_MGR_GAP_ADV_START_CMD, sizeof(*rsp));
                        rsp->status = BLE_ERROR_NOT_ALLOWED;
                        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
                } else if (ral_sync_param->handler == ble_mgr_gap_scan_start_cmd_exec) {
                        ble_mgr_gap_scan_start_rsp_t *rsp;
                        ble_msg_free(ral_sync_param->param);
                        rsp = ble_msg_init(BLE_MGR_GAP_SCAN_START_CMD, sizeof(*rsp));
                        rsp->status = BLE_ERROR_NOT_ALLOWED;
                        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
                } else {
                        ble_mgr_gap_connect_rsp_t *rsp;
                        ble_msg_free(ral_sync_param->param);
                        rsp = ble_msg_init(BLE_MGR_GAP_CONNECT_CMD, sizeof(*rsp));
                        rsp->status = BLE_ERROR_NOT_ALLOWED;
                        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
                }

                OS_FREE(ral_sync_param);
                return;
        }

        if (ral_sync_param->dev_info_index == ral_sync_param->dev_info_count) {
                /* RAL cleared and synced, start air operation */
                ral_sync_param->handler(ral_sync_param->param);
                OS_FREE(ral_sync_param);
                return;
        } else {
                /* RAL cleared, still more devices to add */
        }

        gmsg = ble_gtl_alloc(GAPM_RAL_MGT_CMD, TASK_ID_GAPM,
                                        sizeof(struct gapm_rslv_list_mgt_cmd) + sizeof(struct gap_ral_dev_info));

        gcmd = (struct gapm_rslv_list_mgt_cmd *) gmsg->msg.gtl.param;

        gcmd->operation = GAPM_ADD_DEV_IN_RAL;
        gcmd->nb = 1;
        memcpy(gcmd->devices, &ral_sync_param->dev_info[ral_sync_param->dev_info_index],
                                        sizeof(struct gap_ral_dev_info));

        ral_sync_param->dev_info_index++;

        ble_gtl_waitqueue_add(0, GAPM_CMP_EVT, GAPM_ADD_DEV_IN_RAL, ble_mgr_gap_ral_sync_complete, param);
        ble_gtl_send(gmsg);
}


static void ble_mgr_gap_ral_sync(ble_mgr_cmd_handler_t handler, void *param)
{
        ble_mgr_gap_ral_sync_param_t* ral_sync_param;
        bd_address_t adv_direct_address;
        ble_dev_params_t *ble_dev_params;
        ble_mgr_common_stack_msg_t *gmsg;
        struct gapm_rslv_list_mgt_cmd *gcmd;
        uint8_t irk_count = 1;
        own_addr_type_t addr_type;
        bool ral_synced;
        device_t* dev;
        ble_mgr_ral_op_t privacy_operation;

        ble_dev_params = ble_mgr_dev_params_acquire();
        addr_type = ble_dev_params->own_addr.addr_type;
        adv_direct_address = ble_dev_params->adv_direct_address;
        ble_mgr_dev_params_release();

        if (addr_type != PRIVATE_CNTL) {
                /* This will ensure that RAL will be cleared when privacy is turned off/on */
                privacy_operation = BLE_MGR_RAL_OP_NO_PRIVACY;
        } else {
                if (handler == ble_mgr_gap_adv_start_cmd_exec) {
                        const ble_mgr_gap_adv_start_cmd_t *cmd = param;
                        if ((cmd->adv_type == GAP_CONN_MODE_DIRECTED) || (cmd->adv_type == GAP_CONN_MODE_DIRECTED_LDC)) {
                                dev = find_device_by_addr(&adv_direct_address, false);
                                if (!dev) {
                                        /* Always resync RAL as peer address might have been changed */
                                        privacy_operation = BLE_MGR_RAL_OP_NONE;
                                } else {
                                        privacy_operation = BLE_MGR_RAL_OP_ADV_DIRECTED;
                                }
                        } else {
                                privacy_operation = BLE_MGR_RAL_OP_ADV_UNDIRECTED;
                        }
                } else if (handler == ble_mgr_gap_scan_start_cmd_exec) {
                        privacy_operation = BLE_MGR_RAL_OP_SCAN;
                } else { /* ble_mgr_gap_connect_cmd_exec */
                        const ble_mgr_gap_connect_cmd_t *cmd = param;
                        dev = find_device_by_addr(cmd->peer_addr, false);
                        if (!dev) {
                                /* Always resync RAL as peer address might have been changed */
                                privacy_operation = BLE_MGR_RAL_OP_NONE;
                        } else {
                                privacy_operation = BLE_MGR_RAL_OP_CONNECT;
                        }
                }
        }

        /*
         * If RAL is synced, the air operation will be executed directly.
         * If RAL is out of sync, the air operation will  take place after
         * host's IRK list is provided to the controller. prev_privacy_operation is updated
         * with the current operation before the actual synchronization is completed. This
         * will allow ble events/cmds (like pairing failed) to desynchronize RAL
         * while RAL synchronization is ongoing by setting prev_privacy_operation
         * to BLE_MGR_RAL_OP_NONE.
         */
        ble_dev_params = ble_mgr_dev_params_acquire();
        if ((privacy_operation != BLE_MGR_RAL_OP_NONE) && (privacy_operation == ble_dev_params->prev_privacy_operation)) {
                ral_synced = true;
        } else {
                ral_synced = false;
        }
        ble_dev_params->prev_privacy_operation = privacy_operation;
        ble_mgr_dev_params_release();

        if (ral_synced) {
                /* RAL synced */
                handler(param);
                return;
        }

        if (addr_type == PRIVATE_CNTL) {
                device_foreach(irk_count_cb, &irk_count);

                ral_sync_param = OS_MALLOC(sizeof(ble_mgr_gap_ral_sync_param_t) +
                                                        irk_count * sizeof(struct gap_ral_dev_info));
                ral_sync_param->handler = handler;
                ral_sync_param->param = param;
                ral_sync_param->dev_info_index = 0;
                ral_sync_param->dev_info_count = irk_count;

                /* Resolving list entry for own device */
                uint8_t identity_address[BD_ADDR_LEN];
                uint8_t local_irk[KEY_LEN];
                ad_ble_get_irk(local_irk);
                ad_ble_get_public_address(identity_address);
                memcpy(&ral_sync_param->dev_info[0].addr, identity_address, BD_ADDR_LEN);
                ral_sync_param->dev_info[0].addr_type = ADDR_PUBLIC;
                memcpy(ral_sync_param->dev_info[0].local_irk, local_irk, KEY_LEN);
                memset(ral_sync_param->dev_info[0].peer_irk, 0, KEY_LEN);

                /*
                 * If we are not bonded with the peer and the air operation is either directed advertisement
                 * or connection initiation, add peer's address to RAL with zero IRK to generate an RPA initA.
                 * For undirected advertisement/scanning we are adding our identity address with all-zero peer IRK.
                 */
                if (ral_sync_param->handler == ble_mgr_gap_adv_start_cmd_exec) {
                        const ble_mgr_gap_adv_start_cmd_t *cmd = param;
                        if ((cmd->adv_type == GAP_CONN_MODE_DIRECTED) || (cmd->adv_type == GAP_CONN_MODE_DIRECTED_LDC)) {
                                dev = find_device_by_addr(&adv_direct_address, false);
                                if (!dev) {
                                        memcpy(&ral_sync_param->dev_info[0].addr, adv_direct_address.addr, BD_ADDR_LEN);
                                        ral_sync_param->dev_info[0].addr_type = adv_direct_address.addr_type;
                                }
                        }
                } else if (ral_sync_param->handler == ble_mgr_gap_connect_cmd_exec) {
                        const ble_mgr_gap_connect_cmd_t *cmd = param;
                        dev = find_device_by_addr(cmd->peer_addr, false);
                        if (!dev) {
                                memcpy(&ral_sync_param->dev_info[0].addr, cmd->peer_addr->addr, BD_ADDR_LEN);
                                ral_sync_param->dev_info[0].addr_type = cmd->peer_addr->addr_type;
                        }
                }

                ral_sync_param->dev_info_index++;

                /* Resolving list entries for other devices */
                device_foreach(ral_copy_cb, ral_sync_param);
        } else {
                /* Just clear RAL */
                ral_sync_param = OS_MALLOC(sizeof(ble_mgr_gap_ral_sync_param_t));
                ral_sync_param->handler = handler;
                ral_sync_param->param = param;
                ral_sync_param->dev_info_count = 0;
        }

        ral_sync_param->dev_info_index = 0;

        /* Send GAPM_CLEAR_RAL command */
        gmsg = ble_gtl_alloc(GAPM_RAL_MGT_CMD, TASK_ID_GAPM, sizeof(struct gapm_rslv_list_mgt_cmd));
        gcmd = (struct gapm_rslv_list_mgt_cmd *) gmsg->msg.gtl.param;
        gcmd->operation = GAPM_CLEAR_RAL;
        ble_gtl_waitqueue_add(0, GAPM_CMP_EVT, GAPM_CLEAR_RAL, ble_mgr_gap_ral_sync_complete, ral_sync_param);
        ble_gtl_send(gmsg);
}
#endif /* (dg_configBLE_PRIVACY_1_2 == 1) */
