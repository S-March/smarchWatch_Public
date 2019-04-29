/**
 ****************************************************************************************
 *
 * @file ll.h
 *
 * @brief Declaration of low level functions.
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef LL_H_
#define LL_H_

#ifndef __GNUC__
#error "File only included with ARM GCC"
#endif // __GNUC__

#include <stdint.h>
#include "arch.h"

#include "ARMCM0.h"


/** @brief Enable interrupts globally in the system.
 * This macro must be used when the initialization phase is over and the interrupts
 * can start being handled by the system.
 */



#define GLOBAL_INT_START() __enable_irq();

/** @brief Disable interrupts globally in the system.
 * This macro must be used when the system wants to disable all the interrupt
 * it could handle.
 */

#define GLOBAL_INT_STOP() __disable_irq();

/** @brief Disable interrupts globally in the system.
 * This macro must be used in conjunction with the @ref GLOBAL_INT_RESTORE macro since this
 * last one will close the brace that the current macro opens.  This means that both
 * macros must be located at the same scope level.
 */

//#define GLOBAL_INT_DISABLE()  __disable_irq();
//#define GLOBAL_INT_RESTORE() __enable_irq();


/** @brief Invoke the wait for interrupt procedure of the processor.
 *
 * @warning It is suggested that this macro is called while the interrupts are disabled
 * to have performed the checks necessary to decide to move to sleep mode.
 *
 */

#define WFI()   __WFI();


#endif // LL_H_
