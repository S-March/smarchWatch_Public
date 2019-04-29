#ifndef _REG_MODEM_H_
#define _REG_MODEM_H_

#include <stdint.h>
#include "_reg_modem.h"
#include "compiler.h"
#include "arch.h"
#include "reg_access.h"

#define REG_MODEM_COUNT 53

#define REG_MODEM_DECODING_MASK 0x000000FF

/**
 * @brief VERSION register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:24                  TYP   0x1
 *  23:16                  REL   0x1
 *  15:08                  UPG   0x1
 *  07:00                BUILD   0x0
 * </pre>
 */
#define MDM_VERSION_ADDR   0x00000080
#define MDM_VERSION_OFFSET 0x00000080
#define MDM_VERSION_INDEX  0x00000020
#define MDM_VERSION_RESET  0x01010100

__STATIC_INLINE uint32_t mdm_version_get(void)
{
    return REG_RPL_RD(MDM_VERSION_ADDR);
}

// field definitions
#define MDM_TYP_MASK     ((uint32_t)0xFF000000)
#define MDM_TYP_LSB      24
#define MDM_TYP_WIDTH    ((uint32_t)0x00000008)
#define MDM_REL_MASK     ((uint32_t)0x00FF0000)
#define MDM_REL_LSB      16
#define MDM_REL_WIDTH    ((uint32_t)0x00000008)
#define MDM_UPG_MASK     ((uint32_t)0x0000FF00)
#define MDM_UPG_LSB      8
#define MDM_UPG_WIDTH    ((uint32_t)0x00000008)
#define MDM_BUILD_MASK   ((uint32_t)0x000000FF)
#define MDM_BUILD_LSB    0
#define MDM_BUILD_WIDTH  ((uint32_t)0x00000008)

#define MDM_TYP_RST      0x1
#define MDM_REL_RST      0x1
#define MDM_UPG_RST      0x1
#define MDM_BUILD_RST    0x0

__STATIC_INLINE void mdm_version_unpack(uint8_t* typ, uint8_t* rel, uint8_t* upg, uint8_t* build)
{
    uint32_t localVal = REG_RPL_RD(MDM_VERSION_ADDR);

    *typ = (localVal & ((uint32_t)0xFF000000)) >> 24;
    *rel = (localVal & ((uint32_t)0x00FF0000)) >> 16;
    *upg = (localVal & ((uint32_t)0x0000FF00)) >> 8;
    *build = (localVal & ((uint32_t)0x000000FF)) >> 0;
}

__STATIC_INLINE uint8_t mdm_typ_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_VERSION_ADDR);
    return ((localVal & ((uint32_t)0xFF000000)) >> 24);
}

__STATIC_INLINE uint8_t mdm_rel_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_VERSION_ADDR);
    return ((localVal & ((uint32_t)0x00FF0000)) >> 16);
}

__STATIC_INLINE uint8_t mdm_upg_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_VERSION_ADDR);
    return ((localVal & ((uint32_t)0x0000FF00)) >> 8);
}

__STATIC_INLINE uint8_t mdm_build_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_VERSION_ADDR);
    return ((localVal & ((uint32_t)0x000000FF)) >> 0);
}

/**
 * @brief MDM_CNTL register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *     01         DUAL_MODE_EN   0
 *     00          RXFE_BYPASS   0
 * </pre>
 */
#define MDM_MDM_CNTL_ADDR   0x00000081
#define MDM_MDM_CNTL_OFFSET 0x00000081
#define MDM_MDM_CNTL_INDEX  0x00000020
#define MDM_MDM_CNTL_RESET  0x00000000

__STATIC_INLINE uint32_t mdm_mdm_cntl_get(void)
{
    return REG_RPL_RD(MDM_MDM_CNTL_ADDR);
}

__STATIC_INLINE void mdm_mdm_cntl_set(uint32_t value)
{
    REG_RPL_WR(MDM_MDM_CNTL_ADDR, value);
}

// field definitions
#define MDM_DUAL_MODE_EN_BIT    ((uint32_t)0x00000002)
#define MDM_DUAL_MODE_EN_POS    1
#define MDM_RXFE_BYPASS_BIT     ((uint32_t)0x00000001)
#define MDM_RXFE_BYPASS_POS     0

#define MDM_DUAL_MODE_EN_RST    0x0
#define MDM_RXFE_BYPASS_RST     0x0

__STATIC_INLINE void mdm_mdm_cntl_pack(uint8_t dualmodeen, uint8_t rxfebypass)
{
    ASSERT_ERR((((uint32_t)dualmodeen << 1) & ~((uint32_t)0x00000002)) == 0);
    ASSERT_ERR((((uint32_t)rxfebypass << 0) & ~((uint32_t)0x00000001)) == 0);
    REG_RPL_WR(MDM_MDM_CNTL_ADDR,  ((uint32_t)dualmodeen << 1) | ((uint32_t)rxfebypass << 0));
}

__STATIC_INLINE void mdm_mdm_cntl_unpack(uint8_t* dualmodeen, uint8_t* rxfebypass)
{
    uint32_t localVal = REG_RPL_RD(MDM_MDM_CNTL_ADDR);

    *dualmodeen = (localVal & ((uint32_t)0x00000002)) >> 1;
    *rxfebypass = (localVal & ((uint32_t)0x00000001)) >> 0;
}

__STATIC_INLINE uint8_t mdm_dual_mode_en_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_MDM_CNTL_ADDR);
    return ((localVal & ((uint32_t)0x00000002)) >> 1);
}

__STATIC_INLINE void mdm_dual_mode_en_setf(uint8_t dualmodeen)
{
    ASSERT_ERR((((uint32_t)dualmodeen << 1) & ~((uint32_t)0x00000002)) == 0);
    REG_RPL_WR(MDM_MDM_CNTL_ADDR, (REG_RPL_RD(MDM_MDM_CNTL_ADDR) & ~((uint32_t)0x00000002)) | ((uint32_t)dualmodeen << 1));
}

__STATIC_INLINE uint8_t mdm_rxfe_bypass_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_MDM_CNTL_ADDR);
    return ((localVal & ((uint32_t)0x00000001)) >> 0);
}

__STATIC_INLINE void mdm_rxfe_bypass_setf(uint8_t rxfebypass)
{
    ASSERT_ERR((((uint32_t)rxfebypass << 0) & ~((uint32_t)0x00000001)) == 0);
    REG_RPL_WR(MDM_MDM_CNTL_ADDR, (REG_RPL_RD(MDM_MDM_CNTL_ADDR) & ~((uint32_t)0x00000001)) | ((uint32_t)rxfebypass << 0));
}

/**
 * @brief CLKCNTL register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *     08           FORCE_GCLK   0
 *     07           RX_GCLK_EN   0
 *     06       RXDPSK_GCLK_EN   0
 *     05       RXACDL_GCLK_EN   0
 *     04       RXGFSK_GCLK_EN   0
 *     02           TX_GCLK_EN   0
 *     01       TXDPSK_GCLK_EN   0
 *     00       TXGFSK_GCLK_EN   0
 * </pre>
 */
#define MDM_CLKCNTL_ADDR   0x00000082
#define MDM_CLKCNTL_OFFSET 0x00000082
#define MDM_CLKCNTL_INDEX  0x00000020
#define MDM_CLKCNTL_RESET  0x00000000

__STATIC_INLINE uint32_t mdm_clkcntl_get(void)
{
    return REG_RPL_RD(MDM_CLKCNTL_ADDR);
}

__STATIC_INLINE void mdm_clkcntl_set(uint32_t value)
{
    REG_RPL_WR(MDM_CLKCNTL_ADDR, value);
}

// field definitions
#define MDM_FORCE_GCLK_BIT        ((uint32_t)0x00000100)
#define MDM_FORCE_GCLK_POS        8
#define MDM_RX_GCLK_EN_BIT        ((uint32_t)0x00000080)
#define MDM_RX_GCLK_EN_POS        7
#define MDM_RXDPSK_GCLK_EN_BIT    ((uint32_t)0x00000040)
#define MDM_RXDPSK_GCLK_EN_POS    6
#define MDM_RXACDL_GCLK_EN_BIT    ((uint32_t)0x00000020)
#define MDM_RXACDL_GCLK_EN_POS    5
#define MDM_RXGFSK_GCLK_EN_BIT    ((uint32_t)0x00000010)
#define MDM_RXGFSK_GCLK_EN_POS    4
#define MDM_TX_GCLK_EN_BIT        ((uint32_t)0x00000004)
#define MDM_TX_GCLK_EN_POS        2
#define MDM_TXDPSK_GCLK_EN_BIT    ((uint32_t)0x00000002)
#define MDM_TXDPSK_GCLK_EN_POS    1
#define MDM_TXGFSK_GCLK_EN_BIT    ((uint32_t)0x00000001)
#define MDM_TXGFSK_GCLK_EN_POS    0

#define MDM_FORCE_GCLK_RST        0x0
#define MDM_RX_GCLK_EN_RST        0x0
#define MDM_RXDPSK_GCLK_EN_RST    0x0
#define MDM_RXACDL_GCLK_EN_RST    0x0
#define MDM_RXGFSK_GCLK_EN_RST    0x0
#define MDM_TX_GCLK_EN_RST        0x0
#define MDM_TXDPSK_GCLK_EN_RST    0x0
#define MDM_TXGFSK_GCLK_EN_RST    0x0

__STATIC_INLINE void mdm_clkcntl_pack(uint8_t forcegclk, uint8_t rxgclken, uint8_t rxdpskgclken, uint8_t rxacdlgclken, uint8_t rxgfskgclken, uint8_t txgclken, uint8_t txdpskgclken, uint8_t txgfskgclken)
{
    ASSERT_ERR((((uint32_t)forcegclk << 8) & ~((uint32_t)0x00000100)) == 0);
    ASSERT_ERR((((uint32_t)rxgclken << 7) & ~((uint32_t)0x00000080)) == 0);
    ASSERT_ERR((((uint32_t)rxdpskgclken << 6) & ~((uint32_t)0x00000040)) == 0);
    ASSERT_ERR((((uint32_t)rxacdlgclken << 5) & ~((uint32_t)0x00000020)) == 0);
    ASSERT_ERR((((uint32_t)rxgfskgclken << 4) & ~((uint32_t)0x00000010)) == 0);
    ASSERT_ERR((((uint32_t)txgclken << 2) & ~((uint32_t)0x00000004)) == 0);
    ASSERT_ERR((((uint32_t)txdpskgclken << 1) & ~((uint32_t)0x00000002)) == 0);
    ASSERT_ERR((((uint32_t)txgfskgclken << 0) & ~((uint32_t)0x00000001)) == 0);
    REG_RPL_WR(MDM_CLKCNTL_ADDR,  ((uint32_t)forcegclk << 8) | ((uint32_t)rxgclken << 7) | ((uint32_t)rxdpskgclken << 6) | ((uint32_t)rxacdlgclken << 5) | ((uint32_t)rxgfskgclken << 4) | ((uint32_t)txgclken << 2) | ((uint32_t)txdpskgclken << 1) | ((uint32_t)txgfskgclken << 0));
}

__STATIC_INLINE void mdm_clkcntl_unpack(uint8_t* forcegclk, uint8_t* rxgclken, uint8_t* rxdpskgclken, uint8_t* rxacdlgclken, uint8_t* rxgfskgclken, uint8_t* txgclken, uint8_t* txdpskgclken, uint8_t* txgfskgclken)
{
    uint32_t localVal = REG_RPL_RD(MDM_CLKCNTL_ADDR);

    *forcegclk = (localVal & ((uint32_t)0x00000100)) >> 8;
    *rxgclken = (localVal & ((uint32_t)0x00000080)) >> 7;
    *rxdpskgclken = (localVal & ((uint32_t)0x00000040)) >> 6;
    *rxacdlgclken = (localVal & ((uint32_t)0x00000020)) >> 5;
    *rxgfskgclken = (localVal & ((uint32_t)0x00000010)) >> 4;
    *txgclken = (localVal & ((uint32_t)0x00000004)) >> 2;
    *txdpskgclken = (localVal & ((uint32_t)0x00000002)) >> 1;
    *txgfskgclken = (localVal & ((uint32_t)0x00000001)) >> 0;
}

__STATIC_INLINE uint8_t mdm_force_gclk_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_CLKCNTL_ADDR);
    return ((localVal & ((uint32_t)0x00000100)) >> 8);
}

__STATIC_INLINE void mdm_force_gclk_setf(uint8_t forcegclk)
{
    ASSERT_ERR((((uint32_t)forcegclk << 8) & ~((uint32_t)0x00000100)) == 0);
    REG_RPL_WR(MDM_CLKCNTL_ADDR, (REG_RPL_RD(MDM_CLKCNTL_ADDR) & ~((uint32_t)0x00000100)) | ((uint32_t)forcegclk << 8));
}

__STATIC_INLINE uint8_t mdm_rx_gclk_en_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_CLKCNTL_ADDR);
    return ((localVal & ((uint32_t)0x00000080)) >> 7);
}

__STATIC_INLINE void mdm_rx_gclk_en_setf(uint8_t rxgclken)
{
    ASSERT_ERR((((uint32_t)rxgclken << 7) & ~((uint32_t)0x00000080)) == 0);
    REG_RPL_WR(MDM_CLKCNTL_ADDR, (REG_RPL_RD(MDM_CLKCNTL_ADDR) & ~((uint32_t)0x00000080)) | ((uint32_t)rxgclken << 7));
}

__STATIC_INLINE uint8_t mdm_rxdpsk_gclk_en_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_CLKCNTL_ADDR);
    return ((localVal & ((uint32_t)0x00000040)) >> 6);
}

__STATIC_INLINE void mdm_rxdpsk_gclk_en_setf(uint8_t rxdpskgclken)
{
    ASSERT_ERR((((uint32_t)rxdpskgclken << 6) & ~((uint32_t)0x00000040)) == 0);
    REG_RPL_WR(MDM_CLKCNTL_ADDR, (REG_RPL_RD(MDM_CLKCNTL_ADDR) & ~((uint32_t)0x00000040)) | ((uint32_t)rxdpskgclken << 6));
}

