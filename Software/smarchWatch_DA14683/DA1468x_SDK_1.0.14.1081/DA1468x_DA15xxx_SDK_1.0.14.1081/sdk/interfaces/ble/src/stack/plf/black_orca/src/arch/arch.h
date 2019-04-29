/**
 ****************************************************************************************
 *
 * @file arch.h
 *
 * @brief This file contains the definitions of the macros and functions that are
 * architecture dependent.  The implementation of those is implemented in the
 * appropriate architecture directory.
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */


#ifndef _ARCH_H_
#define _ARCH_H_

/**
 ****************************************************************************************
 * @defgroup REFIP
 * @brief Reference IP Platform
 *
 * This module contains reference platform components - REFIP.
 *
 *
 * @{
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup DRIVERS
 * @ingroup REFIP
 * @brief Reference IP Platform Drivers
 *
 * This module contains the necessary drivers to run the platform with the
 * RW BT SW protocol stack.
 *
 * This has the declaration of the platform architecture API.
 *
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>             // standard integer definition
#include "compiler.h"           // inline functions
#include "user_config_defs.h"

#include "sdk_defs.h"

#include "ble_config.h"

/*
 * CPU WORD SIZE
 ****************************************************************************************
 */
/// ARM is a 32-bit CPU
#define CPU_WORD_SIZE           4

/*
 * CPU Endianness
 ****************************************************************************************
 */
/// ARM is little endian
#define CPU_LE                  1

/*
 * DEBUG configuration
 ****************************************************************************************
 */
#if defined(CFG_DBG)
#define PLF_DEBUG               1
#else //CFG_DBG
#define PLF_DEBUG               0
#endif //CFG_DBG

/*
 * NVDS
 ****************************************************************************************
 */

/// NVDS
#ifdef CFG_NVDS
#define PLF_NVDS                1
#else // CFG_NVDS
#define PLF_NVDS                0
#endif // CFG_NVDS

/*
 * LLD ROM defines
 ****************************************************************************************
 */
struct lld_sleep_env_tag
{
        uint32_t irq_mask;
};

/*
 * UART
 ****************************************************************************************
 */

/// UART
#define PLF_UART                1

/*
 * DEFINES
 ****************************************************************************************
 */
/*
 * Deep sleep threshold. Application specific. Control if during deep sleep the system RAM will be powered off and if OTP copy will be required.
 ****************************************************************************************
*/		
//#if (DEEP_SLEEP)
/// Sleep Duration Value in periodic wake-up mode
#define MAX_SLEEP_DURATION_PERIODIC_WAKEUP_DEF          0x0320  // 0.5s
/// Sleep Duration Value in external wake-up mode
#define MAX_SLEEP_DURATION_EXTERNAL_WAKEUP_DEF          0x3E80  // 10s
//#endif //DEEP_SLEEP

/// Possible errors detected by FW
#define RESET_NO_ERROR          0x00000000
#define RESET_MEM_ALLOC_FAIL    0xF2F2F2F2

/// Reset platform and stay in ROM
#define RESET_TO_ROM            0xA5A5A5A5

/// Reset platform and reload FW
#define RESET_AND_LOAD_FW       0xC3C3C3C3

/*
 * EXPORTED FUNCTION DECLARATION
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Compute size of SW stack used.
 *
 * This function is compute the maximum size stack used by SW.
 *
 * @return Size of stack used (in bytes)
 ****************************************************************************************
 */
uint16_t get_stack_usage(void);

/**
 ****************************************************************************************
 * @brief Re-boot FW.
 *
 * This function is used to re-boot the FW when error has been detected, it is the end of
 * the current FW execution.
 * After waiting transfers on UART to be finished, and storing the information that
 * FW has re-booted by itself in a non-loaded area, the FW restart by branching at FW
 * entry point.
 *
 * Note: when calling this function, the code after it will not be executed.
 *
 * @param[in] error      Error detected by FW
 ****************************************************************************************
 */
void platform_reset(uint32_t error);

//#if PLF_DEBUG
///**
// ****************************************************************************************
// * @brief Print the assertion error reason and loop forever.
// *
// * @param condition C string containing the condition.
// * @param file C string containing file where the assertion is located.
// * @param line Line number in the file where the assertion is located.
// ****************************************************************************************
// */
//void assert_err(const char *condition, const char * file, int line);
//
///**
// ****************************************************************************************
// * @brief Print the assertion error reason and loop forever.
// * The parameter value that is causing the assertion will also be disclosed.
// *
// * @param param parameter value that is caused the assertion.
// * @param file C string containing file where the assertion is located.
// * @param line Line number in the file where the assertion is located.
// ****************************************************************************************
// */
//void assert_param(int param0, int param1, const char * file, int line);
//
///**
// ****************************************************************************************
// * @brief Print the assertion warning reason.
// *
// * @param condition C string containing the condition.
// * @param file C string containing file where the assertion is located.
// * @param line Line number in the file where the assertion is located.
// ****************************************************************************************
// */
//void assert_warn(const char *condition, const char * file, int line);
//#endif //PLF_DEBUG


/*
 * ASSERTION CHECK
 ****************************************************************************************
 */
/// Assertions showing a critical error that could require a full system reset
#define ASSERT_ERR(cond) 

/// Assertions showing a critical error that could require a full system reset
#define ASSERT_INFO(cond, param0, param1)

/// Assertions showing a non-critical problem that has to be fixed by the SW
#define ASSERT_WARN(cond)


/*
 * Unused BLE hooks (not used in the ROM code). They constitute a "hole" in the RetRAM that could be
 * used (in principle) for storing variables or other data.
 */
extern unsigned char (*custom_preinit)(void);
extern unsigned char (*custom_postinit)(void);
extern unsigned char (*custom_appinit)(void);
extern unsigned char (*custom_preloop)(void);
extern unsigned char (*custom_preschedule)(void);
extern unsigned char (*custom_postschedule)(void);
extern unsigned char (*custom_postschedule_async)(void);
extern unsigned char (*custom_presleepcheck)(void);
extern unsigned char (*custom_appsleepset)(void *sleep_mode);
extern unsigned char (*custom_postsleepcheck)(void *sleep_mode);
extern unsigned char (*custom_presleepenter)(void *sleep_mode);
extern unsigned char (*custom_postsleepexit)(void *sleep_mode);
extern unsigned char (*custom_prewakeup)(void);
extern unsigned char (*custom_postwakeup)(void);
extern unsigned char (*custom_preidlecheck)(void);

extern const uint32_t rom_func_addr_table_var[];

#define CHECK_AND_CALL(name)                            (name != 0 ? (*name)() : 0)
#define CHECK_AND_CALL_WITH_VAR(name, v)                (name != 0 ? (*name)(v) : 0)

/*
 * Variants of the above macros, which cast away the return value.
 * Convenient to use when you don't care about the return value, to avoid compilation warnings.
 */
#define CHECK_AND_CALL_VOID(func_ptr)                   (void)CHECK_AND_CALL(func_ptr)
#define CHECK_AND_CALL_WITH_VAR_VOID(func_ptr, v)       (void)CHECK_AND_CALL_WITH_VAR(func_ptr, v)

/// @} DRIVERS
#endif // _ARCH_H_
