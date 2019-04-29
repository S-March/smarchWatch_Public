/**
 * \addtogroup BSP
 * \{
 * \addtogroup INTERFACES
 * \{
 * \addtogroup SECURITY_TOOLBOX
 * \{
 * \addtogroup HMAC
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file crypto_ecdh.c
 *
 * @brief ECDH API implementation.
 *
 * Copyright (C) 2016-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configUSE_HW_ECC

#include <crypto_ecdh.h>
#include <sys_trng.h>
#include <ad_crypto.h>
#include <hw_ecc.h>


#if !dg_configCRYPTO_ADAPTER
#error dg_configCRYPTO_ADAPTER macro must be set to (1) in order to use ECDH.
#endif

#if (CRYPTO_ECDH_DO_NOT_USE_CURVE25519 == 1) && (CRYPTO_ECDH_USE_ONLY_CURVE25519 == 1)
#error Conflicting configuration macros for ECDH module.
#endif

#define CRYPTO_ECDH_ENABLE_CALCR2       (1 << ECC_ECC_COMMAND_REG_ECC_CalcR2_Pos)
#define CRYPTO_ECDH_ENABLE_SIGNB        (1 << ECC_ECC_COMMAND_REG_ECC_SignB_Pos)

static inline void _crypto_ecdh_cleanup(void)
{
        ad_crypto_disable_ecc_event();
        if (ad_crypto_release_ecc() != OS_OK) {
                /* This means that the resource was acquired by a different task or under ISR context.
                   The code should not reach here normally. */
                OS_ASSERT(0);
        }
}

static CRYPTO_ECDH_RET crypto_ecdh_compute_private_key(crypto_ec_params_t *curve, uint8_t *d)
{

#if (CRYPTO_ECDH_DO_NOT_USE_CURVE25519 == 0)
        if (curve->q == hw_ecc_curve25519_p) {
                sys_trng_get_bytes(d, 32);
                return CRYPTO_ECDH_RET_OK;
        }
#endif
#if (CRYPTO_ECDH_USE_ONLY_CURVE25519 == 0)
        if (curve->q != hw_ecc_curve25519_p) {
                uint32_t cmd = curve->cmd | CRYPTO_ECDH_ENABLE_SIGNB | HW_ECC_CMD_OP_CHECK_PXY;
                volatile uint8_t *base_addr = ad_crypto_get_ecc_base_addr();
                unsigned int ecc_status = 0;

                /* Curve operand size cannot be larger than 32 bytes (256-bits) */
                ASSERT_WARNING(curve->o_sz <= 32);

                memset(d, 0, 32 - curve->o_sz); /* make sure it is properly zero-padded */
                hw_ecc_write256_r(1, curve->n, base_addr);
                hw_ecc_write_command_register_raw(cmd);
                hw_ecc_cfg_ops(6, 0, 0);
                /* Loop until we get a number smaller than the cyclic subgroup order n */
                do {
                        sys_trng_get_bytes(&d[32 - curve->o_sz], curve->o_sz);
                        d[31] |= 1; /* Avoid the extremely unlikely case of d = 0 */
                        hw_ecc_write256_r(6, d, base_addr);
                        hw_ecc_start();

                        ad_crypto_wait_ecc_event(OS_EVENT_FOREVER, &ecc_status);
                } while (ecc_status & HW_ECC_STATUS_COUPLE_NOT_VALID);

                return CRYPTO_ECDH_RET_OK;
        }
#endif

        return CRYPTO_ECDH_RET_ER;
}

