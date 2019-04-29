/**
 ****************************************************************************************
 *
 * @file mcif_ascii.c
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <string.h>
#include <stdlib.h>

#include <ctype.h>

#include "sdk_defs.h"
#include "hw_uart.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "logging.h"

#include "mcif.h"
#include "mcif_internal.h"

#define  MCIF_ASCII_FRAMING_CHAR '\r'

PRIVILEGED_DATA uint8_t framebuffer[MCIF_ASCII_MAX_LINE + 1];
PRIVILEGED_DATA int pos;
extern int parsebuflen;

int mcif_parse_frame(uint8_t                      rxbyte[],
                     int                          len,
                     struct mcif_message_s      **rxmsg)
{
        int res = -1;

        for (int i=0; i<len; i++){
                switch (rxbyte[i])
                {
                case 8:
                        /* Backspace. Delete previous byte, erase rest of line in terminal */
                        pos--;
                        if (pos < 0) pos = 0;
                        framebuffer[pos] = '\0';
                        break;
                case '\r':
                case '\n':
                        /* increase pos to account for terminating null */
                        pos++;
                        framebuffer[pos] = '\0';
                        *rxmsg = OS_MALLOC(sizeof(struct mcif_message_s) + pos);
                        memcpy((*rxmsg)->buffer, framebuffer, pos);
                        (*rxmsg)->len = pos;
                        /* reset and prepare for next line input */
                        memset(framebuffer, 0, MCIF_ASCII_MAX_LINE + 1);
                        parsebuflen = 0;
                        pos = 0;
                        res = 0;
                        return res;
                default:
                        if (rxbyte[i]>=32 && pos < MCIF_ASCII_MAX_LINE-1) {
                                framebuffer[pos++] = rxbyte[i];
                                framebuffer[pos] = '\0';
                        }

                        if (pos == MCIF_ASCII_MAX_LINE-1) {
                                /* No more space in the buffer. Check if there is a command. */
                                pos++;
                                framebuffer[pos] = '\0';
                                *rxmsg = OS_MALLOC(sizeof(struct mcif_message_s) + pos);
                                memcpy((*rxmsg)->buffer, framebuffer, pos);
                                (*rxmsg)->len = pos;
                                /* reset and prepare for next line input */
                                memset(framebuffer, 0, MCIF_ASCII_MAX_LINE + 1);
                                parsebuflen = 0;
                                pos = 0;
                                res = 0;
                                return res;
                        }
                }
        }

        log_printf(LOG_DEBUG, MCIF_LOG_TAG, "Framebuffer: [%s]\r\n", framebuffer);
        return res;
}

void mcif_framing_init(void)
{
        pos = 0;
        framebuffer[0] = '\0';
}

static void mcif_ascii_print_simple_msg(
        const struct mcif_ascii_parse_element_s *parse_table,
        xQueueHandle txq,
        struct mcif_message_s *msg, const char *simple_msg)
{
        struct mcif_message_s *tx_msg;
        int len = strlen(simple_msg) + 1;

        tx_msg = OS_MALLOC(sizeof(struct mcif_message_s) + len);
        strcpy((char *)tx_msg->buffer, simple_msg);
        tx_msg->len = len;
        if (mcif_queue_send(0, &tx_msg, 0) != pdPASS) {
                OS_FREE(tx_msg);
        }
}

void mcif_ascii_print_prompt(
        const struct mcif_ascii_parse_element_s *parse_table,
        xQueueHandle txq,
        struct mcif_message_s *msg)
{
        mcif_ascii_print_simple_msg(parse_table, txq, msg, MCIF_ASCII_PROMPT);
}

static void mcif_ascii_print_einval(const struct mcif_ascii_parse_element_s *parse_table,
        const struct mcif_ascii_parse_element_s *elem,
        xQueueHandle txq,
        struct mcif_message_s *msg)
{
        struct mcif_message_s *tx_msg;
        int len = strlen(elem->help_str);
        char *bp;

