#ifndef _REG_BLE_EM_RX_BUFFER_H_
#define _REG_BLE_EM_RX_BUFFER_H_

#include <stdint.h>
#include "_reg_ble_em_rx_buffer.h"
#include "compiler.h"
#include "arch.h"
#include "em_map.h"
#include "reg_access.h"

#define REG_BLE_EM_RX_BUFFER_COUNT 1

#define REG_BLE_EM_RX_BUFFER_DECODING_MASK 0x00000000

#define REG_BLE_EM_RX_BUFFER_ADDR_GET(idx) (EM_BLE_RX_BUFFER_OFFSET + (idx) * REG_BLE_EM_RX_BUFFER_SIZE)

/**
 * @brief RXBUF register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  15:00                RXBUF   0x0
 * </pre>
 */
#define BLE_RXBUF_ADDR   (0x00 + (_ble_base) + EM_BLE_RX_BUFFER_OFFSET)
#define BLE_RXBUF_INDEX  0x00000000
#define BLE_RXBUF_RESET  0x00000000
#define BLE_RXBUF_COUNT  19

__STATIC_INLINE uint16_t ble_rxbuf_get(int elt_idx, int reg_idx)
{
    ASSERT_ERR(reg_idx <= 18);
    return EM_BLE_RD(BLE_RXBUF_ADDR + elt_idx * REG_BLE_EM_RX_BUFFER_SIZE + reg_idx * 2);
}

__STATIC_INLINE void ble_rxbuf_set(int elt_idx, int reg_idx, uint16_t value)
{
    ASSERT_ERR(reg_idx <= 18);
    EM_BLE_WR(BLE_RXBUF_ADDR + elt_idx * REG_BLE_EM_RX_BUFFER_SIZE + reg_idx * 2, value);
}

// field definitions
#define BLE_RXBUF_MASK   ((uint16_t)0x0000FFFF)
#define BLE_RXBUF_LSB    0
#define BLE_RXBUF_WIDTH  ((uint16_t)0x00000010)

#define BLE_RXBUF_RST    0x0

__STATIC_INLINE uint16_t ble_rxbuf_getf(int elt_idx, int reg_idx)
{
    ASSERT_ERR(reg_idx <= 18);
    uint16_t localVal = EM_BLE_RD(BLE_RXBUF_ADDR + elt_idx * REG_BLE_EM_RX_BUFFER_SIZE + reg_idx * 2);
    ASSERT_ERR((localVal & ~((uint16_t)0x0000FFFF)) == 0);
    return (localVal >> 0);
}

__STATIC_INLINE void ble_rxbuf_setf(int elt_idx, int reg_idx, uint16_t rxbuf)
{
    ASSERT_ERR(reg_idx <= 18);
    ASSERT_ERR((((uint16_t)rxbuf << 0) & ~((uint16_t)0x0000FFFF)) == 0);
    EM_BLE_WR(BLE_RXBUF_ADDR + elt_idx * REG_BLE_EM_RX_BUFFER_SIZE + reg_idx * 2, (uint16_t)rxbuf << 0);
}


#endif // _REG_BLE_EM_RX_BUFFER_H_

