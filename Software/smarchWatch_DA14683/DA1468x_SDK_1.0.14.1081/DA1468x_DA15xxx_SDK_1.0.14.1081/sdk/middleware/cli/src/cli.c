/**
 * \addtogroup BSP
 * \{
 * \addtogroup MIDDLEWARE
 * \{
 * \addtogroup CLI
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file cli.c
 *
 * @brief CLI service for the Dialog Black Orca platform
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configUSE_CLI

#if !dg_configUSE_CLI_STUBS

#if !dg_configUSE_CONSOLE
#       error "cli requires dg_configUSE_CONSOLE to be enabled!"
#endif

#include <string.h>
#include <osal.h>
#include <console.h>
#include "cli.h"

#if CONFIG_CLI_LINEBUF_SIZE > 0
#       define LINEBUF_SIZE    (CONFIG_CLI_LINEBUF_SIZE)
#else
#       define LINEBUF_SIZE    64
#endif

#if CONFIG_CLI_ARGC_MAX > 0
#       define ARGC_MAX        (CONFIG_CLI_ARGC_MAX)
#else
#       define ARGC_MAX        10
#endif

#if CONFIG_CLI_QUEUE_LEN > 0
#       define QUEUE_LEN        (CONFIG_CLI_QUEUE_LEN)
#else
#       define QUEUE_LEN        1
#endif

typedef struct {
        OS_TASK task;
        uint32_t notif_mask;
        OS_QUEUE queue;

        const cli_command_t *commands;
        cli_handler_t def_handler;
} client_t;

/* cli task handle */
PRIVILEGED_DATA static OS_TASK task;

/* Registered client data */
PRIVILEGED_DATA static client_t reg_client;

/* Current command line buffer */
PRIVILEGED_DATA static struct {
        char *buf;
        size_t len;
} line;

static void notify_client(void)
{
        if (!line.len) {
                goto done;
        }

        line.buf[line.len] = '\0';

        if (OS_QUEUE_PUT(reg_client.queue, &line.buf, OS_QUEUE_NO_WAIT) != OS_QUEUE_OK) {
                OS_FREE(line.buf);
        } else {
                OS_TASK_NOTIFY(reg_client.task, reg_client.notif_mask, OS_NOTIFY_SET_BITS);
        }

done:
        line.buf = OS_MALLOC(LINEBUF_SIZE + 1);
        line.len = 0;
}

static void cli_task_func(void *param)
{
        /* Dummy call to initialize line */
        notify_client();

        for (;;) {
                console_read(line.buf + line.len, 1);

                if (!line.len && (line.buf[line.len] == '\r' || line.buf[line.len] == '\n')) {
                        continue;
                }

                if (line.buf[line.len] == 0x7f /* (del) */) {
                        if (line.len > 0) {
                                line.len--;
                        }
                        continue;
                }

                if (line.buf[line.len] == '\r' || line.buf[line.len] == '\n') {
                        notify_client();
                        continue;
                }

                if (line.len < LINEBUF_SIZE) {
                        line.len++;
                }
        }
}

void cli_init(void)
{
        if (task) {
                OS_ASSERT(0);
                return;
        }

        OS_TASK_CREATE("cli", cli_task_func, NULL, configMINIMAL_STACK_SIZE * OS_STACK_WORD_SIZE,
                CLI_TASK_PRIORITY, task);
}

cli_t cli_register(uint32_t notif_mask, const cli_command_t commands[],
                                                                cli_handler_t def_handler)
{
        if (reg_client.task) {
                OS_ASSERT(0);
                return NULL;
        }

        reg_client.task = OS_GET_CURRENT_TASK();
        reg_client.notif_mask = notif_mask;
        reg_client.commands = commands;
        reg_client.def_handler = def_handler;
        OS_QUEUE_CREATE(reg_client.queue, sizeof(typeof(line.buf)), QUEUE_LEN);

        return (cli_t) &reg_client;
}

void cli_handle_notified(cli_t cli)
{
        client_t *client = (client_t *) cli;
        char *line_buf;
        char *ptr;
        int argc;
        const char *argv[ARGC_MAX];
        const cli_command_t *cmd;

        if (!client) {
                OS_ASSERT(0);
                return;
        }

        if (OS_QUEUE_GET(client->queue, &line_buf, OS_QUEUE_NO_WAIT) != OS_QUEUE_OK) {
                return;
        }

        ptr = line_buf;
        while (*ptr == ' ' || *ptr == '\t') {
                ptr++;
        }

        argv[0] = ptr;
        argc = 1;

        while (argc < ARGC_MAX) {
                ptr = strpbrk(ptr, " \t");
                if (!ptr) {
                        break;
                }

                while (*ptr == ' ' || *ptr == '\t') {
                        *ptr = '\0';
                        ptr++;
                }

                if (!(*ptr)) {
                        break;
                }

                argv[argc++] = ptr;
        }

        for (cmd = client->commands; cmd && cmd->name; cmd++) {
                if (!strcmp(cmd->name, argv[0])) {
                        cmd->handler(argc, argv, cmd->user_data);
                        goto done;
                }
        }

        if (client->def_handler) {
                client->def_handler(argc, argv, NULL);
        }

done:
        OS_FREE(line_buf);
}

#endif /* !dg_configUSE_CLI_STUBS */

#endif /* dg_configUSE_CLI */

/**
 * \}
 * \}
 * \}
 */
