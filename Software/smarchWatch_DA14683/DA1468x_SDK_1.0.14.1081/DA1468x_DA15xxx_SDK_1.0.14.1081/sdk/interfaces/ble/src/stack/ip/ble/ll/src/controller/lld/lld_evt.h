/**
 ****************************************************************************************
 *
 * @file lld_evt.h
 *
 * @brief Declaration of functions used for event scheduling
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef LLD_EVT_H_
#define LLD_EVT_H_

/**
 ****************************************************************************************
 * @addtogroup LLDEVT
 * @ingroup LLD
 * @brief Event scheduling functions
 *
 * This module implements the primitives used for event scheduling
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include <stdint.h>
#include <stdbool.h>
#include "rwip_config.h"
#include "co_list.h"
#include "co_version.h"
#include "reg_blecore.h"
#include "llm.h"
#include "llc_task.h"


/*
 * MACROS
 ****************************************************************************************
 */

/// Get BLE Event environment address from an element
#define LLD_EVT_ENV_ADDR_GET(elt)        \
                ((struct lld_evt_tag *)(&(elt->env)))


/*
 * DEFINES
 ****************************************************************************************
 */

/// Size of the LLD event table. It shall be equal to the max number of supported
/// connections * 2 + 1 for the scanning/advertising
#define LLD_EVT_TAB_SIZE  (BLE_CONNECTION_MAX * 2 + 1)

/// Invalid value for an interval
#define LLD_EVT_INTERVAL_INVALID  0xFFFF

/// Maximum value an interval can take. This value is for time comparison
#define MAX_INTERVAL_TIME  3193600  // Max number of 625us periods for a connection interval

/// Default RX window size
#define LLD_EVT_DEFAULT_RX_WIN_SIZE     (14)
/// Slot Duration
#define LLD_EVT_SLOT_DURATION           (625)
/// Frame Duration
#define LLD_EVT_FRAME_DURATION          (1250)
/// Max Finecounter value
#define LLD_EVT_FINECNT_MAX             (LLD_EVT_SLOT_DURATION - 1)
/// MAX LP Clock Jitter allowed by the specification (Core 4.0 - vol 6, -B - 4.2.2)
#define LLD_EVT_MAX_JITTER              (16)
/// Duration of IFS (150 us)
#define LLD_EVT_IFS_DURATION            (150)
/// Synchronization Word Duration
#define LLD_EVT_SYNC_WORD_DURATION      (40)
/// CONNECT_REQ PDU duration
#define LLD_EVT_CONNECT_REQ_DURATION    (312)

/// Default RX window offset
#define LLD_EVT_RX_WIN_DEFAULT_OFFSET   (LLD_EVT_SYNC_WORD_DURATION - (LLD_EVT_DEFAULT_RX_WIN_SIZE / 2))

/// Maximum duration of a sleep, in low power clock cycles (around 300s)
#define LLD_EVT_MAX_SLEEP_DURATION      12000000

/// Default sleep duration when no event is programmed (in slot count)
#define LLD_EVT_DEFAULT_SLEEP_DURATION  8000

/// Maximum slave latency supported when total SCA is 1000ppm
#define LLD_EVT_MAX_LATENCY             450

/// Duration of Event Abort Counter (475 us) (Slot duration (625us) - IFS duration(150us) + Margin(10us to avoid prefetch in same time than abort))
#define LLD_EVT_ABORT_CNT_DURATION      rom_cfg_table[lld_evt_abort_cnt_duration_pos] //(485)

/*****************************************************************************************
 * Event Flags (Status)
 ****************************************************************************************/
