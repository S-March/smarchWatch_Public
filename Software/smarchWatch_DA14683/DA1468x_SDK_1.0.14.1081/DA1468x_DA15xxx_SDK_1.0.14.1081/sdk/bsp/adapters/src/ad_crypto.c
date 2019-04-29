/**
 * \addtogroup BSP
 * \{
 * \addtogroup ADAPTERS
 * \{
 * \addtogroup CRYPTO
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_crypto.c
 *
 * @brief ECC and AES/HASH device access API implementation
 *
 * Copyright (C) 2016-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configCRYPTO_ADAPTER

#if dg_configUSE_HW_AES_HASH
#include "hw_aes_hash.h"
#endif
#if dg_configUSE_HW_ECC
#include "hw_ecc_ucode.h"
#include "hw_ecc.h"
#endif
#include "hw_crypto.h"
#include "ad_crypto.h"
#include <sys_power_mgr.h>
#include <osal.h>

#define AD_CRYPTO_ECC_CFG_UCODE_LOADED          0x1UL
#define AD_CRYPTO_ECC_CFG_BASE_ADDR_SET         0x2UL
#define AD_CRYPTO_AES_HASH_EVENT_EN             0x4UL
#define AD_CRYPTO_ECC_EVENT_EN                  0x8UL

#if dg_configUSE_HW_AES_HASH

#if (AD_CRYPTO_CFG_ONE_AES_HASH_USER == 0)
__RETAINED static OS_MUTEX aes_hash_lock;
#define _AD_CRYPTO_INIT_AES_HASH_LOCK(_s)       OS_MUTEX_CREATE(_s)
#define _AD_CRYPTO_ACQ_AES_HASH(_t)             OS_MUTEX_GET(aes_hash_lock, _t)
#define _AD_CRYPTO_REL_AES_HASH()               OS_MUTEX_PUT(aes_hash_lock)
#else /* (AD_CRYPTO_CFG_ONE_AES_HASH_USER == 1) */
#define _AD_CRYPTO_INIT_AES_HASH_LOCK(_s)       OS_MUTEX_CREATE_SUCCESS
#define _AD_CRYPTO_ACQ_AES_HASH(_t)             OS_MUTEX_TAKEN
#define _AD_CRYPTO_REL_AES_HASH()               OS_OK
#endif /* (AD_CRYPTO_CFG_ONE_AES_HASH_USER == 0) */

#endif /* dg_configUSE_HW_AES_HASH */

#if dg_configUSE_HW_ECC

#if (AD_CRYPTO_CFG_ONE_ECC_USER == 0)
__RETAINED static OS_MUTEX ecc_lock;
#define _AD_CRYPTO_INIT_ECC_LOCK(s)             OS_MUTEX_CREATE(s)
#define _AD_CRYPTO_ACQ_ECC(t)                   OS_MUTEX_GET(ecc_lock, t)
#define _AD_CRYPTO_REL_ECC()                    OS_MUTEX_PUT(ecc_lock)
#else /* (AD_CRYPTO_CFG_ONE_ECC_USER == 1) */
#define _AD_CRYPTO_INIT_ECC_LOCK(s)             OS_MUTEX_CREATE_SUCCESS
#define _AD_CRYPTO_ACQ_ECC(t)                   OS_MUTEX_TAKEN
#define _AD_CRYPTO_REL_ECC()                    OS_OK
#endif /* (AD_CRYPTO_CFG_ONE_ECC_USER == 0) */


/*
 * The shared ECC RAM (AD_CRYPTO_SHARED_ECC_RAM_SIZE bytes) must be aligned to 1 KByte and
 * within [0x7fc0000, 0x7fdfc00]
 */
