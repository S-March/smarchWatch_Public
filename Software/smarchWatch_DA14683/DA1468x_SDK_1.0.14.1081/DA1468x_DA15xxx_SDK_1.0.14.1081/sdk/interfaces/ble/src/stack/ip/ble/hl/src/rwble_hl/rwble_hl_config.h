/**
 ****************************************************************************************
 *
 * @file rwble_hl_config.h
 *
 * @brief Configuration of the BLE protocol stack (max number of supported connections,
 * type of partitioning, etc.)
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef RWBLE_HL_CONFIG_H_
#define RWBLE_HL_CONFIG_H_

/**
 ****************************************************************************************
 * @addtogroup ROOT
 * @{
 * @name BLE stack configuration
 * @{
 ****************************************************************************************
 */

#include "rwble_hl_error.h"
#include "user_config_defs.h"
/******************************************************************************************/
/* -------------------------   BLE PARTITIONING      -------------------------------------*/
/******************************************************************************************/


/******************************************************************************************/
/* --------------------------   INTERFACES        ----------------------------------------*/
/******************************************************************************************/

//VM
//#if BLE_APP_PRESENT
//#define APP_MAIN_TASK       TASK_APP
//#else // BLE_APP_PRESENT
//#define APP_MAIN_TASK       TASK_GTL
//#endif // BLE_APP_PRESENT
#define APP_MAIN_TASK   rom_cfg_table[app_main_task_pos]
// Host Controller Interface (Host side)
#define BLEHL_HCIH_ITF            HCIH_ITF

/******************************************************************************************/
/* --------------------------   COEX SETUP        ----------------------------------------*/
/******************************************************************************************/

///WLAN coex
#define BLEHL_WLAN_COEX          RW_WLAN_COEX
///WLAN test mode
#define BLEHL_WLAN_COEX_TEST     RW_WLAN_COEX_TEST

/******************************************************************************************/
/* --------------------------   HOST MODULES      ----------------------------------------*/
/******************************************************************************************/

#define BLE_GAPM                    1
#if (BLE_CENTRAL || BLE_PERIPHERAL)
#define BLE_GAPC                    1
#define BLE_GAPC_HEAP_ENV_SIZE      (sizeof(struct gapc_env_tag)  + KE_HEAP_MEM_RESERVED)
#else //(BLE_CENTRAL || BLE_PERIPHERAL)
#define BLE_GAPC                    0
#define BLE_GAPC_HEAP_ENV_SIZE      0
#endif //(BLE_CENTRAL || BLE_PERIPHERAL)

#if (BLE_CENTRAL || BLE_PERIPHERAL)
#define BLE_L2CM                    1
#define BLE_L2CC                    1
#define BLE_ATTM                    1
#define BLE_GATTM                   1
#define BLE_GATTC                   1
#define BLE_GATTC_HEAP_ENV_SIZE     (sizeof(struct gattc_env_tag)  + KE_HEAP_MEM_RESERVED)
#define BLE_L2CC_HEAP_ENV_SIZE      (sizeof(struct l2cc_env_tag)   + KE_HEAP_MEM_RESERVED)
#else //(BLE_CENTRAL || BLE_PERIPHERAL)
#define BLE_L2CM                    0
#define BLE_L2CC                    0
#define BLE_ATTC                    0
#define BLE_ATTS                    0
#define BLE_ATTM                    0
#define BLE_GATTM                   0
#define BLE_GATTC                   0
#define BLE_GATTC_HEAP_ENV_SIZE     0
#define BLE_L2CC_HEAP_ENV_SIZE      0
#endif //(BLE_CENTRAL || BLE_PERIPHERAL)

#if RW_BLE_USE_CRYPT
#define BLE_SMPM                    1
#if (BLE_CENTRAL || BLE_PERIPHERAL)
#define BLE_SMPC                    1
#else //(BLE_CENTRAL || BLE_PERIPHERAL)
#define BLE_SMPC                    0
#endif //(BLE_CENTRAL || BLE_PERIPHERAL)
#else //RW_BLE_USE_CRYPT
#define BLE_SMPM                    0
#define BLE_SMPC                    0
#endif //RW_BLE_USE_CRYPT


/******************************************************************************************/
/* --------------------------   ATT DB            ----------------------------------------*/
/******************************************************************************************/

//ATT DB,Testing and Qualification related flags
#if (BLE_CENTRAL || BLE_PERIPHERAL)
    /// Support of External DB Management
    #if defined(CFG_EXT_DB)
        #define BLE_EXT_ATT_DB         1
    #else
        #define BLE_EXT_ATT_DB         0
    #endif // defined(CFG_EXT_DB)
#else
    #define BLE_EXT_ATT_DB         0
#endif // (BLE_CENTRAL || BLE_PERIPHERAL)
/******************************************************************************************/
/* --------------------------   PROFILES          ----------------------------------------*/
/******************************************************************************************/
#ifdef CFG_PRF
#define BLE_PROFILES      (1)
/// Number of Profile tasks managed by GAP manager.
#define BLE_NB_PROFILES   (CFG_NB_PRF)
#include "rwprf_config.h"
#else
#define BLE_PROFILES      (0)
#define BLE_NB_PROFILES   (0)
#endif // CFG_PRF


#ifndef   BLE_ATTS
#if (BLE_CENTRAL || BLE_PERIPHERAL || defined(CFG_ATTS))
#define BLE_ATTS                    1
#else
#define BLE_ATTS                    0
#endif // (BLE_CENTRAL || BLE_PERIPHERAL || defined(CFG_ATTS))
#endif // BLE_ATTS


#ifndef   BLE_ATTC
#if (BLE_CENTRAL || defined(CFG_ATTC))
#define BLE_ATTC                    1
#else
#define BLE_ATTC                    0
#endif // (BLE_CENTRAL || defined(CFG_ATTC))
#endif // BLE_ATTC



/// Attribute Server
#if (BLE_ATTS)
#define BLE_ATTS                    1
#else
#define BLE_ATTS                    0
#endif //(BLE_ATTS)


/// Size of the heap
#if (BLE_CENTRAL || BLE_PERIPHERAL)
    /// some heap must be reserved for attribute database
    #if (BLE_ATTS || BLE_ATTC)
        #define BLEHL_HEAP_DB_SIZE                 (3072)
    #else
        #define BLEHL_HEAP_DB_SIZE                 (0)
    #endif /* (BLE_ATTS || BLE_ATTC) */

    #define BLEHL_HEAP_MSG_SIZE                    (256 + 256 * BLE_CONNECTION_MAX)
#else
    #define BLEHL_HEAP_MSG_SIZE                    (256)
    #define BLEHL_HEAP_DB_SIZE                     (0)
#endif /* #if (BLE_CENTRAL || BLE_PERIPHERAL) */




/// Number of BLE HL tasks
#define BLEHL_TASK_SIZE       BLE_HOST_TASK_SIZE + BLE_PRF_TASK_SIZE

/// Size of environment variable needed on BLE Host Stack for one link
#define BLEHL_HEAP_ENV_SIZE ( BLE_GAPC_HEAP_ENV_SIZE       +  \
                              BLE_GATTC_HEAP_ENV_SIZE      +  \
                              BLE_L2CC_HEAP_ENV_SIZE)

/// @} BLE stack configuration
/// @} ROOT

#endif // RWBLE_HL_CONFIG_H_
