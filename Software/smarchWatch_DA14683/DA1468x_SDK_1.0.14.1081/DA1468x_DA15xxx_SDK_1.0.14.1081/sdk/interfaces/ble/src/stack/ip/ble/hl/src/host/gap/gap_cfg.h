/**
 ****************************************************************************************
 *
 * @file gap_cfg.h
 *
 * @brief Header file - GAPCFG.
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef GAP_CFG_H_
#define GAP_CFG_H_

/**
 ****************************************************************************************
 * @addtogroup GAPCFG Settings
 * @ingroup GAP
 * @brief Contains GAP-related configurable values
 *
 * The GAPCFG is a header file that contains defined values necessary
 * for GAP operations particularly values for GAP modes. These values
 * are changeable in order to suit a particular application.
 *
 * @{
 ****************************************************************************************
 */

#include "co_version.h"

struct gap_cfg_user_struct {
    uint16 GAP_TMR_LIM_ADV_TIMEOUT_VAR;
    uint16 GAP_TMR_GEN_DISC_SCAN_VAR;
    uint16 GAP_TMR_LIM_DISC_SCAN_VAR;
    uint16 GAP_TMR_PRIV_ADDR_INT_VAR;
    uint16 GAP_TMR_CONN_PAUSE_CT_VAR;
    uint16 GAP_TMR_CONN_PAUSE_PH_VAR;
    uint16 GAP_TMR_CONN_PARAM_TIMEOUT_VAR;
    uint16 GAP_TMR_LECB_CONN_TIMEOUT_VAR;
    uint16 GAP_TMR_LECB_DISCONN_TIMEOUT_VAR;
    uint16 GAP_TMR_SCAN_FAST_PERIOD_VAR;
    uint16 GAP_TMR_ADV_FAST_PERIOD_VAR;
    uint16 GAP_LIM_DISC_SCAN_INT_VAR;
    uint16 GAP_SCAN_FAST_INTV_VAR;
    uint16 GAP_SCAN_FAST_WIND_VAR;
    uint16 GAP_SCAN_SLOW_INTV1_VAR;
    uint16 GAP_SCAN_SLOW_INTV2_VAR;
    uint16 GAP_SCAN_SLOW_WIND1_VAR;
    uint16 GAP_SCAN_SLOW_WIND2_VAR;
    uint16 GAP_ADV_FAST_INTV1_VAR;
    uint16 GAP_ADV_FAST_INTV2_VAR;
    uint16 GAP_ADV_SLOW_INTV_VAR;
    uint16 GAP_INIT_CONN_MIN_INTV_VAR;
    uint16 GAP_INIT_CONN_MAX_INTV_VAR;
    uint16 GAP_INQ_SCAN_INTV_VAR;
    uint16 GAP_INQ_SCAN_WIND_VAR;
    uint16 GAP_CONN_SUPERV_TIMEOUT_VAR;
    uint16 GAP_CONN_MIN_CE_VAR;
    uint16 GAP_CONN_MAX_CE_VAR;
    uint16 GAP_CONN_LATENCY_VAR;
    uint16 GAP_APPEARANCE_VAR;
    uint16 GAP_PPCP_CONN_INTV_MAX_VAR;
    uint16 GAP_PPCP_CONN_INTV_MIN_VAR;
    uint16 GAP_PPCP_SLAVE_LATENCY_VAR;
    uint16 GAP_PPCP_STO_MULT_VAR;
    uint16 GAP_MAX_LE_MTU_VAR;
    uint8 GAP_DEV_NAME_VAR[20];
};


#if (RWBLE_SW_VERSION_MAJOR >= 8)
extern struct gap_cfg_user_struct *gap_cfg_user;
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */

/*
 * DEFINES
 ****************************************************************************************
 */
/// Maximum time to remain advertising when in the Limited
/// Discover able mode: TGAP(lim_adv_timeout)
/// required value: 180s: (18000 for ke timer)
#define GAP_TMR_LIM_ADV_TIMEOUT                 gap_cfg_user->GAP_TMR_LIM_ADV_TIMEOUT_VAR// 0x4650

/// Minimum time to perform scanning when performing
/// the General Discovery procedure: TGAP(gen_disc_scan_min)
/// recommended value: 10.24s: (1024 for ke timer)
#define GAP_TMR_GEN_DISC_SCAN                   gap_cfg_user->GAP_TMR_GEN_DISC_SCAN_VAR//0x0300

/// Minimum time to perform scanning when performing the
/// Limited Discovery procedure: TGAP(lim_disc_scan_min)
/// recommended value: 10.24s: (1024 for ke timer)
#define GAP_TMR_LIM_DISC_SCAN                   gap_cfg_user->GAP_TMR_LIM_DISC_SCAN_VAR//0x0300

