/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup Keyboard_Scanner
 * \{
 * \brief Keyboard Scanner
 */

/**
 ****************************************************************************************
 *
 * @file hw_keyboard_scanner.h
 *
 * @brief Definition of API for the Keyboard scanner Low Level Driver.
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_KEYBOARD_SCANNER_H_
#define HW_KEYBOARD_SCANNER_H_

#if dg_configUSE_HW_KEYBOARD_SCANNER

#include <stdbool.h>
#include <stdint.h>


#define FIFO_LAST_MSG           (0x7FF)
#define FIFO_SIZE               (26)

#define KEY_IS_GHOST            (0x40)          // key is a ghost (ignore)
#define KEY_SCAN_CMP            (0x20)          // last msg of a scan cycle (for deghosting)
#define KEY_STATUS_MASK         (0x10)          // pressed or released
#define KEY_FN_SET_MASK         (0x0F)          // mask for fn modifier

#define UNUSED_INDEX            (41)
#define UNUSED_PIN              (0x50)

// '41' marks 'unused'
#define CONV_PORT_PIN_TO_INDEX(x)                               \
        ((x) < 8 ?            (x) :                             \
         ( (x) < 0x18 ?       (x) - 0x10 + 8 :                  \
           ( (x) < 0x25 ?     (x) - 0x20 + 16:                  \
             ( (x) < 0x38 ?   (x) - 0x30 + 24:                  \
               ( (x) < 0x48 ? (x) - 0x40 + 32: UNUSED_INDEX ) ) ) ) )     \

// '0x50' marks 'unused'
#define CONV_INDEX_TO_PORT_PIN(x)                               \
        ((x) < 8 ?          (x) :                               \
         ( (x) < 16 ?       0x10 + ((x) - 8) :                  \
           ( (x) < 22 ?     0x20 + ((x) - 16) :                 \
             ( (x) < 32 ?   0x30 + ((x) - 24) :                 \
               ( (x) < 40 ? 0x40 + ((x) - 32) : UNUSED_PIN ) ) ) ) )  \


/// Block's status
typedef enum {
        KBSCN_DISABLED = 0,
        KBSCN_INITIALIZED,
        KBSCN_ENABLED,
} HW_KBSCN_STATUS;

/// Clock divisors
typedef enum {
        KBSCN_PCLK_DIV1 = 0,
        KBSCN_PCLK_DIV4,
        KBSCN_PCLK_DIV16,
        KBSCN_PCLK_DIV64,
} KBSCN_PCLK_DIV;

/// Key status
typedef enum {
        RELEASED = 0,
        PRESSED = KEY_STATUS_MASK,
        LAST_MSG = KEY_SCAN_CMP,
} KBSCN_KEY_STATUS;

/// Message struct
struct kbscn_msg_tag
{
        uint8_t flags;
        uint8_t row;
        uint8_t column;
};

typedef int (*kbscn_cback)(void);

/// Initialization struct for the configuration of the block
struct kbscn_init_tag
{
        const uint8_t *rows;            /**< the rows of the key matrix declared as the "distance"
                                             from P0_0 (i.e. P0_0 = 0, P1_0 = 8, P3_0 = 21 etc.) */
        const uint8_t *columns;         /**< the columns of the key matrix */
        uint8_t num_rows;               /**< the number of rows in the key matrix */
        uint8_t num_columns;            /**< the number of columns in the key matrix */
        uint16_t row_scan_active_time;  /**< the time a row scan will last (in clk cycles) */
        uint8_t debounce_press_time;    /**< the debounce time of a key press (in scan cycles) */
        uint8_t debounce_release_time;  /**< the debounce time of a key release */
        uint8_t inactive_time;          /**< if not zero then the inactive mode will be enabled */
        uint8_t clock_div;              /**< the frequency of the clock used by the block */
        kbscn_cback inactivity_cb;      /**< Callback for inactivity timeout */
        kbscn_cback fifo_under_cb;      /**< Callback for FIFO underrun */
        kbscn_cback fifo_over_cb;       /**< Callback for FIFO overrun */
        kbscn_cback msg_cb;             /**< Callback for message passing */
        struct kbscn_msg_tag *msg_buf;  /**< Buffer for Key events */
        uint8_t msg_buf_sz;             /**< Size of the Key events buffer (must be a base-2 number,
                                             i.e. 16, 32, 64... because key_buffer_full() uses an
                                             optimized modulo algorithm ) */
        bool msg_evt;                   /**< message ready event will trigger an INT to the M0 */
        bool inactive_evt;              /**< inactive timeout event will trigger an INT to the M0 */
        bool fifo_evt;                  /**< FIFO under/over-run event will trigger an INT to the M0 */
};


