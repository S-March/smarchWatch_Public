/**
****************************************************************************************
*
* @file rwip.h
*
* @brief RW IP SW main module
*
* Copyright (C) RivieraWaves 2009-2014
*
*
****************************************************************************************
*/
#ifndef _RWIP_H_
#define _RWIP_H_

/**
 ****************************************************************************************
 * @addtogroup ROOT
 * @brief Entry points of the RW IP stacks/modules
 *
 * This module contains the primitives that allow an application accessing and running the
 * RW IP protocol stacks / modules.
 *
 * @{
 ****************************************************************************************
 */

#include "rwip_config.h"          // stack configuration
#include "co_version.h"

#include <stdint.h>               // standard integer definitions
#include <stdbool.h>              // standard boolean definitions

/// RWBT Environment
struct rwip_env_tag
{
    uint16_t prevent_sleep;
    uint8_t wakeup_delay;
    bool sleep_enable;
    uint8_t ext_wakeup_enable;
};

extern struct rwip_env_tag rwip_env;

#if (DEEP_SLEEP)
/// Definition of the bits preventing the system from sleeping
enum prevent_sleep
{
    /// Flag indicating that the wake up process is ongoing
    RW_WAKE_UP_ONGOING = 0x0001,
    /// Flag indicating that an TX transfer is ongoing on Transport Layer
    RW_TL_TX_ONGOING = 0x0002,
    /// Flag indicating that an RX transfer is ongoing on Transport Layer
    RW_TL_RX_ONGOING = 0x0004,
    /// Flag indicating HCI timeout is ongoing
    RW_GTL_TIMEOUT = 0x0008,
    /// Flag indicating that an encryption is ongoing
    RW_CRYPT_ONGOING = 0x0010,
    /// Flag indicating that a element deletion is on going
    RW_DELETE_ELT_ONGOING = 0x0020,
#if (RWBLE_SW_VERSION_MAJOR >= 8)
    /// Flag indicating that an ECC operation is ongoing
    RW_P256_ONGOING = 0x0040,
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */
};
#endif //DEEP_SLEEP



/**
 * External interface type types.
 */
enum rwip_eif_types
{
    /// Host Controller Interface - Controller part
    RWIP_EIF_HCIC,

    /// Host Controller Interface - Host part
    RWIP_EIF_HCIH,

    /// Application Host interface
    RWIP_EIF_AHI,
};


/// Enumeration of External Interface status codes
enum rwip_eif_status
{
    /// EIF status OK
    RWIP_EIF_STATUS_OK,
    /// EIF status KO
    RWIP_EIF_STATUS_ERROR,

#if (BLE_EMB_PRESENT == 0)
    /// External interface detached
    RWIP_EIF_STATUS_DETACHED,
    /// External interface attached
    RWIP_EIF_STATUS_ATTACHED,
#endif // (BLE_EMB_PRESENT == 0)
};

/// Enumeration of RF modulations
enum rwip_rf_mod
{
    MOD_GFSK  = 0x01,
    MOD_DQPSK = 0x02,
    MOD_8DPSK = 0x03,
};

/// API functions of the RF driver that are used by the BLE or BT software
struct rwip_rf_api
{
    /// Function called upon HCI reset command reception
    void (*reset)(void);
    /// Function called to enable/disable force AGC mechanism (true: en / false : dis)
    void (*force_agc_enable)(bool);
    /// Function called when TX power has to be increased for a specific link id
    bool (*txpwr_inc)(uint8_t);
    /// Function called when TX power has to be decreased for a specific link id
    bool (*txpwr_dec)(uint8_t);
    /// Function called to execute an EPC request for a specific link id
    uint8_t (*txpwr_epc_req)(uint8_t, uint8_t);
    /// Function called to convert a TX power CS power field into the corresponding value in dBm
    uint8_t (*txpwr_dbm_get)(uint8_t, uint8_t);
    /// Function called to convert a power in dBm into a control structure tx power field
    uint8_t (*txpwr_cs_get)(int8_t);
    /// Function called to convert the RSSI read from the control structure into a real RSSI
    uint8_t (*rssi_convert)(uint8_t);
    /// Function used to handle RF interrupt
    void (*isr)(void);
    /// Function used to read a RF register
    uint32_t (*reg_rd)(uint16_t);
    /// Function used to write a RF register
    void (*reg_wr)(uint16_t, uint32_t);
    /// Function called to put the RF in deep sleep mode
    void (*sleep)(void);
    /// Index of maximum TX power
    uint8_t txpwr_max;
    /// RSSI high threshold
    uint8_t rssi_high_thr;
    /// RSSI low threshold
    uint8_t rssi_low_thr;
    /// interferer threshold
    uint8_t rssi_interf_thr;
    /// RF wakeup delay (in slots)
    uint8_t wakeup_delay;
};


