/**
 ****************************************************************************************
 *
 * @file hci.h
 *
 * @brief This file contains definitions related to the HCI module.
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef HCI_H_
#define HCI_H_

/**
 ****************************************************************************************
 * @addtogroup HCI Host Controller Interface
 * @ingroup ROOT
 * @brief HCI module handling communication between lower and higher layers in split
 * architecture.
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"       // SW configuration

#if (HCI_PRESENT)

#include <stddef.h>          // standard definition
#include <stdint.h>          // standard integer
#include "co_bt.h"           // BT standard definitions

#include "ke_task.h"            // SW main module

/*
 * DEFINES
 ****************************************************************************************
 */

#if (BLE_EMB_PRESENT || BLE_HOST_PRESENT)
#if (BLE_CENTRAL || BLE_PERIPHERAL)
#define HCI_BLE_CON_SUPPORT   1
#else // (BLE_CENTRAL || BLE_PERIPHERAL)
#define HCI_BLE_CON_SUPPORT   0
#endif // (BLE_CENTRAL || BLE_PERIPHERAL)
#else //(BLE_EMB_PRESENT || BLE_HOST_PRESENT)
#define HCI_BLE_CON_SUPPORT   0
#endif //(BLE_EMB_PRESENT || BLE_HOST_PRESENT)

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Message API of the HCI task
enum HCI_MSG
{
    HCI_MSG_ID_FIRST = KE_FIRST_MSG(TASK_ID_HCI),

    HCI_CMD_CMP_EVENT,
    HCI_CMD_STAT_EVENT,
    HCI_EVENT,
    HCI_LE_EVENT,

    HCI_COMMAND,

    #if (HCI_BLE_CON_SUPPORT)
    HCI_ACL_DATA_RX,
    HCI_ACL_DATA_TX,
    #endif // (HCI_BLE_CON_SUPPORT)

    #if BT_EMB_PRESENT
    HCI_BT_ACL_DATA_TX,
    HCI_BT_ACL_DATA_RX,
    #endif //BT_EMB_PRESENT

    HCI_MSG_ID_LAST
};

/// Status of HCI command header processing
enum HCI_CMD_HDR
{
    /// Header is correct
    HCI_CMD_HDR_STATUS_OK,
    /// Opcode is unknown
    HCI_CMD_HDR_STATUS_UNKNOWN,
    /// Header is not correct
    HCI_CMD_HDR_STATUS_FAIL
};

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
****************************************************************************************
* @brief Initialize HCI (including transport)
*****************************************************************************************
*/
void hci_init(void);

/**
****************************************************************************************
* @brief Reset HCI
*****************************************************************************************
*/
void hci_reset(void);

#if (BLE_EMB_PRESENT || BT_EMB_PRESENT)
/**
 ****************************************************************************************
 * @brief Function called when an internal task needs to send a HCI message to Host
 *
 * This function decides whether the message is sent externally onto HCI Transport Layer
 * or redirected into an internal task of the other side of the HCI.
 *
 * The input message ID, length and parameters must be filled.
 * In case the message is an HCI command or event, the source ID must be filled with the
 * command opcode or event code.
 * In case the message concerns a particular BT or BLE link, the destination ID must be
 * filled with the associated link ID.
 *
 * @param[in]  param   Pointer to the parameters of the Kernel message carrying the HCI message
 *****************************************************************************************
 */
void hci_send_2_host(void *param);
#endif // (BLE_EMB_PRESENT || BT_EMB_PRESENT)

#if BLE_HOST_PRESENT
/**
 ****************************************************************************************
 * @brief Function called when an internal task needs to send a HCI message to Controller
 *
 * This function decides whether the message is sent externally onto HCI Transport Layer
 * or redirected into an internal task of the other side of the HCI.
 *
 * The input message ID, length and parameters must be filled.
 * In case the message is an HCI command or event, the source ID must be filled with the
 * command opcode or event code.
 * In case the message concerns a particular BT or BLE link, the destination ID must be
 * filled with the associated link ID.
 *
 * @param[in]  param   Pointer to the parameters of the Kernel message carrying the HCI message
 *****************************************************************************************
 */
void hci_send_2_controller(void *param);
#endif //BLE_HOST_PRESENT

#if  (BT_EMB_PRESENT)
/**
 ****************************************************************************************
 * @brief Register BD address for a BT ACL connection
 *
 * @param[in]  link_id  BT ACL connection link ID
 * @param[in]  bd_addr  Pointer to the device BD address associated to the connection
 *****************************************************************************************
 */
void hci_bt_acl_bdaddr_register(uint8_t link_id, struct bd_addr* bd_addr);
/**
 ****************************************************************************************
 * @brief Register connection handle for a BT ACL connection
 *
 * @param[in]  link_id  BT ACL connection link ID
 *****************************************************************************************
 */
void hci_bt_acl_conhdl_register(uint8_t link_id);

