/**
****************************************************************************************
*
* @file EA.h
*
* @brief EA main module
*
* Copyright (C) RivieraWaves 2009-2014
*
*
****************************************************************************************
*/

#ifndef _EA_H_
#define _EA_H_

/**
 ****************************************************************************************
 * @addtogroup EA
 * @brief Entry points of the Event Arbiter module
 *
 * This module contains the primitives that allow stacks to schedule an event or frame.
 *
 * @{
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (EA_PRESENT)

#include <stdint.h>               // Standard integer definitions
#include <stdbool.h>              // Standard boolean definitions
#include "co_list.h"              // List management functions definitions


#if (BT_EMB_PRESENT)
#define EA_ALARM_SUPPORT      1
#else
#define EA_ALARM_SUPPORT      0
#endif //(BT_EMB_PRESENT)

/*
 * MACROS
 ****************************************************************************************
 */



/*
 * DEFINES
 ****************************************************************************************
 */

/// Default BW 2 slots
#define EA_BW_USED_DFT              rom_cfg_table[ea_be_used_dft_pos] //(2)
/// Clock correction delay
#define EA_CLOCK_CORR_LAT           rom_cfg_table[ea_clock_corr_lat_pos] //(2) //this one makes the difference for sleep to work

/// Set ASAP fields
#define EA_ASAP_FIELD_SET(TYPE,LIMIT,PAR,FIELD) FIELD = ((((TYPE) << 30) & 0xC0000000) | (((PAR) << 29) & 0x20000000) | ((LIMIT) & 0x07FFFFFF))

/// Get type from an ASAP field
#define EA_ASAP_FIELD_GET_TYPE(FIELD) ((uint8_t)((FIELD >> 30) & 0x3))

/// Get slot parity from an ASAP field
#define EA_ASAP_FIELD_GET_PAR(FIELD) ((uint8_t)((FIELD >> 29) & 0x1))

/// Get limit timestamp from an ASAP field
#define EA_ASAP_FIELD_GET_LIMIT(FIELD) ((uint32_t)(FIELD & 0x07FFFFFF))

/// Clear an ASAP field
#define EA_ASAP_FIELD_CLEAR(FIELD) FIELD = 0

/*
 * DEFINITIONS
 ****************************************************************************************
 */

enum ea_error
{
    EA_ERROR_OK                 = 0,
    EA_ERROR_PROG,
    EA_ERROR_REJECTED,
    EA_ERROR_CANCELED,
    EA_ERROR_NOT_FOUND,
    EA_ERROR_ALREADY_FREE,
    EA_ERROR_BW_FULL
};

/// Action for the parameters request API
enum ea_param_req_action
{
    EA_PARAM_REQ_GET,
    EA_PARAM_REQ_CHECK,
};

/// ASAP type definition
enum ea_elt_asap_type
{
    EA_FLAG_NO_ASAP                = 0,
    EA_FLAG_ASAP_NO_LIMIT,
    EA_FLAG_ASAP_LIMIT,
    EA_FLAG_MAX
};

/// ASAP slot parity definition
enum ea_elt_asap_parity
{
    EA_EVEN_SLOT,
    EA_ODD_SLOT,
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */



/// Event Arbiter Element
struct ea_elt_tag
{
    /// List element for chaining in the Even Arbiter lists
    struct co_list_hdr hdr;

    /// Pointer on the next element linked to the current action
    struct ea_elt_tag *linked_element;

    /// Programming time in basetime (CLOCKN) absolute value
    uint32_t timestamp;

    /**
     * ASAP field contains the type and the limit
     * bit |31  30|   29   |28 27|26 ...................... 0|
     * def | TYPE | Parity | RFU | LIMIT (absolute time)     |
     *
     * Type:
     *  - 00: No ASAP
     *  - 01: ASAP no limit
     *  - 10: ASAP with limit
     *
     * Parity (only for ASAP requests):
     *  - 0: even slots
     *  - 1: odd slots
     *
     * Limit (only for ASAP LIMIT requests):
     *  - Absolute value in slots
     *  - The reservation can not cross over this limit
     */
    uint32_t asap_field;

