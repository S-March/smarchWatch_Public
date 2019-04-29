/**
 \addtogroup UTILITIES
 \{
 */

/**
 ****************************************************************************************
 *
 * @file mcif.h
 *
 * @brief Monitor and Control I/F API
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef MCIF_H
#define MCIF_H
#include "osal.h"
#include "queue.h"

/** @name CONFIGURATION
 *
 * Configuration of MCIF
 */
///@{
/**
 * \brief Maximum number of MCIF clients
 *
 * The maximum number of MCIF clients.
 * Currently must be set to 1 (only MCIF_ASCII is implemented, that supports
 * a single client)
 *
 */

#define MCIF_CLIENTS_NR 1

/**
 * \brief Use task notifications to indicate a new frame
 *
 * If set to 1, task notifications will be used to notify the
 * application task that a new frame is in the queue
 */
#ifndef MCIF_USE_TASK_NOTIFICATIONS
#define MCIF_USE_TASK_NOTIFICATIONS 0
#endif

/**
 * \brief Maximum number of arguments to use
 *
 * Range: [2, 6]
 */
#ifndef MCIF_MAX_ARGS
#define MCIF_MAX_ARGS 2
#endif

/**
 * \brief MCIF UART
 *
 * Define the UART to use for MCIF
 *
 */
#ifndef MCIF_UART
#define MCIF_UART HW_UART1
#endif

/**
 * \brief MCIF UART RX GPIO PORT
 *
 * Define the GPIO port to use for the RX pin
 */
#ifndef MCIF_GPIO_PORT_UART_RX
#define MCIF_GPIO_PORT_UART_RX HW_GPIO_PORT_1
#endif

/**
 * \brief MCIF UART RX GPIO PIN
 *
 * Define the GPIO pin to use for the RX pin
 */
#ifndef MCIF_GPIO_PIN_UART_RX
#define MCIF_GPIO_PIN_UART_RX HW_GPIO_PIN_5
#endif

/**
 * \brief MCIF UART TX GPIO PORT
 *
 * Define the GPIO port to use for the TX pin
 */
#ifndef MCIF_GPIO_PORT_UART_TX
#define MCIF_GPIO_PORT_UART_TX HW_GPIO_PORT_1
#endif

/**
 * \brief MCIF UART TX GPIO PIN
 *
 * Define the GPIO pin to use for the TX pin
 */
#ifndef MCIF_GPIO_PIN_UART_TX
#define MCIF_GPIO_PIN_UART_TX HW_GPIO_PIN_0
#endif


/**
 * \brief MCIF UART baudrate
 *
 * Define the UART baudrate to use
 *
 */
#ifndef MCIF_UART_BAUDRATE
#define MCIF_UART_BAUDRATE HW_UART_BAUDRATE_115200
#endif

/**
 * \brief MCIF UART databits
 *
 * Define the UART databits to use
 *
 */
#ifndef MCIF_UART_DATABITS
#define MCIF_UART_DATABITS HW_UART_DATABITS_8
#endif

/**
 * \brief MCIF UART stopbits
 *
 * Define the UART stopbits to use
 *
 */
#ifndef MCIF_UART_STOPBITS
#define MCIF_UART_STOPBITS HW_UART_STOPBITS_1
#endif

/**
 * \brief MCIF UART parity
 *
 * Define the UART parity to use
 *
 */
#ifndef MCIF_UART_PARITY
#define MCIF_UART_PARITY HW_UART_PARITY_NONE
#endif

/**
 * \brief MCIF LOG TAG
 *
 * Define the LOG TAG (the tag used when logging using the logging
 * library)
 *
 */
#ifndef MCIF_LOG_TAG
#define MCIF_LOG_TAG 30
#endif

/**
 * \brief MCIF ASCII max line
 *
 * Define the maximum line length for input
 *
 */
#ifndef MCIF_ASCII_MAX_LINE
#define MCIF_ASCII_MAX_LINE 80
#endif

/**
 * \brief MCIF MAX DMA Buffer
 *
 * Define the maximum DMA buffer to be used for UART input
 *
 */
#ifndef MCIF_UART_DMA_BUFFER
#define MCIF_UART_DMA_BUFFER MCIF_ASCII_MAX_LINE
#else
#if (MCIF_UART_DMA_BUFFER < MCIF_ASCII_MAX_LINE)
#undef MCIF_UART_DMA_BUFFER
#define MCIF_UART_DMA_BUFFER MCIF_ASCII_MAX_LINE
#endif
#endif

