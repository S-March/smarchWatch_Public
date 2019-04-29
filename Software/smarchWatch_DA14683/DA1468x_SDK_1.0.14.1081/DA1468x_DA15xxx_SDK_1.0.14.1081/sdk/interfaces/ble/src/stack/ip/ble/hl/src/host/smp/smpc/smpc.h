/**
 ****************************************************************************************
 *
 * @file smpc.h
 *
 * @brief Header file - SMPC.
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef SMPC_H_
#define SMPC_H_

/**
 ****************************************************************************************
 * @addtogroup SMP Security Manager Protocol
 * @ingroup HOST
 * @brief Security Manager Protocol.
 *
 * The SMP is responsible for the over-all security policies of BLE.
 * It defines methods for pairing and key distribution, handles encryption,
 * data signing and privacy features such as random addressing generation and resolution.
 *
 * Pairing is performed to exchange pairing features and generate a short term
 * key for link encryption.
 * A transport specific key distribution is performed to
 * share the keys that can be used to encrypt the link in the future
 * reconnection process, signed data verification and random address
 * resolution.
 *
 * There exist 3 phases in the complete security procedure:
 * 1. Feature exchange (IO capabilities, OOB flags, Authentication Requirements, Key distributions)
 * 2. Short Term Key generation
 *    Generation method depends on exchanged features:
 *     - Just Works - use Temporary key = 0
 *     - PassKey Entry - use Temporary Key = 6-digit provided by user
 *     - Out of Band (OOB) - use Temporary Key = 16-octet key, available form OOB source
 * 3. Transport Specific Key Distribution (TKDP)(LTK+EDIV+RAND_NB, IRK+ADDR, CSRK)
 *---------------------------------------------------------------------
 * @addtogroup SMPC Security Manager Protocol Controller
 * @ingroup SMP
 * @brief Security Manager Protocol Controller.
 *
 * This block handles control of SM procedures for several possible existing connections,
 * for which the security procedure may be conducted simultaneously.
 *
 * It allows flow control for HCI access to encryption and random number generation, used
 * at different moments in the procedure.
 *
 * It handles PDU creation and sending through L2CAP, also their reception from L2CAP
 * and interpretation.
 *
 * Other small utilities such as maximum key size determination and TKDP organization are
 * implemented in SMPC.
 * @{
 *
 ****************************************************************************************
 */

#include "smp_common.h"
#include "co_version.h"

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#if (RW_BLE_USE_CRYPT)

#if (BLE_CENTRAL || BLE_PERIPHERAL)

#include <stdbool.h>
#include <stdint.h>

#include "co_bt.h"
#include "gap.h"

#include "smpc_api.h"
#include "l2cc_pdu.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// Minimum Encryption key size
#define SMPC_MIN_ENC_SIZE_LEN                   (7)
/// Maximum Encryption Key size
#define SMPC_MAX_ENC_SIZE_LEN                   (16)

/// MAC length
#define SMPC_SIGN_MAC_LEN                       (8)
/// SignCounter length
#define SMPC_SIGN_COUNTER_LEN                   (4)
/// Signature length
#define SMPC_SIGN_LEN                           (SMPC_SIGN_MAC_LEN + SMPC_SIGN_COUNTER_LEN)

/// Pairing Request and Pairing Response PDU Length
#define SMPC_CODE_PAIRING_REQ_RESP_LEN          (7)

/**
 * Timer State Masks
 */
/// Timeout Timer
#define SMPC_TIMER_TIMEOUT_FLAG                 (0x01)
/// Repeated Attempts Timer
#define SMPC_TIMER_REP_ATT_FLAG                 (SMPC_TIMER_TIMEOUT_FLAG << 1)
/// Blocked because of SMP Timeout
#define SMPC_TIMER_TIMEOUT_BLOCKED_FLAG         (SMPC_TIMER_REP_ATT_FLAG << 1)

/**
 * Repeated Attempts Timer Configuration
 */
