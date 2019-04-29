/**
 ****************************************************************************************
 *
 * @file ad_ble.c
 *
 * @brief BLE FreeRTOS Adapter
 *
 * Copyright (C) 2015-2018 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifdef CONFIG_USE_BLE
#include <string.h>

#include "co_version.h"
#include "ble_config.h"

#include "osal.h"
#include "queue.h"
#include "event_groups.h"
#if (BLE_WINDOW_STATISTICS == 1) || (BLE_SLEEP_PERIOD_DEBUG == 1)
#include "logging.h"
#endif

#include "sys_clock_mgr.h"
#include "sys_power_mgr.h"
#include "sys_rtc.h"
#include "sys_watchdog.h"

#include "hw_gpio.h"
#include "hw_rf.h"
#include "hw_cpm.h"

#include "ad_nvms.h"

#include "ad_nvparam.h"
#include "platform_nvparam.h"

#include "ad_ble.h"
#include "ad_ble_msg.h"
#include "ble_config.h"
#include "ble_common.h"
#include "ble_mgr.h"
#include "ble_mgr_common.h"
#include "ble_mgr_ad_msg.h"
#include "ble_mgr_gtl.h"
#if (dg_configBLE_ADV_STOP_DELAY_ENABLE == 1)
#include "../../include/util/list.h"
#include "rwble.h"
#endif

#include "rwip_config.h"
#include "reg_blecore.h"

#include "gapm_task.h"
#include "hw_rf.h"
#include "ad_rf.h"

#include "window_stats.h"

#ifdef BLE_PROD_TEST
#include "ble_prod_test.h"
#endif

#if (BLE_ADAPTER_DEBUG == 1)
#pragma message "BLE Adapter: GPIO debugging is on!"
#endif

#if (dg_configNVMS_ADAPTER == 0)
#pragma message "NVMS Adapter is disabled. BLE device is going to use default BD_ADDR and IRK."
#endif

#if (dg_configSYSTEMVIEW)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_BLE_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_BLE_ISR_EXIT()
#endif

/*------------------------------------- Local definitions ----------------------------------------*/

#define NVMS_PARAMS_TAG_BD_ADDRESS_OFFSET       0x0000
#define NVMS_PARAMS_TAG_IRK_OFFSET              0x0024

/* Task stack size */
#define mainBLE_TASK_STACK_SIZE 1024

/* Task priorities */
#define mainBLE_TASK_PRIORITY           ( OS_TASK_PRIORITY_HIGHEST - 3 )

/* BLE manager event group bits */
#define mainBIT_EVENT_QUEUE_TO_MGR      (1 << 1)

typedef enum {
        BLE_ACTIVE = 0,
        BLE_SLEEPING,
        BLE_WAKING_UP
} eSleepStatus;

#if (dg_configBLE_ADV_STOP_DELAY_ENABLE == 1)
/* Delayed list element */
struct delayed_msg {
        struct delayed_msg  *next;
        ble_mgr_common_stack_msg_t *msg;
};
#endif

/*------------------------------------- Local variables ------------------------------------------*/

static eSleepStatus sleep_status;       // BLE_ACTIVE is the initial value
PRIVILEGED_DATA static bool stay_active; // Disabled by default
PRIVILEGED_DATA static bool sleep_for_ever;
PRIVILEGED_DATA static ad_ble_op_code_t current_op;

PRIVILEGED_DATA static ad_ble_interface_t adapter_if;
PRIVILEGED_DATA static OS_TASK mgr_task = NULL;

#ifndef CONFIG_USE_FTDF
/* Notification flag to indicate an RX operation was performed, in order
 * to perform the RX dc offset calib check (and possible recovery)
 */
PRIVILEGED_DATA bool rf_dcoffset_failure;
#endif

/* BLE stack I/O RX buffer pointer */
PRIVILEGED_DATA static uint8_t* ble_stack_io_rx_buf_ptr;
/* BLE stack I/O RX size */
PRIVILEGED_DATA static uint32_t ble_stack_io_rx_size_req;
/* BLE stack I/O RX done callback */
PRIVILEGED_DATA static void (*ble_stack_io_rx_done_cb) (uint8_t);
/* BLE stack I/O TX done callback */
PRIVILEGED_DATA static void (*ble_stack_io_tx_done_cb) (uint8_t);
#if (dg_configBLE_ADV_STOP_DELAY_ENABLE == 1)
/* Advertising flag */
PRIVILEGED_DATA static bool advertising;
/* Wait for event flag */
PRIVILEGED_DATA static bool waiting_for_evt;
/* Delayed message list */
PRIVILEGED_DATA static void *delayed_list;
#endif

PRIVILEGED_DATA static uint8_t public_address[BD_ADDR_LEN];

#if dg_configNVPARAM_ADAPTER
/* Global BLE NV-Parameter handle */
PRIVILEGED_DATA static nvparam_t ble_parameters = NULL;
#endif