/**
 * \brief MCIF ASCII prompt
 *
 * This is the prompt shown on the terminal. Can be changed, if needed.
 *
 */
#ifndef MCIF_ASCII_PROMPT
#define MCIF_ASCII_PROMPT "\r\nEnter command (or ?/help for help) > "
#endif

///@}

/** @name MCIF message buffer
 *
 * A message passed to MCIF must be contained in this struct. This struct
 * must be allocated with a size equal to sizeof(struct mcif_message_s) + l,
 * where l is the message length (that in the case of MCIF-ASCII must include
 * the terminating null). The len parameter must contain the length of the
 * message stored in the buffer parameter.
 * When going towards MCIF, the struct will be freed by MCIF. On the other
 * direction, towards the client task, it must be freed by the task.
 *
 *
 */
struct mcif_message_s {
        uint16_t len; /**< The length of the message in buffer */
        char  buffer[0]; /**< The buffer to store the message in */
};

/** @name MCIF ASCII Specific Definitions
 *
 * The following definitions are specific to the MCIF ASCII backend.
 * The MCIF ASCII backend can be used to provide a simple terminal-like
 * interface to the application through the UART.
 *
 */
///@{

#define MCIF_ASCII_FLAGS_MASK 0x3

/**
 * \brief ASCII command-set argument X not-applicable flag
 *
 * When this flag is set on a command, argument X (and
 * subsequently, arguments > X as well) is not applicable.
 * i.e. this command does not accept an argument
 */
#define MCIF_ASCII_FLAGS_ARG1_NA 0x0
#define MCIF_ASCII_FLAGS_ARG2_NA 0x0
#define MCIF_ASCII_FLAGS_ARG3_NA 0x0
#define MCIF_ASCII_FLAGS_ARG4_NA 0x0
#define MCIF_ASCII_FLAGS_ARG5_NA 0x0
#define MCIF_ASCII_FLAGS_ARG6_NA 0x0

/**
 * \brief ASCII command-set integer argument X
 *
 * When this flag is set on a command, argument X of the command
 * is expected to be an integer, expressed either in decimal, or in
 * hex, using the 0x prefix (e.g. 0xcafe).
 */
#define MCIF_ASCII_FLAGS_ARG1_INT 0x1
#define MCIF_ASCII_FLAGS_ARG2_INT 0x4
#define MCIF_ASCII_FLAGS_ARG3_INT 0x10
#define MCIF_ASCII_FLAGS_ARG4_INT 0x40
#define MCIF_ASCII_FLAGS_ARG5_INT 0x100
#define MCIF_ASCII_FLAGS_ARG6_INT 0x400

/**
 * \brief ASCII command-set string argument X
 *
 * When this flag is set on a command, argument X of the command
 * is expected to be a string. Note that the string comprises the
 * rest of the entered line, including any spaces. Therefore,
 * if this is set, there can be no second argument to the command
 *
 */
#define MCIF_ASCII_FLAGS_ARG1_STR 0x2
#define MCIF_ASCII_FLAGS_ARG2_STR 0x8
#define MCIF_ASCII_FLAGS_ARG3_STR 0x20
#define MCIF_ASCII_FLAGS_ARG4_STR 0x80
#define MCIF_ASCII_FLAGS_ARG5_STR 0x200
#define MCIF_ASCII_FLAGS_ARG6_STR 0x800

/**
 * \brief ASCII command-set string with no whitespace argument X
 *
 * When this flag is set on a command, argument X of the command
 * is expected to be a string. This differs from ARGX_STR in that
 * this string must have no whitespace, and therefore, additional
 * arguments are allowed in the line
 *
 */
#define MCIF_ASCII_FLAGS_ARG1_STR_NO_WHITE 0x3
#define MCIF_ASCII_FLAGS_ARG2_STR_NO_WHITE 0xc
#define MCIF_ASCII_FLAGS_ARG3_STR_NO_WHITE 0x30
#define MCIF_ASCII_FLAGS_ARG4_STR_NO_WHITE 0xc0
#define MCIF_ASCII_FLAGS_ARG5_STR_NO_WHITE 0x300
#define MCIF_ASCII_FLAGS_ARG6_STR_NO_WHITE 0xc00

/**
 * \brief ASCII command-set auto-reply flag
 *
 * When this flag is set on a command, when the command returns,
 * the MCIF-ASCII framework will automatically reply "OK" to the
 * user. This is a convenience option, for commands that always
 * succeed.
 */