/**
 * \brief Initialize the Keyboard scanner
 *
 * \param [in] init_env The initialization parameters for this block. The information must be
 *                      accessible from the driver at all times (i.e. RO data)
 * \param [in] msg_wr_idx The position in the buffer where the driver writes to
 * \param [in] msg_rd_idx The position in the buffer where the application reads from
 *
 * \return The initialization result: 0 for success, 1 for failure
 *
 */
int hw_kbscn_init(const struct kbscn_init_tag *init_env,
                  uint32_t *msg_wr_idx,
                  uint32_t *msg_rd_idx);

/**
 * \brief Initialize the Keyboard scanner matrix. Used to initialize the GPIOs of the matrix
 *         (i.e. after a wake-up).
 *
 * \param [in] init_env The initialization parameters for this block. The information must be
 *                      accessible from the driver at all times (i.e. RO data)
 *
 * \return The initialization result: 0 for success, 1 for failure
 *
 */
int hw_kbscn_init_matrix(const struct kbscn_init_tag *init_env);

/**
 * \brief Enable the Keyboard scanner
 *
 * \return The initialization result: 0 for success, 1 for failure
 *
 * \sa hw_kbscn_enable_ex
 *
 */
int hw_kbscn_enable(void);

/**
 * \brief Enable the Keyboard scanner
 *
 * This function enables KBSCN. If \p set_isr is set, this function will
 * do the same as hw_kbscn_enable. If not, this function will not set
 * mask in NVIC interrupt controller.
 *
 * \param [in] enable_isr Set NVIC interrupt controller mask
 *
 * \return The initialization result: 0 for success, 1 for failure
 *
 * \sa hw_kbscn_enable
 *
 */
int hw_kbscn_enable_ex(bool enable_isr);

/**
 * \brief Disable the Keyboard scanner
 *
 * \return The initialization result: 0 for success, 1 for failure
 *
 * \sa hw_kbscn_disable_ex
 *
 */
int hw_kbscn_disable(void);

/**
 * \brief Disable the Keyboard scanner
 *
 * This function disables KBSCN. If \p set_isr is set, this function will
 * do the same as hw_kbscn_disable. If not, this function will not set mask
 * in NVIC interrupt controller.
 *
 * \param [in] disable_isr Set NVIC interrupt controller mask
 *
 * \return The initialization result: 0 for success, 1 for failure
 *
 * \sa hw_kbscn_disable
 *
 */
int hw_kbscn_disable_ex(bool disable_isr);

/**
 * \brief Get the driver's status
 *
 * \return The status: false for INITIALIZED/DISABLED, true for ENABLED
 *
 */
bool hw_kbscn_get_status(void);

/**
 * \brief Enable the pending message IRQ
 *
 * \return The initialization result: 0 for success, 1 for failure
 *
 */
int hw_kbscn_activate_msg_evt(void);

/**
 * \brief Disable inactivity check
 *
 * \return void
 *
 */
void hw_kbscn_disable_inactivity(void);

/**
 * \brief Set inactivity check. The Keyboard scanner will stop when inactivity times out
 *        and an interrupt will hit.
 *
 * \return void
 *
 */
void hw_kbscn_set_inactivity(uint8_t timeout);

/**
 * \brief Reset FIFO
 *
 * \return void
 *
 * \warning Must be called with interrupts disabled!
 *
 */
void hw_kbscn_reset_fifo(void);

/**
 * \brief Setup the Keyboard scanner matrix for "all-keys" scanning.
 *
 * \warning The KBSCN module should be disabled.
 *
 * \param [in] init_env The initialization parameters for this block. The information must be
 *                      accessible from the driver at all times (i.e. RO data)
 *
 * \param [in] mode     The mode of operation; false, the matrix is disabled - true, the matrix is
 *                      enabled for "any key" scanning (all rows are low)
 *
 * \return void
 *
 */
void hw_kbscn_setup_matrix(const struct kbscn_init_tag *init_env, bool mode);

#endif /* dg_configUSE_HW_KEYBOARD_SCANNER */

#endif /* HW_KEYBOARD_SCANNER_H_ */

/**
 * \}
 * \}
 * \}
 */
