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
 * @file crypto_hmac.c
 *
 * @brief Hash-based Message Authentication Code implementation
 *
 * Copyright (C) 2016-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configUSE_HW_AES_HASH

#include <crypto_hmac.h>
#include <hw_aes_hash.h>
#include <ad_crypto.h>
#include <string.h>

#if !dg_configCRYPTO_ADAPTER
#error dg_configCRYPTO_ADAPTER macro must be set to (1) in order to use HMAC.
#endif

#define CRYPTO_HMAC_SHA256_BLOCK_SZ             64 /* bytes */
#define CRYPTO_HMAC_SHA256_OUTPUT_SZ            32 /* bytes */


static crypto_hmac_ctx_t _crypto_hmac_clean(crypto_hmac_ctx_t c)
{
        /* Disable engine clock and adapter event handling */
        ad_crypto_disable_aes_hash_event();
        if (ad_crypto_release_aes_hash() != OS_OK) {
                /* This means that the resource was acquired by a different task or under ISR context.
                   The code should not reach here normally. */
                OS_ASSERT(0);
        }

        return c;
}

crypto_hmac_ctx_t crypto_hmac_sha256(const uint8_t *text, size_t text_sz,
                                     const uint8_t *key, size_t key_sz,
                                     uint8_t *hmac, unsigned int flags,
                                     OS_TICK_TIME timeout)
{
        int i;
        uint8_t k0[CRYPTO_HMAC_SHA256_BLOCK_SZ];
        uint8_t h_k0_ipad[CRYPTO_HMAC_SHA256_BLOCK_SZ];
        uint8_t *h_k0_opad = NULL;

        if (ad_crypto_acquire_aes_hash(timeout) != OS_MUTEX_TAKEN) {
                return -1;
        }
        /* Enable engine clock and adapter event handling */
        ad_crypto_enable_aes_hash_event();

        hw_aes_hash_cfg_sha_256(CRYPTO_HMAC_SHA256_OUTPUT_SZ);

        /* Compute K0 */
        if (key_sz == CRYPTO_HMAC_SHA256_BLOCK_SZ) {
                memcpy(k0, key, key_sz);
        } else if (key_sz > CRYPTO_HMAC_SHA256_BLOCK_SZ) {
                hw_aes_hash_mark_input_block_as_last();
                hw_aes_hash_cfg_dma(key, (uint8_t *)k0, key_sz);
                hw_aes_hash_start();
                /* Fill the remaining k0 buffer with zero */
                memset(&k0[CRYPTO_HMAC_SHA256_OUTPUT_SZ], 0, (CRYPTO_HMAC_SHA256_BLOCK_SZ - CRYPTO_HMAC_SHA256_OUTPUT_SZ));
                ad_crypto_wait_aes_hash_event(OS_EVENT_FOREVER, NULL);
        } else {
                memcpy(k0, key, key_sz);
                memset(&k0[key_sz], 0, (CRYPTO_HMAC_SHA256_BLOCK_SZ - key_sz));
        }

        /* Setup h_k0_buffer */
        if (flags & CRYPTO_HMAC_I_TXT) {
                h_k0_opad = OS_MALLOC(CRYPTO_HMAC_SHA256_BLOCK_SZ);
                if (!h_k0_opad) {
                        return _crypto_hmac_clean(-2);
                }
        } else {
                /* k0 can be reused here because the sequence in the next loop
                   sets up ipad first from k0. After this, k0 is not needed anymore
                   and can be used to store k_k0_opad */
                h_k0_opad = k0;
        }

        /* Compute (K0 xor opad) and start HASH(K0 xor ipad) */
        for (i = 0; i < CRYPTO_HMAC_SHA256_BLOCK_SZ; i += 4) {
                h_k0_ipad[i + 0] = k0[i + 0] ^ 0x36;
                h_k0_opad[i + 0] = k0[i + 0] ^ 0x5c;
                h_k0_ipad[i + 1] = k0[i + 1] ^ 0x36;
                h_k0_opad[i + 1] = k0[i + 1] ^ 0x5c;
                h_k0_ipad[i + 2] = k0[i + 2] ^ 0x36;
                h_k0_opad[i + 2] = k0[i + 2] ^ 0x5c;
                h_k0_ipad[i + 3] = k0[i + 3] ^ 0x36;
                h_k0_opad[i + 3] = k0[i + 3] ^ 0x5c;
        }
        hw_aes_hash_mark_input_block_as_not_last();
        hw_aes_hash_cfg_dma((const uint8_t *)h_k0_ipad, NULL, CRYPTO_HMAC_SHA256_BLOCK_SZ);
        hw_aes_hash_start();
        ad_crypto_wait_aes_hash_event(OS_EVENT_FOREVER, NULL);

        /* Compute HASH((K0 xor ipad) || text) */
        if (flags & CRYPTO_HMAC_I_TXT) {
                /* In case text is not complete, its size must be a multiple of 8 */
                OS_ASSERT(!(text_sz & 0x7));
                hw_aes_hash_cfg_dma(text, NULL, text_sz);
        } else {
                hw_aes_hash_mark_input_block_as_last();
                hw_aes_hash_cfg_dma(text, hmac, text_sz);
        }
        hw_aes_hash_start();
        ad_crypto_wait_aes_hash_event(OS_EVENT_FOREVER, NULL);

        if (flags & CRYPTO_HMAC_I_TXT) {
                /* Operation will not complete until the remaining text is received */
                return  (crypto_hmac_ctx_t)h_k0_opad;
        }

        /* Compute HASH((K0 xor opad) || HASH((K0 xor ipad) || text)) */
        hw_aes_hash_mark_input_block_as_not_last();
        hw_aes_hash_cfg_dma((const uint8_t *)h_k0_opad, NULL, CRYPTO_HMAC_SHA256_BLOCK_SZ);
        hw_aes_hash_start();
        ad_crypto_wait_aes_hash_event(OS_EVENT_FOREVER, NULL);
        hw_aes_hash_mark_input_block_as_last();
        hw_aes_hash_cfg_dma((const uint8_t *)hmac, hmac, CRYPTO_HMAC_SHA256_OUTPUT_SZ);
        hw_aes_hash_start();
        ad_crypto_wait_aes_hash_event(OS_EVENT_FOREVER, NULL);

        return _crypto_hmac_clean(0);
}

