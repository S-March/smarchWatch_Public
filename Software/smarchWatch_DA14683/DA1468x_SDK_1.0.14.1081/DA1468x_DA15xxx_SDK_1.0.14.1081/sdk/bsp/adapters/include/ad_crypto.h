/**
 * \addtogroup BSP
 * \{
 * \addtogroup ADAPTERS
 * \{
 * \addtogroup CRYPTO
 *
 * \brief Crypto adapter
 *
 * The crypto adapter API can be used in order to ensure exclusive access to the AES/HASH
 * and ECC hardware engines, between tasks. It also provides a mechanism for reloading
 * the ECC engine microcode and configuring the ECC RAM whenever this is needed.
 *
 * Once the engine is acquired, the corresponding driver API can be used for performing
 * the desired operations. More information on how to use the engines can be found in
 * the \link AES_HASH AES hash engine documentation \endlink and in the \link ECC ECC engine
 * documentation \endlink.
 *
 * Example of SHA256 of a buffer:
 * \code{.c}
 * while (ad_crypto_acquire_aes_hash(10) != OS_MUTEX_TAKEN) {
 *      ;
 * }
 * hw_aes_hash_enable_clock();
 * hw_aes_hash_cfg_sha_256(32);
 * hw_aes_hash_cfg_dma(in_buffer, out_buffer, sizeof(in_buffer));
 * hw_aes_hash_start();
 * while (hw_aes_hash_is_active()) {
 *      ;
 * }
 * ad_crypto_release_aes_hash();
 * \endcode
 *
 * Example of a P-256 point multiplication
 * \code{.c}
 *  while (ad_crypto_acquire_ecc(10) != OS_MUTEX_TAKEN) {
 *      ;
 * }
 * hw_ecc_write256_r(0, hw_ecc_p256_q, ad_crypto_get_ecc_base_addr());
 * hw_ecc_write256_r(4, hw_ecc_p256_a, ad_crypto_get_ecc_base_addr());
 * hw_ecc_write256_r(5, hw_ecc_p256_b, ad_crypto_get_ecc_base_addr());
 * hw_ecc_write256_r(6, k, ad_crypto_get_ecc_base_addr());
 * hw_ecc_write256_r(8, P1x, ad_crypto_get_ecc_base_addr());
 * hw_ecc_write256_r(9, P1y, ad_crypto_get_ecc_base_addr);
 * hw_ecc_cfg_ops(8, 6, 10);
 * hw_ecc_write_command_register(HW_ECC_CMD_CALCR2_TRUE,
 *                               HW_ECC_CMD_SIGNB_POS,
 *                               HW_ECC_CMD_SIGNA_POS,
 *                               HW_ECC_CMD_OP_SIZE_256B,
 *                               HW_ECC_CMD_FIELD_FP,
 *                               HW_ECC_CMD_OP_POINT_MLT);
 * hw_ecc_enable_clock();
 * hw_ecc_start();
 * while (hw_ecc_read_status() & HW_ECC_STATUS_BUSY) {
 *      ;
 * }
 * hw_ecc_disable_clock();
 * hw_ecc_read256_r(10, result, ad_crypto_get_ecc_base_addr());
 * hw_ecc_read256_r(11, &result[32], ad_crypto_get_ecc_base_addr());
 * ad_crypto_release_ecc();
 * \endcode
 *
 * \note In order to avoid deadlocks, the calling tasks should avoid blocking forever when
 *       trying to acquire an engine. Therefore, calling functions should be able to handle
 *       failures in acquiring the resource.
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_crypto.h
 *
 * @brief ECC and AES/HASH device access API
 *
 * Copyright (C) 2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configCRYPTO_ADAPTER

#ifndef AD_CRYPTO_H_
#define AD_CRYPTO_H_

#include <osal.h>

/**
 * \addtogroup Configuration
 * \brief Configuration options for the crypto adapter.
 * \{
 */

