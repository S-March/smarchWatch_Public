/**
 ****************************************************************************************
 *
 * @file logging.c
 *
 * @brief Logging module implementation
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "osal.h"
#include "queue.h"
#include "event_groups.h"
#include "sys_power_mgr.h"


#include "sdk_defs.h"
#include "logging.h"

/* Internal FLAG */
#undef USE_QUEUE

/**
 * \brief Basic configuration checks
 */
#ifdef LOGGING_MODE_STANDALONE
#if defined(LOGGING_MODE_QUEUE) || defined(LOGGING_MODE_RETARGET) || defined(LOGGING_MODE_RTT)
#error Only one logging mode can be set
#endif
#define USE_QUEUE
#endif

#ifdef LOGGING_MODE_QUEUE
#if defined(LOGGING_MODE_STANDALONE) || defined(LOGGING_MODE_RETARGET) || defined(LOGGING_MODE_RTT)
#error Only one logging mode can be set
#endif
#define USE_QUEUE
#endif

#if defined(LOGGING_MODE_RETARGET) && !defined(CONFIG_RETARGET)
#error "Logging mode RETARGET requires system-wide CONFIG_RETARGET to be defined"
#endif

#if defined(LOGGING_MODE_RTT) && !defined(CONFIG_RTT)
#error "Logging mode RTT requires system-wide CONFIG_RTT to be defined"
#endif

#ifdef LOGGING_MODE_STANDALONE
/* Task stack size */
#define mainTASK_STACK_SIZE 100

/* Task priorities */
#define mainTASK_PRIORITY               ( tskIDLE_PRIORITY + 1 )

#endif

#ifdef USE_QUEUE

PRIVILEGED_DATA static OS_QUEUE xLogQueue;

#if LOGGING_SUPPRESSED_COUNT_ENABLE == 1
PRIVILEGED_DATA static uint32_t suppressed_messages;
#endif /* LOGGING_SUPPRESSED_COUNT_ENABLE == 1 */

#endif /* USE_QUEUE */

#ifdef LOGGING_ENABLED
const char logging_severity_chars[] = "DNWECCCC";

PRIVILEGED_DATA logging_severity_e logging_min_severity;
#endif /* LOGGING_ENABLED */

#ifdef LOGGING_MODE_STANDALONE

#ifndef LOGGING_STANDALONE_UART
#       define LOGGING_STANDALONE_UART HW_UART2
#endif

#ifndef LOGGING_STANDALONE_UART_BAUDRATE
#       define LOGGING_STANDALONE_UART_BAUDRATE    HW_UART_BAUDRATE_115200
#endif

#ifndef LOGGING_STANDALONE_UART_DATABITS
#       define LOGGING_STANDALONE_UART_DATABITS    HW_UART_DATABITS_8
#endif

#ifndef LOGGING_STANDALONE_UART_STOPBITS
#       define LOGGING_STANDALONE_UART_STOPBITS    HW_UART_STOPBITS_1
#endif

#ifndef LOGGING_STANDALONE_UART_PARITY
#       define LOGGING_STANDALONE_UART_PARITY      HW_UART_PARITY_NONE
#endif

#if LOGGING_USE_DMA == 1

#if HW_UART_USE_DMA_SUPPORT == 0
#error "Cannot Use DMA if HW_UART_USE_DMA_SUPPORT is set to 0"
#endif


PRIVILEGED_DATA static bool is_active;

static void uart_init(void)
{
        hw_gpio_set_pin_function(LOGGING_STANDALONE_GPIO_PORT_UART_TX,
                LOGGING_STANDALONE_GPIO_PIN_UART_TX, HW_GPIO_MODE_OUTPUT,
                (LOGGING_STANDALONE_UART == HW_UART1) ? HW_GPIO_FUNC_UART_TX : HW_GPIO_FUNC_UART2_TX);
        hw_gpio_set_pin_function(LOGGING_STANDALONE_GPIO_PORT_UART_RX, LOGGING_STANDALONE_GPIO_PIN_UART_RX,
                HW_GPIO_MODE_INPUT,
                (LOGGING_STANDALONE_UART == HW_UART1) ? HW_GPIO_FUNC_UART_RX : HW_GPIO_FUNC_UART2_RX);

        uart_config uart_init = {
                .baud_rate = LOGGING_STANDALONE_UART_BAUDRATE,
                .data = LOGGING_STANDALONE_UART_DATABITS,
                .stop = LOGGING_STANDALONE_UART_STOPBITS,
                .parity = LOGGING_STANDALONE_UART_PARITY,
                .use_dma = LOGGING_USE_DMA,
                .use_fifo = 0,
                .rx_dma_channel = (LOGGING_STANDALONE_UART == HW_UART1)
                                  ? HW_DMA_CHANNEL_0 : HW_DMA_CHANNEL_2,
                .tx_dma_channel =
                        (LOGGING_STANDALONE_UART == HW_UART1)
                        ? HW_DMA_CHANNEL_1 : HW_DMA_CHANNEL_3, };

        hw_uart_init(LOGGING_STANDALONE_UART, &uart_init);

}