/// Minimum time interval between private address change
/// TGAP(private_addr_int)
/// recommended value: 15 minutes; 0x01F4 for PTS
/// 0x3A98 is 150 seconds; 0xEA60 is 10 minutes
#define GAP_TMR_PRIV_ADDR_INT                   gap_cfg_user->GAP_TMR_PRIV_ADDR_INT_VAR//0x3A98

/// Central idle timer
/// TGAP(conn_pause_central)
/// recommended value: 1 s: (100 for ke timer)
#define GAP_TMR_CONN_PAUSE_CT                   gap_cfg_user->GAP_TMR_CONN_PAUSE_CT_VAR//0x0064

/// Minimum time upon connection establishment before the peripheral
/// starts a connection update procedure: TGAP(conn_pause_peripheral)
/// recommended value: 5 s: (500 for ke timer)
#define GAP_TMR_CONN_PAUSE_PH                   gap_cfg_user->GAP_TMR_CONN_PAUSE_PH_VAR//0x01F4

/// Timer used in connection parameter update procedure
/// TGAP(conn_param_timeout)
/// recommended value: 30 s: (3000 for ke timer)
#define GAP_TMR_CONN_PARAM_TIMEOUT              gap_cfg_user->GAP_TMR_CONN_PARAM_TIMEOUT_VAR//0x0BB8

/// Timer used in LE credit based connection procedure
/// TGAP(lecb_conn_timeout)
/// recommended value: 30 s: (3000 for ke timer)
#define GAP_TMR_LECB_CONN_TIMEOUT               gap_cfg_user->GAP_TMR_LECB_CONN_TIMEOUT_VAR//0x0BB8

/// Timer used in LE credit based disconnection procedure
/// TGAP(lecb_disconn_timeout)
/// recommended value: 30 s: (3000 for ke timer)
#define GAP_TMR_LECB_DISCONN_TIMEOUT            gap_cfg_user->GAP_TMR_LECB_DISCONN_TIMEOUT_VAR//0x0BB8

/// Minimum time to perform scanning when user initiated
/// TGAP(scan_fast_period)
/// recommended value: 30.72 s: (3072 for ke timer)
#define GAP_TMR_SCAN_FAST_PERIOD                gap_cfg_user->GAP_TMR_SCAN_FAST_PERIOD_VAR//0x0C00

/// Minimum time to perform advertising when user initiated
/// TGAP(adv_fast_period)
/// recommended value: 30 s: (3000 for ke timer)
#define GAP_TMR_ADV_FAST_PERIOD                 gap_cfg_user->GAP_TMR_ADV_FAST_PERIOD_VAR//0x0BB8

/// Scan interval used during Link Layer Scanning State when
/// performing the Limited Discovery procedure
/// TGAP(lim_disc_scan_int)
/// recommended value: 11.25ms; (18 decimal)
#define GAP_LIM_DISC_SCAN_INT                   gap_cfg_user->GAP_LIM_DISC_SCAN_INT_VAR//0x0012

/// Scan interval in any discovery or connection establishment
/// procedure when user initiated: TGAP(scan_fast_interval)
/// recommended value: 30 to 60 ms; N * 0.625
#define GAP_SCAN_FAST_INTV                      gap_cfg_user->GAP_SCAN_FAST_INTV_VAR//0x0030

/// Scan window in any discovery or connection establishment
/// procedure when user initiated: TGAP(scan_fast_window)
/// recommended value: 30 ms; N * 0.625
#define GAP_SCAN_FAST_WIND                      gap_cfg_user->GAP_SCAN_FAST_WIND_VAR//0x0030

/// Scan interval in any discovery or connection establishment
/// procedure when background scanning: TGAP(scan_slow_interval1)
/// recommended value: 1.28 s : 0x00CD (205); N * 0.625
#define GAP_SCAN_SLOW_INTV1                     gap_cfg_user->GAP_SCAN_SLOW_INTV1_VAR//0x00CD

/// Scan interval in any discovery or connection establishment
/// procedure when background scanning: TGAP(scan_slow_interval2)
/// recommended value: 2.56 s : 0x019A (410); N * 0.625
#define GAP_SCAN_SLOW_INTV2                     gap_cfg_user->GAP_SCAN_SLOW_INTV2_VAR//0x019A

/// Scan window in any discovery or connection establishment
/// procedure when background scanning: TGAP(scan_slow_window1)
/// recommended value: 11.25 ms : 0x0012 (18); N * 0.625
#define GAP_SCAN_SLOW_WIND1                     gap_cfg_user->GAP_SCAN_SLOW_WIND1_VAR//0x0012

/// Scan window in any discovery or connection establishment
/// procedure when background scanning: TGAP(scan_slow_window2)
/// recommended value: 22.5 ms : 0x0024 (36); N * 0.625
#define GAP_SCAN_SLOW_WIND2                     gap_cfg_user->GAP_SCAN_SLOW_WIND2_VAR//0x0024