__STATIC_INLINE uint8_t mdm_rxacdl_gclk_en_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_CLKCNTL_ADDR);
    return ((localVal & ((uint32_t)0x00000020)) >> 5);
}

__STATIC_INLINE void mdm_rxacdl_gclk_en_setf(uint8_t rxacdlgclken)
{
    ASSERT_ERR((((uint32_t)rxacdlgclken << 5) & ~((uint32_t)0x00000020)) == 0);
    REG_RPL_WR(MDM_CLKCNTL_ADDR, (REG_RPL_RD(MDM_CLKCNTL_ADDR) & ~((uint32_t)0x00000020)) | ((uint32_t)rxacdlgclken << 5));
}

__STATIC_INLINE uint8_t mdm_rxgfsk_gclk_en_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_CLKCNTL_ADDR);
    return ((localVal & ((uint32_t)0x00000010)) >> 4);
}

__STATIC_INLINE void mdm_rxgfsk_gclk_en_setf(uint8_t rxgfskgclken)
{
    ASSERT_ERR((((uint32_t)rxgfskgclken << 4) & ~((uint32_t)0x00000010)) == 0);
    REG_RPL_WR(MDM_CLKCNTL_ADDR, (REG_RPL_RD(MDM_CLKCNTL_ADDR) & ~((uint32_t)0x00000010)) | ((uint32_t)rxgfskgclken << 4));
}

__STATIC_INLINE uint8_t mdm_tx_gclk_en_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_CLKCNTL_ADDR);
    return ((localVal & ((uint32_t)0x00000004)) >> 2);
}

__STATIC_INLINE void mdm_tx_gclk_en_setf(uint8_t txgclken)
{
    ASSERT_ERR((((uint32_t)txgclken << 2) & ~((uint32_t)0x00000004)) == 0);
    REG_RPL_WR(MDM_CLKCNTL_ADDR, (REG_RPL_RD(MDM_CLKCNTL_ADDR) & ~((uint32_t)0x00000004)) | ((uint32_t)txgclken << 2));
}

__STATIC_INLINE uint8_t mdm_txdpsk_gclk_en_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_CLKCNTL_ADDR);
    return ((localVal & ((uint32_t)0x00000002)) >> 1);
}

__STATIC_INLINE void mdm_txdpsk_gclk_en_setf(uint8_t txdpskgclken)
{
    ASSERT_ERR((((uint32_t)txdpskgclken << 1) & ~((uint32_t)0x00000002)) == 0);
    REG_RPL_WR(MDM_CLKCNTL_ADDR, (REG_RPL_RD(MDM_CLKCNTL_ADDR) & ~((uint32_t)0x00000002)) | ((uint32_t)txdpskgclken << 1));
}

__STATIC_INLINE uint8_t mdm_txgfsk_gclk_en_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_CLKCNTL_ADDR);
    return ((localVal & ((uint32_t)0x00000001)) >> 0);
}

__STATIC_INLINE void mdm_txgfsk_gclk_en_setf(uint8_t txgfskgclken)
{
    ASSERT_ERR((((uint32_t)txgfskgclken << 0) & ~((uint32_t)0x00000001)) == 0);
    REG_RPL_WR(MDM_CLKCNTL_ADDR, (REG_RPL_RD(MDM_CLKCNTL_ADDR) & ~((uint32_t)0x00000001)) | ((uint32_t)txgfskgclken << 0));
}

/**
 * @brief RX_STARTUPDEL register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  19:16       BEFORE_RXSTART   0x1
 *  07:00        RX_STARTUPDEL   0xB4
 * </pre>
 */
#define MDM_RX_STARTUPDEL_ADDR   0x00000083
#define MDM_RX_STARTUPDEL_OFFSET 0x00000083
#define MDM_RX_STARTUPDEL_INDEX  0x00000020
#define MDM_RX_STARTUPDEL_RESET  0x000100B4

__STATIC_INLINE uint32_t mdm_rx_startupdel_get(void)
{
    return REG_RPL_RD(MDM_RX_STARTUPDEL_ADDR);
}

__STATIC_INLINE void mdm_rx_startupdel_set(uint32_t value)
{
    REG_RPL_WR(MDM_RX_STARTUPDEL_ADDR, value);
}

// field definitions
#define MDM_BEFORE_RXSTART_MASK   ((uint32_t)0x000F0000)
#define MDM_BEFORE_RXSTART_LSB    16
#define MDM_BEFORE_RXSTART_WIDTH  ((uint32_t)0x00000004)
#define MDM_RX_STARTUPDEL_MASK    ((uint32_t)0x000000FF)
#define MDM_RX_STARTUPDEL_LSB     0
#define MDM_RX_STARTUPDEL_WIDTH   ((uint32_t)0x00000008)

#define MDM_BEFORE_RXSTART_RST    0x1
#define MDM_RX_STARTUPDEL_RST     0xB4

__STATIC_INLINE void mdm_rx_startupdel_pack(uint8_t beforerxstart, uint8_t rxstartupdel)
{
    ASSERT_ERR((((uint32_t)beforerxstart << 16) & ~((uint32_t)0x000F0000)) == 0);
    ASSERT_ERR((((uint32_t)rxstartupdel << 0) & ~((uint32_t)0x000000FF)) == 0);
    REG_RPL_WR(MDM_RX_STARTUPDEL_ADDR,  ((uint32_t)beforerxstart << 16) | ((uint32_t)rxstartupdel << 0));
}

__STATIC_INLINE void mdm_rx_startupdel_unpack(uint8_t* beforerxstart, uint8_t* rxstartupdel)
{
    uint32_t localVal = REG_RPL_RD(MDM_RX_STARTUPDEL_ADDR);

    *beforerxstart = (localVal & ((uint32_t)0x000F0000)) >> 16;
    *rxstartupdel = (localVal & ((uint32_t)0x000000FF)) >> 0;
}

__STATIC_INLINE uint8_t mdm_before_rxstart_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_RX_STARTUPDEL_ADDR);
    return ((localVal & ((uint32_t)0x000F0000)) >> 16);
}

__STATIC_INLINE void mdm_before_rxstart_setf(uint8_t beforerxstart)
{
    ASSERT_ERR((((uint32_t)beforerxstart << 16) & ~((uint32_t)0x000F0000)) == 0);
    REG_RPL_WR(MDM_RX_STARTUPDEL_ADDR, (REG_RPL_RD(MDM_RX_STARTUPDEL_ADDR) & ~((uint32_t)0x000F0000)) | ((uint32_t)beforerxstart << 16));
}

__STATIC_INLINE uint8_t mdm_rx_startupdel_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_RX_STARTUPDEL_ADDR);
    return ((localVal & ((uint32_t)0x000000FF)) >> 0);
}

__STATIC_INLINE void mdm_rx_startupdel_setf(uint8_t rxstartupdel)
{
    ASSERT_ERR((((uint32_t)rxstartupdel << 0) & ~((uint32_t)0x000000FF)) == 0);
    REG_RPL_WR(MDM_RX_STARTUPDEL_ADDR, (REG_RPL_RD(MDM_RX_STARTUPDEL_ADDR) & ~((uint32_t)0x000000FF)) | ((uint32_t)rxstartupdel << 0));
}

/**
 * @brief TX_STARTUPDEL register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  19:16       BEFORE_TXSTART   0x1
 *  07:00        TX_STARTUPDEL   0xB4
 * </pre>
 */
#define MDM_TX_STARTUPDEL_ADDR   0x00000084
#define MDM_TX_STARTUPDEL_OFFSET 0x00000084
#define MDM_TX_STARTUPDEL_INDEX  0x00000021
#define MDM_TX_STARTUPDEL_RESET  0x000100B4

__STATIC_INLINE uint32_t mdm_tx_startupdel_get(void)
{
    return REG_RPL_RD(MDM_TX_STARTUPDEL_ADDR);
}

__STATIC_INLINE void mdm_tx_startupdel_set(uint32_t value)
{
    REG_RPL_WR(MDM_TX_STARTUPDEL_ADDR, value);
}

// field definitions
#define MDM_BEFORE_TXSTART_MASK   ((uint32_t)0x000F0000)
#define MDM_BEFORE_TXSTART_LSB    16
#define MDM_BEFORE_TXSTART_WIDTH  ((uint32_t)0x00000004)
#define MDM_TX_STARTUPDEL_MASK    ((uint32_t)0x000000FF)
#define MDM_TX_STARTUPDEL_LSB     0
#define MDM_TX_STARTUPDEL_WIDTH   ((uint32_t)0x00000008)

#define MDM_BEFORE_TXSTART_RST    0x1
#define MDM_TX_STARTUPDEL_RST     0xB4

__STATIC_INLINE void mdm_tx_startupdel_pack(uint8_t beforetxstart, uint8_t txstartupdel)
{
    ASSERT_ERR((((uint32_t)beforetxstart << 16) & ~((uint32_t)0x000F0000)) == 0);
    ASSERT_ERR((((uint32_t)txstartupdel << 0) & ~((uint32_t)0x000000FF)) == 0);
    REG_RPL_WR(MDM_TX_STARTUPDEL_ADDR,  ((uint32_t)beforetxstart << 16) | ((uint32_t)txstartupdel << 0));
}

__STATIC_INLINE void mdm_tx_startupdel_unpack(uint8_t* beforetxstart, uint8_t* txstartupdel)
{
    uint32_t localVal = REG_RPL_RD(MDM_TX_STARTUPDEL_ADDR);

    *beforetxstart = (localVal & ((uint32_t)0x000F0000)) >> 16;
    *txstartupdel = (localVal & ((uint32_t)0x000000FF)) >> 0;
}

__STATIC_INLINE uint8_t mdm_before_txstart_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_TX_STARTUPDEL_ADDR);
    return ((localVal & ((uint32_t)0x000F0000)) >> 16);
}

__STATIC_INLINE void mdm_before_txstart_setf(uint8_t beforetxstart)
{
    ASSERT_ERR((((uint32_t)beforetxstart << 16) & ~((uint32_t)0x000F0000)) == 0);
    REG_RPL_WR(MDM_TX_STARTUPDEL_ADDR, (REG_RPL_RD(MDM_TX_STARTUPDEL_ADDR) & ~((uint32_t)0x000F0000)) | ((uint32_t)beforetxstart << 16));
}

__STATIC_INLINE uint8_t mdm_tx_startupdel_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_TX_STARTUPDEL_ADDR);
    return ((localVal & ((uint32_t)0x000000FF)) >> 0);
}

__STATIC_INLINE void mdm_tx_startupdel_setf(uint8_t txstartupdel)
{
    ASSERT_ERR((((uint32_t)txstartupdel << 0) & ~((uint32_t)0x000000FF)) == 0);
    REG_RPL_WR(MDM_TX_STARTUPDEL_ADDR, (REG_RPL_RD(MDM_TX_STARTUPDEL_ADDR) & ~((uint32_t)0x000000FF)) | ((uint32_t)txstartupdel << 0));
}

/**
 * @brief TX_GFSKMODE register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *     01             FMTX2PEN   0
 *     00               FMTXEN   0
 * </pre>
 */
#define MDM_TX_GFSKMODE_ADDR   0x00000085
#define MDM_TX_GFSKMODE_OFFSET 0x00000085
#define MDM_TX_GFSKMODE_INDEX  0x00000021
#define MDM_TX_GFSKMODE_RESET  0x00000000

__STATIC_INLINE uint32_t mdm_tx_gfskmode_get(void)
{
    return REG_RPL_RD(MDM_TX_GFSKMODE_ADDR);
}

__STATIC_INLINE void mdm_tx_gfskmode_set(uint32_t value)
{
    REG_RPL_WR(MDM_TX_GFSKMODE_ADDR, value);
}

// field definitions
#define MDM_FMTX2PEN_BIT    ((uint32_t)0x00000002)
#define MDM_FMTX2PEN_POS    1
#define MDM_FMTXEN_BIT      ((uint32_t)0x00000001)
#define MDM_FMTXEN_POS      0

#define MDM_FMTX2PEN_RST    0x0
#define MDM_FMTXEN_RST      0x0

__STATIC_INLINE void mdm_tx_gfskmode_pack(uint8_t fmtx2pen, uint8_t fmtxen)
{
    ASSERT_ERR((((uint32_t)fmtx2pen << 1) & ~((uint32_t)0x00000002)) == 0);
    ASSERT_ERR((((uint32_t)fmtxen << 0) & ~((uint32_t)0x00000001)) == 0);
    REG_RPL_WR(MDM_TX_GFSKMODE_ADDR,  ((uint32_t)fmtx2pen << 1) | ((uint32_t)fmtxen << 0));
}

__STATIC_INLINE void mdm_tx_gfskmode_unpack(uint8_t* fmtx2pen, uint8_t* fmtxen)
{
    uint32_t localVal = REG_RPL_RD(MDM_TX_GFSKMODE_ADDR);

    *fmtx2pen = (localVal & ((uint32_t)0x00000002)) >> 1;
    *fmtxen = (localVal & ((uint32_t)0x00000001)) >> 0;
}

__STATIC_INLINE uint8_t mdm_fmtx2pen_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_TX_GFSKMODE_ADDR);
    return ((localVal & ((uint32_t)0x00000002)) >> 1);
}

__STATIC_INLINE void mdm_fmtx2pen_setf(uint8_t fmtx2pen)
{
    ASSERT_ERR((((uint32_t)fmtx2pen << 1) & ~((uint32_t)0x00000002)) == 0);
    REG_RPL_WR(MDM_TX_GFSKMODE_ADDR, (REG_RPL_RD(MDM_TX_GFSKMODE_ADDR) & ~((uint32_t)0x00000002)) | ((uint32_t)fmtx2pen << 1));
}

__STATIC_INLINE uint8_t mdm_fmtxen_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_TX_GFSKMODE_ADDR);
    return ((localVal & ((uint32_t)0x00000001)) >> 0);
}

