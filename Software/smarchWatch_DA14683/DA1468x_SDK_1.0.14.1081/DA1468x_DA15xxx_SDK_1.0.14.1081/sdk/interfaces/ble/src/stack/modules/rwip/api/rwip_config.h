/**
 ****************************************************************************************
 *
 * @file rwip_config.h
 *
 * @brief Configuration of the RW IP SW
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef RWIP_CONFIG_H_
#define RWIP_CONFIG_H_

#include "ble_stack_config.h"
#include "co_version.h"

/* need to ignore -Wcomment here since we use -Werror so this won't otherwise build */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcomment"
#include "arch.h"
#pragma GCC diagnostic pop

/**
 ****************************************************************************************
 * @addtogroup ROOT
 * @{
 *
 *  Information about RW SW IP options and flags
 *
 *        BT_DUAL_MODE             BT/BLE Dual Mode
 *        BT_STD_MODE              BT Only
 *        BLE_STD_MODE             BLE Only
 *
 *        RW_DM_SUPPORT            Dual mode is supported
 *        RW_BLE_SUPPORT           Configured as BLE only
 *
 *        BT_EMB_PRESENT           BT controller exists
 *        BLE_EMB_PRESENT          BLE controller exists
 *        BLE_HOST_PRESENT         BLE host exists
 *
 * @name RW Stack Configuration
 * @{
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */

/******************************************************************************************/
/* --------------------------   GENERAL SETUP       --------------------------------------*/
/******************************************************************************************/

/// Flag indicating if stack is compiled in dual or single mode
#if defined(CFG_BT)
    #define BLE_STD_MODE                     0
    #if defined(CFG_BLE)
        #define BT_DUAL_MODE                 1
        #define BT_STD_MODE                  0
    #else // CFG_BLE
        #define BT_DUAL_MODE                 0
        #define BT_STD_MODE                  1
    #endif // CFG_BLE
#elif defined(CFG_BLE)
    #define BT_DUAL_MODE                     0
    #define BT_STD_MODE                      0
    #define BLE_STD_MODE                     1
#endif // CFG_BT

/// Flag indicating if Dual mode is supported
#define RW_DM_SUPPORT         BT_DUAL_MODE

/// Flag indicating if BLE handles main parts of the stack
#define RW_BLE_SUPPORT        BLE_STD_MODE
//TBD should be removed later
#define BLE12_HW                    1
/******************************************************************************************/
/* -------------------------   STACK PARTITIONING      -----------------------------------*/
/******************************************************************************************/

#if (BT_DUAL_MODE)
    #define BT_EMB_PRESENT              1
    #define BLE_EMB_PRESENT             1
    #define HCI_PRESENT                 1
    #define BLE_HOST_PRESENT            0
    #define BLE_APP_PRESENT             0
#elif (BT_STD_MODE)
    #define BT_EMB_PRESENT              1
    #define BLE_EMB_PRESENT             0
    #define HCI_PRESENT                 1
    #define BLE_HOST_PRESENT            0
    #define BLE_APP_PRESENT             0
#elif (BLE_STD_MODE)
    #define BT_EMB_PRESENT              0
    #define HCI_PRESENT                 1
    #if defined(CFG_EMB)
        #define BLE_EMB_PRESENT         1
    #else
        #define BLE_EMB_PRESENT         0
    #endif //CFG_EMB
    #if defined(CFG_HOST)
        #define BLE_HOST_PRESENT        1
    #else
        #define BLE_HOST_PRESENT        0
    #endif //CFG_HOST
    #if defined(CFG_APP)
        #define BLE_APP_PRESENT         1
    #else
        #define BLE_APP_PRESENT         0
    #endif //CFG_APP
#endif // BT_DUAL_MODE / BT_STD_MODE / BLE_STD_MODE

#define EA_PRESENT                      (BT_EMB_PRESENT || BLE_EMB_PRESENT)

/******************************************************************************************/
/* -------------------------   INTERFACES DEFINITIONS      -------------------------------*/
/******************************************************************************************/

/// Generic Transport Layer
#if defined(CFG_GTL)
    #define GTL_ITF           1
#else // defined(CFG_GTL)
    #define GTL_ITF           0
#endif // defined(CFG_GTL)

/// H4 Transport Layer
#if defined(CFG_H4TL)
    #define H4TL_SUPPORT      1
