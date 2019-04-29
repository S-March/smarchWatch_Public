/**
 \addtogroup INTERFACES
 \{
 \addtogroup BLE
 \{
 \addtogroup MGR
 \{
 */

/**
 ****************************************************************************************
 *
 * @file ble_mgr_helper.h
 *
 * @brief BLE message creation and handling
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BLE_MGR_HELPER_H_
#define BLE_MGR_HELPER_H_

#include <stdbool.h>
#include "osal.h"
#include "ble_mgr_cmd.h"

/**
 * \brief Allocates new BLE message
 *
 * \param [in] op_code  message op code
 * \param [in] size     message size
 *
 * \return allocated message pointer
 *
 */
void *alloc_ble_msg(uint16_t op_code, uint16_t size);

/**
 * \brief Initialize BLE message
 *
 * Initialize a BLE message with op_code and size. This will allocate a buffer with size equal to
 * \p size and fill it's op_code with \p op_code. 
 *
 * \param [in]      op_code  message op code
 * \param [in]      size     message size
 *
 * \return allocated message pointer
 *
 */
void *ble_msg_init(uint16_t op_code, uint16_t size);

/**
 * \brief Initialize BLE event
 *
 * Initialize a BLE event with evt_code and size. This will allocate a buffer with size equal to
 * \p size and fill it's op_code with \p evt_code. 
 *
 * \param [in]      evt_code event code
 * \param [in]      size     event size
 *
 * \return allocated event buffer pointer
 */
void *ble_evt_init(uint16_t evt_code, uint16_t size);

/**
 * \brief Free BLE message buffer
 *
 * \param [in]      msg      Message buffer
 *
 */
void ble_msg_free(void *msg);

/**
 * \brief Execute BLE command
 *
 * This calls manager's \p handler with command (if #BLE_MGR_DIRECT_ACCESS is defined to 1) or sends
 * it to manager's command queue (if #BLE_MGR_DIRECT_ACCESS is defined to 0) and waits for response.
 * Buffer pointed by \p cmd is owned by the BLE manager after calling this function and should not
 * be accessed.
 * Buffer returned in \p rsp is owned by caller and should be freed there.
 *
 * \param [in]  cmd      command buffer
 * \param [out] rsp      response buffer
 * \param [in]  handler  command handler
  *
 * \return true if executed successfully, false otherwise
 *
 */
bool ble_cmd_execute(void *cmd, void **rsp, ble_mgr_cmd_handler_t handler);

#endif /* BLE_MGR_HELPER_H_ */
/**
 \}
 \}
 \}
 */