PRIVILEGED_DATA bool ble_stack_initialized;
/*------------------------------------- Prototypes -----------------------------------------------*/

/* BLE stack internal scheduler */
void rwip_schedule(void);

/* BLE wake-up interrupt service routine */
void ble_lp_isr(void);

/* BLE stack generic interrupt service routine */
void rwble_isr_sdk(void);

/* BLE stack main function */
void ble_stack_init(void);

/* BLE sleep function */
int ble_sleep(bool forever, uint32_t *sleep_duration_in_lp_cycles);

/* BLE check block function */
bool ble_block(void);

/* Forced wake-up of the BLE core. */
bool ble_force_wakeup(void);

/* BLE platform initialization function */
void ble_platform_initialization(void);

/*------------------------------------------ Prototypes ------------------------------------------*/

/**
 * \brief Send a message to the BLE stack
 *
 * \param[in] ptr_msg  Pointer to the BLE stack message buffer.
 */
static void ad_ble_send_to_stack(const ble_mgr_common_stack_msg_t *ptr_msg);

/**
 * \brief Handle a BLE stack message.
 *
 * \param [in] msg Pointer to the message to be handled.
 */
static void ad_ble_handle_stack_msg(ble_mgr_common_stack_msg_t *msg);

/**
 * \brief Handle a BLE adapter configuration message.
 *
 * \param [in] msg Pointer to the message to be handled.
 */
static void ad_ble_handle_adapter_msg(ad_ble_msg_t *msg);

bool ke_mem_is_empty(uint8_t type);
/*--------------------------------------- Global variables ---------------------------------------*/


/*--------------------------------------- Local functions  ---------------------------------------*/
void ad_ble_notify_gen_irq(void)
{
        OS_TASK_NOTIFY_FROM_ISR(adapter_if.task, mainBIT_BLE_GEN_IRQ, OS_NOTIFY_SET_BITS);
}

void ad_ble_cscnt_serviced(void)
{
        sleep_status = BLE_ACTIVE;

        pm_resource_is_awake(PM_BLE_ID);

        DBG_SET_HIGH(BLE_ADAPTER_DEBUG, BLEBDG_ADAPTER);
}

void ad_ble_finegtim_serviced(void)
{
        sleep_status = BLE_ACTIVE;
}

/**
 * \brief ble_gen_irq interrupt service routine
 */
__RETAINED_CODE void BLE_GEN_Handler(void)
{
        SEGGER_SYSTEMVIEW_BLE_ISR_ENTER();

        DBG_SET_HIGH(BLE_USE_TIMING_DEBUG, CPMDBG_BLE_IRQ);

        // Call RW stack interrupt service routine
        rwble_isr_sdk();

        // Notify BLE task of the interrupt
        ad_ble_notify_gen_irq();

        DBG_SET_LOW(BLE_USE_TIMING_DEBUG, CPMDBG_BLE_IRQ);

        SEGGER_SYSTEMVIEW_BLE_ISR_EXIT();
}

/**
 * \brief ble_wakup_irq interrupt service routine
 */
__RETAINED_CODE void BLE_WAKEUP_LP_Handler(void)
{
        SEGGER_SYSTEMVIEW_BLE_ISR_ENTER();

        DBG_SET_HIGH(BLE_USE_TIMING_DEBUG, CPMDBG_BLE_IRQ);

        /*
         * Switch to XTAL 16MHz
         */
        if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE) {
                // Make sure that the XTAL16M has already settled
                ASSERT_WARNING(cm_poll_xtal16m_ready());
        }
        else {
                // Block if the XTAL16M has not settled until now
                while (!cm_poll_xtal16m_ready());
        }

        /* System clock must NOT be RC16 for BLE to work */
        ASSERT_WARNING(hw_cpm_get_sysclk() != SYS_CLK_IS_RC16);

        sleep_status = BLE_WAKING_UP;

        ble_lp_isr();

        SEGGER_SYSTEMVIEW_BLE_ISR_EXIT();
}

void ad_ble_lpclock_available(void)
{
        if (adapter_if.task) {
               OS_TASK_NOTIFY(adapter_if.task, mainBIT_EVENT_LPCLOCK_AVAIL, OS_NOTIFY_SET_BITS);
        }
}

OS_BASE_TYPE ad_ble_command_queue_send( const void *item, OS_TICK_TIME wait_ticks)
{
        if (OS_QUEUE_PUT(adapter_if.cmd_q, item, wait_ticks) != OS_OK) {
                return OS_FAIL;
        }
        OS_TASK_NOTIFY(adapter_if.task, mainBIT_COMMAND_QUEUE, OS_NOTIFY_SET_BITS);

        return OS_OK;
}