#else //defined(CFG_H4TL)
    #define H4TL_SUPPORT      0
#endif //defined(CFG_H4TL)

/// Transport Layer interface (defines if a TL is present or not)
#define TL_ITF           H4TL_SUPPORT

/******************************************************************************************/
/* --------------------------   BLE COMMON DEFINITIONS      ------------------------------*/
/******************************************************************************************/
/// Kernel Heap memory sized reserved for allocate dynamically connection environment
#define KE_HEAP_MEM_RESERVED        (4)

#if defined(CFG_BLE)
    /// Application role definitions
    #define BLE_BROADCASTER   (defined(CFG_BROADCASTER) || defined(CFG_ALLROLES))
    #define BLE_OBSERVER      (defined(CFG_OBSERVER)    || defined(CFG_ALLROLES))
    #define BLE_PERIPHERAL    (defined(CFG_PERIPHERAL)  || defined(CFG_ALLROLES))
    #define BLE_CENTRAL       (defined(CFG_CENTRAL)     || defined(CFG_ALLROLES))

    #if (!BLE_BROADCASTER) && (!BLE_OBSERVER) && (!BLE_PERIPHERAL) && (!BLE_CENTRAL)
        #error "No application role defined"
    #endif /* #if (!BLE_BROADCASTER) && (!BLE_OBSERVER) && (!BLE_PERIPHERAL) && (!BLE_CENTRAL) */


    /// Maximum number of simultaneous connections
    #if (BLE_CENTRAL)
        #define BLE_CONNECTION_MAX          CFG_CON
    #elif (BLE_PERIPHERAL)
        #define BLE_CONNECTION_MAX          1
    #else
        #define BLE_CONNECTION_MAX          1
    #endif /* #if (BLE_CENTRAL) */

    #if (RWBLE_SW_VERSION_MINOR < 1)

        /// Number of TX data descriptors
        #if (BLE_CONNECTION_MAX == 1)
            #if (BLE_CENTRAL || BLE_PERIPHERAL)
                #define BLE_TX_DESC_DATA      5
            #else
                #define BLE_TX_DESC_DATA      0
            #endif /* (BLE_CENTRAL || BLE_PERIPHERAL) */
        #else
            #define BLE_TX_DESC_DATA     (BLE_CONNECTION_MAX * 3)
        #endif /* (BLE_CONNECTION_MAX == 1) */
    #else
        extern unsigned int BLE_TX_DESC_DATA_USER;
        extern unsigned int BLE_TX_DESC_CNTL_USER;

        /// Number of TX data descriptors
        #if (BLE_CONNECTION_MAX == 1)
            #if (BLE_CENTRAL || BLE_PERIPHERAL)
                #define _BLE_TX_DESC_DATA      5
            #else
                #define _BLE_TX_DESC_DATA      0
            #endif /* (BLE_CENTRAL || BLE_PERIPHERAL) */
        #else
            #define _BLE_TX_DESC_DATA     (BLE_CONNECTION_MAX * 3)
        #endif //(BLE_CONNECTION_MAX == 1)

        #define BLE_TX_DESC_DATA     BLE_TX_DESC_DATA_USER//(BLE_CONNECTION_MAX * 3)
    #endif /* (RWBLE_SW_VERSION_MINOR < 1) */

    #if (BLE_CENTRAL || BLE_PERIPHERAL)
        #if (RWBLE_SW_VERSION_MINOR < 1)
            /// Number of TX advertising descriptors
            #define BLE_TX_DESC_ADV       3
            /// Number of TX control descriptors
            #define BLE_TX_DESC_CNTL      BLE_CONNECTION_MAX
        #else
            /// Number of TX advertising descriptors
            #define BLE_TX_DESC_ADV       3
            #define _BLE_TX_DESC_ADV      3
            /// Number of TX control descriptors
            #define _BLE_TX_DESC_CNTL      BLE_CONNECTION_MAX
            #define BLE_TX_DESC_CNTL      BLE_TX_DESC_CNTL_USER //BLE_CONNECTION_MAX
        #endif /* (RWBLE_SW_VERSION_MINOR < 1) */
    #else
        #if (BLE_BROADCASTER)
            /// Number of TX advertising descriptors
            #define BLE_TX_DESC_ADV       2
            /// Number of TX control descriptors
            #define BLE_TX_DESC_CNTL      0
        #else
            /// Number of TX advertising descriptors
            #define BLE_TX_DESC_ADV       1
            /// Number of TX control descriptors
            #define BLE_TX_DESC_CNTL      0
        #endif // BLE_BROADCASTER
    #endif //(BLE_CENTRAL || BLE_PERIPHERAL)

    /// Total number of elements in the TX Descriptor pool
    #if (RWBLE_SW_VERSION_MINOR >= 1)
        #define _BLE_TX_DESC_CNT            (_BLE_TX_DESC_DATA + _BLE_TX_DESC_CNTL + _BLE_TX_DESC_ADV)
    #endif /* (RWBLE_SW_VERSION_MINOR >= 1) */
    #define BLE_TX_DESC_CNT                 (BLE_TX_DESC_DATA + BLE_TX_DESC_CNTL + BLE_TX_DESC_ADV)
    /// Number of RX Buffers
    #if (RWBLE_SW_VERSION_MINOR >= 1)
        #define _BLE_TX_BUFFER_CNT          (_BLE_TX_DESC_CNT)
    #endif /* (RWBLE_SW_VERSION_MINOR >= 1) */
    #define BLE_TX_BUFFER_CNT               (BLE_TX_DESC_CNT)

