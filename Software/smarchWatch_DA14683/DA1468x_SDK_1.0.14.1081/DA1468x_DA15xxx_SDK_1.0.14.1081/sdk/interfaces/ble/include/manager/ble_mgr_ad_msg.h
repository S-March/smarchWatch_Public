/**
 \addtogroup INTERFACES
 \{
 \addtogroup BLE
 \{
 \addtogroup MANAGER
 \{
 */

/**
 ****************************************************************************************
 *
 * @file ble_mgr_ad_msg.h
 *
 * @brief Helper library API for BLE adapter message handling in BLE Manager
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BLE_MGR_AD_MSG_H_
#define BLE_MGR_AD_MSG_H_

#include <stdbool.h>
#include "ad_ble.h"

/**
 * \brief Waitqueue callback
 *
 */
typedef void (* ble_ad_msg_wqueue_cb_t) (ad_ble_msg_t *ad_msg, void *param);

/**
 * \brief Allocate buffer for a BLE adapter message
 *
 * Message will be set with passed operation code.
 *
 * \param [in] operation  BLE adapter operation code
 * \param [in] len        message length
 *
 * \return message pointer
 *
 */
void *ble_ad_msg_alloc(ad_ble_operation_t operation, uint16_t len);

/**
 * \brief Send BLE adapter message to adapter
 *
 * \param [in] msg      message pointer
 *
 */
static inline void ble_ad_msg_send(void *msg)
{
        ad_ble_command_queue_send(&msg, OS_QUEUE_FOREVER);
}

/**
 * \brief Add callback to waitqueue
 *
 * This adds callback for waitqueue associated with specific combination of response's operation
 * code and command's operation code.
 *
 * \param [in] rsp_op
 * \param [in] cmd_op
 * \param [in] cb
 * \param [in] param
 */
void ble_ad_msg_wqueue_add(ad_ble_operation_t rsp_op, ad_ble_operation_t cmd_op,
                           ble_ad_msg_wqueue_cb_t cb, void *param);

/**
 * Match BLE adapter message against waitqueue
 *
 * On positive match, this will remove element from waitqueue and fire associated callback.
 *
 * \param [in] ad_msg      BLE adapter message pointer
 *
 * \return true if matches, false otherwise
 *
 */
bool ble_ad_msg_waitqueue_match(ad_ble_msg_t *ad_msg);

/**
 * Follow-up action on reception of a BLE_ADAPTER_CMP_EVT from the BLE adapter
 *
 * \param [in] ad_msg     BLE adapter message pointer
 * \param [in] param      Pointer to passing parameter
 */
void ble_adapter_cmp_evt_init(ad_ble_msg_t *ad_msg, void *param);

#endif /* BLE_MGR_AD_MSG_H_ */
/**
 \}
 \}
 \}
 */