OS_BASE_TYPE ad_ble_event_queue_send( const void *item, OS_TICK_TIME wait_ticks)
{
        if (OS_QUEUE_PUT(adapter_if.evt_q, item, wait_ticks) != OS_OK) {
                return OS_FAIL;
        }
        OS_TASK_NOTIFY(mgr_task, mainBIT_EVENT_QUEUE_TO_MGR, OS_NOTIFY_SET_BITS);

        return OS_OK;
}

void ad_ble_notify_event_queue_avail(void)
{
        OS_TASK_NOTIFY(adapter_if.task, mainBIT_EVENT_QUEUE_AVAIL, OS_NOTIFY_SET_BITS);
}

void ad_ble_task_notify_from_isr(uint32_t value)
{
        OS_TASK_NOTIFY_FROM_ISR(adapter_if.task, value, OS_NOTIFY_SET_BITS);
}

bool ad_ble_non_retention_heap_in_use(void)
{
        if (!ble_stack_initialized) {
                return false;
        } else {
                return !ke_mem_is_empty(KE_MEM_NON_RETENTION);
        }
}

/**
 * \brief Wake-up the BLE block.
 */
static void ad_ble_wake_up(void)
{
        /* Switch to XTAL16 if RC16 is used. */
        if (cm_sys_clk_get() == sysclk_RC16) {
                cm_wait_xtal16m_ready();
        }

        /* Wake up the BLE core. */
        ble_force_wakeup();
}

#if (BLE_SLEEP_PERIOD_DEBUG == 1)
extern uint32_t logged_sleep_duration;
extern uint32_t retained_slp_duration;
#endif

/**
 * \brief Check if the BLE core can enter sleep and, if so, enter sleep.
 *
 * \return The status of the requested operation.
 *         <ul>
 *             <li> 0, if the BLE core cannot sleep
 *             <li> 1, if the BLE core was put to sleep
 *             <li> other, if the BLE core has to stay active but the caller may block
 *         </ul>
 */
static int sleep_when_possible(void)
{
        uint32_t sleep_duration_in_lp_cycles;
        int ret = 0;

        do {
                if (sleep_status != BLE_ACTIVE) {
                        break;
                }

                // Hook to cancel BLE sleep
#ifdef dg_configBLE_HOOK_BLOCK_SLEEP
                if (dg_configBLE_HOOK_BLOCK_SLEEP() != 0) {
                        if (ble_block()) {
                                // There are no pending BLE actions
                                ret = -1;
                        }
                        break;
                }
#endif

                OS_ENTER_CRITICAL_SECTION();
                ret = ble_sleep(sleep_for_ever, &sleep_duration_in_lp_cycles);
                if (ret == 1) {
                        sleep_status = BLE_SLEEPING;

                        DBG_SET_LOW(BLE_ADAPTER_DEBUG, BLEBDG_ADAPTER);

                        hw_rf_request_off(true);

#if (BLE_SLEEP_PERIOD_DEBUG == 1)
                        log_printf(LOG_NOTICE, 3, "\tSleep period: %d (in ticks: %d)\r\n",
                                logged_sleep_duration, retained_slp_duration);
#endif
                }
                OS_LEAVE_CRITICAL_SECTION();
        } while (0);

        return ret;
}

#ifndef BLE_PROD_TEST
static bool ad_ble_read_nvms_param(uint8_t* param, uint8_t len, uint8_t nvparam_tag, uint32_t nvms_addr)
{
#if (dg_configNVMS_ADAPTER == 1)
#if (dg_configNVPARAM_ADAPTER == 1)
        uint16_t read_len = 0;
        uint16_t param_len;
        uint8_t valid;

        /* Parameter length shall be long enough to store address and validity flag */
        param_len = ad_nvparam_get_length(ble_parameters, nvparam_tag, NULL);
        if (param_len == len + sizeof(valid)) {
                ad_nvparam_read_offset(ble_parameters, nvparam_tag,
                                                len, sizeof(valid), &valid);

                /* Read param from nvparam only if validity flag is set to 0x00 and read_len is correct */
                if (valid == 0x00) {
                        read_len = ad_nvparam_read(ble_parameters, nvparam_tag,
                                len, param);
                        if (read_len == len) {
                                return true; /* Success */
                        }
                }
        }
#else
        nvms_t nvms;
        int i;

        nvms = ad_nvms_open(NVMS_PARAM_PART);

        ad_nvms_read(nvms, nvms_addr, (uint8_t *) param, len);

        for (i = 0; i < len; i++) {
                if (param[i] != 0xFF) {
                        return true; /* Success */
                }
        }
#endif /* (dg_configNVPARAM_ADAPTER == 1) */
#endif /* (dg_configNVMS_ADAPTER == 1) */

        return false; /* Failure */
}
#endif