/// Number of receive buffers in the RX ring. This number defines the interrupt
/// rate during a connection event. An interrupt is asserted every BLE_RX_BUFFER_CNT/2
/// reception. This number has an impact on the size of the exchange memory. This number
/// may have to be increased when CPU is very slow to free the received data, in order not
/// to overflow the RX ring of buffers.

#if (BLE_CENTRAL || BLE_PERIPHERAL)
    /// Number of RX Descriptors
    #define BLE_RX_DESC_CNT             (8)
#elif (BLE_BROADCASTER)
    #define BLE_RX_DESC_CNT             (1)
#else
    #define BLE_RX_DESC_CNT             (4)
#endif //(BLE_CENTRAL || BLE_PERIPHERAL)
/// Number of RX Buffers
#define BLE_RX_BUFFER_CNT               (BLE_RX_DESC_CNT)

/// Max advertising reports before sending the info to the host
#define BLE_ADV_REPORTS_MAX             1

/// Use of security manager block
#define RW_BLE_USE_CRYPT  (defined(CFG_SECURITY_ON))
#endif //defined(CFG_BLE)


/******************************************************************************************/
/* -------------------------   BLE APPLICATION SETTINGS      -----------------------------*/
/******************************************************************************************/

/// Health Thermometer Application
#if defined(CFG_APP_HT)
#define BLE_APP_HT           1
#else // defined(CFG_APP_HT)
#define BLE_APP_HT           0
#endif // defined(CFG_APP_HT)

/// HID Application
#if defined(CFG_APP_HID)
#define BLE_APP_HID          1
#else // defined(CFG_APP_HID)
#define BLE_APP_HID          0
#endif // defined(CFG_APP_HID)

/// DIS Application
#if defined(CFG_APP_DIS)
#define BLE_APP_DIS          1
#else // defined(CFG_APP_DIS)
#define BLE_APP_DIS          0
#endif // defined(CFG_APP_DIS)

/// Device Information Service Application
#if (BLE_APP_HT || BLE_APP_HID || BLE_APP_DIS)
#define BLE_APP_DIS          1
#else
#define BLE_APP_DIS          0
#endif // (BLE_APP_HT || BLE_APP_HID)

/// Time Application
#if defined(CFG_APP_TIME)
#define BLE_APP_TIME         1
#else // defined(CFG_APP_TIME)
#define BLE_APP_TIME         0
#endif // defined(CFG_APP_TIME)

/// Battery Service Application
#if (BLE_APP_HID)
#define BLE_APP_BATT          1
#else
#define BLE_APP_BATT          0
#endif // (BLE_APP_HID)

/// Security Application
#if (defined(CFG_APP_SEC) || BLE_APP_HID)
#define BLE_APP_SEC          1
#else // defined(CFG_APP_SEC)
#define BLE_APP_SEC          0
#endif // defined(CFG_APP_SEC)


/******************************************************************************************/
/* --------------------------   DISPLAY SETUP        -------------------------------------*/
/******************************************************************************************/

/// Display controller enable/disable
#if defined(CFG_DISPLAY)
#define DISPLAY_SUPPORT      1
#else
#define DISPLAY_SUPPORT      0
#endif //CFG_DISPLAY


