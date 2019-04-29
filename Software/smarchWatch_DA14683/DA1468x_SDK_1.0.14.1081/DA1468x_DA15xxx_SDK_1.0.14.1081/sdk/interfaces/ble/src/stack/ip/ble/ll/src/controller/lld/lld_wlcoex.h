/**
****************************************************************************************
*
* @file lld_wlcoex.h
*
* @brief Wireless LAN Coexistence mailbox functions
*
* Copyright (C) RivieraWaves 2009-2014
*
*
****************************************************************************************
*/
#ifndef LLD_WLCOEX_H_
#define LLD_WLCOEX_H_
/**
****************************************************************************************
* @defgroup LDWLCOEX WIFI Coexistence
* @ingroup LD
* @brief Responsible for WIFI coexistence.
* @{
****************************************************************************************
*/

/*
 * #define CONSTANTS
 ****************************************************************************************
 */
#if (RW_BLE_WLAN_COEX)
/// Coexistence disabled
#define BLECOEX_DISABLED          0
/// Coexistence WLAN enabled
#define BLECOEX_WLAN              1

/// Coexistence Definitions
#define BLEMPRIO_CONREQ     0
#define BLEMPRIO_LLCP       1
#define BLEMPRIO_DATA       2
#define BLEMPRIO_INITSC     3
#define BLEMPRIO_ACTSC      4
#define BLEMPRIO_CONADV     5
#define BLEMPRIO_NCONADV    6
#define BLEMPRIO_PASSC      7
#endif // RW_BLE_WLAN_COEX

#if (RW_BLE_WLAN_COEX_TEST)
#define    BLE_WLCOEX_TST_MSK              0xFFFF0000
#define    BLE_WLCOEX_TST_NCONADV_PASSC    0x00010000
#define    BLE_WLCOEX_TST_CONADV_ACTSC     0x00020000
#define    BLE_WLCOEX_TST_INITSC           0x00040000
#define    BLE_WLCOEX_TST_INITCONREQ       0x00080000
#define    BLE_WLCOEX_TST_ACTCON           0x00100000
#endif // RW_BLE_WLAN_COEX_TEST

/*
 * VARIABLE DECLARATION
 ****************************************************************************************
 */
#if (RW_BLE_WLAN_COEX)
extern bool lld_wlcoex_enable;

#if (RW_BLE_WLAN_COEX_TEST)
extern uint32_t lld_wlcoex_scenario;
#endif // RW_BLE_WLAN_COEX_TEST

/*
 * FUNCTION PROTOTYPES
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Wireless LAN coexistence update when a new connection occurs.
 *
 ****************************************************************************************
 */

void lld_wlcoex_connection_complete(void);
/**
 ****************************************************************************************
 * @brief Wireless LAN coexistence update when a connection ends.
 *
 ****************************************************************************************
 */
void lld_wlcoex_remove_connection(void);
/**
 ****************************************************************************************
 * @brief Enable/Disable the Wireless LAN coexistence interface.
 *
 * @param[in] CoexSetting    Coexistence value
 *
 * @return Status
 *
 ****************************************************************************************
 */
void lld_wlcoex_set(uint8_t CoexSetting);

#if (RW_BLE_WLAN_COEX_TEST)
/**
 ****************************************************************************************
 * @brief Set the scenario for the unitary testing.
 *
 * @param[in] scenario    Scenario type
 *
 * @return none
 *
 ****************************************************************************************
 */
void lld_wlcoex_scen_set(uint32_t scenario);
#endif // RW_BLE_WLAN_COEX_TEST
#endif // RW_BLE_WLAN_COEX

///@} LLDWLCOEX

#endif // LLD_WLCOEX_H_
