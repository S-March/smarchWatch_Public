/**
 ****************************************************************************************
 *
 * @file window_stats.c
 *
 * @brief RX Window Statistics
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */



/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <string.h>
#include "co_version.h"
#include "rwble.h"
#include "ke_event.h"
#include "ke_timer.h"
#include "co_buf.h"
#include "lld.h"
#include "llc.h"
#include "llm.h"
#include "dbg.h"
#include "llc_util.h"
#include "lld_evt.h"
#include "reg_blecore.h"
#include "pll_vcocal_lut.h"

#include "window_stats.h"

#if (BLE_WINDOW_STATISTICS == 1)

#define WINSTAT_LOG_ENABLE                      0
#define WINSTAT_DBG_ENABLE                      0
#define WINSTAT_APPLY_CONTROL_AFTER_EVENTS      0       //Measure difference and use it only after a so many connection events
#define WINDOW_OK_MARGIN                        30      //Measure in microseconds. -WINDOW_OK_MARGIN .. +WINDOW_OK_MARGIN is the space around the center
#define WINSTAT_I_GAIN                          3

static int ble_sync_error;                      //set flag if there was a sync error
static uint8 ble_missed_count;                  //before how many connection events was the master seen
static uint16 ble_winsize;                      //the programmed RX Window in the stack
static uint16 ble_drift;                        //the half of the programmed stack RX Window (does
                                                //not contain the jump table offset)

static uint32 fine_cnt1;                        //the start of the RX Window will be stored here
static uint32 base_cnt1;                        //the start of the RX Window will be stored here
static uint32 fine_cnt_sync;                    //the synch point of the Master will be stored here
static uint32 base_cnt_sync;                    //the synch point of the Master will be stored here

uint32_t mst_sca __RETAINED;
uint32_t slv_sca __RETAINED;
uint32_t sca_drift __RETAINED;
uint32_t diff_pos __RETAINED;
uint32_t diff_neg __RETAINED;
uint32_t diff_zero __RETAINED;
uint32_t max_pos_diff __RETAINED;
uint32_t max_neg_diff __RETAINED;
uint32_t diff_events __RETAINED;
uint32_t sync_errors __RETAINED;
uint32_t type_errors __RETAINED;
uint32_t len_errors __RETAINED;
uint32_t crc_errors __RETAINED;
uint32_t stat_runs __RETAINED;

#if (WINSTAT_APPLY_CONTROL_AFTER_EVENTS > 0)
static int32_t calibration_error __RETAINED;    //a variable that will contain the error to feedback
                                                //back in the RX calibration
#endif

#if WINSTAT_LOG_ENABLE
static uint16_t idx __RETAINED;
static uint16_t log_data[0x600] __RETAINED;
#endif

#if WINSTAT_DBG_ENABLE
struct dbg_struct {
        uint16 ble_winsize;
        uint16 ble_drift;
        uint32 base_cnt_sync;
        uint32 fine_cnt_sync;
        uint32 base_cnt1;
        uint32 fine_cnt1;
};
static struct dbg_struct dbg_data[0x80] __RETAINED;
static int dbg_idx;
#endif

static void rx_detect_errors(void)
{
        struct ea_elt_tag *elt = (struct ea_elt_tag *)co_list_pick(&lld_evt_env.elt_prog);
        struct lld_evt_tag *evt = LLD_EVT_ENV_ADDR_GET(elt);
        uint8_t rx_cnt = ble_rxdesccnt_getf(evt->conhdl);

        ble_sync_error = 0;             //assume there is no error

//        if (rx_cnt == 1)                //check only if the rx_cnt is one
        {
                uint8_t rx_hdl = co_buf_rx_current_get();
                struct co_buf_rx_desc *rxdesc = co_buf_rx_get(rx_hdl);
                unsigned char rx_status = llc_util_rxerr_getf(rxdesc);

                if (rx_status & BLE_SYNC_ERR_BIT) {
                        ble_sync_error = 1;
                        sync_errors++;
                }

                if (rx_status & BLE_TYPE_ERR_BIT) {
                        type_errors++;
                }

                if (rx_status & BLE_LEN_ERR_BIT) {
                        len_errors++;
                }

                if (rx_status & BLE_CRC_ERR_BIT) {
                        crc_errors++;
                }
        }
}