/******************************************************************************************/
/* --------------------------      RTC SETUP         -------------------------------------*/
/******************************************************************************************/

/// RTC enable/disable
#if defined(CFG_RTC)
#define RTC_SUPPORT      1
#else
#define RTC_SUPPORT      0
#endif //CFG_DISPLAY

/******************************************************************************************/
/* --------------------------      PS2 SETUP         -------------------------------------*/
/******************************************************************************************/

/// PS2 enable/disable
#if defined(CFG_PS2)
#define PS2_SUPPORT      1
#else
#define PS2_SUPPORT      0
#endif //CFG_PS2


/******************************************************************************************/
/* -------------------------   DEEP SLEEP SETUP      -------------------------------------*/
/******************************************************************************************/

/// DEEP SLEEP enable
#if defined(CFG_SLEEP) && (BLE_EMB_PRESENT || BT_EMB_PRESENT)
    #define DEEP_SLEEP                              1
#else
    #define DEEP_SLEEP                              0
#endif /* CFG_SLEEP */

/// Use 32K Hz Clock if set to 1 else 32,768k is used
#define HZ32000 rom_cfg_table[hz32000_pos]//HZ32000                                     0


/******************************************************************************************/
/* -------------------------    PROCESSOR SETUP      -------------------------------------*/
/******************************************************************************************/

/// 8 BIT processor
#define PROC_8BITS                        0

/******************************************************************************************/
/* --------------------------   RADIO SETUP       ----------------------------------------*/
/******************************************************************************************/

/// Power control features
#define RF_TXPWR                            1
/// Class of device
#define RF_CLASS1                           0

/******************************************************************************************/
/* -------------------------   COEXISTENCE SETUP      ------------------------------------*/
/******************************************************************************************/

/// WLAN Coexistence
#define RW_WLAN_COEX                     (defined(CFG_WLAN_COEX))
///WLAN test mode
#if defined(CFG_WLAN_COEX)
    #define RW_WLAN_COEX_TEST            (defined(CFG_WLAN_COEX_TEST))
#else
    #define RW_WLAN_COEX_TEST            0
#endif // defined(CFG_WLAN_COEX)

/******************************************************************************************/
/* -------------------------   CHANNEL ASSESSMENT SETUP      -----------------------------*/
/******************************************************************************************/

/// Channel Assessment
#if defined(CFG_BLE)
#if (defined(CFG_CHNL_ASSESS) && BLE_CENTRAL)
    #define BLE_CHNL_ASSESS        (1)
#else
    #define BLE_CHNL_ASSESS        (0)
#endif //(defined(CFG_CHNL_ASSESS) && BLE_CENTRAL)
#endif //defined(CFG_BLE)
/******************************************************************************************/
/* --------------------------   DEBUG SETUP       ----------------------------------------*/
/******************************************************************************************/

/// Flag indicating if debug mode is activated or not
#if defined(CFG_DBG)
    #define RW_DEBUG                        ((BLE_EMB_PRESENT) || (BT_EMB_PRESENT))
#if (BLE_EMB_PRESENT || BT_EMB_PRESENT)
    #define RW_SWDIAG                       0
#else
    #define RW_SWDIAG                       0
#endif
    #define KE_PROFILING                    1
#else
    #define RW_DEBUG                        0
    #define RW_SWDIAG                       0
    #define KE_PROFILING                    0
#endif /* CFG_DBG */

/// Flag indicating if Read/Write memory commands are supported or not
#if defined(CFG_DBG_MEM)
    #define RW_DEBUG_MEM               1
#else //CFG_DBG_MEM
    #define RW_DEBUG_MEM               0
#endif //CFG_DBG_MEM

/// Flag indicating if Flash debug commands are supported or not
#if defined(CFG_DBG_FLASH)
    #define RW_DEBUG_FLASH                  1
#else //CFG_DBG_FLASH
    #define RW_DEBUG_FLASH                  0
#endif //CFG_DBG_FLASH

/// Flag indicating if NVDS feature is supported or not
#if defined(CFG_DBG_NVDS)
    #define RW_DEBUG_NVDS                   1
#else //CFG_DBG_NVDS
    #define RW_DEBUG_NVDS                   0
#endif //CFG_DBG_NVDS

/// Flag indicating if CPU stack profiling commands are supported or not
#if defined(CFG_DBG_STACK_PROF)
    #define RW_DEBUG_STACK_PROF             1