__STATIC_INLINE void mdm_fmtxen_setf(uint8_t fmtxen)
{
    ASSERT_ERR((((uint32_t)fmtxen << 0) & ~((uint32_t)0x00000001)) == 0);
    REG_RPL_WR(MDM_TX_GFSKMODE_ADDR, (REG_RPL_RD(MDM_TX_GFSKMODE_ADDR) & ~((uint32_t)0x00000001)) | ((uint32_t)fmtxen << 0));
}

/**
 * @brief DIAGCNTL register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  07:00             DIAGCNTL   0x0
 * </pre>
 */
#define MDM_DIAGCNTL_ADDR   0x00000086
#define MDM_DIAGCNTL_OFFSET 0x00000086
#define MDM_DIAGCNTL_INDEX  0x00000021
#define MDM_DIAGCNTL_RESET  0x00000000

__STATIC_INLINE uint32_t mdm_diagcntl_get(void)
{
    return REG_RPL_RD(MDM_DIAGCNTL_ADDR);
}

__STATIC_INLINE void mdm_diagcntl_set(uint32_t value)
{
    REG_RPL_WR(MDM_DIAGCNTL_ADDR, value);
}

// field definitions
#define MDM_DIAGCNTL_MASK   ((uint32_t)0x000000FF)
#define MDM_DIAGCNTL_LSB    0
#define MDM_DIAGCNTL_WIDTH  ((uint32_t)0x00000008)

#define MDM_DIAGCNTL_RST    0x0

__STATIC_INLINE uint8_t mdm_diagcntl_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_DIAGCNTL_ADDR);
    ASSERT_ERR((localVal & ~((uint32_t)0x000000FF)) == 0);
    return (localVal >> 0);
}

__STATIC_INLINE void mdm_diagcntl_setf(uint8_t diagcntl)
{
    ASSERT_ERR((((uint32_t)diagcntl << 0) & ~((uint32_t)0x000000FF)) == 0);
    REG_RPL_WR(MDM_DIAGCNTL_ADDR, (uint32_t)diagcntl << 0);
}

/**
 * @brief RX_PWR register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  05:00                RXPWR   0x0
 * </pre>
 */
#define MDM_RX_PWR_ADDR   0x00000087
#define MDM_RX_PWR_OFFSET 0x00000087
#define MDM_RX_PWR_INDEX  0x00000021
#define MDM_RX_PWR_RESET  0x00000000

__STATIC_INLINE uint32_t mdm_rx_pwr_get(void)
{
    return REG_RPL_RD(MDM_RX_PWR_ADDR);
}

__STATIC_INLINE void mdm_rx_pwr_set(uint32_t value)
{
    REG_RPL_WR(MDM_RX_PWR_ADDR, value);
}

// field definitions
#define MDM_RXPWR_MASK   ((uint32_t)0x0000003F)
#define MDM_RXPWR_LSB    0
#define MDM_RXPWR_WIDTH  ((uint32_t)0x00000006)

#define MDM_RXPWR_RST    0x0

__STATIC_INLINE uint8_t mdm_rxpwr_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_RX_PWR_ADDR);
    ASSERT_ERR((localVal & ~((uint32_t)0x0000003F)) == 0);
    return (localVal >> 0);
}

/**
 * @brief RXFE_CNTL register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *     01             FE_DCCEN   1
 *     00             FE_FOCEN   1
 * </pre>
 */
#define MDM_RXFE_CNTL_ADDR   0x00000090
#define MDM_RXFE_CNTL_OFFSET 0x00000090
#define MDM_RXFE_CNTL_INDEX  0x00000024
#define MDM_RXFE_CNTL_RESET  0x00000003

__STATIC_INLINE uint32_t mdm_rxfe_cntl_get(void)
{
    return REG_RPL_RD(MDM_RXFE_CNTL_ADDR);
}

__STATIC_INLINE void mdm_rxfe_cntl_set(uint32_t value)
{
    REG_RPL_WR(MDM_RXFE_CNTL_ADDR, value);
}

// field definitions
#define MDM_FE_DCCEN_BIT    ((uint32_t)0x00000002)
#define MDM_FE_DCCEN_POS    1
#define MDM_FE_FOCEN_BIT    ((uint32_t)0x00000001)
#define MDM_FE_FOCEN_POS    0

#define MDM_FE_DCCEN_RST    0x1
#define MDM_FE_FOCEN_RST    0x1

__STATIC_INLINE void mdm_rxfe_cntl_pack(uint8_t fedccen, uint8_t fefocen)
{
    ASSERT_ERR((((uint32_t)fedccen << 1) & ~((uint32_t)0x00000002)) == 0);
    ASSERT_ERR((((uint32_t)fefocen << 0) & ~((uint32_t)0x00000001)) == 0);
    REG_RPL_WR(MDM_RXFE_CNTL_ADDR,  ((uint32_t)fedccen << 1) | ((uint32_t)fefocen << 0));
}

__STATIC_INLINE void mdm_rxfe_cntl_unpack(uint8_t* fedccen, uint8_t* fefocen)
{
    uint32_t localVal = REG_RPL_RD(MDM_RXFE_CNTL_ADDR);

    *fedccen = (localVal & ((uint32_t)0x00000002)) >> 1;
    *fefocen = (localVal & ((uint32_t)0x00000001)) >> 0;
}

__STATIC_INLINE uint8_t mdm_fe_dccen_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_RXFE_CNTL_ADDR);
    return ((localVal & ((uint32_t)0x00000002)) >> 1);
}

__STATIC_INLINE void mdm_fe_dccen_setf(uint8_t fedccen)
{
    ASSERT_ERR((((uint32_t)fedccen << 1) & ~((uint32_t)0x00000002)) == 0);
    REG_RPL_WR(MDM_RXFE_CNTL_ADDR, (REG_RPL_RD(MDM_RXFE_CNTL_ADDR) & ~((uint32_t)0x00000002)) | ((uint32_t)fedccen << 1));
}

__STATIC_INLINE uint8_t mdm_fe_focen_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_RXFE_CNTL_ADDR);
    return ((localVal & ((uint32_t)0x00000001)) >> 0);
}

__STATIC_INLINE void mdm_fe_focen_setf(uint8_t fefocen)
{
    ASSERT_ERR((((uint32_t)fefocen << 0) & ~((uint32_t)0x00000001)) == 0);
    REG_RPL_WR(MDM_RXFE_CNTL_ADDR, (REG_RPL_RD(MDM_RXFE_CNTL_ADDR) & ~((uint32_t)0x00000001)) | ((uint32_t)fefocen << 0));
}

/**
 * @brief FCS_IFMHZ register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  12:00           FE_IFSHIFT   0x276
 * </pre>
 */
#define MDM_FCS_IFMHZ_ADDR   0x00000091
#define MDM_FCS_IFMHZ_OFFSET 0x00000091
#define MDM_FCS_IFMHZ_INDEX  0x00000024
#define MDM_FCS_IFMHZ_RESET  0x00000276

__STATIC_INLINE uint32_t mdm_fcs_ifmhz_get(void)
{
    return REG_RPL_RD(MDM_FCS_IFMHZ_ADDR);
}

__STATIC_INLINE void mdm_fcs_ifmhz_set(uint32_t value)
{
    REG_RPL_WR(MDM_FCS_IFMHZ_ADDR, value);
}

// field definitions
#define MDM_FE_IFSHIFT_MASK   ((uint32_t)0x00001FFF)
#define MDM_FE_IFSHIFT_LSB    0
#define MDM_FE_IFSHIFT_WIDTH  ((uint32_t)0x0000000D)

#define MDM_FE_IFSHIFT_RST    0x276

__STATIC_INLINE uint16_t mdm_fe_ifshift_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_FCS_IFMHZ_ADDR);
    ASSERT_ERR((localVal & ~((uint32_t)0x00001FFF)) == 0);
    return (localVal >> 0);
}

__STATIC_INLINE void mdm_fe_ifshift_setf(uint16_t feifshift)
{
    ASSERT_ERR((((uint32_t)feifshift << 0) & ~((uint32_t)0x00001FFF)) == 0);
    REG_RPL_WR(MDM_FCS_IFMHZ_ADDR, (uint32_t)feifshift << 0);
}

/**
 * @brief RXGFSK_CNTL register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *     07      LE_DFE_FORCERAW   1
 *     03      BT_DFE_FORCERAW   0
 *     02             GFO_ENPL   1
 *     01             GFO_ENSW   1
 *     00            PSD_AVGEN   1
 * </pre>
 */
#define MDM_RXGFSK_CNTL_ADDR   0x000000A0
#define MDM_RXGFSK_CNTL_OFFSET 0x000000A0
#define MDM_RXGFSK_CNTL_INDEX  0x00000028
#define MDM_RXGFSK_CNTL_RESET  0x00000087

__STATIC_INLINE uint32_t mdm_rxgfsk_cntl_get(void)
{
    return REG_RPL_RD(MDM_RXGFSK_CNTL_ADDR);
}

__STATIC_INLINE void mdm_rxgfsk_cntl_set(uint32_t value)
{
    REG_RPL_WR(MDM_RXGFSK_CNTL_ADDR, value);
}

// field definitions
#define MDM_LE_DFE_FORCERAW_BIT    ((uint32_t)0x00000080)
#define MDM_LE_DFE_FORCERAW_POS    7
#define MDM_BT_DFE_FORCERAW_BIT    ((uint32_t)0x00000008)
#define MDM_BT_DFE_FORCERAW_POS    3
#define MDM_GFO_ENPL_BIT           ((uint32_t)0x00000004)
#define MDM_GFO_ENPL_POS           2
#define MDM_GFO_ENSW_BIT           ((uint32_t)0x00000002)
#define MDM_GFO_ENSW_POS           1
#define MDM_PSD_AVGEN_BIT          ((uint32_t)0x00000001)
#define MDM_PSD_AVGEN_POS          0

#define MDM_LE_DFE_FORCERAW_RST    0x1
#define MDM_BT_DFE_FORCERAW_RST    0x0
#define MDM_GFO_ENPL_RST           0x1
#define MDM_GFO_ENSW_RST           0x1
#define MDM_PSD_AVGEN_RST          0x1

__STATIC_INLINE void mdm_rxgfsk_cntl_pack(uint8_t ledfeforceraw, uint8_t btdfeforceraw, uint8_t gfoenpl, uint8_t gfoensw, uint8_t psdavgen)
{
    ASSERT_ERR((((uint32_t)ledfeforceraw << 7) & ~((uint32_t)0x00000080)) == 0);
    ASSERT_ERR((((uint32_t)btdfeforceraw << 3) & ~((uint32_t)0x00000008)) == 0);
    ASSERT_ERR((((uint32_t)gfoenpl << 2) & ~((uint32_t)0x00000004)) == 0);
    ASSERT_ERR((((uint32_t)gfoensw << 1) & ~((uint32_t)0x00000002)) == 0);
    ASSERT_ERR((((uint32_t)psdavgen << 0) & ~((uint32_t)0x00000001)) == 0);
    REG_RPL_WR(MDM_RXGFSK_CNTL_ADDR,  ((uint32_t)ledfeforceraw << 7) | ((uint32_t)btdfeforceraw << 3) | ((uint32_t)gfoenpl << 2) | ((uint32_t)gfoensw << 1) | ((uint32_t)psdavgen << 0));
}

__STATIC_INLINE void mdm_rxgfsk_cntl_unpack(uint8_t* ledfeforceraw, uint8_t* btdfeforceraw, uint8_t* gfoenpl, uint8_t* gfoensw, uint8_t* psdavgen)
{
    uint32_t localVal = REG_RPL_RD(MDM_RXGFSK_CNTL_ADDR);

    *ledfeforceraw = (localVal & ((uint32_t)0x00000080)) >> 7;
    *btdfeforceraw = (localVal & ((uint32_t)0x00000008)) >> 3;
    *gfoenpl = (localVal & ((uint32_t)0x00000004)) >> 2;
    *gfoensw = (localVal & ((uint32_t)0x00000002)) >> 1;
    *psdavgen = (localVal & ((uint32_t)0x00000001)) >> 0;
}

__STATIC_INLINE uint8_t mdm_le_dfe_forceraw_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_RXGFSK_CNTL_ADDR);
    return ((localVal & ((uint32_t)0x00000080)) >> 7);
}

__STATIC_INLINE void mdm_le_dfe_forceraw_setf(uint8_t ledfeforceraw)
{
    ASSERT_ERR((((uint32_t)ledfeforceraw << 7) & ~((uint32_t)0x00000080)) == 0);
    REG_RPL_WR(MDM_RXGFSK_CNTL_ADDR, (REG_RPL_RD(MDM_RXGFSK_CNTL_ADDR) & ~((uint32_t)0x00000080)) | ((uint32_t)ledfeforceraw << 7));
}

__STATIC_INLINE uint8_t mdm_bt_dfe_forceraw_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_RXGFSK_CNTL_ADDR);
    return ((localVal & ((uint32_t)0x00000008)) >> 3);
}

__STATIC_INLINE void mdm_bt_dfe_forceraw_setf(uint8_t btdfeforceraw)
{
    ASSERT_ERR((((uint32_t)btdfeforceraw << 3) & ~((uint32_t)0x00000008)) == 0);
    REG_RPL_WR(MDM_RXGFSK_CNTL_ADDR, (REG_RPL_RD(MDM_RXGFSK_CNTL_ADDR) & ~((uint32_t)0x00000008)) | ((uint32_t)btdfeforceraw << 3));
}

__STATIC_INLINE uint8_t mdm_gfo_enpl_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_RXGFSK_CNTL_ADDR);
    return ((localVal & ((uint32_t)0x00000004)) >> 2);
}

__STATIC_INLINE void mdm_gfo_enpl_setf(uint8_t gfoenpl)
{
    ASSERT_ERR((((uint32_t)gfoenpl << 2) & ~((uint32_t)0x00000004)) == 0);
    REG_RPL_WR(MDM_RXGFSK_CNTL_ADDR, (REG_RPL_RD(MDM_RXGFSK_CNTL_ADDR) & ~((uint32_t)0x00000004)) | ((uint32_t)gfoenpl << 2));
}