        tx_msg = OS_MALLOC(sizeof(struct mcif_message_s) + sizeof(MCIF_ASCII_EINVAL) + len + 3);
        bp = tx_msg->buffer;
        strncpy(bp, MCIF_ASCII_EINVAL, sizeof(MCIF_ASCII_EINVAL));
        bp += sizeof(MCIF_ASCII_EINVAL);
        strncpy(bp, elem->help_str, len);
        bp += len;
        strcpy(bp, "\r\n");

        tx_msg->len = sizeof(MCIF_ASCII_PROMPT) + len + 3;

        if (mcif_queue_send(0, &tx_msg, 0) != pdPASS) {
                OS_FREE(tx_msg);
        }

        mcif_ascii_print_prompt(parse_table, txq, msg);
}

static inline void mcif_ascii_print_done(
        const struct mcif_ascii_parse_element_s *parse_table,
        xQueueHandle txq,
        struct mcif_message_s *msg, bool autoreply)
{
        if (autoreply) {
                mcif_ascii_print_simple_msg(parse_table, txq, msg, MCIF_ASCII_DONE_MESSAGE);
                mcif_ascii_print_prompt(parse_table, txq, msg);
        }
}

static void mcif_ascii_print_help(
        const struct mcif_ascii_parse_element_s *parse_table,
        xQueueHandle txq,
        struct mcif_message_s *msg)
{

        struct mcif_message_s *tx_msg;
        const struct mcif_ascii_parse_element_s *elem;
        int len = sizeof(MCIF_ASCII_HELP) + 1;
        char *bp;
        bool first = true;

        /* Compute space needed for all commands */
        for (elem = parse_table; elem->name != NULL; elem++) {
                /* Add 2 for comma and space */
                len += 2 + strlen(elem->name);
        }

        tx_msg = OS_MALLOC(sizeof(struct mcif_message_s) + len);
        memcpy(tx_msg->buffer, MCIF_ASCII_HELP, sizeof(MCIF_ASCII_HELP));

        bp = tx_msg->buffer + sizeof(MCIF_ASCII_HELP);

        for (elem = parse_table; elem->name != NULL; elem++) {
                if (first) {
                        first = false;
                }
                else {
                        memcpy(bp, ", ", 2);
                        bp += 2;
                }

                memcpy(bp, elem->name, strlen(elem->name));
                bp += strlen(elem->name);
        }
        strcpy(bp, "\r\n");
        bp += 3;

        tx_msg->len = bp - tx_msg->buffer;
        if (mcif_queue_send(0, &tx_msg, 0) != pdPASS) {
                OS_FREE(tx_msg);
        }
}

void mcif_ascii_print_unknown(
        const struct mcif_ascii_parse_element_s *parse_table,
        xQueueHandle txq,
        struct mcif_message_s *msg)
{
        mcif_ascii_print_simple_msg(parse_table, txq, msg, MCIF_ASCII_UNKNOWN_HEADER);
        mcif_ascii_print_help(parse_table, txq, msg);
}

static inline char * skip_whitespace(char *s)
{
        /* skip leading whitespaces */
        while (*s && isspace((int ) *s)) {
                s++;
        }

        return s;
}

void mcif_ascii_parse_message(
        const struct mcif_ascii_parse_element_s *parse_table,
        xQueueHandle txq,
        struct mcif_message_s *msg)
{
        int i;
        const struct mcif_ascii_parse_element_s *elem;
        char *bp = skip_whitespace(msg->buffer);
        char *be;
        char c;

        /* If user just pressed enter, show prompt */
        if (*bp == '\0') {
                mcif_ascii_print_prompt(parse_table, txq, msg);
                return;
        }

        if (!strcmp(bp, "?") || !strcmp(bp, "help")) {
                mcif_ascii_print_help(parse_table, txq, msg);
                mcif_ascii_print_prompt(parse_table, txq, msg);
                return;
        }

        /* Find where name ends, and temporarily close the string there. This
         * way, there can be an exact match for the command */
        be = strchr(bp, ' ');
        if (be != NULL) {
                c = *be;
                *be = '\0';
        }

        /* Search a matching command */
        for (elem = parse_table; elem->name != NULL; elem++) {
                if (!strcmp(elem->name, bp)) {
                        break;
                }
        }

        /* restore the string */
        *be = c;

