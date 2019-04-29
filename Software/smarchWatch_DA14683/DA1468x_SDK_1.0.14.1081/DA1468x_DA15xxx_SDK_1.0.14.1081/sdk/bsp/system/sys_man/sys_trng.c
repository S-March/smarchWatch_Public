/**
\addtogroup BSP
\{
\addtogroup SYSTEM
\{
\addtogroup SYS_TRNG
\{
*/

/**
 ****************************************************************************************
 *
 * @file sys_trng.c
 *
 * @brief System true random number generation
 *
 * Copyright (C) 2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include "hw_trng.h"
#include "osal.h"
#include <string.h>
#include <stdlib.h>

#define SYS_TRNG_SIZE_OF_BUFFER         32
#define SYS_TRNG_GENERATOR_STARTED      0x1UL

__RETAINED static uint32_t sys_trng_numbers[SYS_TRNG_SIZE_OF_BUFFER];
__RETAINED_RW static unsigned int sys_trng_next_number_index = 32;
__RETAINED static unsigned int sys_trng_status;

static void sys_trng_cb(void)
{
        hw_trng_get_numbers(sys_trng_numbers, 32);
        sys_trng_next_number_index = 0;
        hw_trng_stop();
        hw_trng_disable_clk();
        hw_trng_disable_interrupt();
        sys_trng_status = 0;
}

__RETAINED_CODE static void sys_trng_start_generator(void)
{
        hw_trng_enable(sys_trng_cb);
        sys_trng_status |= SYS_TRNG_GENERATOR_STARTED;
}

__RETAINED_CODE static uint32_t _sys_trng_random(void)
{
#if dg_configUSE_HW_TRNG == 1
        uint32_t rand_num;
        unsigned int rand_number_obtained = 0;

        do {
                GLOBAL_INT_DISABLE();
                if (sys_trng_next_number_index < SYS_TRNG_SIZE_OF_BUFFER) {
                        rand_num = sys_trng_numbers[sys_trng_next_number_index++];
                        rand_number_obtained = 1;
                        /* If this was the last number, start TRNG */
                        if (sys_trng_next_number_index == SYS_TRNG_SIZE_OF_BUFFER) {
                                sys_trng_start_generator();
                        }
                } else if (hw_trng_get_fifo_level() > 0) {
                        rand_num = hw_trng_get_number();
                        rand_number_obtained = 1;
                } else if (!(sys_trng_status & SYS_TRNG_GENERATOR_STARTED)) {
                        sys_trng_start_generator();
                }
                GLOBAL_INT_RESTORE();
        } while (!rand_number_obtained);

        return rand_num;
#else
        #error HW_TRNG must be configured as enabled
#endif
}

int sys_trng_producing_numbers(void)
{
        return (sys_trng_status & SYS_TRNG_GENERATOR_STARTED);
}

__RETAINED_CODE int sys_trng_rand(void)
{
#if dg_configUSE_HW_TRNG == 1
        return (int)(RAND_MAX & _sys_trng_random());
#else
        #error HW_TRNG must be configured as enabled
#endif
}

void sys_trng_get_bytes(uint8_t *buffer, size_t size)
{
#if dg_configUSE_HW_TRNG == 1
        uint32_t rand_num;
        uint8_t *buf_p = buffer;

        while (size) {
                rand_num = _sys_trng_random();
                if (size < 4) {
                        memcpy(buf_p, &rand_num, size);
                        size = 0;
                } else {
                        memcpy(buf_p, &rand_num, 4);
                        size -= 4;
                        buf_p += 4;
                }
        }
#else
        #error HW_TRNG must be configured as enabled
#endif
}

void sys_trng_get_numbers(uint32_t *buffer, size_t size)
{
#if dg_configUSE_HW_TRNG == 1
        size_t i = 0;

        for (i = 0; i < size; i++) {
                buffer[i] = _sys_trng_random();
        }
#else
        #error HW_TRNG must be configured as enabled
#endif
}

uint32_t sys_trng_rand_range(uint32_t range_min, uint32_t range_max)
{
#if dg_configUSE_HW_TRNG == 1
        uint32_t rand_num;

        do {
                rand_num = _sys_trng_random();
        } while (rand_num < range_min || rand_num > range_max);

        return rand_num;
#else
        #error HW_TRNG must be configured as enabled
#endif
}

void sys_trng_get_numbers_range(uint32_t *buffer, size_t size, uint32_t range_min, uint32_t range_max)
{
#if dg_configUSE_HW_TRNG == 1
        size_t i = 0;

        for (i = 0; i < size; i++) {
                buffer[i] = sys_trng_rand_range(range_min, range_max);
        }
#else
        #error HW_TRNG must be configured as enabled
#endif
}

/**
\}
\}
\}
*/