#if (AD_CRYPTO_CFG_RETAIN_ECC_MEM == 1)
#define _AD_CRYPTO_ECC_RAM_ATTR         __RETAINED
#else /* (AD_CRYPTO_CFG_RETAIN_ECC_MEM == 0) */
#if (dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A) && (dg_configEXEC_MODE != MODE_IS_CACHED)
/*
 * In DA14680/1-01 and in mirrored execution mode, the CODE is placed at the
 * beginning, while the RetRAM0 must be placed, at least, in the RAM5 block.
 * Since the Cache is used as an extension of RAM in this mode, it is a good
 * approach to place the RAM memory section of the linker at this region. If
 * its size is enough to hold the applications data, space is freed for the
 * CODE and the RetRAM0 with this approach. The problem is that the ECC
 * buffer cannot be higher than the end of the RAM5 block due to a HW
 * limitation. A possible solution is to place it in the RetRAM0 in this case.
 * Other solutions exist (like i.e. defining 2 RAM sections, RAM1 and RAM2,
 * set the ordering to CODE, RAM1, RetRAM0 and then RAM2 (located at the Cache
 * block) and, then, place the ECC buffer explicitly in RAM1).
 */
#pragma message "Shared ECC RAM buffer will be placed in RetRAM0."
#define _AD_CRYPTO_ECC_RAM_ATTR         __RETAINED
#else
#define _AD_CRYPTO_ECC_RAM_ATTR         __attribute__((section(".bss.ecc_buffer")))
#endif
#endif /* AD_CRYPTO_CFG_RETAIN_ECC_MEM */

_AD_CRYPTO_ECC_RAM_ATTR static volatile uint32_t ad_crypto_shared_ecc_mem[AD_CRYPTO_SHARED_ECC_RAM_SIZE/4] __attribute__((aligned(1024)));
__RETAINED static uint8_t *ad_crypto_ecc_base_addr;

#endif /* dg_configUSE_HW_ECC */

__RETAINED static unsigned int ad_crypto_status;

#if dg_configUSE_HW_AES_HASH
__RETAINED static OS_EVENT aes_hash_event;
#endif
#if dg_configUSE_HW_ECC
__RETAINED static OS_EVENT ecc_event;
#endif

#if dg_configUSE_HW_AES_HASH
static unsigned int ad_crypto_last_aes_hash_status_reg;

static void aes_hash_irq_cb(unsigned int status)
{
        ad_crypto_last_aes_hash_status_reg = status;
        OS_EVENT_SIGNAL_FROM_ISR(aes_hash_event);
}
#endif

#if dg_configUSE_HW_ECC

static unsigned int ad_crypto_last_ecc_status_reg;

static void ecc_irq_cb(unsigned int status)
{
        ad_crypto_last_ecc_status_reg = status;
        OS_EVENT_SIGNAL_FROM_ISR(ecc_event);
}

void ad_crypto_set_ecc_base_addr(uint8_t *buffer)
{
        unsigned int remap_type = REG_GETF(CRG_TOP, SYS_CTRL_REG, REMAP_ADR0);

        if (IS_SYSRAM_ADDRESS((void *)buffer) ||
           (IS_REMAPPED_ADDRESS((void *)buffer) && (remap_type == 0x3))) {
                if ((uintptr_t )buffer & 0x3FF) {
                        /* Buffer needs to be aligned to 1KByte */
                        OS_ASSERT(0);
                } else {
                        ad_crypto_ecc_base_addr = buffer;
                        ad_crypto_status &= ~AD_CRYPTO_ECC_CFG_BASE_ADDR_SET;
                }
        } else {
                /* Buffer address can only reside in RAM */
                OS_ASSERT(0);
        }
}

uint8_t *ad_crypto_get_ecc_base_addr(void)
{
        return ad_crypto_ecc_base_addr;
}

void ad_crypto_reset_ecc_base_addr(void)
{
        ad_crypto_ecc_base_addr = (uint8_t *)ad_crypto_shared_ecc_mem;
        ad_crypto_status &= ~AD_CRYPTO_ECC_CFG_BASE_ADDR_SET;
}

