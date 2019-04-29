/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup IR_Generator
 * \{
 * \brief IR Generator
 */

/**
 ****************************************************************************************
 *
 * @file hw_irgen.h
 *
 * @brief Definition of API for the IR generator Low Level Driver.
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_IRGEN_H_
#define HW_IRGEN_H_

#if dg_configUSE_HW_IRGEN

#include <stdbool.h>
#include <stdint.h>
#include <sdk_defs.h>

/**
 * \brief IR logic bit format
 *
 */
typedef enum {
        HW_IRGEN_LOGIC_FORMAT_MARK = 0, /**< logic bit starts with mark followed by space */
        HW_IRGEN_LOGIC_FORMAT_SPACE = 1 /**< logic bit starts with space followed by mark */
} HW_IRGEN_LOGIC_FORMAT;

/**
 * \brief IR FIFO
 *
 */
typedef enum {
        HW_IRGEN_FIFO_CODE = 0,         /**< code FIFO */
        HW_IRGEN_FIFO_REPEAT = 1        /**< repeat FIFO */
} HW_IRGEN_FIFO;

/**
 * \brief Paint symbol
 *
 */
typedef enum {
	HW_IRGEN_PAINT_SPACE = 0,       /**< space */
	HW_IRGEN_PAINT_MARK = 1         /**< mark */
} HW_IRGEN_PAINT;

/**
 * \brief IR generator output mode
 *
 */
typedef enum {
        HW_IRGEN_OUTPUT_NORMAL = 0,     /**< output is not inverted */
        HW_IRGEN_OUTPUT_INVERTED = 1    /**< output is inverted */
} HW_IRGEN_OUTPUT;

/**
 * \brief IR generator configuration
 *
 * Parameters have the same function as in corresponding setter functions.
 *
 */
typedef struct {
        uint16_t        carrier_hi;     /**< carrier high duration */
        uint16_t        carrier_lo;     /**< carrier low duration */

        struct {
                HW_IRGEN_LOGIC_FORMAT   format;         /**< format */
                uint8_t                 mark_time;      /**< mark duration */
                uint8_t                 space_time;     /**< space duration */
        } logic0;                       /**< digital message logic "0" settings */

        struct {
                HW_IRGEN_LOGIC_FORMAT   format;         /**< format */
                uint8_t                 mark_time;      /**< mark duration */
                uint8_t                 space_time;     /**< space duration */
        } logic1;                       /**< digital message logic "1" settings */

        HW_IRGEN_FIFO   repeat_fifo;    /**< FIFO with repeat message */
        uint16_t        repeat_time;    /**< repeat interval */
        HW_IRGEN_OUTPUT output;         /**< output type */
} irgen_config;

/**
 * \brief IR generator interrupt handler
 *
 */
typedef void (hw_irgen_interrupt_cb)(void);

/**
 * \brief Initialize IR generator controller
 *
 * \param [in] cfg configuration (optional)
 *
 */
void hw_irgen_init(irgen_config *cfg);

/**
 * \brief Configure IR generator
 *
 * If \p cfg is NULL, this function does nothing.
 *
 * \param [in] cfg configuration
 *
 */
void hw_irgen_configure(irgen_config *cfg);

/**
 * \brief Register interrupt callback
 *
 * This effectively enables interrupt from IR generator.
 * Handler function is responsible for clearing interrupt using hw_irgen_clear_interrupt().
 *
 * \param [in] cb   interrupt handler
 *
 * \sa hw_irgen_clear_interrupt
 *
 */
void hw_irgen_register_interrupt(hw_irgen_interrupt_cb cb);

/**
 * \brief Unregister interrupt callback
 *
 * This effectively disables interrupt from IR generator.
 *
 */
void hw_irgen_unregister_interrupt(void);

/**
 * \brief Clear interrupt
 *
 * This should be called in interrupt handler to clear interrupt.
 *
 * \sa hw_irgen_register_interrupt
 *
 */
static inline void hw_irgen_clear_interrupt(void)
{
        (void) REG_GETF(IR, IR_IRQ_STATUS_REG, IR_IRQ_ACK);
}

/**
 * \brief Set carrier frequency
 *
 * \param [in] hi_time   carrier signal high duration in IR clock cycles (must be >0)
 * \param [in] lo_time   carrier signal low duration in IR clock cycles (must be >0)
 *
 */
static inline void hw_irgen_set_carrier_freq(uint16_t hi_time, uint16_t lo_time)
{
        IR->IR_FREQ_CARRIER_ON_REG = hi_time;
        IR->IR_FREQ_CARRIER_OFF_REG = lo_time;
}

/**
 * \brief Set "logic 0" parameters
 *
 * \param [in] format       "logic 0" format
 * \param [in] mark_time    mark duration in carrier clock cycles (must be >0)
 * \param [in] space_time   space duration in carrier clock cycles (must be >0)
 *
 * \sa hw_ir_insert_digital_message
 */
static inline void hw_irgen_set_logic0_param(HW_IRGEN_LOGIC_FORMAT format, uint8_t mark_time,
                                                                                uint8_t space_time)
{
        IR->IR_LOGIC_ZERO_TIME_REG = (uint16_t)
                (mark_time << IR_IR_LOGIC_ZERO_TIME_REG_IR_LOGIC_ZERO_MARK_Pos | space_time);
        REG_SETF(IR, IR_CTRL_REG, IR_LOGIC_ZERO_FORMAT, format);
}

/**
 * \brief Set "logic 1" parameters
 *
 * \param [in] format       "logic 1" format
 * \param [in] mark_time    mark duration in carrier clock cycles (must be >0)
 * \param [in] space_time   space duration in carrier clock cycles (must be >0)
 *
 * \sa hw_ir_insert_digital_message
 */
static inline void hw_irgen_set_logic1_param(HW_IRGEN_LOGIC_FORMAT format, uint8_t mark_time,
                                                                                uint8_t space_time)
{
        IR->IR_LOGIC_ONE_TIME_REG = (uint16_t)
                (mark_time << IR_IR_LOGIC_ONE_TIME_REG_IR_LOGIC_ONE_MARK_Pos | space_time);
        REG_SETF(IR, IR_CTRL_REG, IR_LOGIC_ONE_FORMAT, format);
}

/**
 * \brief Set command repeat source FIFO
 *
 * This selects FIFO to be used for sending repeated commands.
 *
 * \param [in] fifo   FIFO used for command repeat
 *
 * \sa hw_irgen_set_repeat_time
 *
 */
static inline void hw_irgen_set_repeat_fifo(HW_IRGEN_FIFO fifo)
{
        REG_SETF(IR, IR_CTRL_REG, IR_REPEAT_TYPE, fifo);
}

/**
 * \brief Set command repeat time
 *
 * \param [in] time repeat time in carrier clock cycles
 *
 * \sa hw_irgen_set_repeat_type
 *
 */
static inline void hw_irgen_set_repeat_time(uint16_t time)
{
        IR->IR_REPEAT_TIME_REG = time;
}

/**
 * \brief Insert Digital message into FIFO
 *
 * Digital message is represented by \p payload as sequence of "logic 0" and "logic 1".
 *
 * \param [in] fifo      FIFO to be used
 * \param [in] length    number of valid bits in \p message (must be >0)
 * \param [in] payload   message content
 *
 * \sa hw_irgen_set_logic0_format
 * \sa hw_irgen_set_logic1_format
 *
 */
static inline void hw_irgen_insert_digital_message(HW_IRGEN_FIFO fifo, uint8_t length,
                                                                                uint16_t payload)
{
        /* reg[15] = digital message (1), reg[14:11] = length-1, reg[10:0] = payload */
        uint16_t value = 0x8000 | (((length - 1) & 0xF) << 11) | (payload & 0x7FF);
        if (fifo == HW_IRGEN_FIFO_REPEAT) {
                IR->IR_REPEAT_FIFO_REG = value;
        } else {
                IR->IR_MAIN_FIFO_REG = value;
        }
}

/**
 * \brief Insert Paint message into FIFO
 *
 * Paint message is represented by either mark or space and its duration. This allows to represent
 * any custom "painted" waveform.
 *
 * \param [in] fifo       FIFO to be used
 * \param [in] symbol     symbol type
 * \param [in] duration   symbol duration in carrier clock cycles
 *
 */
static inline void hw_irgen_insert_paint_message(HW_IRGEN_FIFO fifo, HW_IRGEN_PAINT symbol,
                                                                                uint16_t duration)
{
        /* reg[15] = paint message (0), reg[14] = symbol, reg[13:0] = duration */
        uint16_t value = (!!symbol << 14) | (duration & 0x3FFF);

        if (fifo == HW_IRGEN_FIFO_REPEAT) {
                IR->IR_REPEAT_FIFO_REG = value;
        } else {
                IR->IR_MAIN_FIFO_REG = value;
        }
}

/**
 * \brief Flush FIFO
 *
 * Paint message is represented by either mark or space and its duration. This allows to represent
 * any custom "painted" waveform.
 *
 * \param [in] fifo   FIFO to be flushed
 *
 */
static inline void hw_irgen_flush_fifo(HW_IRGEN_FIFO fifo)
{
        if (fifo == HW_IRGEN_FIFO_REPEAT) {
                REG_SETF(IR, IR_CTRL_REG, IR_REP_FIFO_RESET, 1);
        } else {
                REG_SETF(IR, IR_CTRL_REG, IR_CODE_FIFO_RESET, 1);
        }
}

/**
 * \brief Set output type (normal or inverted)
 *
 * \param [in] output   output type
 *
 */
static inline void hw_irgen_set_output_type(HW_IRGEN_OUTPUT output)
{
        REG_SETF(IR, IR_CTRL_REG, IR_INVERT_OUTPUT, output);
}

/**
 * \brief Start IR data transmission
 *
 */
static inline void hw_irgen_start(void)
{
        REG_SETF(IR, IR_CTRL_REG, IR_TX_START, 1);
}

/**
 * \brief Stop IR data transmission
 *
 * When called after transmission is started, this will also flush code FIFO.
 *
 */
static inline void hw_irgen_stop(void)
{
        REG_SETF(IR, IR_CTRL_REG, IR_TX_START, 0);
}

/**
 * \brief Check if IR generator is busy
 *
 * \return true if IR generator is busy, false otherwise
 *
 */
static inline bool hw_irgen_is_busy(void)
{
        return REG_GETF(IR, IR_STATUS_REG, IR_BUSY);
}

/**
 * \brief Get number of messages in FIFO
 *
 * \param [in] fifo   FIFO
 *
 * \return number of messages
 *
 */
static inline uint16_t hw_irgen_get_fifo_level(HW_IRGEN_FIFO fifo)
{
        if (fifo == HW_IRGEN_FIFO_REPEAT) {
                return REG_GETF(IR, IR_STATUS_REG, IR_REP_FIFO_WRDS);
        } else {
                return REG_GETF(IR, IR_STATUS_REG, IR_CODE_FIFO_WRDS);
        }
}

/**
 * \brief Reverse bit order
 *
 * Helper function to reverse order of bits in message.
 *
 * \param [in] val    message value
 * \param [in] bits   number of valid bits in \p val
 *
 * \return \p val with bit order reversed
 *
 */
static inline uint16_t hw_irgen_reverse_bit_order(uint16_t val, uint8_t bits)
{
        uint16_t ret = 0;

        while (bits--) {
                ret = (ret << 1) | (val & 1);
                val >>= 1;
        }

        return ret;
}

#endif /* dg_configUSE_HW_IRGEN */

#endif /* HW_IRGEN_H_ */



/**
 * \}
 * \}
 * \}
 */
