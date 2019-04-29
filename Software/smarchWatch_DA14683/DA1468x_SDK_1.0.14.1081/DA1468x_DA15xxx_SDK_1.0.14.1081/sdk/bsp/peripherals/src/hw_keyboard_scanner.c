/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup Keyboard_Scanner
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file hw_keyboard_scanner.c
 *
 * @brief Implementation of the Keyboard scanner Low Level Driver.
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configUSE_HW_KEYBOARD_SCANNER


#include <stdint.h>
#include "sdk_defs.h"
#include "hw_gpio.h"
#include "hw_keyboard_scanner.h"

#if (dg_configSYSTEMVIEW)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif

#define KBSCN_STATUS_REG_INVALID_VALUE  (0x1000)
#define KBSCN_GPIOx_MODE_REG_ADDR(_x)   ((uint16_t *)KBSCAN_BASE + 6 + _x)
#define KBSCN_GPIOx_MODE_REG(_x)        *KBSCN_GPIOx_MODE_REG_ADDR(_x)


/*
 * Local variables
 */
HW_KBSCN_STATUS hw_kbscn_status;

const struct kbscn_init_tag *hw_kbscn_env;

uint32_t *msg_wr_pos;
uint32_t *msg_rd_pos;



/*
 * Forward declarations
 */
static void hw_kbscn_clear_irq(void);
static int process_key_events(void);



/*
 * Function definitions
 */


/**
 * \brief Get the actual port and pin that a GPIO index refers to.
 *
 * \param [in] index The GPIO index.
 *
 * \param [out] port The actual GPIO port.
 * \param [out] pin  The actual GPIO pin.
 *
 * \return void
 *
 */
void conv_index_to_port_pin(int index, HW_GPIO_PORT *port, HW_GPIO_PIN *pin)
{
        *port = index >> 3; // div by 8
        *pin = index & 0x7; // fast modulo by 8
}


void hw_kbscn_setup_matrix(const struct kbscn_init_tag *init_env, bool mode)
{
        int i;
        HW_GPIO_PORT port;
        HW_GPIO_PIN pin;

        // The rows first...
        for (i = 0; i < init_env->num_rows; i++) {
                ASSERT_WARNING(init_env->rows[i] < (UNUSED_INDEX + 1)); // out of range
                if (init_env->rows[i] == UNUSED_INDEX) {
                        continue; // unused pin
                }

                conv_index_to_port_pin(init_env->rows[i], &port, &pin);

                if (mode) {
                        hw_gpio_configure_pin(port, pin, HW_GPIO_MODE_OUTPUT, HW_GPIO_FUNC_GPIO, false);
                }
                else {
                        hw_gpio_set_pin_function(port, pin, HW_GPIO_MODE_INPUT, HW_GPIO_FUNC_GPIO);
                }
        }

        // Then the columns...
        for (i = 0; i < init_env->num_columns; i++) {
                int index;

                index = init_env->columns[i];

                ASSERT_WARNING(index < (UNUSED_INDEX + 1)); // out of range
                if (index == UNUSED_INDEX) {
                        continue; // unused pin
                }

                conv_index_to_port_pin(index, &port, &pin);

                if (mode) {
                        hw_gpio_set_pin_function(port, pin, HW_GPIO_MODE_INPUT_PULLUP, HW_GPIO_FUNC_GPIO);
                }
                else {
                        hw_gpio_set_pin_function(port, pin, HW_GPIO_MODE_INPUT, HW_GPIO_FUNC_GPIO);
                }
        }
}