void read_public_address()
{
        uint8_t default_addr[BD_ADDR_LEN] = defaultBLE_STATIC_ADDRESS;

#ifdef BLE_PROD_TEST
        memcpy(public_address, &default_addr, sizeof(public_address));
#else
        bool valid;

        valid = ad_ble_read_nvms_param(public_address, BD_ADDR_LEN, TAG_BLE_PLATFORM_BD_ADDRESS,
                                                                NVMS_PARAMS_TAG_BD_ADDRESS_OFFSET);
        if (!valid) {
                memcpy(public_address, &default_addr, BD_ADDR_LEN);
        }
#endif
}

/**
 * \brief Main BLE Interrupt and event queue handling task
 */
static void ad_ble_task(void *pvParameters)
{
        ad_ble_hdr_t *received_msg;
        uint32_t ulNotifiedValue;
        OS_BASE_TYPE xResult __attribute__((unused));
        int ret;
        int8_t wdog_id;

#if dg_configNVPARAM_ADAPTER
        /* Open BLE NV-Parameters - area name is defined in platform_nvparam.h */
        ble_parameters = ad_nvparam_open("ble_platform");
#endif

        read_public_address();

        /* Register task to be monitored by watch dog. */
        wdog_id = sys_watchdog_register(false);

        DBG_SET_HIGH(BLE_ADAPTER_DEBUG, BLEBDG_ADAPTER); // Debug LED active (i.e. not sleeping)

        /* Run BLE stack internal scheduler once before entering task's main function. */
        rwip_schedule();

        sleep_for_ever = true;

        for (;;) {
                /* Notify watch dog on each loop since there's no other trigger for this. */
                sys_watchdog_notify(wdog_id);

                /* Suspend monitoring while task is blocked on OS_TASK_NOTIFY_WAIT(). */
                sys_watchdog_suspend(wdog_id);

                /*
                 * Wait on any of the event group bits, then clear them all.
                 */
                xResult = OS_TASK_NOTIFY_WAIT(0x0, OS_TASK_NOTIFY_ALL_BITS, &ulNotifiedValue,
                                                                            OS_TASK_NOTIFY_FOREVER);
                /* Guaranteed to succeed since we're waiting forever for the notification */
                OS_ASSERT(xResult == OS_OK);

                /* Resume watch dog monitoring. */
                sys_watchdog_notify_and_resume(wdog_id);

                /* Check if we should call the previously skipped TX done callback. */
                if (ble_stack_io_tx_done_cb && uxQueueSpacesAvailable(adapter_if.evt_q)) {
                        /* Call BLE stack TX done callback. */
                        ble_stack_io_tx_done_cb(BLE_STACK_IO_OK);

                        /* Reset BLE stack TX done callback pointer. */
                        ble_stack_io_tx_done_cb = NULL;

                        ble_mgr_notify_adapter_blocked(false);
                }
#if (dg_configBLE_ADV_STOP_DELAY_ENABLE == 1)
                if (ulNotifiedValue & mainBIT_EVENT_ADV_END) {
                        struct delayed_msg *d_msg;

                        /* Don't delay next commands. */
                        waiting_for_evt = false;

                        /* Disable end of advertising event notifications. */
                        rwble_evt_end_adv_ntf_set(false);

                        /* Run stack scheduler. */
                        rwip_schedule();

                        /* Send delayed messages to stack. */
                        do {
                                d_msg = list_pop_back(&delayed_list);

                                if (d_msg) {
                                        /* Send delayed message to stack */
                                        ad_ble_send_to_stack(d_msg->msg);

                                        /* Free previously allocated message buffer. */
                                        OS_FREE(d_msg->msg);

                                        /* Free allocated list element. */
                                        OS_FREE(d_msg);
                                }
                        } while (d_msg != NULL);
                }
#endif

                if (ulNotifiedValue & mainBIT_COMMAND_QUEUE) {
                        /* The message may have already been read in the while () loop below! */
                        if ( OS_QUEUE_GET(adapter_if.cmd_q, &received_msg, 0)) {
                                /* Make sure a valid OP CODE is received */
                                OS_ASSERT(received_msg->op_code < AD_BLE_OP_CODE_LAST);
                                current_op = received_msg->op_code;

                                if (current_op == AD_BLE_OP_CODE_STACK_MSG) {
                                        /* Send message to BLE stack */
                                        ble_mgr_common_stack_msg_t *stack_msg = (ble_mgr_common_stack_msg_t *) received_msg;
#ifdef BLE_PROD_TEST
                                        if (stack_msg->msg_type == HCI_CMD_MSG &&
                                                stack_msg->msg.hci.cmd.op_code >= 0xFC80) {
                                                ble_prod_test_cmd(stack_msg);

                                                /* Free previously allocated message buffer. */
                                                OS_FREE(stack_msg);
                                        } else {
#endif
                                                ad_ble_handle_stack_msg(stack_msg);
#ifdef BLE_PROD_TEST
                                        }
#endif
                                }
                                else if (current_op == AD_BLE_OP_CODE_ADAPTER_MSG) {
                                        ad_ble_handle_adapter_msg((ad_ble_msg_t *) received_msg);

                                        /* Free previously allocated message buffer. */
                                        OS_FREE(received_msg);
                                }

                                if (sleep_status == BLE_SLEEPING) {
                                        ad_ble_wake_up();
                                }
                        }
                }

                if (ulNotifiedValue & mainBIT_EVENT_LPCLOCK_AVAIL) {
                     /* LP Clock is available, check if BLE sleep is possible */
                }

                if (ulNotifiedValue & mainBIT_STAY_ACTIVE_UPDATED) {
                        /* BLE's stay_active status updated */
                        if (stay_active && (sleep_status == BLE_SLEEPING)) {
                                ad_ble_wake_up();
                        }
                }

                ret = 0;

                /* Run this loop as long as BLE is active and there are pending BLE actions */
                while ((sleep_status == BLE_ACTIVE) && (ret != -1)) {
#ifdef BLE_PROD_TEST
                        ble_prod_test_check_tx_packet_count();
#endif
                        /* Run the BLE stack internal scheduler. */
                        rwip_schedule();

                        /* If RX DC offset partial calibration has failed, do a
                         * full DCOC
                         */
#ifndef CONFIG_USE_FTDF
                        OS_ENTER_CRITICAL_SECTION();
                        if (rf_dcoffset_failure) {
                                /* force RADIO_BUSY to BLE to 0 */
                                REG_SETF(COEX, COEX_CTRL_REG, SEL_BLE_RADIO_BUSY, 1);
                                /* Ignore BLE TX/RX_EN */
                                REG_SET_BIT(COEX, COEX_CTRL_REG, IGNORE_BLE);
                                /* Wait until DCFs have settled */
                                while (REG_GETF(COEX, COEX_STAT_REG, COEX_RADIO_BUSY));
                                /* Perform full dc offset calibration */
                                hw_rf_dc_offset_calibration();
                                /* Restore state to normal BLE MAC operation */
                                REG_CLR_BIT(COEX, COEX_CTRL_REG, IGNORE_BLE);
                                REG_SETF(COEX, COEX_CTRL_REG, SEL_BLE_RADIO_BUSY, 0);
                                rf_dcoffset_failure = false;
                        }
                        OS_LEAVE_CRITICAL_SECTION();
#endif

                        /* Check command queue for incoming messages */
                        if (uxQueueMessagesWaiting(adapter_if.cmd_q)) {
                                /* Get message from the command queue. */
                                OS_QUEUE_GET(adapter_if.cmd_q, &received_msg, 0);
                                /* Make sure a valid op code is received */
                                OS_ASSERT(received_msg->op_code < AD_BLE_OP_CODE_LAST);

                                /* Save message's OP code. */
                                current_op = received_msg->op_code;

                                if (current_op == AD_BLE_OP_CODE_STACK_MSG) {
                                        /* Send message to BLE stack */
                                        ble_mgr_common_stack_msg_t *stack_msg = (ble_mgr_common_stack_msg_t *) received_msg;
#ifdef BLE_PROD_TEST
                                        if (stack_msg->msg_type == HCI_CMD_MSG &&
                                                stack_msg->msg.hci.cmd.op_code >= 0xFC80) {
                                                ble_prod_test_cmd(stack_msg);

                                                /* Free previously allocated message buffer. */
                                                OS_FREE(stack_msg);
                                        } else {
#endif
                                                ad_ble_handle_stack_msg(stack_msg);
#ifdef BLE_PROD_TEST
                                        }
#endif
                                }
                                else if (current_op == AD_BLE_OP_CODE_ADAPTER_MSG) {
                                        ad_ble_handle_adapter_msg((ad_ble_msg_t *) received_msg);

                                        /* Free previously allocated message buffer. */
                                        OS_FREE(received_msg);
                                }
                        }
                        else if ( (USE_BLE_SLEEP == 1) && (cm_lp_clk_is_avail()) && (!stay_active) ) {
                                /* Sleep is possible only when the LP clock is ready! */
                                ret = sleep_when_possible();
                        }
                        else if (ble_block()) {
                                /* There are no pending BLE actions, so exit the while () loop. */
                                ret = -1;
                        }

#if ((BLE_WINDOW_STATISTICS == 1) && (stat_runs == WINSTAT_LOG_THRESHOLD))
                        log_printf(LOG_NOTICE, 2,
                                "sca:{M=%d, S=%d, dft=%d}, sync=%4d, type=%4d, len=%4d, crc=%4d, evt=%5d, zero=%3d, pos=%5d (%5d), neg=%5d (%5d)\r\n",
                                mst_sca, slv_sca, sca_drift, sync_errors, type_errors,
                                len_errors, crc_errors, diff_events, diff_zero, diff_pos,
                                max_pos_diff, diff_neg, max_neg_diff);

                        stat_runs = 0;
#endif

                        /* Now is a good time to notify the watch dog. */
                        sys_watchdog_notify(wdog_id);
                }
        }
}