#else
    #define RW_DEBUG_STACK_PROF             0
#endif // defined (CFG_DBG_STACK_PROF)

/// Debug printing
#if (RW_DEBUG)
    #define WARNING(P)                      dbg_warning P
#else
    #define WARNING(P)
#endif //RW_DEBUG

/// Modem back to back setup
#define MODEM2MODEM                          0
/// Special clock testing
#define CLK_WRAPPING                         0

/******************************************************************************************/
/* --------------------------      NVDS SETUP       --------------------------------------*/
/******************************************************************************************/

/// Flag indicating if NVDS feature is supported or not
#if defined(CFG_NVDS)
    #define NVDS_SUPPORT                    1
#else //CFG_DBG_NVDS
    #define NVDS_SUPPORT                    0
#endif //CFG_DBG_NVDS

/******************************************************************************************/
/* --------------------------      MISC SETUP       --------------------------------------*/
/******************************************************************************************/
/// Manufacturer: RivieraWaves SAS
#define RW_COMP_ID                           rom_cfg_table[man_id_pos]//0x00D2 //0x0060         vm: change man_id from RW to dialog

/// Bluetooth technologies version
#define RW_BT40_VERSION                      (6)
#define RW_BT41_VERSION                      (7)

/******************************************************************************************/
/* -------------------------   BT / BLE / BLE HL CONFIG    -------------------------------*/
/******************************************************************************************/

#if (BT_EMB_PRESENT)
#include "rwbt_config.h"    // bt stack configuration
#endif //BT_EMB_PRESENT

#if (BLE_EMB_PRESENT) || (BLE_HOST_PRESENT)
#include "rwble_config.h"   // ble stack configuration
#endif //BLE_EMB_PRESENT

#if (BLE_HOST_PRESENT)
#include "rwble_hl_config.h"  // ble Host stack configuration
#endif //BLE_HOST_PRESENT



/******************************************************************************************/
/* -------------------------   KERNEL SETUP          -------------------------------------*/
/******************************************************************************************/

/// Flag indicating Kernel is supported
#define KE_SUPPORT  (BLE_EMB_PRESENT || BT_EMB_PRESENT || BLE_HOST_PRESENT || BLE_APP_PRESENT)


/// Event types definition
enum KE_EVENT_TYPE
{
    #if DISPLAY_SUPPORT
    KE_EVENT_DISPLAY         ,
    #endif //DISPLAY_SUPPORT

    #if RTC_SUPPORT
    KE_EVENT_RTC_1S_TICK     ,
    #endif //RTC_SUPPORT

    #if BLE_EMB_PRESENT
    KE_EVENT_BLE_CRYPT       ,
    #endif //BLE_EMB_PRESENT

    KE_EVENT_KE_MESSAGE      ,
    KE_EVENT_KE_TIMER        ,

    #if (GTL_ITF)
    KE_EVENT_GTL_TX_DONE     ,
    #endif //(GTL_ITF)


    #if HCI_PRESENT
    KE_EVENT_HCI_TX_DONE     ,
    #endif //HCI_PRESENT

    #if BT_EMB_PRESENT
    KE_EVENT_BT_PSCAN_PROC   ,
    #endif //BT_EMB_PRESENT

    #if BLE_EMB_PRESENT
    KE_EVENT_BLE_EVT_DEFER   ,
    #endif //BLE_EMB_PRESENT

    KE_EVENT_MAX             ,
};

/// Tasks types definition
enum KE_TASK_TYPE
{
#if (BT_EMB_PRESENT)
    // BT Controller Tasks
    TASK_LM,
    TASK_LC,
    TASK_LB,
    TASK_LD,
    TASK_HCI,
#endif // (BT_EMB_PRESENT)

#if (BLE_EMB_PRESENT)
    // Link Layer Tasks
    TASK_LLM ,
    TASK_LLC ,
    TASK_LLD ,
#endif // (BLE_EMB_PRESENT)

#if ((BLE_EMB_PRESENT) || (BT_EMB_PRESENT))
    TASK_DBG,
#endif // ((BLE_EMB_PRESENT) || (BT_EMB_PRESENT))

#if (DISPLAY_SUPPORT)
    TASK_DISPLAY,
#endif // (DISPLAY_SUPPORT)
//VM
//#if (BLE_APP_PRESENT)
    TASK_APP,
//#endif // (BLE_APP_PRESENT)

#if (GTL_ITF)
    TASK_GTL,
#endif // (GTL_ITF)

#if (BLE_HOST_PRESENT)
    TASK_L2CC,    // L2CAP Controller Task
    TASK_GATTM,   // Generic Attribute Profile Manager Task
    TASK_GATTC,   // Generic Attribute Profile Controller Task
    TASK_GAPM,    // Generic Access Profile Manager
    TASK_GAPC,    // Generic Access Profile Controller