__STATIC_INLINE uint8_t mdm_gfo_ensw_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_RXGFSK_CNTL_ADDR);
    return ((localVal & ((uint32_t)0x00000002)) >> 1);
}

__STATIC_INLINE void mdm_gfo_ensw_setf(uint8_t gfoensw)
{
    ASSERT_ERR((((uint32_t)gfoensw << 1) & ~((uint32_t)0x00000002)) == 0);
    REG_RPL_WR(MDM_RXGFSK_CNTL_ADDR, (REG_RPL_RD(MDM_RXGFSK_CNTL_ADDR) & ~((uint32_t)0x00000002)) | ((uint32_t)gfoensw << 1));
}

__STATIC_INLINE uint8_t mdm_psd_avgen_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_RXGFSK_CNTL_ADDR);
    return ((localVal & ((uint32_t)0x00000001)) >> 0);
}

__STATIC_INLINE void mdm_psd_avgen_setf(uint8_t psdavgen)
{
    ASSERT_ERR((((uint32_t)psdavgen << 0) & ~((uint32_t)0x00000001)) == 0);
    REG_RPL_WR(MDM_RXGFSK_CNTL_ADDR, (REG_RPL_RD(MDM_RXGFSK_CNTL_ADDR) & ~((uint32_t)0x00000001)) | ((uint32_t)psdavgen << 0));
}

/**
 * @brief GFO_P2PTHR register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  09:00           GFO_P2PTHR   0x100
 * </pre>
 */
#define MDM_GFO_P2PTHR_ADDR   0x000000A1
#define MDM_GFO_P2PTHR_OFFSET 0x000000A1
#define MDM_GFO_P2PTHR_INDEX  0x00000028
#define MDM_GFO_P2PTHR_RESET  0x00000100

__STATIC_INLINE uint32_t mdm_gfo_p2pthr_get(void)
{
    return REG_RPL_RD(MDM_GFO_P2PTHR_ADDR);
}

__STATIC_INLINE void mdm_gfo_p2pthr_set(uint32_t value)
{
    REG_RPL_WR(MDM_GFO_P2PTHR_ADDR, value);
}

// field definitions
#define MDM_GFO_P2PTHR_MASK   ((uint32_t)0x000003FF)
#define MDM_GFO_P2PTHR_LSB    0
#define MDM_GFO_P2PTHR_WIDTH  ((uint32_t)0x0000000A)

#define MDM_GFO_P2PTHR_RST    0x100

__STATIC_INLINE uint16_t mdm_gfo_p2pthr_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_GFO_P2PTHR_ADDR);
    ASSERT_ERR((localVal & ~((uint32_t)0x000003FF)) == 0);
    return (localVal >> 0);
}

__STATIC_INLINE void mdm_gfo_p2pthr_setf(uint16_t gfop2pthr)
{
    ASSERT_ERR((((uint32_t)gfop2pthr << 0) & ~((uint32_t)0x000003FF)) == 0);
    REG_RPL_WR(MDM_GFO_P2PTHR_ADDR, (uint32_t)gfop2pthr << 0);
}

/**
 * @brief GFO_REFINIT register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  10:00          GFO_REFINIT   0x6CD
 * </pre>
 */
#define MDM_GFO_REFINIT_ADDR   0x000000A2
#define MDM_GFO_REFINIT_OFFSET 0x000000A2
#define MDM_GFO_REFINIT_INDEX  0x00000028
#define MDM_GFO_REFINIT_RESET  0x000006CD

__STATIC_INLINE uint32_t mdm_gfo_refinit_get(void)
{
    return REG_RPL_RD(MDM_GFO_REFINIT_ADDR);
}

__STATIC_INLINE void mdm_gfo_refinit_set(uint32_t value)
{
    REG_RPL_WR(MDM_GFO_REFINIT_ADDR, value);
}

// field definitions
#define MDM_GFO_REFINIT_MASK   ((uint32_t)0x000007FF)
#define MDM_GFO_REFINIT_LSB    0
#define MDM_GFO_REFINIT_WIDTH  ((uint32_t)0x0000000B)

#define MDM_GFO_REFINIT_RST    0x6CD

__STATIC_INLINE uint16_t mdm_gfo_refinit_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_GFO_REFINIT_ADDR);
    ASSERT_ERR((localVal & ~((uint32_t)0x000007FF)) == 0);
    return (localVal >> 0);
}

__STATIC_INLINE void mdm_gfo_refinit_setf(uint16_t gforefinit)
{
    ASSERT_ERR((((uint32_t)gforefinit << 0) & ~((uint32_t)0x000007FF)) == 0);
    REG_RPL_WR(MDM_GFO_REFINIT_ADDR, (uint32_t)gforefinit << 0);
}

/**
 * @brief GFO_GFSKDETECT register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  10:00       GFO_GFSKDETECT   0x12C
 * </pre>
 */
#define MDM_GFO_GFSKDETECT_ADDR   0x000000A3
#define MDM_GFO_GFSKDETECT_OFFSET 0x000000A3
#define MDM_GFO_GFSKDETECT_INDEX  0x00000028
#define MDM_GFO_GFSKDETECT_RESET  0x0000012C

__STATIC_INLINE uint32_t mdm_gfo_gfskdetect_get(void)
{
    return REG_RPL_RD(MDM_GFO_GFSKDETECT_ADDR);
}

__STATIC_INLINE void mdm_gfo_gfskdetect_set(uint32_t value)
{
    REG_RPL_WR(MDM_GFO_GFSKDETECT_ADDR, value);
}

// field definitions
#define MDM_GFO_GFSKDETECT_MASK   ((uint32_t)0x000007FF)
#define MDM_GFO_GFSKDETECT_LSB    0
#define MDM_GFO_GFSKDETECT_WIDTH  ((uint32_t)0x0000000B)

#define MDM_GFO_GFSKDETECT_RST    0x12C

__STATIC_INLINE uint16_t mdm_gfo_gfskdetect_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_GFO_GFSKDETECT_ADDR);
    ASSERT_ERR((localVal & ~((uint32_t)0x000007FF)) == 0);
    return (localVal >> 0);
}

__STATIC_INLINE void mdm_gfo_gfskdetect_setf(uint16_t gfogfskdetect)
{
    ASSERT_ERR((((uint32_t)gfogfskdetect << 0) & ~((uint32_t)0x000007FF)) == 0);
    REG_RPL_WR(MDM_GFO_GFSKDETECT_ADDR, (uint32_t)gfogfskdetect << 0);
}

/**
 * @brief GFO_SETKDELSW register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  06:00        GFO_SETKDELSW   0x14
 * </pre>
 */
#define MDM_GFO_SETKDELSW_ADDR   0x000000A4
#define MDM_GFO_SETKDELSW_OFFSET 0x000000A4
#define MDM_GFO_SETKDELSW_INDEX  0x00000029
#define MDM_GFO_SETKDELSW_RESET  0x00000014

__STATIC_INLINE uint32_t mdm_gfo_setkdelsw_get(void)
{
    return REG_RPL_RD(MDM_GFO_SETKDELSW_ADDR);
}

__STATIC_INLINE void mdm_gfo_setkdelsw_set(uint32_t value)
{
    REG_RPL_WR(MDM_GFO_SETKDELSW_ADDR, value);
}

// field definitions
#define MDM_GFO_SETKDELSW_MASK   ((uint32_t)0x0000007F)
#define MDM_GFO_SETKDELSW_LSB    0
#define MDM_GFO_SETKDELSW_WIDTH  ((uint32_t)0x00000007)

#define MDM_GFO_SETKDELSW_RST    0x14

__STATIC_INLINE uint8_t mdm_gfo_setkdelsw_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_GFO_SETKDELSW_ADDR);
    ASSERT_ERR((localVal & ~((uint32_t)0x0000007F)) == 0);
    return (localVal >> 0);
}

__STATIC_INLINE void mdm_gfo_setkdelsw_setf(uint8_t gfosetkdelsw)
{
    ASSERT_ERR((((uint32_t)gfosetkdelsw << 0) & ~((uint32_t)0x0000007F)) == 0);
    REG_RPL_WR(MDM_GFO_SETKDELSW_ADDR, (uint32_t)gfosetkdelsw << 0);
}

/**
 * @brief GFO_SETKDELPL register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  06:00        GFO_SETKDELPL   0x14
 * </pre>
 */
#define MDM_GFO_SETKDELPL_ADDR   0x000000A5
#define MDM_GFO_SETKDELPL_OFFSET 0x000000A5
#define MDM_GFO_SETKDELPL_INDEX  0x00000029
#define MDM_GFO_SETKDELPL_RESET  0x00000014

__STATIC_INLINE uint32_t mdm_gfo_setkdelpl_get(void)
{
    return REG_RPL_RD(MDM_GFO_SETKDELPL_ADDR);
}

__STATIC_INLINE void mdm_gfo_setkdelpl_set(uint32_t value)
{
    REG_RPL_WR(MDM_GFO_SETKDELPL_ADDR, value);
}

// field definitions
#define MDM_GFO_SETKDELPL_MASK   ((uint32_t)0x0000007F)
#define MDM_GFO_SETKDELPL_LSB    0
#define MDM_GFO_SETKDELPL_WIDTH  ((uint32_t)0x00000007)

#define MDM_GFO_SETKDELPL_RST    0x14

__STATIC_INLINE uint8_t mdm_gfo_setkdelpl_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_GFO_SETKDELPL_ADDR);
    ASSERT_ERR((localVal & ~((uint32_t)0x0000007F)) == 0);
    return (localVal >> 0);
}

__STATIC_INLINE void mdm_gfo_setkdelpl_setf(uint8_t gfosetkdelpl)
{
    ASSERT_ERR((((uint32_t)gfosetkdelpl << 0) & ~((uint32_t)0x0000007F)) == 0);
    REG_RPL_WR(MDM_GFO_SETKDELPL_ADDR, (uint32_t)gfosetkdelpl << 0);
}

/**
 * @brief GFO_CONVDEL register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  06:00          GFO_CONVDEL   0x1E
 * </pre>
 */
#define MDM_GFO_CONVDEL_ADDR   0x000000A6
#define MDM_GFO_CONVDEL_OFFSET 0x000000A6
#define MDM_GFO_CONVDEL_INDEX  0x00000029
#define MDM_GFO_CONVDEL_RESET  0x0000001E

__STATIC_INLINE uint32_t mdm_gfo_convdel_get(void)
{
    return REG_RPL_RD(MDM_GFO_CONVDEL_ADDR);
}

__STATIC_INLINE void mdm_gfo_convdel_set(uint32_t value)
{
    REG_RPL_WR(MDM_GFO_CONVDEL_ADDR, value);
}

// field definitions
#define MDM_GFO_CONVDEL_MASK   ((uint32_t)0x0000007F)
#define MDM_GFO_CONVDEL_LSB    0
#define MDM_GFO_CONVDEL_WIDTH  ((uint32_t)0x00000007)

#define MDM_GFO_CONVDEL_RST    0x1E

__STATIC_INLINE uint8_t mdm_gfo_convdel_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_GFO_CONVDEL_ADDR);
    ASSERT_ERR((localVal & ~((uint32_t)0x0000007F)) == 0);
    return (localVal >> 0);
}

__STATIC_INLINE void mdm_gfo_convdel_setf(uint8_t gfoconvdel)
{
    ASSERT_ERR((((uint32_t)gfoconvdel << 0) & ~((uint32_t)0x0000007F)) == 0);
    REG_RPL_WR(MDM_GFO_CONVDEL_ADDR, (uint32_t)gfoconvdel << 0);
}

/**
 * @brief GFO_ESTSW register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  10:00            GFO_ESTSW   0x0
 * </pre>
 */
#define MDM_GFO_ESTSW_ADDR   0x000000A7
#define MDM_GFO_ESTSW_OFFSET 0x000000A7
#define MDM_GFO_ESTSW_INDEX  0x00000029
#define MDM_GFO_ESTSW_RESET  0x00000000

__STATIC_INLINE uint32_t mdm_gfo_estsw_get(void)
{
    return REG_RPL_RD(MDM_GFO_ESTSW_ADDR);
}

__STATIC_INLINE void mdm_gfo_estsw_set(uint32_t value)
{
    REG_RPL_WR(MDM_GFO_ESTSW_ADDR, value);
}

// field definitions
#define MDM_GFO_ESTSW_MASK   ((uint32_t)0x000007FF)
#define MDM_GFO_ESTSW_LSB    0
#define MDM_GFO_ESTSW_WIDTH  ((uint32_t)0x0000000B)

#define MDM_GFO_ESTSW_RST    0x0

__STATIC_INLINE uint16_t mdm_gfo_estsw_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_GFO_ESTSW_ADDR);
    ASSERT_ERR((localVal & ~((uint32_t)0x000007FF)) == 0);
    return (localVal >> 0);
}

/**
 * @brief GFO_ESTPL register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  10:00            GFO_ESTPL   0x0
 * </pre>
 */
#define MDM_GFO_ESTPL_ADDR   0x000000A8
#define MDM_GFO_ESTPL_OFFSET 0x000000A8
#define MDM_GFO_ESTPL_INDEX  0x0000002A
#define MDM_GFO_ESTPL_RESET  0x00000000

__STATIC_INLINE uint32_t mdm_gfo_estpl_get(void)
{
    return REG_RPL_RD(MDM_GFO_ESTPL_ADDR);
}

__STATIC_INLINE void mdm_gfo_estpl_set(uint32_t value)
{
    REG_RPL_WR(MDM_GFO_ESTPL_ADDR, value);
}

// field definitions
#define MDM_GFO_ESTPL_MASK   ((uint32_t)0x000007FF)
#define MDM_GFO_ESTPL_LSB    0
#define MDM_GFO_ESTPL_WIDTH  ((uint32_t)0x0000000B)

#define MDM_GFO_ESTPL_RST    0x0

