/**
 ****************************************************************************************
 *
 * @file mcif.c
 *
 * @brief Main Logging task. Only used for standalone or queue
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#include <stdbool.h>
#include "sdk_defs.h"
#include "hw_uart.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "sys_power_mgr.h"

#include "logging.h"

#include "mcif.h"
#include "mcif_internal.h"
#include "hw_gpio.h"

#define TXQ_LENGTH 2
#define RXQ_LENGTH 2

/* Task stack size */
#define mainTASK_STACK_SIZE 500

/* Task priorities */
#define mainTASK_PRIORITY               ( tskIDLE_PRIORITY + 1)

#define mainBIT_TX_IRQ (1 << 30)
#define mainBIT_RX_IRQ (1 << 31)

#define UART_CHECK_TIMER_MS     10

PRIVILEGED_DATA static enum mcif_states_e {
        MCIF_ST_RX = 0,
        MCIF_ST_TX
} current_state;

PRIVILEGED_DATA OS_QUEUE xQueueTx[MCIF_CLIENTS_NR];
PRIVILEGED_DATA OS_QUEUE xQueueRx[MCIF_CLIENTS_NR];
#if MCIF_USE_TASK_NOTIFICATIONS == 1
PRIVILEGED_DATA OS_TASK xClientTaskHandles[MCIF_CLIENTS_NR];
PRIVILEGED_DATA uint8_t uiClientNotifBit[MCIF_CLIENTS_NR];
#endif

PRIVILEGED_DATA static OS_TASK xMcifTaskHandle = NULL;

static uint8_t rxbuf[MCIF_UART_DMA_BUFFER];

PRIVILEGED_DATA static uint32_t clientEnableMask = 0;
PRIVILEGED_DATA static OS_TIMER xUartCheck;
PRIVILEGED_DATA int dmabuflen, parsebuflen;
PRIVILEGED_DATA int tmp_pos, bkspc;
bool dmabufferfull=0;

static void uart_init(void);

static void uart_tx_cb(void *user_data, uint16_t written)
{
        OS_TASK_NOTIFY_FROM_ISR(xMcifTaskHandle, mainBIT_TX_IRQ, OS_NOTIFY_SET_BITS);
}

static void uart_rx_cb(void *user_data, uint16_t read)
{
        if (dmabufferfull==1)
        {
                parsebuflen=MCIF_UART_DMA_BUFFER;
                dmabufferfull = 1;
        }

        tmp_pos = 0;
        bkspc = 0;

        OS_TASK_NOTIFY_FROM_ISR(xMcifTaskHandle, mainBIT_RX_IRQ, OS_NOTIFY_SET_BITS);
}


static bool ad_prepare_for_sleep(void)
{
        if (current_state == MCIF_ST_RX) {
                hw_uart_abort_receive(MCIF_UART);
                return true;
        } else {
                return false;
        }
}

static void ad_sleep_canceled(void)
{

        /* Re-Start uart reception */
        hw_uart_receive(MCIF_UART, rxbuf, MCIF_UART_DMA_BUFFER, uart_rx_cb, NULL);

}

static void ad_wake_up_ind(bool arg)
{

}

static void ad_xtal16m_ready_ind(void)
{
        uart_init();
        /* Re-Start uart reception */
        hw_uart_receive(MCIF_UART, rxbuf, MCIF_UART_DMA_BUFFER, uart_rx_cb, NULL);

}

static const adapter_call_backs_t sleep_cbs = {
        .ad_prepare_for_sleep = ad_prepare_for_sleep,
        .ad_sleep_canceled = ad_sleep_canceled,
        .ad_wake_up_ind = ad_wake_up_ind,
        .ad_xtal16m_ready_ind = ad_xtal16m_ready_ind,
        0
};


void vUartCheckTmrCallback(OS_TIMER pxTimer)
{
        int i;

        /* check how many bytes already transferred by DMA dmabuflen will be zero when
         * the DMA will be completed or canceled.
         */
        if (MCIF_UART == HW_UART1) {
                dmabuflen = DMA->DMA0_IDX_REG;
        } else {
                dmabuflen = DMA->DMA2_IDX_REG;
        }

        /* if there byte transfered echo back until \r or \n is detected.
         * If \r or \n is detected stop receiving and parse the buffer
         */
        if (dmabuflen>=1) {
                for (i=tmp_pos; i<dmabuflen && i<MCIF_UART_DMA_BUFFER; i++)
                {
                        switch (rxbuf[i]){
                        case 8:
                                /* Putty sends char 127 with BKSP */
                                tmp_pos++;
                                /* Backspace. Delete previous byte, erase rest of line in terminal */
                                if (bkspc>0){ //send back the backspace
                                        bkspc--;
                                        hw_uart_send(MCIF_UART, "\x8\x1b[K", 4, NULL, NULL);
                                        break;
                                }
                                break;
                        case '\r':
                        case '\n':
                                /* Send a LF, just in case it is needed */
                                hw_uart_send(MCIF_UART, "\r\n", 2, NULL, NULL);
                                parsebuflen = dmabuflen;
                                dmabufferfull = 0;
                                tmp_pos = 0;
                                hw_uart_abort_receive(MCIF_UART); //This will cause the callback to 'hit'
                                break;
                        default:
                                dmabufferfull = 1;
                                tmp_pos++;
                                hw_uart_send(MCIF_UART, &rxbuf[i], 1, NULL, NULL);
                                bkspc++;
                        }

                        if (MCIF_UART == HW_UART1) {
                                dmabuflen = DMA->DMA0_IDX_REG;
                        } else {
                                dmabuflen = DMA->DMA2_IDX_REG;
                        }
                }
        }

        OS_TIMER_START(xUartCheck, OS_MS_2_TICKS(UART_CHECK_TIMER_MS));
}


