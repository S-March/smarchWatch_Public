/**
 \addtogroup UTILITIES
 \{
 */

/**
 ****************************************************************************************
 *
 * @file logging.h
 *
 * @brief Logging API
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>

#include "queue.h"
#include "task.h"

#include "hw_uart.h"
#include "hw_gpio.h"

#include "mcif.h"

/**
 * \brief Debug levels
 *
 * Debug levels, in ascending level of importance. Please use these enums. Using
 * arbitrary numbers may lead to a crash.
 *
 */
typedef enum {
        LOG_DEBUG = 0,
        LOG_NOTICE,
        LOG_WARNING,
        LOG_ERROR,
        LOG_CRITICAL
} logging_severity_e;

/** @name CONFIGURATION
 *
 * The logging module can be configured in four distinct, mutually exclusive
 * modes.
 *
 * The STANDALONE mode uses a queue into which messages are inserted. For each
 * message, a buffer is allocated. A logging-specific task is instantiated, that
 * dequeues the messages, one at a time, writes them to uart and then frees up
 * the buffer. The queue is used to provide 1. rate-decoupling, i.e. absorb
 * peaks of logging rate, and 2. to provide atomicity, i.e. each message will be
 * printed on its entirety on the UART; no messages will be mixed, even if
 * logged simultaneously by two different tasks.
 * Messages are ASCII-encoded, i.e. they can be viewed using a simple terminal.
 *
 * The QUEUE mode also uses a queue into which messages are inserted (with all
 * the benefits mentioned in the STANDALONE section). In this
 * case, however, messages are NOT dequeued and written to uart by a logging-
 * specific task, but by the generic serial-link communication task, used by
 * the entire system. In this case, the log message will be encapsulated by the
 * serial link framework into the link-layer PDU (and, obviously, it may not
 * be able to be viewed by a simple terminal, depending on the encapsulating
 * frame format/protocol).
 *
 * The RETARGET mode uses the standard RETARGET infrastructure to write to the
 * UART. Retarget initialization must be done by the system init (i.e. external
 * to the logging module). CONFIG_RETARGET must be set system wide for this to
 * work. Messages may be garbled if many tasks log simultaneously, or very
 * fast.
 *
 * The RTT mode uses the standard RTT infrastructure to write to the
 * UART. CONFIG_RTT must be set system wide for this to work. The same
 * limitations as in RETARGET are applicable.
 *
 */
///@{

/* Logging modes. Choose one in the config file
        #undef LOGGING_MODE_STANDALONE
        #undef LOGGING_MODE_QUEUE
        #undef LOGGING_MODE_RETARGET
        #undef LOGGING_MODE_RTT
*/

#if defined(LOGGING_MODE_STANDALONE) || defined(LOGGING_MODE_QUEUE) || defined(LOGGING_MODE_RETARGET) || defined(LOGGING_MODE_RTT)
#define LOGGING_ENABLED
#else
#undef LOGGING_ENABLED
#endif


/**
 * \brief Minimum compiled severity level
 *
 * Logs with a severity less than this will not be compiled in
 *
 */
#ifndef LOGGING_MIN_COMPILED_SEVERITY
#define LOGGING_MIN_COMPILED_SEVERITY LOG_DEBUG
#endif

/**
 * \brief Minimum default severity level
 *
 * This is the default minimum severity level (if not changed in runtime using
 * log_set_severity())
 *
 */
#ifndef LOGGING_MIN_DEFAULT_SEVERITY
#define LOGGING_MIN_DEFAULT_SEVERITY LOG_DEBUG
#endif

/**
 * \brief Minimum size of free heap before malloc
 *
 * If the free system heap before calling malloc for the log message buffer is
 * less than this number (in bytes), the log will be suppressed. This ensures that
 * logs don't fill up the system memory.
 *
 */
#ifndef LOGGING_MIN_ALLOWED_FREE_HEAP
#define LOGGING_MIN_ALLOWED_FREE_HEAP 600
#endif

#ifdef LOGGING_MODE_STANDALONE
/**
 * \brief LOGGING_USE_DMA
 *
 * If set, DMA will be used for writing the message to the
 * UART. Otherwise, the UART will be polled. (STANDALONE mode only)
 */
#ifndef LOGGING_USE_DMA
#define LOGGING_USE_DMA  1
#endif

