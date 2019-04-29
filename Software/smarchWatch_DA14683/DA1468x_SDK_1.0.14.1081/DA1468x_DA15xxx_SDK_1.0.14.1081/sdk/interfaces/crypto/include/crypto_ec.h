/**
 * \addtogroup BSP
 * \{
 * \addtogroup INTERFACES
 * \{
 * \addtogroup SECURITY_TOOLBOX
 * \{
 * \addtogroup CURVES
 *
 * \brief Elliptic curves data.
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file crypto_ec.h
 *
 * @brief Elliptic curves data.
 *
 * Copyright (C) 2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef CRYPTO_EC_H_
#define CRYPTO_EC_H_

#include <hw_ecc_curves.h>
#include <stdint.h>

/**
 * \brief Elliptic curve parameters.
 */
typedef struct {
        const uint8_t *q;                     /**< Field size. */
        const uint8_t *n;                     /**< Subgroup order. */
        const uint8_t *Gx;                    /**< x coordinate of generator point. */
        const uint8_t *Gy;                    /**< y coordinate of generator point. */
        const uint8_t *a;                     /**< Parameter a of the curve. */
        const uint8_t *b;                     /**< Parameter b of the curve. */
        const uint32_t cmd;                   /**< Command register for the curve. */
        const uint8_t o_sz;                   /**< Operands size (bytes). */
} crypto_ec_params_t;

#define _CRYPTO_EC_CMD(_sb, _sa, _os, _f) \
        ((_sb << ECC_ECC_COMMAND_REG_ECC_SignB_Pos) | (_sa << ECC_ECC_COMMAND_REG_ECC_SignA_Pos) | \
         (_os << ECC_ECC_COMMAND_REG_ECC_SizeOfOperands_Pos) | (_f << ECC_ECC_COMMAND_REG_ECC_Field_Pos))

/**
 * \brief Parameter initialization for secp192r1 curve.
 *
 * This is NIST P-192, ANSI X9.62 prime192v1 curve.
 */
#define CRYPTO_EC_PARAMS_SECP192R1 \
        { hw_ecc_p192_q, hw_ecc_p192_n, hw_ecc_p192_Gx, hw_ecc_p192_Gy, hw_ecc_p192_a, hw_ecc_p192_b, \
          _CRYPTO_EC_CMD(HW_ECC_CMD_SIGNB_POS, HW_ECC_CMD_SIGNA_POS, HW_ECC_CMD_OP_SIZE_256B, HW_ECC_CMD_FIELD_FP), 24 }

/**
 * \brief Parameter initialization for secp224r1 curve.
 *
 * This is NIST P-224 curve.
 */
#define CRYPTO_EC_PARAMS_SECP224R1 \
        { hw_ecc_p224_q, hw_ecc_p224_n, hw_ecc_p224_Gx, hw_ecc_p224_Gy, hw_ecc_p224_a, hw_ecc_p224_b, \
          _CRYPTO_EC_CMD(HW_ECC_CMD_SIGNB_POS, HW_ECC_CMD_SIGNA_POS, HW_ECC_CMD_OP_SIZE_256B, HW_ECC_CMD_FIELD_FP), 28 }

/**
 * \brief Parameter initialization for secp256r1 curve.
 *
 * This is NIST P-256, ANSI X9.62 prime256v1 curve.
 */
#define CRYPTO_EC_PARAMS_SECP256R1 \
        { hw_ecc_p256_q, hw_ecc_p256_n, hw_ecc_p256_Gx, hw_ecc_p256_Gy, hw_ecc_p256_a, hw_ecc_p256_b, \
          _CRYPTO_EC_CMD(HW_ECC_CMD_SIGNB_POS, HW_ECC_CMD_SIGNA_POS, HW_ECC_CMD_OP_SIZE_256B, HW_ECC_CMD_FIELD_FP), 32 }

/**
 * \brief Parameter initialization for Curve25519.
 *
 * This is a Montgomery curve used for fast ECDH.
 */
#define CRYPTO_EC_PARAMS_CURVE25519 \
        { hw_ecc_curve25519_p, NULL, hw_ecc_curve25519_G, NULL, hw_ecc_curve25519_a24, NULL, \
          _CRYPTO_EC_CMD(HW_ECC_CMD_SIGNB_POS, HW_ECC_CMD_SIGNA_POS, HW_ECC_CMD_OP_SIZE_256B, HW_ECC_CMD_FIELD_FP), 32 }

#endif  /* CRYPTO_EC_H_ */

/**
 * \}
 * \}
 * \}
 * \}
 */

