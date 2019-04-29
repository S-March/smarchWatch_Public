/**
 \addtogroup INTERFACES
 \{
 \addtogroup AUDIO
 \{
 \brief Audio
 \addtogroup PDM
 \{
 */

/**
 ****************************************************************************************
 *
 * @file if_pdm.h
 *
 * @brief PDM audio interface driver.
 *
 * Copyright (C) 2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef IF_PDM_H_
#define IF_PDM_H_

#if dg_configUSE_IF_PDM

#include "sdk_defs.h"
#include "hw_gpio.h"
#include <stdbool.h>

/**
 \brief PDM interface mode.
 */
typedef enum {
        /** PDM interface in slave mode, i.e. clocked externally. */
        IF_PDM_MODE_SLAVE = 0,
        /** PDM interface in master mode, i.e. it provides the clock signal. */
        IF_PDM_MODE_MASTER = 1,
} IF_PDM_MODE;

/**
 \brief PDM signal direction.
 */
typedef enum {
        IF_PDM_DIRECTION_IN = 0, ///< PDM interface in receive mode.
        IF_PDM_DIRECTION_OUT_RIGHT = 1, ///< PDM interface transmits on the right channel.
        IF_PDM_DIRECTION_OUT_LEFT = 2, ///< PDM interface transmits on the left channel.
        IF_PDM_DIRECTION_OUT_STEREO = 3, ///< PDM interface transmits right and left channels.
} IF_PDM_DIRECTION;

/**
 \brief PDM output direction.
 \note \ref IF_PDM_SRC_DIRECTION_PCM is not currently supported.
 */
typedef enum {
        IF_PDM_SRC_DIRECTION_PCM, ///< The PDM input is directed to the PCM output registers.
        IF_PDM_SRC_DIRECTION_REG, ///< The PDM input is directed to the SRC output registers.
} IF_PDM_SRC_DIRECTION;

/**
 \brief PDM interface GPIO structure containing the GPIO port and the GPIO pin.
 */
typedef struct {
        HW_GPIO_PORT port; ///< GPIO port.
        HW_GPIO_PIN pin; ///< GPIO pin.
} if_pdm_gpio;

/**
 \brief PDM interrupt data structure. Used to return PDM data through the PDM callback function.
 Depending on the Direction of the PDM, the SRC in or out interrupt handler will call
 the callback function.
 */
typedef struct {
        /** Value of SRC1_OUT1_REG. 0 if PDM direction is not \ref IF_PDM_DIRECTION_IN. */
        uint32_t src_out1_value;

        /** Value of SRC1_OUT2_REG. 0 if PDM direction is not \ref IF_PDM_DIRECTION_IN. */
        uint32_t src_out2_value;

        /** Pointer to SRC1_IN1_REG. NULL if PDM direction is \ref IF_PDM_DIRECTION_IN. */
        volatile uint32_t *src_in1_reg;

        /** Pointer to SRC1_IN2_REG. NULL if PDM direction is \ref IF_PDM_DIRECTION_IN. */
        volatile uint32_t *src_in2_reg;
} if_pdm_src_isr_data;

/**
 \brief PDM interface SRC callback
 This function is called by the PDM interface whenever an SRC interrupt is fired.

 \param [in] src_isr_data PDM ISR data.
 */
typedef void (*if_pdm_src_interrupt_cb)(if_pdm_src_isr_data *src_isr_data);

/**
 * \brief PDM interface configuration.
 */
typedef struct {
        /** PDM clock GPIO pin. */
        if_pdm_gpio clk_gpio;

        /** PDM data GPIO pin. */
        if_pdm_gpio data_gpio;

        /** The mode of the PDM interface. Master or slave. */
        IF_PDM_MODE mode;

        /** The direction of the PDM interface. */
        IF_PDM_DIRECTION direction;

        /**
         * The direction of the sample rate converter. For example, when direction is
         * \ref IF_PDM_DIRECTION_IN and \ref src_direction is \ref IF_PDM_SRC_DIRECTION_PCM the PDM
         * signal will be passed to the PCM interface. Or when the PDM direction is out and \ref
         * src_direction is \ref IF_PDM_SRC_DIRECTION_REG the SRC I/O registers are used for PDM
         * input.
         */
        IF_PDM_SRC_DIRECTION src_direction;

        /**
         * The sample rate of the sample rate converter (Hz). This is only used when \ref src_direction
         * is \ref IF_PDM_SRC_DIRECTION_REG.
         */
        unsigned int src_sample_rate;

        /** This is used to bypass the sample rate converter out upsampling filters. */
        bool bypass_out_filter;

        /** This is used to bypass the sample rate converter in upsampling filters. */
        bool bypass_in_filter;

        /** This is used to enable the dithering feature of the sample rate converter. */
        bool enable_dithering;

        /**
         * This is used to set the PDM_DIV field of the PDM_DIV_REG. If this equals 0 the PDM_DIV
         * is set to 8.
         */
        uint8_t pdm_div;

        /** Enable the SRC interrupt for PDM samples. */
        bool enable_interrupt;

        /** The priority of the SRC interrupt. */
        uint8_t interrupt_priority;

        /** Callback function that is called when the SRC interrupt for PDM samples is fired. */
        if_pdm_src_interrupt_cb callback;
} if_pdm_config;

/**
 \brief Initializes and enables the PDM interface.

 \param[in] config PDM interface configuration.
 */
void if_pdm_enable(const if_pdm_config *config);

/**
 \brief Disables the PDM interface.
 */
void if_pdm_disable(void);

#endif /* dg_configUSE_IF_PDM */

#endif /* IF_PDM_H_ */

/**
 \}
 \}
 \}
 */
