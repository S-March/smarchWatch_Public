/**
 ****************************************************************************************
 *
 * @file ad_keyboard_scanner.c
 *
 * @brief Keyboard Scanner Adapter
 *
 * Copyright (C) 2016-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configKEYBOARD_SCANNER_ADAPTER
#if (dg_configLATCH_WKUP_SOURCE == 1) && (dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A)
#error "Keyboard scanner adapter and wakeup latch support (dg_configLATCH_WKUP_SOURCE) are not concurrently supported!"
#endif
#include <string.h>
#include "osal.h"
#include "sys_power_mgr.h"
#include "hw_wkup.h"
#include "hw_keyboard_scanner.h"
#include "ad_keyboard_scanner.h"

/* Helpers converting column value to port and pin */
#define GET_PORT(column)        ((column) >> 3)
#define GET_PIN(column)         ((column) & 0x07)

/* Helpers for setting and clearing key matrix */
#define SET_KEY(matrix, column, row) matrix[column] |= (1 << row)
#define CLEAR_KEY(matrix, column, row) matrix[column] &= ~(1 << row);

/* Keyboard scanner buffer size, shall be power of 2 */
#define BUFFER_SIZE             16
#if BUFFER_SIZE & (BUFFER_SIZE - 1)
#       error "BUFFER_SIZE must be a power of 2"
#endif

/* Keyboard scanner adapter data */
PRIVILEGED_DATA static struct {
        /* HW_keayboard_scanner init environment */
        struct kbscn_init_tag init_env;
        /* Matrix passed by user */
        const char *matrix;
        /* Application callback */
        ad_kbscn_cb_t app_cb;
        /* Message write index */
        uint32_t msg_wr_idx;
        /* Message read index */
        uint32_t msg_rd_idx;
        /* HW keyboard scanner message buffer */
        struct kbscn_msg_tag msg_buf[BUFFER_SIZE];
        /* Columns setup */
        uint8_t columns[AD_KBSCN_MAX_COLUMNS];
        /* Rows setup */
        uint8_t rows[AD_KBSCN_MAX_ROWS];
        /* Adapter id returned from pm_register_adapter() */
        pm_id_t adapter_id;
#if CONFIG_KEYBOARD_SCANNER_DEGHOSTING
        /* Matrix states used for deghosting */
        uint16_t matrix_state[AD_KBSCN_MAX_COLUMNS];
        uint16_t matrix_ghost[AD_KBSCN_MAX_COLUMNS];
#endif
} ad_data;

static bool checking_inactivity = false;

static inline char get_char(uint8_t row, uint8_t column)
{
        return *(ad_data.matrix + (row * ad_data.init_env.num_columns) + column);
}

static void set_inactivity(bool inactivity_check)
{
        GLOBAL_INT_DISABLE();
        checking_inactivity = inactivity_check;
        if (inactivity_check) {
                hw_kbscn_set_inactivity(ad_data.init_env.inactive_time);
        } else {
                hw_kbscn_disable_inactivity();
        }
        GLOBAL_INT_RESTORE();

}

static void app_cb(uint8_t column, uint8_t row, AD_KBSCN_EVENT status)
{
        char c = get_char(row, column);

        ad_data.app_cb(status, c);
}

static void enable_scanner(bool inactivity_check)
{
        set_inactivity(inactivity_check);
        hw_kbscn_enable_ex(false);
        hw_kbscn_activate_msg_evt();
}

static void kbscn_wkup_intr_cb()
{
#if (dg_configLATCH_WKUP_SOURCE == 1) && (dg_configBLACK_ORCA_IC_REV != BLACK_ORCA_IC_REV_A)
        uint8_t i;
        bool restore_scan = false;
        for (i = 0; i < ad_data.init_env.num_columns; i++) {
                if (hw_wkup_get_status(GET_PORT(ad_data.columns[i])) & (1 << GET_PIN(ad_data.columns[i]))) {
                        hw_wkup_clear_status(GET_PORT(ad_data.columns[i]), 1 << GET_PIN(ad_data.columns[i]));
                        restore_scan = true;
                }
        }
        if (restore_scan) {
                /* Block sleep for inactivity time period */
                set_inactivity(true);
        }
#else
        /* Block sleep for inactivity time period */
        set_inactivity(true);
#endif
        /*
         * Interrupt handler should always reset interrupt state, otherwise it will be called again.
         */
        hw_wkup_reset_interrupt();
}

