/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup ECC_uCODE
 * \{
 * \brief ECC microcode
 */

/**
 ****************************************************************************************
 *
 * @file hw_ecc_ucode.h
 *
 * @brief ECC Engine microcode.
 *
 * Copyright (C) 2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_ECC_UCODE_H_
#define HW_ECC_UCODE_H_

#if dg_configUSE_HW_ECC

#include <stdint.h>

/**
 * \brief Size of ECC engine microcode (in bytes)
 */
#define HW_ECC_UCODE1_SIZE  (1535 * 4)

/**
 * \brief ECC engine microcode
 *
 * This microcode supports the following features:
 * - Primitive arithmetic
 * - Support for the following curves:
 *   + NIST recommended curves up to 256 bits (prime and binary fields)
 *   + Brainpool and SECG curves up to 256 bits
 *   + Curve25519
 *   + Ed25519
 * - Primitive ECC and check point operations
 * - High-level ECC operations forECDSA, EdDSA, J-PAKE, ECMQV
 */
extern const uint32_t hw_ecc_ucode1[];

#endif /* dg_configUSE_HW_ECC */

#endif /* HW_ECC_UCODE_H_ */

/**
 * \}
 * \}
 * \}
 */
