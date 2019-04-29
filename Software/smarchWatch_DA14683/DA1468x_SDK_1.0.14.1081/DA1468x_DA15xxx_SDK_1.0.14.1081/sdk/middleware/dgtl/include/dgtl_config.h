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
 * @file dgtl_config.h
 *
 * @brief Default DGTL configuration
 *
 * Copyright (C) 2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef DGTL_CONFIG_H_
#define DGTL_CONFIG_H_

/**
 * \brief Enable application-specific HCI commands
 *
 * When this macro is enabled, HCI commands with an opcode within the vendor-specific
 * address range (0xFE00 - 0xFFFF) will be forwarded to an application-specific callback.
 *
 * Please see \p sdk/middleware/dgtl/include/dgtl.h for more details.
 *
 */
#ifndef DGTL_APP_SPECIFIC_HCI_ENABLE
#define DGTL_APP_SPECIFIC_HCI_ENABLE    (0)
#endif

/**
 * \brief Enable LOG Queue dropped messages counter
 *
 * The LOG queue, in contrast to the rest of the queues, works in
 * non-blocking mode. That means that if there is no more space in the queue
 * left, a message to be pushed into the queue will be silently dropped.
 *
 * When this macro is enabled, DGTL instantiates a counter that counts the dropped
 * messages.
 *
 */
#ifndef DGTL_DROPPED_LOG_QUEUE_COUNTER
#define DGTL_DROPPED_LOG_QUEUE_COUNTER  (0)
#endif

/**
 * \brief Enable DGTL HCI/GTL queue
 *
 * This macro enables the use of the DGTL HCI/GTL queue (packet types 1-5)
 *
 */
#ifndef DGTL_QUEUE_ENABLE_HCI
#define DGTL_QUEUE_ENABLE_HCI           (1)
#endif

/**
 * \brief Enable DGTL APP queue
 *
 * This macro enables the use of the DGTL APP queue (packet types 6-7)
 *
 */
#ifndef DGTL_QUEUE_ENABLE_APP
#define DGTL_QUEUE_ENABLE_APP           (1)
#endif

/**
 * \brief Enable DGTL LOG queue
 *
 * This macro enables the use of the DGTL LOG queue (packet type 8)
 *
 */
#ifndef DGTL_QUEUE_ENABLE_LOG
#define DGTL_QUEUE_ENABLE_LOG           (1)
#endif

#endif /* DGTL_CONFIG_H_ */

/**
 * \}
 * \}
 * \}
 */
