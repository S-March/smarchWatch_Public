#ifndef _REG_BLE_EM_RX_DESC_H_
#define _REG_BLE_EM_RX_DESC_H_

#include <stdint.h>
#include "_reg_ble_em_rx_desc.h"
#include "compiler.h"
#include "arch.h"
#include "em_map.h"
#include "reg_access.h"

#define REG_BLE_EM_RX_DESC_COUNT 5

#define REG_BLE_EM_RX_DESC_DECODING_MASK 0x0000000F

#define REG_BLE_EM_RX_DESC_ADDR_GET(idx) (EM_BLE_RX_DESC_OFFSET + (idx) * REG_BLE_EM_RX_DESC_SIZE)

/**
 * @brief RXCNTL register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *     15               RXDONE   0
 *  14:00            RXNEXTPTR   0x0
 * </pre>
 */
#define BLE_RXCNTL_ADDR   (0x00 + (_ble_base) + EM_BLE_RX_DESC_OFFSET)
#define BLE_RXCNTL_INDEX  0x00000000
#define BLE_RXCNTL_RESET  0x00000000

__STATIC_INLINE uint16_t ble_rxcntl_get(int elt_idx)
{
    return EM_BLE_RD(BLE_RXCNTL_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);
}

__STATIC_INLINE void ble_rxcntl_set(int elt_idx, uint16_t value)
{
    EM_BLE_WR(BLE_RXCNTL_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE, value);
}

// field definitions
#define BLE_RXDONE_BIT       ((uint16_t)0x00008000)
#define BLE_RXDONE_POS       15
#define BLE_RXNEXTPTR_MASK   ((uint16_t)0x00007FFF)
#define BLE_RXNEXTPTR_LSB    0
#define BLE_RXNEXTPTR_WIDTH  ((uint16_t)0x0000000F)

#define BLE_RXDONE_RST       0x0
#define BLE_RXNEXTPTR_RST    0x0

__STATIC_INLINE void ble_rxcntl_pack(int elt_idx, uint8_t rxdone, uint16_t rxnextptr)
{
    ASSERT_ERR((((uint16_t)rxdone << 15) & ~((uint16_t)0x00008000)) == 0);
    ASSERT_ERR((((uint16_t)rxnextptr << 0) & ~((uint16_t)0x00007FFF)) == 0);
    EM_BLE_WR(BLE_RXCNTL_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE,  ((uint16_t)rxdone << 15) | ((uint16_t)rxnextptr << 0));
}

__STATIC_INLINE void ble_rxcntl_unpack(int elt_idx, uint8_t* rxdone, uint16_t* rxnextptr)
{
    uint16_t localVal = EM_BLE_RD(BLE_RXCNTL_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);

    *rxdone = (localVal & ((uint16_t)0x00008000)) >> 15;
    *rxnextptr = (localVal & ((uint16_t)0x00007FFF)) >> 0;
}

__STATIC_INLINE uint8_t ble_rxdone_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_RXCNTL_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);
    return ((localVal & ((uint16_t)0x00008000)) >> 15);
}

__STATIC_INLINE void ble_rxdone_setf(int elt_idx, uint8_t rxdone)
{
    ASSERT_ERR((((uint16_t)rxdone << 15) & ~((uint16_t)0x00008000)) == 0);
    EM_BLE_WR(BLE_RXCNTL_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE, (EM_BLE_RD(BLE_RXCNTL_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE) & ~((uint16_t)0x00008000)) | ((uint16_t)rxdone << 15));
}

__STATIC_INLINE uint16_t ble_rxnextptr_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_RXCNTL_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);
    return ((localVal & ((uint16_t)0x00007FFF)) >> 0);
}

__STATIC_INLINE void ble_rxnextptr_setf(int elt_idx, uint16_t rxnextptr)
{
    ASSERT_ERR((((uint16_t)rxnextptr << 0) & ~((uint16_t)0x00007FFF)) == 0);
    EM_BLE_WR(BLE_RXCNTL_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE, (EM_BLE_RD(BLE_RXCNTL_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE) & ~((uint16_t)0x00007FFF)) | ((uint16_t)rxnextptr << 0));
}