__STATIC_INLINE uint16_t mdm_gfo_estpl_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_GFO_ESTPL_ADDR);
    ASSERT_ERR((localVal & ~((uint32_t)0x000007FF)) == 0);
    return (localVal >> 0);
}

/**
 * @brief GFO_INIT register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  25:16          GFO_INITDEL   0x34
 *  09:00        GFO_DPLATENCY   0x1E
 * </pre>
 */
#define MDM_GFO_INIT_ADDR   0x000000A9
#define MDM_GFO_INIT_OFFSET 0x000000A9
#define MDM_GFO_INIT_INDEX  0x0000002A
#define MDM_GFO_INIT_RESET  0x0034001E

__STATIC_INLINE uint32_t mdm_gfo_init_get(void)
{
    return REG_RPL_RD(MDM_GFO_INIT_ADDR);
}

__STATIC_INLINE void mdm_gfo_init_set(uint32_t value)
{
    REG_RPL_WR(MDM_GFO_INIT_ADDR, value);
}

// field definitions
#define MDM_GFO_INITDEL_MASK     ((uint32_t)0x03FF0000)
#define MDM_GFO_INITDEL_LSB      16
#define MDM_GFO_INITDEL_WIDTH    ((uint32_t)0x0000000A)
#define MDM_GFO_DPLATENCY_MASK   ((uint32_t)0x000003FF)
#define MDM_GFO_DPLATENCY_LSB    0
#define MDM_GFO_DPLATENCY_WIDTH  ((uint32_t)0x0000000A)

#define MDM_GFO_INITDEL_RST      0x34
#define MDM_GFO_DPLATENCY_RST    0x1E

__STATIC_INLINE void mdm_gfo_init_pack(uint16_t gfoinitdel, uint16_t gfodplatency)
{
    ASSERT_ERR((((uint32_t)gfoinitdel << 16) & ~((uint32_t)0x03FF0000)) == 0);
    ASSERT_ERR((((uint32_t)gfodplatency << 0) & ~((uint32_t)0x000003FF)) == 0);
    REG_RPL_WR(MDM_GFO_INIT_ADDR,  ((uint32_t)gfoinitdel << 16) | ((uint32_t)gfodplatency << 0));
}

__STATIC_INLINE void mdm_gfo_init_unpack(uint16_t* gfoinitdel, uint16_t* gfodplatency)
{
    uint32_t localVal = REG_RPL_RD(MDM_GFO_INIT_ADDR);

    *gfoinitdel = (localVal & ((uint32_t)0x03FF0000)) >> 16;
    *gfodplatency = (localVal & ((uint32_t)0x000003FF)) >> 0;
}

__STATIC_INLINE uint16_t mdm_gfo_initdel_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_GFO_INIT_ADDR);
    return ((localVal & ((uint32_t)0x03FF0000)) >> 16);
}

__STATIC_INLINE void mdm_gfo_initdel_setf(uint16_t gfoinitdel)
{
    ASSERT_ERR((((uint32_t)gfoinitdel << 16) & ~((uint32_t)0x03FF0000)) == 0);
    REG_RPL_WR(MDM_GFO_INIT_ADDR, (REG_RPL_RD(MDM_GFO_INIT_ADDR) & ~((uint32_t)0x03FF0000)) | ((uint32_t)gfoinitdel << 16));
}

__STATIC_INLINE uint16_t mdm_gfo_dplatency_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_GFO_INIT_ADDR);
    return ((localVal & ((uint32_t)0x000003FF)) >> 0);
}

__STATIC_INLINE void mdm_gfo_dplatency_setf(uint16_t gfodplatency)
{
    ASSERT_ERR((((uint32_t)gfodplatency << 0) & ~((uint32_t)0x000003FF)) == 0);
    REG_RPL_WR(MDM_GFO_INIT_ADDR, (REG_RPL_RD(MDM_GFO_INIT_ADDR) & ~((uint32_t)0x000003FF)) | ((uint32_t)gfodplatency << 0));
}

/**
 * @brief ACSYNCTUNE register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  03:00       DFE_ACSYNCTUNE   0x5
 * </pre>
 */
#define MDM_ACSYNCTUNE_ADDR   0x000000AA
#define MDM_ACSYNCTUNE_OFFSET 0x000000AA
#define MDM_ACSYNCTUNE_INDEX  0x0000002A
#define MDM_ACSYNCTUNE_RESET  0x00000005

__STATIC_INLINE uint32_t mdm_acsynctune_get(void)
{
    return REG_RPL_RD(MDM_ACSYNCTUNE_ADDR);
}

__STATIC_INLINE void mdm_acsynctune_set(uint32_t value)
{
    REG_RPL_WR(MDM_ACSYNCTUNE_ADDR, value);
}

// field definitions
#define MDM_DFE_ACSYNCTUNE_MASK   ((uint32_t)0x0000000F)
#define MDM_DFE_ACSYNCTUNE_LSB    0
#define MDM_DFE_ACSYNCTUNE_WIDTH  ((uint32_t)0x00000004)

#define MDM_DFE_ACSYNCTUNE_RST    0x5

__STATIC_INLINE uint8_t mdm_dfe_acsynctune_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_ACSYNCTUNE_ADDR);
    ASSERT_ERR((localVal & ~((uint32_t)0x0000000F)) == 0);
    return (localVal >> 0);
}

__STATIC_INLINE void mdm_dfe_acsynctune_setf(uint8_t dfeacsynctune)
{
    ASSERT_ERR((((uint32_t)dfeacsynctune << 0) & ~((uint32_t)0x0000000F)) == 0);
    REG_RPL_WR(MDM_ACSYNCTUNE_ADDR, (uint32_t)dfeacsynctune << 0);
}

/**
 * @brief PE_POWERTHR register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  05:00          PE_POWERTHR   0x0
 * </pre>
 */
#define MDM_PE_POWERTHR_ADDR   0x000000AB
#define MDM_PE_POWERTHR_OFFSET 0x000000AB
#define MDM_PE_POWERTHR_INDEX  0x0000002A
#define MDM_PE_POWERTHR_RESET  0x00000000

__STATIC_INLINE uint32_t mdm_pe_powerthr_get(void)
{
    return REG_RPL_RD(MDM_PE_POWERTHR_ADDR);
}

__STATIC_INLINE void mdm_pe_powerthr_set(uint32_t value)
{
    REG_RPL_WR(MDM_PE_POWERTHR_ADDR, value);
}

// field definitions
#define MDM_PE_POWERTHR_MASK   ((uint32_t)0x0000003F)
#define MDM_PE_POWERTHR_LSB    0
#define MDM_PE_POWERTHR_WIDTH  ((uint32_t)0x00000006)

#define MDM_PE_POWERTHR_RST    0x0

__STATIC_INLINE uint8_t mdm_pe_powerthr_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_PE_POWERTHR_ADDR);
    ASSERT_ERR((localVal & ~((uint32_t)0x0000003F)) == 0);
    return (localVal >> 0);
}

__STATIC_INLINE void mdm_pe_powerthr_setf(uint8_t pepowerthr)
{
    ASSERT_ERR((((uint32_t)pepowerthr << 0) & ~((uint32_t)0x0000003F)) == 0);
    REG_RPL_WR(MDM_PE_POWERTHR_ADDR, (uint32_t)pepowerthr << 0);
}

/**
 * @brief DPLATENCY_CNTL register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  17:16             DPL_TUNE   0x2
 *  09:00            GB_LENGTH   0x0
 * </pre>
 */
#define MDM_DPLATENCY_CNTL_ADDR   0x000000AC
#define MDM_DPLATENCY_CNTL_OFFSET 0x000000AC
#define MDM_DPLATENCY_CNTL_INDEX  0x0000002B
#define MDM_DPLATENCY_CNTL_RESET  0x00020000

__STATIC_INLINE uint32_t mdm_dplatency_cntl_get(void)
{
    return REG_RPL_RD(MDM_DPLATENCY_CNTL_ADDR);
}

__STATIC_INLINE void mdm_dplatency_cntl_set(uint32_t value)
{
    REG_RPL_WR(MDM_DPLATENCY_CNTL_ADDR, value);
}

// field definitions
#define MDM_DPL_TUNE_MASK    ((uint32_t)0x00030000)
#define MDM_DPL_TUNE_LSB     16
#define MDM_DPL_TUNE_WIDTH   ((uint32_t)0x00000002)
#define MDM_GB_LENGTH_MASK   ((uint32_t)0x000003FF)
#define MDM_GB_LENGTH_LSB    0
#define MDM_GB_LENGTH_WIDTH  ((uint32_t)0x0000000A)

#define MDM_DPL_TUNE_RST     0x2
#define MDM_GB_LENGTH_RST    0x0

__STATIC_INLINE void mdm_dplatency_cntl_unpack(uint8_t* dpltune, uint16_t* gblength)
{
    uint32_t localVal = REG_RPL_RD(MDM_DPLATENCY_CNTL_ADDR);

    *dpltune = (localVal & ((uint32_t)0x00030000)) >> 16;
    *gblength = (localVal & ((uint32_t)0x000003FF)) >> 0;
}

__STATIC_INLINE uint8_t mdm_dpl_tune_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_DPLATENCY_CNTL_ADDR);
    return ((localVal & ((uint32_t)0x00030000)) >> 16);
}

__STATIC_INLINE void mdm_dpl_tune_setf(uint8_t dpltune)
{
    ASSERT_ERR((((uint32_t)dpltune << 16) & ~((uint32_t)0x00030000)) == 0);
    REG_RPL_WR(MDM_DPLATENCY_CNTL_ADDR, (uint32_t)dpltune << 16);
}

__STATIC_INLINE uint16_t mdm_gb_length_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_DPLATENCY_CNTL_ADDR);
    return ((localVal & ((uint32_t)0x000003FF)) >> 0);
}

/**
 * @brief RXDPSK_CNTL register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  19:08             RRC_GAIN   0x0
 *     07           RRC_ENGAIN   1
 *     02               COC_EN   1
 *     01            DFD_ENFOT   1
 *     00               FOE_EN   1
 * </pre>
 */
#define MDM_RXDPSK_CNTL_ADDR   0x000000B0
#define MDM_RXDPSK_CNTL_OFFSET 0x000000B0
#define MDM_RXDPSK_CNTL_INDEX  0x0000002C
#define MDM_RXDPSK_CNTL_RESET  0x00000087

__STATIC_INLINE uint32_t mdm_rxdpsk_cntl_get(void)
{
    return REG_RPL_RD(MDM_RXDPSK_CNTL_ADDR);
}

__STATIC_INLINE void mdm_rxdpsk_cntl_set(uint32_t value)
{
    REG_RPL_WR(MDM_RXDPSK_CNTL_ADDR, value);
}

// field definitions
#define MDM_RRC_GAIN_MASK     ((uint32_t)0x000FFF00)
#define MDM_RRC_GAIN_LSB      8
#define MDM_RRC_GAIN_WIDTH    ((uint32_t)0x0000000C)
#define MDM_RRC_ENGAIN_BIT    ((uint32_t)0x00000080)
#define MDM_RRC_ENGAIN_POS    7
#define MDM_COC_EN_BIT        ((uint32_t)0x00000004)
#define MDM_COC_EN_POS        2
#define MDM_DFD_ENFOT_BIT     ((uint32_t)0x00000002)
#define MDM_DFD_ENFOT_POS     1
#define MDM_FOE_EN_BIT        ((uint32_t)0x00000001)
#define MDM_FOE_EN_POS        0

#define MDM_RRC_GAIN_RST      0x0
#define MDM_RRC_ENGAIN_RST    0x1
#define MDM_COC_EN_RST        0x1
#define MDM_DFD_ENFOT_RST     0x1
#define MDM_FOE_EN_RST        0x1

__STATIC_INLINE void mdm_rxdpsk_cntl_pack(uint16_t rrcgain, uint8_t rrcengain, uint8_t cocen, uint8_t dfdenfot, uint8_t foeen)
{
    ASSERT_ERR((((uint32_t)rrcgain << 8) & ~((uint32_t)0x000FFF00)) == 0);
    ASSERT_ERR((((uint32_t)rrcengain << 7) & ~((uint32_t)0x00000080)) == 0);
    ASSERT_ERR((((uint32_t)cocen << 2) & ~((uint32_t)0x00000004)) == 0);
    ASSERT_ERR((((uint32_t)dfdenfot << 1) & ~((uint32_t)0x00000002)) == 0);
    ASSERT_ERR((((uint32_t)foeen << 0) & ~((uint32_t)0x00000001)) == 0);
    REG_RPL_WR(MDM_RXDPSK_CNTL_ADDR,  ((uint32_t)rrcgain << 8) | ((uint32_t)rrcengain << 7) | ((uint32_t)cocen << 2) | ((uint32_t)dfdenfot << 1) | ((uint32_t)foeen << 0));
}

__STATIC_INLINE void mdm_rxdpsk_cntl_unpack(uint16_t* rrcgain, uint8_t* rrcengain, uint8_t* cocen, uint8_t* dfdenfot, uint8_t* foeen)
{
    uint32_t localVal = REG_RPL_RD(MDM_RXDPSK_CNTL_ADDR);

    *rrcgain = (localVal & ((uint32_t)0x000FFF00)) >> 8;
    *rrcengain = (localVal & ((uint32_t)0x00000080)) >> 7;
    *cocen = (localVal & ((uint32_t)0x00000004)) >> 2;
    *dfdenfot = (localVal & ((uint32_t)0x00000002)) >> 1;
    *foeen = (localVal & ((uint32_t)0x00000001)) >> 0;
}

__STATIC_INLINE uint16_t mdm_rrc_gain_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_RXDPSK_CNTL_ADDR);
    return ((localVal & ((uint32_t)0x000FFF00)) >> 8);
}

