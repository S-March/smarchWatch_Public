/**
 ****************************************************************************************
 *
 * @file ad_ftdf_mac_api.c
 *
 * @brief FTDF FreeRTOS Adapter
 *
 * Copyright (C) 2014-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef FTDF_PHY_API

#include <string.h>
#include <stdbool.h>
#include "osal.h"
#include "queue.h"

#include "sys_power_mgr.h"

#include "hw_rf.h"

#include "sdk_defs.h"
#include "ad_ftdf.h"
#include "ad_ftdf_mac_api.h"

#include "internal.h"
#if FTDF_DBG_BUS_ENABLE
#include "hw_gpio.h"
#endif /* FTDF_DBG_BUS_ENABLE */

/* Task stack size in bytes. */
#define mainTASK_STACK_SIZE (1000 * sizeof(StackType_t))

/* Task priorities */
#define mainTEMPLATE_TASK_PRIORITY              ( tskIDLE_PRIORITY + 2 )

/* Add 2 for the two semaphores */
#define QUEUE_SET_SIZE (AD_FTDF_DOWN_QUEUE_LENGTH + 2)

/* Event group bits */
#define mainBIT_GEN_IRQ (1 << 0)
#define mainBIT_WUP_IRQ (1 << 1)
#define mainBIT_DOWN_Q  (1 << 2)

PRIVILEGED_DATA static OS_QUEUE xDownQueue;
PRIVILEGED_DATA static OS_QUEUE xUpQueue;

PRIVILEGED_DATA static OS_MUTEX xConfigSemaphore;


PRIVILEGED_DATA static OS_TASK xFtdfTaskHandle = NULL;

/**
 * @brief ftdf_gen_irq interrupt service routine
 */
void FTDF_GEN_Handler(void)
{
        ftdf_confirm_lmac_interrupt();
        OS_TASK_NOTIFY_FROM_ISR(xFtdfTaskHandle, mainBIT_GEN_IRQ, eSetBits);
}
/**
 * @brief ftdf_wakup_irq interrupt service routine
 */
void FTDF_WAKEUP_Handler(void)
{
        OS_TASK_NOTIFY_FROM_ISR(xFtdfTaskHandle, mainBIT_WUP_IRQ, eSetBits);
}

OS_BASE_TYPE ad_ftdf_queue_send(const void *item,
        OS_TICK_TIME wait_ticks)
{
        if (OS_QUEUE_PUT(xDownQueue, item, wait_ticks) != OS_QUEUE_OK) {
                return pdFAIL;
        }
        OS_TASK_NOTIFY(xFtdfTaskHandle, mainBIT_DOWN_Q, eSetBits);
        return OS_QUEUE_OK;
}

/**
 * @brief Main FTDF Interrupt and event queue handling task
 */
static void prvFtdfTask(void *pvParameters)
{
        ftdf_msg_buffer_t *ftdf_msg;
        uint32_t ulNotifiedValue;
        uint32_t bitsToWaitFor;
        bool is_idle;
        OS_BASE_TYPE xResult;
        OS_TICK_TIME ticks_to_wait;

        NVIC_ClearPendingIRQ(FTDF_WAKEUP_IRQn);
        NVIC_EnableIRQ(FTDF_WAKEUP_IRQn);

        NVIC_ClearPendingIRQ(FTDF_GEN_IRQn);
        NVIC_EnableIRQ(FTDF_GEN_IRQn);

        bitsToWaitFor = mainBIT_DOWN_Q | mainBIT_GEN_IRQ | mainBIT_WUP_IRQ;
        sleep_status = BLOCK_ACTIVE;

        ftdf_reset(1);

        ticks_to_wait = AD_FTDF_IDLE_TIMEOUT;

        for (;;) {
                /* If block is active and is able to sleep, then give it a little time, and then try
                 * to put it to sleep
                 * If block is sleeping, block indefinitely
                 * If block is active but is actually doing something, then block indefinitely to
                 * allow the processor to WFI. It will block anyway when the operation is done.
                 */
                if (sleep_status == BLOCK_ACTIVE && ftdf_can_sleep()) {
                        ticks_to_wait = AD_FTDF_IDLE_TIMEOUT;
                } else {
                        ticks_to_wait = portMAX_DELAY;
                }

                /*
                 * Wait on any of the event group bits, then clear them all
                 */
                xResult = OS_TASK_NOTIFY_WAIT(0x0, 0xFFFFFFFF, &ulNotifiedValue, ticks_to_wait);



                if (xResult == pdPASS) {
                        if (ulNotifiedValue & mainBIT_GEN_IRQ) {
                                ftdf_event_handler();
                        }

                        if (ulNotifiedValue & mainBIT_WUP_IRQ) {
#if FTDF_USE_SLEEP_DURING_BACKOFF
                                ftdf_sdb_fsm_wake_up_irq();
#endif /* FTDF_USE_SLEEP_DURING_BACKOFF */
                                ad_ftdf_wake_up_async();
                        }

                        if (ulNotifiedValue & mainBIT_DOWN_Q) {
                                switch (sleep_status) {
                                case BLOCK_ACTIVE:
                                        if (OS_QUEUE_GET(xDownQueue, &ftdf_msg, 0) == OS_QUEUE_OK) {
                                                ftdf_snd_msg(ftdf_msg);
                                        }
                                        if (uxQueueMessagesWaiting(xDownQueue)) {
                                                OS_TASK_NOTIFY(xFtdfTaskHandle,
                                                        mainBIT_DOWN_Q, eSetBits);
                                        }
                                        break;
                                case BLOCK_SLEEPING:
                                        ad_ftdf_wake_up_async();
                                        break;
                                }
                        }
                }
                else {
                        /* can try to go to sleep. Ask UMAC if this is
                         * possible
                         */
                        sleep_when_possible( FTDF_FALSE, 0 );
                }
        }
}