static void rxwin_get_last_event_stats(void)
{
        struct ea_elt_tag *elt = (struct ea_elt_tag *)co_list_pick(&lld_evt_env.elt_prog);
        struct lld_evt_tag *evt = LLD_EVT_ENV_ADDR_GET(elt);

        if (!ble_evtrxok_getf(evt->conhdl)) {
                ble_winsize = 0;
                return;
        }

        stat_runs++;

        ble_missed_count = evt->missed_cnt;

        ble_drift = evt->duration_dft * 625;

        if (evt->sync_win_size & BLE_RXWIDE_BIT) {
                ble_winsize = (evt->sync_win_size & (BLE_RXWIDE_BIT - 1)) * 625 ;
        }
        else {
                ble_winsize = evt->sync_win_size;
        }

        base_cnt_sync = ble_btcntsync0_get(evt->conhdl) |
                        (((uint32_t)ble_btcntsync1_get(evt->conhdl)) << 16);    //actual sync

        fine_cnt_sync = LLD_EVT_FINECNT_MAX - ble_fcntrxsync_getf(evt->conhdl); //actual sync

        fine_cnt1 = evt->anchor_point.finetime_cnt;                             //scheduled event
        base_cnt1 = evt->anchor_point.basetime_cnt;                             //scheduled event

        mst_sca = evt->mst_sca;
        slv_sca = lld_evt_sca_get();
        sca_drift = evt->sca_drift;

#if WINSTAT_DBG_ENABLE
        dbg_data[dbg_idx].ble_winsize = ble_winsize;
        dbg_data[dbg_idx].ble_drift = ble_drift;
        dbg_data[dbg_idx].base_cnt_sync = base_cnt_sync;
        dbg_data[dbg_idx].fine_cnt_sync = fine_cnt_sync;
        dbg_data[dbg_idx].base_cnt1 = base_cnt1;
        dbg_data[dbg_idx].fine_cnt1 = fine_cnt1;

        dbg_idx++;
        if (dbg_idx == 0x80) {
                dbg_idx = 0;
        }
#endif
}

static void inline rxwin_set_breakpoint(void)
{
        __asm("BKPT #0\n");
}

void rxwin_calculate_lag(void)
{
#if WINSTAT_LOG_ENABLE
        uint16_t *pointer_data = log_data;
#endif
        int32_t difftime = 0;
        int device_slower = 0;

        if (co_list_is_empty(&lld_evt_env.elt_prog)) {
                return;
        }

        rx_detect_errors();
        rxwin_get_last_event_stats();

        if (ble_winsize == 0) {
                return;
        }

        if (ble_sync_error) {
                /* The master was not found! So, master is outside the Rx window. There's is no way
                 * to tell whether the device is slower or faster than the master.
                 */
                difftime = ble_drift;
                device_slower = 0;
        }
        else {
                //the difference between the master and the center of the window
                difftime = (base_cnt_sync - base_cnt1) + (fine_cnt_sync - fine_cnt1);
                diff_events++;

                if (difftime > 0) {
                        diff_pos++;
                        if (max_pos_diff < difftime) max_pos_diff = difftime;
                }
                else if (difftime < 0) {
                        diff_neg++;
                        if (max_neg_diff < (difftime * (-1))) max_neg_diff = (difftime * (-1));
                }
                else {
                        diff_zero++;
                }

                if (ble_missed_count > WINSTAT_APPLY_CONTROL_AFTER_EVENTS) {
                        device_slower = 0;

                        if (difftime > WINDOW_OK_MARGIN) {
                                device_slower = -1;
                        }
                        else if (difftime < -WINDOW_OK_MARGIN) {
                                device_slower = 1;
                        }

#if WINSTAT_LOG_ENABLE
                        pointer_data[idx++] = ble_sync_error;
                        pointer_data[idx++] = ble_missed_count;
                        pointer_data[idx++] = ble_winsize;
                        pointer_data[idx++] = difftime & 0xFFFF;
                        pointer_data[idx++] = calibration_error & 0xFFFF;
                        pointer_data[idx++] = device_slower & 0xFFFF;
#endif
                }
        }

#if (WINSTAT_APPLY_CONTROL_AFTER_EVENTS > 0)
        // if device is slower than expected it means that the calibration overestimates the RX
        // period. To amend this we need to "decrease" the period's value.
        if (device_slower > 0) {
                calibration_error += 1;
        }
        else if (device_slower < 0) {
                calibration_error -= 1;
        }
#endif

#if WINSTAT_LOG_ENABLE
        if (idx >= 0x600) {
                idx = 0;
                rxwin_set_breakpoint();
        }
#endif
}


int32_t rxwin_calibrate(void)
{
#if (WINSTAT_APPLY_CONTROL_AFTER_EVENTS > 0)
        return (calibration_error >> WINSTAT_I_GAIN);
#endif
}

#endif // USE_WINDOW_STATISTICS
