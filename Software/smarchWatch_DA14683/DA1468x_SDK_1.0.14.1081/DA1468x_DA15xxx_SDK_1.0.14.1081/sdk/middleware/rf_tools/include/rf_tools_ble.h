/**
 \addtogroup UTILITIES
 \{
 */

/**
 ****************************************************************************************
 *
 * @file rf_tools_ble.h
 *
 * @brief RF Tools for BLE
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef RF_TOOLS_BLE_H
#define RF_TOOLS_BLE_H

typedef void (*evt_pkt_tx)(uint8_t status);
typedef void (*evt_pkt_tx_intv)(uint8_t completed, uint8_t status);
typedef void (*evt_pkt_rx_stats)(void);
typedef void (*evt_pkt_stop)(uint8_t status, uint16_t packets);
typedef void (*evt_pkt_stop_rx_stats)(uint16_t packets,
        uint16_t sync_errors, uint16_t crc_errors, uint16_t rssi);
typedef void (*evt_start_cont_tx)(void);
typedef void (*evt_stop_cont_tx)(void);

typedef struct {
        evt_pkt_tx tx;
        evt_pkt_rx_stats rx_stats;
        evt_pkt_stop stop;
        evt_pkt_stop_rx_stats stop_rx_stats;
        evt_start_cont_tx start_cont_tx;
        evt_stop_cont_tx stop_cont_tx;
        evt_pkt_tx_intv tx_intv;
} rf_tools_ble_evt_cbs_t;

#define mainBIT_BLE_MGR_EVT (1 << 0)

void rf_tools_ble_init(xTaskHandle task_handle, rf_tools_ble_evt_cbs_t cbs);
void rf_tools_ble_handle_evt(uint32_t ulNotifiedValue);

/**
* \brief  Start continuous ble tx
*
* \param  [in] freq: frequency channel, 0-39
* \param  [in] payload_length: the length of the packet payload to TX (0x0 - 0x25)
* \param  [in] payload_type: 0: PRBS9, 1: 11110000, 2: 10101010, 3: Vendor Specific
*
* \return void
*
*/
void rf_tools_ble_start_cont_pkt_tx(uint8_t freq, uint8_t payload_length,
        uint8_t payload_type);

/**
* \brief Stop TX
*
* \return void
*
*/
void rf_tools_ble_stop_test(void);

void rf_tools_ble_start_cont_tx(uint8_t freq, uint8_t payload_type);
void rf_tools_ble_stop_cont_tx(void);

/**
* \brief  Start ble rx and count packets
*
* \param  [in] freq: frequency channel, 0-39
*
* \return void
*
*/
void rf_tools_ble_start_pkt_rx_stats(uint8_t freq);

void rf_tools_ble_stop_pkt_rx_stats(void);

void rf_tools_ble_start_pkt_tx_interval(uint8_t freq, uint8_t payload_length,
                uint8_t payload_type, uint16_t num, uint32_t intv);

static inline int8_t rf_tools_ble_get_channel_rf(void *freq)
{
        uint16_t fr = *(uint16_t *)freq;

        if (fr < 2402 || fr > 2480)
                return -12;

        return (fr - 2402) / 2;
}




#endif /* RF_TOOLS_BLE_H */
/**
 \}
 */
