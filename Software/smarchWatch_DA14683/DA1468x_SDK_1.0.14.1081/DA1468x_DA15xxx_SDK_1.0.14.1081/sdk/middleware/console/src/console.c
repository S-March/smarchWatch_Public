/**
 * \addtogroup BSP
 * \{
 * \addtogroup MIDDLEWARE
 * \{
 * \addtogroup CONSOLE
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file console.c
 *
 * @brief Serial console service for the Dialog Black Orca platform
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configUSE_CONSOLE

#if !dg_configUSE_CONSOLE_STUBS

#include <console.h>
#include <string.h>
#include <stdio.h>
#include <osal.h>
#include <sdk_defs.h>
#include <resmgmt.h>
#include "platform_devices.h"
#include "interrupts.h"

#if CONFIG_CONSOLE_RINGBUF_SIZE > 0
#       define RINGBUF_SIZE (CONFIG_CONSOLE_RINGBUF_SIZE)
#else
#       define RINGBUF_SIZE 256
#endif

/**
 * Console write timeout defined in ticks
 */
#if CONFIG_CONSOLE_WRITE_TIMEOUT > 0
#       define WRITE_TIMEOUT (CONFIG_CONSOLE_WRITE_TIMEOUT)
#else
#       define WRITE_TIMEOUT 0x2000
#endif

typedef struct {
        OS_MUTEX mutex;               /**< Mutex for reading clients */
        OS_TASK task;                 /**< Console task */
        OS_EVENT fifo_not_full;       /**< Event to wake up waiting writers */
        OS_EVENT read_finished;       /**< Event to wake up readers */
        int16_t read_size;            /**< Number of requested bytes */
        int16_t fifo_wrix;            /**< write ring buffer index */
        int16_t fifo_rdix;            /**< read ring buffer index */
        int16_t fifo_free;            /**< number of free bytes in fifo */
        uint32_t drop_count;          /**< number of bytes already dropped */
        bool fifo_blocked;            /**< flag indicating that fifo is blocked */
        char ring_buf[RINGBUF_SIZE];  /**< ring buffer */
        char *read_buf;               /**< user buffer provided for read */
} console_data_t;

static PRIVILEGED_DATA console_data_t console;

#define CONSOLE_WRITE_REQUEST           0x01
#define CONSOLE_WRITE_DONE              0x02
#define CONSOLE_READ_REQUEST            0x04
#define CONSOLE_READ_DONE               0x08

/*
 * Copy data to console ring buffer, caller must ensure that data will fit.
 * This function will be called inside critical section.
 */
static void console_write_to_ring_buffer(const char *ptr, int len)
{
        console.fifo_free -= len;
        if (console.fifo_wrix + len > sizeof(console.ring_buf)) {
                /*
                 * This is case when some data must be written at the end of ring buffer.
                 * and some from the beginning.
                 */
                int left;
                memcpy(console.ring_buf + console.fifo_wrix, ptr,
                                                        sizeof(console.ring_buf) - console.fifo_wrix);
                left  = len - (sizeof(console.ring_buf) - console.fifo_wrix);
                memcpy(console.ring_buf, ptr + sizeof(console.ring_buf) - console.fifo_wrix, left);
                console.fifo_wrix = left;
        } else {
                /* Simple case without overlap */
                memcpy(console.ring_buf + console.fifo_wrix, ptr, len);
                console.fifo_wrix += len;
                if (console.fifo_wrix >= sizeof(console.ring_buf)) {
                        console.fifo_wrix = 0;
                }
        }
}

