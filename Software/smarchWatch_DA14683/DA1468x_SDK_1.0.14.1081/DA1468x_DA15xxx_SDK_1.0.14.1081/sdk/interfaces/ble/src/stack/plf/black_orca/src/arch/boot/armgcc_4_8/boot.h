/**
 ****************************************************************************************
 *
 * @file boot.h
 *
 * @brief This file contains the declarations of the boot related variables.
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef _BOOT_H_
#define _BOOT_H_

/// Address of beginning of the CODE
extern char code_base;
#define CODE_BASE (&(code_base))

/// Address of the end of the CODE
extern char code_end;
#define CODE_END (&(code_end))

/// Length of the code
#define CODE_LENGTH ((CODE_END) - (CODE_BASE))

/// Address of beginning of the DATA
extern char data_base;
#define DATA_BASE (&(data_base))

/// Address of the end of the DATA
extern char data_end;
#define DATA_END (&(data_end))

/// Length of the DATA
#define DATA_LENGTH ((DATA_END) - (DATA_BASE))

/// Unloaded RAM area base address
extern char unloaded_area_start;
#define RAM_UNLOADED_BASE   (&(unloaded_area_start))

/// Stack base address

extern char stack_base_unused;
#define STACK_BASE_UNUSED   (&(stack_base_unused))
extern char stack_len_unused;
#define STACK_LEN_UNUSED   (&(stack_len_unused))

extern char stack_base_svc ;
#define STACK_BASE_SVC   (&(stack_base_svc))

extern char stack_len_svc;
#define STACK_LEN_SVC  (&(stack_len_svc))

extern char stack_base_irq;
#define STACK_BASE_IRQ  (&(stack_base_irq))
extern char stack_len_irq;
#define STACK_LEN_IRQ   (&(stack_len_irq))

extern char stack_base_fiq;
#define STACK_BASE_FIQ   (&(stack_base_fiq))
extern char stack_len_fiq;
#define STACK_LEN_FIQ   (&(stack_len_fiq))

#define BOOT_PATTERN_UNUSED   0xAA      // Pattern to fill UNUSED stack
#define BOOT_PATTERN_SVC      0xBB      // Pattern to fill SVC stack
#define BOOT_PATTERN_IRQ      0xCC      // Pattern to fill IRQ stack
#define BOOT_PATTERN_FIQ      0xDD      // Pattern to fill FIQ stack


#endif // _BOOT_H_
