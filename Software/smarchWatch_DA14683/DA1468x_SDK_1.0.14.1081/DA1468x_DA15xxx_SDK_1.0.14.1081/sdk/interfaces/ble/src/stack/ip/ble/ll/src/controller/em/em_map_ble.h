/**
 ****************************************************************************************
 *
 * @file em_map_ble.h
 *
 * @brief Mapping of the different descriptors in the exchange memory
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 ****************************************************************************************
 */

#ifndef EM_MAP_BLE_H_
#define EM_MAP_BLE_H_

/**
 ****************************************************************************************
 * @addtogroup EM EM
 * @ingroup CONTROLLER
 * @brief Mapping of the different descriptors in the exchange memory
 *
 * @{
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"                // stack configuration

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "co_bt.h"
#include "co_buf.h"
#include "co_version.h"
#include "_reg_ble_em_tx_desc.h"
#include "_reg_ble_em_tx_buffer.h"
#include "_reg_ble_em_rx_desc.h"
#include "_reg_ble_em_rx_buffer.h"
#include "_reg_ble_em_wpb.h"
#include "_reg_ble_em_wpv.h"
#include "_reg_ble_em_cs.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// Size of the encryption area
#define EM_BLE_ENC_LEN         16

/// number of control structure entries for the exchange table
#define EM_BLE_CS_COUNT        (BLE_CONNECTION_MAX + 1)

/// number of tx descriptor entries for the exchange table
#if (RWBLE_SW_VERSION_MINOR < 1)
#define EM_BLE_TX_DESC_COUNT   (BLE_TX_DESC_CNT)
#else
#define EM_BLE_TX_DESC_COUNT   (_BLE_TX_DESC_CNT)
#endif /* (RWBLE_SW_VERSION_MINOR < 1) */

/// number of tx buffer entries for the exchange table
#define EM_BLE_TX_BUFFER_COUNT (BLE_TX_BUFFER_CNT)
#if (RWBLE_SW_VERSION_MINOR >= 1)
#define _EM_BLE_TX_BUFFER_COUNT (_BLE_TX_BUFFER_CNT)
#endif /* (RWBLE_SW_VERSION_MINOR >= 1) */

/// number of rx descriptor entries for the exchange table
#define EM_BLE_RX_DESC_COUNT   (BLE_RX_DESC_CNT)

/// number of rx buffer entries for the exchange table
#define EM_BLE_RX_BUFFER_COUNT (BLE_RX_BUFFER_CNT)

/*
 * Mapping of the different elements in EM
 ****************************************************************************************
 */
/// Offset of the plain data area (used for SW initiated encryption)
#define EM_BLE_ENC_PLAIN_OFFSET  (EM_BLE_OFFSET)
/// Offset of the ciphered data area (used for SW initiated encryption)
#define EM_BLE_ENC_CIPHER_OFFSET (EM_BLE_ENC_PLAIN_OFFSET + EM_BLE_ENC_LEN * sizeof(uint8_t))
/// Offset of the control structure area
#define EM_BLE_CS_OFFSET         (EM_BLE_ENC_CIPHER_OFFSET + EM_BLE_ENC_LEN * sizeof(uint8_t))
/// Offset of the public white list area
#define EM_BLE_WPB_OFFSET        (EM_BLE_CS_OFFSET + EM_BLE_CS_COUNT * REG_BLE_EM_CS_SIZE)
/// Offset of the private white list area
#define EM_BLE_WPV_OFFSET        (EM_BLE_WPB_OFFSET + BLE_WHITELIST_MAX * REG_BLE_EM_WPB_SIZE)
/// Offset of the TX descriptor area
#define EM_BLE_TX_DESC_OFFSET    (EM_BLE_WPV_OFFSET + BLE_WHITELIST_MAX * REG_BLE_EM_WPV_SIZE)
/// Offset of the RX descriptor area
#define EM_BLE_RX_DESC_OFFSET    (EM_BLE_TX_DESC_OFFSET + EM_BLE_TX_DESC_COUNT * REG_BLE_EM_TX_DESC_SIZE)
/// Offset of the TX buffer area
#define EM_BLE_TX_BUFFER_OFFSET  (EM_BLE_RX_DESC_OFFSET + EM_BLE_RX_DESC_COUNT * REG_BLE_EM_RX_DESC_SIZE)
/// Offset of the RX buffer area
#if (RWBLE_SW_VERSION_MAJOR >= 8)
    #if (RWBLE_SW_VERSION_MINOR < 1)
        #define _EM_BLE_RX_BUFFER_OFFSET  (EM_BLE_TX_BUFFER_OFFSET + EM_BLE_TX_BUFFER_COUNT * _REG_BLE_EM_TX_BUFFER_SIZE)
    #else
        #define _EM_BLE_RX_BUFFER_OFFSET  (EM_BLE_TX_BUFFER_OFFSET + _EM_BLE_TX_BUFFER_COUNT * _REG_BLE_EM_TX_BUFFER_SIZE)
    #endif /* (RWBLE_SW_VERSION_MINOR < 1) */
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */
#define EM_BLE_RX_BUFFER_OFFSET  (EM_BLE_TX_BUFFER_OFFSET + EM_BLE_TX_BUFFER_COUNT * REG_BLE_EM_TX_BUFFER_SIZE)
/// Offset of the Connection Address
#if (RWBLE_SW_VERSION_MAJOR >= 8)
#define _EM_BLE_CNXADD_OFFSET     (_EM_BLE_RX_BUFFER_OFFSET + EM_BLE_RX_BUFFER_COUNT * _REG_BLE_EM_RX_BUFFER_SIZE)
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */
#define EM_BLE_CNXADD_OFFSET     (EM_BLE_RX_BUFFER_OFFSET + EM_BLE_RX_BUFFER_COUNT * REG_BLE_EM_RX_BUFFER_SIZE)

/// End of BLE EM
#if (RWBLE_SW_VERSION_MAJOR >= 8)
#define _EM_BLE_END               (_EM_BLE_CNXADD_OFFSET + BD_ADDR_LEN)
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */
#define EM_BLE_END               (EM_BLE_CNXADD_OFFSET + BD_ADDR_LEN)

/// @} LLDEXMEM

#endif // EM_MAP_BLE_H_