/**
 * \brief Initialization function of BLE adapter
 */
void ad_ble_init(void)
{
        // BLE ROM variables initialization
        ble_platform_initialization();

        OS_QUEUE_CREATE(adapter_if.cmd_q, sizeof(ble_mgr_common_stack_msg_t *), AD_BLE_COMMAND_QUEUE_LENGTH);
        OS_QUEUE_CREATE(adapter_if.evt_q, sizeof(ble_mgr_common_stack_msg_t *), AD_BLE_EVENT_QUEUE_LENGTH);

        OS_ASSERT(adapter_if.cmd_q);
        OS_ASSERT(adapter_if.evt_q);

        // create FreeRTOS task
        OS_TASK_CREATE("bleA",                     // Text name assigned to the task
                       ad_ble_task,                // Function implementing the task
                       NULL,                       // No parameter passed
                       mainBLE_TASK_STACK_SIZE,    // Size of the stack to allocate to task
                       mainBLE_TASK_PRIORITY,      // Priority of the task
                       adapter_if.task);           // No task handle

        OS_ASSERT(adapter_if.task);

        DBG_CONFIGURE_LOW(BLE_ADAPTER_DEBUG, BLEBDG_ADAPTER); /* led (on: active, off: sleeping) */

#ifdef BLE_STACK_PASSTHROUGH_MODE
        ble_stack_init();
#endif
}