/**
 * \brief STANDALONE mode UART
 *
 * Define the UART to use in standalone mode
 */
#ifndef LOGGING_STANDALONE_UART
#define LOGGING_STANDALONE_UART           HW_UART
#endif



/**
 * \brief MCIF UART TX GPIO PORT
 *
 * Define the GPIO port to use for the TX pin
 */
#ifndef LOGGING_STANDALONE_GPIO_PORT_UART_TX
#define LOGGING_STANDALONE_GPIO_PORT_UART_TX HW_GPIO_PORT_1
#endif

/**
 * \brief MCIF UART TX GPIO PIN
 *
 * Define the GPIO pin to use for the TX pin
 */
#ifndef LOGGING_STANDALONE_GPIO_PIN_UART_TX
#define LOGGING_STANDALONE_GPIO_PIN_UART_TX HW_GPIO_PIN_0
#endif

/**
 * \brief MCIF UART RX GPIO PORT
 *
 * Define the GPIO port to use for the RX pin
 */
#ifndef LOGGING_STANDALONE_GPIO_PORT_UART_RX
#define LOGGING_STANDALONE_GPIO_PORT_UART_RX HW_GPIO_PORT_1
#endif

/**
 * \brief MCIF UART RX GPIO PIN
 *
 * Define the GPIO pin to use for the RX pin
 */
#ifndef LOGGING_STANDALONE_GPIO_PIN_UART_RX
#define LOGGING_STANDALONE_GPIO_PIN_UART_RX HW_GPIO_PIN_5
#endif



/**
 * \brief STANDALONE mode UART baudrate
 *
 * Define the UART baudrate to use in standalone mode
 */
#ifndef LOGGING_STANDALONE_UART_BAUDRATE
#define LOGGING_STANDALONE_UART_BAUDRATE  HW_UART_BAUDRATE_115200
#endif

/**
 * \brief STANDALONE mode UART data bits
 *
 * Define the UART data bits to use in standalone mode
 */
#ifndef LOGGING_STANDALONE_UART_DATABITS
#define LOGGING_STANDALONE_UART_DATABITS  HW_UART_DATABITS_8
#endif

/**
 * \brief STANDALONE mode UART stop bits
 *
 * Define the UART stop bits to use in standalone mode
 */
#ifndef LOGGING_STANDALONE_UART_STOPBITS
#define LOGGING_STANDALONE_UART_STOPBITS  HW_UART_STOPBITS_1
#endif

/**
 * \brief STANDALONE mode UART parity
 *
 * Define the UART parity to use in standalone mode
 */
#ifndef LOGGING_STANDALONE_UART_PARITY
#define LOGGING_STANDALONE_UART_PARITY    HW_UART_PARITY_NONE
#endif

#endif /* STANDALONE_MODE */

#if defined(LOGGING_MODE_STANDALONE) || defined (LOGGING_MODE_QUEUE)

/**
 * \brief Logging queue length
 *
 * In Standalone or Queue mode, defines the number of log entries available in
 * the logging queue. When the queue fills up, any additional entries will be
 * silently discarded.
 */
#ifndef LOGGING_QUEUE_LENGTH
#define LOGGING_QUEUE_LENGTH 12
#endif

/**
 * \brief Minimum message size
 *
 * When in Standalone or Queue mode, the log_printf function will first
 * allocate a buffer of LOGGING_MIN_MSG_SIZE size and attempt to fill it with
 * the parsed log message. If the buffer doesn't fit the message, it will be
 * freed and a new buffer will be allocated with enough space to fill the, then
 * known, parsed message.
 *
 * This value should be large enough to accomodate most messages with incurring
 * the extra processing, but also small enough to avoid unnecessary space waste.
 *
 */
#ifndef LOGGING_MIN_MSG_SIZE
#define LOGGING_MIN_MSG_SIZE 16
#endif

/**
 * \brief Enable suppressed logs counter
 *
 * If set to 1, suppressed logs (i.e. logs dropped because the log queue is
 * full) will be counted. When the queue gets empty, a log indicating the
 * number of suppressed messages will be sent to the queue.
 *
 * NOTE: This involves a critical section, so it may incur an overhead, if set
 *
 */
#ifndef LOGGING_SUPPRESSED_COUNT_ENABLE
#define LOGGING_SUPPRESSED_COUNT_ENABLE 1
#endif