/**
 * @brief RXSTAT register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  15:11            RXLINKLBL   0x0
 *     08            RXTIMEERR   0
 *     07         BDADDR_MATCH   0
 *     06             NESN_ERR   0
 *     05               SN_ERR   0
 *     04              MIC_ERR   0
 *     03              CRC_ERR   0
 *     02              LEN_ERR   0
 *     01             TYPE_ERR   0
 *     00             SYNC_ERR   0
 * </pre>
 */
#define BLE_RXSTAT_ADDR   (0x02 + (_ble_base) + EM_BLE_RX_DESC_OFFSET)
#define BLE_RXSTAT_INDEX  0x00000001
#define BLE_RXSTAT_RESET  0x00000000

__STATIC_INLINE uint16_t ble_rxstat_get(int elt_idx)
{
    return EM_BLE_RD(BLE_RXSTAT_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);
}

__STATIC_INLINE void ble_rxstat_set(int elt_idx, uint16_t value)
{
    EM_BLE_WR(BLE_RXSTAT_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE, value);
}

// field definitions
#define BLE_RXLINKLBL_MASK      ((uint16_t)0x0000F800)
#define BLE_RXLINKLBL_LSB       11
#define BLE_RXLINKLBL_WIDTH     ((uint16_t)0x00000005)
#define BLE_RXTIMEERR_BIT       ((uint16_t)0x00000100)
#define BLE_RXTIMEERR_POS       8
#define BLE_BDADDR_MATCH_BIT    ((uint16_t)0x00000080)
#define BLE_BDADDR_MATCH_POS    7
#define BLE_NESN_ERR_BIT        ((uint16_t)0x00000040)
#define BLE_NESN_ERR_POS        6
#define BLE_SN_ERR_BIT          ((uint16_t)0x00000020)
#define BLE_SN_ERR_POS          5
#define BLE_MIC_ERR_BIT         ((uint16_t)0x00000010)
#define BLE_MIC_ERR_POS         4
#define BLE_CRC_ERR_BIT         ((uint16_t)0x00000008)
#define BLE_CRC_ERR_POS         3
#define BLE_LEN_ERR_BIT         ((uint16_t)0x00000004)
#define BLE_LEN_ERR_POS         2
#define BLE_TYPE_ERR_BIT        ((uint16_t)0x00000002)
#define BLE_TYPE_ERR_POS        1
#define BLE_SYNC_ERR_BIT        ((uint16_t)0x00000001)
#define BLE_SYNC_ERR_POS        0

#define BLE_RXLINKLBL_RST       0x0
#define BLE_RXTIMEERR_RST       0x0
#define BLE_BDADDR_MATCH_RST    0x0
#define BLE_NESN_ERR_RST        0x0
#define BLE_SN_ERR_RST          0x0
#define BLE_MIC_ERR_RST         0x0
#define BLE_CRC_ERR_RST         0x0
#define BLE_LEN_ERR_RST         0x0
#define BLE_TYPE_ERR_RST        0x0
#define BLE_SYNC_ERR_RST        0x0

__STATIC_INLINE void ble_rxstat_pack(int elt_idx, uint8_t rxlinklbl, uint8_t rxtimeerr, uint8_t bdaddrmatch, uint8_t nesnerr, uint8_t snerr, uint8_t micerr, uint8_t crcerr, uint8_t lenerr, uint8_t typeerr, uint8_t syncerr)
{
    ASSERT_ERR((((uint16_t)rxlinklbl << 11) & ~((uint16_t)0x0000F800)) == 0);
    ASSERT_ERR((((uint16_t)rxtimeerr << 8) & ~((uint16_t)0x00000100)) == 0);
    ASSERT_ERR((((uint16_t)bdaddrmatch << 7) & ~((uint16_t)0x00000080)) == 0);
    ASSERT_ERR((((uint16_t)nesnerr << 6) & ~((uint16_t)0x00000040)) == 0);
    ASSERT_ERR((((uint16_t)snerr << 5) & ~((uint16_t)0x00000020)) == 0);
    ASSERT_ERR((((uint16_t)micerr << 4) & ~((uint16_t)0x00000010)) == 0);
    ASSERT_ERR((((uint16_t)crcerr << 3) & ~((uint16_t)0x00000008)) == 0);
    ASSERT_ERR((((uint16_t)lenerr << 2) & ~((uint16_t)0x00000004)) == 0);
    ASSERT_ERR((((uint16_t)typeerr << 1) & ~((uint16_t)0x00000002)) == 0);
    ASSERT_ERR((((uint16_t)syncerr << 0) & ~((uint16_t)0x00000001)) == 0);
    EM_BLE_WR(BLE_RXSTAT_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE,  ((uint16_t)rxlinklbl << 11) | ((uint16_t)rxtimeerr << 8) | ((uint16_t)bdaddrmatch << 7) | ((uint16_t)nesnerr << 6) | ((uint16_t)snerr << 5) | ((uint16_t)micerr << 4) | ((uint16_t)crcerr << 3) | ((uint16_t)lenerr << 2) | ((uint16_t)typeerr << 1) | ((uint16_t)syncerr << 0));
}

