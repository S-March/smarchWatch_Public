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
 * @file ble_mgr_gtl.h
 *
 * @brief tbd
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BLE_MGR_GTL_H_
#define BLE_MGR_GTL_H_

#include <stdbool.h>
#include "ad_ble.h"

/** Extract connection index from task ID */
#define TASK_2_CONNIDX(T) (T >> 8)

/**
 * \brief Waitqueue callback
 *
 */
typedef void (* ble_gtl_waitqueue_cb_t) (ble_gtl_msg_t *gtl, void *param);

/**
 * \brief Alloc stack API HCI message
 *
 * \param [in] hci_msg_type   message type
 * \param [in] len            message length
 *
 * \return message pointer
 *
 */
void *ble_hci_alloc(uint8_t hci_msg_type, uint16_t len);

/**
 * \brief Alloc stack API GTL message
 *
 * Message will be set to TASK_GTL as source task.
 *
 * \param [in] msg_id   message ID
 * \param [in] dest_id  destination task
 * \param [in] len      message length
 *
 * \return message pointer
 *
 */
void *ble_gtl_alloc(uint16_t msg_id, uint16_t dest_id, uint16_t len);

/**
 * \brief Alloc stack API GTL message (with connection index)
 *
 * This is the same as ble_gtl_alloc() except it should be used when sending message to a task
 * which is multi-instantiated (i.e. includes connection index in task ID).
 *
 * \param [in] msg_id   message ID
 * \param [in] dest_id  destination task
 * \param [in] conn_idx connection index
 * \param [in] len      message length
 *
 * \return message pointer
 *
 */
static inline void *ble_gtl_alloc_with_conn(uint16_t msg_id, uint16_t dest_id,
                                                                uint16_t conn_idx, uint16_t len)
{
        return ble_gtl_alloc(msg_id, conn_idx << 8 | (dest_id & 0xFF), len);
}

/**
 * \brief Send stack API GTL message to adapter
 *
 * \param [in] msg      message pointer
 *
 */
static inline void ble_gtl_send(void *msg)
{
        ad_ble_command_queue_send(&msg, OS_QUEUE_FOREVER);
}

/**
 * \brief Add callback to waitqueue
 *
 * This adds callback for waitqueue associated with specific message ID and optional extended ID
 * (if required, for example 'operation' in GATM and 'seq_num' in GATTC).
 *
 * \param [in] conn_idx connection index (for non-connection oriented messages should be 0)
 * \param [in] msg_id   message ID
 * \param [in] ext_id   extended ID (optional, depending on \p msg_id)
 * \param [in] cb       callback
 * \param [in] param    param for callback
 */
void ble_gtl_waitqueue_add(uint16_t conn_idx, uint16_t msg_id, uint16_t ext_id,
                                                            ble_gtl_waitqueue_cb_t cb, void *param);

/**
 * Match GTL message against waitqueue
 *
 * On positive match, this will remove element from waitqueue and fire associated callback.
 *
 * \param [in] gtl      GTL message pointer
 *
 * \return true if matches, false otherwise
 *
 */
bool ble_gtl_waitqueue_match(ble_gtl_msg_t *gtl);

/**
 * Flush waitqueue of connection-related elements
 *
 * On positive match, this will remove element from waitqueue and fire associated callback with NULL
 * GTL message pointer.
 *
 * \param [in] conn_idx    Connection index that corresponds to disconnected device
 */
void ble_gtl_waitqueue_flush(uint16_t conn_idx);

/**
 * Handle GTL event
 *
 * This will only handle known events, for other messages it will return without doing anything.
 *
 * \param [in] gtl      GTL message pointer
 *
 * \return true if handled, false otherwise
 *
 */
bool ble_gtl_handle_event(ble_gtl_msg_t *gtl);

#endif /* BLE_MGR_GTL_H_ */
/**
 \}
 \}
 \}
 */