/// Repeated Attempts Timer default value (x10ms)
#define SMPC_REP_ATTEMPTS_TIMER_DEF_VAL         rom_cfg_table[smpc_rep_attempts_timer_def_val_pos] //(200)      //2s
/// Repeated Attempts Timer max value (x10ms)
#define SMPC_REP_ATTEMPTS_TIMER_MAX_VAL         rom_cfg_table[smpc_rep_attempts_timer_max_val_pos] //(3000)     //30s
/// Repeated Attempts Timer multiplier
#define SMPC_REP_ATTEMPTS_TIMER_MULT            rom_cfg_table[smpc_rep_attempts_timer_mult_pos] //(2)

/**
 * Timeout Timer Configuration
 */
#define SMPC_TIMEOUT_TIMER_DURATION             rom_cfg_table[smpc_timeout_timer_duration_pos] //(3000)     //30s

/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/// Information source.
enum smpc_addr_src
{
    /// Local info.
    SMPC_INFO_LOCAL,
    /// Peer info.
    SMPC_INFO_PEER,
    /// Maximum info source.
    SMPC_INFO_MAX
};


///Security Properties for distributed keys(all have the issued STK's properties)
enum
{
    ///No security properties
    SMP_KSEC_NONE = 0x00,
    ///Unauthenticated no MITM
    SMP_KSEC_UNAUTH_NO_MITM,
    ///Authenticated with MITM
    SMP_KSEC_AUTH_MITM,
};

/// Repeated Attempts Attack Detection status
enum
{
    /// No attack has been detected
    SMPC_REP_ATTEMPTS_NO_ERROR          = SMP_ERROR_NO_ERROR,           // 0x00
    /// An attack has already been detected, drop the message
    SMPC_REP_ATTEMPTS_ATTACK,
    /// An attack has been detected, an indication has been sent to the HL
    SMPC_REP_ATTEMPS_ATTACK_DETECTED,
    /// Repeated Attempt detected, need to send a Pairing Failed PDU to the peer device
    SMPC_REP_ATTEMPT                    = SMP_ERROR_REPEATED_ATTEMPTS   // 0x09
};

/// SMPC Internal State Code
enum
{
    SMPC_STATE_RESERVED     = 0x00,

    /********************************************************
     * Pairing Procedure
     ********************************************************/

    /**------------------------------------**
     * Pairing Features Exchange Phase      *
     **------------------------------------**/
    /// Is waiting for the pairing response
    SMPC_PAIRING_RSP_WAIT,
    /// Is waiting for the pairing features
    SMPC_PAIRING_FEAT_WAIT,

    /**------------------------------------**
     * Authentication and Encryption Phase  *
     **------------------------------------**/

    /// Is waiting for the TK
    SMPC_PAIRING_TK_WAIT,
    /// Is waiting for the TK, peer confirm value has been received
    SMPC_PAIRING_TK_WAIT_CONF_RCV,
    /// Calculate the Random Number, part 1
    SMPC_PAIRING_GEN_RAND_P1,
    /// Calculate the Random Number, part 2
    SMPC_PAIRING_GEN_RAND_P2,
    /// The first part of the device's confirm value is being generated
    SMPC_PAIRING_CFM_P1,
    /// The device's confirm value is being generated
    SMPC_PAIRING_CFM_P2,
    /// The first part of the peer device's confirm value is being generated
    SMPC_PAIRING_REM_CFM_P1,
    /// The peer device's confirm value is being generated
    SMPC_PAIRING_REM_CFM_P2,
    /// The device is waiting for the confirm value generated by the peer device
    SMPC_PAIRING_WAIT_CONFIRM,
    /// The device is waiting for the random value generated by the peer device
    SMPC_PAIRING_WAIT_RAND,
    /// The STK is being generated
    SMPC_PAIRING_GEN_STK,

    /**------------------------------------**
     * Transport Keys Distribution Phase    *
     **------------------------------------**/