__STATIC_INLINE void ble_rxstat_unpack(int elt_idx, uint8_t* rxlinklbl, uint8_t* rxtimeerr, uint8_t* bdaddrmatch, uint8_t* nesnerr, uint8_t* snerr, uint8_t* micerr, uint8_t* crcerr, uint8_t* lenerr, uint8_t* typeerr, uint8_t* syncerr)
{
    uint16_t localVal = EM_BLE_RD(BLE_RXSTAT_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);

    *rxlinklbl = (localVal & ((uint16_t)0x0000F800)) >> 11;
    *rxtimeerr = (localVal & ((uint16_t)0x00000100)) >> 8;
    *bdaddrmatch = (localVal & ((uint16_t)0x00000080)) >> 7;
    *nesnerr = (localVal & ((uint16_t)0x00000040)) >> 6;
    *snerr = (localVal & ((uint16_t)0x00000020)) >> 5;
    *micerr = (localVal & ((uint16_t)0x00000010)) >> 4;
    *crcerr = (localVal & ((uint16_t)0x00000008)) >> 3;
    *lenerr = (localVal & ((uint16_t)0x00000004)) >> 2;
    *typeerr = (localVal & ((uint16_t)0x00000002)) >> 1;
    *syncerr = (localVal & ((uint16_t)0x00000001)) >> 0;
}

__STATIC_INLINE uint8_t ble_rxlinklbl_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_RXSTAT_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);
    return ((localVal & ((uint16_t)0x0000F800)) >> 11);
}

__STATIC_INLINE void ble_rxlinklbl_setf(int elt_idx, uint8_t rxlinklbl)
{
    ASSERT_ERR((((uint16_t)rxlinklbl << 11) & ~((uint16_t)0x0000F800)) == 0);
    EM_BLE_WR(BLE_RXSTAT_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE, (EM_BLE_RD(BLE_RXSTAT_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE) & ~((uint16_t)0x0000F800)) | ((uint16_t)rxlinklbl << 11));
}

__STATIC_INLINE uint8_t ble_rxtimeerr_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_RXSTAT_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);
    return ((localVal & ((uint16_t)0x00000100)) >> 8);
}

__STATIC_INLINE void ble_rxtimeerr_setf(int elt_idx, uint8_t rxtimeerr)
{
    ASSERT_ERR((((uint16_t)rxtimeerr << 8) & ~((uint16_t)0x00000100)) == 0);
    EM_BLE_WR(BLE_RXSTAT_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE, (EM_BLE_RD(BLE_RXSTAT_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE) & ~((uint16_t)0x00000100)) | ((uint16_t)rxtimeerr << 8));
}

__STATIC_INLINE uint8_t ble_bdaddr_match_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_RXSTAT_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);
    return ((localVal & ((uint16_t)0x00000080)) >> 7);
}

__STATIC_INLINE void ble_bdaddr_match_setf(int elt_idx, uint8_t bdaddrmatch)
{
    ASSERT_ERR((((uint16_t)bdaddrmatch << 7) & ~((uint16_t)0x00000080)) == 0);
    EM_BLE_WR(BLE_RXSTAT_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE, (EM_BLE_RD(BLE_RXSTAT_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE) & ~((uint16_t)0x00000080)) | ((uint16_t)bdaddrmatch << 7));
}

__STATIC_INLINE uint8_t ble_nesn_err_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_RXSTAT_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);
    return ((localVal & ((uint16_t)0x00000040)) >> 6);
}

__STATIC_INLINE void ble_nesn_err_setf(int elt_idx, uint8_t nesnerr)
{
    ASSERT_ERR((((uint16_t)nesnerr << 6) & ~((uint16_t)0x00000040)) == 0);
    EM_BLE_WR(BLE_RXSTAT_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE, (EM_BLE_RD(BLE_RXSTAT_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE) & ~((uint16_t)0x00000040)) | ((uint16_t)nesnerr << 6));
}