__STATIC_INLINE void mdm_rrc_gain_setf(uint16_t rrcgain)
{
    ASSERT_ERR((((uint32_t)rrcgain << 8) & ~((uint32_t)0x000FFF00)) == 0);
    REG_RPL_WR(MDM_RXDPSK_CNTL_ADDR, (REG_RPL_RD(MDM_RXDPSK_CNTL_ADDR) & ~((uint32_t)0x000FFF00)) | ((uint32_t)rrcgain << 8));
}

__STATIC_INLINE uint8_t mdm_rrc_engain_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_RXDPSK_CNTL_ADDR);
    return ((localVal & ((uint32_t)0x00000080)) >> 7);
}

__STATIC_INLINE void mdm_rrc_engain_setf(uint8_t rrcengain)
{
    ASSERT_ERR((((uint32_t)rrcengain << 7) & ~((uint32_t)0x00000080)) == 0);
    REG_RPL_WR(MDM_RXDPSK_CNTL_ADDR, (REG_RPL_RD(MDM_RXDPSK_CNTL_ADDR) & ~((uint32_t)0x00000080)) | ((uint32_t)rrcengain << 7));
}

__STATIC_INLINE uint8_t mdm_coc_en_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_RXDPSK_CNTL_ADDR);
    return ((localVal & ((uint32_t)0x00000004)) >> 2);
}

__STATIC_INLINE void mdm_coc_en_setf(uint8_t cocen)
{
    ASSERT_ERR((((uint32_t)cocen << 2) & ~((uint32_t)0x00000004)) == 0);
    REG_RPL_WR(MDM_RXDPSK_CNTL_ADDR, (REG_RPL_RD(MDM_RXDPSK_CNTL_ADDR) & ~((uint32_t)0x00000004)) | ((uint32_t)cocen << 2));
}

__STATIC_INLINE uint8_t mdm_dfd_enfot_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_RXDPSK_CNTL_ADDR);
    return ((localVal & ((uint32_t)0x00000002)) >> 1);
}

__STATIC_INLINE void mdm_dfd_enfot_setf(uint8_t dfdenfot)
{
    ASSERT_ERR((((uint32_t)dfdenfot << 1) & ~((uint32_t)0x00000002)) == 0);
    REG_RPL_WR(MDM_RXDPSK_CNTL_ADDR, (REG_RPL_RD(MDM_RXDPSK_CNTL_ADDR) & ~((uint32_t)0x00000002)) | ((uint32_t)dfdenfot << 1));
}

__STATIC_INLINE uint8_t mdm_foe_en_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_RXDPSK_CNTL_ADDR);
    return ((localVal & ((uint32_t)0x00000001)) >> 0);
}

__STATIC_INLINE void mdm_foe_en_setf(uint8_t foeen)
{
    ASSERT_ERR((((uint32_t)foeen << 0) & ~((uint32_t)0x00000001)) == 0);
    REG_RPL_WR(MDM_RXDPSK_CNTL_ADDR, (REG_RPL_RD(MDM_RXDPSK_CNTL_ADDR) & ~((uint32_t)0x00000001)) | ((uint32_t)foeen << 0));
}

/**
 * @brief TE_TIMEINIT register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  21:16      BEFORE_TIMEINIT   0x3F
 *  10:00          TE_TIMEINIT   0x302
 * </pre>
 */
#define MDM_TE_TIMEINIT_ADDR   0x000000B1
#define MDM_TE_TIMEINIT_OFFSET 0x000000B1
#define MDM_TE_TIMEINIT_INDEX  0x0000002C
#define MDM_TE_TIMEINIT_RESET  0x003F0302

__STATIC_INLINE uint32_t mdm_te_timeinit_get(void)
{
    return REG_RPL_RD(MDM_TE_TIMEINIT_ADDR);
}

__STATIC_INLINE void mdm_te_timeinit_set(uint32_t value)
{
    REG_RPL_WR(MDM_TE_TIMEINIT_ADDR, value);
}

// field definitions
#define MDM_BEFORE_TIMEINIT_MASK   ((uint32_t)0x003F0000)
#define MDM_BEFORE_TIMEINIT_LSB    16
#define MDM_BEFORE_TIMEINIT_WIDTH  ((uint32_t)0x00000006)
#define MDM_TE_TIMEINIT_MASK       ((uint32_t)0x000007FF)
#define MDM_TE_TIMEINIT_LSB        0
#define MDM_TE_TIMEINIT_WIDTH      ((uint32_t)0x0000000B)

#define MDM_BEFORE_TIMEINIT_RST    0x3F
#define MDM_TE_TIMEINIT_RST        0x302

__STATIC_INLINE void mdm_te_timeinit_pack(uint8_t beforetimeinit, uint16_t tetimeinit)
{
    ASSERT_ERR((((uint32_t)beforetimeinit << 16) & ~((uint32_t)0x003F0000)) == 0);
    ASSERT_ERR((((uint32_t)tetimeinit << 0) & ~((uint32_t)0x000007FF)) == 0);
    REG_RPL_WR(MDM_TE_TIMEINIT_ADDR,  ((uint32_t)beforetimeinit << 16) | ((uint32_t)tetimeinit << 0));
}

__STATIC_INLINE void mdm_te_timeinit_unpack(uint8_t* beforetimeinit, uint16_t* tetimeinit)
{
    uint32_t localVal = REG_RPL_RD(MDM_TE_TIMEINIT_ADDR);

    *beforetimeinit = (localVal & ((uint32_t)0x003F0000)) >> 16;
    *tetimeinit = (localVal & ((uint32_t)0x000007FF)) >> 0;
}

__STATIC_INLINE uint8_t mdm_before_timeinit_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_TE_TIMEINIT_ADDR);
    return ((localVal & ((uint32_t)0x003F0000)) >> 16);
}

__STATIC_INLINE void mdm_before_timeinit_setf(uint8_t beforetimeinit)
{
    ASSERT_ERR((((uint32_t)beforetimeinit << 16) & ~((uint32_t)0x003F0000)) == 0);
    REG_RPL_WR(MDM_TE_TIMEINIT_ADDR, (REG_RPL_RD(MDM_TE_TIMEINIT_ADDR) & ~((uint32_t)0x003F0000)) | ((uint32_t)beforetimeinit << 16));
}

__STATIC_INLINE uint16_t mdm_te_timeinit_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_TE_TIMEINIT_ADDR);
    return ((localVal & ((uint32_t)0x000007FF)) >> 0);
}

__STATIC_INLINE void mdm_te_timeinit_setf(uint16_t tetimeinit)
{
    ASSERT_ERR((((uint32_t)tetimeinit << 0) & ~((uint32_t)0x000007FF)) == 0);
    REG_RPL_WR(MDM_TE_TIMEINIT_ADDR, (REG_RPL_RD(MDM_TE_TIMEINIT_ADDR) & ~((uint32_t)0x000007FF)) | ((uint32_t)tetimeinit << 0));
}

/**
 * @brief DFD_KFACTOR register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  08:00          DFD_KFACTOR   0x33
 * </pre>
 */
#define MDM_DFD_KFACTOR_ADDR   0x000000B2
#define MDM_DFD_KFACTOR_OFFSET 0x000000B2
#define MDM_DFD_KFACTOR_INDEX  0x0000002C
#define MDM_DFD_KFACTOR_RESET  0x00000033

__STATIC_INLINE uint32_t mdm_dfd_kfactor_get(void)
{
    return REG_RPL_RD(MDM_DFD_KFACTOR_ADDR);
}

__STATIC_INLINE void mdm_dfd_kfactor_set(uint32_t value)
{
    REG_RPL_WR(MDM_DFD_KFACTOR_ADDR, value);
}

// field definitions
#define MDM_DFD_KFACTOR_MASK   ((uint32_t)0x000001FF)
#define MDM_DFD_KFACTOR_LSB    0
#define MDM_DFD_KFACTOR_WIDTH  ((uint32_t)0x00000009)

#define MDM_DFD_KFACTOR_RST    0x33

__STATIC_INLINE uint16_t mdm_dfd_kfactor_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_DFD_KFACTOR_ADDR);
    ASSERT_ERR((localVal & ~((uint32_t)0x000001FF)) == 0);
    return (localVal >> 0);
}

__STATIC_INLINE void mdm_dfd_kfactor_setf(uint16_t dfdkfactor)
{
    ASSERT_ERR((((uint32_t)dfdkfactor << 0) & ~((uint32_t)0x000001FF)) == 0);
    REG_RPL_WR(MDM_DFD_KFACTOR_ADDR, (uint32_t)dfdkfactor << 0);
}

/**
 * @brief COC_KFACTOR register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  09:00          COC_KFACTOR   0x5
 * </pre>
 */
#define MDM_COC_KFACTOR_ADDR   0x000000B3
#define MDM_COC_KFACTOR_OFFSET 0x000000B3
#define MDM_COC_KFACTOR_INDEX  0x0000002C
#define MDM_COC_KFACTOR_RESET  0x00000005

__STATIC_INLINE uint32_t mdm_coc_kfactor_get(void)
{
    return REG_RPL_RD(MDM_COC_KFACTOR_ADDR);
}

__STATIC_INLINE void mdm_coc_kfactor_set(uint32_t value)
{
    REG_RPL_WR(MDM_COC_KFACTOR_ADDR, value);
}

// field definitions
#define MDM_COC_KFACTOR_MASK   ((uint32_t)0x000003FF)
#define MDM_COC_KFACTOR_LSB    0
#define MDM_COC_KFACTOR_WIDTH  ((uint32_t)0x0000000A)

#define MDM_COC_KFACTOR_RST    0x5

__STATIC_INLINE uint16_t mdm_coc_kfactor_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_COC_KFACTOR_ADDR);
    ASSERT_ERR((localVal & ~((uint32_t)0x000003FF)) == 0);
    return (localVal >> 0);
}

__STATIC_INLINE void mdm_coc_kfactor_setf(uint16_t cockfactor)
{
    ASSERT_ERR((((uint32_t)cockfactor << 0) & ~((uint32_t)0x000003FF)) == 0);
    REG_RPL_WR(MDM_COC_KFACTOR_ADDR, (uint32_t)cockfactor << 0);
}

/**
 * @brief COC_THR register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  09:00              COC_THR   0x200
 * </pre>
 */
#define MDM_COC_THR_ADDR   0x000000B4
#define MDM_COC_THR_OFFSET 0x000000B4
#define MDM_COC_THR_INDEX  0x0000002D
#define MDM_COC_THR_RESET  0x00000200

__STATIC_INLINE uint32_t mdm_coc_thr_get(void)
{
    return REG_RPL_RD(MDM_COC_THR_ADDR);
}

__STATIC_INLINE void mdm_coc_thr_set(uint32_t value)
{
    REG_RPL_WR(MDM_COC_THR_ADDR, value);
}

// field definitions
#define MDM_COC_THR_MASK   ((uint32_t)0x000003FF)
#define MDM_COC_THR_LSB    0
#define MDM_COC_THR_WIDTH  ((uint32_t)0x0000000A)

#define MDM_COC_THR_RST    0x200

__STATIC_INLINE uint16_t mdm_coc_thr_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_COC_THR_ADDR);
    ASSERT_ERR((localVal & ~((uint32_t)0x000003FF)) == 0);
    return (localVal >> 0);
}

__STATIC_INLINE void mdm_coc_thr_setf(uint16_t cocthr)
{
    ASSERT_ERR((((uint32_t)cocthr << 0) & ~((uint32_t)0x000003FF)) == 0);
    REG_RPL_WR(MDM_COC_THR_ADDR, (uint32_t)cocthr << 0);
}

/**
 * @brief COC_TD register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  08:00               COC_TD   0x32
 * </pre>
 */
#define MDM_COC_TD_ADDR   0x000000B5
#define MDM_COC_TD_OFFSET 0x000000B5
#define MDM_COC_TD_INDEX  0x0000002D
#define MDM_COC_TD_RESET  0x00000032

__STATIC_INLINE uint32_t mdm_coc_td_get(void)
{
    return REG_RPL_RD(MDM_COC_TD_ADDR);
}

__STATIC_INLINE void mdm_coc_td_set(uint32_t value)
{
    REG_RPL_WR(MDM_COC_TD_ADDR, value);
}

// field definitions
#define MDM_COC_TD_MASK   ((uint32_t)0x000001FF)
#define MDM_COC_TD_LSB    0
#define MDM_COC_TD_WIDTH  ((uint32_t)0x00000009)

#define MDM_COC_TD_RST    0x32

__STATIC_INLINE uint16_t mdm_coc_td_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_COC_TD_ADDR);
    ASSERT_ERR((localVal & ~((uint32_t)0x000001FF)) == 0);
    return (localVal >> 0);
}

__STATIC_INLINE void mdm_coc_td_setf(uint16_t coctd)
{
    ASSERT_ERR((((uint32_t)coctd << 0) & ~((uint32_t)0x000001FF)) == 0);
    REG_RPL_WR(MDM_COC_TD_ADDR, (uint32_t)coctd << 0);
}

/**
 * @brief COC_TENABLE register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  07:00          COC_TENABLE   0x19
 * </pre>
 */
#define MDM_COC_TENABLE_ADDR   0x000000B6
#define MDM_COC_TENABLE_OFFSET 0x000000B6
#define MDM_COC_TENABLE_INDEX  0x0000002D
#define MDM_COC_TENABLE_RESET  0x00000019

__STATIC_INLINE uint32_t mdm_coc_tenable_get(void)
{
    return REG_RPL_RD(MDM_COC_TENABLE_ADDR);
}

__STATIC_INLINE void mdm_coc_tenable_set(uint32_t value)
{
    REG_RPL_WR(MDM_COC_TENABLE_ADDR, value);
}

// field definitions
#define MDM_COC_TENABLE_MASK   ((uint32_t)0x000000FF)
#define MDM_COC_TENABLE_LSB    0
#define MDM_COC_TENABLE_WIDTH  ((uint32_t)0x00000008)

#define MDM_COC_TENABLE_RST    0x19

__STATIC_INLINE uint8_t mdm_coc_tenable_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_COC_TENABLE_ADDR);
    ASSERT_ERR((localVal & ~((uint32_t)0x000000FF)) == 0);
    return (localVal >> 0);
}