#if MCIF_MAX_ARGS < 4
#define MCIF_ASCII_FLAGS_AUTO_REPLY 0x80
#else
#define MCIF_ASCII_FLAGS_AUTO_REPLY 0x8000
#endif

#if MCIF_MAX_ARGS > 2
typedef void (*cmd_cb_t)(xQueueHandle txq, void **args);
#else
typedef void (*cmd_cb_t)(xQueueHandle txq, void *arg1, void *arg2);
#endif

/**
 * \brief MCIF-ASCII command description structure
 *
 * This structure is used to describe to MCIF-ASCII framework a terminal
 * command.
 * The function called (fn) receives the MCIF-ASCII transmit queue (that can be
 * used to send messages to the terminal, if needed), and pointers to the two
 * parsed arguments. The arguments are already parsed, according to the type defined
 * by the command flags, so all that is needed in the command implementation is to
 * cast them to the proper type (e.g. int v = *(int *)arg1;). It is the responsibility
 * of the user function to make sure it does valid casting of the arguements, and it
 * doesn't attempt to use not defined arguments.
 *
 */
struct mcif_ascii_parse_element_s
{
        const char *name; /**< The command name. This is used to enter the command  */
#if MCIF_MAX_ARGS > 2
        void (*fn)(xQueueHandle txq, void **args); /**< Pointer to the command user function */
#else
        void (*fn)(xQueueHandle txq, void *arg1, void *arg2); /**< Pointer to the command user function */
#endif
        const char *help_str; /**< The string displayed for command usage */
#if MCIF_MAX_ARGS < 4
        uint8_t flags; /**< Command flags as defined by the MCIF_ASCII_FLAGS_* macros */
#else
        uint16_t flags; /**< Command flags as defined by the MCIF_ASCII_FLAGS_* macros */
#endif
};

/**
 * \brief Parse the received message
 *
 * This function is the main entry point of the MCIF-ASCII framework in the task
 * application. After receiving the message from the MCIF-ASCII RX queue, the
 * application can call this function to parse the message and call the appropriate
 * user callback.
 *
 * \param [in] parse_table - a pointer to an array of struct mcif_ascii_parse_element_s
 * elements, that each define a user command. The array must be NULL terminated
 * (i.e. the last item on the array must be a NULL).
 *
 * \param [in] txq - the MCIF-ASCII transmit queue. Can be used to send responses
 * back to the terminal
 *
 * \param [in] msg - a pointer to the message received that is to be parsed.
 *
 */
void mcif_ascii_parse_message(
        const struct mcif_ascii_parse_element_s *parse_table,
        xQueueHandle txq,
        struct mcif_message_s *msg);

/**
 * \brief Print a prompt
 *
 * This function can be used (usually from within a command user callback) to print
 * a response to the terminal. This is usually done after completing the command
 * processing so that the user can continue entering more commands.
 * Additionally, this function is also called internally by the framework when a
 * prompt needs to be displayed (e.g. when MCIF_ASCII_FLAGS_AUTO_REPLY is set for
 * the command, or when the command entered in the terminal is invalid).
 *
 * \param [in] parse_table - a pointer to an array of struct mcif_ascii_parse_element_s
 * elements, that each define a user command. The array must be NULL terminated
 * (i.e. the last item on the array must be a NULL).
 *
 * \param [in] txq - the MCIF-ASCII transmit queue. Can be used to send responses
 * back to the terminal
 *
 * \param [in] msg - a pointer to the message received (if needed for prompt). It must
 * be freed sometime after the call to mcif_ascii_parse_message() by the user app.
 *
 */
void mcif_ascii_print_prompt(
        const struct mcif_ascii_parse_element_s *parse_table,
        xQueueHandle txq,
        struct mcif_message_s *msg) __attribute__((weak));

/**
 * \brief Print the unknown command message
 *
 * This function is used by the framework to print an error message and a help
 * screen, based on the available commands, when an unknown command is entered
 * by the user. This function is defined as weak and can be therefore overridden
 * if a different output is needed in this case.
 *
 * \param [in] parse_table - a pointer to an array of struct mcif_ascii_parse_element_s
 * elements, that each define a user command. The array must be NULL terminated
 * (i.e. the last item on the array must be a NULL).
 *
 * \param [in] txq - the MCIF-ASCII transmit queue. Can be used to send responses
 * back to the terminal
 *
 * \param [in] msg - a pointer to the message received (if needed by an overriding
 * function)
 *
 */
