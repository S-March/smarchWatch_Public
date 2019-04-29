/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup ECC
 * \{
 * \brief ECC Engine
 */

/**
 ****************************************************************************************
 *
 * @file hw_ecc.c
 *
 * @brief Implementation of the ECC Engine Low Level Driver.
 *
 * Copyright (C) 2016-2018 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configUSE_HW_ECC

#include "hw_ecc.h"

void hw_ecc_set_base_addr(volatile void *base_addr)
{
        /* The address needs to be in SysRAM */
        ASSERT_ERROR(IS_SYSRAM_ADDRESS(base_addr));

        GPREG->ECC_BASE_ADDR_REG = ((uint32_t)base_addr - MEMORY_SYSRAM_BASE) >> 10;
}

static void hw_ecc_cpy_to_ecc_8(volatile uint8_t *dst, const uint8_t *src, size_t nbytes)
{
        unsigned int i;

        for (i = 0; i < nbytes; i++) {
                dst[i] = src[i];
        }
}

static void hw_ecc_cpy_from_ecc_8(uint8_t *dst, volatile uint8_t *src, size_t nbytes)
{
        unsigned int i;

        for (i = 0; i < nbytes; i++) {
                dst[i] = src[i];
        }
}

static void hw_ecc_cpy_to_ecc_r_8(volatile uint8_t *dst, const uint8_t *src, size_t nbytes)
{
        unsigned int i;

        for (i = 0; i < nbytes; i++) {
                dst[nbytes - 1 - i] = src[i];
        }
}

static void hw_ecc_cpy_from_ecc_r_8(uint8_t *dst, volatile uint8_t *src, size_t nbytes)
{
        unsigned int i;

        for (i = 0; i < nbytes; i++) {
                dst[nbytes - 1 - i] = src[i];
        }
}

static void hw_ecc_cpy_to_ecc_32(volatile uint32_t *dst, const uint32_t *src, size_t nbytes)
{
        unsigned int i;

        for (i = 0; i < nbytes/4; i++) {
                dst[i] = src[i];
        }
}

static void hw_ecc_cpy_from_ecc_32(uint32_t *dst, volatile uint32_t *src, size_t nbytes)
{
        unsigned int i;

        for (i = 0; i < nbytes/4; i++) {
                dst[i] = src[i];
        }
}

static void hw_ecc_cpy_to_ecc_r_32(volatile uint32_t *dst, const uint32_t *src, size_t nbytes)
{
        unsigned int i;

        for (i = 0; i < nbytes/4; i++) {
                dst[i] = SWAP32(src[nbytes/4 - 1 - i]);
        }
}

static void hw_ecc_cpy_from_ecc_r_32(uint32_t *dst, volatile uint32_t *src, size_t nbytes)
{
        unsigned int i;

        for (i = 0; i < nbytes/4; i++) {
                dst[i] = SWAP32(src[nbytes/4 - 1 - i]);
        }
}

void hw_ecc_write256(unsigned int location, const uint8_t *data, volatile uint8_t *base_addr)
{
        /* Only 16 256-bit locations are available in the Data RAM used by ECC */
        ASSERT_WARNING(location < 16);

        if ((unsigned long)data & 0x03UL) {
                hw_ecc_cpy_to_ecc_8(base_addr + (32 * location), data, 32);
        } else {
                hw_ecc_cpy_to_ecc_32((volatile uint32_t *)(base_addr + (32 * location)), (uint32_t *)data, 32);
        }
}

void hw_ecc_write256_r(unsigned int location, const uint8_t *data, volatile uint8_t *base_addr)
{
        /* Only 16 256-bit locations are available in the Data RAM used by ECC */
        ASSERT_WARNING(location < 16);

        if ((unsigned long)data & 0x03UL) {
                hw_ecc_cpy_to_ecc_r_8(base_addr + (32 * location), data, 32);
        } else {
                hw_ecc_cpy_to_ecc_r_32((volatile uint32_t *)(base_addr + (32 * location)), (uint32_t *)data, 32);
        }
}

