/**
 ****************************************************************************************
 *
 * @file mcif_internal.h
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef MCIF_INTERNAL_H
#define MCIF_INTERNAL_H

#include "osal.h"
#include "queue.h"

#include "mcif.h"

#define MCIF_ASCII_UNKNOWN_HEADER "\r\nERROR: Unknown command.\r\n  "
#define MCIF_ASCII_HELP "\r\nAvailable commands:\r\n\r\n  "
#define MCIF_ASCII_EINVAL "\r\nERROR: Invalid arguments. Usage:\r\n\r\n  "
#define MCIF_ASCII_DONE_MESSAGE "\r\nOK\r\n"
#define MCIF_ASCII_FLAGS_ARG1_MASK 0x3
#define MCIF_ASCII_FLAGS_ARG2_MASK 0xC

#define MCIF_HALF_DUPLEX_PROTO

struct mcif_client
{
        uint8_t msgid;
        OS_QUEUE txq;
        OS_QUEUE rxq;

};

int mcif_parse_frame(uint8_t rxbyte[], int len, struct mcif_message_s **rxmsg);

void mcif_framing_init(void);
#endif /* MCIF_INTERNAL_H */
