#ifndef _REG_BLE_EM_TX_DESC_H_
#define _REG_BLE_EM_TX_DESC_H_

#include <stdint.h>
#include "_reg_ble_em_tx_desc.h"
#include "compiler.h"
#include "arch.h"
#include "em_map.h"
#include "reg_access.h"

#define REG_BLE_EM_TX_DESC_COUNT 3

#define REG_BLE_EM_TX_DESC_DECODING_MASK 0x00000007

#define REG_BLE_EM_TX_DESC_ADDR_GET(idx) (EM_BLE_TX_DESC_OFFSET + (idx) * REG_BLE_EM_TX_DESC_SIZE)

/**
 * @brief TXCNTL register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *     15               TXDONE   0
 *  14:00              NEXTPTR   0x0
 * </pre>
 */
#define BLE_TXCNTL_ADDR   (0x00 + (_ble_base) + EM_BLE_TX_DESC_OFFSET)
#define BLE_TXCNTL_INDEX  0x00000000
#define BLE_TXCNTL_RESET  0x00000000

__STATIC_INLINE uint16_t ble_txcntl_get(int elt_idx)
{
    return EM_BLE_RD(BLE_TXCNTL_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE);
}

__STATIC_INLINE void ble_txcntl_set(int elt_idx, uint16_t value)
{
    EM_BLE_WR(BLE_TXCNTL_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE, value);
}

// field definitions
#define BLE_TXDONE_BIT     ((uint16_t)0x00008000)
#define BLE_TXDONE_POS     15
#define BLE_NEXTPTR_MASK   ((uint16_t)0x00007FFF)
#define BLE_NEXTPTR_LSB    0
#define BLE_NEXTPTR_WIDTH  ((uint16_t)0x0000000F)

#define BLE_TXDONE_RST     0x0
#define BLE_NEXTPTR_RST    0x0

__STATIC_INLINE void ble_txcntl_pack(int elt_idx, uint8_t txdone, uint16_t nextptr)
{
    ASSERT_ERR((((uint16_t)txdone << 15) & ~((uint16_t)0x00008000)) == 0);
    ASSERT_ERR((((uint16_t)nextptr << 0) & ~((uint16_t)0x00007FFF)) == 0);
    EM_BLE_WR(BLE_TXCNTL_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE,  ((uint16_t)txdone << 15) | ((uint16_t)nextptr << 0));
}

__STATIC_INLINE void ble_txcntl_unpack(int elt_idx, uint8_t* txdone, uint16_t* nextptr)
{
    uint16_t localVal = EM_BLE_RD(BLE_TXCNTL_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE);

    *txdone = (localVal & ((uint16_t)0x00008000)) >> 15;
    *nextptr = (localVal & ((uint16_t)0x00007FFF)) >> 0;
}

__STATIC_INLINE uint8_t ble_txdone_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_TXCNTL_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE);
    return ((localVal & ((uint16_t)0x00008000)) >> 15);
}

__STATIC_INLINE void ble_txdone_setf(int elt_idx, uint8_t txdone)
{
    ASSERT_ERR((((uint16_t)txdone << 15) & ~((uint16_t)0x00008000)) == 0);
    EM_BLE_WR(BLE_TXCNTL_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE, (EM_BLE_RD(BLE_TXCNTL_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE) & ~((uint16_t)0x00008000)) | ((uint16_t)txdone << 15));
}

__STATIC_INLINE uint16_t ble_nextptr_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_TXCNTL_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE);
    return ((localVal & ((uint16_t)0x00007FFF)) >> 0);
}

__STATIC_INLINE void ble_nextptr_setf(int elt_idx, uint16_t nextptr)
{
    ASSERT_ERR((((uint16_t)nextptr << 0) & ~((uint16_t)0x00007FFF)) == 0);
    EM_BLE_WR(BLE_TXCNTL_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE, (EM_BLE_RD(BLE_TXCNTL_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE) & ~((uint16_t)0x00007FFF)) | ((uint16_t)nextptr << 0));
}

/**
 * @brief TXPHCE register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  15:08                TXLEN   0x0
 *     04                 TXMD   0
 *     03                 TXSN   0
 *     02               TXNESN   0
 *  01:00               TXLLID   0x0
 * </pre>
 */
#define BLE_TXPHCE_ADDR   (0x02 + (_ble_base) + EM_BLE_TX_DESC_OFFSET)
#define BLE_TXPHCE_INDEX  0x00000001
#define BLE_TXPHCE_RESET  0x00000000