    /// Is waiting for the LTK
    SMPC_PAIRING_LTK_WAIT,
    /// Is waiting for the CSRK
    SMPC_PAIRING_CSRK_WAIT,
    /// Is waiting for the remote LTK
    SMPC_PAIRING_REM_LTK_WAIT,
    /// Is waiting for the remote EDIV and Rand Value
    SMPC_PAIRING_REM_MST_ID_WAIT,
    /// Is waiting for the remote IRK
    SMPC_PAIRING_REM_IRK_WAIT,
    /// Is waiting for the remote BD Address
    SMPC_PAIRING_REM_BD_ADDR_WAIT,
    /// Is waiting for the remote CSRK
    SMPC_PAIRING_REM_CSRK_WAIT,

    /********************************************************
     * Signing Procedure
     ********************************************************/
    /// Generation of L
    SMPC_SIGN_L_GEN,
    /// Generation of Ci
    SMPC_SIGN_Ci_GEN,

    /********************************************************
     * Encryption Procedure (STK or LTK)
     ********************************************************/
    /// Is waiting the change encryption event with LTK
    SMPC_START_ENC_LTK,
    /// Is waiting the change encryption event with STK
    SMPC_START_ENC_STK,

#if (RWBLE_SW_VERSION_MAJOR >= 8)
    /// Is waiting for public key exchange
    SMPC_PAIRING_WAIT_PUBLIC_KEY,
    /// Is waiting for numeric comparison calculation
    SMPC_PAIRING_WAIT_6DIGIT_CALC,
    /// Is waiting for LTK and MacKey calculation, first T is calculated
    SMPC_PAIRING_WAIT_LTK_CALC,
    /// Is waiting for LTK and MacKey calculation
    SMPC_PAIRING_WAIT_LTK_CALC_P2,
    /// Is waiting for LTK and MacKey calculation
    SMPC_PAIRING_WAIT_LTK_CALC_P2_LTK,
    /// Is waiting for Secure connections dhkey check Ea/Eb calculation
    SMPC_PAIRING_WAIT_DHKEY_CHECK_CALC,
    /// Is waiting for Secure connections dhkey check Ea/Eb value from peer
    SMPC_PAIRING_WAIT_DHKEY_CHECK_PEER,
    /// Is waiting for Secure connections PEER dhkey check Ea/Eb calculation
    SMPC_PAIRING_WAIT_DHKEY_CHECK_PEER_CALC,

    /// First part of calculation of our confirm value (Cai for master, Cbi for slave)
    SMPC_PAIRING_PK_CFM_P1,

    /// Second part of calculation of our confirm value
    SMPC_PAIRING_PK_CFM_P2,

    /// Waiting for remote confirm value to be received (Cbi for master, Cai for slave)
    SMPC_PAIRING_PK_WAIT_CONFIRM,

    /// Waiting for remote random number (Nai or Nbi) to be received
    SMPC_PAIRING_PK_WAIT_RAND,

    /// First part of calculation of remote confirm value (Cbi for master, Cai for slave)
    SMPC_PAIRING_PK_REM_CFM_P1,

    /// Second part of calculation of remote confirm value
    SMPC_PAIRING_PK_REM_CFM_P2,

    /********************************************************
     Authentication using Secure Connection, passkey method.
     State transition for the two peers:

     Initiating (master)
     SMPC_PAIRING_GEN_RAND_P1
     SMPC_PAIRING_GEN_RAND_P2
     SMPC_PAIRING_PK_CFM_P1
     SMPC_PAIRING_PK_CFM_P2
     SMPC_PAIRING_PK_WAIT_CONFIRM
     SMPC_PAIRING_PK_WAIT_RAND
     SMPC_PAIRING_PK_REM_CFM_P1
     SMPC_PAIRING_PK_REM_CFM_P2

     Non-Initiating (slave)
     SMPC_PAIRING_GEN_RAND_P1
     SMPC_PAIRING_GEN_RAND_P2
     SMPC_PAIRING_PK_CFM_P1
     SMPC_PAIRING_PK_CFM_P2
     [SMPC_PAIRING_PK_WAIT_CONFIRM}
     SMPC_PAIRING_PK_WAIT_RAND
     SMPC_PAIRING_PK_REM_CFM_P1
     SMPC_PAIRING_PK_REM_CFM_P2
     ********************************************************/
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */
};