static CRYPTO_ECDH_RET crypto_ecdh_compute_public_key(crypto_ec_params_t *curve, uint8_t *d, uint8_t *Q,
                                                      unsigned int full_setup_needed)
{
        uint32_t cmd;
        unsigned int ecc_status = 0;
        volatile uint8_t *base_addr = ad_crypto_get_ecc_base_addr();

#if (CRYPTO_ECDH_DO_NOT_USE_CURVE25519 == 0)
        if (curve->q == hw_ecc_curve25519_p) {
                volatile uint8_t *din = (uint8_t *)hw_ecc_get_location_address(4, base_addr);
                cmd = curve->cmd | CRYPTO_ECDH_ENABLE_CALCR2 | HW_ECC_CMD_OP_C25519_PNT_MULT;

                hw_ecc_write256(0, curve->q, base_addr);
                hw_ecc_write256(2, curve->Gx, base_addr);
                hw_ecc_write256(3, curve->a, base_addr);
                hw_ecc_write256(4, (const uint8_t *)d, base_addr);
                din[0] &= 248;
                din[31] &= 127;
                din[31] |= 64;
                hw_ecc_cfg_ops(2, 4, 6);
                hw_ecc_write_command_register_raw(cmd);
                hw_ecc_start();

                ad_crypto_wait_ecc_event(OS_EVENT_FOREVER, &ecc_status);

                if (ecc_status != 0) {
                        return CRYPTO_ECDH_RET_EE;
                }

                hw_ecc_read256(6, Q, base_addr);

                return CRYPTO_ECDH_RET_OK;
        }
#endif
#if (CRYPTO_ECDH_USE_ONLY_CURVE25519 == 0)
        if (curve->q != hw_ecc_curve25519_p) {
                cmd = curve->cmd | CRYPTO_ECDH_ENABLE_CALCR2 | HW_ECC_CMD_OP_POINT_MLT;

                hw_ecc_write256_r(0, curve->q, base_addr);
                hw_ecc_write256_r(2, curve->Gx, base_addr);
                hw_ecc_write256_r(3, curve->Gy, base_addr);
                hw_ecc_write256_r(4, curve->a, base_addr);
                hw_ecc_write256_r(5, curve->b, base_addr);
                if (full_setup_needed) {
                        hw_ecc_write256_r(6, d, base_addr);
                }
                hw_ecc_cfg_ops(2, 6, 8);
                hw_ecc_write_command_register_raw(cmd);
                hw_ecc_start();

                ad_crypto_wait_ecc_event(OS_EVENT_FOREVER, &ecc_status);

                if (ecc_status != 0) {
                        return CRYPTO_ECDH_RET_EE;
                }

                hw_ecc_read256_r(8, Q, base_addr);
                hw_ecc_read256_r(9, Q + 32, base_addr);

                return CRYPTO_ECDH_RET_OK;
        }
#endif

        return CRYPTO_ECDH_RET_ER;
}

static CRYPTO_ECDH_RET crypto_ecdh_compute_shared_secret(crypto_ec_params_t *curve, uint8_t *d, uint8_t *Qp,
                                                         uint8_t *s, unsigned int full_setup_needed)
{
        uint32_t cmd;
        unsigned int ecc_status = 0;
        volatile uint8_t *base_addr = ad_crypto_get_ecc_base_addr();

#if (CRYPTO_ECDH_DO_NOT_USE_CURVE25519 == 0)
        if (curve->q == hw_ecc_curve25519_p) {
                cmd = curve->cmd | HW_ECC_CMD_OP_C25519_PNT_MULT;

                hw_ecc_write256(2, Qp, base_addr);
                if (full_setup_needed) {
                        hw_ecc_write256(0, curve->q, base_addr);
                        hw_ecc_write256(3, curve->a, base_addr);
                        hw_ecc_write256(4, d, base_addr);
                        cmd |= CRYPTO_ECDH_ENABLE_CALCR2;
                }
                hw_ecc_cfg_ops(2, 4, 8);
                hw_ecc_write_command_register_raw(cmd);
                hw_ecc_start();

                ad_crypto_wait_ecc_event(OS_EVENT_FOREVER, &ecc_status);

                if (ecc_status != 0) {
                        return CRYPTO_ECDH_RET_EE;
                }

                hw_ecc_read256(8, s, base_addr);

                return CRYPTO_ECDH_RET_OK;
        }
#endif
#if (CRYPTO_ECDH_USE_ONLY_CURVE25519 == 0)
        if (curve->q != hw_ecc_curve25519_p) {
                cmd = curve->cmd | HW_ECC_CMD_OP_POINT_MLT;
                hw_ecc_write256_r(10, Qp, base_addr);
                hw_ecc_write256_r(11, (Qp + 32), base_addr);
                if (full_setup_needed) {
                        hw_ecc_write256_r(0, curve->q, base_addr);
                        hw_ecc_write256_r(4, curve->a, base_addr);
                        hw_ecc_write256_r(5, curve->b, base_addr);
                        hw_ecc_write256_r(6, d, base_addr);
                        cmd |= CRYPTO_ECDH_ENABLE_CALCR2;
                }
                hw_ecc_cfg_ops(10, 6, 12);
                hw_ecc_write_command_register_raw(cmd);
                hw_ecc_start();

                ad_crypto_wait_ecc_event(OS_EVENT_FOREVER, &ecc_status);

                if (ecc_status != 0) {
                        if ((ecc_status & HW_ECC_STATUS_FAIL_LOCATION) == 10) {
                                return CRYPTO_ECDH_RET_IP;
                        } else {
                                return CRYPTO_ECDH_RET_EE;
                        }
                }

                hw_ecc_read256_r(12, s, base_addr);

                return CRYPTO_ECDH_RET_OK;
        }
#endif

        return CRYPTO_ECDH_RET_ER;
}