/**
 ****************************************************************************************
 * @brief Unregister a BT ACL connection
 *
 * @param[in]  link_id  BT ACL connection link ID
 *****************************************************************************************
 */
void hci_bt_acl_bdaddr_unregister(uint8_t link_id);
#endif //(BT_EMB_PRESENT)

/**
 ****************************************************************************************
 * @brief Set the event mask
 *
 * @param[in] evt_msk Pointer to the new event mask
 * @param[in] page indicate which event page should be changed
 *
 * @return The status of the event mask saving
 *****************************************************************************************
 */
uint8_t hci_evt_mask_set(struct evt_mask const *evt_msk, uint8_t page);

#if (BT_EMB_PRESENT)
/**
 ****************************************************************************************
 * @brief Add an event filter according to the parameters of the HCI command
 *
 * Note: the consistency of the parameters according to the input has already been checked by HCI during the special
 * unpacking.
 *
 * @param[in] param      Pointer to the HCI parameter
 *
 * @return The status of the filter addition
 *****************************************************************************************
 */
uint8_t hci_evt_filter_add(struct hci_set_evt_filter_cmd const *param);
#endif //(BT_EMB_PRESENT)

#if (TL_ITF)
#if  (BLE_EMB_PRESENT || BT_EMB_PRESENT)
/**
 ****************************************************************************************
 * @brief Get the maximum parameter size for a specific command
 *
 * This function is used by TL to know the theoretical maximum parameters size for a
 * specific HCI command.
 * Note: if the command is not supported by HCI (unknown), the maximum possible value of
 * 255 bytes is returned.
 *
 * @param[in]  opcode  Opcode received
 *
 * @return     The command maximum parameters size / 255 if command is unknown
 *****************************************************************************************
 */
uint8_t hci_cmd_get_max_param_size(uint16_t opcode);

/**
 ****************************************************************************************
 * @brief Indicates that a HCI command has been received
 *
 * This function is used by TL to indicate the reception of a HCI command.
 *
 * @param[in]  opcode   Command Opcode
 * @param[in]  length   Parameters length
 * @param[in]  payload  Pointer to payload
 *****************************************************************************************
 */
void hci_cmd_received(uint16_t opcode, uint8_t length, uint8_t *payload);

/**
 ****************************************************************************************
 * @brief Allocates the reception buffer for ACL TX data
 *
 * @param[in]   hdl_flags Connection handle and data flags from HCI ACL packet header
 * @param[in]   len       Length to receive (from HCI ACL packet header)
 *
 * @return   Buffer for data reception (NULL if not possible to allocate one)
 *****************************************************************************************
 */
uint8_t* hci_acl_tx_data_alloc(uint16_t hdl_flags, uint16_t len);

/**
 ****************************************************************************************
 * @brief Indicates that a HCI ACL TX data packet has been received
 *
 * This function is used by TL to indicate the reception of a HCI ACL TX data.
 *
 * @param[in]   hdl_flags Connection handle and data flags from HCI ACL packet header
 * @param[out]  datalen   Data length
 * @param[in]   payload   Pointer to payload
 *****************************************************************************************
 */
void hci_acl_tx_data_received(uint16_t hdl_flags, uint16_t datalen, uint8_t * payload);
#endif // (BLE_EMB_PRESENT || BT_EMB_PRESENT)

#if BLE_HOST_PRESENT
/**
 ****************************************************************************************
 * @brief Allocates the reception buffer for ACL RX data
 *
 * @param[in]   hdl_flags Connection handle and data flags from HCI ACL RX packet header
 * @param[in]   len       Length to receive (from HCI ACL packet header)
 *
 * @return   Buffer for data reception (NULL if not possible to allocate one)
 *****************************************************************************************
 */
uint8_t* hci_acl_rx_data_alloc(uint16_t hdl_flags, uint16_t len);

/**
 ****************************************************************************************
 * @brief Indicates that a HCI ACL RX data packet has been received
 *
 * This function is used by TL to indicate the reception of a HCI ACL RX data.
 *
 * @param[in]   hdl_flags Connection handle and data flags from HCI ACL packet header
 * @param[out]  datalen   Data length
 * @param[in]   payload   Pointer to payload
 *****************************************************************************************
 */
void hci_acl_rx_data_received(uint16_t hdl_flags, uint16_t datalen, uint8_t * payload);

/**
 ****************************************************************************************
 * @brief Indicates that a HCI event has been received
 *
 * This function is used by TL to indicate the reception of a HCI event.
 *
 * @param[in]  code     Event code
 * @param[in]  length   Parameters length
 * @param[in]  payload  Pointer to payload
 *
 * @return status of receive operation
 *****************************************************************************************
 */
uint8_t hci_evt_received(uint8_t code, uint8_t length, uint8_t *payload);
#endif //BLE_HOST_PRESENT
#endif // TL_ITF

#endif //HCI_PRESENT

/// @} HCI

#endif // HCI_H_