/// STK generation methods
enum
{
    ///Just Works Method
    SMPC_METH_JW            = 0x00,
    ///PassKey Entry Method
    SMPC_METH_PK,
    ////OOB Method
    SMPC_METH_OOB,
#if (RWBLE_SW_VERSION_MAJOR >= 8)
    ////Numeric Comparison Method
    SMPC_METH_NC
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */
};

/// Signature Command Types
enum
{
    /// Generate Signature
    SMPC_SIGN_GEN           = 0x00,
    /// Verify Signature
    SMPC_SIGN_VERIF
};

enum
{
    /// Use of STK in start encryption command
    SMPC_USE_STK     = 0x00,
    /// Use of LTK in start encryption command
    SMPC_USE_LTK
};

/*
 * STRUCTURES DEFINITION
 ****************************************************************************************
 */

/// Master ID Information Structure
struct smpc_mst_id_info
{
    // Encryption Diversifier
    uint16_t ediv;

    // Random Number
    uint8_t randnb[RAND_NB_LEN];
};

/// Pairing Information
struct smpc_pair_info
{
    /// TK during Phase 2, LTK or IRK during Phase 3
    struct gap_sec_key key;
    /// Pairing request command
    struct gapc_pairing pair_req_feat;
    /// Pairing response feature
    struct gapc_pairing pair_rsp_feat;
    /// Random number value
    uint8_t rand[RAND_VAL_LEN];
    /// Remote random number value
    uint8_t rem_rand[RAND_VAL_LEN];
#if (RWBLE_SW_VERSION_MAJOR >= 8)
    /// My confirm value
    uint8_t local_conf_value[KEY_LEN];
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */
    /// Confirm value to check
    uint8_t conf_value[KEY_LEN];
    /// Pairing Method
    uint8_t pair_method;
    /// Authentication level
    uint8_t auth;
#if (RWBLE_SW_VERSION_MAJOR >= 8)
    /// dhkey check value received
    bool    dhkey_check_value_received;
    /// dhkey check value to check
    uint8_t dhkey_check_value[KEY_LEN];	
    /// aes offset
    uint16_t aes_block_size;
    /// Number of block
    uint8_t block_nb;
    /// K1 subkey
    uint8_t K1[KEY_LEN];
    /// K2 subkey
    uint8_t K2[KEY_LEN];
    /// T key
    uint8_t T[KEY_LEN];
    /// MacKey
    uint8_t MacKey[KEY_LEN];
    /// tmp buffer used for AES-CMAK
    uint8_t tmp[KEY_LEN];
    bool cfm_received;
    bool    rand_generated;
    bool    rand_received;
    uint8_t tmp0[KEY_LEN];
    uint8_t tmp1[KEY_LEN];
    uint8_t tmp2[KEY_LEN];
    uint8_t tmp3[KEY_LEN];
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */
};

/// Signing Information
struct smpc_sign_info
{
    /// Operation requester task id
    ke_task_id_t requester;

    /// Message offset
    uint16_t msg_offset;
    /// Number of block
    uint8_t block_nb;
    /// Cn-1 value -> Need to kept this value to retrieve it after L generation
    uint8_t cn1[KEY_LEN];
};

/// SMPC environment structure
struct smpc_env
{
    /// SMPC temporary information
    union smpc_info
    {
        /**
         * Pairing Information - This structure is allocated at the beginning of a pairing
         * or procedure. It is freed when a disconnection occurs or at the end of
         * the pairing procedure. If not enough memory can be found, the procedure will fail
         *  with an "Unspecified Reason" error
         */
        struct smpc_pair_info *pair;

        /**
         * Signature Procedure Information - This structure is allocated at the beginning of a
         * signing procedure. It is freed when a disconnection occurs or at the end of
         * the signing procedure. If not enough memory can be found, the procedure will fail
         *  with an "Unspecified Reason" error.
         */
        struct smpc_sign_info *sign;
    } info;

#if (RWBLE_SW_VERSION_MAJOR >= 8)
    /// Passkey. A 32bit number is enough for a six digit passkey.
    uint32_t passkey;

