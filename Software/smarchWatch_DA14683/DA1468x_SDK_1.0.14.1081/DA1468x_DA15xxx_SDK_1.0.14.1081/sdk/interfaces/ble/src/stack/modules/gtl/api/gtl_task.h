/**
 ****************************************************************************************
 *
 * @file gtl_task.h
 *
 * @brief This file contains definitions related to the Generic Transport Layer
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef GTL_TASK_H_
#define GTL_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup GTL Generic Transport Layer
 *@{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

#if (GTL_ITF)

#include "ke_task.h"         // kernel task

/*
 * INSTANCES
 ****************************************************************************************
 */
/// Maximum number of instances of the GTL task
#define GTL_IDX_MAX 1

/*
 * STATES
 ****************************************************************************************
 */
/// Possible states of the GTL task
enum GTL_STATE
{
    /// TX IDLE state
    GTL_TX_IDLE,
    /// TX ONGOING state
    GTL_TX_ONGOING,
    /// Number of states.
    GTL_STATE_MAX
};

/*
 * MESSAGES
 ****************************************************************************************
 */
/// Message API of the GTL task
enum GTL_MSG
{
    GTL_MSG_ID_FIRST = KE_FIRST_MSG(TASK_ID_GTL),

    GTL_MSG_ID_LAST
};

/*
 * TASK DESCRIPTOR DECLARATIONS
 ****************************************************************************************
 */
extern const struct ke_state_handler gtl_default_handler;
extern ke_state_t gtl_state[GTL_IDX_MAX];

#endif //GTL_ITF

/// @} GTL

#endif // GTL_TASK_H_