/**
 * \brief Minimum number of suppressed logs to report
 *
 * If LOGGING_SUPPRESSED_COUNT_ENABLE is set, this one defines the minimum
 * number of suppressed count to report. This can be used to avoid overloading
 * the logs with suppressed message reports.
 *
 */
#ifndef LOGGING_SUPPRESSED_MIN_COUNT
#define LOGGING_SUPPRESSED_MIN_COUNT 5
#endif

/**
 * \brief Suppressed messages report template
 *
 * A printf-like template to be used for the message logged to
 * report suppressed messages. Must include a single %d
 */
#ifndef LOGGING_SUPPRESSED_MSG_TMPL
#define LOGGING_SUPPRESSED_MSG_TMPL "%d messages were suppressed\n\r"
#endif

/**
 * \brief Suppressed messages report severity
 *
 * The severity of the suppressed messages report log. Obeys the
 * same rules as normal logs.
 *
 */
#ifndef LOGGING_SUPPRESSED_SEVERITY
#define LOGGING_SUPPRESSED_SEVERITY LOG_NOTICE
#endif

/**
 * \brief Suppressed messages report tag
 *
 * The tag to use for the suppressed messages report log. A short
 * integer.
 */
#ifndef LOGGING_SUPPRESSED_TAG
#define LOGGING_SUPPRESSED_TAG 0
#endif

#endif /* defined(LOGGING_MODE_STANDALONE) || defined (LOGGING_MODE_QUEUE) */
///@}

/**
 * \brief Initialization function
 *
 * This must be called once (and only once) before any call is made to any other
 * logging function, in order to initialize the logging framework.
 */
void log_init(void);

/**
 * \brief Set minimum severity level
 *
 * This function can be used during runtime to change the minimum severity
 * level.
 *
 * NOTE: This is NOT thread safe AND race conditions may occur. However, the
 * worst that can happen is the loss of a log message. Therefore, it doesn't
 * make any sense to incur the overhead to make it atomic (using a critical
 * section)
 *
 * \param[in] severity - The new minimum severity level
 */
void log_set_severity(logging_severity_e severity);

/* Internal use */
extern logging_severity_e logging_min_severity;
extern const char logging_severity_chars[];

#if defined(LOGGING_MODE_STANDALONE) || defined(LOGGING_MODE_QUEUE)
void log_printf_raw(const char *fmt, ...);
#define LOG_FUNCTION log_printf_raw

#elif defined(LOGGING_MODE_RETARGET) || defined(LOGGING_MODE_RTT)
#define LOG_FUNCTION printf

#else
#define LOG_FUNCTION
#endif

/**
 * \brief Log a message
 *
 * The main logging function. It is used by all logging modes. It creates and
 * writes a log of the form:
 *
 *    [\<tick\>] \<S\> \<T\> \<message\>
 *
 *    where:
 *       \<tick\>: the FreeRTOS tick number at the time of the macro invocation
 *       \<S\>: The log severity, represented by a single letter. One of
 *            D, N, W, E and C
 *       \<T\>: The log tag. A small number (0, 1, etc...)
 *       \<message\>: The actual log message
 *
 * This function (in Standalone or Queue mode) allocates / frees memory. It
 * MUST NOT be used from an ISR.
 *
 * \param[in] severity - A logging_severity_e enum value. Represents the severity
 *            level for the log. If this is >= LOGGING_MIN_COMPILED_SEVERITY
 *            and >= the current runtime severity (as set by log_set_severity()
 *            and LOGGING_MIN_DEFAULT_SEVERITY), the log will be created.
 * \param[in] tag - A small number that can be used to identify a specific
 *            set of messages. Application specific
 * \param[in] format - The template of the message, as in printf(3)
 * \param[in] args - The list of arguments for the message, as in printf(3)
 *
 */
#ifdef LOGGING_ENABLED
#define log_printf(severity, tag, format, args...) \
                do {\
                        if ((severity) >= LOGGING_MIN_COMPILED_SEVERITY && \
                                        (severity) >= logging_min_severity) { \
                                LOG_FUNCTION(\
                                                "[%d] %c %d " format, OS_GET_TICK_COUNT(), \
                                                logging_severity_chars[(severity) & 0x7], (tag) , ##args); \
                        } \
                } while (0)
#else
#define log_printf(severity, tag, format, args...)
#endif
#endif /* LOGGING_H */
/**
 \}
 */