static void ad_crypto_check_ecc_cfg(void)
{
        if (!(ad_crypto_status & AD_CRYPTO_ECC_CFG_UCODE_LOADED)) {
                hw_ecc_enable_clock();
                hw_ecc_load_ucode(hw_ecc_ucode1, HW_ECC_UCODE1_SIZE);
                hw_ecc_disable_clock();
                ad_crypto_status |= AD_CRYPTO_ECC_CFG_UCODE_LOADED;
        }

        if (!(ad_crypto_status & AD_CRYPTO_ECC_CFG_BASE_ADDR_SET)) {
                hw_ecc_set_base_addr((void *)ad_crypto_ecc_base_addr);
                ad_crypto_status |= AD_CRYPTO_ECC_CFG_BASE_ADDR_SET;
        }
}

#endif /* dg_configUSE_HW_ECC */

#if dg_configUSE_HW_AES_HASH

static void ad_crypto_clear_aes_hash_event(void)
{
        OS_EVENT_CHECK(aes_hash_event);
}

OS_BASE_TYPE ad_crypto_acquire_aes_hash(OS_TICK_TIME timeout)
{
        OS_BASE_TYPE acquisition_result = _AD_CRYPTO_ACQ_AES_HASH(timeout);

        if (acquisition_result == OS_MUTEX_TAKEN) {
                /* The resource is expected to be disabled just after acquiring it. This is
                   a possible sign of some task using the resource without acquiring it, or
                   releasing it without disabling it. */
                OS_ASSERT(!hw_aes_hash_clock_is_enabled());

                pm_stay_alive();
        }

        return acquisition_result;
}

void ad_crypto_enable_aes_hash_event(void)
{
        hw_aes_hash_enable_clock();
        ad_crypto_clear_aes_hash_event();
        hw_aes_hash_clear_interrupt_req();
        hw_aes_hash_enable_interrupt_source();
        hw_crypto_enable_aes_hash_interrupt(aes_hash_irq_cb);
        ad_crypto_status |= AD_CRYPTO_AES_HASH_EVENT_EN;
}

OS_BASE_TYPE ad_crypto_wait_aes_hash_event(OS_TICK_TIME timeout, unsigned int *status_reg)
{
        OS_BASE_TYPE wait_result = OS_EVENT_WAIT(aes_hash_event, timeout);

        if (status_reg && (wait_result == OS_EVENT_SIGNALED)) {
                *status_reg = ad_crypto_last_aes_hash_status_reg;
        }

        return wait_result;
}

void ad_crypto_disable_aes_hash_event(void)
{
        hw_aes_hash_disable_interrupt_source();
        hw_aes_hash_clear_interrupt_req();
        hw_crypto_disable_aes_hash_interrupt();
        hw_crypto_clear_pending_interrupt();
        hw_aes_hash_disable_clock();
        ad_crypto_clear_aes_hash_event();
        ad_crypto_status &= ~AD_CRYPTO_AES_HASH_EVENT_EN;
}

OS_BASE_TYPE ad_crypto_release_aes_hash(void)
{
        OS_BASE_TYPE release_result;

        /* Event signaling must be disabled before releasing the resource */
        ASSERT_WARNING(!(ad_crypto_status & AD_CRYPTO_AES_HASH_EVENT_EN));

        release_result = _AD_CRYPTO_REL_AES_HASH();

        if (release_result == OS_OK) {
                pm_resume_sleep();
        }

        return release_result;
}

#endif /* dg_configUSE_HW_AES_HASH */

#if dg_configUSE_HW_ECC

static void ad_crypto_clear_ecc_event(void)
{
        OS_EVENT_CHECK(ecc_event);
}

