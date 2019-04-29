/**
 \addtogroup UTILITIES
 \{
 */

/**
 ****************************************************************************************
 *
 * @file rf_tools_ftdf.h
 *
 * @brief RF Tools for FTDF
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef RF_TOOLS_FTDF_H
#define RF_TOOLS_FTDF_H

#include "ad_ftdf_phy_api.h"

typedef void (*ftdf_pkt_tx_done)(void);

typedef struct {
        ftdf_pkt_tx_done tx_done;
} rf_tools_ftdf_cbs_t;

void rf_tools_ftdf_init(rf_tools_ftdf_cbs_t);

void rf_tools_ftdf_start_tx(uint8_t ch, uint8_t len, uint16_t num_packets, uint32_t intv);
void rf_tools_ftdf_start_txstream(uint8_t ch);
void rf_tools_ftdf_stop_txstream(void);
void rf_tools_ftdf_check_stop_txstream(void) __attribute__((weak));
void rf_tools_ftdf_stop_tx(void);
void rf_tools_ftdf_send_frame_confirm(void *handle, ftdf_bitmap32_t status);
void rf_tools_ftdf_recv_frame(ftdf_data_length_t frameLength,
                              ftdf_octet_t       *frame,
                              ftdf_bitmap32_t    status);
void rf_tools_ftdf_start_rx(uint8_t ch);

void rf_tools_ftdf_stop_rx(ftdf_count_t *rx_success_count, ftdf_count_t *rx_fcs_error_count);

static inline int8_t rf_tools_ftdf_get_channel_rf(void *freq)
{
        uint16_t fr = *(uint16_t *)freq;

        if (fr < 2405 || fr > 2480)
                return -1;

        return (int8_t)((fr - 2405) / 2.5);
}

static inline int8_t rf_tools_ftdf_get_channel_mac(void *freq)
{
        uint16_t fr = *(uint16_t *)freq;

        if (fr < 2405 || fr > 2480)
                return -1;

        return 11 + (fr - 2405) / 5;
}


#endif /* RF_TOOLS_FTDF_H */
/**
 \}
 */