__STATIC_INLINE uint16_t ble_txphce_get(int elt_idx)
{
    return EM_BLE_RD(BLE_TXPHCE_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE);
}

__STATIC_INLINE void ble_txphce_set(int elt_idx, uint16_t value)
{
    EM_BLE_WR(BLE_TXPHCE_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE, value);
}

// field definitions
#define BLE_TXLEN_MASK    ((uint16_t)0x0000FF00)
#define BLE_TXLEN_LSB     8
#define BLE_TXLEN_WIDTH   ((uint16_t)0x00000008)
#define BLE_TXMD_BIT      ((uint16_t)0x00000010)
#define BLE_TXMD_POS      4
#define BLE_TXSN_BIT      ((uint16_t)0x00000008)
#define BLE_TXSN_POS      3
#define BLE_TXNESN_BIT    ((uint16_t)0x00000004)
#define BLE_TXNESN_POS    2
#define BLE_TXLLID_MASK   ((uint16_t)0x00000003)
#define BLE_TXLLID_LSB    0
#define BLE_TXLLID_WIDTH  ((uint16_t)0x00000002)

#define BLE_TXLEN_RST     0x0
#define BLE_TXMD_RST      0x0
#define BLE_TXSN_RST      0x0
#define BLE_TXNESN_RST    0x0
#define BLE_TXLLID_RST    0x0

__STATIC_INLINE void ble_txphce_pack(int elt_idx, uint8_t txlen, uint8_t txmd, uint8_t txsn, uint8_t txnesn, uint8_t txllid)
{
    ASSERT_ERR((((uint16_t)txlen << 8) & ~((uint16_t)0x0000FF00)) == 0);
    ASSERT_ERR((((uint16_t)txmd << 4) & ~((uint16_t)0x00000010)) == 0);
    ASSERT_ERR((((uint16_t)txsn << 3) & ~((uint16_t)0x00000008)) == 0);
    ASSERT_ERR((((uint16_t)txnesn << 2) & ~((uint16_t)0x00000004)) == 0);
    ASSERT_ERR((((uint16_t)txllid << 0) & ~((uint16_t)0x00000003)) == 0);
    EM_BLE_WR(BLE_TXPHCE_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE,  ((uint16_t)txlen << 8) | ((uint16_t)txmd << 4) | ((uint16_t)txsn << 3) | ((uint16_t)txnesn << 2) | ((uint16_t)txllid << 0));
}

__STATIC_INLINE void ble_txphce_unpack(int elt_idx, uint8_t* txlen, uint8_t* txmd, uint8_t* txsn, uint8_t* txnesn, uint8_t* txllid)
{
    uint16_t localVal = EM_BLE_RD(BLE_TXPHCE_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE);

    *txlen = (localVal & ((uint16_t)0x0000FF00)) >> 8;
    *txmd = (localVal & ((uint16_t)0x00000010)) >> 4;
    *txsn = (localVal & ((uint16_t)0x00000008)) >> 3;
    *txnesn = (localVal & ((uint16_t)0x00000004)) >> 2;
    *txllid = (localVal & ((uint16_t)0x00000003)) >> 0;
}

__STATIC_INLINE uint8_t ble_txlen_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_TXPHCE_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE);
    return ((localVal & ((uint16_t)0x0000FF00)) >> 8);
}

__STATIC_INLINE void ble_txlen_setf(int elt_idx, uint8_t txlen)
{
    ASSERT_ERR((((uint16_t)txlen << 8) & ~((uint16_t)0x0000FF00)) == 0);
    EM_BLE_WR(BLE_TXPHCE_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE, (EM_BLE_RD(BLE_TXPHCE_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE) & ~((uint16_t)0x0000FF00)) | ((uint16_t)txlen << 8));
}

__STATIC_INLINE uint8_t ble_txmd_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_TXPHCE_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE);
    return ((localVal & ((uint16_t)0x00000010)) >> 4);
}

__STATIC_INLINE void ble_txmd_setf(int elt_idx, uint8_t txmd)
{
    ASSERT_ERR((((uint16_t)txmd << 4) & ~((uint16_t)0x00000010)) == 0);
    EM_BLE_WR(BLE_TXPHCE_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE, (EM_BLE_RD(BLE_TXPHCE_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE) & ~((uint16_t)0x00000010)) | ((uint16_t)txmd << 4));
}

__STATIC_INLINE uint8_t ble_txsn_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_TXPHCE_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE);
    return ((localVal & ((uint16_t)0x00000008)) >> 3);
}