    /// Passkey bit counter, used during authentication stage 1.
    uint8_t current_passkey_bit;
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */

    /// CSRK values (Local and remote)
    struct gap_sec_key csrk[SMPC_INFO_MAX];

    /// signature counter values (Local and remote)
    uint32_t sign_counter[SMPC_INFO_MAX];

    /// Repeated Attempt Timer value
    uint16_t rep_att_timer_val;

    /// Encryption key size
    uint8_t key_size;

    /**
     * Contains the current state of the two timers needed in the SMPC task
     *      Bit 0 - Is Timeout Timer running
     *      Bit 1 - Is Repeated Attempt Timer running
     *      Bit 2 - Has task reached a SMP Timeout
     */
    uint8_t timer_state;

    /// State of the current procedure
    uint8_t state;

#if (RWBLE_SW_VERSION_MAJOR >= 8)
    uint8_t peer_public_key[ECDH_KEY_LEN*2];
    ec_point session_key;
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */
};

/*
 * GLOBAL VARIABLES DEFINITION
 ****************************************************************************************
 */


/*
 * MACROS
 ****************************************************************************************
 */

/// Authentication Request mask
#define SMPC_MASK_AUTH_REQ(req)    (req & 0x07)

#define SMPC_IS_FLAG_SET(conidx, flag)        ((gapc_env[conidx]->smpc.timer_state & flag) == flag)

#define SMPC_TIMER_SET_FLAG(conidx, flag)     (gapc_env[conidx]->smpc.timer_state |= flag)

#define SMPC_TIMER_UNSET_FLAG(conidx, flag)   (gapc_env[conidx]->smpc.timer_state &= ~flag)

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Send a SMPM_USE_ENC_BLOCK_CMD message to the SMPM. Shall be use when the AES_128
 *        encryption block need to be used.
 *
 * @param[in] conidx      Connection Index
 * @param[in] operand_1   First operand
 * @param[in] operand_2   Second operand
 ****************************************************************************************
 */
void smpc_send_use_enc_block_cmd(uint8_t conidx,
                                 uint8_t *operand_1, uint8_t *operand_2);

/**
 ****************************************************************************************
 * @brief Send a request to the controller to start the encryption procedure.
 *
 * @param[in] conidx      Connection Index
 * @param[in] operand_1   First operand
 * @param[in] operand_2   Second operand
 ****************************************************************************************
 */
void smpc_send_start_enc_cmd(uint8_t idx, uint8_t key_type, uint8_t *key,
                             uint8_t *randnb, uint16_t ediv);

/**
 ****************************************************************************************
 * @brief Send the LTK provided by the HL to the controller.
 *
 * @param[in] idx         Connection Index
 * @param[in] found       Indicate if the requested LTK has been found by the application
 * @param[in] key         Found LTK, used only if found is set to true
 ****************************************************************************************
 */
void smpc_send_ltk_req_rsp(uint8_t idx, bool found, uint8_t *key);

/**
 ****************************************************************************************
 * @brief Send a SMPC_PAIRING_REQ_IND message to the HL
 *
 * @param[in] conidx      Connection Index
 * @param[in] req_type    Kind of request
 ****************************************************************************************
 */
void smpc_send_pairing_req_ind(uint8_t conidx, uint8_t req_type);

/**
 ****************************************************************************************
 * @brief Send a SMPC_PAIRING_IND message to the HL
 *
 * @param[in] conidx      Connection Index
 * @param[in] ind_type    Kind of indication
 * @param[in] value       Value to indicate (keys, ...)
 ****************************************************************************************
 */
void smpc_send_pairing_ind(uint8_t conidx, uint8_t ind_type, void *value);

/**
 ****************************************************************************************
 * @brief Check if the provided pairing features are within the specified range.
 *
 * @param[in] pair_feat   Pairing Features values to check
 *
 * @param[out] true if features are valid, else false
 ****************************************************************************************
 */