static void ad_ble_handle_stack_msg(ble_mgr_common_stack_msg_t *msg)
{
#if (dg_configBLE_ADV_STOP_DELAY_ENABLE == 1)
        if (waiting_for_evt) {
                struct delayed_msg *d_msg = OS_MALLOC(sizeof(*d_msg));

                d_msg->msg = msg;

                /* Add command to delayed command queue. */
                list_add(&delayed_list, d_msg);
        }
        else if (advertising && (msg->msg_type == GTL_MSG)
                && (msg->msg.gtl.msg_id == GAPM_CANCEL_CMD)) {
                struct delayed_msg *d_msg = OS_MALLOC(sizeof(*d_msg));

                d_msg->msg = msg;

                /* Set wait for event flag. */
                waiting_for_evt = true;

                /* Enable EVENT END notification.*/
                rwble_evt_end_adv_ntf_set(true);

                /* Add command to delayed command queue. */
                list_add(&delayed_list, d_msg);
        }
        else
#endif
        {
                /* Send message to stack. */
                ad_ble_send_to_stack(msg);

                /* Free previously allocated message buffer. */
                OS_FREE(msg);
        }
}

static void ad_ble_handle_adapter_msg(ad_ble_msg_t *msg)
{
        switch (msg->operation) {
        // Only handle initialization command for now
        case AD_BLE_OP_INIT_CMD:
        {
                ble_stack_init();
                ble_stack_initialized = true;
                break;
        }
        default:
                break;
        }
}

void ad_ble_send_to_stack(const ble_mgr_common_stack_msg_t *ptr_msg)
{
        ble_stack_msg_type_t msg_type = ptr_msg->msg_type;
        volatile uint16_t msgSize = ptr_msg->hdr.msg_len + sizeof(uint8_t);
        uint8_t *msgPtr = (uint8_t *) &ptr_msg->msg;
#if (dg_configBLE_ADV_STOP_DELAY_ENABLE == 1)
        ble_gtl_msg_t *gtl_msg = (ble_gtl_msg_t *) &(ptr_msg->msg.gtl);

        if ((msg_type == GTL_MSG) && (gtl_msg->msg_id == GAPM_START_ADVERTISE_CMD)) {
                struct gapm_start_advertise_cmd *cmd =
                                                 (struct gapm_start_advertise_cmd *) gtl_msg->param;
                if ((cmd->op.code >= GAPM_ADV_NON_CONN) && (cmd->op.code <= GAPM_ADV_DIRECT_LDC)) {
                        /* Set advertising flag. */
                        advertising = true;
                }
        }
#endif

        if (ble_stack_io_rx_buf_ptr != NULL) {
                // Indicate message type to BLE stack
                *ble_stack_io_rx_buf_ptr++ = msg_type;
                // Decrement message size
                msgSize--;

                // Call the BLE stack to decide on the message type
                if (ble_stack_io_rx_done_cb != NULL) {
                        ble_stack_io_rx_done_cb(BLE_STACK_IO_OK);
                }
                else
                {
                        ASSERT_ERROR(0);
                }

                // Continue sending the message
                while (msgSize) {
                        memcpy(ble_stack_io_rx_buf_ptr, msgPtr, ble_stack_io_rx_size_req);
                        msgSize -= ble_stack_io_rx_size_req;
                        msgPtr += ble_stack_io_rx_size_req;
                        ble_stack_io_rx_done_cb(BLE_STACK_IO_OK);
                }
        }
}

