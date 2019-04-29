/**
 ****************************************************************************************
 *
 * @file co_utils.h
 *
 * @brief Common utilities definitions
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */
#ifndef _CO_UTILS_H_
#define _CO_UTILS_H_

/**
 ****************************************************************************************
 * @defgroup CO_UTILS Utilities
 * @ingroup COMMON
 * @brief  Common utilities
 *
 * This module contains the common utilities functions and macros.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include <stdint.h>       // standard definitions
#include <stddef.h>       // standard definitions
#include "co_bt.h"        // common bt definitions
#include "rwip_config.h"  // SW configuration
#include "compiler.h"     // for inline functions


/*
 * MACRO DEFINITIONS
 ****************************************************************************************
 */

/// Get the number of elements within an array
#define ARRAY_LEN(array) (sizeof((array))/sizeof((array)[0]))


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


/*
 * CONSTANT DECLARATIONS
 ****************************************************************************************
 */
/// Conversion table Sleep Clock Accuracy to PPM
extern const uint16_t co_sca2ppm[];

/// NULL BD address
extern const struct bd_addr co_null_bdaddr;

/// NULL BD address
extern const struct bd_addr co_default_bdaddr;

/*
 * MACROS
 ****************************************************************************************
 */

/// MACRO to build a subversion field from the Minor and Release fields
#define CO_SUBVERSION_BUILD(minor, release)     (((minor) << 8) | (release))


/// Macro to get a structure from one of its structure field
#define CONTAINER_OF(ptr, type, member)    ((type *)( (char *)ptr - offsetof(type,member) ))

/*
 * OPERATIONS ON BT CLOCK
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Clocks addition with 2 operands
 *
 * @param[in]   clock_a   1st operand value (in BT slots)
 * @param[in]   clock_b   2nd operand value (in BT slots)
 * @return      result    operation result (in BT slots)
 ****************************************************************************************
 */
#define CLK_ADD_2(clock_a, clock_b)     ((uint32_t)(((clock_a) + (clock_b)) & MAX_SLOT_CLOCK))

/**
 ****************************************************************************************
 * @brief Clocks addition with 3 operands
 *
 * @param[in]   clock_a   1st operand value (in BT slots)
 * @param[in]   clock_b   2nd operand value (in BT slots)
 * @param[in]   clock_c   3rd operand value (in BT slots)
 * @return      result    operation result (in BT slots)
 ****************************************************************************************
 */
#define CLK_ADD_3(clock_a, clock_b, clock_c)     ((uint32_t)(((clock_a) + (clock_b) + (clock_c)) & MAX_SLOT_CLOCK))

/**
 ****************************************************************************************
 * @brief Clocks subtraction
 *
 * @param[in]   clock_a   1st operand value (in BT slots)
 * @param[in]   clock_b   2nd operand value (in BT slots)
 * @return      result    operation result (in BT slots)
 ****************************************************************************************
 */
#define CLK_SUB(clock_a, clock_b)     ((uint32_t)(((clock_a) - (clock_b)) & MAX_SLOT_CLOCK))

/**
 ****************************************************************************************
 * @brief Cocks time difference
 *
 * @param[in]   clock_a   1st operand value (in BT slots)
 * @param[in]   clock_b   2nd operand value (in BT slots)
 * @return      result    return the time difference from clock A to clock B
 *                           - result < 0  => clock_b is in the past
 *                           - result == 0 => clock_a is equal to clock_b
 *                           - result > 0  => clock_b is in the future
 ****************************************************************************************
 */
#define CLK_DIFF(clock_a, clock_b)     ( (CLK_SUB((clock_b), (clock_a)) > ((MAX_SLOT_CLOCK+1) >> 1)) ?                      \
                          ((int32_t)((-CLK_SUB((clock_a), (clock_b))))) : ((int32_t)((CLK_SUB((clock_b), (clock_a))))) )

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Read an aligned 32 bit word.
 * @param[in] ptr32 The address of the first byte of the 32 bit word.
 * @return The 32 bit value.
 ****************************************************************************************
 */
__STATIC_INLINE uint32_t co_read32(void const *ptr32)
{
    return *((uint32_t*)ptr32);
}

/**
 ****************************************************************************************
 * @brief Read an aligned 16 bits word.
 * @param[in] ptr16 The address of the first byte of the 16 bits word.
 * @return The 16 bits value.
 ****************************************************************************************
 */
__STATIC_INLINE uint16_t co_read16(void const *ptr16)
{
    return *((uint16_t*)ptr16);
}

/**
 ****************************************************************************************
 * @brief Write an aligned 32 bits word.
 * @param[in] ptr32 The address of the first byte of the 32 bits word.
 * @param[in] value The value to write.
 ****************************************************************************************
 */
__STATIC_INLINE void co_write32(void const *ptr32, uint32_t value)
{
    *(uint32_t*)ptr32 = value;
}

