#ifndef _REG_COMMON_EM_ET_H_
#define _REG_COMMON_EM_ET_H_

#include <stdint.h>
#include "_reg_common_em_et.h"
#include "compiler.h"
#include "arch.h"
#include "em_map.h"
#include "reg_access.h"

#define REG_COMMON_EM_ET_COUNT 2

#define REG_COMMON_EM_ET_DECODING_MASK 0x00000003

#define REG_COMMON_EM_ET_ADDR_GET(idx) (EM_ET_OFFSET + (idx) * REG_COMMON_EM_ET_SIZE)

/**
 * @brief EXTAB0 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  05:04               STATUS   0x0
 *  03:00                 MODE   0x0
 * </pre>
 */
#define EM_COMMON_EXTAB0_ADDR   (0x00 + (_ble_base) + EM_ET_OFFSET)
#define EM_COMMON_EXTAB0_INDEX  0x00000000
#define EM_COMMON_EXTAB0_RESET  0x00000000

__STATIC_INLINE uint16_t em_common_extab0_get(int elt_idx)
{
    return EM_RD(EM_COMMON_EXTAB0_ADDR + elt_idx * REG_COMMON_EM_ET_SIZE);
}

__STATIC_INLINE void em_common_extab0_set(int elt_idx, uint16_t value)
{
    EM_WR(EM_COMMON_EXTAB0_ADDR + elt_idx * REG_COMMON_EM_ET_SIZE, value);
}

// field definitions
#define EM_COMMON_STATUS_MASK   ((uint16_t)0x00000030)
#define EM_COMMON_STATUS_LSB    4
#define EM_COMMON_STATUS_WIDTH  ((uint16_t)0x00000002)
#define EM_COMMON_MODE_MASK     ((uint16_t)0x0000000F)
#define EM_COMMON_MODE_LSB      0
#define EM_COMMON_MODE_WIDTH    ((uint16_t)0x00000004)

#define EM_COMMON_STATUS_RST    0x0
#define EM_COMMON_MODE_RST      0x0

__STATIC_INLINE void em_common_extab0_pack(int elt_idx, uint8_t status, uint8_t mode)
{
    ASSERT_ERR((((uint16_t)status << 4) & ~((uint16_t)0x00000030)) == 0);
    ASSERT_ERR((((uint16_t)mode << 0) & ~((uint16_t)0x0000000F)) == 0);
    EM_WR(EM_COMMON_EXTAB0_ADDR + elt_idx * REG_COMMON_EM_ET_SIZE,  ((uint16_t)status << 4) | ((uint16_t)mode << 0));
}

__STATIC_INLINE void em_common_extab0_unpack(int elt_idx, uint8_t* status, uint8_t* mode)
{
    uint16_t localVal = EM_RD(EM_COMMON_EXTAB0_ADDR + elt_idx * REG_COMMON_EM_ET_SIZE);

    *status = (localVal & ((uint16_t)0x00000030)) >> 4;
    *mode = (localVal & ((uint16_t)0x0000000F)) >> 0;
}

__STATIC_INLINE uint8_t em_common_extab0_status_getf(int elt_idx)
{
    uint16_t localVal = EM_RD(EM_COMMON_EXTAB0_ADDR + elt_idx * REG_COMMON_EM_ET_SIZE);
    return ((localVal & ((uint16_t)0x00000030)) >> 4);
}

__STATIC_INLINE void em_common_extab0_status_setf(int elt_idx, uint8_t status)
{
    ASSERT_ERR((((uint16_t)status << 4) & ~((uint16_t)0x00000030)) == 0);
    EM_WR(EM_COMMON_EXTAB0_ADDR + elt_idx * REG_COMMON_EM_ET_SIZE, (EM_RD(EM_COMMON_EXTAB0_ADDR + elt_idx * REG_COMMON_EM_ET_SIZE) & ~((uint16_t)0x00000030)) | ((uint16_t)status << 4));
}

__STATIC_INLINE uint8_t em_common_extab0_mode_getf(int elt_idx)
{
    uint16_t localVal = EM_RD(EM_COMMON_EXTAB0_ADDR + elt_idx * REG_COMMON_EM_ET_SIZE);
    return ((localVal & ((uint16_t)0x0000000F)) >> 0);
}

__STATIC_INLINE void em_common_extab0_mode_setf(int elt_idx, uint8_t mode)
{
    ASSERT_ERR((((uint16_t)mode << 0) & ~((uint16_t)0x0000000F)) == 0);
    EM_WR(EM_COMMON_EXTAB0_ADDR + elt_idx * REG_COMMON_EM_ET_SIZE, (EM_RD(EM_COMMON_EXTAB0_ADDR + elt_idx * REG_COMMON_EM_ET_SIZE) & ~((uint16_t)0x0000000F)) | ((uint16_t)mode << 0));
}

