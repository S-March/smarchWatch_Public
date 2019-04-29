/**
 ****************************************************************************************
 *
 * @file rwble_config.h
 *
 * @brief Configuration of the BLE protocol stack (max number of supported connections,
 * type of partitioning, etc.)
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef RWBLE_CONFIG_H_
#define RWBLE_CONFIG_H_

/**
 ****************************************************************************************
 * @addtogroup ROOT
 * @{
 * @name BLE stack configuration
 * @{
 ****************************************************************************************
 */

#include "rwip_config.h"
#include "co_version.h"

/******************************************************************************************/
/* -------------------------   COEXISTENCE SETUP      ------------------------------------*/
/******************************************************************************************/

///WLAN coex
#define RW_BLE_WLAN_COEX        RW_WLAN_COEX
///WLAN test mode
#define RW_BLE_WLAN_COEX_TEST   RW_WLAN_COEX_TEST

/******************************************************************************************/
/* --------------------------   DEBUG SETUP       ----------------------------------------*/
/******************************************************************************************/

/// Flag indicating if tester emulator is available or not
#if defined(CFG_BLE_TESTER)
/// Flag indicating if tester emulator is available or not
#define BLE_TESTER              1
#else // defined (CFG_BLE_TESTER)
#define BLE_TESTER              0
#endif // defined (CFG_BLE_TESTER)

/// Flag indicating if debug mode is activated or not
#define BLE_DEBUG               1
#define BLE_SWDIAG              RW_SWDIAG

/// Flag indicating if Read/Write memory commands are supported or not
#define BLE_DEBUG_MEM           RW_DEBUG_MEM

/// Flag indicating if Flash debug commands are supported or not
#define BLE_DEBUG_FLASH         RW_DEBUG_FLASH

/// Flag indicating if NVDS feature is supported or not
#define BLE_DEBUG_NVDS          RW_DEBUG_NVDS

/// Flag indicating if CPU stack profiling commands are supported or not
#define BLE_DEBUG_STACK_PROF    RW_DEBUG_STACK_PROF

/******************************************************************************************/
/* -------------------------   BLE SETUP      --------------------------------------------*/
/******************************************************************************************/

/// Exchange memory presence
#define BLE_EM_PRESENT              BLE_EMB_PRESENT

#define BLE_TEST_MODE_SUPPORT           1

/// Number of devices in the white list
#define BLE_WHITELIST_MAX               (BLE_CONNECTION_MAX + 2)

/// Number of devices capacity for the scan filtering
#if (BLE_CENTRAL || BLE_OBSERVER)
#if (RWBLE_SW_VERSION_MAJOR >= 8)
#define BLE_DUPLICATE_FILTER_MAX    (100)
#else
#define BLE_DUPLICATE_FILTER_MAX    (10)
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */
#endif //(BLE_CENTRAL || BLE_OBSERVER)

#if RWBLE_SW_VERSION_MAJOR >= 8
#define LLM_RESOLVING_LIST_MAX      (50)
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */

/** Size of the heap
 * - For KE messages: (N+1) x 256
 * - For LLC environment: N x 80 Bytes
 * - For LLD events/intervals: (2N+1) x (80 + 16)
 */
#if (BLE_CENTRAL || BLE_PERIPHERAL)
    #define BLE_HEAP_MSG_SIZE               (256 * (BLE_CONNECTION_MAX+1) + 80 * (BLE_CONNECTION_MAX) + 96 * (2*BLE_CONNECTION_MAX+1))
    /// Size required to allocate environment variable for one link
    #define BLE_HEAP_ENV_SIZE               (sizeof(struct llc_env_tag) + 4)
#else
    #define BLE_HEAP_MSG_SIZE               (256)
    /// Size required to allocate environment variable for one link
    #define BLE_HEAP_ENV_SIZE               (4)
#endif /* #if (BLE_CENTRAL || BLE_PERIPHERAL) */


/******************************************************************************************/
/* --------------------------   RADIO SETUP       ----------------------------------------*/
/******************************************************************************************/
/// Class of device
#define RF_CLASS1                            0

/******************************************************************************************/
/* --------------------------   REGISTER SETUP       --------------------------------------*/
/******************************************************************************************/

#define RW_BLE_CUST1_INST                    0

/******************************************************************************************/
/* --------------------------   PRIORITY SETUP       --------------------------------------*/
/******************************************************************************************/
//VM
#define RWBLE_PRIORITY_ADV_LDC    rom_cfg_table[rwble_priority_adv_ldc_pos]// = 0,
#define RWBLE_PRIORITY_SCAN       rom_cfg_table[rwble_priority_scan_pos]// = 0,
#define RWBLE_PRIORITY_MCONNECT   rom_cfg_table[rwble_priority_mconnect_pos]// = 3,
#define RWBLE_PRIORITY_SCONNECT   rom_cfg_table[rwble_priority_sconnect_pos]// = 3,
#define RWBLE_PRIORITY_ADV_HDC    rom_cfg_table[rwble_priority_adv_hdc_pos]// = 5,
#define RWBLE_PRIORITY_INIT       rom_cfg_table[rwble_priority_init_pos]// = 5,
#define RWBLE_PRIORITY_MAX        rom_cfg_table[rwble_priority_max_pos]//,

/******************************************************************************************/
/* --------------------------   DEFERRING SETUP       --------------------------------------*/
/******************************************************************************************/

enum rwble_defer_type
{
    RWBLE_DEFER_TYPE_RX                 = 0,
    RWBLE_DEFER_TYPE_END,
    RWBLE_DEFER_TYPE_TEST_END,

    RWBLE_DEFER_MAX
};
/******************************************************************************************/
/* --------------------------   SCHEDULING SETUP       --------------------------------------*/
/******************************************************************************************/