int hw_kbscn_init_matrix(const struct kbscn_init_tag *init_env)
{
        int i;
        HW_GPIO_PORT port;
        HW_GPIO_PIN pin;

        ASSERT_WARNING(init_env); // init_env cannot be NULL
        ASSERT_WARNING(init_env->rows); // rows[] cannot be NULL
        ASSERT_WARNING(init_env->columns); // columns[] cannot be NULL
        ASSERT_WARNING(init_env->num_rows < 16); // the number of rows must be within limits
        ASSERT_WARNING(init_env->num_columns < 20); // the number of columns must be within limits

        // Reset everything
        for (i = 0; i <= 36; i++) {
                if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE) {
                        KBSCN_GPIOx_MODE_REG(i) = 0;
                }
        }

        // The rows first...
        for (i = 0; i < init_env->num_rows; i++) {
                ASSERT_WARNING(init_env->rows[i] < (UNUSED_INDEX + 1)); // out of range
                if (init_env->rows[i] == UNUSED_INDEX) {
                        continue; // unused pin
                }
                /* Assert if already used */
                ASSERT_WARNING(!(KBSCN_GPIOx_MODE_REG(init_env->rows[i]) & KBSCAN_KBSCN_P00_MODE_REG_KBSCN_GPIO_EN_Msk));
                KBSCN_GPIOx_MODE_REG(init_env->rows[i]) = (KBSCAN_KBSCN_P00_MODE_REG_KBSCN_GPIO_EN_Msk |
                                                           KBSCAN_KBSCN_P00_MODE_REG_KBSCN_ROW_Msk | i);
                /* And now the actual GPIO (only the direction matters now) */
                conv_index_to_port_pin(init_env->rows[i], &port, &pin);
                hw_gpio_set_pin_function(port, pin, HW_GPIO_MODE_INPUT, HW_GPIO_FUNC_KB_ROW);
        }

        // Then the columns...
        for (i = 0; i < init_env->num_columns; i++) {
                ASSERT_WARNING(init_env->columns[i] < (UNUSED_INDEX + 1)); // out of range
                if (init_env->columns[i] == UNUSED_INDEX) {
                        continue; // unused pin
                }
                /* Assert if already used */
                ASSERT_WARNING(!(KBSCN_GPIOx_MODE_REG(init_env->columns[i]) & KBSCAN_KBSCN_P00_MODE_REG_KBSCN_GPIO_EN_Msk));
                KBSCN_GPIOx_MODE_REG(init_env->columns[i]) = (KBSCAN_KBSCN_P00_MODE_REG_KBSCN_GPIO_EN_Msk | i);
                /* And now the actual GPIO (only the direction matters now) */
                conv_index_to_port_pin(init_env->columns[i], &port, &pin);
                hw_gpio_set_pin_function(port, pin, HW_GPIO_MODE_INPUT_PULLUP, HW_GPIO_FUNC_GPIO);
        }

        return 0;
}