__STATIC_INLINE void ble_txsn_setf(int elt_idx, uint8_t txsn)
{
    ASSERT_ERR((((uint16_t)txsn << 3) & ~((uint16_t)0x00000008)) == 0);
    EM_BLE_WR(BLE_TXPHCE_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE, (EM_BLE_RD(BLE_TXPHCE_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE) & ~((uint16_t)0x00000008)) | ((uint16_t)txsn << 3));
}

__STATIC_INLINE uint8_t ble_txnesn_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_TXPHCE_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE);
    return ((localVal & ((uint16_t)0x00000004)) >> 2);
}

__STATIC_INLINE void ble_txnesn_setf(int elt_idx, uint8_t txnesn)
{
    ASSERT_ERR((((uint16_t)txnesn << 2) & ~((uint16_t)0x00000004)) == 0);
    EM_BLE_WR(BLE_TXPHCE_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE, (EM_BLE_RD(BLE_TXPHCE_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE) & ~((uint16_t)0x00000004)) | ((uint16_t)txnesn << 2));
}

__STATIC_INLINE uint8_t ble_txllid_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_TXPHCE_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE);
    return ((localVal & ((uint16_t)0x00000003)) >> 0);
}

__STATIC_INLINE void ble_txllid_setf(int elt_idx, uint8_t txllid)
{
    ASSERT_ERR((((uint16_t)txllid << 0) & ~((uint16_t)0x00000003)) == 0);
    EM_BLE_WR(BLE_TXPHCE_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE, (EM_BLE_RD(BLE_TXPHCE_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE) & ~((uint16_t)0x00000003)) | ((uint16_t)txllid << 0));
}

/**
 * @brief TXPHADV register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  15:08             TXADVLEN   0x0
 *     07              TXRXADD   0
 *     06              TXTXADD   0
 *  03:00               TXTYPE   0x0
 * </pre>
 */
#define BLE_TXPHADV_ADDR   (0x02 + (_ble_base) + EM_BLE_TX_DESC_OFFSET)
#define BLE_TXPHADV_INDEX  0x00000001
#define BLE_TXPHADV_RESET  0x00000000

__STATIC_INLINE uint16_t ble_txphadv_get(int elt_idx)
{
    return EM_BLE_RD(BLE_TXPHADV_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE);
}

__STATIC_INLINE void ble_txphadv_set(int elt_idx, uint16_t value)
{
    EM_BLE_WR(BLE_TXPHADV_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE, value);
}

// field definitions
#define BLE_TXADVLEN_MASK   ((uint16_t)0x0000FF00)
#define BLE_TXADVLEN_LSB    8
#define BLE_TXADVLEN_WIDTH  ((uint16_t)0x00000008)
#define BLE_TXRXADD_BIT     ((uint16_t)0x00000080)
#define BLE_TXRXADD_POS     7
#define BLE_TXTXADD_BIT     ((uint16_t)0x00000040)
#define BLE_TXTXADD_POS     6
#define BLE_TXTYPE_MASK     ((uint16_t)0x0000000F)
#define BLE_TXTYPE_LSB      0
#define BLE_TXTYPE_WIDTH    ((uint16_t)0x00000004)

#define BLE_TXADVLEN_RST    0x0
#define BLE_TXRXADD_RST     0x0
#define BLE_TXTXADD_RST     0x0
#define BLE_TXTYPE_RST      0x0

__STATIC_INLINE void ble_txphadv_pack(int elt_idx, uint8_t txadvlen, uint8_t txrxadd, uint8_t txtxadd, uint8_t txtype)
{
    ASSERT_ERR((((uint16_t)txadvlen << 8) & ~((uint16_t)0x0000FF00)) == 0);
    ASSERT_ERR((((uint16_t)txrxadd << 7) & ~((uint16_t)0x00000080)) == 0);
    ASSERT_ERR((((uint16_t)txtxadd << 6) & ~((uint16_t)0x00000040)) == 0);
    ASSERT_ERR((((uint16_t)txtype << 0) & ~((uint16_t)0x0000000F)) == 0);
    EM_BLE_WR(BLE_TXPHADV_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE,  ((uint16_t)txadvlen << 8) | ((uint16_t)txrxadd << 7) | ((uint16_t)txtxadd << 6) | ((uint16_t)txtype << 0));
}

__STATIC_INLINE void ble_txphadv_unpack(int elt_idx, uint8_t* txadvlen, uint8_t* txrxadd, uint8_t* txtxadd, uint8_t* txtype)
{
    uint16_t localVal = EM_BLE_RD(BLE_TXPHADV_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE);

    *txadvlen = (localVal & ((uint16_t)0x0000FF00)) >> 8;
    *txrxadd = (localVal & ((uint16_t)0x00000080)) >> 7;
    *txtxadd = (localVal & ((uint16_t)0x00000040)) >> 6;
    *txtype = (localVal & ((uint16_t)0x0000000F)) >> 0;
}

