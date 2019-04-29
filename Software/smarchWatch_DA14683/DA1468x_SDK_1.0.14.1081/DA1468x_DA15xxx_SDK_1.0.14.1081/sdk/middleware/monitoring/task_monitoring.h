/**
 * \addtogroup BSP
 * \{
 * \addtogroup MIDDLEWARE
 * \{
 * \addtogroup TASK_MONITORING
 *
 * \brief Utilities for task monitoring
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file task_monitoring.h
 *
 * @brief Monitoring Task functions
 *
 * Copyright (C) 2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *****************************************************************************************
 */

#ifndef TASK_MONITORING_H
#define TASK_MONITORING_H

#if (dg_configENABLE_TASK_MONITORING == 1)
/*Maximum number of monitored tasks*/
#define MAX_NUMBER_OF_MONITORED_TASKS 5

/**
 * \brief Register a task to be monitored.
 *
 * \param [in] id the id to be used for monitoring
 *
 */
void tm_register_monitor_task(uint16_t id);

/**
 * \brief Unregister a task from the monitoring function.
 *
 * \param [in] id the id of the monitored task
 *
 */
void tm_unregister_monitor_task(uint16_t id);

/**
 * \brief Print the status of registered a tasks.
 *
 */
void tm_print_registered_tasks(void);

/**
 * \brief Print the status of a all the tasks.
 *
 */
void tm_print_tasks_status(void);

#endif /*dg_configENABLE_TASK_MONITORING*/

#endif /*TASK_MONITORING_H*/

/**
 * \}
 * \}
 * \}
 */