/**
 * @brief Main Logging task. Only used for standalone or queue
 * logging modes
 */
static void prvMcifAsciiTask(void *pvParameters)
{
        struct mcif_message_s *msg = NULL;
        uint32_t ulNotifiedValue;
        uint32_t bitsToWaitFor;
        int last_served_client = 0;
        bool polling = true;

        int i;

        OS_BASE_TYPE xResult;
        hw_uart_tx_callback cb = uart_tx_cb;

        log_printf(LOG_NOTICE, MCIF_LOG_TAG, "MCIF started\n\r");

        /* Start uart reception */
        /* Create the Timer. */
        xUartCheck = OS_TIMER_CREATE("UARTChkTmr",
                                OS_MS_2_TICKS(UART_CHECK_TIMER_MS),             /* Expire after UART_CHECK_TIMER_MS msec */
                                OS_TIMER_FAIL,                                  /* Run once */
                                (void *) 0,                                     /* Timer id */
                                vUartCheckTmrCallback );                        /* call-back */
        OS_ASSERT(xUartCheck != NULL);
        OS_TIMER_START(xUartCheck, OS_MS_2_TICKS(UART_CHECK_TIMER_MS));

        hw_uart_receive(MCIF_UART, rxbuf, MCIF_UART_DMA_BUFFER, uart_rx_cb, NULL);

        current_state = MCIF_ST_RX;
        //pm_stay_alive();
        for (;;) {
                switch (current_state) {
                case MCIF_ST_RX:
                        bitsToWaitFor = mainBIT_RX_IRQ | clientEnableMask;
                        break;
                case MCIF_ST_TX:
#ifdef MCIF_HALF_DUPLEX_PROTO
                        bitsToWaitFor = mainBIT_TX_IRQ;
#else
                        bitsToWaitFor = mainBIT_TX_IRQ | mainBIT_RX_IRQ;
#endif
                        break;
                default:
                        /* Illegal state */
                        OS_ASSERT(0);
                }

                xResult = OS_TASK_NOTIFY_WAIT(0, 0xffffffff, &ulNotifiedValue, //500);
                        portMAX_DELAY);
                if (xResult == pdFAIL) {
//                        if (polling) {
//                                pm_resume_sleep();
//                        } else {
//                                pm_stay_alive();
//                        }
//                        polling = !polling;
                        continue;
                }
                ulNotifiedValue &= bitsToWaitFor;
                if (ulNotifiedValue & mainBIT_RX_IRQ) {
                        int cli_id;
                        struct mcif_message_s *rxmsg;
                        cli_id = mcif_parse_frame(rxbuf, parsebuflen, &rxmsg);
                        if (cli_id != -1) {
                                /* A frame has been received and stored in
                                 * rxmsg
                                 */
                                if (OS_QUEUE_PUT(xQueueRx[cli_id], &rxmsg, 0) != pdPASS) {
                                        OS_FREE(rxmsg);
                                        log_printf(LOG_WARNING, MCIF_LOG_TAG,
                                                "A message to client %d "
                                                "been dropped\n\r", cli_id);
                                }
#if MCIF_USE_TASK_NOTIFICATIONS
                                else if (xClientTaskHandles[cli_id] != NULL) {
                                        /* Notify client task that a new frame has been queued */
                                        OS_TASK_NOTIFY(xClientTaskHandles[cli_id], 1 << uiClientNotifBit[cli_id],
                                                OS_NOTIFY_SET_BITS);
                                }
#endif
                        }

                        /* Wait for next character */
                        hw_uart_receive(MCIF_UART, rxbuf, MCIF_UART_DMA_BUFFER, uart_rx_cb, NULL);
                }


                if (ulNotifiedValue & clientEnableMask) {

                        int start = last_served_client;
                        /* TX request */

                        /* For now, round-robin */
                        while (1) {
                                last_served_client++;
                                if (last_served_client >= MCIF_CLIENTS_NR)
                                        last_served_client = 0;

                                if (xQueueTx[last_served_client] == NULL) {
                                        /* Handle initial condition where
                                         * last_served_client is initialized to
                                         * an empty queue
                                         */
                                        if (last_served_client == start)
                                                break;
                                        else
                                                continue;
                                }
                                /* The previous message must already be transmitted
                                   and freed before getting here */
                                OS_ASSERT(msg == NULL);
                                if (OS_QUEUE_GET(xQueueTx[last_served_client],
                                        &msg, 0) == pdPASS) {
                                        current_state = MCIF_ST_TX;
                                        //pm_stay_alive();
#ifdef MCIF_HALF_DUPLEX_PROTO
                                        parsebuflen = dmabuflen;
                                        dmabufferfull = 0;
                                        tmp_pos = 0;
                                        hw_uart_abort_receive(MCIF_UART);
#endif
                                        hw_uart_send(MCIF_UART, msg->buffer, msg->len, uart_tx_cb, NULL);
                                        break;
                                }
                                if (last_served_client == start)
                                        break;
                        }
                }

                if (ulNotifiedValue & mainBIT_TX_IRQ) {
                        /* Reenable TX Q */
                        if (msg != NULL) {
                                OS_FREE(msg);
                        }
                        msg = NULL;

                        //pm_resume_sleep();
                        current_state = MCIF_ST_RX;
#ifdef MCIF_HALF_DUPLEX_PROTO
                        hw_uart_receive(MCIF_UART, rxbuf, MCIF_UART_DMA_BUFFER, uart_rx_cb, NULL);
#endif

                        ulNotifiedValue = 0;
                        for (i = 0; i < MCIF_CLIENTS_NR; i++) {
                                if (xQueueTx[i] != NULL &&
                                        uxQueueMessagesWaiting(xQueueTx[i])) {
                                        ulNotifiedValue |= (1 << i);
                                }
                        }
                        if (ulNotifiedValue) {
                                OS_TASK_NOTIFY(xMcifTaskHandle,
                                        ulNotifiedValue, OS_NOTIFY_SET_BITS);

                        }
                }

        }
}