static bool ad_prepare_for_sleep(void)
{
        return !is_active;
}

static void ad_sleep_canceled(void)
{

}

static void ad_wake_up_ind(bool arg)
{

}

static void ad_xtal16m_ready_ind(void)
{
        uart_init();
}

PRIVILEGED_DATA static OS_MUTEX xSemaphore;

static const adapter_call_backs_t sleep_cbs = {
        .ad_prepare_for_sleep = ad_prepare_for_sleep,
        .ad_sleep_canceled = ad_sleep_canceled,
        .ad_wake_up_ind = ad_wake_up_ind,
        .ad_xtal16m_ready_ind = ad_xtal16m_ready_ind,
        0
};


/**
 * @brief uart tx cb routine
 */

static void uart_tx_cb(void *user_data, uint16_t written)
{
        OS_EVENT_SIGNAL_FROM_ISR(xSemaphore);
}
#endif /* LOGGING_USE_DMA == 1 */

/**
 * @brief Main Logging task. Only used for standalone or queue
 * logging modes
 */
static void prvLogTask(void *pvParameters)
{
        struct mcif_message_s *current_message;

#if LOGGING_USE_DMA == 1
        hw_uart_tx_callback cb = uart_tx_cb;
#else
        hw_uart_tx_callback cb = NULL;
#endif

        for (;;) {
                is_active = false;
                OS_QUEUE_GET(xLogQueue, &current_message, portMAX_DELAY);
                is_active = true;
                hw_uart_send(LOGGING_STANDALONE_UART, current_message->buffer, current_message->len,
                        cb, NULL);
#if LOGGING_USE_DMA == 1
                OS_EVENT_WAIT(xSemaphore, OS_EVENT_FOREVER);
#endif
                OS_FREE(current_message);
        }
}

static void standalone_init(void)
{
        uart_init();
        pm_register_adapter(&sleep_cbs);
}

#endif /* LOGGING_MODE_STANDALONE == 1 */

/**
 * @brief Initialization function of logging module
 */
void log_init(void)
{
#ifdef LOGGING_ENABLED
        logging_min_severity = LOGGING_MIN_DEFAULT_SEVERITY;
#endif /* LOGGING_ENABLED */

#ifdef USE_QUEUE
#if LOGGING_SUPPRESSED_COUNT_ENABLE == 1
        OS_ENTER_CRITICAL_SECTION();
        suppressed_messages = 0;
        OS_LEAVE_CRITICAL_SECTION();
#endif

        OS_QUEUE_CREATE(xLogQueue, sizeof(struct mcif_message_s *), LOGGING_QUEUE_LENGTH);
        OS_ASSERT(xLogQueue);
#endif

#ifdef LOGGING_MODE_STANDALONE

        standalone_init();

#if LOGGING_USE_DMA == 1
        OS_EVENT_CREATE(xSemaphore);
#endif
        // create FreeRTOS task
        OS_TASK LoggingTaskHandle = NULL;
        OS_TASK_CREATE("LOGGING",                                       // Text name assigned to the task
                       prvLogTask,                                      // Function implementing the task
                       NULL,                                            // No parameter passed
                       mainTASK_STACK_SIZE * OS_STACK_WORD_SIZE,        // Size of the stack to allocate to task
                       mainTASK_PRIORITY,                               // Priority of the task
                       LoggingTaskHandle);                              // Task handle
        OS_ASSERT(LoggingTaskHandle);

#endif /* LOGGING_MODE_STANDALONE == 1 */

}

void log_set_severity(logging_severity_e severity)
{
#ifdef LOGGING_ENABLED
        logging_min_severity = severity;
#endif
}

#ifdef USE_QUEUE