/// EA programming latency for only 1 activity
#define RWBLE_PROG_LATENCY_DFT         rom_cfg_table[rwble_prog_latency_dft_pos]//(2)
/// EA asap latency
#define RWBLE_ASAP_LATENCY             rom_cfg_table[rwble_asap_latency_pos]//(2)
/// Instant value mask
#define RWBLE_INSTANT_MASK         (0x0000FFFF)

/******************************************************************************************/
/* -----------------------   SUPPORTED HCI COMMANDS       --------------------------------*/
/******************************************************************************************/

//byte0
#define BLE_CMDS_BYTE0      BLE_DISC_CMD
//byte2
#define BLE_CMDS_BYTE2      BLE_RD_REM_VERS_CMD
//byte5
#define BLE_CMDS_BYTE5      (BLE_SET_EVT_MSK_CMD | BLE_RESET_CMD)
//byte10
#define BLE_CMDS_BYTE10     BLE_HL_NB_CMP_PKT_CMD | BLE_RD_TX_PWR_CMD
//byte14
#define BLE_CMDS_BYTE14     (BLE_RD_LOC_VERS_CMD | BLE_RD_LOC_SUP_FEAT_CMD\
                            |BLE_RD_BUF_SIZE_CMD)
//byte15
#define BLE_CMDS_BYTE15     (BLE_RD_BD_ADDR_CMD | BLE_RD_RSSI_CMD)
//byte22
#define BLE_CMDS_BYTE22     (BLE_SET_EVT_MSK_PG2_CMD)
//byte25
#define BLE_CMDS_BYTE25     (BLE_LE_SET_EVT_MSK_CMD | BLE_LE_RD_BUF_SIZE_CMD\
                            |BLE_LE_RD_LOC_SUP_FEAT_CMD | BLE_LE_SET_RAND_ADDR_CMD\
                            |BLE_LE_SET_ADV_PARAM_CMD | BLE_LE_RD_ADV_TX_PWR_CMD\
                            |BLE_LE_SET_ADV_DATA_CMD)
//byte26
#define BLE_CMDS_BYTE26     (BLE_LE_SET_SC_RSP_DATA_CMD | BLE_LE_SET_ADV_EN_CMD\
                            |BLE_LE_SET_SC_PARAM_CMD | BLE_LE_SET_SC_EN_CMD\
                            |BLE_LE_CREAT_CNX_CMD | BLE_LE_CREAT_CNX_CNL_CMD\
                            |BLE_LE_RD_WL_SIZE_CMD | BLE_LE_CLEAR_WL_CMD)
//byte27
#define BLE_CMDS_BYTE27     (BLE_LE_ADD_DEV_WL_CMD | BLE_LE_REM_DEV_WL_CMD\
                            |BLE_LE_CNX_UPDATE_CMD | BLE_LE_SET_HL_CH_CLASS_CMD\
                            |BLE_LE_RD_CH_MAP_CMD | BLE_LE_RD_REM_USED_FEAT_CMD\
                            |BLE_LE_ENCRYPT_CMD | BLE_LE_RAND_CMD)
//byte28
#define BLE_CMDS_BYTE28     (BLE_LE_START_ENC_CMD | BLE_LE_LTK_REQ_RPLY_CMD\
                            |BLE_LE_LTK_REQ_NEG_RPLY_CMD | BLE_LE_RD_SUPP_STATES_CMD\
                            |BLE_LE_RX_TEST_CMD | BLE_LE_TX_TEST_CMD\
                            |BLE_LE_STOP_TEST_CMD)
//byte32
#define BLE_CMDS_BYTE32     (BLE_RD_AUTH_PAYL_TO_CMD | BLE_WR_AUTH_PAYL_TO_CMD)
//byte33
#if (RWBLE_SW_VERSION_MINOR < 1)
#define BLE_CMDS_BYTE33     (BLE_LE_REM_CON_PARA_REQ_RPLY_CMD | BLE_LE_REM_CON_PARA_REQ_NEG_RPLY_CMD)
#else
#define BLE_CMDS_BYTE33     (BLE_LE_REM_CON_PARA_REQ_RPLY_CMD | BLE_LE_REM_CON_PARA_REQ_NEG_RPLY_CMD | \
                             BLE_LE_SET_DATA_LENGTH_CMD|BLE_LE_READ_SUGGESTED_DEFAULT_DATA_LENGTH_CMD)
//byte34
#define BLE_CMDS_BYTE34     (BLE_LE_WRITE_SUGGESTED_DEFAULT_DATA_LENGTH_CMD | \
                             BLE_READ_LOCAL_P256_PUBLIC_KEY_CMD | BLE_GENERATE_DHKEY_CMD | \
                             BLE_ADD_DEV_TO_RL_LIST_CMD | BLE_REM_DEV_FROM_RL_LIST_CMD | \
                             BLE_CLEAR_RL_LIST_CMD | BLE_READ_RL_LIST_SIZE_CMD | \
                             BLE_READ_PEER_RPA_CMD)
//byte35
#define BLE_CMDS_BYTE35     (BLE_READ_LOCAL_RPA_CMD | BLE_SET_ADDR_RESOLUTION_ENABLE_CMD | \
                             BLE_SET_RPA_TO_CMD | BLE_RD_MAX_DATA_LENGTH_CMD)
//byte39 (ESR10)
#define BLE_CMDS_BYTE39     (BLE_LE_SET_PRIVACY_MODE_CMD)
#endif /* (RWBLE_SW_VERSION_MINOR < 1) */

/// @} BLE stack configuration
/// @} ROOT

#endif // RWBLE_CONFIG_H_
