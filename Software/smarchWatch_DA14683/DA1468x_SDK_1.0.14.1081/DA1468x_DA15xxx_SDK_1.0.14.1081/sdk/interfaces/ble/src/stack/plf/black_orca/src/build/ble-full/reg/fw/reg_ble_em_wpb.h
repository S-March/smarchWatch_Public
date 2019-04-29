#ifndef _REG_BLE_EM_WPB_H_
#define _REG_BLE_EM_WPB_H_

#include <stdint.h>
#include "_reg_ble_em_wpb.h"
#include "compiler.h"
#include "arch.h"
#include "em_map.h"
#include "reg_access.h"

#define REG_BLE_EM_WPB_COUNT 1

#define REG_BLE_EM_WPB_DECODING_MASK 0x00000000

#define REG_BLE_EM_WPB_ADDR_GET(idx) (EM_BLE_WPB_OFFSET + (idx) * REG_BLE_EM_WPB_SIZE)

/**
 * @brief WLPUB register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  15:00                WLPUB   0x0
 * </pre>
 */
#define BLE_WLPUB_ADDR   (0x00 + (_ble_base) + EM_BLE_WPB_OFFSET)
#define BLE_WLPUB_INDEX  0x00000000
#define BLE_WLPUB_RESET  0x00000000
#define BLE_WLPUB_COUNT  3

__STATIC_INLINE uint16_t ble_wlpub_get(int elt_idx, int reg_idx)
{
    ASSERT_ERR(reg_idx <= 2);
    return EM_BLE_RD(BLE_WLPUB_ADDR + elt_idx * REG_BLE_EM_WPB_SIZE + reg_idx * 2);
}

__STATIC_INLINE void ble_wlpub_set(int elt_idx, int reg_idx, uint16_t value)
{
    ASSERT_ERR(reg_idx <= 2);
    EM_BLE_WR(BLE_WLPUB_ADDR + elt_idx * REG_BLE_EM_WPB_SIZE + reg_idx * 2, value);
}

// field definitions
#define BLE_WLPUB_MASK   ((uint16_t)0x0000FFFF)
#define BLE_WLPUB_LSB    0
#define BLE_WLPUB_WIDTH  ((uint16_t)0x00000010)

#define BLE_WLPUB_RST    0x0

__STATIC_INLINE uint16_t ble_wlpub_getf(int elt_idx, int reg_idx)
{
    ASSERT_ERR(reg_idx <= 2);
    uint16_t localVal = EM_BLE_RD(BLE_WLPUB_ADDR + elt_idx * REG_BLE_EM_WPB_SIZE + reg_idx * 2);
    ASSERT_ERR((localVal & ~((uint16_t)0x0000FFFF)) == 0);
    return (localVal >> 0);
}

__STATIC_INLINE void ble_wlpub_setf(int elt_idx, int reg_idx, uint16_t wlpub)
{
    ASSERT_ERR(reg_idx <= 2);
    ASSERT_ERR((((uint16_t)wlpub << 0) & ~((uint16_t)0x0000FFFF)) == 0);
    EM_BLE_WR(BLE_WLPUB_ADDR + elt_idx * REG_BLE_EM_WPB_SIZE + reg_idx * 2, (uint16_t)wlpub << 0);
}


#endif // _REG_BLE_EM_WPB_H_

