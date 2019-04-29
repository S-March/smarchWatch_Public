/**
 ****************************************************************************************
 *
 * @file llm_task.h
 *
 * @brief LLM task header file
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 ****************************************************************************************
 */

#ifndef LLM_TASK_H_
#define LLM_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup LLMTASK LLMTASK
 * @ingroup LLM
 * @brief Link Layer Manager Task
 *
 * The LLM task is responsible for managing link layer actions not related to a
 * specific connection with a peer (e.g. scanning, advertising, etc.). It implements the
 * state machine related to these actions.
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>
#include "co_bt.h"
#include "co_version.h"
#include "rwip_config.h" // stack configuration
#include "ke_task.h"


/*
 * INSTANCES
 ****************************************************************************************
 */
/// Maximum number of instances of the LLM task
#define LLM_IDX_MAX 1

/// LE events subcodes
#define LLM_LE_EVT_ADV_REPORT       0x02
/*
 * STATES
 ****************************************************************************************
 */
/// Possible states of the LLM task
enum llm_state_id
{
    /// IDLE state
    LLM_IDLE,
    /// ADVERTISING state
    LLM_ADVERTISING,
    /// SCANNING state
    LLM_SCANNING,
    /// INITIATING state
    LLM_INITIATING,
    /// STOPPING state
    LLM_STOPPING,
    /// TEST states.
    LLM_TEST,
    /// Number of states.
    LLM_STATE_MAX
};

/*
 * MESSAGES
 ****************************************************************************************
 */
/// Message API of the LLM task
enum llm_msg_id
{
    /*
     * ************** Msg LLM->LLM****************
     */
    /// Time out message
    /// to authorize the reception of the cmd
    LLM_LE_SET_HOST_CH_CLASS_CMD_STO = KE_FIRST_MSG(TASK_ID_LLM),
    /// Request to initialize LLM
    LLM_STOP_IND,

    /*
     * ************** Msg LLM->LLC****************
     */
    /// request changing map
    LLM_LE_SET_HOST_CH_CLASS_REQ,
    /// channel map req indication
    LLM_LE_SET_HOST_CH_CLASS_REQ_IND,
    /*
     * ************** Msg LLC->LLM****************
     */
    /// link layer disconnection indication
    LLC_DISCO_IND,
    LLM_ENC_REQ,
    LLM_ENC_IND,

    /*
     * ************** Channel Assessment ********
     */
    /// Channel Assessment Timer
    LLM_LE_CHNL_ASSESS_TIMER,
    /// Channel Assessment - Generate Channel classification
    LLM_GEN_CHNL_CLS_CMD,

#if (RWBLE_SW_VERSION_MAJOR >= 8)
    LM_ADDR_RENEW_TO_IND,
    LLM_P256_REQ
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */
};

///LLC Encrypt Request parameters structure
struct llm_enc_req
{
    ///Long term key structure
    struct ltk     key;
    ///Pointer to buffer with plain data to encrypt - 16 bytes
    uint8_t        plain_data[16];
#if (RWBLE_SW_VERSION_MAJOR >= 8)
    struct ll_pending_events *pevent;
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */
};

///LLM LE Encrypt indication structure
struct llm_enc_ind
{
    /// Status of the command reception
    uint8_t status;
    ///Encrypted data to return to command source.
    uint8_t encrypted_data[ENC_DATA_LEN];
};

#if (RWBLE_SW_VERSION_MAJOR >= 8)
///LLM LE P256 request structure
struct llm_p256_req
{
    /// Status of the command reception
    uint8_t status;
    ///data to return to command source.
    uint8_t p256_data[ECDH_KEY_LEN*2];
};
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */

/*
 * TASK DESCRIPTOR DECLARATIONS
 ****************************************************************************************
 */
extern const struct ke_state_handler llm_state_handler[LLM_STATE_MAX];
extern const struct ke_state_handler llm_default_handler;
extern ke_state_t llm_state[LLM_IDX_MAX];

/// @} LLMTASK

#endif // LLM_TASK_H_