/**
 * @brief Releases a message buffer and associated data buffers (if any)
 */
void ad_ftdf_rel_msg_data(ftdf_msg_buffer_t *msgBuf)
{
        switch (msgBuf->msg_id) {
        case FTDF_TRANSPARENT_INDICATION:
                ad_ftdf_rel_data_buffer(((ftdf_transparent_indication_t*)msgBuf)->frame);
                break;
        case FTDF_TRANSPARENT_CONFIRM:
                ad_ftdf_rel_data_buffer(
                        (ftdf_octet_t*)(((ftdf_transparent_confirm_t*)msgBuf)->handle));
                break;
        case FTDF_DATA_INDICATION:
                ad_ftdf_rel_data_buffer(((ftdf_data_indication_t *)msgBuf)->msdu);
                ad_ftdf_rel_data_buffer(
                        (ftdf_octet_t*)(((ftdf_data_indication_t *)msgBuf)->payload_ie_list));
                break;
        case FTDF_DATA_CONFIRM:
                ad_ftdf_rel_data_buffer(
                               (ftdf_octet_t*)(((ftdf_data_confirm_t *)msgBuf)->ack_payload));
                break;
        case FTDF_BEACON_NOTIFY_INDICATION:
                ad_ftdf_rel_data_buffer(((ftdf_beacon_notify_indication_t*)msgBuf)->sdu);
                ad_ftdf_rel_data_buffer((ftdf_octet_t*)(((ftdf_beacon_notify_indication_t*)
                        msgBuf)->ie_list));
                break;
        case FTDF_BEACON_REQUEST_INDICATION:
                ad_ftdf_rel_data_buffer((ftdf_octet_t*)(((ftdf_beacon_request_indication_t*)
                        msgBuf)->ie_list));
                break;
        }

        ad_ftdf_rel_msg_buffer(msgBuf);

}

/**
 * @brief Puts a message in the UP queue
 */
void ad_ftdf_rcv_msg(ftdf_msg_buffer_t *msgBuf)
{

        if (OS_QUEUE_PUT(xUpQueue, &msgBuf, 0) != OS_QUEUE_OK) {
                ad_ftdf_rel_msg_data(msgBuf);
        }
}

OS_QUEUE ad_ftdf_get_up_queue()
{
        return xUpQueue;
}

void ad_ftdf_set_ext_address(ftdf_ext_address_t address)
{
        OS_BASE_TYPE xResult;
        if (OS_EVENT_WAIT(xConfigSemaphore, portMAX_DELAY)) {
                u_ext_address = address;
                xResult = OS_EVENT_SIGNAL(xConfigSemaphore);
                /* The semaphore must not be obtained properly for the following to fail */
                OS_ASSERT(xResult == OS_EVENT_SIGNALED);
        }
}