void crypto_hmac_sha256_continue(const uint8_t *text, size_t text_sz, crypto_hmac_ctx_t context,
                                 unsigned int flags, uint8_t *hmac)
{
        if (flags & CRYPTO_HMAC_I_TXT) {
                /* In case text is not complete, its size must be a multiple of 8 */
                OS_ASSERT(!(text_sz & 0x7));
                /* Continue computing HASH((K0 xor ipad) || text) */
                hw_aes_hash_cfg_dma(text, NULL, text_sz);
                hw_aes_hash_start();
                ad_crypto_wait_aes_hash_event(OS_EVENT_FOREVER, NULL);
        } else {
                /* Finalize computing HASH((K0 xor ipad) || text) */
                hw_aes_hash_mark_input_block_as_last();
                hw_aes_hash_cfg_dma(text, hmac, text_sz);
                hw_aes_hash_start();
                ad_crypto_wait_aes_hash_event(OS_EVENT_FOREVER, NULL);

                /* Compute HASH((K0 xor opad) || HASH((K0 xor ipad) || text)) */
                const uint8_t *h_k0_opad = (const uint8_t *)context;
                hw_aes_hash_mark_input_block_as_not_last();
                hw_aes_hash_cfg_dma(h_k0_opad, NULL, CRYPTO_HMAC_SHA256_BLOCK_SZ);
                hw_aes_hash_start();
                ad_crypto_wait_aes_hash_event(OS_EVENT_FOREVER, NULL);

                hw_aes_hash_mark_input_block_as_last();
                hw_aes_hash_cfg_dma((const uint8_t *)hmac, hmac, CRYPTO_HMAC_SHA256_OUTPUT_SZ);
                hw_aes_hash_start();
                OS_FREE((void *)h_k0_opad);
                ad_crypto_wait_aes_hash_event(OS_EVENT_FOREVER, NULL);

                /* Disable engine clock and adapter even handling */
                ad_crypto_disable_aes_hash_event();
                if (ad_crypto_release_aes_hash() != OS_OK) {
                        /* This means that the resource was acquired by a different task or under ISR context.
                           The code should not reach here normally. */
                        OS_ASSERT(0);
                }
        }
}

#endif /* dg_configUSE_HW_AES_HASH */

/**
 * \}
 * \}
 * \}
 * \}
 */