int console_write(const char *buf, int len)
{
        int dropped;
        int left = len;

        for (;;) {
                dropped = 0;
                /*
                 * Put as much as possible data into ring buffer.
                 */
                OS_ENTER_CRITICAL_SECTION();
                if (left > console.fifo_free) {
                        /* not all can fit in ring buffer */
                        dropped = left - console.fifo_free;
                        left = console.fifo_free;
                }
                /* There was something to write this time, just put it in buffer */
                if (left) {
                        console_write_to_ring_buffer(buf, left);
                }
                /*
                 * If something was not fitting in ring buffer but we are in interrupt or
                 * FIFO is blocked, bad luck. Data will be just dropped forever.
                 */
                if (dropped && (in_interrupt() || console.fifo_blocked)) {
                        console.drop_count += dropped;
                        dropped = 0;
                }
                OS_LEAVE_CRITICAL_SECTION();

                /* If something was put in ring buffer notify task to take over printing */
                if (left) {
                        OS_TASK_NOTIFY_FROM_ISR(console.task, CONSOLE_WRITE_REQUEST,
                                                                                OS_NOTIFY_SET_BITS);
                }

                buf += left;
                left = 0;

                if (dropped) {
                        /*
                         * ring buffer did not took everything, let's wait for a while and try
                         * again. We can do this since this code is not interrupt.
                         */
                        left = dropped;
                        if (OS_EVENT_WAIT(console.fifo_not_full, WRITE_TIMEOUT) == OS_EVENT_SIGNALED) {
                                /*
                                 * Now some space should show up in ring buffer.
                                 */
                                continue;
                        }
                        /*
                         * Wait failed with timeout, don't try again. Just count dropped data.
                         */
                        console.drop_count += left;

                        /*
                         * Timeout is usually caused by flow control. Mark the FIFO as blocked and
                         * don't wait in next console_write attempts until there is no space in
                         * FIFO.
                         */
                        console.fifo_blocked = true;
                }
                break;
        }
        return len - left;
}

int console_read(char *ptr, int len)
{
        /*
         * Only one client can request read at a time.
         */
        OS_MUTEX_GET(console.mutex, OS_MUTEX_FOREVER);

        /*
         * Pass read request parameters to task.
         */
        console.read_size = len;
        console.read_buf = ptr;
        OS_TASK_NOTIFY(console.task, CONSOLE_READ_REQUEST, OS_NOTIFY_SET_BITS);

        /*
         * Let's wait for ad_uart_read to finish in console task context.
         */
        OS_EVENT_WAIT(console.read_finished, OS_EVENT_FOREVER);

        OS_MUTEX_PUT(console.mutex);

        return console.read_size;
}

/*
 * Callback function called when single write to UART finished.
 * This callback will wake up console task so next writes can start.
 */
static void console_write_cb(void *user_data, uint16_t transferred)
{
        console_data_t *console = (console_data_t *) user_data;

        OS_ENTER_CRITICAL_SECTION();

        /* Move read index and increase free FIFO counter */
        console->fifo_rdix += transferred;
        if (console->fifo_rdix >= sizeof(console->ring_buf)) {
                console->fifo_rdix -= sizeof(console->ring_buf);
        }
        console->fifo_free += transferred;
        console->fifo_blocked = false;

        OS_LEAVE_CRITICAL_SECTION();

        OS_TASK_NOTIFY_FROM_ISR(console->task, CONSOLE_WRITE_DONE, OS_NOTIFY_SET_BITS);
}

/*
 * Callback function called when read on UART ended.
 * This callback will wake up console task.
 */
static void console_read_cb(void *user_data, uint16_t transferred)
{
        console_data_t *console = (console_data_t *) user_data;
        console->read_size = transferred;
        OS_TASK_NOTIFY_FROM_ISR(console->task, CONSOLE_READ_DONE, OS_NOTIFY_SET_BITS);
}