int hw_kbscn_init(const struct kbscn_init_tag *init_env,
                  uint32_t *msg_wr_idx,
                  uint32_t *msg_rd_idx)
{
        uint16_t tmp;

        // Sanity tests
        ASSERT_WARNING(init_env); // init_env cannot be NULL
        ASSERT_WARNING(init_env->row_scan_active_time != 0); // row scan time cannot be 0
        ASSERT_WARNING(init_env->debounce_press_time != 0); // press debounce time cannot be 0
        ASSERT_WARNING(init_env->debounce_press_time < 64); // press debounce time too big...
        ASSERT_WARNING(init_env->debounce_release_time != 0); // release debounce time cannot be 0
        ASSERT_WARNING(init_env->debounce_release_time < 64); // release debounce time too big...
        ASSERT_WARNING(init_env->inactive_time < 128); // inactivity timeout too big...
        ASSERT_WARNING( !((init_env->inactive_evt) && (init_env->inactive_time == 0)) ); // if the inactivity interrupt is enabled then the inactivity timeout cannot be zero
        ASSERT_WARNING(init_env->clock_div < 4); // invalid clock divisor
        ASSERT_WARNING(init_env->msg_cb); // msg_cb must be defined
        ASSERT_WARNING(init_env->msg_buf); // key events buffer cannot be NULL
        ASSERT_WARNING(init_env->msg_buf_sz); // the key events buffer size cannot be 0
        ASSERT_WARNING((init_env->msg_buf_sz & (init_env->msg_buf_sz - 1)) == 0); // the key events buffer size must be a base-2 number

        // 1. Set up the key matrix
        hw_kbscn_init_matrix(init_env);

        // Then the size...
        KBSCAN->KBSCN_MATRIX_SIZE_REG = ((init_env->num_columns - 1) << 4) | (init_env->num_rows - 1);

        // 2. Set up row scan time
        KBSCAN->KBSCN_CTRL2_REG = init_env->row_scan_active_time;

        // 3. Set up debouncing times
        tmp = (init_env->debounce_press_time << 6) | init_env->debounce_release_time;
        KBSCAN->KBSCN_DEBOUNCE_REG = tmp;

        // 4. Set up inactivity, IRQ masks and clock
        tmp = init_env->inactive_time << KBSCAN_KBSCN_CTRL_REG_KBSCN_INACTIVE_TIME_Pos;

        if (init_env->inactive_time) {
                tmp |= KBSCAN_KBSCN_CTRL_REG_KBSCN_INACTIVE_EN_Msk;
        }

        if (init_env->inactive_evt) {
                tmp |= KBSCAN_KBSCN_CTRL_REG_KBSCN_IRQ_INACTIVE_MASK_Msk;
        }

        if (init_env->msg_evt) {
                tmp |= KBSCAN_KBSCN_CTRL_REG_KBSCN_IRQ_MESSAGE_MASK_Msk;
        }

        if (init_env->fifo_evt) {
                tmp |= KBSCAN_KBSCN_CTRL_REG_KBSCN_IRQ_FIFO_MASK_Msk;
        }

        tmp |= (init_env->clock_div << KBSCAN_KBSCN_CTRL_REG_KBSCN_CLKDIV_Pos);

        KBSCAN->KBSCN_CTRL_REG = tmp;

        // 5. Set the clock source
        GLOBAL_INT_DISABLE();
        uint32_t clk_per_reg = CRG_PER->CLK_PER_REG;
        REG_SET_FIELD(CRG_PER, CLK_PER_REG, KBSCAN_CLK_SEL, clk_per_reg, 0); // DIVN => use always 16MHz for this block
        REG_SET_FIELD(CRG_PER, CLK_PER_REG, KBSCAN_ENABLE, clk_per_reg, 1); // enable clock
        CRG_PER->CLK_PER_REG = clk_per_reg;
        GLOBAL_INT_RESTORE();

        // 6. Set the environment
        hw_kbscn_env = init_env;

        // 7. Set the buffer pointers
        msg_wr_pos = msg_wr_idx;
        msg_rd_pos = msg_rd_idx;

        hw_kbscn_status = KBSCN_INITIALIZED;

        return 0;
}


int hw_kbscn_enable_ex(bool enable_isr)
{
        /* Block must be initialized */
        ASSERT_WARNING(hw_kbscn_status == KBSCN_INITIALIZED);

        GLOBAL_INT_DISABLE();
        hw_kbscn_clear_irq(); // Clear any pending IRQ
        NVIC_ClearPendingIRQ(KEYBRD_IRQn); // Clear from NVIC too
        if (enable_isr) {
                NVIC_EnableIRQ(KEYBRD_IRQn); // Enable in NVIC
        }
        GLOBAL_INT_RESTORE();

        REG_SET_BIT(KBSCAN, KBSCN_CTRL_REG, KBSCN_EN); // Enable block

        hw_kbscn_status = KBSCN_ENABLED;

        return 0;
}


int hw_kbscn_enable(void)
{
        return hw_kbscn_enable_ex(true);
}


int hw_kbscn_disable_ex(bool disable_isr)
{
        /* Block must be enabled to disable it */
        ASSERT_WARNING(hw_kbscn_status == KBSCN_ENABLED);

        REG_CLR_BIT(KBSCAN, KBSCN_CTRL_REG, KBSCN_EN);; // Disable block

        GLOBAL_INT_DISABLE();
        hw_kbscn_clear_irq(); // Clear any pending IRQ
        if (disable_isr) {
                NVIC_DisableIRQ(KEYBRD_IRQn); // Disable in NVIC
        }
        NVIC_ClearPendingIRQ(KEYBRD_IRQn); // Clear from NVIC too
        GLOBAL_INT_RESTORE();

        hw_kbscn_status = KBSCN_INITIALIZED;

        return 0;
}


int hw_kbscn_disable(void)
{
        return hw_kbscn_disable_ex(true);
}


bool hw_kbscn_get_status(void)
{
        return (hw_kbscn_status == KBSCN_ENABLED);
}