static int kbscn_inactivity_cb(void)
{
        /* Inactivity time elapsed, keep scanner on but don't block sleep any more */
        enable_scanner(false);

        return 0;
}

static int kbscn_fifo_over_cb(void)
{
        GLOBAL_INT_DISABLE();

        hw_kbscn_reset_fifo();
        hw_kbscn_disable();

        ad_data.msg_wr_idx = 0;
        ad_data.msg_rd_idx = 0;

        hw_kbscn_enable();

        GLOBAL_INT_RESTORE();

        return 0;
}

static int kbscn_fifo_under_cb(void)
{
        GLOBAL_INT_DISABLE();

        hw_kbscn_reset_fifo();
        hw_kbscn_disable();

        ad_data.msg_wr_idx = 0;
        ad_data.msg_rd_idx = 0;

        hw_kbscn_enable();

        GLOBAL_INT_RESTORE();

        return 0;
}

#if CONFIG_KEYBOARD_SCANNER_DEGHOSTING
static bool press_is_ghost(const uint8_t column, const uint8_t row)
{
        uint16_t mask = 1 << row;
        int i;

        if (ad_data.matrix_state[column] & ~(mask)) {
                mask = ad_data.matrix_state[column];

                for (i = 0; i < AD_KBSCN_MAX_COLUMNS; i++) {
                        if (i == column) {
                                continue;
                        }

                        if (ad_data.matrix_state[i] & mask) {
                                return true;
                        }
                }
        } else {
                for (i = 0; i < AD_KBSCN_MAX_COLUMNS; i++) {
                        if (i == column) {
                                continue;
                        }

                        if ((ad_data.matrix_state[i] & mask) && (ad_data.matrix_state[i] & ~mask)) {
                                return true;
                        }
                }
        }

        return false;
}

static bool copy_new_data(uint32_t start_idx, uint32_t end_idx)
{
        struct kbscn_msg_tag *msg;

        while (start_idx != end_idx) {
                msg = &ad_data.msg_buf[start_idx];

                if (msg->flags & LAST_MSG) {
                        // kbd_new_scandata contains the "next" scan status
                        return true;
                }

                if (msg->flags & PRESSED) {
                        SET_KEY(ad_data.matrix_state, msg->column, msg->row);
                }
                else {
                        CLEAR_KEY(ad_data.matrix_state, msg->column, msg->row);
                }

                start_idx++;
                start_idx &= (BUFFER_SIZE - 1); // fast modulo operation
        }

        return false;
}
#endif

static int kbscn_msg_cb(void)
{
        struct kbscn_msg_tag *msg;
        uint32_t msg_read_to_idx;
#if CONFIG_KEYBOARD_SCANNER_DEGHOSTING
        msg_read_to_idx = ad_data.msg_rd_idx;

        do {
                /* Check if scan result is complete */
                if (!copy_new_data(msg_read_to_idx, ad_data.msg_wr_idx)) {
                        break;
                }

                // do deghosting
                while (msg_read_to_idx != ad_data.msg_wr_idx) {
                        msg = &ad_data.msg_buf[msg_read_to_idx];

                        msg_read_to_idx++;
                        msg_read_to_idx &= (BUFFER_SIZE - 1);

                        if (msg->flags & LAST_MSG) {
                                break;
                        }

                        if (msg->flags & PRESSED) {
                                if (press_is_ghost(msg->column, msg->row)) {
                                        msg->flags |= KEY_IS_GHOST;
                                        SET_KEY(ad_data.matrix_ghost, msg->column, msg->row);
                                }
                        }
                        else {
                                if (ad_data.matrix_ghost[msg->column] & 1 << msg->row) {
                                        msg->flags |= KEY_IS_GHOST;
                                        CLEAR_KEY(ad_data.matrix_ghost, msg->column, msg->row);
                                }
                        }
                }
        } while (msg_read_to_idx != ad_data.msg_wr_idx);
#else
        msg_read_to_idx = ad_data.msg_wr_idx;
#endif
        // Consume the deghosted key (i.e. print them)
        while (ad_data.msg_rd_idx != msg_read_to_idx) {
                msg = &ad_data.msg_buf[ad_data.msg_rd_idx];

                if (!(msg->flags & LAST_MSG) && !(msg->flags & KEY_IS_GHOST)) {
                        if (msg->flags & PRESSED) {
                                app_cb(msg->column, msg->row, AD_KBSCN_EVENT_PRESSED);
                                // Enable inactivity check
                                if (!checking_inactivity) {
                                        set_inactivity(true);
                                }
                        } else {
                                app_cb(msg->column, msg->row, AD_KBSCN_EVENT_RELEASED);
                        }
                }

                ad_data.msg_rd_idx++;
                ad_data.msg_rd_idx &= (BUFFER_SIZE - 1);
        }

        // activate message events in case they were disabled
        hw_kbscn_activate_msg_evt();

        return 0;
}