__STATIC_INLINE uint8_t ble_sn_err_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_RXSTAT_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);
    return ((localVal & ((uint16_t)0x00000020)) >> 5);
}

__STATIC_INLINE void ble_sn_err_setf(int elt_idx, uint8_t snerr)
{
    ASSERT_ERR((((uint16_t)snerr << 5) & ~((uint16_t)0x00000020)) == 0);
    EM_BLE_WR(BLE_RXSTAT_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE, (EM_BLE_RD(BLE_RXSTAT_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE) & ~((uint16_t)0x00000020)) | ((uint16_t)snerr << 5));
}

__STATIC_INLINE uint8_t ble_mic_err_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_RXSTAT_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);
    return ((localVal & ((uint16_t)0x00000010)) >> 4);
}

__STATIC_INLINE void ble_mic_err_setf(int elt_idx, uint8_t micerr)
{
    ASSERT_ERR((((uint16_t)micerr << 4) & ~((uint16_t)0x00000010)) == 0);
    EM_BLE_WR(BLE_RXSTAT_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE, (EM_BLE_RD(BLE_RXSTAT_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE) & ~((uint16_t)0x00000010)) | ((uint16_t)micerr << 4));
}

__STATIC_INLINE uint8_t ble_crc_err_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_RXSTAT_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);
    return ((localVal & ((uint16_t)0x00000008)) >> 3);
}

__STATIC_INLINE void ble_crc_err_setf(int elt_idx, uint8_t crcerr)
{
    ASSERT_ERR((((uint16_t)crcerr << 3) & ~((uint16_t)0x00000008)) == 0);
    EM_BLE_WR(BLE_RXSTAT_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE, (EM_BLE_RD(BLE_RXSTAT_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE) & ~((uint16_t)0x00000008)) | ((uint16_t)crcerr << 3));
}

__STATIC_INLINE uint8_t ble_len_err_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_RXSTAT_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);
    return ((localVal & ((uint16_t)0x00000004)) >> 2);
}

__STATIC_INLINE void ble_len_err_setf(int elt_idx, uint8_t lenerr)
{
    ASSERT_ERR((((uint16_t)lenerr << 2) & ~((uint16_t)0x00000004)) == 0);
    EM_BLE_WR(BLE_RXSTAT_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE, (EM_BLE_RD(BLE_RXSTAT_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE) & ~((uint16_t)0x00000004)) | ((uint16_t)lenerr << 2));
}

__STATIC_INLINE uint8_t ble_type_err_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_RXSTAT_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);
    return ((localVal & ((uint16_t)0x00000002)) >> 1);
}

__STATIC_INLINE void ble_type_err_setf(int elt_idx, uint8_t typeerr)
{
    ASSERT_ERR((((uint16_t)typeerr << 1) & ~((uint16_t)0x00000002)) == 0);
    EM_BLE_WR(BLE_RXSTAT_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE, (EM_BLE_RD(BLE_RXSTAT_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE) & ~((uint16_t)0x00000002)) | ((uint16_t)typeerr << 1));
}

__STATIC_INLINE uint8_t ble_sync_err_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_RXSTAT_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);
    return ((localVal & ((uint16_t)0x00000001)) >> 0);
}

__STATIC_INLINE void ble_sync_err_setf(int elt_idx, uint8_t syncerr)
{
    ASSERT_ERR((((uint16_t)syncerr << 0) & ~((uint16_t)0x00000001)) == 0);
    EM_BLE_WR(BLE_RXSTAT_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE, (EM_BLE_RD(BLE_RXSTAT_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE) & ~((uint16_t)0x00000001)) | ((uint16_t)syncerr << 0));
}

/**
 * @brief RXPHCE register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  15:08                RXLEN   0x0
 *     04                 RXMD   0
 *     03                 RXSN   0
 *     02               RXNESN   0
 *  01:00               RXLLID   0x0
 * </pre>
 */
#define BLE_RXPHCE_ADDR   (0x04 + (_ble_base) + EM_BLE_RX_DESC_OFFSET)
#define BLE_RXPHCE_INDEX  0x00000002
#define BLE_RXPHCE_RESET  0x00000000

__STATIC_INLINE uint16_t ble_rxphce_get(int elt_idx)
{
    return EM_BLE_RD(BLE_RXPHCE_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);
}