/**
 ****************************************************************************************
 * @brief Write an aligned 16 bits word.
 * @param[in] ptr16 The address of the first byte of the 16 bits word.
 * @param[in] value The value to write.
 ****************************************************************************************
 */
__STATIC_INLINE void co_write16(void const *ptr16, uint32_t value)
{
    *(uint16_t*)ptr16 = value;
}

/**
 ****************************************************************************************
 * @brief Write a 8 bits word.
 * @param[in] ptr8 The address of the first byte of the 8 bits word.
 * @param[in] value The value to write.
 ****************************************************************************************
 */
__STATIC_INLINE void co_write8(void const *ptr8, uint32_t value)
{
    *(uint8_t*)ptr8 = value;
}

/**
 ****************************************************************************************
 * @brief Read a packed 16 bits word.
 * @param[in] ptr16 The address of the first byte of the 16 bits word.
 * @return The 16 bits value.
 ****************************************************************************************
 */
__STATIC_INLINE uint16_t co_read16p(void const *ptr16)
{
    uint16_t value = ((uint8_t *)ptr16)[0] | ((uint8_t *)ptr16)[1] << 8;
    return value;
}

/**
 ****************************************************************************************
 * @brief Read a packed 24 bits word.
 * @param[in] ptr24 The address of the first byte of the 24 bits word.
 * @return The 24 bits value.
 ****************************************************************************************
 */
__STATIC_INLINE uint32_t co_read24p(void const *ptr24)
{
    uint16_t addr_l, addr_h;
    addr_l = co_read16p((uint16_t *)ptr24);
    addr_h = *((uint16_t *)ptr24 + 1) & 0x00FF;
    return ((uint32_t)addr_l | (uint32_t)addr_h << 16);
}

/**
 ****************************************************************************************
 * @brief Write a packed 24 bits word.
 * @param[in] ptr24 The address of the first byte of the 24 bits word.
 * @param[in] value The value to write.
 ****************************************************************************************
 */
__STATIC_INLINE void co_write24p(void const *ptr24, uint32_t value)
{
    uint8_t *ptr=(uint8_t*)ptr24;

    *ptr++ = (uint8_t)(value&0xff);
    *ptr++ = (uint8_t)((value&0xff00)>>8);
    *ptr++ = (uint8_t)((value&0xff0000)>>16);
}

/**
 ****************************************************************************************
 * @brief Read a packed 32 bits word.
 * @param[in] ptr32 The address of the first byte of the 32 bits word.
 * @return The 32 bits value.
 ****************************************************************************************
 */
__STATIC_INLINE uint32_t co_read32p(void const *ptr32)
{
    uint16_t addr_l, addr_h;
    addr_l = co_read16p((uint16_t *)ptr32);
    addr_h = co_read16p((uint16_t *)ptr32 + 1);
    return ((uint32_t)addr_l | (uint32_t)addr_h << 16);
}
/**
 ****************************************************************************************
 * @brief Write a packed 32 bits word.
 * @param[in] ptr32 The address of the first byte of the 32 bits word.
 * @param[in] value The value to write.
 ****************************************************************************************
 */
__STATIC_INLINE void co_write32p(void const *ptr32, uint32_t value)
{
    uint8_t *ptr=(uint8_t*)ptr32;

    *ptr++ = (uint8_t)(value&0xff);
    *ptr++ = (uint8_t)((value&0xff00)>>8);
    *ptr++ = (uint8_t)((value&0xff0000)>>16);
    *ptr = (uint8_t)((value&0xff000000)>>24);
}

/**
 ****************************************************************************************
 * @brief Write a packed 16 bits word.
 * @param[in] ptr16 The address of the first byte of the 16 bits word.
 * @param[in] value The value to write.
 ****************************************************************************************
 */
__STATIC_INLINE void co_write16p(void const *ptr16, uint16_t value)
{
    uint8_t *ptr=(uint8_t*)ptr16;

    *ptr++ = value&0xff;
    *ptr = (value&0xff00)>>8;
}


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Compares two Bluetooth device addresses
 *
 * This function checks if the two bd address are equal.
 *
 * @param[in] bd_address1        Pointer on the first bd address to be compared.
 * @param[in] bd_address2        Pointer on the second bd address to be compared.
 *
 * @return result of the comparison (true or false).
 *
 ****************************************************************************************
 */
bool co_bdaddr_compare(struct bd_addr const *bd_address1,
                          struct bd_addr const *bd_address2);

#if (BT_EMB_PRESENT)

/**
 ******************************************************************************
 * @brief Convert an duration in baseband slot to a duration in number of ticks.
 * @param[in]  slot_cnt  Duration in number of baseband slot
 * @return  Duration (in number of ticks).
 ******************************************************************************
 */
uint32_t co_slot_to_duration(uint16_t slot_cnt);

#endif //BT_EMB_PRESENT

/// @} CO_UTILS

#endif // _CO_UTILS_H_