/// Flag forcing the slave to wait for the next acknowledgment
#define LLD_EVT_FLAG_WAITING_ACK        (0x01)
/// Flag forcing the slave to wait for the next sync with the master
#define LLD_EVT_FLAG_WAITING_SYNC       (LLD_EVT_FLAG_WAITING_ACK << 1)
/// Flag forcing the slave to wake up for a programmed transmission
#define LLD_EVT_FLAG_WAITING_TXPROG     (LLD_EVT_FLAG_WAITING_SYNC << 1)
/// Flag forcing the slave to wake up at instant
#define LLD_EVT_FLAG_WAITING_INSTANT    (LLD_EVT_FLAG_WAITING_TXPROG << 1)
/// Delete the event after next End of Event ISR
#define LLD_EVT_FLAG_DELETE             (LLD_EVT_FLAG_WAITING_INSTANT<< 1)
/// Do not restart the element
#define LLD_EVT_FLAG_NO_RESTART         (LLD_EVT_FLAG_DELETE << 1)
/// Indicate that end of event is due to an APFM interrupt
#define LLD_EVT_FLAG_APFM               (LLD_EVT_FLAG_NO_RESTART << 1)
/// Indicate that a pending event should be sent to the host
#define LLD_EVT_FLAG_EVT_TO_HOST         (LLD_EVT_FLAG_APFM << 1)
/*
 * MACROS
 ****************************************************************************************
 */

