/**
 * \addtogroup BSP
 * \{
 * \addtogroup SYSTEM
 * \{
 * \addtogroup SYS_TRNG
 * 
 * \brief System true number generation
 *
 * \{
 *
 * This service provides random numbers by using the TRNG engine. The TRNG engine contains
 * a FIFO of 32 32-bit random numbers. If the FIFO is not empty a random number can be 
 * provided within a few (~10) cycles. If more than 32 numbers are requested then each
 * number requires on average ~30 cycles.
 *
 * The service is started and stopped by the power manager module.
 *
 * Extra care must be taken when requiring numbers in a specific range. Requested numbers
 * in a range of 0 - 100,000 can take up to several millions of cycles.
 */

/**
 ****************************************************************************************
 *
 * @file sys_trng.h
 *
 * @brief sys_trng header file.
 *
 * Copyright (C) 2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SYS_TRNG_H_
#define SYS_TRNG_H_

#include "sdk_defs.h"

/**
 * \brief Get a random number.
 *
 * \return A random number from the TRNG engine, in the range between 0 and
 *         stdlib's RAND_MAX.
 */
__RETAINED_CODE int sys_trng_rand(void);

/**
 * \brief Get many random bytes
 *
 * The function fills the input buffer with random bytes from the TRNG engine.
 *
 * \param [out] buffer A pointer to the buffer were the bytes will be stored.
 * \param [in] size The number of random bytes requested.
 */
void sys_trng_get_bytes(uint8_t *buffer, size_t size);

/**
 * \brief Get many random numbers
 *
 * The function fills the input buffer with random numbers from the TRNG engine.
 *
 * \param [out] buffer A pointer to the buffer were the numbers will be stored.
 * \param [in] size The number of random numbers requested.
 */
void sys_trng_get_numbers(uint32_t *buffer, size_t size);

/**
 * \brief Get a random number in range.
 *
 * \param [in] range_min The low limit of the range (included in the range).
 * \param [in] range_max The high limit of the range (included in the range).
 *
 * \return A random number from the TRNG engine in the requested range.
 *
 * \warning This function reads random numbers from TRNG until it gets one within
 *          the required range in order to provide the random number uniformly within
 *          this range. This renders the timing of the function unpredictable
 *          especially for narrow ranges.
 */
uint32_t sys_trng_rand_range(uint32_t range_min, uint32_t range_max);

/**
 * \brief Get many random numbers in range.
 *
 * \param [out] buffer A pointer to the buffer were the numbers will be stored.
 * \param [in] size The number of random numbers requested.
 * \param [in] range_min The low limit of the range (included in the range).
 * \param [in] range_max The high limit of the range (included in the range).
 *
 * \warning This function reads random numbers from TRNG until it gets one within
 *          the required range in order to provide random numbers uniformly within
 *          this range. This renders the timing of the function unpredictable
 *          especially for narrow ranges.
 */
void sys_trng_get_numbers_range(uint32_t *buffer, size_t size, uint32_t range_min, uint32_t range_max);

/**
 * \brief Check if the service is producing numbers using TRNG
 *
 * \return 0 if the service is not currently producing numbers using TRNG, 1 otherwise
 */
int sys_trng_producing_numbers(void);

#endif /* SYS_TRNG_H_ */

/**
\}
\}
\}
*/