bool smpc_check_pairing_feat(struct gapc_pairing *pair_feat);

/**
 ****************************************************************************************
 * @brief Check if an attack by repeated attempts has been triggered by the peer device
 *
 * @param[in] conidx   Connection Index
 ****************************************************************************************
 */
uint8_t smpc_check_repeated_attempts(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Compute and check the encryption key size to use during the connection.
 *
 * @param[in] conidx   Connection Index
 *
 * @param[out] true if the resultant EKS is within the specified range [7-16 bytes], else false
 ****************************************************************************************
 */
bool smpc_check_max_key_size(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Check if the keys distribution scheme is compliant with the required security
 *        level
 *
 * @param[in] conidx      Connection Index
 * @param[in] sec_level   Security level required by the device.
 ****************************************************************************************
 */
bool smpc_check_key_distrib(uint8_t conidx, uint8_t sec_level);

/**
 ****************************************************************************************
 * @brief Apply the XOR operator to the two provided operands
 *
 * @param[in|out] result      Buffer which will contain the result of the XOR operation
 * @param[in]     operand_1   First operand
 * @param[in]     operand_2   Second operand
 ****************************************************************************************
 */
void smpc_xor(uint8_t *result, uint8_t *operand_1, uint8_t *operand_2);

/**
 ****************************************************************************************
 * @brief Generate the L value during a signature verification/generation procedure.
 *
 * @param[in] conidx   Connection Index
 * @param[in] src      Indicate the source of the CSRK which will be used (LOCAL or PEER)
 ****************************************************************************************
 */
void smpc_generate_l(uint8_t conidx, uint8_t src);

/**
 ****************************************************************************************
 * @brief Generate one of the Ci value during a signature verification/generation procedure.
 *
 * @param[in] conidx   Connection Index
 * @param[in] src      Indicate the source of the CSRK which will be used (LOCAL or PEER)
 * @param[in] ci1      Previous computed Ci value
 * @param[in] mi       16-byte block used to generate the ci value
 ****************************************************************************************
 */
void smpc_generate_ci(uint8_t conidx, uint8_t src, uint8_t *ci1, uint8_t *mi);

/**
 ****************************************************************************************
 * @brief Generate the random value exchanged during the pairing procedure (phase 2)
 *
 * @param[in] conidx   Connection Index
 * @param[in] state    New state of the SMPC task.
 ****************************************************************************************
 */
void smpc_generate_rand(uint8_t conidx, uint8_t state);

/**
 ****************************************************************************************
 * @brief Generate the first value needed in the confirm value generation
 *
 * @param[in] conidx   Connection Index
 * @param[in] role     Current role of the device
 * @param[in] local    true if the confirm value to generate is the confirm value of the
 *                     device, false if it is the remote device's one.
 ****************************************************************************************
 */
void smpc_generate_e1(uint8_t conidx, uint8_t role, bool local);

/**
 ****************************************************************************************
 * @brief Generate the confirm value
 *
 * @param[in] conidx   Connection Index
 * @param[in] role     Current role of the device
 * @param[in] e1       e1 value
 ****************************************************************************************
 */
void smpc_generate_cfm(uint8_t conidx, uint8_t role, uint8_t *e1);

/**
 ****************************************************************************************
 * @brief Generate the STK used to encrypt a link after the pairing procedure
 *
 * @param[in] conidx   Connection Index
 * @param[in] role     Current role of the device
 ****************************************************************************************
 */
void smpc_generate_stk(uint8_t conidx, uint8_t role);

/**
 ****************************************************************************************
 * @brief Calculate one of the subkey used during the signature generation/verification
 *        procedure.
 *
 * @param[in] gen_k2        true if the returned subkeys is k2, false if k1
 * @param[in] l_value       L value obtained from the CSRK.
 * @param[in|out] subkey    Buffer which will contain the generated subkey.
 ****************************************************************************************
 */
void smpc_calc_subkeys(bool gen_k2, uint8_t *l_value, uint8_t *subkey);

/**
 ****************************************************************************************
 * @brief Start to send the keys defined during the pairing features exchange procedure.
 *
 * @param[in] conidx        Connection Index
 * @param[in] role          Current role of the device
 ****************************************************************************************
 */
void smpc_tkdp_send_start(uint8_t conidx, uint8_t role);

/**
 ****************************************************************************************
 * @brief Define the next step of TKDP procedure (sending side).
 *
 * @param[in] conidx        Connection Index
 * @param[in] role          Current role of the device
 * @param[in] ltk           LTK to send
 * @param[in] mst_id_info   Master Identifier structure (EDIV + Random number)
 ****************************************************************************************
 */
void smpc_tkdp_send_continue(uint8_t conidx, uint8_t role, uint8_t *ltk,
                             struct smpc_mst_id_info *mst_id_info);

/**
 ****************************************************************************************
 * @brief Put the task in a state allowing to receive the keys defined during the pairing
 *        features exchange procedure.
 *
 * @param[in] conidx        Connection Index
 * @param[in] role          Current role of the device
 ****************************************************************************************
 */
void smpc_tkdp_rcp_start(uint8_t conidx, uint8_t role);

/**
 ****************************************************************************************
 * @brief Define the next step of TKDP procedure (reception side).
 *
 * @param[in] conidx        Connection Index
 * @param[in] role          Current role of the device
 ****************************************************************************************
 */
void smpc_tkdp_rcp_continue(uint8_t conidx, uint8_t role);

/**
 ****************************************************************************************
 * @brief Inform the HL that the pairing procedure currently in progress is over.
 *
 * @param[in] conidx          Connection Index
 * @param[in] role            Current role of the device
 * @param[in] status          Status
 * @param[in] start_ra_timer  Indicate if the repeated attempts timer shall be started in
 *                            the case of a pairing failed.
 ****************************************************************************************
 */
void smpc_pairing_end(uint8_t conidx, uint8_t role, uint8_t status, bool start_ra_timer);

/**
 ****************************************************************************************
 * @brief Stop the timer used to detect a SMP Timeout
 *
 * @param[in] conidx        Connection Index
 ****************************************************************************************
 */
void smpc_clear_timeout_timer(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Start the timer used to detect a Repeated Attempts attack
 *
 * @param[in] conidx        Connection Index
 ****************************************************************************************
 */
void smpc_launch_rep_att_timer(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Determine the method which will be used to generate the STK during a pairing
 *        procedure
 *
 * @param[in] conidx        Connection Index
 ****************************************************************************************
 */
void smpc_get_key_sec_prop(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Check if the security mode requested by the application or the peer device can
 *        be reached with the exchanged pairing features.
 *
 * @param[in] conidx        Connection Index
 * @param[in] role          Current role of the device
 ****************************************************************************************
 */
bool smpc_is_sec_mode_reached(uint8_t conidx, uint8_t role);

/**
 ****************************************************************************************
 * @brief Define what to do once a start encryption procedure has been successfully finished.
 *
 * @param[in] conidx        Connection Index
 * @param[in] role          Current role of the device
 * @param[in] status        Status
 ****************************************************************************************
 */
void smpc_handle_enc_change_evt(uint8_t conidx, uint8_t role, uint8_t status);

/**
 ****************************************************************************************
 * @brief Send a SMP PDU to the peer device
 *
 * @param[in] conidx        Connection Index
 * @param[in] cmd_code      Code of the PDU to send
 * @param[in] value         Unpacked value
 ****************************************************************************************
 */
void smpc_pdu_send(uint8_t conidx, uint8_t cmd_code, void *value);

/**
 ****************************************************************************************
 * @brief Handle reception of a SMP PDU sent by the peer device.
 *
 * @param[in] conidx        Connection Index
 * @param[in] pdu           Unpacked PDU
 ****************************************************************************************
 */
void smpc_pdu_recv(uint8_t conidx, struct l2cc_pdu *pdu);

#endif //(BLE_CENTRAL || BLE_PERIPHERAL)

#endif //(RW_BLE_USE_CRYPT)

#endif //SMPC_H_

/// @} SMPC