/// Set Event status flag
#define LLD_EVT_FLAG_SET(evt, flag)          \
                (evt->evt_flag |= (LLD_EVT_FLAG_ ## flag))
/// Reset Event status flag
#define LLD_EVT_FLAG_RESET(evt, flag)        \
                (evt->evt_flag &= (~(LLD_EVT_FLAG_ ## flag)))
/// Get Event status flag
#define LLD_EVT_FLAG_GET(evt, flag)          \
                (evt->evt_flag & (LLD_EVT_FLAG_ ## flag))

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

enum lld_evt_mode
{
    LLD_EVT_ADV_MODE             = 0,
    LLD_EVT_SCAN_MODE,
    LLD_EVT_TEST_MODE,
    LLD_EVT_MST_MODE,
    LLD_EVT_SLV_MODE,

    LLD_EVT_MODE_MAX
};

enum lld_evt_instant_action
{
    LLD_EVT_NO_ACTION           = 0,
    LLD_EVT_PARAM_UPDATE,
    LLD_EVT_CHMAP_UPDATE
};

/// Type of events - Format value set in the Control Structure
enum lld_evt_cs_format
{
    // Master Connect
    LLD_MASTER_CONNECTED     = 0x02,
    // Slave Connect
    LLD_SLAVE_CONNECTED      = 0x03,
    // Low Duty Cycle Advertiser
    LLD_LD_ADVERTISER        = 0x04,
    // High Duty Cycle Advertiser
    LLD_HD_ADVERTISER        = 0x05,
    // Passive Scanner
    LLD_PASSIVE_SCANNING     = 0x08,
    // Active Scanner
    LLD_ACTIVE_SCANNING      = 0x09,
    // Initiator
    LLD_INITIATING           = 0x0F,
    // Tx Test Mode
    LLD_TXTEST_MODE          = 0x1C,
    // Rx Test Mode
    LLD_RXTEST_MODE          = 0x1D,
    // Tx / Rx Test Mode
    LLD_TXRXTEST_MODE        = 0x1E,
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Synchronization counters
struct lld_evt_anchor
{
    /// Base time counter value of the latest found sync
    uint32_t basetime_cnt;
    /// Fine time counter value of the latest found sync
    uint16_t finetime_cnt;
    /// Event counter of the of the latest found sync
    uint16_t evt_cnt;
};

/// Structure describing an event
struct lld_evt_tag
{
    /// List element for chaining in the scheduling lists
    struct co_list_hdr hdr;

    /// Information about the latest found synchronization
    struct lld_evt_anchor anchor_point;

    /// List of TX Data descriptors programmed for transmission (i.e. chained with the CS)
    struct co_list tx_prog;
    /// List of TX Data descriptors ready for transmission (i.e. not yet chained with the CS)
    struct co_list tx_rdy;

    /// Interval element pointer linked to this event
    struct ea_interval_tag* interval_elt;

    /// Synchronization Window Size (in us)
    uint32_t sync_win_size;
    /// SCA Drift (in us)
    uint32_t sca_drift;

    /// Event end tie stamp
    uint32_t evt_end_ts;

    /// Connection Handle
    uint16_t conhdl;

    /// Control structure pointer address
    uint16_t cs_ptr;

    /// Connection Interval
    uint16_t interval;
    /// Instant of the next action (in events)
    uint16_t instant;
    /// Latency
    uint16_t latency;
    /// Event counter
    uint16_t counter;
    /// Number of connection events missed since last anchor point
    uint16_t missed_cnt;
    /// Minimum duration of the event or frame (in slots)
    uint16_t duration_dft;

    /// Update offset
    uint16_t update_offset;
    /// Slot on which the event occurs
    uint16_t slot;
    /// Update window size
    uint8_t update_size;

    /// Number of RX Descriptors already handled in the event
    uint8_t rx_cnt;
    /// Mode of the link (Master connect, slave connect, ...)
    uint8_t mode;
    /// Describe the action to be done when instant occurs
    uint8_t instant_action;
    /// TX Power
    uint8_t tx_pwr;
    /// Number of TX Descriptors already handled in the event
    uint8_t tx_cnt;
    /// Master sleep clock accuracy
    uint8_t mst_sca;

    /// Internal status
    uint8_t evt_flag;
#if (RWBLE_SW_VERSION_MAJOR >= 8)
    /// value of the latest More Data bit received
    uint8_t last_md_rx;
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */

};

/// Structure describing the parameters for a connection update
struct lld_evt_update_tag
{
    /// Offset to be put in the connection update request
    uint16_t win_offset;
    /// Instant for the connection update
    uint16_t instant;
    /// Size of the window to be put in the connection update request
    uint8_t win_size;
};

/// Structure describing an event
struct lld_evt_int_tag
{
    /// List element for chaining
    struct co_list_hdr hdr;
    /// List of events attached to this interval
    struct lld_evt_tag *evt;
    /// Number of free slots
    uint32_t freeslot;
    /// Base interval time
    uint16_t int_base;
};

/// Environment structure for the LLD module
struct lld_evt_env_tag
{
    /// List of programmed element
    struct co_list elt_prog;
    /// List of element pending to be freed
    struct co_list elt_deferred;
    /// Accuracy of the low power clock connected to the BLE core
    uint8_t sca;
};

/// Deferred element structure
struct lld_evt_deferred_tag
{
    /// List element for chaining in the Even Arbiter lists
    struct co_list_hdr hdr;

    /// Pointer on the deferred element
    struct ea_elt_tag *elt_ptr;
    /// Type of deferring
    uint8_t type;
    /// Number of used RX Descriptors
    uint8_t rx_desc_cnt;
};
/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */

/// Environment of the LLDEVT module
extern struct lld_evt_env_tag lld_evt_env;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Get sleep clock accuracy
 *
 * @return The sleep clock accuracy as defined in the standard
 *
 ****************************************************************************************
 */
__STATIC_INLINE uint8_t lld_evt_sca_get(void)
{
    // Read sleep clock accuracy from the environment
    return (lld_evt_env.sca);
}

/**
 ****************************************************************************************
 * @brief Get current time value from HW
 *
 * @return The current time in units of 625us
 *
 ****************************************************************************************
 */
__STATIC_INLINE uint32_t lld_evt_time_get(void)
{
    // Sample the base time count
    ble_sampleclk_set(BLE_SAMP_BIT);
    while(ble_sampleclk_get());
    // Read current time in HW
    return (ble_basetimecnt_get());
}

/**
 ****************************************************************************************
 * @brief Compare absolute times
 *
 * The absolute time difference between time1 and time2 is supposed to be less than the
 * maximum interval time
 *
 * @param[in] time1 First time to compare
 * @param[in] time2 Second time to compare
 *
 * @return true if time1 is smaller than time2.
 ****************************************************************************************
 */
__STATIC_INLINE bool lld_evt_time_cmp(uint32_t time1,
                               uint32_t time2)
{
    return (((time1 - time2) & BLE_BASETIMECNT_MASK) > MAX_INTERVAL_TIME);
}

/**
 ****************************************************************************************
 * @brief Check if time passed as parameter is in the past
 *
 * @param[in] time Time to be compare with current time to see if it in the past
 *
 * @return true if time is in the past, false otherwise
 *
 ****************************************************************************************
 */
__STATIC_INLINE bool lld_evt_time_past(uint32_t time)
{
    // Compare time and current time
    return(lld_evt_time_cmp(time & BLE_BASETIMECNT_MASK, lld_evt_time_get()));
}


/**
 ****************************************************************************************
 * @brief get the connection event counter for a dedicated event
 *
 * @param[in] evt   Pointer to the event for which the counter is requested
 *
 * @return The connection event counter
 *
 ****************************************************************************************
 */
__STATIC_INLINE uint16_t lld_evt_con_count_get(struct lld_evt_tag *evt)
{
    return (evt->counter - evt->missed_cnt);
}

/**
 ****************************************************************************************
 * @brief Update the TX power field of the event passed as parameter with the requested
 * value of TX power
 *
 * @param[in] evt    The event for which the TX power is updated
 * @param[in] tx_pwr The TX power to be applied to this event
 *
 * @return The computed compensation
 *
 ****************************************************************************************
 */
__STATIC_INLINE void lld_evt_txpwr_update(struct lld_evt_tag *evt, uint8_t tx_pwr)
{
    // Update Power in the event structure
    evt->tx_pwr = tx_pwr;
}

/**
 ****************************************************************************************
 * @brief Go through the current event list to find the one corresponding to the
 * connection handle passed as parameter
 *
 * @param[in] conhdl      Connection handle for which the corresponding event is searched
 *
 * @return The pointer to the found event (NULL if no event is attached to this handle)
 *
 ****************************************************************************************
 */
struct ea_elt_tag *lld_evt_conhdl2elt(uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Computes the maximum drift according to the master clock accuracy and the delay
 * passed as parameters
 *
 * @param[in] delay       Duration for which the drift is computed
 * @param[in] master_sca  Sleep clock accuracy of the master
 *
 * @return The value of the RX window size formatted for the RXWINCNTL field of the
 * control structure
 *
 ****************************************************************************************
 */
uint16_t lld_evt_drift_compute(uint16_t delay, uint8_t master_sca);

/**
 ****************************************************************************************
 * @brief Create a connection or scanning event and chain it in the scheduling and
 * interval lists
 *
 * @param[in] handle      Connection handle for which the event is created (LLD_ADV_HDL if
 *                        non-connected event)
 * @param[in] duration    Expected duration of the event, in units of 625us. This duration
 *                        can be reduced if scheduling of other events is requiring that.
 * @param[in] mininterval Minimum interval for the connection event
 * @param[in] maxinterval Maximum interval for the connection event
 * @param[in] latency     Requested latency for the event
 *
 * @return The pointer to the event just created
 *
 ****************************************************************************************
 */
struct ea_elt_tag* lld_evt_scan_create(uint16_t handle,
                                       uint16_t duration,
                                       uint16_t mininterval,
                                       uint16_t maxinterval,
                                       uint16_t latency);

struct ea_elt_tag *lld_evt_scan_connect_create(uint16_t scan_handle,
                                       uint16_t scan_interval,
                                       uint16_t connect_duration,
                                       uint16_t connect_mininterval,
                                       uint16_t connect_maxinterval,
                                       uint16_t connect_latency);
/**
 ****************************************************************************************
 * @brief Create an advertising event and chain it in the scheduling list
 *
 * @param[in] handle      Connection handle for which the event is created (LLD_ADV_HDL if
 *                        non-connected event)
 * @param[in] mininterval Minimum interval for the advertising event
 * @param[in] maxinterval Maximum interval for the advertising event
 * @param[in] restart_pol Requested restart policy, i.e. LLD_ADV_RESTART or LLD_NO_RESTART
 * @param[in] adv_type    High or Low Duty Cycle
 *
 * @return The pointer to the event just created
 *
 ****************************************************************************************
 */
struct ea_elt_tag *lld_evt_adv_create(uint16_t handle,
                                      uint16_t mininterval,
                                      uint16_t maxinterval,
                                      bool restart_pol,
                                      uint8_t adv_type);

#if (BLE_CENTRAL || BLE_PERIPHERAL)
/**
 ****************************************************************************************
 * @brief Create a connection event for parameter update
 *
 * @param[in]  evt_old     Pointer to the current connection event
 * @param[in]  ce_len      Requested size of the connection event (in 625us slots)
 * @param[in]  mininterval Minimum interval for the connection event
 * @param[in]  maxinterval Maximum interval for the connection event
 * @param[in]  latency     Requested slave latency
 * @param[out] upd_par     Computed updated parameters, to be put in the LLCP frame
 *
 * @return The pointer to the new event created (used after instant)
 *
 ****************************************************************************************
 */
struct ea_elt_tag *lld_evt_update_create(struct ea_elt_tag *evt_old,
                                         uint16_t ce_len,
                                         uint16_t mininterval,
                                         uint16_t maxinterval,
                                         uint16_t latency,
                                         struct lld_evt_update_tag *upd_par);

/**
 ****************************************************************************************
 * @brief Create a slave connection event
 *
 * @param[in]  con_par     Pointer to the decoded connection parameters
 * @param[in]  con_req_pdu Pointer to the connection request frame as received
 * @param[in]  evt_adv     Pointer to the advertising event that triggered the connection
 * @param[in]  conhdl      Connection handle to the established connection
 *
 * @return The pointer to the slave event created
 *
 ****************************************************************************************
 */
struct ea_elt_tag* lld_evt_move_to_slave(struct llc_create_con_req_ind const *con_par,
                                         struct llm_pdu_con_req_rx *con_req_pdu,
                                         struct ea_elt_tag *elt_adv,
                                         uint16_t conhdl);
/**
 ****************************************************************************************
 * @brief Indicates to the LLD the occurrence of a connection parameter update.
 *
 * @param[in]  param_pdu   Pointer to the connection parameter update PDU
 * @param[in]  evt_old     Pointer to the current event used for this connection
 *
 ****************************************************************************************
 */
void lld_evt_slave_update(struct llcp_con_up_req const *param_pdu,
                          struct ea_elt_tag *elt_old);

/**
 ****************************************************************************************
 * @brief Indicates to the LLD to move from initiating to master connected state.
 *
 * @param[in]  evt      Pointer to the event used for initiation
 * @param[in]  conhdl   Handle of the new master connection
 *
 ****************************************************************************************
 */
struct ea_elt_tag* lld_evt_move_to_master(struct ea_elt_tag *elt_scan, uint16_t conhdl, struct llc_create_con_req_ind const *pdu_tx);
#endif //(BLE_CENTRAL || BLE_PERIPHERAL)

/**
 ****************************************************************************************
 * @brief Delete an event by removing it from all the lists it is in
 *
 * @param[in] evt        Event to be deleted
 * @param[in] abort_bit  Bit of the BLE core controlling the abort for the event
 *
 ****************************************************************************************
 */
//void lld_evt_delete(struct lld_evt_tag *evt, uint32_t abort_bit);


/**
 ****************************************************************************************
 * @brief Program the next occurrence of the slave event passed as parameter
 * In case the slave event passed as parameter is far enough in the future (e.g. due to
 * slave latency), the event is canceled and replaced with the earliest possible one. This
 * function is called when a data has been pushed for transmission in order to send it as
 * soon as possible even if slave latency is used.
 *
 * @param[in] elt        Element to be deleted
 ****************************************************************************************
 */
void lld_evt_schedule_next(struct ea_elt_tag *elt);


/**
 ****************************************************************************************
 * @brief Initialization of the BLE event scheduler
 *
 * This function initializes the lists used for event scheduling.
 *
 * @param[in] reset  true if it's requested by a reset; false if it's boot initialization
 ****************************************************************************************
 */
void lld_evt_init(bool reset);

/**
 ****************************************************************************************
 * @brief Initialization of BLE event environement
 ****************************************************************************************
 */
void lld_evt_init_evt(struct lld_evt_tag *evt);

/**
 ****************************************************************************************
 * @brief Handle insertion of an element in Event Arbitrer list of elements
 ****************************************************************************************
 */
void lld_evt_elt_insert(struct ea_elt_tag *elt);

/**
 ****************************************************************************************
 * @brief Handle removing of an element in Event Arbitrer list of elements
 ****************************************************************************************
 */
bool lld_evt_elt_delete(struct ea_elt_tag *elt, bool flush_data);

/**
 ****************************************************************************************
 * @brief Kernel event scheduled when BLE events has to be programmed in the HW
 *
 * This function is a deferred action of the BLE wake up interrupt. It programs the data
 * Tx/Rx exchange in the control structure corresponding to the event, and programs the
 * target time of the next event to be scheduled.
 *
 ****************************************************************************************
 */
void lld_evt_schedule(struct ea_elt_tag *elt);

/**
 ****************************************************************************************
 * @brief Kernel event scheduled when a BLE event has to be handled by the HW
 *
 * This function is a deferred action of the BLE end of event interrupt. It flushes the
 * data Tx/Rx exchange corresponding to the event, and programs the
 * target time of the next event to be scheduled.
 *
 ****************************************************************************************
 */
void lld_evt_end(struct ea_elt_tag *elt);

/**
 ****************************************************************************************
 * @brief Kernel event scheduled when a BLE RX has to be handled by the HW
 *
 * This function is a deferred action of the BLE RX interrupt. It flushes the
 * data Tx/Rx exchange corresponding to the event.
 *
 ****************************************************************************************
 */
void lld_evt_rx(struct ea_elt_tag *elt);

/**
 ****************************************************************************************
 * @brief End of sleep interrupt handler
 *
 * This function is called under interrupt when an end of sleep interrupt is generated by
 * the BLE core. It sets the associated kernel event in order to perform the handling as a
 * deferred action in background context.
 *
 ****************************************************************************************
 */
void lld_evt_start_isr(void);


/**
 ****************************************************************************************
 * @brief End of event/frame interrupt handler
 *
 * This function is called under interrupt when an end of event/frame interrupt is
 * generated by the BLE/BT core.
 *
 * param[in] apfm  - Indicate if end of event is due to an apfm interrupt
 *
 ****************************************************************************************
 */
void lld_evt_end_isr(bool apfm);

/**
 ****************************************************************************************
 * @brief RX interrupt handler
 *
 * This function is called under interrupt when an RX interrupt is generated by
 * the BLE core. It sets the associated kernel event in order to perform the handling as a
 * deferred action in background context.
 *
 ****************************************************************************************
 */
void lld_evt_rx_isr(void);

/**
 ****************************************************************************************
 * @brief General purpose timer interrupt handler
 *
 * This function is called under interrupt when a general purpose timer interrupt is
 * generated by the BLE core. It sets the associated kernel event in order to perform the
 * handling as a deferred action in background context.
 *
 ****************************************************************************************
 */
void lld_evt_timer_isr(void);

/**
 ****************************************************************************************
 * @brief Request to program a channel map update
 *
 * @param[in] evt     Pointer to the event for which the update has to be programmed
 *
 * @return The instant computed for the update
 *
 ****************************************************************************************
 */
uint16_t lld_evt_ch_map_update_req(struct lld_evt_tag *evt);

/**
 ****************************************************************************************
 * @brief Check if event has an alternative event, if yes, delete it.
 *
 * @param[in] evt          Pointer to the event which can have alternative event
 ****************************************************************************************
 */
void lld_delete_alt_evt(struct lld_evt_tag * evt);


void lld_evt_canceled(struct ea_elt_tag *elt);

void lld_evt_deffered_elt_handler(void);

void lld_evt_prevent_stop(struct ea_elt_tag *elt);

/// @} LLDEVT

#endif // LLD_EVT_H_
