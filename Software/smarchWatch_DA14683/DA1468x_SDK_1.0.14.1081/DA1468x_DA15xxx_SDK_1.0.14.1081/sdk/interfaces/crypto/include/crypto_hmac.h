/**
 * \addtogroup BSP
 * \{
 * \addtogroup INTERFACES
 * \{
 * \addtogroup SECURITY_TOOLBOX
 * \{
 * \addtogroup HMAC
 *
 * \brief Hash-based Message Authentication Code implementation (RFC2104)
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file crypto_hmac.h
 *
 * @brief Hash-based Message Authentication Code API
 *
 * Copyright (C) 2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef CRYPTO_HMAC_H_
#define CRYPTO_HMAC_H_

#include <osal.h>
#include <stdint.h>
#include <stddef.h>

/**
 * \brief Options passed in flags of the HMAC API
 */
typedef enum {
        CRYPTO_HMAC_NO_OPTION = 0x0UL,  /**< No option. */
        CRYPTO_HMAC_I_TXT = 0x1UL,      /**< The text passed is not complete. More chunks will follow. */
        CRYPTO_HMAC_F_TXT = 0x2UL       /**< The text passed is the final chunk. */
} CRYPTO_HMAC_OPTIONS;

/**
 * \brief HMAC context type
 */
typedef intptr_t crypto_hmac_ctx_t;

/**
 * \brief Calculate the HMAC of a message and a key using SHA256 for hashing.
 *
 * This function is used to calculate the hash-based message authentication code of a message ("text")
 * using SHA256 as hashing algorithm. It takes as input the text and a key and produces the message
 * authentication code. Since the function needs to acquire a hardware resource for calculating hashes,
 * it also takes as input a timeout value used when waiting to acquire the resource. Here is an example
 * use:
 *
 * \code{c}
 * uint8_t txt[] = "what do ya want for nothing?"
 * uint8_t key[] = "Jefe"
 * uint8_t hmac[32];
 *
 * crypto_hmac_ctx_t hmac_status = crypto_hmac_sha256(txt, 28, key, 4, hmac, CRYPTO_HMAC_NO_OPTION, 100);
 *
 * if (hmac_status == -1) {
 *      // Failed to acquire the AES/HASH engine within 100 ticks
 * } else {
 *      // Everything is fine
 * }
 * \endcode
 *
 * It is possible to call this function even if the complete message is partially available, by setting
 * the ::CRYPTO_HMAC_I_TXT flag. The size of text must be a multiple of 8. The operation concludes when
 * crypto_hmac_sha256_continue() is called with ::CRYPTO_HMAC_F_TXT flag set. Here is an example use:
 *
 * \code{c}
 * uint8_t txt1[] = "what do "
 * uint8_t txt2[] = "ya want "
 * uint8_t txt3[] = "for nothing?"
 * uint8_t key[] = "Jefe"
 * uint8_t hmac[32];
 *
 * crypto_hmac_ctx_t hmac_status = crypto_hmac_sha256(txt1, 8, key, 4, NULL, CRYPTO_HMAC_I_TXT, 100);
 *
 * if (hmac_status == -1) {
 *      // Failed to acquire the AES/HASH engine within 100 ticks
 * } else if (hmac_status == -2) {
 *      // Failed to allocate memory for the context
 * } else {
 *      // Everything is fine. The AES/HASH engine is acquired, configured and ready to continue.
 *      crypto_hmac_sha256_continue(txt2, 8, hmac_status, CRYPTO_HMAC_I_TXT, NULL);
 *      crypto_hmac_sha256_continue(txt3, 8, hmac_status, CRYPTO_HMAC_F_TXT, hmac);
 *      // The hmac is generated, the engine is released and availabe for further use
 * }
 * \endcode
 *
 * \param [in]  text    A buffer containing the data.
 * \param [in]  text_sz The size of the data (must be a multiple of 8 if ::CRYPTO_HMAC_I_TXT is
 *                      set in the flags).
 * \param [in]  key     A buffer containing the key.
 * \param [in]  key_sz  The size of the key.
 * \param [out] hmac    A buffer where the result will be stored, the size of which must be 32 bytes.
 *                      It can be NULL if ::CRYPTO_HMAC_I_TXT is set in the flags.
 * \param [in]  flags   Options to pass. Valid options are ::CRYPTO_HMAC_NO_OPTION, ::CRYPTO_HMAC_I_TXT and
 *                      ::CRYPTO_HMAC_F_TXT (equivalent to ::CRYPTO_HMAC_NO_OPTION).
 * \param [in]  timeout Time in ticks to wait while trying to acquire hardware resources.
 *
 * \return Returns a crypto HMAC context ID. The ID can take the following values:
 *         - 0 if the operation has completed and hmac contains the calculated HMAC.
 *         - -1 if the operation timed-out before acquiring the necessary hardware resource.
 *         - -2 in case of memory allocation failure. This can only happen if ::CRYPTO_HMAC_I_TXT
 *              is set in the flags.
 *         - Any other value if the operation is incomplete waiting for more text data
 *           before completing. This is the case where ::CRYPTO_HMAC_I_TXT is set in the flags. In
 *           this case the context must be used in subsequent calls to crypto_hmac_sha256_continue().
 *
 * \warning When this function returns after being called with ::CRYPTO_HMAC_I_TXT flag, the system
 *          is in a state where the AES/HASH engine is acquired (and hence no other task can use it) and
 *          the system does not go to sleep. The system remains in this state until
 *          crypto_hmac_sha256_continue() is called with ::CRYPTO_HMAC_F_TXT flag set.
 *
 * \sa crypto_hmac_sha256_continue()
 */
crypto_hmac_ctx_t crypto_hmac_sha256(const uint8_t *text, size_t text_sz,
                                     const uint8_t *key, size_t key_sz,
                                     uint8_t *hmac, unsigned int flags,
                                     OS_TICK_TIME timeout);

/**
 * \brief Continue the HMAC calculation.
 *
 * This function is used in case the initial call to HMAC calculation had incomplete text.
 * When the next chunk of text is available then this function is called by providing the
 * text chunk, the context returned by the initial calculation call, a flag marking whether
 * this is the last text chunk or not and a buffer where the result will be stored if this
 * is the last text chunk.
 *
 * \param [in]  text A  buffer containing the data.
 * \param [in]  text_sz The size of the data (must be a multiple of 8 if ::CRYPTO_HMAC_I_TXT is
 *                      set in the flags).
 * \param [in]  context The context returned from the initial call to HMAC generation.
 * \param [in]  flags   Options to pass. Valid options are ::CRYPTO_HMAC_NO_OPTION, ::CRYPTO_HMAC_I_TXT and
 *                      ::CRYPTO_HMAC_F_TXT (equivalent to ::CRYPTO_HMAC_NO_OPTION).
 * \param [out] hmac    A buffer where the result will be stored, the size of which must be 32 bytes.
 *                      It can be NULL if ::CRYPTO_HMAC_I_TXT is set in the flags.
 *
 * \note This function assumes that the AES/HASH hardware engine has been already acquired and properly
 *       initialized by the initial HMAC calculation call. When the text is marked as final, then it
 *       releases the engine which becomes again available for use.
 *
 * \sa crypto_hmac_sha256()
 */
void crypto_hmac_sha256_continue(const uint8_t *text, size_t text_sz, crypto_hmac_ctx_t context,
                                 unsigned int flags, uint8_t *hmac);
#endif /* CRYPTO_HMAC_H_ */

/**
 * \}
 * \}
 * \}
 * \}
 */

