/**
 \addtogroup INTERFACES
 \{
 \addtogroup FTDF
 \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_ftdf_mac_api.h
 *
 * @brief FTDF MAC Adapter API
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef AD_FTDF_MAC_API_H
#define AD_FTDF_MAC_API_H

#include "osal.h"
#include "ftdf.h"
#include "ad_ftdf_config.h"

/**
 * \brief Send a message to the FTDF adapter DOWN queue
 *
 * Sends a message to the FTDF adapter DOWN queue
 *
 * \param[in] item - A pointer to the item to put to the queue. Usually this is
 *                   a pointer to a pointer (since we actually need to put the pointer 
 *                   of the data structure to the queue).
 * \param[in] wait_ticks - Time to wait
 */
OS_BASE_TYPE ad_ftdf_queue_send(const void *item, OS_TICK_TIME wait_ticks);

/**
 * \brief Initialize adapter - create queues
 *
 */
void ad_ftdf_init(void);

/**
 * \brief Get UP Queue
 *
 * Get a pointer handle to the queue used to transfer messages from
 * UMAC to the client app (upstream)
 *
 */
OS_QUEUE ad_ftdf_get_up_queue(void);

/**
 * \brief Allocate message buffer
 *
 * Allocates a message buffer for storing a UMAC-bound message. NOTE:
 * If the message needs to also store a data buffer, the latter needs to
 * be allocated separately via a call to ftdf_get_data_buffer().
 *
 * It should be called like:
 *     msg = ftdf_get_msg_buffer(sizeof(ftdf_data_request_t));
 *
 * \param[in] len - The sizes, in bytes, of the buffer to allocate
 * \returns - A pointer to the allocated buffer
 */
ftdf_msg_buffer_t* ad_ftdf_get_msg_buffer(ftdf_size_t len);

/**
 * \brief Release message buffer
 *
 * Releases a message buffer allocated by UMAC. NOTE: If the message
 * buffer contains a pointer to a data buffer, the latter should also
 * be released using ftdf_rel_data_buffer()
 *
 * \param[in] msg_buf - Pointer to the buffer to release
 */
void ad_ftdf_rel_msg_buffer(ftdf_msg_buffer_t* msg_buf);

/**
 * \brief Allocate data buffer
 *
 * Allocates a data buffer for storing a UMAC-bound frame
 *
 * \param[in] len - The sizes, in bytes, of the buffer to allocate
 * \returns - A pointer to the allocated buffer
 */
ftdf_octet_t* ad_ftdf_get_data_buffer(ftdf_data_length_t len);

/**
 * \brief Release data buffer
 *
 * Releases a data buffer allocated by UMAC.
 *
 * \param[in] data_buf - Pointer to the buffer to release
 */
void ad_ftdf_rel_data_buffer(ftdf_octet_t* data_buf);

/**
 * \brief Release message and associated data buffer
 *
 * Releases a message buffer along with any additional data
 * buffer contained in it.
 *
 * \param[in] msg_buf - Pointer to the message to release
 */
void ad_ftdf_rel_msg_data(ftdf_msg_buffer_t *msg_buf);

/**
 * \brief Set Extended Address
 *
 * Sets interface extended address. This is thread safe.
 *
 * \param[in] address - The extended address to set
 */
void ad_ftdf_set_ext_address(ftdf_ext_address_t address);

/**
 * \brief Get Extended Address
 *
 * Gets interface extended address. This is thread safe.
 *
 * \returns The extended address of the interface
 */
ftdf_ext_address_t ad_ftdf_get_ext_address(void);

#if FTDF_DBG_BUS_ENABLE
/**
 * \brief Configures GPIO pins for the FTDF debug bus
 *
 * if FTDF_DBG_BUS_USE_PORT_4 == 0, the FTDF debug bus uses the following (fixed) GPIO pins,
 *
 * bit 0: HW_GPIO_PORT_1, HW_GPIO_PIN_4
 * bit 1: HW_GPIO_PORT_1, HW_GPIO_PIN_5
 * bit 2: HW_GPIO_PORT_1, HW_GPIO_PIN_6
 * bit 3: HW_GPIO_PORT_1, HW_GPIO_PIN_7
 * bit 4: HW_GPIO_PORT_0, HW_GPIO_PIN_6
 * bit 5: HW_GPIO_PORT_0, HW_GPIO_PIN_7
 * bit 6: HW_GPIO_PORT_1, HW_GPIO_PIN_3
 * bit 7: HW_GPIO_PORT_2, HW_GPIO_PIN_3
 *
 * if FTDF_DBG_BUS_USE_PORT_4 == 1, the FTDF debug bus uses the following pins:
 *
 * bit 0: HW_GPIO_PORT_4, HW_GPIO_PIN_0
 * bit 1: HW_GPIO_PORT_4, HW_GPIO_PIN_1
 * bit 2: HW_GPIO_PORT_4, HW_GPIO_PIN_2
 * bit 3: HW_GPIO_PORT_4, HW_GPIO_PIN_3
 * bit 4: HW_GPIO_PORT_4, HW_GPIO_PIN_4
 * bit 5: HW_GPIO_PORT_4, HW_GPIO_PIN_5
 * bit 6: HW_GPIO_PORT_4, HW_GPIO_PIN_6
 * bit 7: HW_GPIO_PORT_4, HW_GPIO_PIN_7
 *
 */
void ad_ftdf_dbg_bus_gpio_config(void);
#endif /* FTDF_DBG_BUS_ENABLE */

#endif /* AD_FTDF_MAC_API_H */

/**
 \}
 \}
 \}
 */