/**
 * \brief Write hook for BLE stack
 *
 * BLE stack uses this hook to deliver a received message to the BLE adapter layer.
 *
 * \param [in]  bufPtr     Pointer to the message buffer.
 * \param [in]  size       Size of the message (including the message type byte).
 * \param [in]  callback   BLE stack callback that must be called when the message is consumed.
 */
void ble_stack_io_write(uint8_t* bufPtr, uint32_t size, void (*callback) (uint8_t))
{
#ifndef BLE_STACK_PASSTHROUGH_MODE
        if (current_op == AD_BLE_OP_CODE_ADAPTER_MSG)
        {
                // Get msg id - bufPtr points to a packed message
                uint16_t stack_msg_id = *(bufPtr+1) + (*(bufPtr+2)<<8);

                switch (stack_msg_id) {
                case GAPM_DEVICE_READY_IND:
                {
                        /* The stack has been initialized */

                        // Notify the stack that the message has been consumed
                        callback(BLE_STACK_IO_OK);

                        // Create and send a GAPM_RESET_CMD to initialize the GAP layer
                        ble_mgr_common_stack_msg_t *msg;
                        struct gapm_reset_cmd *cmd;

                        msg = ble_gtl_alloc(GAPM_RESET_CMD, TASK_ID_GAPM, sizeof(struct gapm_reset_cmd));
                        cmd = (struct gapm_reset_cmd *) msg->msg.gtl.param;

                        cmd->operation = GAPM_RESET;

                        ad_ble_send_to_stack(msg);

                        OS_FREE(msg);

                        break;
                }
                case GAPM_CMP_EVT:
                {
                        /* Reset of the GAP layer has been completed */

                        // Make sure the reset was completed successfully
                        OS_ASSERT(*(bufPtr+9)==GAPM_RESET);
                        OS_ASSERT(*(bufPtr+10)==GAP_ERR_NO_ERROR);

                        // Create and send an AD_BLE_CMP_EVT
                        ad_ble_cmp_evt_t *ad_msg;

                        ad_msg = ble_ad_msg_alloc(AD_BLE_OP_CMP_EVT, sizeof(ad_ble_cmp_evt_t));

                        ad_msg->op_req = AD_BLE_OP_INIT_CMD;
                        ad_msg->status = BLE_STATUS_OK;

                        ad_ble_event_queue_send(&ad_msg, OS_QUEUE_FOREVER);


                        // Notify the stack that the message has been consumed
                        callback(BLE_STACK_IO_OK);

                        break;
                }
                default:
                        break;
                }
        }
        else if (current_op == AD_BLE_OP_CODE_STACK_MSG)
#endif
        {
                ble_mgr_common_stack_msg_t* msgBuf = NULL;
                uint8_t *pxMsgPacked = bufPtr;
                volatile uint16_t param_length;
                volatile uint8_t header_length;

                // Sanity check
                ASSERT_ERROR(bufPtr != NULL);
                ASSERT_ERROR(size != 0);

                // Extract message parameter length in bytes
                switch (*pxMsgPacked) {
                case BLE_HCI_CMD_MSG:
                        param_length = *(pxMsgPacked + HCI_CMD_PARAM_LEN_OFFSET);
                        header_length = HCI_CMD_HEADER_LENGTH;
                        break;

                case BLE_HCI_ACL_MSG:
                        param_length = *(pxMsgPacked + HCI_ACL_PARAM_LEN_OFFSET) +
                                        ( *(pxMsgPacked + HCI_ACL_PARAM_LEN_OFFSET + 1) << 8 );
                        header_length = HCI_ACL_HEADER_LENGTH;
                        break;

                case BLE_HCI_SCO_MSG:
                        param_length = *(pxMsgPacked + HCI_SCO_PARAM_LEN_OFFSET);
                        header_length = HCI_SCO_HEADER_LENGTH;
                        break;

                case BLE_HCI_EVT_MSG:
                        param_length = *(pxMsgPacked + HCI_EVT_PARAM_LEN_OFFSET);
                        header_length = HCI_EVT_HEADER_LENGTH;
                        break;

                case BLE_GTL_MSG:
                        param_length = *(pxMsgPacked + GTL_MSG_PARAM_LEN_OFFSET) +
                                        ( *(pxMsgPacked + GTL_MSG_PARAM_LEN_OFFSET+1) << 8 );
                        header_length = GTL_MSG_HEADER_LENGTH;
                        break;

                default:
                        break;
                }

                // Allocate the space needed for the message
                msgBuf = OS_MALLOC(sizeof(ble_mgr_common_stack_msg_t) + param_length);

                msgBuf->hdr.op_code = BLE_MGR_COMMON_STACK_MSG;     // fill message OP code
                msgBuf->msg_type = *pxMsgPacked++;                  // fill stack message type
                msgBuf->hdr.msg_len = header_length + param_length; // fill stack message length

                // copy the rest of the message
                memcpy(&msgBuf->msg, pxMsgPacked, msgBuf->hdr.msg_len);

#if (dg_configBLE_ADV_STOP_DELAY_ENABLE == 1)
                if (msgBuf->msg.gtl.msg_id == GAPM_CMP_EVT) {
                        struct gapm_cmp_evt *evt = (struct gapm_cmp_evt *) msgBuf->msg.gtl.param;

                        if ((evt->operation >= GAPM_ADV_NON_CONN)
                                && (evt->operation <= GAPM_ADV_DIRECT_LDC)) {
                                /* Set advertising flag. */
                                advertising = false;

                                /* Notify adapter because no ADV event is expected. */
                                OS_TASK_NOTIFY(adapter_if.task, mainBIT_EVENT_ADV_END,
                                                                                OS_NOTIFY_SET_BITS);
                        }
                }
#endif /* (dg_configBLE_ADV_STOP_DELAY_ENABLE == 1) */

                /* Post item to queue. */
                if (ad_ble_event_queue_send(&msgBuf, 0) == OS_OK) {
                        /* Check free space on BLE adapter's event queue. */
                        if (uxQueueSpacesAvailable(adapter_if.evt_q)) {
                                /* Call BLE stack I/O TX done callback right away. */
                                callback(BLE_STACK_IO_OK);
                        }
                        else {
                                /* Store BLE stack I/O TX done callback to be called when there is
                                 * some free space on the BLE adapter's event queue. */
                                ble_stack_io_tx_done_cb = callback;

                                /* Notify BLE manager that the adapter has blocked on a full event
                                 * queue. BLE manager will notify the adapter when there is free
                                 * space in the event queue. */
                                ble_mgr_notify_adapter_blocked(true);
                        }
                }
                else {
                        /* The following line should never be reached! */
                        ASSERT_ERROR(0);
                }
        }
}