__STATIC_INLINE uint8_t ble_txadvlen_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_TXPHADV_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE);
    return ((localVal & ((uint16_t)0x0000FF00)) >> 8);
}

__STATIC_INLINE void ble_txadvlen_setf(int elt_idx, uint8_t txadvlen)
{
    ASSERT_ERR((((uint16_t)txadvlen << 8) & ~((uint16_t)0x0000FF00)) == 0);
    EM_BLE_WR(BLE_TXPHADV_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE, (EM_BLE_RD(BLE_TXPHADV_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE) & ~((uint16_t)0x0000FF00)) | ((uint16_t)txadvlen << 8));
}

__STATIC_INLINE uint8_t ble_txrxadd_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_TXPHADV_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE);
    return ((localVal & ((uint16_t)0x00000080)) >> 7);
}

__STATIC_INLINE void ble_txrxadd_setf(int elt_idx, uint8_t txrxadd)
{
    ASSERT_ERR((((uint16_t)txrxadd << 7) & ~((uint16_t)0x00000080)) == 0);
    EM_BLE_WR(BLE_TXPHADV_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE, (EM_BLE_RD(BLE_TXPHADV_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE) & ~((uint16_t)0x00000080)) | ((uint16_t)txrxadd << 7));
}

__STATIC_INLINE uint8_t ble_txtxadd_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_TXPHADV_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE);
    return ((localVal & ((uint16_t)0x00000040)) >> 6);
}

__STATIC_INLINE void ble_txtxadd_setf(int elt_idx, uint8_t txtxadd)
{
    ASSERT_ERR((((uint16_t)txtxadd << 6) & ~((uint16_t)0x00000040)) == 0);
    EM_BLE_WR(BLE_TXPHADV_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE, (EM_BLE_RD(BLE_TXPHADV_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE) & ~((uint16_t)0x00000040)) | ((uint16_t)txtxadd << 6));
}

__STATIC_INLINE uint8_t ble_txtype_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_TXPHADV_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE);
    return ((localVal & ((uint16_t)0x0000000F)) >> 0);
}

__STATIC_INLINE void ble_txtype_setf(int elt_idx, uint8_t txtype)
{
    ASSERT_ERR((((uint16_t)txtype << 0) & ~((uint16_t)0x0000000F)) == 0);
    EM_BLE_WR(BLE_TXPHADV_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE, (EM_BLE_RD(BLE_TXPHADV_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE) & ~((uint16_t)0x0000000F)) | ((uint16_t)txtype << 0));
}

/**
 * @brief TXDATAPTR register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  15:00            TXDATAPTR   0x0
 * </pre>
 */
#define BLE_TXDATAPTR_ADDR   (0x04 + (_ble_base) + EM_BLE_TX_DESC_OFFSET)
#define BLE_TXDATAPTR_INDEX  0x00000002
#define BLE_TXDATAPTR_RESET  0x00000000

__STATIC_INLINE uint16_t ble_txdataptr_get(int elt_idx)
{
    return EM_BLE_RD(BLE_TXDATAPTR_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE);
}

__STATIC_INLINE void ble_txdataptr_set(int elt_idx, uint16_t value)
{
    EM_BLE_WR(BLE_TXDATAPTR_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE, value);
}

// field definitions
#define BLE_TXDATAPTR_MASK   ((uint16_t)0x0000FFFF)
#define BLE_TXDATAPTR_LSB    0
#define BLE_TXDATAPTR_WIDTH  ((uint16_t)0x00000010)

#define BLE_TXDATAPTR_RST    0x0

__STATIC_INLINE uint16_t ble_txdataptr_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_TXDATAPTR_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE);
    ASSERT_ERR((localVal & ~((uint16_t)0x0000FFFF)) == 0);
    return (localVal >> 0);
}

__STATIC_INLINE void ble_txdataptr_setf(int elt_idx, uint16_t txdataptr)
{
    ASSERT_ERR((((uint16_t)txdataptr << 0) & ~((uint16_t)0x0000FFFF)) == 0);
    EM_BLE_WR(BLE_TXDATAPTR_ADDR + elt_idx * REG_BLE_EM_TX_DESC_SIZE, (uint16_t)txdataptr << 0);
}


#endif // _REG_BLE_EM_TX_DESC_H_