__STATIC_INLINE void ble_rxphce_set(int elt_idx, uint16_t value)
{
    EM_BLE_WR(BLE_RXPHCE_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE, value);
}

// field definitions
#define BLE_RXLEN_MASK    ((uint16_t)0x0000FF00)
#define BLE_RXLEN_LSB     8
#define BLE_RXLEN_WIDTH   ((uint16_t)0x00000008)
#define BLE_RXMD_BIT      ((uint16_t)0x00000010)
#define BLE_RXMD_POS      4
#define BLE_RXSN_BIT      ((uint16_t)0x00000008)
#define BLE_RXSN_POS      3
#define BLE_RXNESN_BIT    ((uint16_t)0x00000004)
#define BLE_RXNESN_POS    2
#define BLE_RXLLID_MASK   ((uint16_t)0x00000003)
#define BLE_RXLLID_LSB    0
#define BLE_RXLLID_WIDTH  ((uint16_t)0x00000002)

#define BLE_RXLEN_RST     0x0
#define BLE_RXMD_RST      0x0
#define BLE_RXSN_RST      0x0
#define BLE_RXNESN_RST    0x0
#define BLE_RXLLID_RST    0x0

__STATIC_INLINE void ble_rxphce_pack(int elt_idx, uint8_t rxlen, uint8_t rxmd, uint8_t rxsn, uint8_t rxnesn, uint8_t rxllid)
{
    ASSERT_ERR((((uint16_t)rxlen << 8) & ~((uint16_t)0x0000FF00)) == 0);
    ASSERT_ERR((((uint16_t)rxmd << 4) & ~((uint16_t)0x00000010)) == 0);
    ASSERT_ERR((((uint16_t)rxsn << 3) & ~((uint16_t)0x00000008)) == 0);
    ASSERT_ERR((((uint16_t)rxnesn << 2) & ~((uint16_t)0x00000004)) == 0);
    ASSERT_ERR((((uint16_t)rxllid << 0) & ~((uint16_t)0x00000003)) == 0);
    EM_BLE_WR(BLE_RXPHCE_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE,  ((uint16_t)rxlen << 8) | ((uint16_t)rxmd << 4) | ((uint16_t)rxsn << 3) | ((uint16_t)rxnesn << 2) | ((uint16_t)rxllid << 0));
}

__STATIC_INLINE void ble_rxphce_unpack(int elt_idx, uint8_t* rxlen, uint8_t* rxmd, uint8_t* rxsn, uint8_t* rxnesn, uint8_t* rxllid)
{
    uint16_t localVal = EM_BLE_RD(BLE_RXPHCE_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);

    *rxlen = (localVal & ((uint16_t)0x0000FF00)) >> 8;
    *rxmd = (localVal & ((uint16_t)0x00000010)) >> 4;
    *rxsn = (localVal & ((uint16_t)0x00000008)) >> 3;
    *rxnesn = (localVal & ((uint16_t)0x00000004)) >> 2;
    *rxllid = (localVal & ((uint16_t)0x00000003)) >> 0;
}

__STATIC_INLINE uint8_t ble_rxlen_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_RXPHCE_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);
    return ((localVal & ((uint16_t)0x0000FF00)) >> 8);
}

__STATIC_INLINE void ble_rxlen_setf(int elt_idx, uint8_t rxlen)
{
    ASSERT_ERR((((uint16_t)rxlen << 8) & ~((uint16_t)0x0000FF00)) == 0);
    EM_BLE_WR(BLE_RXPHCE_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE, (EM_BLE_RD(BLE_RXPHCE_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE) & ~((uint16_t)0x0000FF00)) | ((uint16_t)rxlen << 8));
}

__STATIC_INLINE uint8_t ble_rxmd_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_RXPHCE_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);
    return ((localVal & ((uint16_t)0x00000010)) >> 4);
}

__STATIC_INLINE void ble_rxmd_setf(int elt_idx, uint8_t rxmd)
{
    ASSERT_ERR((((uint16_t)rxmd << 4) & ~((uint16_t)0x00000010)) == 0);
    EM_BLE_WR(BLE_RXPHCE_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE, (EM_BLE_RD(BLE_RXPHCE_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE) & ~((uint16_t)0x00000010)) | ((uint16_t)rxmd << 4));
}