static void console_task_fun(void *param)
{
        uint32_t pending_requests = 0;
        uint32_t current_requests;
        uint32_t mask = CONSOLE_WRITE_REQUEST | CONSOLE_READ_REQUEST;
        uart_device uart = ad_uart_open(SERIAL_CONSOLE);

        for (;;) {
                /*
                 * If there are some unmasked requests already, no need to wait for new ones.
                 */
                if ((pending_requests & mask) == 0) {
                        uint32_t new_bits = 0;
                        OS_TASK_NOTIFY_WAIT(0, ~0, &new_bits, OS_TASK_NOTIFY_FOREVER);
                        pending_requests |= new_bits;
                }
                /*
                 * Filter requests that are not masked, and remove ones that will be handled.
                 */
                current_requests = pending_requests & mask;
                pending_requests ^= current_requests;

                /*
                 * Ring buffer has some new data that should go to UART.
                 */
                if (0 != (current_requests & CONSOLE_WRITE_REQUEST) &&
                                                        console.fifo_free < sizeof(console.ring_buf)) {
                        uint16_t wx = console.fifo_wrix;
                        uint16_t size = 0;
                        if (console.fifo_rdix < wx) {
                                /*
                                 * In case when data to write is in one block in ring buffer
                                 * just print it in one run
                                 */
                                size = wx - console.fifo_rdix;
                        } else {
                                /*
                                 * This time data to print starts at the end of ring buffer.
                                 * UART will print this part first, and after writing that,
                                 * data at the beginning will be printed.
                                 */
                                size = sizeof(console.ring_buf) - console.fifo_rdix;
                                /*
                                 * Write request was already cleared, but here asked for it again.
                                 * This request will be masked till UART writes finishes.
                                 */
                                pending_requests |= CONSOLE_WRITE_REQUEST;
                        }

                        if (size > 0) {
                                /*
                                 * There was something to print, mask write request and wait for
                                 * write done, then start sending data.
                                 */
                                mask ^= CONSOLE_WRITE_REQUEST | CONSOLE_WRITE_DONE;
                                ad_uart_write_async(uart, console.ring_buf + console.fifo_rdix,
                                                                size, console_write_cb, &console);
                        }
                }

                if (0 != (current_requests & CONSOLE_WRITE_DONE)) {
                        /*
                         * UART finished printing, enable write request again, and notify clients.
                         */
                        mask ^= CONSOLE_WRITE_REQUEST | CONSOLE_WRITE_DONE;
                        OS_EVENT_SIGNAL(console.fifo_not_full);
                }

                if (0 != (current_requests & CONSOLE_READ_REQUEST)) {
                        /*
                         * Some task want to read from UART. Start reading, block read requests
                         * and wait for read done.
                         */
                        mask ^= CONSOLE_READ_DONE | CONSOLE_READ_REQUEST;
                        ad_uart_read_async(uart, console.read_buf, console.read_size,
                                                                        console_read_cb, &console);
                }

                if (0 != (current_requests & CONSOLE_READ_DONE)) {
                        /*
                         * Something was received. Enable read request again, and notify reader.
                         */
                        mask ^= CONSOLE_READ_DONE | CONSOLE_READ_REQUEST;
                        OS_EVENT_SIGNAL(console.read_finished);
                }
        }
}

void console_init(void)
{
        if (console.task) {
                return;
        }

        console.fifo_free = sizeof(console.ring_buf);
        OS_MUTEX_CREATE(console.mutex);
        OS_EVENT_CREATE(console.fifo_not_full);
        OS_EVENT_CREATE(console.read_finished);
        OS_TASK_CREATE("console", console_task_fun, NULL,
                configMINIMAL_STACK_SIZE * OS_STACK_WORD_SIZE, CONSOLE_TASK_PRIORITY, console.task);
}

#ifdef CONFIG_RETARGET
__attribute((externally_visible))
int _write (int fd, char *ptr, int len)
{
        console_write(ptr, len);

        return len;
}

__attribute((externally_visible))
int _read (int fd, char *ptr, int len)
{
        return console_read(ptr, 1);
}
#endif

#endif /* !dg_configUSE_CONSOLE_STUBS */

#endif /* dg_configUSE_CONSOLE */

/**
 * \}
 * \}
 * \}
 */