        /* Unknown command entered */
        if (elem->name == NULL) {
                mcif_ascii_print_unknown(parse_table, txq, msg);
                mcif_ascii_print_prompt(parse_table, txq, msg);
                return;
        }

        /* Command found. Parse args if any */
        void *arg[MCIF_MAX_ARGS];

        for (i = 0; i < MCIF_MAX_ARGS; i++)
                arg[i] = NULL;
        int val[MCIF_MAX_ARGS];
        int argsnr = 0;

        /* Go to start of args (if any) */
        bp += strlen(elem->name);

        for (i = 0; i < MCIF_MAX_ARGS; i++) {
                /* skip whitespace, and go to start of next arg */
                bp = skip_whitespace(bp);

                /* arg i is an int */
                if (((elem->flags >> i * 2) & MCIF_ASCII_FLAGS_MASK) == MCIF_ASCII_FLAGS_ARG1_INT) {
                        if (sscanf(bp, "0x%x", &val[i]) != 1) {
                                if (sscanf(bp, "%d", &val[i]) != 1) {
                                        log_printf(LOG_DEBUG, MCIF_LOG_TAG,
                                                "Arg %d: Not an integer\r\n", i);
                                        mcif_ascii_print_einval(parse_table, elem, txq, msg);
                                        return;
                                }

                                /* Move pointer to next whitespace */
                                while (*bp && !isspace((int ) *bp)) {
                                        bp++;
                                }
                        }

                        log_printf(LOG_DEBUG, MCIF_LOG_TAG, "Parse int arg %d: %d\r\n", i, val[i]);
                        arg[i] = &val[i];
                        argsnr++;
                } else if (((elem->flags >> i * 2) & MCIF_ASCII_FLAGS_MASK) == MCIF_ASCII_FLAGS_ARG1_STR)  {
                        if (*bp == '\0') {
                                log_printf(LOG_DEBUG, MCIF_LOG_TAG,
                                        "Arg %d: String expected, got EOL\r\n", i);
                                mcif_ascii_print_einval(parse_table, elem, txq, msg);
                                return;
                        }

                        arg[i] = bp;
                        log_printf(LOG_DEBUG, MCIF_LOG_TAG, "Parse str arg %d: %s\r\n", i, bp);
                        /* This string is the reminder of args. Stop here */
                        argsnr++;
                        break;

                } else if (((elem->flags >> i * 2) & MCIF_ASCII_FLAGS_MASK) == MCIF_ASCII_FLAGS_ARG1_STR_NO_WHITE) {
                        if (*bp == '\0') {
                                log_printf(LOG_DEBUG, MCIF_LOG_TAG,
                                        "Arg %d: String expected, got EOL\r\n", i);
                                mcif_ascii_print_einval(parse_table, elem, txq, msg);
                                return;
                        }

                        arg[i] = bp;

                        /* Move pointer to next whitespace */
                        while (*bp && !isspace((int ) *bp)) {
                                bp++;
                        }

                        if (*bp) {
                                /* If there is more bytes in the line, mark the end of string */
                                *bp++ = '\0';
                        }

                        log_printf(LOG_DEBUG, MCIF_LOG_TAG, "Parse str (no white) arg %d: %s\r\n", i, bp);
                        argsnr++;
                } else {
                        log_printf(LOG_DEBUG, MCIF_LOG_TAG, "Stopped parsing args at %d\r\n", i);
                        /* Stop processing any further args */
                        break;

                }
        }

#if MCIF_MAX_ARGS > 2
        elem->fn(txq, arg);
#else
        elem->fn(txq, arg[0], arg[1]);
#endif

        mcif_ascii_print_done(parse_table, txq, msg,
                elem->flags & MCIF_ASCII_FLAGS_AUTO_REPLY);

}

void mcif_ascii_send_response(const struct mcif_ascii_parse_element_s *parse_table,
xQueueHandle txq,
        struct mcif_message_s *msg, bool show_prompt)
{
        if (mcif_queue_send(0, &msg, 0) != pdPASS) {
                OS_FREE(msg);
        }

        if (show_prompt)
                mcif_ascii_print_prompt(parse_table, txq, msg);
}

