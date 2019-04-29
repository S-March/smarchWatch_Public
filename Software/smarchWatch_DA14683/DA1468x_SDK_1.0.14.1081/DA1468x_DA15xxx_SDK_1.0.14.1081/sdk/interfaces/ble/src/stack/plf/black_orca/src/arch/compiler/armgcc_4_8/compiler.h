/**
 ****************************************************************************************
 *
 * @file rvds/compiler.h
 *
 * @brief Definitions of compiler specific directives.
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 * $Rev: 6863 $
 *
 ****************************************************************************************
 */

#ifndef _COMPILER_H_
#define _COMPILER_H_

#if defined ( __GNUC__ )

/// define the force inlining attribute for this compiler
#define __INLINE inline

/// define the IRQ handler attribute for this compiler
#define __IRQ __attribute__ ((interrupt))
/// define the BLE IRQ handler attribute for this compiler
#define __BTIRQ

/// define the BLE IRQ handler attribute for this compiler
#define __BLEIRQ

/// define the FIQ handler attribute for this compiler
#define __FIQ __attribute__ ((interrupt))
/// define size of an empty array (used to declare structure with an array size not defined)
#define __ARRAY_EMPTY

/// Put a variable in a memory maintained during deep sleep
#define __LOWPOWER_SAVED

/// Put a variable in a memory not maintained during deep sleep
#define __LOWPOWER_UNSAVED  __attribute__ ((section (".lpus")));

#else
#ifndef __ARMCC_VERSION
#error "File only included with RVDS!"
#endif // __ARMCC_VERSION

/// define the force inlining attribute for this compiler
#define __INLINE __forceinline static

/// define the IRQ handler attribute for this compiler
#define __IRQ __irq

/// define the BLE IRQ handler attribute for this compiler
#define __BTIRQ

/// define the BLE IRQ handler attribute for this compiler
#define __BLEIRQ

/// define the FIQ handler attribute for this compiler
#define __FIQ __irq
/// define size of an empty array (used to declare structure with an array size not defined)
#define __ARRAY_EMPTY

/// Put a variable in a memory maintained during deep sleep
#define __LOWPOWER_SAVED

/// Put a variable in a memory not maintained during deep sleep
#define __LOWPOWER_UNSAVED  __attribute__ ((section (".lpus")));
#endif // __GNUC__

#endif // _COMPILER_H_