/**
 * @brief EXTAB1 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *     15           EXCPTRNRDY   0
 *  14:00                CSPTR   0x0
 * </pre>
 */
#define EM_COMMON_EXTAB1_ADDR   (0x02 + (_ble_base) + EM_ET_OFFSET)
#define EM_COMMON_EXTAB1_INDEX  0x00000001
#define EM_COMMON_EXTAB1_RESET  0x00000000

__STATIC_INLINE uint16_t em_common_extab1_get(int elt_idx)
{
    return EM_RD(EM_COMMON_EXTAB1_ADDR + elt_idx * REG_COMMON_EM_ET_SIZE);
}

__STATIC_INLINE void em_common_extab1_set(int elt_idx, uint16_t value)
{
    EM_WR(EM_COMMON_EXTAB1_ADDR + elt_idx * REG_COMMON_EM_ET_SIZE, value);
}

// field definitions
#define EM_COMMON_EXCPTRNRDY_BIT    ((uint16_t)0x00008000)
#define EM_COMMON_EXCPTRNRDY_POS    15
#define EM_COMMON_CSPTR_MASK        ((uint16_t)0x00007FFF)
#define EM_COMMON_CSPTR_LSB         0
#define EM_COMMON_CSPTR_WIDTH       ((uint16_t)0x0000000F)

#define EM_COMMON_EXCPTRNRDY_RST    0x0
#define EM_COMMON_CSPTR_RST         0x0

__STATIC_INLINE void em_common_extab1_pack(int elt_idx, uint8_t excptrnrdy, uint16_t csptr)
{
    ASSERT_ERR((((uint16_t)excptrnrdy << 15) & ~((uint16_t)0x00008000)) == 0);
    ASSERT_ERR((((uint16_t)csptr << 0) & ~((uint16_t)0x00007FFF)) == 0);
    EM_WR(EM_COMMON_EXTAB1_ADDR + elt_idx * REG_COMMON_EM_ET_SIZE,  ((uint16_t)excptrnrdy << 15) | ((uint16_t)csptr << 0));
}

__STATIC_INLINE void em_common_extab1_unpack(int elt_idx, uint8_t* excptrnrdy, uint16_t* csptr)
{
    uint16_t localVal = EM_RD(EM_COMMON_EXTAB1_ADDR + elt_idx * REG_COMMON_EM_ET_SIZE);

    *excptrnrdy = (localVal & ((uint16_t)0x00008000)) >> 15;
    *csptr = (localVal & ((uint16_t)0x00007FFF)) >> 0;
}

__STATIC_INLINE uint8_t em_common_extab1_excptrnrdy_getf(int elt_idx)
{
    uint16_t localVal = EM_RD(EM_COMMON_EXTAB1_ADDR + elt_idx * REG_COMMON_EM_ET_SIZE);
    return ((localVal & ((uint16_t)0x00008000)) >> 15);
}

__STATIC_INLINE void em_common_extab1_excptrnrdy_setf(int elt_idx, uint8_t excptrnrdy)
{
    ASSERT_ERR((((uint16_t)excptrnrdy << 15) & ~((uint16_t)0x00008000)) == 0);
    EM_WR(EM_COMMON_EXTAB1_ADDR + elt_idx * REG_COMMON_EM_ET_SIZE, (EM_RD(EM_COMMON_EXTAB1_ADDR + elt_idx * REG_COMMON_EM_ET_SIZE) & ~((uint16_t)0x00008000)) | ((uint16_t)excptrnrdy << 15));
}

__STATIC_INLINE uint16_t em_common_extab1_csptr_getf(int elt_idx)
{
    uint16_t localVal = EM_RD(EM_COMMON_EXTAB1_ADDR + elt_idx * REG_COMMON_EM_ET_SIZE);
    return ((localVal & ((uint16_t)0x00007FFF)) >> 0);
}

__STATIC_INLINE void em_common_extab1_csptr_setf(int elt_idx, uint16_t csptr)
{
    ASSERT_ERR((((uint16_t)csptr << 0) & ~((uint16_t)0x00007FFF)) == 0);
    EM_WR(EM_COMMON_EXTAB1_ADDR + elt_idx * REG_COMMON_EM_ET_SIZE, (EM_RD(EM_COMMON_EXTAB1_ADDR + elt_idx * REG_COMMON_EM_ET_SIZE) & ~((uint16_t)0x00007FFF)) | ((uint16_t)csptr << 0));
}


#endif // _REG_COMMON_EM_ET_H_