OS_BASE_TYPE ad_crypto_acquire_ecc(OS_TICK_TIME timeout)
{
        OS_BASE_TYPE acquisition_result = _AD_CRYPTO_ACQ_ECC(timeout);

        if (acquisition_result == OS_MUTEX_TAKEN) {
                /* The resource is expected to be disabled just after acquiring it. This is
                   a possible sign of some task using the resource without acquiring it, or
                   releasing it without disabling it. */
                OS_ASSERT(!hw_ecc_clock_is_enabled());

                pm_stay_alive();
                ad_crypto_check_ecc_cfg();
        }

        return acquisition_result;
}

void ad_crypto_enable_ecc_event(void)
{
        ad_crypto_clear_ecc_event();
        hw_ecc_enable_clock();
        hw_ecc_clear_interrupt_source();
        hw_crypto_enable_ecc_interrupt(ecc_irq_cb);
        ad_crypto_status |= AD_CRYPTO_ECC_EVENT_EN;
}

OS_BASE_TYPE ad_crypto_wait_ecc_event(OS_TICK_TIME timeout, unsigned int *status_reg)
{
        OS_BASE_TYPE wait_result = OS_EVENT_WAIT(ecc_event, timeout);

        if (status_reg && (wait_result == OS_EVENT_SIGNALED)) {
                *status_reg = ad_crypto_last_ecc_status_reg;
        }

        return wait_result;
}

void ad_crypto_disable_ecc_event(void)
{
        hw_crypto_disable_ecc_interrupt();
        hw_ecc_clear_interrupt_source();
        hw_crypto_clear_pending_interrupt();
        hw_ecc_disable_clock();
        ad_crypto_clear_ecc_event();
        ad_crypto_status &= ~AD_CRYPTO_ECC_EVENT_EN;
}

OS_BASE_TYPE ad_crypto_release_ecc(void)
{
        OS_BASE_TYPE release_result;

        /* Event signaling must be disabled before releasing the resource */
        ASSERT_WARNING(!(ad_crypto_status & AD_CRYPTO_ECC_EVENT_EN));

        release_result = _AD_CRYPTO_REL_ECC();

        if (release_result == OS_OK) {
                pm_resume_sleep();
        }

        return release_result;
}

static void ad_crypto_wake_up_ind(bool arg)
{
        if (dg_configECC_UCODE_RAM_RETAINED == 0) {
                ad_crypto_status &= ~AD_CRYPTO_ECC_CFG_UCODE_LOADED;
        }

        ad_crypto_status &= ~AD_CRYPTO_ECC_CFG_BASE_ADDR_SET;
}

#endif /* dg_configUSE_HW_ECC */

const adapter_call_backs_t ad_crypto_pm_cbs = {
        .ad_prepare_for_sleep = NULL,
        .ad_sleep_canceled = NULL,
#if dg_configUSE_HW_ECC
        .ad_wake_up_ind = ad_crypto_wake_up_ind,
#else
        .ad_wake_up_ind = NULL,
#endif
        .ad_xtal16m_ready_ind = NULL,
        .ad_sleep_preparation_time = 0
};

void ad_crypto_init(void *params)
{
        OS_BASE_TYPE status __attribute__((unused));
        ad_crypto_status = 0;


#if dg_configUSE_HW_AES_HASH
        status = _AD_CRYPTO_INIT_AES_HASH_LOCK(aes_hash_lock);
        OS_ASSERT(status == OS_MUTEX_CREATE_SUCCESS);
        OS_EVENT_CREATE(aes_hash_event);
        OS_ASSERT(aes_hash_event);
#endif

#if dg_configUSE_HW_ECC
        ad_crypto_reset_ecc_base_addr();
        status = _AD_CRYPTO_INIT_ECC_LOCK(ecc_lock);
        OS_ASSERT(status == OS_MUTEX_CREATE_SUCCESS);

        OS_EVENT_CREATE(ecc_event);
        OS_ASSERT(ecc_event);
#endif

        pm_register_adapter(&ad_crypto_pm_cbs);
}

ADAPTER_INIT(crypto_adapter, ad_crypto_init);

#endif /* dg_configCRYPTO_ADAPTER */

/**
 * \}
 * \}
 * \}
 */

