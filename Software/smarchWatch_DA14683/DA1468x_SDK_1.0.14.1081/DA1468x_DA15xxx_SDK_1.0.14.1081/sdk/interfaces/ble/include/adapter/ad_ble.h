/**
 \addtogroup INTERFACES
 \{
 \addtogroup BLE
 \{
 \addtogroup ADAPTER
 \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_ble.h
 *
 * @brief BLE Adapter API
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef AD_BLE_H
#define AD_BLE_H

#include "osal.h"

#include "ad_ble_config.h"

#include "ad_nvparam.h"
#include "co_bt.h"

/* Event group bits */
#define mainBIT_BLE_GEN_IRQ             (1 << 0)
#define mainBIT_COMMAND_QUEUE           (1 << 1)
#define mainBIT_EVENT_QUEUE_AVAIL       (1 << 2)
#define mainBIT_EVENT_LPCLOCK_AVAIL     (1 << 3)
#define mainBIT_STAY_ACTIVE_UPDATED     (1 << 4)
#if (dg_configBLE_ADV_STOP_DELAY_ENABLE == 1)
#define mainBIT_EVENT_ADV_END           (1 << 31)
#endif

///Kernel message header length for transport through interface between App and SW stack.
#define HCI_CMD_HEADER_LENGTH      3
#define HCI_ACL_HEADER_LENGTH      4
#define HCI_SCO_HEADER_LENGTH      3
#define HCI_EVT_HEADER_LENGTH      2
#define GTL_MSG_HEADER_LENGTH      8

#define HCI_CMD_PARAM_LEN_OFFSET   3
#define HCI_ACL_PARAM_LEN_OFFSET   3
#define HCI_SCO_PARAM_LEN_OFFSET   3
#define HCI_EVT_PARAM_LEN_OFFSET   2
#define GTL_MSG_PARAM_LEN_OFFSET   7

#define HCI_RESET_CMD_OP_CODE      (0x0C03)

// Maximum wait time for BLE stack configuration operations
#define MAX_WAIT_TIME  portMAX_DELAY

// Operations for BLE adapter messages
typedef enum ad_ble_op_codes {
        AD_BLE_OP_CODE_STACK_MSG      = 0x00,
        AD_BLE_OP_CODE_ADAPTER_MSG    = 0x01,
        /*
         * last command ID
         * make sure above are sorted in ascending order, otherwise this one will be incorrect!
         */
        AD_BLE_OP_CODE_LAST,
} ad_ble_op_code_t;

// Operations for BLE adapter messages
typedef enum ad_ble_operations {
        AD_BLE_OP_CMP_EVT     = 0x00,
        AD_BLE_OP_INIT_CMD    = 0x01,
        /*
         * last command ID
         * make sure above are sorted in ascending order, otherwise this one will be incorrect!
         */
        AD_BLE_OP_LAST,
} ad_ble_operation_t;

// Statuses for BLE adapter operations
typedef enum ad_ble_statuses {
        AD_BLE_STATUS_NO_ERROR    = 0x00,
        AD_BLE_STATUS_TIMEOUT     = 0x01,
        /*
         * last error code for BLE adapter operations
         * make sure above are sorted in ascending order, otherwise this one will be incorrect!
         */
        BLE_ADAPTER_OP_LAST
} ad_ble_status_t;

// Statuses for BLE stack I/O callback operations
enum ble_stack_io_statuses {
        BLE_STACK_IO_OK    = 0x00,
        BLE_STACK_IO_ERROR = 0x01,
        /*
         * last error code for BLE stack I/O operations
         * make sure above are sorted in ascending order, otherwise this one will be incorrect!
         */
        BLE_STACK_IO_LAST
};

typedef enum {
        BLE_HCI_CMD_MSG  = 0x01,
        BLE_HCI_ACL_MSG  = 0x02,
        BLE_HCI_SCO_MSG  = 0x03,
        BLE_HCI_EVT_MSG  = 0x04,
        BLE_GTL_MSG      = 0x05,
#ifdef CONFIG_USE_FTDF
        FTDF_DTS_MSG     = 0xAA,
#endif
} ble_msg_type_t;

typedef uint16_t hci_cmd_op_code_t;
typedef uint16_t hci_cmd_op_code_t;

/** HCI command message header format */
typedef struct ble_hci_cmd_hdr {
        uint8_t op_code_l;
        uint8_t op_code_h;
        uint8_t data_length;
} ble_hci_cmd_hdr_t;

/** HCI ACL data message header format */
typedef struct ble_hci_acl_msg_hdr {
        uint16_t handle_flags;
        uint16_t data_length;
} ble_hci_acl_hdr_t;

/** HCI synchronous data message header format */
typedef struct ble_hci_sco_hdr {
        uint16_t conn_handle_flags;
        uint8_t data_length;
} ble_hci_sco_hdr_t;

/** HCI event message header format */
typedef struct ble_hci_evt_hdr {
        uint8_t event_code_;
        uint8_t data_length;
} ble_hci_evt_hdr_t;

/** HCI command message format */
typedef struct hci_cmd_msg {
        uint16_t op_code;
        uint8_t  param_length;
        uint8_t  param[0];
} hci_cmd_msg_t;

/** HCI ACL data message format */
typedef struct hci_acl_msg {
        uint16_t handle_flags;
        uint16_t param_length;
        uint8_t  param[0];
} hci_acl_msg_t;