__STATIC_INLINE uint8_t ble_rxsn_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_RXPHCE_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);
    return ((localVal & ((uint16_t)0x00000008)) >> 3);
}

__STATIC_INLINE void ble_rxsn_setf(int elt_idx, uint8_t rxsn)
{
    ASSERT_ERR((((uint16_t)rxsn << 3) & ~((uint16_t)0x00000008)) == 0);
    EM_BLE_WR(BLE_RXPHCE_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE, (EM_BLE_RD(BLE_RXPHCE_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE) & ~((uint16_t)0x00000008)) | ((uint16_t)rxsn << 3));
}

__STATIC_INLINE uint8_t ble_rxnesn_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_RXPHCE_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);
    return ((localVal & ((uint16_t)0x00000004)) >> 2);
}

__STATIC_INLINE void ble_rxnesn_setf(int elt_idx, uint8_t rxnesn)
{
    ASSERT_ERR((((uint16_t)rxnesn << 2) & ~((uint16_t)0x00000004)) == 0);
    EM_BLE_WR(BLE_RXPHCE_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE, (EM_BLE_RD(BLE_RXPHCE_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE) & ~((uint16_t)0x00000004)) | ((uint16_t)rxnesn << 2));
}

__STATIC_INLINE uint8_t ble_rxllid_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_RXPHCE_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);
    return ((localVal & ((uint16_t)0x00000003)) >> 0);
}

__STATIC_INLINE void ble_rxllid_setf(int elt_idx, uint8_t rxllid)
{
    ASSERT_ERR((((uint16_t)rxllid << 0) & ~((uint16_t)0x00000003)) == 0);
    EM_BLE_WR(BLE_RXPHCE_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE, (EM_BLE_RD(BLE_RXPHCE_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE) & ~((uint16_t)0x00000003)) | ((uint16_t)rxllid << 0));
}

/**
 * @brief RXPHADV register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  15:08             RXADVLEN   0x0
 *     07              RXRXADD   0
 *     06              RXTXADD   0
 *  03:00               RXTYPE   0x0
 * </pre>
 */
#define BLE_RXPHADV_ADDR   (0x04 + (_ble_base) + EM_BLE_RX_DESC_OFFSET)
#define BLE_RXPHADV_INDEX  0x00000002
#define BLE_RXPHADV_RESET  0x00000000

__STATIC_INLINE uint16_t ble_rxphadv_get(int elt_idx)
{
    return EM_BLE_RD(BLE_RXPHADV_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);
}

__STATIC_INLINE void ble_rxphadv_set(int elt_idx, uint16_t value)
{
    EM_BLE_WR(BLE_RXPHADV_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE, value);
}

// field definitions
#define BLE_RXADVLEN_MASK   ((uint16_t)0x0000FF00)
#define BLE_RXADVLEN_LSB    8
#define BLE_RXADVLEN_WIDTH  ((uint16_t)0x00000008)
#define BLE_RXRXADD_BIT     ((uint16_t)0x00000080)
#define BLE_RXRXADD_POS     7
#define BLE_RXTXADD_BIT     ((uint16_t)0x00000040)
#define BLE_RXTXADD_POS     6
#define BLE_RXTYPE_MASK     ((uint16_t)0x0000000F)
#define BLE_RXTYPE_LSB      0
#define BLE_RXTYPE_WIDTH    ((uint16_t)0x00000004)

#define BLE_RXADVLEN_RST    0x0
#define BLE_RXRXADD_RST     0x0
#define BLE_RXTXADD_RST     0x0
#define BLE_RXTYPE_RST      0x0

__STATIC_INLINE void ble_rxphadv_pack(int elt_idx, uint8_t rxadvlen, uint8_t rxrxadd, uint8_t rxtxadd, uint8_t rxtype)
{
    ASSERT_ERR((((uint16_t)rxadvlen << 8) & ~((uint16_t)0x0000FF00)) == 0);
    ASSERT_ERR((((uint16_t)rxrxadd << 7) & ~((uint16_t)0x00000080)) == 0);
    ASSERT_ERR((((uint16_t)rxtxadd << 6) & ~((uint16_t)0x00000040)) == 0);
    ASSERT_ERR((((uint16_t)rxtype << 0) & ~((uint16_t)0x0000000F)) == 0);
    EM_BLE_WR(BLE_RXPHADV_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE,  ((uint16_t)rxadvlen << 8) | ((uint16_t)rxrxadd << 7) | ((uint16_t)rxtxadd << 6) | ((uint16_t)rxtype << 0));
}