/**
 * \brief Configure only one task using AES/HASH
 *
 * When only one task is using AES/HASH then mutual exclusion for the resource can be disabled
 * leading to improved performance and memory utilization.
 *
 */
#ifndef AD_CRYPTO_CFG_ONE_AES_HASH_USER
#define AD_CRYPTO_CFG_ONE_AES_HASH_USER (1)
#endif

/**
 * \brief Configure only one task using ECC
 *
 * When only one task is using ECC then mutual exclusion for the resource can be disabled
 * leading to improved performance and memory utilization.
 *
 * \warning ECC is used by the BLE framework so if an additional application task uses it,
 *          this macro must be set to 0.
 */
#ifndef AD_CRYPTO_CFG_ONE_ECC_USER
#define AD_CRYPTO_CFG_ONE_ECC_USER      (1)
#endif

/**
 * \brief Configure retainment of ECC RAM
 *
 * The ECC RAM is a buffer used for exchanging input and output data with the ECC engine
 * as well as for storing intermediate data during ECC operations. By configuring this
 * buffer as retained it is possible to avoid reloading some parameters after sleeping.
 * For example if only one curve is used for ECC operations, its parameters will be copied
 * only the first time, leading to improved performance.
 *
 * \note For DA14680/1-01 and mirrored/RAM mode execution, the ECC RAM buffer will be placed
 *       in RetRAM0 and hence will be retained even if this macro has value 0. This is due
 *       to the fact that in this case the RAM memory section is placed in the Cache RAM
 *       memory region. The ECC RAM block cannot reside there, so it has to be declared
 *       as retained.
 */
#ifndef AD_CRYPTO_CFG_RETAIN_ECC_MEM
#define AD_CRYPTO_CFG_RETAIN_ECC_MEM    (0)
#endif

/**
 * \brief Size of shared ECC RAM
 *
 * The requirement for the shared ECC RAM size depends on the type of ECC operations used.
 * Adjust the size by consulting the following table:
 *
 * Type of operation                                          | Shared ECC RAM (bytes)
 * ---------------------------------------------------------- | -----------------------
 * Primitive Arithmetic, Primitive ECC and Ed25519 Operations | 896
 * ECDSA Operations                                           | 1024 (default value)
 * All other Operations                                       | 1216
 */
#ifndef AD_CRYPTO_SHARED_ECC_RAM_SIZE
#define AD_CRYPTO_SHARED_ECC_RAM_SIZE   (1024)
#endif

/**
 * \}
 */

#if dg_configUSE_HW_ECC

/**
 * \brief Set the shared ECC RAM.
 *
 * The crypto adapter defines a buffer that is used as ECC shared RAM, used for exchanging
 * data with the engine and also storing intermediate data during operations. An application
 * can use its own buffer by calling this function (for example to have access to
 * ECC data without acquiring the engine). Here is an example of correct use:
 *
 * \code{.c}
 * // Copy input data for the ECC operation to task_buffer
 *
 *  while (ad_crypto_acquire_ecc(10) != OS_MUTEX_TAKEN) {
 *      ;
 *
 * ad_crypto_set_ecc_base_addr(task_buffer);
 *
 * // Use ECC engine using the low level driver API for the ECC operation
 *
 * ad_crypto_reset_ecc_base_addr(); // Do not forget!!!
 * ad_crypto_release_ecc();
 *
 * // At this point the task can access output data of the ECC operation in task_buffer
 * // even if other tasks use ECC.
 * \endcode
 *
 * \param [in] buffer The buffer to be used as ECC shared RAM. This buffer needs to be in
 *                    system RAM, be aligned to 1KByte and have a size that follows the
 *                    rules from table in ::AD_CRYPTO_SHARED_ECC_RAM_SIZE.
 *
 * \note This function allows a task to have access to its own ECC RAM block without acquiring
 *       the ECC resource (e.g. for copying input data for a future operation). It also
 *       allows a task to protect the ECC output data from other users of ECC (like the
 *       BLE framework) that are using the default shared ECC RAM defined by the adapter,
 *       after releasing the resource. However, the memory block has special size and
 *       alignment requirements that may be need to be addressed in the linker script.
 *       As a result this function should be used with care and only if absolutely necessary.
 *
 * \warning The original value of this pointer must be restored by calling
 *          ad_crypto_reset_ecc_base_addr() before releasing the resource!!!
 */
