/**
 ****************************************************************************************
 *
 * @file reg_access.h
 *
 * @brief File implementing the basic primitives for register accesses
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef REG_ACCESS_H_
#define REG_ACCESS_H_

/**
 ****************************************************************************************
 * @addtogroup REG REG_ACCESS
 * @ingroup DRIVERS
 *
 * @brief Basic primitives for register access
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <string.h>            // string functions
#if defined(CFG_EMB)
//#include "_reg_common_em_et.h"    // exchange table
#include "co_utils.h"
#include "em_map.h"       // EM Map
#endif // CFG_EMB
/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * MACROS
 ****************************************************************************************
 */
/// Macro to read a platform register
#define REG_PL_RD(addr)              (*(volatile uint32_t *)(addr))

/// Macro to write a platform register
#define REG_PL_WR(addr, value)       (*(volatile uint32_t *)(addr)) = (value)

/// Macro to read a BLE register
#define REG_BLE_RD(addr)             (*(volatile uint32_t *)(addr))

/// Macro to write a BLE register
#define REG_BLE_WR(addr, value)      (*(volatile uint32_t *)(addr)) = (value)

/// Macro to read a BLE control structure field (16-bit wide)
#define EM_BLE_RD(addr)              (*(volatile uint16_t *)(addr))

/// Macro to write a BLE control structure field (16-bit wide)
#define EM_BLE_WR(addr, value)       (*(volatile uint16_t *)(addr)) = (value)

/// Macro to read a BT register
#define REG_BT_RD(addr)              (*(volatile uint32_t *)(addr))

/// Macro to write a BT register
#define REG_BT_WR(addr, value)       (*(volatile uint32_t *)(addr)) = (value)

/// Macro to read a BT control structure field (16-bit wide)
#define EM_BT_RD(addr)               (*(volatile uint16_t *)(addr))

/// Macro to write a BT control structure field (16-bit wide)
#define EM_BT_WR(addr, value)        (*(volatile uint16_t *)(addr)) = (value)

/// Macro to read a EM field (16-bit wide)
#define EM_RD(addr)               (*(volatile uint16_t *)(addr))

/// Macro to write a EM field (16-bit wide)
#define EM_WR(addr, value)        (*(volatile uint16_t *)(addr)) = (value)
/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

#if (defined(CFG_BT) || (defined(CFG_BLE) && defined(CFG_EMB)))
/// Read bytes from EM
__STATIC_INLINE void em_rd(void *sys_addr, uint16_t em_addr, uint16_t len)
{
    memcpy(sys_addr, (void *)(em_addr + EM_BASE_ADDR), len);
}
/// Write bytes to EM
__STATIC_INLINE void em_wr(void const *sys_addr, uint16_t em_addr, uint16_t len)
{
    memcpy((void *)(em_addr + EM_BASE_ADDR), sys_addr, len);
}

/// Read 32-bits value from EM
__STATIC_INLINE uint32_t em_rd32p(uint16_t em_addr)
{
    return co_read32p((void *)(em_addr + EM_BASE_ADDR));
}
/// Write 32-bits value to EM
__STATIC_INLINE void em_wr32p(uint16_t em_addr, uint32_t value)
{
    co_write32p((void *)(em_addr + EM_BASE_ADDR), value);
}

/// Read 16-bits value from EM
__STATIC_INLINE uint16_t em_rd16p(uint16_t em_addr)
{
    return co_read16p((void *)(em_addr + EM_BASE_ADDR));
}
/// Write 16-bits value to EM
__STATIC_INLINE void em_wr16p(uint16_t em_addr, uint16_t value)
{
    co_write16p((void *)(em_addr + EM_BASE_ADDR), value);
}

/// Read 8-bits value from EM
__STATIC_INLINE uint16_t em_rd8p(uint16_t em_addr)
{
    return *((uint8_t *)(em_addr + EM_BASE_ADDR));
}
/// Write 8-bits value to EM
__STATIC_INLINE void em_wr8p(uint16_t em_addr, uint8_t value)
{
    *(uint8_t *)(em_addr + EM_BASE_ADDR) = value;
}
#endif // (defined(CFG_BT) || (defined(CFG_BLE) && defined(CFG_EMB)))
#if defined(CFG_BT)
/// EM setting
__STATIC_INLINE void em_bt_set(int value, uint16_t em_addr, uint16_t len)
{
    memset((void *)(em_addr + REG_COMMON_EM_ET_BASE_ADDR), value, len);
}
#endif // CFG_BT
/// @} REG

#endif // REG_ACCESS_H_
