/**
 * \addtogroup INTERFACES
 * \{
 * \addtogroup BLE
 * \{
 * \addtogroup BLE_STACK_CONFIG
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ble_stack_config.h
 *
 * @brief BLE stack configuration options
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifndef _BLE_STACK_CONFIG_H_
#define _BLE_STACK_CONFIG_H_

#include "ble_config.h"

/*
 * BLE stack's variables address base and size
 */
#if ((dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A) \
                && (dg_configBLACK_ORCA_IC_STEP == BLACK_ORCA_IC_STEP_E))
#define BLE_VAR_ADDR            (0x7FDC000)
#elif ((dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_B) \
                && (dg_configBLACK_ORCA_IC_STEP == BLACK_ORCA_IC_STEP_B))
#define BLE_VAR_ADDR            (0x7FC0C00)
#define BLE_VAR_SIZE            (0x3B12)
#else
#error "Unsupported chip version"
#endif

#define RAM_BUILD

/*
 * (ROM variable) : 0=GTL auto, 1=HCI auto, 8=GTL fix, 9=HCI fix
 */
extern char use_h4tl;

/*
 * (ROM variable) : GAP configuration
 */
extern const struct gap_cfg_user_struct gap_cfg_user_var_struct;
extern struct gap_cfg_user_struct *gap_cfg_user;

/*
 * (ROM variable) : Table of pointers of functions called by the ROM code
 */
#if ((dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A) \
                && (dg_configBLACK_ORCA_IC_STEP == BLACK_ORCA_IC_STEP_E))
extern const uint32_t rom_func_addr_table_var[87];
#elif ((dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_B) \
                && (dg_configBLACK_ORCA_IC_STEP == BLACK_ORCA_IC_STEP_B))
extern const uint32_t rom_func_addr_table_var[93];
#else
#error "Unsupported chip version"
#endif

extern uint32_t *rom_func_addr_table;

/*
 * (ROM variable) : HEAPs and basic configuration
 */
extern const uint32_t rom_cfg_table_var[];
extern uint32_t *rom_cfg_table;

/*
 * (ROM variable) : Base address of the variables of the ROM stack
 */
extern unsigned int _ble_base;

/*
 * Stack definitions
 */
#define CFG_NVDS
#define CFG_EMB
#define CFG_HOST
#undef  CFG_APP
#define CFG_GTL
#define CFG_BLE
#define CFG_HCI_UART
#define CFG_EXT_DB
#undef  CFG_DBG_MEM
#define CFG_DBG_FLASH
#undef  CFG_DBG_NVDS
#undef  CFG_DBG_STACK_PROF
#define CFG_RF_RIPPLE
#undef  CFG_PERIPHERAL
#define CFG_ALLROLES            1
#define CFG_CON                 8
#define CFG_SECURITY_ON         1
#define CFG_ATTC
#define CFG_ATTS
#define CFG_PRF
#define CFG_NB_PRF              32//10

#define CFG_DBG
#define DA_DBG_TASK

#define CFG_WLAN_COEX

#define CFG_H4TL                1

#define RIPPLE_ID               26
#define CFG_BLECORE_11
#define CFG_SLEEP

#define RADIO_680               1

#define CFG_DEEP_SLEEP
#define __NO_EMBEDDED_ASM
#undef  CFG_APP_SEC
#define CFG_CHNL_ASSESS
#define CFG_LUT_PATCH
#define CFG_DBG_NVDS
#define CFG_DBG_MEM

#define BLE_CONNECTION_MAX_USER defaultBLE_MAX_CONNECTIONS
#if defaultBLE_MAX_CONNECTIONS > CFG_CON
#       error "defaultBLE_MAX_CONNECTIONS cannot exceed CFG_CON"
#endif

/**
 * \brief Default BLE stack database heap size in bytes
 */
#ifndef dg_configBLE_STACK_DB_HEAP_SIZE
#define dg_configBLE_STACK_DB_HEAP_SIZE    3072
#endif

/**
 * \brief Ble pti values
 */
#define BLE_PTI_CONNECT_REQ_RESPONSE      0
#define BLE_PTI_LLCP_PACKETS              1
#define BLE_PTI_DATA_CHANNEL_TX           2
#define BLE_PTI_INITIATING_SCAN           3
#define BLE_PTI_ACTIVE_SCAN_MODE          4
#define BLE_PTI_CONNECTABLE_ADV_MODE      5
#define BLE_PTI_NON_CONNECTABLE_ADV_MODE  6

#include "bsp_debug.h"

#endif /* _BLE_STACK_CONFIG_H_ */

/**
 * \}
 * \}
 * \}
 */