void hw_ecc_read256(unsigned int location, uint8_t *data, volatile uint8_t *base_addr)
{
        /* Only 16 256-bit locations are available in the Data RAM used by ECC */
        ASSERT_WARNING(location < 16);

        if ((unsigned long)data & 0x03UL) {
                hw_ecc_cpy_from_ecc_8(data, base_addr + (32 * location), 32);
        } else {
                hw_ecc_cpy_from_ecc_32((uint32_t *)data, (volatile uint32_t *)(base_addr + (32 * location)), 32);
        }
}

void hw_ecc_read256_r(unsigned int location, uint8_t *data, volatile uint8_t *base_addr)
{
        /* Only 16 256-bit locations are available in the Data RAM used by ECC */
        ASSERT_WARNING(location < 16);

        if ((unsigned long)data & 0x03UL) {
                hw_ecc_cpy_from_ecc_r_8(data, base_addr + (32 * location), 32);
        } else {
                hw_ecc_cpy_from_ecc_r_32((uint32_t *)data, (volatile uint32_t *)(base_addr + (32 * location)), 32);
        }
}

void hw_ecc_write128(unsigned int location, const uint8_t *data, volatile uint8_t *base_addr)
{
        volatile uint32_t *p = (volatile uint32_t *)(base_addr + (32 * location) + 16);

        /* Only 16 256-bit locations are available in the Data RAM used by ECC */
        ASSERT_WARNING(location < 16);

        if ((unsigned long)data & 0x03UL) {
                hw_ecc_cpy_to_ecc_8(base_addr + (32 * location), data, 16);
        } else {
                hw_ecc_cpy_to_ecc_32((volatile uint32_t *)(base_addr + (32 * location)), (uint32_t *)data, 16);
        }

        *(p++) = 0;
        *(p++) = 0;
        *(p++) = 0;
        *p = 0;
}

void hw_ecc_write128_r(unsigned int location, const uint8_t *data, volatile uint8_t *base_addr)
{
        volatile uint32_t *p = (volatile uint32_t *)(base_addr + (32 * location) + 16);

        /* Only 16 256-bit locations are available in the Data RAM used by ECC */
        ASSERT_WARNING(location < 16);

        if ((unsigned long)data & 0x03UL) {
                hw_ecc_cpy_to_ecc_r_8(base_addr + (32 * location), data, 16);
        } else {
                hw_ecc_cpy_to_ecc_r_32((volatile uint32_t *)(base_addr + (32 * location)), (uint32_t *)data, 16);
        }

        *(p++) = 0;
        *(p++) = 0;
        *(p++) = 0;
        *p = 0;
}

void hw_ecc_read128(unsigned int location, uint8_t *data, volatile uint8_t *base_addr)
{
        /* Only 16 256-bit locations are available in the Data RAM used by ECC */
        ASSERT_WARNING(location < 16);

        if ((unsigned long)data & 0x03UL) {
                hw_ecc_cpy_from_ecc_8(data, base_addr + (32 * location), 16);
        } else {
                hw_ecc_cpy_from_ecc_32((uint32_t *)data, (volatile uint32_t *)(base_addr + (32 * location)), 16);
        }
}

void hw_ecc_read128_r(unsigned int location, uint8_t *data, volatile uint8_t *base_addr)
{
        /* Only 16 256-bit locations are available in the Data RAM used by ECC */
        ASSERT_WARNING(location < 16);

        if ((unsigned long)data & 0x03UL) {
                hw_ecc_cpy_from_ecc_r_8(data, base_addr + (32 * location), 16);
        } else {
                hw_ecc_cpy_from_ecc_r_32((uint32_t *)data, (volatile uint32_t *)(base_addr + (32 * location)), 16);
        }
}

#endif /* dg_configUSE_HW_ECC */
/**
 * \}
 * \}
 * \}
 */