/** HCI synchronous data message format */
typedef struct hci_sco_msg {
        uint16_t handle_flags;
        uint8_t  param_length;
        uint8_t  param[0];
} hci_sco_msg_t;

/** HCI event message format */
typedef struct hci_evt_msg {
        uint8_t event_code;
        uint8_t param_length;
        uint8_t param[0];
} hci_evt_msg_t;

/** HCI message format */
typedef struct ble_hci_msg {
        union {
                hci_cmd_msg_t cmd;
                hci_acl_msg_t acl;
                hci_sco_msg_t sco;
                hci_evt_msg_t evt;
        };
} ble_hci_msg_t;

/** GTL message format */
typedef struct ble_gtl_msg {
        uint16_t msg_id;
        uint16_t dest_id;
        uint16_t src_id;
        uint16_t param_length;
        uint32_t param[0];
} ble_gtl_msg_t;

/** BLE stack message structure */
typedef struct ble_stack_msg {
        union {
                ble_gtl_msg_t gtl;
                ble_hci_msg_t hci;
        };
} ble_stack_msg_t;

/** BLE adapter message structure */
typedef struct ad_ble_msg {
        uint16_t            op_code;
        uint16_t            msg_size;
        ad_ble_operation_t  operation;
        uint8_t             param[1];
} ad_ble_msg_t;

/** BLE adapter message header structure */
typedef struct ad_ble_hdr {
        uint16_t    op_code;
        uint16_t    msg_size;
        uint8_t     param[0];
} ad_ble_hdr_t;

/** BLE Adapter interface */
typedef struct {
        OS_TASK  task;     /**< BLE Adapter task handle */
        OS_QUEUE cmd_q;    /**< BLE Adapter command queue */
        OS_QUEUE evt_q;    /**< BLE Adapter event queue */
} ad_ble_interface_t;

/**
 * \brief Send a message to the BLE adapter command queue
 *
 * Sends a message to the BLE adapter command queue and notifies the BLE adapter task.
 *
 * \param[in] item       Pointer to the item to be sent to the queue.
 * \param[in] wait_ticks Max time in ticks to wait for space in queue.
 *
 * \return OS_OK if the message was successfully sent to the queue, OS_FAIL otherwise.
 */
OS_BASE_TYPE ad_ble_command_queue_send( const void *item, OS_TICK_TIME wait_ticks);

/**
 * \brief Notify the BLE adapter that LP clock is available
 *
 * This will send a notification to the BLE adapter informing it for the availability of the LP clock.
 * From that moment onwards the BLE stack is allowed to enter the sleep state.
 *
 */
void ad_ble_lpclock_available(void);

/**
 * \brief Send a message to the BLE adapter event queue.
 *
 * Sends a message to the BLE adapter event queue and notifies the registered task.
 *
 * \param[in] item       Pointer to the item to be sent to the queue.
 * \param[in] wait_ticks Max time in ticks to wait for space in queue
 *
 * \return OS_OK if the message was successfully sent to the queue, OS_FAIL otherwise.
 */
OS_BASE_TYPE ad_ble_event_queue_send( const void *item, OS_TICK_TIME wait_ticks);

void ad_ble_task_notify_from_isr(uint32_t value);

/**
 * \brief Initialize BLE adapter - create command and event queues.
 *
 */
void ad_ble_init(void);

/**
 * \brief Get BLE Adapter interface
 *
 * \return adapter interface pointer
 *
 */
const ad_ble_interface_t *ad_ble_get_interface(void);

/**
 * \brief Register task for BLE Adapter's event Queue notifications.
 *
 * \param[in] task_handle The handle of the FreeRTOS task that registers for notifications.
 *
 * \return pdPASS if task is successfully registered, pdFAIL otherwise.
 */
OS_BASE_TYPE ad_ble_event_queue_register(const OS_TASK task_handle);

/**
 * \brief Get public static address
 *
 * This will be either the address read from NVMS or the default address.
 * Since this address does not change once it has been loaded, it's safe to call it at any time
 * from any task
 *
 *\param[out] address Public static address
 */
void ad_ble_get_public_address(uint8_t address[BD_ADDR_LEN]);

/**
 * \brief Get device's IRK
 *
 * This will be either the IRK read from NVMS or the default IRK.
 *
 ** \param[out] irk Device's IRK
 */
void ad_ble_get_irk(uint8_t irk[KEY_LEN]);

/**
 * \brief Notifies BLE adapter that there is free space on the event queue
 */
void ad_ble_notify_event_queue_avail(void);

/**
 * \brief Get non-volatile parameter handle
 *
 * Function gets parameter handle to be used
 *
 * \return valid parameter handle on success, NULL otherwise
 */
#if dg_configNVPARAM_ADAPTER
nvparam_t ad_ble_get_nvparam_handle(void);
#endif /* dg_configNVPARAM_ADAPTER */

/**
 * \brief Force BLE to stay active
 *
 * Forcing BLE to stay active could be helpful in periods with notable BLE traffic.
 * This will result to reduced interrupt latencies, as BLE is not going to
 * wake up too often due to the expected traffic load.
 *
 * \param [in] status defines if BLE sleep in allowed or not
 */
void ad_ble_stay_active(bool status);

/*
 * \brief Unblock BLE adapter to process new messages generated from IRQ context.
 */
void ad_ble_notify_gen_irq(void);

/*
 * \brief Check if the non retention BLE heap is in use.
 */
bool ad_ble_non_retention_heap_in_use(void);

#endif /* AD_BLE_H */
/**
 \}
 \}
 \}
 */
