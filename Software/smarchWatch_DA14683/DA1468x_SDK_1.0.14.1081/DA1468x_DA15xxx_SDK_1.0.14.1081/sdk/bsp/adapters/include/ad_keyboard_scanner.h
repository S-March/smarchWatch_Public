/**
 * \addtogroup BSP
 * \{
 * \addtogroup ADAPTERS
 * \{
 * \addtogroup KEYBOARD_SCANNER
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_keyboard_scanner.h
 *
 * @brief Keyboard Scanner Adapter API
 *
 * Copyright (C) 2016-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef AD_KEYBOARD_SCANNER_H_
#define AD_KEYBOARD_SCANNER_H_

#if dg_configKEYBOARD_SCANNER_ADAPTER

#include <stdbool.h>

/**
 * \def CONFIG_KEYBOARD_SCANNER_DEGHOSTING
 *
 * \brief Enable key de-ghosting
 *
 * Enable 2-key rollover mechanism for filtering ghost events.
 */
#ifndef CONFIG_KEYBOARD_SCANNER_DEGHOSTING
#       define CONFIG_KEYBOARD_SCANNER_DEGHOSTING       (1)
#endif

/**
 * \brief Maximum number of columns in keyboard scanner matrix
 *
 */
#define AD_KBSCN_MAX_COLUMNS    19

/**
 * \brief Maximum number of rows in keyboard scanner matrix
 *
 */
#define AD_KBSCN_MAX_ROWS       15

/**
 * \brief Keyboard scanner event
 *
 */
typedef enum {
        AD_KBSCN_EVENT_PRESSED,         /**< Key pressed event */
        AD_KBSCN_EVENT_RELEASED,        /**< Key released event */
} AD_KBSCN_EVENT;

/**
 * \brief Keyboard scanner clock divisor
 *
 */
typedef enum {
        AD_KBSCN_CLOCK_DIV_4  = 1,      /**< Clock is divided by 4 */
        AD_KBSCN_CLOCK_DIV_16 = 2,      /**< Clock is divided by 16 */
        AD_KBSCN_CLOCK_DIV_64 = 3,      /**< Clock is divided by 64 */
} AD_KBSCN_CLOCK_DIV;

/**
 * \brief Keyboard scanner pin setup
 *
 * Macros AD_KBSCN_PIN_SETUP() and ::AD_KBSCN_PIN_UNUSED can be used to define specific pin setup.
 *
 */
typedef struct {
        /** Flag indicating if cell contains pin configuration */
        uint8_t in_use : 1;
        /** GPIO Port configuration */
        uint8_t port : 3;
        /** GPIO Pin configuration */
        uint8_t pin : 3;
} ad_kbscn_pin_setup;

/**
 * \brief Initializer for pin setup
 *
 */
#define AD_KBSCN_PIN_SETUP(_port, _pin) \
        {                               \
                .in_use = true,         \
                .port = _port,          \
                .pin = _pin,            \
        }

/**
 * \brief Initializer for unused pin
 *
 */
#define AD_KBSCN_PIN_UNUSED             \
        {                               \
                .in_use = false,        \
        }

/**
 * \brief Keyboard scanner event callback
 *
 * Adapter callback registered by application.
 *
 * \param [in] event    event
 * \param [in] key      key defined by \p key_matrix in ad_kbscn_config
 *
 * \note This callback is called from ISR context.
 *
 */
typedef void (* ad_kbscn_cb_t) (AD_KBSCN_EVENT event, char key);

/**
 * \brief Keyboard scanner adapter configuration
 *
 */
typedef struct {
        uint8_t num_rows;       /**< Number of rows in matrix \note this value can't be larger than ::AD_KBSCN_MAX_ROWS */
        uint8_t num_columns;    /**< Number of columns in matrix \note this value can't be larger than ::AD_KBSCN_MAX_COLUMNS */
        const ad_kbscn_pin_setup *rows;       /**< Pins setup for rows in matrix */
        const ad_kbscn_pin_setup *columns;    /**< Pins setup for columns in matrix */
        const char *key_matrix; /**< Alphanumeric keys definition, see ad_kbscn_init() for example usage */

        AD_KBSCN_CLOCK_DIV clock_div; /**< Keyboard scanner clock divisor (clock frequency is 16MHz) */

        /**
         * Time to scan each row in each keyboard full scan cycle.
         *
         * This value is expressed as number of keyboard clock cycles, e.g. for
         * <code>clock_div=16,</code> keyboard clock is <code>16 MHz / 16 = 1 MHz</code>, thus each
         * keyboard clock cycle takes 1 µs.
         *
         */
        uint16_t row_scan_time;

        /**
         * Debounce time for button press
         *
         * This value is expressed as number of keyboard full scan cycles.
         * Each full scan cycle takes <code>num_rows * (row_scan_time + 2)</code> clock cycles.
         *
         */
        uint8_t debounce_press_time;

        /**
         * Debounce time for button release
         *
         * This value is expressed as number of keyboard full scan cycles.
         * Each full scan cycle takes <code>num_rows * (row_scan_time + 2)</code> clock cycles.
         *
         */
        uint8_t debounce_release_time;

        /**
         * Inactive time to wait after last kbd event before allowing system to sleep.
         *
         * This value is expressed as number of keyboard full scan cycles.
         * Each full scan cycle takes <code>num_rows * (row_scan_time + 2)</code> clock cycles.
         *
         */
        uint8_t inactive_time;

        ad_kbscn_cb_t cb;       /**< Application defined callback */
} ad_kbscn_config;