/**
 ****************************************************************************************
 * @brief Function called when packet transmission/reception is finished.
 *
 * @param[in]  status ok if action correctly performed, else reason status code.
 *****************************************************************************************
 */
typedef void (*rwip_eif_callback) (uint8_t);

/**
 * Transport layer communication interface.
 */
struct rwip_eif_api
{
    /**
     *************************************************************************************
     * @brief Starts a data reception.
     *
     * @param[out] bufptr      Pointer to the RX buffer
     * @param[in]  size        Size of the expected reception
     * @param[in]  callback    Pointer to the function called back when transfer finished
     *************************************************************************************
     */
    void (*read) (uint8_t *bufptr, uint32_t size, rwip_eif_callback callback);

    /**
     *************************************************************************************
     * @brief Starts a data transmission.
     *
     * @param[in]  bufptr      Pointer to the TX buffer
     * @param[in]  size        Size of the transmission
     * @param[in]  callback    Pointer to the function called back when transfer finished
     *************************************************************************************
     */
    void (*write)(uint8_t *bufptr, uint32_t size, rwip_eif_callback callback);

    /**
     *************************************************************************************
     * @brief Enable Interface flow.
     *************************************************************************************
     */
    void (*flow_on)(void);

    /**
     *************************************************************************************
     * @brief Disable Interface flow.
     *
     * @return True if flow has been disabled, False else.
     *************************************************************************************
     */
    bool (*flow_off)(void);
};

/*
 * VARIABLE DECLARATION
*****************************************************************************************
 */

/// API for RF driver
extern struct rwip_rf_api rwip_rf;

/*
 * FUNCTION DECLARATION
*****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initializes the RW BT SW.
 *
 ****************************************************************************************
 */
void rwip_init(uint32_t error);

/**
 ****************************************************************************************
 * @brief Reset the RW BT SW.
 *
 ****************************************************************************************
 */
void rwip_reset(void);

/**
 ****************************************************************************************
 * @brief Gives FW/HW versions of RW-BT stack.
 *
 ****************************************************************************************
 */
void rwip_version(uint8_t* fw_version, uint8_t* hw_version);

/**
 ****************************************************************************************
 * @brief Schedule all pending events.
 *
 ****************************************************************************************
 */
void rwip_schedule(void);

/**
 ****************************************************************************************
 * @brief Invoke the sleep function.
 *
 * @return  true: processor sleep allowed, false otherwise
 ****************************************************************************************
 */
bool rwip_sleep(void);

/**
 ****************************************************************************************
 * @brief Send an error message to Host.
 *
 * This function is used to send an error message to Host from platform.
 *
 * @param[in] error    Error detected by FW
 ****************************************************************************************
 */
void rwip_send_message(uint32_t error);


#if DEEP_SLEEP
/**
 ****************************************************************************************
 * @brief Handle wake-up.
 ****************************************************************************************
 */
void rwip_wakeup(void);

/**
 ****************************************************************************************
 * @brief Handle end of wake-up.
 ****************************************************************************************
 */
void rwip_wakeup_end(void);

/**
 ****************************************************************************************
 * @brief Set the wake-up delay
 *
 * @param[in] wakeup_delay   Wake-up delay in us
 ****************************************************************************************
 */
void rwip_wakeup_delay_set(uint16_t wakeup_delay);


/**
 ****************************************************************************************
 * @brief Set a bit in the prevent sleep bit field, in order to prevent the system from
 *        going to sleep
 *
 * @param[in] prv_slp_bit   Bit to be set in the prevent sleep bit field
 ****************************************************************************************
 */
void rwip_prevent_sleep_set(uint16_t prv_slp_bit);

/**
 ****************************************************************************************
 * @brief Clears a bit in the prevent sleep bit field, in order to allow the system
 *        going to sleep
 *
 * @param[in] prv_slp_bit   Bit to be cleared in the prevent sleep bit field
 ****************************************************************************************
 */
void rwip_prevent_sleep_clear(uint16_t prv_slp_bit);

/**
 ****************************************************************************************
 * @brief Check if sleep mode is enable
 *
 * @return    true if sleep is enable, false otherwise
 ****************************************************************************************
 */
bool rwip_sleep_enable(void);

/**
 ****************************************************************************************
 * @brief Check if external wake-up is enable
 *
 * @return    true if external wakeup is enable, false otherwise
 ****************************************************************************************
 */
bool rwip_ext_wakeup_enable(void);
#endif // DEEP_SLEEP



/**
 ****************************************************************************************
 * @brief Function to implement in platform in order to retrieve expected external
 * interface such as UART for Host Control Interface.
 *
 * @param[in] type external interface type (@see rwip_eif_types)
 *
 * @return External interface api structure
 ****************************************************************************************
 */
extern const struct rwip_eif_api* rwip_eif_get(uint8_t type);


///@} ROOT

#endif // _RWIP_H_
