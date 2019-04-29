/**
 ****************************************************************************************
 *
 * @file smpm.h
 *
 * @brief Header file - SMPM.
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef SMPM_H_
#define SMPM_H_

/**
 ****************************************************************************************
 * @addtogroup SMPM Security Manager Protocol Manager
 * @ingroup SMP
 * @brief Security Manager Protocol Manager.
 *
 * This Module allows the 1-instanced modules to communicate with multi-instanced SMPC module.
 * It is only an intermediary between the actual SMPC handling SM behavior, and
 * HCI, GAP, or GATT which only indicate the index of the connection for which
 * SMPC actions are necessary.
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "smp_common.h"       // Firmware Configuration Flags
#include "co_version.h"

#if (RW_BLE_USE_CRYPT)

#include "ke_msg.h"

/*
 * DEFINES
 ****************************************************************************************
 */

// Length of resolvable random address prand part
#define SMPM_RAND_ADDR_PRAND_LEN            (3)
// Length of resolvable random address hash part
#define SMPM_RAND_ADDR_HASH_LEN             (3)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/*
 * STRUCTURES
 ****************************************************************************************
 */
#if (RWBLE_SW_VERSION_MAJOR >= 8)
#define ECDH_PRIVATE_KEY_DEBUG	"\x3f\x49\xf6\xd4\xa3\xc5\x5f\x38\x74\xc9\xb3\xe3\xd2\x10\x3f\x50\x4a\xff\x60\x7b\xeb\x40\xb7\x99\x58\x99\xb8\xa6\xcd\x3c\x1a\xbd"
#define ECDH_PUBLIC_KEYX_DEBUG	"\x20\xb0\x03\xd2\xf2\x97\xbe\x2c\x5e\x2c\x83\xa7\xe9\xf9\xa5\xb9\xef\xf4\x91\x11\xac\xf4\xfd\xdb\xcc\x03\x01\x48\x0e\x35\x9d\xe6"
#define ECDH_PUBLIC_KEYY_DEBUG	"\xdc\x80\x9c\x49\x65\x2a\xeb\x6d\x63\x32\x9a\xbf\x5a\x52\x15\x5c\x76\x63\x45\xc2\x8f\xed\x30\x24\x74\x1c\x8e\xd0\x15\x89\xd2\x8b"

#ifdef ALTER_DEV
#undef ECDH_PUBLIC_KEYX_DEBUG
#define ECDH_PUBLIC_KEYX_DEBUG	"\x55\x18\x8b\x3d\x32\xf6\xbb\x9a\x90\x0a\xfc\xfb\xee\xd4\xe7\x2a\x59\xcb\x9a\xc2\xf1\x9d\x7c\xfb\x6b\x4f\xdd\x49\xf4\x7f\xc5\xfd"
#endif

extern struct ecdh_key_pair ecdh_key;
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */

/*
 * GLOBAL VARIABLES DECLARATION
 ****************************************************************************************
 */


/*
 * PUBLIC FUNCTIONS DECLARATION
 ****************************************************************************************
 */



/*
 * PRIVATE FUNCTIONS DECLARATION
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Send an encryption request to the HCI.
 ****************************************************************************************
 */
void smpm_send_encrypt_req(uint8_t *operand_1, uint8_t *operand_2);

/**
 ****************************************************************************************
 * @brief Send a generate Random Number request to the HCI.
 ****************************************************************************************
 */
void smpm_send_gen_rand_nb_req(void);

/**
 ****************************************************************************************
 * @brief Check the address type provided by the application.
 *
 * @param[in]  addr_type            Provided address type to check
 * @param[out] true if the address type is valid, false else
 ****************************************************************************************
 */
bool smpm_check_addr_type(uint8_t addr_type);

#endif // (RW_BLE_USE_CRYPT)

#endif // (SMPM_H_)

/// @} SMPM