void ad_crypto_set_ecc_base_addr(uint8_t *buffer);

/**
 * \brief Get the shared ECC RAM.
 *
 * The crypto adapter defines a buffer that is used as ECC shared RAM, used for exchanging
 * data with the engine and also storing intermediate data during operations. An application
 * can use its own buffer by calling ad_crypto_set_ecc_base_addr() (for example to have access
 * to ECC data without acquiring the engine). This function returns the value of the currently
 * configured buffer.
 */
uint8_t *ad_crypto_get_ecc_base_addr(void);

/**
 * \brief Reset ECC RAM address to its default value.
 *
 * The crypto adapter defines a buffer that is used as ECC shared RAM, used for exchanging
 * data with the engine and also storing intermediate data during operations. An application
 * can use its own buffer by calling ad_crypto_set_ecc_base_addr(). This function
 * can be used to reset the buffer to its original value.
 */
void ad_crypto_reset_ecc_base_addr(void);

#endif /* dg_configUSE_HW_ECC */

#if dg_configUSE_HW_AES_HASH

/**
 * \brief Resource acquire function for AES/HASH engine
 *
 * This function is used to acquire the AES/HASH as hardware resource for the calling task. 
 * It must be called before any other API is called regarding AES/HASH, and must be followed
 * by a release of the resource. While the resource is acquired by a task the system does not
 * go to sleep.
 *
 * \param [in] timeout Maximum number of ticks to wait for the acquisition of the resource.
 *
 * \return OS_MUTEX_TAKEN if the resource was acquired, OS_MUTEX_NOT_TAKEN if the time expired 
 *         before acquiring the resource.
 *
 * \warning Nested acquires are not supported. A task that tries to acquire a resource that it
 *          has already acquired will always fail with timeout.
 *
 * \sa ad_crypto_release_aes_hash()
 */
OS_BASE_TYPE ad_crypto_acquire_aes_hash(OS_TICK_TIME timeout);

/**
 * \brief Enable AES/HASH event signaling.
 *
 * This function enables AES/HASH event signaling. After calling this function, interrupts
 * originating from the AES/HASH engine will signal an event. It must be called only after
 * acquiring the AES/HASH resource.
 *
 * \sa ad_crypto_acquire_aes_hash() ad_crypto_disable_aes_hash_event()
 *
 * \note This function enables also the AES/HASH engine clock.
 */
void ad_crypto_enable_aes_hash_event(void);

/**
 * \brief Wait for AES/HASH event to happen.
 *
 * This function waits for timeout ticks until an event is signaled relevant to the AES/HASH
 * engine. The resource must be acquired and the event signaling enabled before calling this
 * function.
 *
 * \param [in] timeout Maximum number of ticks to wait for the event.
 * \param [out] status_reg This optional output parameter will get the value of the AES/HASH engine status register
 *                         at the time that the event is signaled. If the event is not signaled after \p timeout
 *                         ticks then the output parameter value will not be changed.
 *
 * \sa ad_crypto_acquire_aes_hash(), ad_crypto_enable_aes_hash_event()
 */
OS_BASE_TYPE ad_crypto_wait_aes_hash_event(OS_TICK_TIME timeout, unsigned int *status_reg);

/**
 * \brief Disable AES/HASH event signaling.
 *
 * This function disables AES/HASH event signaling.
 *
 * \note This function disables also the AES/HASH engine clock.
 *
 * \sa ad_crypto_acquire_aes_hash() ad_crypto_enable_aes_hash_event()
 */