    /// Minimum duration of the event or frame (in slots)
    uint16_t duration_min;

    /// Current priority
    uint8_t current_prio;
    /// Threshold1 for prevent stop
    uint8_t prev_stop_thr1;
    /// Threshold2 for prevent stop
    uint8_t prev_stop_thr2;
    /// Threshold for prevent start
    uint8_t prev_start_thr;

    /************************************************************************************
     * ISR CALLBACKS
     ************************************************************************************/

    /// Start event or frame call back function
    void (*ea_cb_prevent_start)(struct ea_elt_tag*);
    /// Prevent stop call back function
    void (*ea_cb_prevent_stop)(struct ea_elt_tag*);
    ///  event or frame canceling call back function
    void (*ea_cb_cancel)(struct ea_elt_tag*);

    /// BT/BLE Specific environment variable
    void *env;
};

/// Interval element strcuture
struct ea_interval_tag
{
    /// List element for chaining in the Interval list
    struct co_list_hdr hdr;
    /// Interval used
    uint16_t interval_used;
    /// Offset used
    uint16_t offset_used;
    /// Bandwidth used
    uint16_t bandwidth_used;
    /// Connection handle used
    uint16_t conhdl_used;
    /// Role used
    uint16_t role_used;
    /// Odd offset or even offset
    bool odd_offset;
    /// Link id
    uint16_t linkid;
};

/// API get/check parameters input
struct ea_param_input
{
    /// Interval minimum requested
    uint16_t interval_min;
    /// Interval maximum requested
    uint16_t interval_max;
    /// Duration minimum requested
    uint16_t duration_min;
    /// Duration maximum requested
    uint16_t duration_max;
    /// Preferred periodicity
    uint8_t pref_period;
    /// Offset requested
    uint16_t offset;
    /// action
    uint8_t action;
    /// Connection handle
    uint16_t conhdl;
    /// Role
    uint16_t role;
    /// Odd offset or even offset
    bool odd_offset;
    /// Link id
    uint16_t linkid;
};

/// API get/check parameters output
struct ea_param_output
{
    /// Interval returned
    uint16_t interval;
    /// Duration returned
    uint16_t duration;
    /// Offset returned
    uint16_t offset;
};

#if (EA_ALARM_SUPPORT)
/// Alarm element structure
struct ea_alarm_tag
{
    /// List element for chaining in the Even Arbiter lists
    struct co_list_hdr hdr;

    /// Timestamp of alarm expiry
    uint32_t timestamp;

