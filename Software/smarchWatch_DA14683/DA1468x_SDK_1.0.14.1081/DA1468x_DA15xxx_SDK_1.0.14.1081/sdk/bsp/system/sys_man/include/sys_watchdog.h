/**
 * \addtogroup BSP
 * \{
 * \addtogroup SYSTEM
 * \{
 * \addtogroup WATCHDOG
 * 
 * \brief Watchdog service
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file sys_watchdog.h
 *
 * @brief Watchdog header file.
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SYS_WATCHDOG_H_
#define SYS_WATCHDOG_H_

#include <stdbool.h>
#include <stdint.h>

#define SYS_WATCHDOG_TRIGGER (1 << 31)

/**
 * Initialize sys_watchdog module
 *
 * This should be called before using sys_watchdog module, preferably as early as possible.
 *
 */
void sys_watchdog_init(void);

/**
 * Register current task in sys_watchdog module
 *
 * Returned identifier shall be used in all other calls to sys_watchdog from current task.
 * Once registered, task shall notify sys_watchdog periodically using sys_watchdog_notify() to
 * prevent watchdog expiration. It's up to actual task how this is done, but task can request that
 * it will be triggered periodically using task notify feature and it should notify-back sys_watchdog
 * as a response.
 *
 * \note
 * \p dg_configWDOG_NOTIFY_TRIGGER_TMO shall be set to non-zero for \p notify_trigger to have any
 * effect.
 *
 * \param [in] notify_trigger   true if task notify should be triggered periodically
 *
 * \return identifier on success, -1 on failure
 *
 * \sa sys_watchdog_notify
 *
 */
int8_t sys_watchdog_register(bool notify_trigger);

/**
 * Unregister task from sys_watchdog module
 *
 * \param [in] id       identifier
 *
 * \sa sys_watchdog_register
 *
 */
void sys_watchdog_unregister(int8_t id);

/**
 * Inform sys_watchdog module about the wdog id of the IDLE task
 *
 * \param [in] id       IDLE task wdog identifier
 *
 * \sa sys_watchdog_register
 *
 */
void sys_watchdog_configure_idle_id(int8_t id);

/**
 * Suspend task monitoring in sys_watchdog module
 *
 * Suspended task is not unregistered entirely but will not be monitored by watchdog until resumed.
 * It's faster than unregistering and registering task again.
 *
 * \param [in] id       identifier
 *
 * \sa sys_watchdog_resume
 *
 */
void sys_watchdog_suspend(int8_t id);

/**
 * Resume task monitoring in sys_watchdog module
 *
 * Resumes task monitoring suspended previously by sys_watchdog_suspend().
 *
 * \param [in] id       identifier
 *
 * \note This function does not notify the watchdog service for this task. It is possible that
 *       monitor resuming occurs too close to the time that the watchdog expires, before the task
 *       has a chance to explicitly send a notification. This can lead to an unwanted reset.
 *       Therefore, either call sys_watchdog_notify() before calling sys_watchdog_resume(), or use
 *       sys_watchdog_notify_and_resume() instead.
 *
 * \sa sys_watchdog_suspend
 * \sa sys_watchdog_notify_and_resume
 *
 */
void sys_watchdog_resume(int8_t id);

/**
 * Notify sys_watchdog module for task
 *
 * Registered task shall use this periodically to notify sys_watchdog module that it's alive. This
 * should be done frequently enough to fit into hw_watchdog interval set by dg_configWDOG_RESET_VALUE.
 *
 * \param [in] id       identifier
 *
 * \sa sys_watchdog_set_latency
 *
 */
void sys_watchdog_notify(int8_t id);

/**
 * Notify sys_watchdog module for task with handle \p id and resume its monitoring
 *
 * This function combines the functionality of sys_watchdog_notify() and sys_watchdog_resume().
 *
 * \param [in] id       identifier
 *
 * \sa sys_watchdog_notify()
 * \sa sys_watchdog_resume()
 *
 */
void sys_watchdog_notify_and_resume(int8_t id);

/**
 * Set watchdog latency for task
 *
 * This allows task to miss given number of notifications to sys_watchdog without triggering
 * platform reset. Once set, it's allowed that task does not notify sys_watchdog for \p latency
 * consecutive hw_watchdog intervals (as set by dg_configWDOG_RESET_VALUE) which can be used to
 * allow for parts of code which are known to block for long period of time (i.e. computation).
 * This value is set once and does not reload automatically, thus it shall be set every time
 * increased latency is required.
 *
 * \param [in] id       identifier
 * \param [in] latency  latency
 *
 * \sa sys_watchdog_notify
 *
 */
void sys_watchdog_set_latency(int8_t id, uint8_t latency);

/**
 * Find out if the only task currently monitored is the IDLE task.
 *
 * \return true if only the IDLE task is currently monitored, else false. This function is used
 * from CPM in order to know if watchdog should be stopped during sleep.
 */
__RETAINED_CODE bool sys_watchdog_monitor_mask_empty(void);

#endif /* SYS_WATCHDOG_H_ */

/**
 * \}
 * \}
 * \}
 */