/// Minimum to maximum advertisement interval in any discoverable
/// or connectable mode when user initiated: TGAP(adv_fast_interval1)
/// recommended value: 30 to 60 ms; N * 0.625
#define GAP_ADV_FAST_INTV1                      gap_cfg_user->GAP_ADV_FAST_INTV1_VAR//0x0030

/// Minimum to maximum advertisement interval in any discoverable
/// or connectable mode when user initiated: TGAP(adv_fast_interval2)
/// recommended value: 100 to 150 ms; N * 0.625
#define GAP_ADV_FAST_INTV2                      gap_cfg_user->GAP_ADV_FAST_INTV2_VAR//0x0064

/// Minimum to maximum advertisement interval in any discoverable or
/// connectable mode when background advertising: TGAP(adv_slow_interval)
/// recommended value: 1 to 1.2 s : 0x00B0 (176); N * 0.625
#define GAP_ADV_SLOW_INTV                       gap_cfg_user->GAP_ADV_SLOW_INTV_VAR//0x00B0

/// Minimum to maximum connection interval upon any connection
/// establishment: TGAP(initial_conn_interval)
/// recommended value: 30 to 50 ms ; N * 1.25 ms
#define GAP_INIT_CONN_MIN_INTV                  gap_cfg_user->GAP_INIT_CONN_MIN_INTV_VAR//0x0018
#define GAP_INIT_CONN_MAX_INTV                  gap_cfg_user->GAP_INIT_CONN_MAX_INTV_VAR//0x0028

/// RW Defines
#define GAP_INQ_SCAN_INTV                       gap_cfg_user->GAP_INQ_SCAN_INTV_VAR//0x0012
#define GAP_INQ_SCAN_WIND                       gap_cfg_user->GAP_INQ_SCAN_WIND_VAR//0x0012

/// Connection supervision timeout
/// recommended value: 20s
#define GAP_CONN_SUPERV_TIMEOUT                 gap_cfg_user->GAP_CONN_SUPERV_TIMEOUT_VAR//0x07D0

/// Minimum connection event
/// default value: 0x0000
#define GAP_CONN_MIN_CE                         gap_cfg_user->GAP_CONN_MIN_CE_VAR//0x0000

/// Maximum connection event
/// default value: 0xFFFF
#define GAP_CONN_MAX_CE                         gap_cfg_user->GAP_CONN_MAX_CE_VAR//0xFFFF

/// Connection latency
/// default value: 0x0000
#define GAP_CONN_LATENCY                        gap_cfg_user->GAP_CONN_LATENCY_VAR//0x0000

/// GAP Device name Characteristic
/// Default device name
#define GAP_DEV_NAME                            gap_cfg_user->GAP_DEV_NAME_VAR//"RIVIERAWAVES-BLE"

/// GAP Appearance or Icon Characteristic - 2 octets
/// Current appearance value is 0x0000 (unknown appearance)
/// Description:
/// http://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicViewer.aspx?u=org.bluetooth.characteristic.gap.appearance.xml
#define GAP_APPEARANCE                          gap_cfg_user->GAP_APPEARANCE_VAR//0x0000

//#if (BLE_PERIPHERAL)
///GAP Peripheral Preferred Connection Parameter - 8 octets
#define GAP_PPCP_CONN_INTV_MAX                  gap_cfg_user->GAP_PPCP_CONN_INTV_MAX_VAR//0x0064
#define GAP_PPCP_CONN_INTV_MIN                  gap_cfg_user->GAP_PPCP_CONN_INTV_MIN_VAR//0x00C8
#define GAP_PPCP_SLAVE_LATENCY                  gap_cfg_user->GAP_PPCP_SLAVE_LATENCY_VAR//0x0000
#define GAP_PPCP_STO_MULT                       gap_cfg_user->GAP_PPCP_STO_MULT_VAR//0x07D0
//#endif /* #if (BLE_PERIPHERAL) */

/// Low energy mask
#define GAP_EVT_MASK                            {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x9F, 0x00,  0x20}
#if (RWBLE_SW_VERSION_MAJOR >= 8)
#define GAP_LE_EVT_MASK                         0xFF
#define GAP_LE_EVT_MASK_1                       0x07
#else
#define GAP_LE_EVT_MASK                         0x3F
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */

#define GAP_LE_EVT_4_0_MASK                     0x1F


/// Maximal authorized MTU value
#define GAP_MAX_LE_MTU                          gap_cfg_user->GAP_MAX_LE_MTU_VAR //(512)

/// Maximum GAP device name size
#define GAP_MAX_NAME_SIZE                       (0x20)
/// @} GAPCFG
#endif // GAP_CFG_H_