static void kbscn_pm_wakeup_ind_cb(bool status)
{
        int i;
        /* Remove wake up pins in active mode */
        for (i = 0; i < ad_data.init_env.num_columns; i++) {
                hw_wkup_configure_pin(GET_PORT(ad_data.columns[i]), GET_PIN(ad_data.columns[i]),
                                                                false, HW_WKUP_PIN_STATE_LOW);
        }

        hw_kbscn_init(&ad_data.init_env, &ad_data.msg_wr_idx, &ad_data.msg_rd_idx);

        /* Start scanner */
        enable_scanner(true);
}

static void kbscn_pm_sleep_cancel_cb(void)
{
        int i;

        for (i = 0; i < ad_data.init_env.num_columns; i++) {
                hw_wkup_configure_pin(GET_PORT(ad_data.columns[i]), GET_PIN(ad_data.columns[i]),
                                                                false, HW_WKUP_PIN_STATE_LOW);
        }

        /* Restore scanner pins */
        for (i = 0; i < ad_data.init_env.num_columns; i++) {
                hw_gpio_set_pin_function(GET_PORT(ad_data.columns[i]), GET_PIN(ad_data.columns[i]),
                                                HW_GPIO_MODE_INPUT_PULLUP, HW_GPIO_FUNC_GPIO);
        }
        for (i = 0; i < ad_data.init_env.num_rows; i++) {
                hw_gpio_set_pin_function(GET_PORT(ad_data.rows[i]), GET_PIN(ad_data.rows[i]),
                                                HW_GPIO_MODE_OUTPUT, HW_GPIO_FUNC_KB_ROW);
        }
        enable_scanner(checking_inactivity);
}

static bool kbscn_pm_prepare_for_sleep_cb(void)
{
        int i;

        /*
         * No sleep if some key is pressed or inactivity period is not finished.
         */
        if (checking_inactivity) {
                return false;
        }

        for (i = 0; i < ad_data.init_env.num_columns; i++) {
                /* Check if any column is pulled down */
                if (!hw_gpio_get_pin_status(GET_PORT(ad_data.columns[i]),
                                                                GET_PIN(ad_data.columns[i]))) {
                        return false;
                }
        }

        for (i = 0; i < ad_data.init_env.num_columns; i++) {
                /* Configure columns as wake-up pins */
                hw_wkup_configure_pin(GET_PORT(ad_data.columns[i]), GET_PIN(ad_data.columns[i]),
                                                                true, HW_WKUP_PIN_STATE_LOW);
        }

        for (i = 0; i < ad_data.init_env.num_rows; i++) {
                /* Configure rows as low outputs */
                hw_gpio_configure_pin(GET_PORT(ad_data.rows[i]), GET_PIN(ad_data.rows[i]),
                                        HW_GPIO_MODE_OUTPUT, HW_GPIO_FUNC_GPIO, false);
        }
        hw_kbscn_disable_ex(false);

        return true;
}

/* Adapter callbacks */
static const adapter_call_backs_t kbscn_pm_callbacks = {
        .ad_prepare_for_sleep = kbscn_pm_prepare_for_sleep_cb,
        .ad_sleep_canceled = kbscn_pm_sleep_cancel_cb,
        .ad_wake_up_ind = kbscn_pm_wakeup_ind_cb,
};

