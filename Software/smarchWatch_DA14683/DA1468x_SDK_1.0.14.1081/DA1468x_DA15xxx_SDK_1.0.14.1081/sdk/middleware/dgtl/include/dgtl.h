/**
 * \addtogroup BSP
 * \{
 * \addtogroup MIDDLEWARE
 * \{
 * \addtogroup DGTL
 *
 * \brief DGTL framework
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file dgtl.h
 *
 * @brief Declarations for DGTL
 *
 * Copyright (C) 2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef DGTL_H_
#define DGTL_H_

#include <stdbool.h>
#include <stdint.h>
#include "dgtl_msg.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief DGTL queue type
 */
typedef enum {
        DGTL_QUEUE_HCI,
        DGTL_QUEUE_APP,
        DGTL_QUEUE_LOG,
} dgtl_queue_t;

/**
 * Callback for application specific HCI commands
 *
 * This function is called by DGTL when HCI command from application specific opcode range is
 * received. It is called when \p DGTL_APP_SPECIFIC_HCI_ENABLE option is non-zero. Application shall
 * define this function in order to override weak reference defined by DGTL code internally.
 *
 * \warning
 * Application is responsible for freeing \p msg when no longer needed.
 *
 * \param [in] msg  DGTL message
 *
 */
void dgtl_app_specific_hci_cb(const dgtl_msg_t *msg);

/**
 * Initialize DGTL
 *
 * This function initializes internal DGTL structures and thus shall be called by the application
 * before using any other DGTL API.
 *
 */
void dgtl_init(void);

/**
 * \brief Register current task for given queue
 *
 * This function allows the calling task to register itself as a client to receive messages from
 * a given queue. The DGTL interface will notify application task using \p notif bitmask whenever
 * a new message is available in the queue, which can be received using dgtl_receive().
 *
 * \note
 * Only one task can be registered to get notifications for a queue.
 *
 * \param [in] queue  queue type
 * \param [in] notif  notification bitmask to be used
 *
 */
void dgtl_register(dgtl_queue_t queue, uint32_t notif);

/**
 * \brief Send message to the DGTL interface
 *
 * This function sends a message to the DGTL interface. The target queue is automatically selected
 * based on the packet type indicator present in the message.
 *
 * After calling this function, the sender is no longer the owner of the message and should not use
 * it anymore.
 *
 * \param [in] msg  DGTL message
 *
 */
void dgtl_send(dgtl_msg_t *msg);

/**
 * \brief Receive message from the DGTL interface
 *
 * This function receives a message from a specified queue from the DGTL interface. The application
 * can only receive messages from the queue it previously registered to using dgtl_register().
 *
 * The receiver becomes owner of the message and shall free it when it is no longer in use.
 *
 * \param [in] queue  queue to receive message from
 *
 * \return received message or NULL if either no message is present in queue or the application is
 *         not allowed to receive message from queue
 *
 */
dgtl_msg_t *dgtl_receive(dgtl_queue_t queue);

#ifdef __cplusplus
}
#endif

#endif /* DGTL_H_ */

/**
 * \}
 * \}
 * \}
 */