__STATIC_INLINE void mdm_coc_tenable_setf(uint8_t coctenable)
{
    ASSERT_ERR((((uint32_t)coctenable << 0) & ~((uint32_t)0x000000FF)) == 0);
    REG_RPL_WR(MDM_COC_TENABLE_ADDR, (uint32_t)coctenable << 0);
}

/**
 * @brief POW_DIGGAINOFF register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  03:00        PE_DIGGAINOFF   0x0
 * </pre>
 */
#define MDM_POW_DIGGAINOFF_ADDR   0x000000B7
#define MDM_POW_DIGGAINOFF_OFFSET 0x000000B7
#define MDM_POW_DIGGAINOFF_INDEX  0x0000002D
#define MDM_POW_DIGGAINOFF_RESET  0x00000000

__STATIC_INLINE uint32_t mdm_pow_diggainoff_get(void)
{
    return REG_RPL_RD(MDM_POW_DIGGAINOFF_ADDR);
}

__STATIC_INLINE void mdm_pow_diggainoff_set(uint32_t value)
{
    REG_RPL_WR(MDM_POW_DIGGAINOFF_ADDR, value);
}

// field definitions
#define MDM_PE_DIGGAINOFF_MASK   ((uint32_t)0x0000000F)
#define MDM_PE_DIGGAINOFF_LSB    0
#define MDM_PE_DIGGAINOFF_WIDTH  ((uint32_t)0x00000004)

#define MDM_PE_DIGGAINOFF_RST    0x0

__STATIC_INLINE uint8_t mdm_pe_diggainoff_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_POW_DIGGAINOFF_ADDR);
    ASSERT_ERR((localVal & ~((uint32_t)0x0000000F)) == 0);
    return (localVal >> 0);
}

__STATIC_INLINE void mdm_pe_diggainoff_setf(uint8_t pediggainoff)
{
    ASSERT_ERR((((uint32_t)pediggainoff << 0) & ~((uint32_t)0x0000000F)) == 0);
    REG_RPL_WR(MDM_POW_DIGGAINOFF_ADDR, (uint32_t)pediggainoff << 0);
}

/**
 * @brief FOE_STATUS register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  25:16             FOE_REAL   0x0
 *  09:00             FOE_IMAG   0x0
 * </pre>
 */
#define MDM_FOE_STATUS_ADDR   0x000000B8
#define MDM_FOE_STATUS_OFFSET 0x000000B8
#define MDM_FOE_STATUS_INDEX  0x0000002E
#define MDM_FOE_STATUS_RESET  0x00000000

__STATIC_INLINE uint32_t mdm_foe_status_get(void)
{
    return REG_RPL_RD(MDM_FOE_STATUS_ADDR);
}

__STATIC_INLINE void mdm_foe_status_set(uint32_t value)
{
    REG_RPL_WR(MDM_FOE_STATUS_ADDR, value);
}

// field definitions
#define MDM_FOE_REAL_MASK   ((uint32_t)0x03FF0000)
#define MDM_FOE_REAL_LSB    16
#define MDM_FOE_REAL_WIDTH  ((uint32_t)0x0000000A)
#define MDM_FOE_IMAG_MASK   ((uint32_t)0x000003FF)
#define MDM_FOE_IMAG_LSB    0
#define MDM_FOE_IMAG_WIDTH  ((uint32_t)0x0000000A)

#define MDM_FOE_REAL_RST    0x0
#define MDM_FOE_IMAG_RST    0x0

__STATIC_INLINE void mdm_foe_status_unpack(uint16_t* foereal, uint16_t* foeimag)
{
    uint32_t localVal = REG_RPL_RD(MDM_FOE_STATUS_ADDR);

    *foereal = (localVal & ((uint32_t)0x03FF0000)) >> 16;
    *foeimag = (localVal & ((uint32_t)0x000003FF)) >> 0;
}

__STATIC_INLINE uint16_t mdm_foe_real_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_FOE_STATUS_ADDR);
    return ((localVal & ((uint32_t)0x03FF0000)) >> 16);
}

__STATIC_INLINE uint16_t mdm_foe_imag_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_FOE_STATUS_ADDR);
    return ((localVal & ((uint32_t)0x000003FF)) >> 0);
}

/**
 * @brief TECOC_STATUS register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  02:00             TE_INDEX   0x0
 * </pre>
 */
#define MDM_TECOC_STATUS_ADDR   0x000000B9
#define MDM_TECOC_STATUS_OFFSET 0x000000B9
#define MDM_TECOC_STATUS_INDEX  0x0000002E
#define MDM_TECOC_STATUS_RESET  0x00000000

__STATIC_INLINE uint32_t mdm_tecoc_status_get(void)
{
    return REG_RPL_RD(MDM_TECOC_STATUS_ADDR);
}

__STATIC_INLINE void mdm_tecoc_status_set(uint32_t value)
{
    REG_RPL_WR(MDM_TECOC_STATUS_ADDR, value);
}

// field definitions
#define MDM_TE_INDEX_MASK   ((uint32_t)0x00000007)
#define MDM_TE_INDEX_LSB    0
#define MDM_TE_INDEX_WIDTH  ((uint32_t)0x00000003)

#define MDM_TE_INDEX_RST    0x0

__STATIC_INLINE uint8_t mdm_te_index_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_TECOC_STATUS_ADDR);
    ASSERT_ERR((localVal & ~((uint32_t)0x00000007)) == 0);
    return (localVal >> 0);
}

/**
 * @brief GSG_DEN register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  02:00              GSG_DEN   0x0
 * </pre>
 */
#define MDM_GSG_DEN_ADDR   0x000000C0
#define MDM_GSG_DEN_OFFSET 0x000000C0
#define MDM_GSG_DEN_INDEX  0x00000030
#define MDM_GSG_DEN_RESET  0x00000000

__STATIC_INLINE uint32_t mdm_gsg_den_get(void)
{
    return REG_RPL_RD(MDM_GSG_DEN_ADDR);
}

__STATIC_INLINE void mdm_gsg_den_set(uint32_t value)
{
    REG_RPL_WR(MDM_GSG_DEN_ADDR, value);
}

// field definitions
#define MDM_GSG_DEN_MASK   ((uint32_t)0x00000007)
#define MDM_GSG_DEN_LSB    0
#define MDM_GSG_DEN_WIDTH  ((uint32_t)0x00000003)

#define MDM_GSG_DEN_RST    0x0

__STATIC_INLINE uint8_t mdm_gsg_den_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_GSG_DEN_ADDR);
    ASSERT_ERR((localVal & ~((uint32_t)0x00000007)) == 0);
    return (localVal >> 0);
}

__STATIC_INLINE void mdm_gsg_den_setf(uint8_t gsgden)
{
    ASSERT_ERR((((uint32_t)gsgden << 0) & ~((uint32_t)0x00000007)) == 0);
    REG_RPL_WR(MDM_GSG_DEN_ADDR, (uint32_t)gsgden << 0);
}

/**
 * @brief GSG_LSTVAL register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  05:00           GSG_LSTVAL   0x14
 * </pre>
 */
#define MDM_GSG_LSTVAL_ADDR   0x000000C1
#define MDM_GSG_LSTVAL_OFFSET 0x000000C1
#define MDM_GSG_LSTVAL_INDEX  0x00000030
#define MDM_GSG_LSTVAL_RESET  0x00000014

__STATIC_INLINE uint32_t mdm_gsg_lstval_get(void)
{
    return REG_RPL_RD(MDM_GSG_LSTVAL_ADDR);
}

__STATIC_INLINE void mdm_gsg_lstval_set(uint32_t value)
{
    REG_RPL_WR(MDM_GSG_LSTVAL_ADDR, value);
}

// field definitions
#define MDM_GSG_LSTVAL_MASK   ((uint32_t)0x0000003F)
#define MDM_GSG_LSTVAL_LSB    0
#define MDM_GSG_LSTVAL_WIDTH  ((uint32_t)0x00000006)

#define MDM_GSG_LSTVAL_RST    0x14

__STATIC_INLINE uint8_t mdm_gsg_lstval_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_GSG_LSTVAL_ADDR);
    ASSERT_ERR((localVal & ~((uint32_t)0x0000003F)) == 0);
    return (localVal >> 0);
}

__STATIC_INLINE void mdm_gsg_lstval_setf(uint8_t gsglstval)
{
    ASSERT_ERR((((uint32_t)gsglstval << 0) & ~((uint32_t)0x0000003F)) == 0);
    REG_RPL_WR(MDM_GSG_LSTVAL_ADDR, (uint32_t)gsglstval << 0);
}

/**
 * @brief GSG_NOM register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  07:00              GSG_NOM   0x1
 * </pre>
 */
#define MDM_GSG_NOM_ADDR   0x000000C2
#define MDM_GSG_NOM_OFFSET 0x000000C2
#define MDM_GSG_NOM_INDEX  0x00000030
#define MDM_GSG_NOM_RESET  0x00000001

__STATIC_INLINE uint32_t mdm_gsg_nom_get(void)
{
    return REG_RPL_RD(MDM_GSG_NOM_ADDR);
}

__STATIC_INLINE void mdm_gsg_nom_set(uint32_t value)
{
    REG_RPL_WR(MDM_GSG_NOM_ADDR, value);
}

// field definitions
#define MDM_GSG_NOM_MASK   ((uint32_t)0x000000FF)
#define MDM_GSG_NOM_LSB    0
#define MDM_GSG_NOM_WIDTH  ((uint32_t)0x00000008)

#define MDM_GSG_NOM_RST    0x1

__STATIC_INLINE uint8_t mdm_gsg_nom_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_GSG_NOM_ADDR);
    ASSERT_ERR((localVal & ~((uint32_t)0x000000FF)) == 0);
    return (localVal >> 0);
}

__STATIC_INLINE void mdm_gsg_nom_setf(uint8_t gsgnom)
{
    ASSERT_ERR((((uint32_t)gsgnom << 0) & ~((uint32_t)0x000000FF)) == 0);
    REG_RPL_WR(MDM_GSG_NOM_ADDR, (uint32_t)gsgnom << 0);
}

/**
 * @brief GSG_THREPS register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  04:00           GSG_THREPS   0x7
 * </pre>
 */
#define MDM_GSG_THREPS_ADDR   0x000000C3
#define MDM_GSG_THREPS_OFFSET 0x000000C3
#define MDM_GSG_THREPS_INDEX  0x00000030
#define MDM_GSG_THREPS_RESET  0x00000007

__STATIC_INLINE uint32_t mdm_gsg_threps_get(void)
{
    return REG_RPL_RD(MDM_GSG_THREPS_ADDR);
}

__STATIC_INLINE void mdm_gsg_threps_set(uint32_t value)
{
    REG_RPL_WR(MDM_GSG_THREPS_ADDR, value);
}

// field definitions
#define MDM_GSG_THREPS_MASK   ((uint32_t)0x0000001F)
#define MDM_GSG_THREPS_LSB    0
#define MDM_GSG_THREPS_WIDTH  ((uint32_t)0x00000005)

#define MDM_GSG_THREPS_RST    0x7

__STATIC_INLINE uint8_t mdm_gsg_threps_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_GSG_THREPS_ADDR);
    ASSERT_ERR((localVal & ~((uint32_t)0x0000001F)) == 0);
    return (localVal >> 0);
}

__STATIC_INLINE void mdm_gsg_threps_setf(uint8_t gsgthreps)
{
    ASSERT_ERR((((uint32_t)gsgthreps << 0) & ~((uint32_t)0x0000001F)) == 0);
    REG_RPL_WR(MDM_GSG_THREPS_ADDR, (uint32_t)gsgthreps << 0);
}

/**
 * @brief GSG_VCO_DEN register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  04:00          GSG_VCO_DEN   0x6
 * </pre>
 */
#define MDM_GSG_VCO_DEN_ADDR   0x000000C4
#define MDM_GSG_VCO_DEN_OFFSET 0x000000C4
#define MDM_GSG_VCO_DEN_INDEX  0x00000031
#define MDM_GSG_VCO_DEN_RESET  0x00000006

__STATIC_INLINE uint32_t mdm_gsg_vco_den_get(void)
{
    return REG_RPL_RD(MDM_GSG_VCO_DEN_ADDR);
}

__STATIC_INLINE void mdm_gsg_vco_den_set(uint32_t value)
{
    REG_RPL_WR(MDM_GSG_VCO_DEN_ADDR, value);
}

// field definitions
#define MDM_GSG_VCO_DEN_MASK   ((uint32_t)0x0000001F)
#define MDM_GSG_VCO_DEN_LSB    0
#define MDM_GSG_VCO_DEN_WIDTH  ((uint32_t)0x00000005)

#define MDM_GSG_VCO_DEN_RST    0x6

__STATIC_INLINE uint8_t mdm_gsg_vco_den_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_GSG_VCO_DEN_ADDR);
    ASSERT_ERR((localVal & ~((uint32_t)0x0000001F)) == 0);
    return (localVal >> 0);
}

__STATIC_INLINE void mdm_gsg_vco_den_setf(uint8_t gsgvcoden)
{
    ASSERT_ERR((((uint32_t)gsgvcoden << 0) & ~((uint32_t)0x0000001F)) == 0);
    REG_RPL_WR(MDM_GSG_VCO_DEN_ADDR, (uint32_t)gsgvcoden << 0);
}

/**
 * @brief GSG_VCO_NOM register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  07:00          GSG_VCO_NOM   0x27
 * </pre>
 */
#define MDM_GSG_VCO_NOM_ADDR   0x000000C5
#define MDM_GSG_VCO_NOM_OFFSET 0x000000C5
#define MDM_GSG_VCO_NOM_INDEX  0x00000031
#define MDM_GSG_VCO_NOM_RESET  0x00000027

__STATIC_INLINE uint32_t mdm_gsg_vco_nom_get(void)
{
    return REG_RPL_RD(MDM_GSG_VCO_NOM_ADDR);
}