CRYPTO_ECDH_RET crypto_ecdh_compute(crypto_ecdh_context_t *ctx, OS_TICK_TIME timeout)
{
        unsigned int flags = 0; /* This variable will hold operations performed within this call */
        if (!ctx) {
                return CRYPTO_ECDH_RET_ER;
        }

        /* Enable engine clock and adapter event handling */
        if (ad_crypto_acquire_ecc(timeout) != OS_MUTEX_TAKEN) {
                return CRYPTO_ECDH_RET_TO;
        }

        ad_crypto_enable_ecc_event();

        /* Compute private key */
        if (!(ctx->flags & CRYPTO_ECDH_CTX_d)) {
                CRYPTO_ECDH_RET ret = crypto_ecdh_compute_private_key(&ctx->curve, ctx->d);
                if (ret != CRYPTO_ECDH_RET_OK) {
                        _crypto_ecdh_cleanup();
                        return ret;
                }
                flags |= CRYPTO_ECDH_CTX_d;
                ctx->flags |= CRYPTO_ECDH_CTX_d;
                ctx->flags &= ~CRYPTO_ECDH_CTX_Ql; /* If a new d is computed, previous Q is not valid */
        }

        /* Compute public key */
        if (!(ctx->flags & CRYPTO_ECDH_CTX_Ql)) {
                CRYPTO_ECDH_RET ret = crypto_ecdh_compute_public_key(&ctx->curve, ctx->d, &ctx->Ql[0][0], !(flags & CRYPTO_ECDH_CTX_d));
                if (ret != CRYPTO_ECDH_RET_OK) {
                        _crypto_ecdh_cleanup();
                        return ret;
                }
                flags |= CRYPTO_ECDH_CTX_Ql;
                ctx->flags |= CRYPTO_ECDH_CTX_Ql;
        }

        /* Check for peer's public key availability. If not available there is nothing more to do so return. */
        if (!(ctx->flags & CRYPTO_ECDH_CTX_Qp)) {
                _crypto_ecdh_cleanup();
                return CRYPTO_ECDH_RET_MP;
        }

        /* Compute shared secret */
        if (!(ctx->flags & CRYPTO_ECDH_CTX_s)) {
                CRYPTO_ECDH_RET ret = crypto_ecdh_compute_shared_secret(&ctx->curve, ctx->d, &ctx->Qp[0][0],
                                                                          ctx->s, !(flags & CRYPTO_ECDH_CTX_Ql));
                if (ret != CRYPTO_ECDH_RET_OK) {
                        _crypto_ecdh_cleanup();
                        return ret;
                }
                ctx->flags |= CRYPTO_ECDH_CTX_s;
        }

        _crypto_ecdh_cleanup();

        return CRYPTO_ECDH_RET_OK;
}

#endif /* dg_configUSE_HW_ECC */

/**
 * \}
 * \}
 * \}
 * \}
 */