    #if (RWBLE_SW_VERSION_MINOR < 1)
    // allocate a certain number of profiles task
    TASK_PRF_MAX = (TASK_GAPC + BLE_NB_PROFILES),
    #else
    TASK_RFU_1,
    TASK_RFU_2,
    TASK_RFU_3,
    TASK_RFU_4,
    TASK_RFU_5,

    // allocate a certain number of profiles task
    TASK_PRF_MAX = (TASK_RFU_5 + BLE_NB_PROFILES),
    #endif /* (RWBLE_SW_VERSION_MINOR < 1) */
#endif // (BLE_HOST_PRESENT)


    /// Maximum number of tasks
    TASK_MAX,

    TASK_NONE = 0xFF,
};


/// Tasks types definition, this value shall be in [0-254] range
enum KE_API_ID
{
    // Link Layer Tasks
    TASK_ID_LLM          = 0,
    TASK_ID_LLC          = 1,
    TASK_ID_LLD          = 2,
    TASK_ID_DBG          = 3,

    // BT Controller Tasks
    TASK_ID_LM           = 4,
    TASK_ID_LC           = 5,
    TASK_ID_LB           = 6,
    TASK_ID_LD           = 7,

    TASK_ID_HCI          = 8,
    TASK_ID_DISPLAY      = 9,

    TASK_ID_L2CC         = 10,
    TASK_ID_GATTM        = 11,   // Generic Attribute Profile Manager Task
    TASK_ID_GATTC        = 12,   // Generic Attribute Profile Controller Task
    TASK_ID_GAPM         = 13,   // Generic Access Profile Manager
    TASK_ID_GAPC         = 14,   // Generic Access Profile Controller

    TASK_ID_APP          = 15,
    TASK_ID_GTL          = 16,

    // -----------------------------------------------------------------------------------
    // --------------------- BLE Profile TASK API Identifiers ----------------------------
    // -----------------------------------------------------------------------------------
    TASK_ID_DISS         = 20,   // Device Information Service Server Task
    TASK_ID_DISC         = 21,   // Device Information Service Client Task

    TASK_ID_PROXM        = 22,   // Proximity Monitor Task
    TASK_ID_PROXR        = 23,   // Proximity Reporter Task

    TASK_ID_FINDL        = 24,   // Find Me Locator Task
    TASK_ID_FINDT        = 25,   // Find Me Target Task

    TASK_ID_HTPC         = 26,   // Health Thermometer Collector Task
    TASK_ID_HTPT         = 27,   // Health Thermometer Sensor Task

    TASK_ID_BLPS         = 28,   // Blood Pressure Sensor Task
    TASK_ID_BLPC         = 29,   // Blood Pressure Collector Task

    TASK_ID_HRPS         = 30,   // Heart Rate Sensor Task
    TASK_ID_HRPC         = 31,   // Heart Rate Collector Task

    TASK_ID_TIPS         = 32,   // Time Server Task
    TASK_ID_TIPC         = 33,   // Time Client Task

    TASK_ID_SCPPS        = 34,   // Scan Parameter Profile Server Task
    TASK_ID_SCPPC        = 35,   // Scan Parameter Profile Client Task

    TASK_ID_BASS         = 36,   // Battery Service Server Task
    TASK_ID_BASC         = 37,   // Battery Service Client Task

    TASK_ID_HOGPD        = 38,   // HID Device Task
    TASK_ID_HOGPBH       = 39,   // HID Boot Host Task
    TASK_ID_HOGPRH       = 40,   // HID Report Host Task

    TASK_ID_GLPS         = 41,   // Glucose Profile Sensor Task
    TASK_ID_GLPC         = 42,   // Glucose Profile Collector Task

    TASK_ID_RSCPS        = 43,   // Running Speed and Cadence Profile Server Task
    TASK_ID_RSCPC        = 44,   // Running Speed and Cadence Profile Collector Task

