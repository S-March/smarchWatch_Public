/**
 ****************************************************************************************
 *
 * @file hw_trng.h
 *
 * @brief Definition of API for the True Random Number Generator Low Level Driver.
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_TRNG_H_
#define HW_TRNG_H_

#if dg_configUSE_HW_TRNG

#include <sdk_defs.h>

/**
 * \brief TRNG callback.
 *
 * This function is called by the TRNG driver when the interrupt is fired.
 *
 * \note If the TRNG is not needed anymore the hw_trng_disable function should be called in the
 * callback function to save power.
 *
 */
typedef void (*hw_trng_cb)(void);

/**
 * \brief TRNG enable.
 *
 * This function enables the TRNG. If callback is not NULL it will be called when a TRNG interrupt
 * occurs. The interrupt is triggered when the TRNG FIFO is full.
 *
 * \param [in] callback The callback function that is called when a TRNG interrupt occurs.
 *
 * \note If the amount of random numbers needed is less than the contents of the FIFO it is faster
 * and more power efficient to use a wait loop in combination with the hw_trng_get_fifo_level
 * function. If the FIFO has the required level use the hw_trng_get_numbers function to get the
 * random numbers.
 *
 */
void hw_trng_enable(hw_trng_cb callback);

/**
 * \brief Get a random number from TRNG.
 *
 * This function reads a random number from the TRNG FIFO.
 *
 * \return A 32-bit unsigned random number.
 *
 * \warning This function does not check for number availability in the FIFO
 *
 */
__attribute__((always_inline)) static inline uint32_t hw_trng_get_number(void)
{
        return *((volatile uint32_t *)MEMORY_TRNG_FIFO);
}

/**
 * \brief Get random numbers from TRNG.
 *
 * This function fills a buffer with random numbers read from the TRNG FIFO.
 *
 * \param [in] buffer The buffer to write the numbers to.
 * \param [in] size The size of the buffer (max 32).
 *
 * \note Do not forget to disable the TRNG after reading the amount of numbers needed to save
 * power.
 *
 * \warning This function does not check for number availability in the FIFO
 *
 */
void hw_trng_get_numbers(uint32_t* buffer, uint8_t size);

/**
 * \brief TRNG get FIFO level.
 *
 * This function returns the current level of the TRNG FIFO.
 *
 * \return The current level of the TRNG FIFO.
 *
 */
__RETAINED_CODE uint8_t hw_trng_get_fifo_level(void);

/**
 * \brief TRNG disable.
 *
 * This function stops TRNG, disables its clock and its interrupt.
 *
 */
void hw_trng_disable(void);

/**
 * \brief Stop TRNG operation.
 */
static inline void hw_trng_stop(void)
{
        REG_CLR_BIT(TRNG, TRNG_CTRL_REG, TRNG_ENABLE);
}

/**
 * \brief Disable TRNG clock.
 */
void hw_trng_disable_clk(void);

/**
 * \brief Disable TRNG interrupt.
 */
void hw_trng_disable_interrupt(void);

/**
 * \brief Clear TRNG pending interrupt.
 */
void hw_trng_clear_pending(void);

#endif /* dg_configUSE_HW_TRNG */

#endif /* HW_TRNG_H_ */