    /// Call back function invoked upon alarm expiry
    void (*ea_cb_alarm)(struct ea_alarm_tag*);
};
#endif //(EA_ALARM_SUPPORT)

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize the Event Arbiter.
 *
 * @param[in] reset         True if software reset
 *
 ****************************************************************************************
 */
void ea_init(bool reset);

/**
 ****************************************************************************************
 * @brief Return the current time half slot boundary rounded
 *
 * @return Current time (in BT slots)
 ****************************************************************************************
 */
uint32_t ea_time_get_halfslot_rounded(void);

/**
 ****************************************************************************************
 * @brief Return the current time slot boundary rounded
 *
 * @return Current time (in BT slots)
 ****************************************************************************************
 */
uint32_t ea_time_get_slot_rounded(void);

/**
 ****************************************************************************************
 * @brief Create a new event element
 *
 * @param[in] size_of_env          Size of the dedicated environment structure
 *
 * @return Pointer on the allocated element
 ****************************************************************************************
 */
struct ea_elt_tag *ea_elt_create(uint16_t size_of_env);

/**
 ****************************************************************************************
 * @brief API to try to insert a new element in the scheduler queue
 *
 * @param[in] elt           Pointer to the element to be inserted
 *
 * @return No error if element is inserted.
 ****************************************************************************************
 */
uint8_t ea_elt_insert(struct ea_elt_tag *elt);

/**
 ****************************************************************************************
 * @brief API to try to delete an element in the wait or program queue
 *
 * @param[in] elt           Pointer to the element to be deleted
 *
 * @return No error if element is found and removed.
 ****************************************************************************************
 */
uint8_t ea_elt_delete(struct ea_elt_tag *elt);

/**
 ****************************************************************************************
 * @brief API to try to remove the current element programmed
 *
 * @param[in] elt           Pointer to the element to be removed from current ptr
 *
 ****************************************************************************************
 */
uint8_t ea_elt_remove(struct ea_elt_tag *elt);

/**
 ****************************************************************************************
 * @brief Start of event/frame interrupt handler
 *
 * This function is called under interrupt when a start of event/frame interrupt is
 * generated by the BLE/BT core.
 *
 ****************************************************************************************
 */
void ea_finetimer_isr(void);

/**
 ****************************************************************************************
 * @brief Software interrupt handler
 *
 * This function is called under interrupt when a SW interrupt is generated by the BLE/BT
 * core.
 *
 ****************************************************************************************
 */
void ea_sw_isr(void);

/**
 ****************************************************************************************
 * @brief Create a new interval element
 *
 * @return Pointer on the allocated element
 ****************************************************************************************
 */
struct ea_interval_tag *ea_interval_create(void);

/**
 ****************************************************************************************
 * @brief API to try to remove an interval from the queue
 *
 * @param[in] interval_to_remove     Interval to be removed
 *
 ****************************************************************************************
 */
void ea_interval_delete(struct ea_interval_tag *interval_to_remove);

/**
 ****************************************************************************************
 * @brief Insert the interval in the common interval queue
 *
 ****************************************************************************************
 */
void ea_interval_insert(struct ea_interval_tag *interval_to_add);

/**
 ****************************************************************************************
 * @brief Check if sleep mode is possible
 *
 * The function takes as argument the allowed sleep duration that must not be increased.
 * If EA needs an earlier wake-up than initial duration, the allowed sleep duration
 * is updated.
 * If EA needs a shorter duration than the wake-up delay, sleep is not possible and
 * the function returns a bad status.
 *
 * @param[in/out] sleep_duration   Initial allowed sleep duration (in slot of 625us)
 * @param[in]     wakeup_delay     Delay for system wake-up (in slot of 625us)
 *
 * @return true if sleep is allowed, false otherwise
 ****************************************************************************************
 */
bool ea_sleep_check(uint32_t *sleep_duration, uint32_t wakeup_delay);

/**
 ****************************************************************************************
 * @brief Choose an appropriate offset according to the interval
 *
 * @param[in] input_param  parameters used to compute the offset
 *
 * @param[in] output_param offset used for the interval
 *
 * @return true if an offset has been found or not
 ****************************************************************************************
 */
uint8_t ea_offset_req(struct ea_param_input* input_param, struct ea_param_output* output_param);
/**
 ****************************************************************************************
 * @brief Choose an appropriate interval and duration for the event
 *
 * @param[in] input_param  parameter used to compute the interval and duration of the event
 *
 * @param[in] output_param interval and duration chosen for the event
 *
 ****************************************************************************************
 */
void ea_interval_duration_req(struct ea_param_input* input_param, struct ea_param_output* output_param);

#if (EA_ALARM_SUPPORT)
/**
 ****************************************************************************************
 * @brief Set an alarm
 *
 * @param[in] elt      Pointer to the alarm element to be programmed
 *
 * @return 0 - no error / 1:255 - error
 ****************************************************************************************
 */
uint8_t ea_alarm_set(struct ea_alarm_tag* elt);

/**
 ****************************************************************************************
 * @brief Clear an alarm
 *
 * @param[in] elt      Pointer to the alarm element to be cleared
 *
 * @return 0 - no error / 1:255 - error
 ****************************************************************************************
 */
uint8_t ea_alarm_clear(struct ea_alarm_tag* elt);
#endif //(EA_ALARM_SUPPORT)

#endif //(EA_PRESENT)

///@} EA

#endif // _EA_H_