bool ad_kbscn_init(const ad_kbscn_config *config)
{
        struct kbscn_init_tag *init_env = &ad_data.init_env;
        int i;

        if (ad_data.matrix) {
                /* Adapter is already initialized */
                return false;
        }

        /* Matrix and callback must be provided */
        if (!config->key_matrix || !config->cb) {
                return false;
        }

        if (config->num_columns == 0 || config->num_columns > AD_KBSCN_MAX_COLUMNS) {
                return false;
        }

        if (config->num_rows == 0 || config->num_rows > AD_KBSCN_MAX_ROWS) {
                return false;
        }

        /* Clock might be up to 4M (16M/4 = 4M)*/
        if (config->clock_div < AD_KBSCN_CLOCK_DIV_4 || config->clock_div > AD_KBSCN_CLOCK_DIV_64) {
                return false;
        }

        ad_data.adapter_id = pm_register_adapter(&kbscn_pm_callbacks);

        ad_data.app_cb = config->cb;
        ad_data.matrix = config->key_matrix;
        ad_data.msg_wr_idx = 0;
        ad_data.msg_rd_idx = 0;

        /* Setup columns */
        memset(ad_data.columns, UNUSED_INDEX, sizeof(ad_data.columns));
        for (i = 0; i < config->num_columns; i++) {
                const ad_kbscn_pin_setup *pin_setup = &config->columns[i];

                if (!config->columns[i].in_use) {
                        continue;
                }

                ad_data.columns[i] = CONV_PORT_PIN_TO_INDEX((pin_setup->port << 4) | pin_setup->pin);
        }

        /* Setup rows */
        memset(ad_data.rows, UNUSED_INDEX, sizeof(ad_data.rows));
        for (i = 0; i < config->num_rows; i++) {
                const ad_kbscn_pin_setup *pin_setup = &config->rows[i];

                if (!config->rows[i].in_use) {
                        continue;
                }

                ad_data.rows[i] = CONV_PORT_PIN_TO_INDEX((pin_setup->port << 4) | pin_setup->pin);
        }

        /* Set values of hw_keyboard_scanner setup */
        init_env->columns = ad_data.columns;
        init_env->rows = ad_data.rows;
        init_env->num_columns = config->num_columns;
        init_env->num_rows = config->num_rows;
        init_env->row_scan_active_time = config->row_scan_time;
        init_env->debounce_press_time = config->debounce_press_time;
        init_env->debounce_release_time = config->debounce_release_time;
        init_env->clock_div = config->clock_div;
        init_env->msg_evt = true;
        init_env->fifo_evt = true;
        init_env->fifo_over_cb = kbscn_fifo_over_cb;
        init_env->fifo_under_cb = kbscn_fifo_under_cb;
        init_env->msg_cb = kbscn_msg_cb;
        init_env->msg_buf = ad_data.msg_buf;
        init_env->msg_buf_sz = BUFFER_SIZE;
        init_env->inactive_time = (config->inactive_time)?(config->inactive_time):1;
        init_env->inactive_evt = true;
        init_env->inactivity_cb = kbscn_inactivity_cb;

        /* Use wkup callback for enabling kbd scanner only when waking up from kbd IO */
        hw_wkup_init(NULL);
#if dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A
        hw_wkup_set_counter_threshold(1);
#endif
#if dg_configLATCH_WKUP_SOURCE == 0
        hw_wkup_set_debounce_time(1);
#endif
        hw_wkup_register_interrupt(kbscn_wkup_intr_cb, 1);

        /* Init kbd scan */
        hw_kbscn_init(&ad_data.init_env, &ad_data.msg_wr_idx, &ad_data.msg_rd_idx);
        /* Start scanner, block sleep for inactivity time period */
        set_inactivity(true);
        hw_kbscn_enable_ex(true);
        hw_kbscn_activate_msg_evt();

        return true;
}

void ad_kbscn_cleanup(void)
{
        hw_kbscn_disable_ex(true);
        hw_kbscn_disable_inactivity();
        pm_unregister_adapter(ad_data.adapter_id);

        ad_data.matrix = NULL;
}

#endif /* dg_configKEYBOARD_SCANNER_ADAPTER */
