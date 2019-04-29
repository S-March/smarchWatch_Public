/**
 ****************************************************************************************
 *
 * @file smpc_api.h
 *
 * @brief Header file SMPC API.
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef SMPC_API_H_
#define SMPC_API_H_

/**
 ****************************************************************************************
 * @addtogroup SMPC_API Task
 * @ingroup SMPC
 * @brief Provides a SMP API for controller tasks.
 *
 * The SMPC api is responsible for all security protocol and secure connections handling.
 *
 * @{
 ****************************************************************************************
 */

#include "rwip_config.h"
#include "co_version.h"
#if (BLE_CENTRAL || BLE_PERIPHERAL)
#if (RW_BLE_USE_CRYPT)
#include "smp_common.h"

/*
 * INCLUDE FILES
 ****************************************************************************************
 */


#include "gapc_task.h"
#include "gap.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * FUNCTION DEFINITION
 ****************************************************************************************
 */

#if (BLE_CENTRAL)
/**
 ****************************************************************************************
 * @brief Handles pairing request from GAP, start the pairing procedure
 *
 * @param[in] idx     Connection Index
 * @param[in] pairing Pairing Information
 *
 * @return Status of Pairing start
 ****************************************************************************************
 */
uint8_t smpc_pairing_start(uint8_t idx, struct gapc_pairing  *pairing);
#endif // (BLE_CENTRAL)


/**
 ****************************************************************************************
 * @brief Handles TK exchange part of pairing
 *
 * @param[in] idx     Connection Index
 * @param[in] accept  True if pairing is accepted, False else
 * @param[in] tk      The TK transmitted by application
 *
 * @return status of pairing
 ****************************************************************************************
 */
uint8_t smpc_pairing_tk_exch(uint8_t idx, bool accept,  struct gap_sec_key *tk);

/**
 ****************************************************************************************
 * @brief Handles LTK exchange part of pairing
 *
 * @param[in] idx     Connection Index
 * @param[in] ltk     The LTK transmitted by application
 *
 * @return status of pairing
 ****************************************************************************************
 */
uint8_t smpc_pairing_ltk_exch(uint8_t idx, struct gapc_ltk* ltk);

/**
 ****************************************************************************************
 * @brief Handles CSRK exchange part of pairing
 *
 * @param[in] idx     Connection Index
 * @param[in] csrk    The CSRK transmitted by application
 *
 * @return status of pairing
 ****************************************************************************************
 */
uint8_t smpc_pairing_csrk_exch(uint8_t idx, struct gap_sec_key *csrk);


/**
 ****************************************************************************************
 * @brief Handles reception of pairing response information
 *
 * @param[in] idx     Connection Index
 * @param[in] accept  True if pairing is accepted, False else
 * @param[in] feat    Pairing response feature information
 *
 * @return status of pairing
 ****************************************************************************************
 */
uint8_t smpc_pairing_rsp(uint8_t idx, bool accept, struct gapc_pairing *feat);


#if (BLE_PERIPHERAL)

/**
 ****************************************************************************************
 * @brief Handles reception of pairing request information
 *
 * @param[in] idx     Connection Index
 * @param[in] feat    Pairing request feature information
 ****************************************************************************************
 */
void smpc_pairing_req_handler(uint8_t idx, struct gapc_pairing *feat);

/**
 ****************************************************************************************
 * @brief Handles request to send a security request to peer device
 *
 * @param[in] idx     Connection Index
 * @param[in] auth    Requested Authentication Level
 *
 * @return status of the request
 ****************************************************************************************
 */
uint8_t smpc_security_req_send(uint8_t idx, uint8_t auth);
#endif // (BLE_PERIPHERAL)


#if (BLE_CENTRAL)
/**
 ****************************************************************************************
 * @brief Master requests to start encryption
 *
 * @param[in] idx     Connection Index
 * @param[in] ltk     LTK information
 *
 * @return status of the request
 ****************************************************************************************
 */
uint8_t smpc_encrypt_start(uint8_t idx, struct gapc_ltk *ltk);
#endif //(BLE_CENTRAL)


#if (BLE_PERIPHERAL)

/**
 ****************************************************************************************
 * @brief Handles reception of encryption request
 *
 * @param[in] idx     Connection Index
 * @param[in] ltk     LTK to search information
 ****************************************************************************************
 */
void smpc_encrypt_start_handler(uint8_t idx, struct gapc_ltk *ltk);

/**
 ****************************************************************************************
 * @brief Slave respond to peer device encryption request
 *
 * @param[in] idx      Connection Index
 * @param[in] accept   Accept or not to start encryption
 * @param[in] ltk      LTK information
 * @param[in] key_size Encryption key size
 ****************************************************************************************
 */
void smpc_encrypt_cfm(uint8_t idx, bool accept, struct gap_sec_key *ltk, uint8_t key_size);
#endif //(BLE_PERIPHERAL)


/**
 ****************************************************************************************
 * @brief Request to sign an attribute packet or check signature
 *
 * @param[in] idx      Connection Index
 * @param[in] param    ATT packet information
 *
 * @return status of signature request
 ****************************************************************************************
 */
uint8_t smpc_sign_command(uint8_t idx, struct gapc_sign_cmd *param);


/**
 ****************************************************************************************
 * @brief Continue signature generation or check of an attribute packet after an AES.
 *
 * @param[in] idx      Connection Index
 * @param[in] aes_res  Result of AES calculation
 ****************************************************************************************
 */
void smpc_sign_cont(uint8_t idx, uint8_t* aes_res);

/**
 ****************************************************************************************
 * @brief Continue generation of rand number for confirm value.
 *
 * @param[in] idx      Connection Index
 * @param[in] randnb   Generated Random Number
 ****************************************************************************************
 */
void smpc_confirm_gen_rand(uint8_t idx, struct rand_nb* randnb);

/**
 ****************************************************************************************
 * @brief Continue Calculation of Confirm Value or STK after AES.
 *
 * @param[in] idx      Connection Index
 * @param[in] aes_res  Result of AES calculation
 ****************************************************************************************
 */
void smpc_calc_confirm_cont(uint8_t idx, uint8_t* aes_res);

#if (RWBLE_SW_VERSION_MAJOR >= 8)
void smpc_public_key_exchange_start(uint8_t idx);

void smpc_dhkey_calc_start(uint8_t idx, struct ecdh_key_pair *my_keys);
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */

#endif //(RW_BLE_USE_CRYPT)

#endif // (BLE_CENTRAL || BLE_PERIPHERAL)
#endif //(SMPC_API_H_)

/// @} SMPC_API
