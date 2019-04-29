/**
 ****************************************************************************************
 *
 * @file ble_prod_test.h
 *
 * @brief BLE Production Test header file
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#ifndef BLE_PROD_TEST_H_
#define BLE_PROD_TEST_H_

#define BLE_PROD_TEST_MIN_OPCODE 0xFC80

#define BPT_OPCODE_START_PKT_RX_STATS 0xFC81
#define BPT_OPCODE_STOP_PKT_RX_STATS 0xFC82
#define BPT_OPCODE_UNMODULATED_TRX 0xFC83
#define BPT_OPCODE_START_CONT_TX 0xFC84
#define BPT_OPCODE_STOP_CONT_TX 0xFC85
#define BPT_OPCODE_PKT_TX_INTV 0xFC90


#define BPT_EVTCODE_START_PKT_RX_STATS 0x0E
#define BPT_EVTCODE_STOP_PKT_RX_STATS 0x0E
#define BPT_EVTCODE_UNMODULATED_TRX 0x0E
#define BPT_EVTCODE_START_CONT_TX 0x0E
#define BPT_EVTCODE_STOP_CONT_TX 0x0E
#define BPT_EVTCODE_PKT_TX_INTV_STARTED 0xF
#define BPT_EVTCODE_PKT_TX_INTV_COMPLETE 0xE

#define BPT_PLEN_START_PKT_RX_STATS 0x03
#define BPT_PLEN_STOP_PKT_RX_STATS 0xb
#define BPT_PLEN_UNMODULATED_TRX 0x03
#define BPT_PLEN_START_CONT_TX 0x3
#define BPT_PLEN_STOP_CONT_TX 0x3
#define BPT_PLEN_PKT_TX_INTV_STARTED 0x3
#define BPT_PLEN_PKT_TX_INTV_COMPLETE 0x4

void ble_prod_test_cmd(const ble_mgr_common_stack_msg_t *stack_msg);
void lld_evt_deffered_elt_handler(void);

void ble_prod_test_check_tx_packet_count(void);

#endif /* BLE_PROD_TEST_H_ */