    TASK_ID_CSCPS        = 45,   // Cycling Speed and Cadence Profile Server Task
    TASK_ID_CSCPC        = 46,   // Cycling Speed and Cadence Profile Client Task

    TASK_ID_ANPS         = 47,   // Alert Notification Profile Server Task
    TASK_ID_ANPC         = 48,   // Alert Notification Profile Client Task

    TASK_ID_PASPS        = 49,   // Phone Alert Status Profile Server Task
    TASK_ID_PASPC        = 50,   // Phone Alert Status Profile Client Task

    TASK_ID_CPPS         = 51,   // Cycling Power Profile Server Task
    TASK_ID_CPPC         = 52,   // Cycling Power Profile Client Task

    TASK_ID_LANS         = 53,   // Location and Navigation Profile Server Task
    TASK_ID_LANC         = 54,   // Location and Navigation Profile Client Task


    TASK_ID_INVALID      = 0xFF, // Invalid Task Identifier
};


/// Kernel memory heaps types.
enum
{
    /// Memory allocated for environment variables
    KE_MEM_ENV,
    #if (BLE_HOST_PRESENT)
    /// Memory allocated for Attribute database
    KE_MEM_ATT_DB,
    #endif // (BLE_HOST_PRESENT)
    /// Memory allocated for kernel messages
    KE_MEM_KE_MSG,
    /// Non Retention memory block
    KE_MEM_NON_RETENTION,
    KE_MEM_BLOCK_MAX,
};



#if (BT_EMB_PRESENT)
#define BT_HEAP_MSG_SIZE_      BT_HEAP_MSG_SIZE
#define BT_HEAP_ENV_SIZE_      BT_HEAP_ENV_SIZE
#else

#define BT_HEAP_MSG_SIZE_      0
#define BT_HEAP_ENV_SIZE_      0
#endif //BT_EMB_PRESENT

#if (BLE_EMB_PRESENT)
#define BLE_HEAP_MSG_SIZE_     BLE_HEAP_MSG_SIZE
#define BLE_HEAP_ENV_SIZE_     BLE_HEAP_ENV_SIZE
#else
#define BLE_HEAP_MSG_SIZE_     0
#define BLE_HEAP_ENV_SIZE_     0
#endif //BLE_EMB_PRESENT

#if (BLE_HOST_PRESENT)

#define BLEHL_HEAP_MSG_SIZE_   BLEHL_HEAP_MSG_SIZE
#define BLEHL_HEAP_ENV_SIZE_   BLEHL_HEAP_ENV_SIZE
#define BLEHL_HEAP_DB_SIZE_    BLEHL_HEAP_DB_SIZE
#else
#define BLEHL_HEAP_MSG_SIZE_   0
#define BLEHL_HEAP_ENV_SIZE_   0
#define BLEHL_HEAP_DB_SIZE_    0
#endif //BLE_HOST_PRESENT


/// Kernel Message Heap
#define RWIP_HEAP_MSG_SIZE         (  BT_HEAP_MSG_SIZE_      + \
                                    BLE_HEAP_MSG_SIZE_     + \
                                    BLEHL_HEAP_MSG_SIZE_      )

#define KE_NB_LINK_IN_HEAP_ENV   4

/// Size of Environment heap
#define RWIP_HEAP_ENV_SIZE         (( BT_HEAP_ENV_SIZE_         + \
                                      BLE_HEAP_ENV_SIZE_        + \
                                      BLEHL_HEAP_ENV_SIZE_       )\
                                      * KE_NB_LINK_IN_HEAP_ENV)

/// Size of Attribute database heap
#define RWIP_HEAP_DB_SIZE         (  BLEHL_HEAP_DB_SIZE  )

/// Size of non retention heap - 1024 bytes per ble link should be sufficient and can be tuned
#if (BLE_EMB_PRESENT)
#define RWIP_HEAP_NON_RET_SIZE    ( 1024 * BLE_CONNECTION_MAX )
#else
#define RWIP_HEAP_NON_RET_SIZE    ( 1024 )
#endif

#define MAX_TL_PENDING_PACKETS_ADV rom_cfg_table[max_tl_pending_packets_adv_pos] //= 50;
#define MAX_TL_PENDING_PACKETS rom_cfg_table[max_tl_pending_packets_pos] //= 60;


/// @} BT Stack Configuration
/// @} ROOT

#endif //RWIP_CONFIG_H_
