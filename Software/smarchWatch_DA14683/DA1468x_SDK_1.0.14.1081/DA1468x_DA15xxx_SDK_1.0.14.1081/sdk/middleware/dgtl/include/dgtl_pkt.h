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
 * @file dgtl_pkt.h
 *
 * @brief Declarations for DGTL Packets API
 *
 * Copyright (C) 2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef DGTL_PKT_H_
#define DGTL_PKT_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief DGTL message packet type indicator
 */
typedef enum {
        DGTL_PKT_TYPE_HCI_CMD = 0x01,
        DGTL_PKT_TYPE_HCI_ACL = 0x02,
        DGTL_PKT_TYPE_HCI_SCO = 0x03,
        DGTL_PKT_TYPE_HCI_EVT = 0x04,
        DGTL_PKT_TYPE_GTL     = 0x05,
        DGTL_PKT_TYPE_APP_CMD = 0x06,
        DGTL_PKT_TYPE_APP_RSP = 0x07,
        DGTL_PKT_TYPE_LOG     = 0x08,
} dgtl_pkt_type_t;

typedef struct {
        uint8_t pkt_type;
        uint16_t opcode;
        uint8_t length;
        uint8_t parameters[0];
} __attribute__((packed)) dgtl_pkt_hci_cmd_t;

typedef struct {
        uint8_t pkt_type;
        uint16_t handle;
        uint16_t length;
        uint8_t parameters[0];
} __attribute__((packed)) dgtl_pkt_hci_acl_t;

typedef struct {
        uint8_t pkt_type;
        uint16_t handle;
        uint8_t length;
        uint8_t parameters[0];
} __attribute__((packed)) dgtl_pkt_hci_sco_t;

typedef struct {
        uint8_t pkt_type;
        uint8_t code;
        uint8_t length;
        uint8_t parameters[0];
} __attribute__((packed)) dgtl_pkt_hci_evt_t;

typedef struct {
        uint8_t pkt_type;
        uint16_t msg_id;
        uint16_t dst_task_id;
        uint16_t src_task_id;
        uint16_t length;
        uint8_t parameters[0];
} __attribute__((packed)) dgtl_pkt_gtl_t;

typedef struct {
        uint8_t pkt_type;
        uint16_t opcode;
        uint16_t length;
        uint8_t parameters[0];
} __attribute__((packed)) dgtl_pkt_app_cmd_t;

typedef struct {
        uint8_t pkt_type;
        uint8_t code;
        uint16_t length;
        uint8_t parameters[0];
} __attribute__((packed)) dgtl_pkt_app_rsp_t;

typedef struct {
        uint8_t pkt_type;
        uint8_t length;
        uint8_t parameters[0];
} __attribute__((packed)) dgtl_pkt_log_t;

typedef union {
        uint8_t pkt_type;
        dgtl_pkt_hci_cmd_t hci_cmd;
        dgtl_pkt_hci_acl_t hci_acl;
        dgtl_pkt_hci_sco_t hci_sco;
        dgtl_pkt_hci_evt_t hci_evt;
        dgtl_pkt_gtl_t     gtl;
        dgtl_pkt_app_cmd_t app_cmd;
        dgtl_pkt_app_rsp_t app_rsp;
        dgtl_pkt_log_t     log;
} __attribute__((packed)) dgtl_pkt_t;

/**
 * \brief Get header length of packet
 *
 * This function returns header length (including packet type indicator) of given packet. The packet
 * has to have at least \p pkt_type field initialized.
 *
 * \param [in] pkt  pointer to a packet buffer
 *
 * \return header length
 *
 */
static inline size_t dgtl_pkt_get_header_length(const dgtl_pkt_t *pkt)
{
        switch (pkt->pkt_type) {
        case DGTL_PKT_TYPE_HCI_CMD:
                return sizeof(dgtl_pkt_hci_cmd_t);
        case DGTL_PKT_TYPE_HCI_ACL:
                return sizeof(dgtl_pkt_hci_acl_t);
        case DGTL_PKT_TYPE_HCI_SCO:
                return sizeof(dgtl_pkt_hci_sco_t);
        case DGTL_PKT_TYPE_HCI_EVT:
                return sizeof(dgtl_pkt_hci_evt_t);
        case DGTL_PKT_TYPE_GTL:
                return sizeof(dgtl_pkt_gtl_t);
        case DGTL_PKT_TYPE_APP_CMD:
                return sizeof(dgtl_pkt_app_cmd_t);
        case DGTL_PKT_TYPE_APP_RSP:
                return sizeof(dgtl_pkt_app_rsp_t);
        case DGTL_PKT_TYPE_LOG:
                return sizeof(dgtl_pkt_log_t);
        }

        /* Unknown packet type, header length is not known */
        return 0;
}

/**
 * \brief Get parameters length of packet
 *
 * This function returns parameters length of given packet. The packet has to have header initialized
 * properly as otherwise return value in undefined.
 *
 * \param [in] pkt  pointer to a packet buffer
 *
 * \return parameters length
 *
 */
static inline size_t dgtl_pkt_get_param_length(const dgtl_pkt_t *pkt)
{
        switch (pkt->pkt_type) {
        case DGTL_PKT_TYPE_HCI_CMD:
                return pkt->hci_cmd.length;
        case DGTL_PKT_TYPE_HCI_ACL:
                return pkt->hci_acl.length;
        case DGTL_PKT_TYPE_HCI_SCO:
                return pkt->hci_sco.length;
        case DGTL_PKT_TYPE_HCI_EVT:
                return pkt->hci_evt.length;
        case DGTL_PKT_TYPE_GTL:
                return pkt->gtl.length;
        case DGTL_PKT_TYPE_APP_CMD:
                return pkt->app_cmd.length;
        case DGTL_PKT_TYPE_APP_RSP:
                return pkt->app_rsp.length;
        case DGTL_PKT_TYPE_LOG:
                return pkt->log.length;
        }

        /* Unknown packet type, parameters length is not known */
        return 0;
}

/**
 * \brief Get length of packet
 *
 * This function returns length of given packet. The packet has to have header initialized properly
 * as otherwise return value in undefined.
 *
 * \param [in] pkt  pointer to a packet buffer
 *
 * \return packet length
 *
 */
static inline size_t dgtl_pkt_get_length(const dgtl_pkt_t *pkt)
{
        return dgtl_pkt_get_header_length(pkt) + dgtl_pkt_get_param_length(pkt);
}

#ifdef __cplusplus
}
#endif

#endif /* DGTL_PKT_H_ */

/**
 * \}
 * \}
 * \}
 */