void mcif_ascii_print_unknown(
        const struct mcif_ascii_parse_element_s *parse_table,
        xQueueHandle txq,
        struct mcif_message_s *msg) __attribute__((weak));

/**
 * \brief Send a response to the terminal
 *
 * This function is used to send a generic response to the terminal. The response
 *
 * \param [in] parse_table - a pointer to an array of struct mcif_ascii_parse_element_s
 * elements, that each define a user command. The array must be NULL terminated
 * (i.e. the last item on the array must be a NULL).
 *
 * \param [in] txq - the MCIF-ASCII transmit queue. Can be used to send responses
 * back to the terminal
 *
 * \param [in] msg - a pointer to the message to be sent to the terminal. This
 * buffer will be freed by MCIF-ASCII.
 *
 * \param [in] show_prompt - If set to true, after displaying the msg to the terminal,
 * MCIF-ASCII will also display a prompt.
 *
 */
void mcif_ascii_send_response(const struct mcif_ascii_parse_element_s *parse_table,
        xQueueHandle txq,
        struct mcif_message_s *msg, bool show_prompt);
///@}

/**
 * \brief Setup the MCIF queues.
 *
 * This function is used by the application to setup the queues to be used by
 * the MCIF framework for this application. The queues must be allocated and
 * initialized by the application. The client id (cli_id) is an identifier for
 * the specific MCIF client (the application). It must be less than MCIF_CLIENTS_NR.
 * For the special case of MCIF-ASCII, cli_id must be set to zero, and, obviously,
 * MCIF-ASCII can only have a single client.
 *
 * NOTE: THIS FUNCTION MUST BE CALLED BEFORE mcif_init()
 *
 * \param [in] cli_id - The ID of the client to setup the queues for.
 *
 * \param [out] txq - The queue to use to transmit messages to MCIF
 *
 * \param [in] rxq - The queue to use to receive messages from MCIF
 *
 */
void mcif_setup_queues(int cli_id, OS_QUEUE txq, QueueHandle_t rxq);


#if MCIF_USE_TASK_NOTIFICATIONS == 1
/**
 * \brief Setup MCIF client task notifications.
 *
 * This function is used by the application to indicate to the MCIF framework that
 * it wants to receive a task notification each time a new frame is enqeueud in the
 * MCIF RX queue towards the application. The client id (cli_id) is an identifier for
 * the specific MCIF client (the application). It must be less than MCIF_CLIENTS_NR.
 * For the special case of MCIF-ASCII, cli_id must be set to zero, and, obviously,
 * MCIF-ASCII can only have a single client.
 *
 * NOTE: THIS FUNCTION MUST BE CALLED BEFORE mcif_init()
 *
 * \param [in] cli_id - The ID of the client to setup the queues for.
 *
 * \param [in] handle - The receiving task's OS handle, to send the notification to.
 *                       If NULL, no notification will be send to this task, when a
 *                       new frame is enqueued in its respective queue
 *
 * \param [in] notif_bit - The task notifications bit number to use for notifying this task.
 *                         NOTE: This is NOT the bit mask, but the bit number (range: [0, 31])
 *
 */

void mcif_setup_client_notifications(int cli_id, OS_TASK handle, uint8_t notif_bit);
#endif

/**
 * \brief Send a message to MCIF
 *
 * This function is used to send a message to MCIF.
 * This function is NOT needed for MCIF-ASCII, since mcif_ascii_send_response() etc
 * functions call this one internally.
 *
 * \param [in] cli_id - The ID of the client to send a message to.
 * \param [in] item - A pointer to the pointer to the message buffer.
 * \param [in] wait_ticks - The number of OS ticks to wait for the queue to have
 *                          available space.
 *
 * \return Returns true if the message was sent, False if not
 * (i.e. the queue was full and the timeout expired)
 */
OS_BASE_TYPE mcif_queue_send(int cli_id, const void *item,
        OS_TICK_TIME wait_ticks);

/**
 * \brief Initialize MCIF framework
 *
 * Initialize the MCIF framework. This includes initializing the framing backend
 * (currently, only MCIF-ASCII is available), setting up the UART (except from the
 * GPIOs) and creating the MCIF OS task.
 *
 */
void mcif_init(void);

#endif /* MCIF_H */
/**
 \}
 */