/**
 * \brief BLE stack variables for reading a message from the COMMAND queue.
 *
 * The BLE stack calls this function while reading a message from the COMMAND queue to parse the
 * message and allocate a buffer for it internally. This functions updates the variables
 * accordingly and enables the ad_ble_rcv_msg to pass the message to the stack.
 *
 * \param [in] bufPtr    Pointer to the message buffer that the read data should be stored.
 * \param [in] size      Size of the data to be read from the current message.
 * \param [in] callback  Pointer to the function to be called when the requested size has been read.
 */
void ble_stack_io_read(uint8_t* bufPtr, uint32_t size, void (*callback) (uint8_t))
{
        GLOBAL_INT_DISABLE();
        ble_stack_io_rx_buf_ptr  = bufPtr;
        ble_stack_io_rx_size_req = size;
        ble_stack_io_rx_done_cb  = callback;
        GLOBAL_INT_RESTORE();
}

const ad_ble_interface_t *ad_ble_get_interface()
{
        return &adapter_if;
}

OS_BASE_TYPE ad_ble_event_queue_register(const OS_TASK task_handle)
{
        // Set event queue task handle
        mgr_task = task_handle;

        return OS_OK;
}

bool ble_stack_io_flow_off(void)
{
        return true;
}

void ad_ble_get_public_address(uint8_t address[BD_ADDR_LEN])
{
        memcpy(address, public_address, BD_ADDR_LEN);
}

void ad_ble_get_irk(uint8_t irk[KEY_LEN])
{
        uint8_t default_irk[KEY_LEN] = defaultBLE_IRK;

#ifdef BLE_PROD_TEST
        memcpy(irk, &default_irk, sizeof(default_irk));
#else
        bool valid;

        valid = ad_ble_read_nvms_param(irk, KEY_LEN, TAG_BLE_PLATFORM_IRK,
                                                                NVMS_PARAMS_TAG_IRK_OFFSET);
        if (!valid) {
                memcpy(irk, &default_irk, KEY_LEN);
        }
#endif
}

#if dg_configNVPARAM_ADAPTER
nvparam_t ad_ble_get_nvparam_handle(void)
{
        return ble_parameters;
}

ADAPTER_INIT_DEP1(ad_ble_adapter, ad_ble_init, ad_nvparam_adapter);

#elif dg_configNVMS_ADAPTER == 1

ADAPTER_INIT_DEP1(ad_ble_adapter, ad_ble_init, ad_nvms_adapter);

#else

ADAPTER_INIT(ad_ble_adapter, ad_ble_init);

#endif /* dg_configNVPARAM_ADAPTER */

void ad_ble_stay_active(bool status)
{
        stay_active = status;
        OS_TASK_NOTIFY(adapter_if.task, mainBIT_STAY_ACTIVE_UPDATED, OS_NOTIFY_SET_BITS);
}
#endif