/**
 * Keyboard scanner adapter config initialization macro
 *
 */
#define AD_KBSCN_CONFIG_WITH_INACTIVE_TIME(_rows, _columns, _key_matrix, _clock_div,            \
                                _row_scan_time, _debounce_press, _debounce_release,             \
                                                _inactive_time, _cb)                            \
        {                                                                                       \
                .num_rows = sizeof(_rows) / sizeof(_rows[0]),                                   \
                .num_columns = sizeof(_columns) / sizeof(_columns[0]),                          \
                .rows = _rows,                                                                  \
                .columns = _columns,                                                            \
                .key_matrix = _key_matrix,                                                      \
                .clock_div = _clock_div,                                                        \
                .row_scan_time = _row_scan_time,                                                \
                .debounce_press_time = _debounce_press,                                         \
                .debounce_release_time = _debounce_release,                                     \
                .inactive_time = _inactive_time,                                                \
                .cb = _cb,                                                                      \
        }

#define AD_KBSCN_CONFIG(_rows, _columns, _key_matrix, _clock_div,                               \
                        _row_scan_time, _debounce_press, _debounce_release,  _cb)               \
        {                                                                                       \
                .num_rows = sizeof(_rows) / sizeof(_rows[0]),                                   \
                .num_columns = sizeof(_columns) / sizeof(_columns[0]),                          \
                .rows = _rows,                                                                  \
                .columns = _columns,                                                            \
                .key_matrix = _key_matrix,                                                      \
                .clock_div = _clock_div,                                                        \
                .row_scan_time = _row_scan_time,                                                \
                .debounce_press_time = _debounce_press,                                         \
                .debounce_release_time = _debounce_release,                                     \
                .inactive_time = 1,                                                             \
                .cb = _cb,                                                                      \
        }
/**
 * \brief Initialize Keyboard Scanner Adapter
 *
 * This function register keyboard scanner matrix and set all columns and rows as defined in config
 *
 * \param [in] config   keyboard scanner adapter config
 *
 * \return true if adapter has been initialized successfully, false if number of columns/rows
 * is incorrect or adapter has been already initialized.
 *
 * Example usage with 12 elements matrix:
 * \code{.c}
 * // define column setup
 * static const ad_kbscn_pin_setup columns[] = {
 *         AD_KBSCN_PIN_SETUP(HW_GPIO_PORT_4, HW_GPIO_PIN_0),
 *         AD_KBSCN_PIN_SETUP(HW_GPIO_PORT_4, HW_GPIO_PIN_1),
 *         AD_KBSCN_PIN_SETUP(HW_GPIO_PORT_4, HW_GPIO_PIN_2),
 * };
 *
 * // define row setup
 * static const ad_kbscn_pin_setup rows[] = {
 *         AD_KBSCN_PIN_SETUP(HW_GPIO_PORT_4, HW_GPIO_PIN_3),
 *         AD_KBSCN_PIN_SETUP(HW_GPIO_PORT_4, HW_GPIO_PIN_4),
 *         AD_KBSCN_PIN_SETUP(HW_GPIO_PORT_4, HW_GPIO_PIN_5),
 *         AD_KBSCN_PIN_SETUP(HW_GPIO_PORT_4, HW_GPIO_PIN_6),
 * };
 *
 * // define matrix with keys - they will be reported in callback
 * static const char key_matrix[] = {
 * //       ,--------------- column 0
 * //       |    ,---------- column 1
 * //       |    |    ,----- column 2
 * //       v    v    v
 *         '7', '8', '9', // row 0
 *         '4', '5', '6', // row 1
 *         '1', '2', '3', // row 2
 *         '*', '0', '#', // row 3
 * };
 *
 * // application callback
 * void key_cb(AD_KBSCN_EVENT event, char c);
 *
 * // keyboard scanner adapter config
 * static const ad_kbscn_config config =
 *         AD_KBSCN_CONFIG(rows, columns, key_matrix, AD_KBSCN_CLOCK_DIV_16, 150, 10, 10, key_cb);
 *
 * static void init(void)
 * {
 *         // initialize keyboard scanner adapter
 *         ad_kbscn_init(&config);
 * }
 * \endcode
 *
 */
bool ad_kbscn_init(const ad_kbscn_config *config);

/**
 * \brief Keyboard scanner adapter cleanup
 *
 * Function unregisters keyboard scanner adapter, callback and clean keyboard scanner matrix.
 * After cleanup, adapter may be initialized again with other values.
 */
void ad_kbscn_cleanup(void);

#endif /* dg_configKEYBOARD_SCANNER_ADAPTER */

#endif /* AD_KEYBOARD_SCANNER_H_ */
/**
 \}
 \}
 \}
 */