int hw_kbscn_activate_msg_evt(void)
{
        /* Block must be enabled to activate it */
        ASSERT_WARNING(hw_kbscn_status == KBSCN_ENABLED);

        REG_SET_BIT(KBSCAN, KBSCN_CTRL_REG, KBSCN_IRQ_MESSAGE_MASK); // Enable message IRQ

        return 0;
}


void hw_kbscn_disable_inactivity(void)
{
        uint16_t tmp = KBSCAN->KBSCN_CTRL_REG;

        tmp &= ~KBSCAN_KBSCN_CTRL_REG_KBSCN_INACTIVE_EN_Msk;
        tmp &= ~KBSCAN_KBSCN_CTRL_REG_KBSCN_IRQ_INACTIVE_MASK_Msk;
        KBSCAN->KBSCN_CTRL_REG = tmp;
}


void hw_kbscn_set_inactivity(uint8_t timeout)
{
        uint16_t tmp = KBSCAN->KBSCN_CTRL_REG;

        tmp |= KBSCAN_KBSCN_CTRL_REG_KBSCN_INACTIVE_EN_Msk;
        tmp |= KBSCAN_KBSCN_CTRL_REG_KBSCN_IRQ_INACTIVE_MASK_Msk;
        tmp |= (timeout << 4);
        KBSCAN->KBSCN_CTRL_REG = tmp;
}


/**
 * \brief Clear any pending interrupts of the Keyboard scanner
 *
 * \return void
 *
 * \warning Must be called with interrupts disabled!
 *
 */
void hw_kbscn_clear_irq(void)
{
        unsigned int i;
        uint32_t stat;

        stat = KBSCAN->KBSCN_STATUS_REG; // INACTIVE is cleared here
        if (stat & KBSCAN_KBSCN_STATUS_REG_KBSCN_INACTIVE_IRQ_STATUS_Msk) {
                for (i = 0; i < ((stat & KBSCAN_KBSCN_STATUS_REG_KBSCN_NUM_MESSAGE_Msk) >>
                                         KBSCAN_KBSCN_STATUS_REG_KBSCN_NUM_MESSAGE_Pos); i++) {
                        KBSCAN->KBSCN_MESSAGE_KEY_REG;
                }
        } // FIFO MSG pending is cleared here

        // KBSCN_FIFO_UNDERFL & KBSCN_FIFO_OVERFL are cleared with KBSCN_EN = 0!
}


void hw_kbscn_reset_fifo(void)
{
        REG_SET_BIT(KBSCAN, KBSCN_CTRL_REG, KBSCN_RESET_FIFO);

        // KBSCN_FIFO_UNDERFL & KBSCN_FIFO_OVERFL are cleared with KBSCN_EN = 0!
}


/**
 * \brief Interrupt Handler of the Keyboard scanner
 *
 * \return void
 *
 */
void KEYBRD_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        uint16_t stat;
        int inactive_on;
        bool inactive_pending = false;

        if (hw_kbscn_status != KBSCN_ENABLED) {
                hw_kbscn_clear_irq(); // the block is disabled - ignore the IRQ
        }
        else {
                stat = KBSCAN->KBSCN_STATUS_REG;
                inactive_on = REG_GETF(KBSCAN, KBSCN_CTRL_REG, KBSCN_IRQ_INACTIVE_MASK);

                if (hw_kbscn_env->fifo_evt) {
                        if (stat & KBSCAN_KBSCN_STATUS_REG_KBSCN_FIFO_UNDERFL_Msk) {
                                // garbage have been delivered to the application
                                if (hw_kbscn_env->fifo_under_cb != NULL) {
                                        hw_kbscn_env->fifo_under_cb();
                                        // re-read Status as the FIFO may have been reset
                                        stat = KBSCAN->KBSCN_STATUS_REG;
                                }
                        }

                        if (stat & KBSCAN_KBSCN_STATUS_REG_KBSCN_FIFO_OVERFL_Msk) {
                                // key events have been missed due to lack of FIFO space
                                if (hw_kbscn_env->fifo_over_cb != NULL) {
                                        hw_kbscn_env->fifo_over_cb();
                                        // re-read Status as the FIFO may have been reset
                                        stat = KBSCAN->KBSCN_STATUS_REG;
                                }
                        }
                }

                if ((stat & KBSCAN_KBSCN_STATUS_REG_KBSCN_INACTIVE_IRQ_STATUS_Msk) && (inactive_on != 0)) {
                        // Inactivity event - The block is stopped.
                        hw_kbscn_status = KBSCN_INITIALIZED;
                        inactive_pending = true; // defer handling for later
                }

                if ((stat & KBSCAN_KBSCN_STATUS_REG_KBSCN_MES_IRQ_STATUS_Msk) && hw_kbscn_env->msg_evt) {
                        // Key events are available!
                        process_key_events();
                }

                if (inactive_pending) {
                        if (hw_kbscn_env->inactivity_cb != NULL) {
                                hw_kbscn_env->inactivity_cb();
                        }
                }
        }

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}


