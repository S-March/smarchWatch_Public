/**
 * \addtogroup BSP
 * \{
 * \addtogroup INTERFACES
 * \{
 * \addtogroup SECURITY_TOOLBOX
 * \{
 * \addtogroup ECDH
 *
 * \brief Elliptic Curve Diffie-Hellman key agreement protocol.
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file crypto_ecdh.h
 *
 * @brief ECDH API.
 *
 * Copyright (C) 2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef CRYPTO_ECDH_H_
#define CRYPTO_ECDH_H_

#include <osal.h>
#include <crypto_ec.h>
#include <stdint.h>

/**
 * \brief Do not use Curve25519.
 *
 * This macro can be set to 1 if your application does not intend to use Curve25519 for ECDH.
 * The benefit is that code size is reduced.
 */
#define CRYPTO_ECDH_DO_NOT_USE_CURVE25519       (0)

/**
 * \brief Use only Curve25519.
 *
 * This macro can be set to 1 if your application uses only Curve25519 for ECDH and no other curve.
 * The benefit is that code size is reduced.
 */
#define CRYPTO_ECDH_USE_ONLY_CURVE25519         (0)

/**
 * \brief ECDH API return codes.
 */
typedef enum {
        CRYPTO_ECDH_RET_OK = 0UL,       /**< No error. */
        CRYPTO_ECDH_RET_TO = 1UL,       /**< Operation timed out. */
        CRYPTO_ECDH_RET_EE = 2UL,       /**< ECC operation error. */
        CRYPTO_ECDH_RET_MP = 3UL,       /**< Missing peer public key. */
        CRYPTO_ECDH_RET_IP = 4UL,       /**< Invalid peer public key. */
        CRYPTO_ECDH_RET_ER = 5UL        /**< Other error. */
} CRYPTO_ECDH_RET;

/**
 * \brief ECDH context flags type.
 */
enum crypto_ecdh_context_flags {
        CRYPTO_ECDH_CTX_d = 0x1,                /**< Private key is present in the context. */
        CRYPTO_ECDH_CTX_Ql = 0x2,               /**< The local public key is present in the context. */
        CRYPTO_ECDH_CTX_Qp = 0x4,               /**< The peer's public key is present in the context. */
        CRYPTO_ECDH_CTX_s = 0x8,                /**< The shared secret is present in the context. */
};

/**
 * \brief ECDH context.
 *
 * The context data are stored. For curves with operands smaller than 256-bits, zero padding is performed.
 * A context should be initialized using #CRYPTO_ECDH_INIT_CTX and a curve from \link CURVES crypto curves \endlink
 * and having its flags set appropriately before passing it as argument to any function. Data are stored in big endian
 * format, except from when using Curve25516 (#CRYPTO_EC_PARAMS_CURVE25519).
 *
 * \note y coordinates are not used with Curve25519.
 */
typedef struct {
        uint8_t d[32];                          /**< Our private key. */
        uint8_t Ql[2][32];                      /**< Our public key (x coordinate in [0], y coordinate in [1]). */
        uint8_t Qp[2][32];                      /**< The peer's public key (x coordinate in [0], y coordinate in [1]). */
        uint8_t s[32];                          /**< The shared secret. */
        crypto_ec_params_t curve;               /**< The curve used. */
        unsigned int flags;                     /**< ECDH context flags (::crypto_ecdh_context_flags). */
} crypto_ecdh_context_t;

/**
 * \brief Initialize ECDH context.
 *
 * \param [in] curve_init       The curve initilization macro. Details can be found in \link CURVES crypto
 *                              curves \endlink documentation.
 */
#define CRYPTO_ECDH_INIT_CTX(curve_init)       { {0}, {{0}, {0}}, {{0}, {0}}, {0}, curve_init, 0 }

/**
 * \brief Generate Elliptic Curve Diffie-Hellman (ECDH) key pair.
 *
 * This function implements the steps defined by the ECDH algorithm, depending on the input context contents.
 * In more detail the steps are:
 *
 * 1. If the ::CRYPTO_ECDH_CTX_d flag is not set, the function will compute a private key, store it in the
 *    context and update the flag. This step invalidates any existing public key in the context.
 *
 * 2. If the ::CRYPTO_ECDH_CTX_Ql flag is not set or if a private key has been computed in the previous step,
 *    the function will compute a public key and update the flag.
 *
 * 3. If the ::CRYPTO_ECDH_CTX_Qp flag (peer's public key) is not set the function will return ::CRYPTO_ECDH_RET_MP.
 *    Otherwise it will check the peer's public key validity and if found valid it will calculate the shared secret
 *    and update the flag.
 *
 * The resulting shared secret may be passed through a key-derivation function (KDF) to derive a symmetric key.
 *
 * The following example shows how this function can be used to generate a public key and a shared secret based on
 * the secp256r1 curve (NIST P-256).
 *
 * \code{c}
 * // Create and initialize the ECDH context
 * crypto_ecdh_context_t c = CRYPTO_ECDH_INIT_CTX(CRYPTO_EC_PARAMS_SECP256R1);
 *
 * // Generate public key
 * if (crypto_ecdh_compute(&c, 100) != CRYPTO_ECDH_RET_MP) {
 *      // handle error
 * }
 *
 * // Exchange public keys (details of the exchange are out of scope for the example)
 * exchange_public_keys(c.Ql, c.Qp);
 * c.flags |= CRYPTO_ECDH_CTX_Qp;
 *
 * // Generate shared secret
 * if (crypto_ecdh_compute(&c, 100) != CRYPTO_ECDH_RET_OK) {
 *      // handle error
 * }
 *
 * // At this point c.s contains the shared secret and relevant flags are set
 * \endcode
 *
 * The next example shows how to generate a public key and a shared secret, if the peer's public key is available
 * before calling this function. The example is based on Curve25519.
 *
 * \code{c}
 * crypto_ecdh_context_t c = CRYPTO_ECDH_INIT_CTX(CRYPTO_EC_PARAMS_CURVE25519);
 *
 * // Get peer's public key (details are out of scope for the example)
 * get_peer_public_key(c.Qp);
 * c.flags |= CRYPTO_ECDH_CTX_Qp;
 *
 * // Generate public key and shared secret
 * if (crypto_ecdh_compute(&c, 100) != CRYPTO_ECDH_RET_OK) {
 *      // handle error
 * }
 *
 * // At this point c.s contains the shared secret and Ql the local public key (relevant
 * // flags are also set). The public key can be sent to the peer (details are out of scope for the example)
 * send_public_key(c.Ql);
 * \endcode
 *
 * \param [in,out] ctx     The ECDH context where the result will be stored. It must be properly initialized with INIT_ECDH_CTX()
 *                         and one of the \link CURVES supported curves \endlink.
 * \param [in]     timeout Time in ticks to wait while trying to acquire hardware resources.
 *
 * \returns The function returns one of the return codes defined in ::CRYPTO_ECDH_RET.
 */
CRYPTO_ECDH_RET crypto_ecdh_compute(crypto_ecdh_context_t *ctx, OS_TICK_TIME timeout);

#endif  /* CRYPTO_ECDH_H_ */

/**
 * \}
 * \}
 * \}
 * \}
 */

