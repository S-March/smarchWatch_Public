/**
 * \addtogroup BSP
 * \{
 * \addtogroup SYSTEM
 * \{
 * \addtogroup Exception_Handlers
 * \{
 * \brief Generic Exception Handlers
 */

/**
 ****************************************************************************************
 *
 * @file hw_hard_fault.h
 *
 * @brief Hard-Fault Handler include file.
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_HARD_FAULT_H_
#define HW_HARD_FAULT_H_


#include <stdbool.h>
#include <stdint.h>
#include <sdk_defs.h>

#define HARDFAULT_MAGIC_NUMBER          0xBADC0FFE

/**
 * \brief Holds the stack contents when a hard-fault occurs.
 *
 * \details The stack contents are copied at this variable when a hard-fault occurs. The first
 *        position is marked with a special "flag" (0xBADC0FFE) to indicate that the data that
 *        follow are valid.
 */
extern uint32_t hardfault_event_data[9];

#endif /* HW_HARD_FAULT_H_ */


/**
 * \}
 * \}
 * \}
 */
