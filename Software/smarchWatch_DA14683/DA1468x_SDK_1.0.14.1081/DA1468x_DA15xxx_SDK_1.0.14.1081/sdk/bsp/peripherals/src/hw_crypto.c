/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup CRYPTO
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file hw_crypto.c
 *
 * @brief Implementation of interrupt handling for the AES/Hash and ECC Engines.
 *
 * Copyright (C) 2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if (dg_configUSE_HW_AES_HASH || dg_configUSE_HW_ECC)

#include "hw_crypto.h"

#if (dg_configSYSTEMVIEW)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif

__RETAINED static hw_crypto_cb hw_crypto_aes_hash_cb;
__RETAINED static hw_crypto_cb hw_crypto_ecc_cb;

void hw_crypto_enable_aes_hash_interrupt(hw_crypto_cb cb)
{
        /* A callback for the interrupt must be provided */
        ASSERT_ERROR(cb);
        hw_crypto_aes_hash_cb = cb;
        NVIC_EnableIRQ(CRYPTO_IRQn);
}

void hw_crypto_enable_ecc_interrupt(hw_crypto_cb cb)
{
        /* A callback for the interrupt must be provided */
        ASSERT_ERROR(cb);
        hw_crypto_ecc_cb = cb;
        NVIC_EnableIRQ(CRYPTO_IRQn);
}

void hw_crypto_disable_aes_hash_interrupt(void)
{
        hw_crypto_aes_hash_cb = NULL;
        if (!hw_crypto_ecc_cb) {
                NVIC_DisableIRQ(CRYPTO_IRQn);
        }
}

void hw_crypto_disable_ecc_interrupt(void)
{
        hw_crypto_ecc_cb = NULL;
        if (!hw_crypto_aes_hash_cb) {
                NVIC_DisableIRQ(CRYPTO_IRQn);
        }
}

void CRYPTO_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        uint32_t status = AES_HASH->CRYPTO_STATUS_REG;

        /* In case both AES/HASH and ECC have triggered an interrupt, first the AES/HASH
           will be served, and then the ISR will be called again since the ECC interrupt
           source is only cleared by reading its status register */
        if (status & AES_HASH_CRYPTO_STATUS_REG_CRYPTO_IRQ_ST_Msk) {
                /* Clear AES/HASH interrupt source */
                AES_HASH->CRYPTO_CLRIRQ_REG = 0x1;

                if (hw_crypto_aes_hash_cb != NULL) {
                        hw_crypto_aes_hash_cb(status);
                }
        } else {
                /* Clear ECC interrupt source */
                status = ECC->ECC_STATUS_REG;

                if (hw_crypto_ecc_cb != NULL) {
                        hw_crypto_ecc_cb(status);
                }
        }

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}


#endif /* (dg_configUSE_HW_AES_HASH || dg_configUSE_HW_ECC) */
/**
 * \}
 * \}
 * \}
 */