/**
 * \brief Checks if the Key events buffer is full
 *
 * \return A flag indicating whether the buffer is full (true) or not (false)
 *
 */
bool key_buffer_full(void)
{
        bool is_full = false;
        uint32_t next_pos;

        next_pos = *msg_wr_pos + 1;
        next_pos = next_pos & (hw_kbscn_env->msg_buf_sz - 1); // fast modulo operation

        if (next_pos == *msg_rd_pos) {
                is_full = true;
        }

        return is_full;
}


/**
 * \brief Process events reported by the Keyboard scanner
 *
 * \return void
 *
 */
int process_key_events(void)
{
        uint32_t stat = KBSCN_STATUS_REG_INVALID_VALUE;
        uint16_t msg;
        struct kbscn_msg_tag *p_msg;
        bool msg_buf_written = false;

        /* Make sure these pointers point somewhere valid */
        ASSERT_WARNING(hw_kbscn_env);
        ASSERT_WARNING(msg_rd_pos);
        ASSERT_WARNING(msg_wr_pos);

        // Read FIFO
        while (!key_buffer_full()) {
                msg = KBSCAN->KBSCN_MESSAGE_KEY_REG;

                p_msg = &hw_kbscn_env->msg_buf[*msg_wr_pos];

                if (msg & KBSCAN_KBSCN_MESSAGE_KEY_REG_KBSCN_LAST_ENTRY_Msk) {
                        p_msg->flags = LAST_MSG; // this is the last msg
                }
                else {
                        p_msg->flags = ((msg & KBSCAN_KBSCN_MESSAGE_KEY_REG_KBSCN_KEY_STATE_Msk) ? PRESSED : RELEASED);
                        p_msg->row = (msg & KBSCAN_KBSCN_MESSAGE_KEY_REG_KBSCN_KEYID_ROW_Msk) >>
                                                KBSCAN_KBSCN_MESSAGE_KEY_REG_KBSCN_KEYID_ROW_Pos;
                        p_msg->column = (msg & KBSCAN_KBSCN_MESSAGE_KEY_REG_KBSCN_KEYID_COLUMN_Msk) >>
                                                KBSCAN_KBSCN_MESSAGE_KEY_REG_KBSCN_KEYID_COLUMN_Pos;
                }

                (*msg_wr_pos)++;
                *msg_wr_pos &= (hw_kbscn_env->msg_buf_sz - 1); // fast modulo operation

                msg_buf_written = true;

                stat = KBSCAN->KBSCN_STATUS_REG; // update status
                if ( !(stat & KBSCAN_KBSCN_STATUS_REG_KBSCN_MES_IRQ_STATUS_Msk) ) {
                        break; // no more "full scan" messages
                }
        }

        if (stat == KBSCN_STATUS_REG_INVALID_VALUE)
                stat = KBSCAN->KBSCN_STATUS_REG;

        // What if the application's buffer is full and not all FIFO data can be copied?
        if (key_buffer_full() && (stat & KBSCAN_KBSCN_STATUS_REG_KBSCN_MES_IRQ_STATUS_Msk)) {
                // No more message events! It is the application's responsibility to re-enable it
                // after emptying its own buffer.

                REG_CLR_BIT(KBSCAN, KBSCN_CTRL_REG, KBSCN_IRQ_MESSAGE_MASK); // Mask message IRQ
        }

        // Inform application
        if (msg_buf_written) {
                hw_kbscn_env->msg_cb();
        }

        return 0;
}

#endif /* dg_configUSE_HW_KEYBOARD_SCANNER */
/**
 * \}
 * \}
 * \}
 */
