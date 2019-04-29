 /**
 ****************************************************************************************
 *
 * @file periph_setup.h
 *
 * @brief Peripheral Setup file.
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifndef _PERIPH_SETUP_H_
#define _PERIPH_SETUP_H_

#define glue(a, b) a##b
#define glue2(a, b) glue(a, b)
#define UART_ID glue2(HW_UART, LOADER_UART)

void periph_init(void);
void periph_deinit(void);

#endif /* _PERIPH_SETUP_H_ */