#if LOGGING_SUPPRESSED_COUNT_ENABLE == 1
#define SUPPRESSED_BUFFER_SZ sizeof(LOGGING_SUPPRESSED_MSG_TMPL) + 24
static inline void log_suppressed(void)
{
        struct mcif_message_s *msg;
        uint32_t suppressed_count;

        if ((LOGGING_SUPPRESSED_SEVERITY < LOGGING_MIN_COMPILED_SEVERITY) ||
                (LOGGING_SUPPRESSED_SEVERITY < logging_min_severity))
                return;


        /* Get a local, thread-safe reference of suppressed_messages. We don't want to
         * close interrupts while calling snprintf
         */
        OS_ENTER_CRITICAL_SECTION();
        suppressed_count = suppressed_messages;
        OS_LEAVE_CRITICAL_SECTION();

        /* If suppressed messages >= LOGGING_SUPPRESSED_MIN_COUNT
         * try to queue a suppressed messages log
         */
        if (suppressed_count >= LOGGING_SUPPRESSED_MIN_COUNT) {

                /* Early check to skip unneeded (and perhaps catastrophic)
                 * allocations and snprintf processing
                 */
                if (uxQueueSpacesAvailable(xLogQueue) == 0) {
                        return;
                }

                /* allocate a few bytes more to account for the
                 * "header" string AND the bytes in the actual
                 * suppressed count.
                 */
                msg = OS_MALLOC(sizeof(struct mcif_message_s) +
                SUPPRESSED_BUFFER_SZ);

                msg->len = 1 + snprintf(msg->buffer, SUPPRESSED_BUFFER_SZ,
                        "[%d] %c %d " LOGGING_SUPPRESSED_MSG_TMPL,
                        OS_GET_TICK_COUNT(),
                        logging_severity_chars[LOGGING_SUPPRESSED_SEVERITY],
                        LOGGING_SUPPRESSED_TAG,
                        suppressed_count);

                /* Attempt to queue the message. If it succeeds, subtract the printed
                 * suppressed_messages counter from the total suppressed messages (because
                 * in the meanwhile, more messages may get suppressed).
                 */
                if (OS_QUEUE_PUT(xLogQueue, &msg, 0) != pdPASS) {
                        /* Still full. Try later */
                        OS_FREE(msg);
                }
                else {
                        OS_ENTER_CRITICAL_SECTION();
                        suppressed_messages -= suppressed_count;
                        OS_LEAVE_CRITICAL_SECTION();
                }

        }
}
#endif

static inline void log_send(struct mcif_message_s *msg)
{

        /* Even though we checked before for space in the queue, some other
         * task may get it before we reach this point
         */
        if (OS_QUEUE_PUT(xLogQueue, &msg, 0) != pdPASS) {
                OS_FREE(msg);
#if LOGGING_SUPPRESSED_COUNT_ENABLE == 1
                OS_ENTER_CRITICAL_SECTION();
                suppressed_messages++;
                OS_LEAVE_CRITICAL_SECTION();
#endif
                return;
        }

#if LOGGING_SUPPRESSED_COUNT_ENABLE == 1
        log_suppressed();
#endif
}

void log_printf_raw(const char *fmt, ...)
{
        va_list args;
        int n;
        struct mcif_message_s *msg;

        /* Check for space (although some other task may still get the available slot later)
         * to save some time (and memory peaking) from the malloc and copy
         */
        if (uxQueueSpacesAvailable(xLogQueue) == 0) {
#if LOGGING_SUPPRESSED_COUNT_ENABLE == 1
                OS_ENTER_CRITICAL_SECTION();
                suppressed_messages++;
                OS_LEAVE_CRITICAL_SECTION();
#endif
                return;
        }

#if LOGGING_MIN_ALLOWED_FREE_HEAP
        OS_ENTER_CRITICAL_SECTION();
        if (xPortGetFreeHeapSize() <= LOGGING_MIN_ALLOWED_FREE_HEAP) {
#if LOGGING_SUPPRESSED_COUNT_ENABLE == 1
                suppressed_messages++;
#endif
                OS_LEAVE_CRITICAL_SECTION();
                return;
        }
#endif
        msg = OS_MALLOC(
                sizeof(struct mcif_message_s) + LOGGING_MIN_MSG_SIZE);
#if LOGGING_MIN_ALLOWED_FREE_HEAP
        OS_LEAVE_CRITICAL_SECTION();
#endif

        va_start(args, fmt);
        n = vsnprintf(msg->buffer, LOGGING_MIN_MSG_SIZE, fmt, args);
        va_end(args);

        if (n >= LOGGING_MIN_MSG_SIZE) {
                OS_FREE(msg);
#if LOGGING_MIN_ALLOWED_FREE_HEAP
                OS_ENTER_CRITICAL_SECTION();
                if (xPortGetFreeHeapSize() <= LOGGING_MIN_ALLOWED_FREE_HEAP) {
#if LOGGING_SUPPRESSED_COUNT_ENABLE == 1
                        suppressed_messages++;
#endif
                        OS_LEAVE_CRITICAL_SECTION();
                        return;
                }
#endif
                msg = OS_MALLOC(sizeof(struct mcif_message_s) + n + 1);
#if LOGGING_MIN_ALLOWED_FREE_HEAP
                OS_LEAVE_CRITICAL_SECTION();
#endif
                va_start(args, fmt);
                vsnprintf(msg->buffer, n + 1, fmt, args);
                va_end(args);
        }
        msg->len = n + 1;
        log_send(msg);
}

#endif /* USE_QUEUE */