ftdf_ext_address_t ad_ftdf_get_ext_address(void)
{
        OS_BASE_TYPE xResult;
        ftdf_ext_address_t address;

        if (OS_EVENT_WAIT(xConfigSemaphore, portMAX_DELAY)) {
                address = u_ext_address;
                xResult = OS_EVENT_SIGNAL(xConfigSemaphore);
                /* The semaphore must not be obtained properly for the following to fail */
                OS_ASSERT(xResult == OS_EVENT_SIGNALED);
        }

        return address;
}



/************************
 * FTDF config functions
 ************************/
ftdf_msg_buffer_t *ad_ftdf_get_msg_buffer(ftdf_size_t len)
{
        if (len == 0) {
                return NULL;
        }

        if (len % sizeof(uint32_t)) {
                len += sizeof(uint32_t) - (len % sizeof(uint32_t));
        }

        return (ftdf_msg_buffer_t*) OS_MALLOC(len);
}

void ad_ftdf_rel_msg_buffer(ftdf_msg_buffer_t *msg_buf)
{
        if (msg_buf != NULL) {
                OS_FREE(msg_buf);
        }
}

ftdf_octet_t* ad_ftdf_get_data_buffer(ftdf_data_length_t len)
{
        if (len == 0) {
                return NULL;
        }

        if (len % sizeof(uint32_t)) {
                len += sizeof(uint32_t) - (len % sizeof(uint32_t));
        }

        return (ftdf_octet_t*) OS_MALLOC(len);
}

void ad_ftdf_rel_data_buffer(ftdf_octet_t *data_buf)
{
        if (data_buf != NULL) {
                OS_FREE(data_buf);
        }
}

void ad_ftdf_wake_up_ready(void)
{
        /* The block must NOT be active to call this function */
        OS_ASSERT(sleep_status != BLOCK_ACTIVE);
        sleep_status = BLOCK_ACTIVE;

#if FTDF_DBG_BUS_ENABLE
        FTDF_checkDbgMode();
#endif /* FTDF_DBG_BUS_ENABLE */
#if dg_configUSE_FTDF_DDPHY == 1
        ftdf_ddphy_restore();
#endif

        if ( explicit_sleep == FTDF_TRUE )
        {
                ftdf_msg_buffer_t *msg = ad_ftdf_get_msg_buffer( sizeof( ftdf_msg_buffer_t ) );
                msg->msg_id = FTDF_EXPLICIT_WAKE_UP;
                ad_ftdf_rcv_msg( msg );
                explicit_sleep = FTDF_FALSE;
        }

        /* If there are pending messages in queue, set a task notification */
        if (uxQueueMessagesWaiting(xDownQueue)) {
                OS_TASK_NOTIFY(xFtdfTaskHandle,
                        mainBIT_DOWN_Q, eSetBits);
        }
#if FTDF_USE_SLEEP_DURING_BACKOFF
        ftdf_sdb_fsm_wake_up();
#endif /* FTDF_USE_SLEEP_DURING_BACKOFF */
}

void ad_ftdf_rcv_frame_transparent(ftdf_data_length_t frameLength, ftdf_octet_t *frame,
        ftdf_bitmap32_t status)
{
}

void ad_ftdf_send_frame_transparent_confirm(void* handle, ftdf_bitmap32_t status)
{
}

void ad_ftdf_init_mac_api(void)
{
        //pm_resource_is_awake(PM_FTDF_ID);
        OS_QUEUE_CREATE(xDownQueue, sizeof(ftdf_msg_buffer_t *), AD_FTDF_DOWN_QUEUE_LENGTH);
        OS_QUEUE_CREATE(xUpQueue, sizeof(ftdf_msg_buffer_t *), AD_FTDF_UP_QUEUE_LENGTH);

        xConfigSemaphore = xSemaphoreCreateMutex();

        OS_ASSERT(xDownQueue);
        OS_ASSERT(xUpQueue);
        OS_ASSERT(xConfigSemaphore);


        // create FreeRTOS task
        OS_TASK_CREATE("FTDF", /* Text name assigned to the task */
                prvFtdfTask,                  /* Function implementing the task */
                NULL,/* No parameter passed */
                mainTASK_STACK_SIZE, /* Size of the stack to allocate to task */
                mainTEMPLATE_TASK_PRIORITY,/* Priority of the task */
                xFtdfTaskHandle); /* No task handle */

        OS_ASSERT(xFtdfTaskHandle);
}

#endif /* !FTDF_PHY_API */