__STATIC_INLINE void mdm_gsg_vco_nom_set(uint32_t value)
{
    REG_RPL_WR(MDM_GSG_VCO_NOM_ADDR, value);
}

// field definitions
#define MDM_GSG_VCO_NOM_MASK   ((uint32_t)0x000000FF)
#define MDM_GSG_VCO_NOM_LSB    0
#define MDM_GSG_VCO_NOM_WIDTH  ((uint32_t)0x00000008)

#define MDM_GSG_VCO_NOM_RST    0x27

__STATIC_INLINE uint8_t mdm_gsg_vco_nom_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_GSG_VCO_NOM_ADDR);
    ASSERT_ERR((localVal & ~((uint32_t)0x000000FF)) == 0);
    return (localVal >> 0);
}

__STATIC_INLINE void mdm_gsg_vco_nom_setf(uint8_t gsgvconom)
{
    ASSERT_ERR((((uint32_t)gsgvconom << 0) & ~((uint32_t)0x000000FF)) == 0);
    REG_RPL_WR(MDM_GSG_VCO_NOM_ADDR, (uint32_t)gsgvconom << 0);
}

/**
 * @brief FM2PSW_LAT register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  03:00           FM2PSW_LAT   0x0
 * </pre>
 */
#define MDM_FM2PSW_LAT_ADDR   0x000000C6
#define MDM_FM2PSW_LAT_OFFSET 0x000000C6
#define MDM_FM2PSW_LAT_INDEX  0x00000031
#define MDM_FM2PSW_LAT_RESET  0x00000000

__STATIC_INLINE uint32_t mdm_fm2psw_lat_get(void)
{
    return REG_RPL_RD(MDM_FM2PSW_LAT_ADDR);
}

__STATIC_INLINE void mdm_fm2psw_lat_set(uint32_t value)
{
    REG_RPL_WR(MDM_FM2PSW_LAT_ADDR, value);
}

// field definitions
#define MDM_FM2PSW_LAT_MASK   ((uint32_t)0x0000000F)
#define MDM_FM2PSW_LAT_LSB    0
#define MDM_FM2PSW_LAT_WIDTH  ((uint32_t)0x00000004)

#define MDM_FM2PSW_LAT_RST    0x0

__STATIC_INLINE uint8_t mdm_fm2psw_lat_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_FM2PSW_LAT_ADDR);
    ASSERT_ERR((localVal & ~((uint32_t)0x0000000F)) == 0);
    return (localVal >> 0);
}

__STATIC_INLINE void mdm_fm2psw_lat_setf(uint8_t fm2pswlat)
{
    ASSERT_ERR((((uint32_t)fm2pswlat << 0) & ~((uint32_t)0x0000000F)) == 0);
    REG_RPL_WR(MDM_FM2PSW_LAT_ADDR, (uint32_t)fm2pswlat << 0);
}

/**
 * @brief GSG_DPHI_DEN register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  19:16      LE_GSG_DPHI_DEN   0x5
 *  03:00      BT_GSG_DPHI_DEN   0x0
 * </pre>
 */
#define MDM_GSG_DPHI_DEN_ADDR   0x000000C7
#define MDM_GSG_DPHI_DEN_OFFSET 0x000000C7
#define MDM_GSG_DPHI_DEN_INDEX  0x00000031
#define MDM_GSG_DPHI_DEN_RESET  0x00050000

__STATIC_INLINE uint32_t mdm_gsg_dphi_den_get(void)
{
    return REG_RPL_RD(MDM_GSG_DPHI_DEN_ADDR);
}

__STATIC_INLINE void mdm_gsg_dphi_den_set(uint32_t value)
{
    REG_RPL_WR(MDM_GSG_DPHI_DEN_ADDR, value);
}

// field definitions
#define MDM_LE_GSG_DPHI_DEN_MASK   ((uint32_t)0x000F0000)
#define MDM_LE_GSG_DPHI_DEN_LSB    16
#define MDM_LE_GSG_DPHI_DEN_WIDTH  ((uint32_t)0x00000004)
#define MDM_BT_GSG_DPHI_DEN_MASK   ((uint32_t)0x0000000F)
#define MDM_BT_GSG_DPHI_DEN_LSB    0
#define MDM_BT_GSG_DPHI_DEN_WIDTH  ((uint32_t)0x00000004)

#define MDM_LE_GSG_DPHI_DEN_RST    0x5
#define MDM_BT_GSG_DPHI_DEN_RST    0x0

__STATIC_INLINE void mdm_gsg_dphi_den_pack(uint8_t legsgdphiden, uint8_t btgsgdphiden)
{
    ASSERT_ERR((((uint32_t)legsgdphiden << 16) & ~((uint32_t)0x000F0000)) == 0);
    ASSERT_ERR((((uint32_t)btgsgdphiden << 0) & ~((uint32_t)0x0000000F)) == 0);
    REG_RPL_WR(MDM_GSG_DPHI_DEN_ADDR,  ((uint32_t)legsgdphiden << 16) | ((uint32_t)btgsgdphiden << 0));
}

__STATIC_INLINE void mdm_gsg_dphi_den_unpack(uint8_t* legsgdphiden, uint8_t* btgsgdphiden)
{
    uint32_t localVal = REG_RPL_RD(MDM_GSG_DPHI_DEN_ADDR);

    *legsgdphiden = (localVal & ((uint32_t)0x000F0000)) >> 16;
    *btgsgdphiden = (localVal & ((uint32_t)0x0000000F)) >> 0;
}

__STATIC_INLINE uint8_t mdm_le_gsg_dphi_den_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_GSG_DPHI_DEN_ADDR);
    return ((localVal & ((uint32_t)0x000F0000)) >> 16);
}

__STATIC_INLINE void mdm_le_gsg_dphi_den_setf(uint8_t legsgdphiden)
{
    ASSERT_ERR((((uint32_t)legsgdphiden << 16) & ~((uint32_t)0x000F0000)) == 0);
    REG_RPL_WR(MDM_GSG_DPHI_DEN_ADDR, (REG_RPL_RD(MDM_GSG_DPHI_DEN_ADDR) & ~((uint32_t)0x000F0000)) | ((uint32_t)legsgdphiden << 16));
}

__STATIC_INLINE uint8_t mdm_bt_gsg_dphi_den_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_GSG_DPHI_DEN_ADDR);
    return ((localVal & ((uint32_t)0x0000000F)) >> 0);
}

__STATIC_INLINE void mdm_bt_gsg_dphi_den_setf(uint8_t btgsgdphiden)
{
    ASSERT_ERR((((uint32_t)btgsgdphiden << 0) & ~((uint32_t)0x0000000F)) == 0);
    REG_RPL_WR(MDM_GSG_DPHI_DEN_ADDR, (REG_RPL_RD(MDM_GSG_DPHI_DEN_ADDR) & ~((uint32_t)0x0000000F)) | ((uint32_t)btgsgdphiden << 0));
}

/**
 * @brief GSG_DPHI_NOM register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  22:16      LE_GSG_DPHI_NOM   0x35
 *  06:00      BT_GSG_DPHI_NOM   0x1
 * </pre>
 */
#define MDM_GSG_DPHI_NOM_ADDR   0x000000C8
#define MDM_GSG_DPHI_NOM_OFFSET 0x000000C8
#define MDM_GSG_DPHI_NOM_INDEX  0x00000032
#define MDM_GSG_DPHI_NOM_RESET  0x00350001

__STATIC_INLINE uint32_t mdm_gsg_dphi_nom_get(void)
{
    return REG_RPL_RD(MDM_GSG_DPHI_NOM_ADDR);
}

__STATIC_INLINE void mdm_gsg_dphi_nom_set(uint32_t value)
{
    REG_RPL_WR(MDM_GSG_DPHI_NOM_ADDR, value);
}

// field definitions
#define MDM_LE_GSG_DPHI_NOM_MASK   ((uint32_t)0x007F0000)
#define MDM_LE_GSG_DPHI_NOM_LSB    16
#define MDM_LE_GSG_DPHI_NOM_WIDTH  ((uint32_t)0x00000007)
#define MDM_BT_GSG_DPHI_NOM_MASK   ((uint32_t)0x0000007F)
#define MDM_BT_GSG_DPHI_NOM_LSB    0
#define MDM_BT_GSG_DPHI_NOM_WIDTH  ((uint32_t)0x00000007)

#define MDM_LE_GSG_DPHI_NOM_RST    0x35
#define MDM_BT_GSG_DPHI_NOM_RST    0x1

__STATIC_INLINE void mdm_gsg_dphi_nom_pack(uint8_t legsgdphinom, uint8_t btgsgdphinom)
{
    ASSERT_ERR((((uint32_t)legsgdphinom << 16) & ~((uint32_t)0x007F0000)) == 0);
    ASSERT_ERR((((uint32_t)btgsgdphinom << 0) & ~((uint32_t)0x0000007F)) == 0);
    REG_RPL_WR(MDM_GSG_DPHI_NOM_ADDR,  ((uint32_t)legsgdphinom << 16) | ((uint32_t)btgsgdphinom << 0));
}

__STATIC_INLINE void mdm_gsg_dphi_nom_unpack(uint8_t* legsgdphinom, uint8_t* btgsgdphinom)
{
    uint32_t localVal = REG_RPL_RD(MDM_GSG_DPHI_NOM_ADDR);

    *legsgdphinom = (localVal & ((uint32_t)0x007F0000)) >> 16;
    *btgsgdphinom = (localVal & ((uint32_t)0x0000007F)) >> 0;
}

__STATIC_INLINE uint8_t mdm_le_gsg_dphi_nom_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_GSG_DPHI_NOM_ADDR);
    return ((localVal & ((uint32_t)0x007F0000)) >> 16);
}

__STATIC_INLINE void mdm_le_gsg_dphi_nom_setf(uint8_t legsgdphinom)
{
    ASSERT_ERR((((uint32_t)legsgdphinom << 16) & ~((uint32_t)0x007F0000)) == 0);
    REG_RPL_WR(MDM_GSG_DPHI_NOM_ADDR, (REG_RPL_RD(MDM_GSG_DPHI_NOM_ADDR) & ~((uint32_t)0x007F0000)) | ((uint32_t)legsgdphinom << 16));
}

__STATIC_INLINE uint8_t mdm_bt_gsg_dphi_nom_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_GSG_DPHI_NOM_ADDR);
    return ((localVal & ((uint32_t)0x0000007F)) >> 0);
}

__STATIC_INLINE void mdm_bt_gsg_dphi_nom_setf(uint8_t btgsgdphinom)
{
    ASSERT_ERR((((uint32_t)btgsgdphinom << 0) & ~((uint32_t)0x0000007F)) == 0);
    REG_RPL_WR(MDM_GSG_DPHI_NOM_ADDR, (REG_RPL_RD(MDM_GSG_DPHI_NOM_ADDR) & ~((uint32_t)0x0000007F)) | ((uint32_t)btgsgdphinom << 0));
}

/**
 * @brief DSG_DEN register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  03:00              DSG_DEN   0x0
 * </pre>
 */
#define MDM_DSG_DEN_ADDR   0x000000D0
#define MDM_DSG_DEN_OFFSET 0x000000D0
#define MDM_DSG_DEN_INDEX  0x00000034
#define MDM_DSG_DEN_RESET  0x00000000

__STATIC_INLINE uint32_t mdm_dsg_den_get(void)
{
    return REG_RPL_RD(MDM_DSG_DEN_ADDR);
}

__STATIC_INLINE void mdm_dsg_den_set(uint32_t value)
{
    REG_RPL_WR(MDM_DSG_DEN_ADDR, value);
}

// field definitions
#define MDM_DSG_DEN_MASK   ((uint32_t)0x0000000F)
#define MDM_DSG_DEN_LSB    0
#define MDM_DSG_DEN_WIDTH  ((uint32_t)0x00000004)

#define MDM_DSG_DEN_RST    0x0

__STATIC_INLINE uint8_t mdm_dsg_den_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_DSG_DEN_ADDR);
    ASSERT_ERR((localVal & ~((uint32_t)0x0000000F)) == 0);
    return (localVal >> 0);
}

__STATIC_INLINE void mdm_dsg_den_setf(uint8_t dsgden)
{
    ASSERT_ERR((((uint32_t)dsgden << 0) & ~((uint32_t)0x0000000F)) == 0);
    REG_RPL_WR(MDM_DSG_DEN_ADDR, (uint32_t)dsgden << 0);
}

/**
 * @brief DSG_NOM register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  09:00              DSG_NOM   0x1
 * </pre>
 */
#define MDM_DSG_NOM_ADDR   0x000000D1
#define MDM_DSG_NOM_OFFSET 0x000000D1
#define MDM_DSG_NOM_INDEX  0x00000034
#define MDM_DSG_NOM_RESET  0x00000001

__STATIC_INLINE uint32_t mdm_dsg_nom_get(void)
{
    return REG_RPL_RD(MDM_DSG_NOM_ADDR);
}

__STATIC_INLINE void mdm_dsg_nom_set(uint32_t value)
{
    REG_RPL_WR(MDM_DSG_NOM_ADDR, value);
}

// field definitions
#define MDM_DSG_NOM_MASK   ((uint32_t)0x000003FF)
#define MDM_DSG_NOM_LSB    0
#define MDM_DSG_NOM_WIDTH  ((uint32_t)0x0000000A)

#define MDM_DSG_NOM_RST    0x1

__STATIC_INLINE uint16_t mdm_dsg_nom_getf(void)
{
    uint32_t localVal = REG_RPL_RD(MDM_DSG_NOM_ADDR);
    ASSERT_ERR((localVal & ~((uint32_t)0x000003FF)) == 0);
    return (localVal >> 0);
}

__STATIC_INLINE void mdm_dsg_nom_setf(uint16_t dsgnom)
{
    ASSERT_ERR((((uint32_t)dsgnom << 0) & ~((uint32_t)0x000003FF)) == 0);
    REG_RPL_WR(MDM_DSG_NOM_ADDR, (uint32_t)dsgnom << 0);
}


#endif // _REG_MODEM_H_