__STATIC_INLINE void ble_rxphadv_unpack(int elt_idx, uint8_t* rxadvlen, uint8_t* rxrxadd, uint8_t* rxtxadd, uint8_t* rxtype)
{
    uint16_t localVal = EM_BLE_RD(BLE_RXPHADV_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);

    *rxadvlen = (localVal & ((uint16_t)0x0000FF00)) >> 8;
    *rxrxadd = (localVal & ((uint16_t)0x00000080)) >> 7;
    *rxtxadd = (localVal & ((uint16_t)0x00000040)) >> 6;
    *rxtype = (localVal & ((uint16_t)0x0000000F)) >> 0;
}

__STATIC_INLINE uint8_t ble_rxadvlen_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_RXPHADV_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);
    return ((localVal & ((uint16_t)0x0000FF00)) >> 8);
}

__STATIC_INLINE void ble_rxadvlen_setf(int elt_idx, uint8_t rxadvlen)
{
    ASSERT_ERR((((uint16_t)rxadvlen << 8) & ~((uint16_t)0x0000FF00)) == 0);
    EM_BLE_WR(BLE_RXPHADV_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE, (EM_BLE_RD(BLE_RXPHADV_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE) & ~((uint16_t)0x0000FF00)) | ((uint16_t)rxadvlen << 8));
}

__STATIC_INLINE uint8_t ble_rxrxadd_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_RXPHADV_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);
    return ((localVal & ((uint16_t)0x00000080)) >> 7);
}

__STATIC_INLINE void ble_rxrxadd_setf(int elt_idx, uint8_t rxrxadd)
{
    ASSERT_ERR((((uint16_t)rxrxadd << 7) & ~((uint16_t)0x00000080)) == 0);
    EM_BLE_WR(BLE_RXPHADV_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE, (EM_BLE_RD(BLE_RXPHADV_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE) & ~((uint16_t)0x00000080)) | ((uint16_t)rxrxadd << 7));
}

__STATIC_INLINE uint8_t ble_rxtxadd_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_RXPHADV_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);
    return ((localVal & ((uint16_t)0x00000040)) >> 6);
}

__STATIC_INLINE void ble_rxtxadd_setf(int elt_idx, uint8_t rxtxadd)
{
    ASSERT_ERR((((uint16_t)rxtxadd << 6) & ~((uint16_t)0x00000040)) == 0);
    EM_BLE_WR(BLE_RXPHADV_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE, (EM_BLE_RD(BLE_RXPHADV_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE) & ~((uint16_t)0x00000040)) | ((uint16_t)rxtxadd << 6));
}

__STATIC_INLINE uint8_t ble_rxtype_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_RXPHADV_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);
    return ((localVal & ((uint16_t)0x0000000F)) >> 0);
}

__STATIC_INLINE void ble_rxtype_setf(int elt_idx, uint8_t rxtype)
{
    ASSERT_ERR((((uint16_t)rxtype << 0) & ~((uint16_t)0x0000000F)) == 0);
    EM_BLE_WR(BLE_RXPHADV_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE, (EM_BLE_RD(BLE_RXPHADV_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE) & ~((uint16_t)0x0000000F)) | ((uint16_t)rxtype << 0));
}

/**
 * @brief RXCHASS register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  13:08          USED_CH_IDX   0x0
 *  07:00                 RSSI   0x0
 * </pre>
 */
#define BLE_RXCHASS_ADDR   (0x06 + (_ble_base) + EM_BLE_RX_DESC_OFFSET)
#define BLE_RXCHASS_INDEX  0x00000003
#define BLE_RXCHASS_RESET  0x00000000

__STATIC_INLINE uint16_t ble_rxchass_get(int elt_idx)
{
    return EM_BLE_RD(BLE_RXCHASS_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);
}

__STATIC_INLINE void ble_rxchass_set(int elt_idx, uint16_t value)
{
    EM_BLE_WR(BLE_RXCHASS_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE, value);
}

// field definitions
#define BLE_USED_CH_IDX_MASK   ((uint16_t)0x00003F00)
#define BLE_USED_CH_IDX_LSB    8
#define BLE_USED_CH_IDX_WIDTH  ((uint16_t)0x00000006)
#define BLE_RSSI_MASK          ((uint16_t)0x000000FF)
#define BLE_RSSI_LSB           0
#define BLE_RSSI_WIDTH         ((uint16_t)0x00000008)

