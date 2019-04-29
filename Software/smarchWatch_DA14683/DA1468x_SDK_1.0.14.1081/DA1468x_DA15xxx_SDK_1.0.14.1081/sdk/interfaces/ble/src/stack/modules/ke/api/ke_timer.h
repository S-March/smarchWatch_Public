/**
 ****************************************************************************************
 *
 * @file ke_timer.h
 *
 * @brief This file contains the definitions used for timer management
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef _KE_TIMER_H_
#define _KE_TIMER_H_

/**
 ****************************************************************************************
 * @defgroup TIMER BT Time
 * @ingroup KERNEL
 * @brief Timer management module.
 *
 * This module implements the functions used for managing kernel timers.
 *
 ****************************************************************************************
 */

#include "rwip_config.h"          // stack configuration
#include "ke_msg.h"               // messaging definition


/*
 * DEFINITIONS
 ****************************************************************************************
 */

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Timer Object
struct ke_timer
{
    /// next ke timer
    struct ke_timer *next;
    /// message identifier
    ke_msg_id_t     id;
    /// task identifier
    ke_task_id_t    task;
    /// time value
    uint32_t        time;
};


/*
 * FUNCTION PROTOTYPES
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @brief Initialize Kernel timer module.
 ****************************************************************************************
 */
void ke_timer_init(void);

/**
 ****************************************************************************************
 * @brief Set a timer.
 *
 * The function first cancel the timer if it is already existing, then
 * it creates a new one. The timer can be one-shot or periodic, i.e. it
 * will be automatically set again after each trigger.
 *
 * When the timer expires, a message is sent to the task provided as
 * argument, with the timer id as message id.
 *
 * The timer is programmed in time units (TU is 10ms).
 *
 * @param[in] timer_id      Timer identifier (message identifier type).
 * @param[in] task_id       Task identifier which will be notified
 * @param[in] delay         Delay in time units.
 ****************************************************************************************
 */
void ke_timer_set(ke_msg_id_t const timer_id, ke_task_id_t const task, uint32_t delay);

/**
 ****************************************************************************************
 * @brief Remove an registered timer.
 *
 * This function search for the timer identified by its id and its task id.
 * If found it is stopped and freed, otherwise an error message is returned.
 *
 * @param[in] timer_id  Timer identifier.
 * @param[in] task      Task identifier.
 ****************************************************************************************
 */
void ke_timer_clear(ke_msg_id_t const timerid, ke_task_id_t const task);

/**
 ****************************************************************************************
 * @brief Checks if a requested timer is active.
 *
 * This function pops the first timer from the timer queue and notifies the appropriate
 * task by sending a kernel message. If the timer is periodic, it is set again;
 * if it is one-shot, the timer is freed. The function checks also the next timers
 * and process them if they have expired or are about to expire.
 ****************************************************************************************
 */
bool ke_timer_active(ke_msg_id_t const timer_id, ke_task_id_t const task_id);

#if (DEEP_SLEEP)
/**
 ****************************************************************************************
 * @brief Check if sleep mode is possible
 *
 * The function takes as argument the allowed sleep duration that must not be increased.
 * If a timer needs an earlier wake-up than initial duration, the allowed sleep duration
 * is updated.
 * If a timer needs a shorter duration than the wake-up delay, sleep is not possible and
 * the function returns a bad status.
 *
 * @param[in/out] sleep_duration   Initial allowed sleep duration (in slot of 625us)
 * @param[in]     wakeup_delay     Delay for system wake-up (in slot of 625us)
 *
 * @return true if sleep is allowed, false otherwise
 ****************************************************************************************
 */
bool ke_timer_sleep_check(uint32_t *sleep_duration, uint32_t wakeup_delay);
#endif //DEEP_SLEEP

/// @} TIMER

#endif // _KE_TIMER_H_