void ad_crypto_disable_aes_hash_event(void);

/**
 * \brief Resource release function for AES/HASH engine.
 *
 * This function is used to release the AES/HASH as hardware resource from the calling task. 
 * It must be ALWAYS be called after the resource has been acquired and as soon as its
 * no longer needed by the task.
 *
 * \return OS_OK if the resource was released, OS_NOK if the calling task is not the one
 *         that has acquired the resource.
 *
 * \sa ad_crypto_acquire_aes_hash()
 */
OS_BASE_TYPE ad_crypto_release_aes_hash(void);

#endif /* dg_configUSE_HW_AES_HASH */

#if dg_configUSE_HW_ECC
/**
 * \brief Resource acquire function for ECC engine.
 *
 * This function is used to acquire the ECC as hardware resource for the calling task. 
 * It must be called before any other API is called regarding ECC, and must be followed
 * by a release of the resource. If the ECC is successfully acquired this function will also
 * load the ECC microcode if necessary and configure the ECC RAM. While the resource is
 * acquired by a task the system does not go to sleep.
 *
 * \param [in] timeout Maximum number of ticks to wait for the acquisition of the resource.
 *
 * \return OS_MUTEX_TAKEN if the resource was acquired, OS_MUTEX_NOT_TAKEN if the time expired 
 *         before acquiring the resource.
 *
 * \warning Nested acquires are not supported. A task that tries to acquire a resource that it
 *          has already acquired will always fail with timeout.
 *
 * \sa ad_crypto_release_ecc()
 */
OS_BASE_TYPE ad_crypto_acquire_ecc(OS_TICK_TIME timeout);

/**
 * \brief Enable ECC event signaling.
 *
 * This function enables ECC event signaling. After calling this function, interrupts
 * originating from the ECC engine will signal an event. It must be called only after
 * acquiring the ECC resource.
 *
 * \note This function enables also the ECC engine clock.
 *
 * \sa ad_crypto_acquire_ecc() ad_crypto_disable_ecc_event()
 */
void ad_crypto_enable_ecc_event(void);

/**
 * \brief Wait for ECC event to happen.
 *
 * This function waits for timeout ticks until an event is signaled relevant to the ECC
 * engine. The resource must be acquired and the event signaling enabled before calling this
 * function.
 *
 * \param [in]  timeout Maximum number of ticks to wait for the event.
 * \param [out] status_reg This optional output parameter will get the value of the ECC engine status register
 *                         at the time that the event is signaled. If the event is not signaled after \p timeout
 *                         ticks then the output parameter value will not be changed.
 *
 * \sa ad_crypto_acquire_ecc(), ad_crypto_enable_ecc_event()
 */
OS_BASE_TYPE ad_crypto_wait_ecc_event(OS_TICK_TIME timeout, unsigned int *status_reg);

/**
 * \brief Disable ECC event signaling.
 *
 * This function disables ECC event signaling.
 *
 * \note This function disables also the ECC engine clock.
 *
 * \sa ad_crypto_acquire_ecc() ad_crypto_enable_ecc_event()
 */
void ad_crypto_disable_ecc_event(void);

/**
 * \brief Resource release function for ECC engine
 *
 * This function is used to release the ECC as hardware resource from the calling task. 
 * It must be ALWAYS be called after the resource has been acquired and as soon as its
 * no longer needed by the task.
 *
 * \return OS_OK if the resource was released, OS_NOK if the calling task is not the one
 *         that has acquired the resource.
 *
 * \sa ad_crypto_acquire_ecc()
 */
OS_BASE_TYPE ad_crypto_release_ecc(void);

#endif /* dg_configUSE_HW_ECC */

#endif /* AD_CRYPTO_H_ */

#endif /* dg_configCRYPTO_ADAPTER */

/**
 * \}
 * \}
 * \}
 */