#define BLE_USED_CH_IDX_RST    0x0
#define BLE_RSSI_RST           0x0

__STATIC_INLINE void ble_rxchass_pack(int elt_idx, uint8_t usedchidx, uint8_t rssi)
{
    ASSERT_ERR((((uint16_t)usedchidx << 8) & ~((uint16_t)0x00003F00)) == 0);
    ASSERT_ERR((((uint16_t)rssi << 0) & ~((uint16_t)0x000000FF)) == 0);
    EM_BLE_WR(BLE_RXCHASS_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE,  ((uint16_t)usedchidx << 8) | ((uint16_t)rssi << 0));
}

__STATIC_INLINE void ble_rxchass_unpack(int elt_idx, uint8_t* usedchidx, uint8_t* rssi)
{
    uint16_t localVal = EM_BLE_RD(BLE_RXCHASS_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);

    *usedchidx = (localVal & ((uint16_t)0x00003F00)) >> 8;
    *rssi = (localVal & ((uint16_t)0x000000FF)) >> 0;
}

__STATIC_INLINE uint8_t ble_used_ch_idx_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_RXCHASS_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);
    return ((localVal & ((uint16_t)0x00003F00)) >> 8);
}

__STATIC_INLINE void ble_used_ch_idx_setf(int elt_idx, uint8_t usedchidx)
{
    ASSERT_ERR((((uint16_t)usedchidx << 8) & ~((uint16_t)0x00003F00)) == 0);
    EM_BLE_WR(BLE_RXCHASS_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE, (EM_BLE_RD(BLE_RXCHASS_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE) & ~((uint16_t)0x00003F00)) | ((uint16_t)usedchidx << 8));
}

__STATIC_INLINE uint8_t ble_rssi_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_RXCHASS_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);
    return ((localVal & ((uint16_t)0x000000FF)) >> 0);
}

__STATIC_INLINE void ble_rssi_setf(int elt_idx, uint8_t rssi)
{
    ASSERT_ERR((((uint16_t)rssi << 0) & ~((uint16_t)0x000000FF)) == 0);
    EM_BLE_WR(BLE_RXCHASS_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE, (EM_BLE_RD(BLE_RXCHASS_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE) & ~((uint16_t)0x000000FF)) | ((uint16_t)rssi << 0));
}

/**
 * @brief RXDATAPTR register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  15:00            RXDATAPTR   0x0
 * </pre>
 */
#define BLE_RXDATAPTR_ADDR   (0x08 + (_ble_base) + EM_BLE_RX_DESC_OFFSET)
#define BLE_RXDATAPTR_INDEX  0x00000004
#define BLE_RXDATAPTR_RESET  0x00000000

__STATIC_INLINE uint16_t ble_rxdataptr_get(int elt_idx)
{
    return EM_BLE_RD(BLE_RXDATAPTR_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);
}

__STATIC_INLINE void ble_rxdataptr_set(int elt_idx, uint16_t value)
{
    EM_BLE_WR(BLE_RXDATAPTR_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE, value);
}

// field definitions
#define BLE_RXDATAPTR_MASK   ((uint16_t)0x0000FFFF)
#define BLE_RXDATAPTR_LSB    0
#define BLE_RXDATAPTR_WIDTH  ((uint16_t)0x00000010)

#define BLE_RXDATAPTR_RST    0x0

__STATIC_INLINE uint16_t ble_rxdataptr_getf(int elt_idx)
{
    uint16_t localVal = EM_BLE_RD(BLE_RXDATAPTR_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE);
    ASSERT_ERR((localVal & ~((uint16_t)0x0000FFFF)) == 0);
    return (localVal >> 0);
}

__STATIC_INLINE void ble_rxdataptr_setf(int elt_idx, uint16_t rxdataptr)
{
    ASSERT_ERR((((uint16_t)rxdataptr << 0) & ~((uint16_t)0x0000FFFF)) == 0);
    EM_BLE_WR(BLE_RXDATAPTR_ADDR + elt_idx * REG_BLE_EM_RX_DESC_SIZE, (uint16_t)rxdataptr << 0);
}


#endif // _REG_BLE_EM_RX_DESC_H_

