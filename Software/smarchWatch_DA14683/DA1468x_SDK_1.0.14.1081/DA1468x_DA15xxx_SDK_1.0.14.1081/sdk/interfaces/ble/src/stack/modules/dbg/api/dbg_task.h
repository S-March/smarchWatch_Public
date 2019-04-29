/**
 ****************************************************************************************
 *
 * @file dbg_task.h
 *
 * @brief DBG task header file
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef DBG_TASK_H_
#define DBG_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup DBGTASK Task
 * @ingroup DBG
 * @brief Debug task
 *
 * The DBG task is responsible for receiving and sending debug informations between the
 * HCI and the embedded.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>              // standard definition
#include "ke_task.h"             // kernel task
#include "co_bt.h"               // common BT definitions

/*
 * INSTANCES
 ****************************************************************************************
 */
/// Maximum number of instances of the DBG task
#define DBG_IDX_MAX 1

/*
 * STATES
 ****************************************************************************************
 */
/// Possible states of the DBG task
enum
{
    /// IDLE state
    DBG_IDLE,
    /// Number of states.
    DBG_STATE_MAX
};

/*
 * MESSAGES
 ****************************************************************************************
 */
/// Message API of the DBG task
enum
{
    /// Debug commands description.
    DBG_MSG_ID_CMD_FIRST = KE_FIRST_MSG(TASK_ID_DBG),
};



/*
 * TASK DESCRIPTOR DECLARATIONS
 ****************************************************************************************
 */
extern const struct ke_state_handler dbg_default_handler;
extern ke_state_t dbg_state[DBG_IDX_MAX];

/// @} DBGTASK

#endif // DBG_TASK_H_