static void uart_init(void)
{
        hw_gpio_set_pin_function(MCIF_GPIO_PORT_UART_TX, MCIF_GPIO_PIN_UART_TX, HW_GPIO_MODE_OUTPUT,
                (MCIF_UART == HW_UART1) ? HW_GPIO_FUNC_UART_TX : HW_GPIO_FUNC_UART2_TX);
        hw_gpio_set_pin_function(MCIF_GPIO_PORT_UART_RX, MCIF_GPIO_PIN_UART_RX, HW_GPIO_MODE_INPUT,
                (MCIF_UART == HW_UART1) ? HW_GPIO_FUNC_UART_RX : HW_GPIO_FUNC_UART2_RX);

        uart_config uart_init = { .baud_rate = MCIF_UART_BAUDRATE, .data = MCIF_UART_DATABITS, .stop =
        MCIF_UART_STOPBITS, .parity = MCIF_UART_PARITY, .use_dma = 1, .use_fifo = 1,
                .rx_dma_channel =
                        (MCIF_UART == HW_UART1) ?
                                                  HW_DMA_CHANNEL_0 :
                                                  HW_DMA_CHANNEL_2,
                .tx_dma_channel =
                        (MCIF_UART == HW_UART1) ?
                                                  HW_DMA_CHANNEL_1 :
                                                  HW_DMA_CHANNEL_3, };

        hw_uart_init(MCIF_UART, &uart_init);
}

OS_BASE_TYPE mcif_queue_send(int cli_id, const void *item,
        OS_TICK_TIME wait_ticks)
{
        if (OS_QUEUE_PUT(xQueueTx[cli_id], item, wait_ticks) != pdPASS) {
                return pdFAIL;
        }
        OS_TASK_NOTIFY(xMcifTaskHandle, (1 << cli_id), OS_NOTIFY_SET_BITS);
        return pdPASS;
}

void mcif_setup_queues(int cli_id, OS_QUEUE txq, QueueHandle_t rxq)
{
        /* Don't allow more than MCIF_CLIENTS_NR queues (i.e. clients) */
        OS_ASSERT(cli_id < MCIF_CLIENTS_NR);
        xQueueTx[cli_id] = txq;
        xQueueRx[cli_id] = rxq;

        clientEnableMask |= (1 << cli_id);
        log_printf(LOG_NOTICE, MCIF_LOG_TAG, "Added client %d\n\r", cli_id);
}

#if MCIF_USE_TASK_NOTIFICATIONS == 1
void mcif_setup_client_notifications(int cli_id, OS_TASK handle, uint8_t notif_bit)
{
        /* Don't allow more than MCIF_CLIENTS_NR queues (i.e. clients) */
        OS_ASSERT(cli_id < MCIF_CLIENTS_NR);
        xClientTaskHandles[cli_id] = handle;
        uiClientNotifBit[cli_id] = notif_bit;
}
#endif

/**
 * @brief Initialization function of logging module
 */
void mcif_init(void)
{
        mcif_framing_init();

        uart_init();
        pm_register_adapter(&sleep_cbs);

        // create FreeRTOS task
        OS_TASK_CREATE("MCIF",                                          // Text name assigned to the task
                       prvMcifAsciiTask,                                // Function implementing the task
                       NULL,                                            // No parameter passed
                       mainTASK_STACK_SIZE * OS_STACK_WORD_SIZE,        // Size of the stack to allocate to task
                       mainTASK_PRIORITY,                               // Priority of the task
                       xMcifTaskHandle);                                // Task handle
        OS_ASSERT(xMcifTaskHandle);
}
