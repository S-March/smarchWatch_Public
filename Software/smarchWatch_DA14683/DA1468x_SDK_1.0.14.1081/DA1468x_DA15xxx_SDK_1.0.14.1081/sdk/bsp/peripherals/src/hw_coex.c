/**
 ****************************************************************************************
 *
 * @file hw_coex.c
 *
 * @brief Arbiter driver.
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/**
\addtogroup BSP
\{
\addtogroup DEVICES
\{
\addtogroup COEX
\{
*/

#if dg_configUSE_HW_COEX

#include <stdint.h>
#include "sdk_defs.h"
#include "hw_coex.h"
#include "hw_gpio.h"
#include <stdint.h>

#if (dg_configSYSTEMVIEW)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif

/***************************************************************************************************
 * Local variables
 **************************************************************************************************/

#if dg_configCOEX_ENABLE_STATS == 1
hw_coex_stats_t hw_coex_stats __RETAINED;
#endif

#if dg_configCOEX_ENABLE_CONFIG == 1
/** True if the arbiter has been initialized. */
static bool hw_coex_initialized __RETAINED;

/** Arbiter configuration. */
static hw_coex_config_t hw_coex_config __RETAINED;
#endif

#if dg_configCOEX_ENABLE_STATS == 1
/** Enables arbiter table debug signals via GPIO port 3 (see below).*/
//#define HW_COEX_DEBUG
#endif

#ifdef HW_COEX_DEBUG
#define HW_COEX_DEBUG_GPIO_CLOCK        HW_GPIO_PORT_4, HW_GPIO_PIN_4
#define HW_COEX_DEBUG_GPIO_BIT0         HW_GPIO_PORT_4, HW_GPIO_PIN_5
#define HW_COEX_DEBUG_GPIO_BIT1         HW_GPIO_PORT_4, HW_GPIO_PIN_6
#define HW_COEX_DEBUG_GPIO_BIT2         HW_GPIO_PORT_4, HW_GPIO_PIN_7
#define HW_COEX_DEBUG_GPIO_BIT3         HW_GPIO_PORT_3, HW_GPIO_PIN_6
#if dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A
#define HW_COEX_DEBUG_GPIO_BIT4         HW_GPIO_PORT_3, HW_GPIO_PIN_7
#endif

static inline void hw_coex_debug_gpio_setup(void) {
        hw_gpio_set_pin_function(HW_COEX_DEBUG_GPIO_CLOCK, HW_GPIO_MODE_OUTPUT,
                HW_GPIO_FUNC_GPIO);
        hw_gpio_set_pin_function(HW_COEX_DEBUG_GPIO_BIT0, HW_GPIO_MODE_OUTPUT,
                HW_GPIO_FUNC_GPIO);
        hw_gpio_set_pin_function(HW_COEX_DEBUG_GPIO_BIT1, HW_GPIO_MODE_OUTPUT,
                HW_GPIO_FUNC_GPIO);
        hw_gpio_set_pin_function(HW_COEX_DEBUG_GPIO_BIT2, HW_GPIO_MODE_OUTPUT,
                HW_GPIO_FUNC_GPIO);
        hw_gpio_set_pin_function(HW_COEX_DEBUG_GPIO_BIT3, HW_GPIO_MODE_OUTPUT,
                HW_GPIO_FUNC_GPIO);
#if dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A
        hw_gpio_set_pin_function(HW_COEX_DEBUG_GPIO_BIT4, HW_GPIO_MODE_OUTPUT,
                HW_GPIO_FUNC_GPIO);
#endif
        hw_gpio_set_active(HW_COEX_DEBUG_GPIO_CLOCK);
}

static inline void hw_coex_debug_decision_update(uint16_t stat_reg)
{
        hw_gpio_set_inactive(HW_COEX_DEBUG_GPIO_CLOCK);
        stat_reg & (1 << 0) ? hw_gpio_set_active(HW_COEX_DEBUG_GPIO_BIT0) :
                hw_gpio_set_inactive(HW_COEX_DEBUG_GPIO_BIT0);
        stat_reg & (1 << 1) ? hw_gpio_set_active(HW_COEX_DEBUG_GPIO_BIT1) :
                hw_gpio_set_inactive(HW_COEX_DEBUG_GPIO_BIT1);
        stat_reg & (1 << 2) ? hw_gpio_set_active(HW_COEX_DEBUG_GPIO_BIT2) :
                hw_gpio_set_inactive(HW_COEX_DEBUG_GPIO_BIT2);
        stat_reg & (1 << 3) ? hw_gpio_set_active(HW_COEX_DEBUG_GPIO_BIT3) :
                hw_gpio_set_inactive(HW_COEX_DEBUG_GPIO_BIT3);
#if dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A
        stat_reg & (1 << 4) ? hw_gpio_set_active(HW_COEX_DEBUG_GPIO_BIT4) :
                hw_gpio_set_inactive(HW_COEX_DEBUG_GPIO_BIT4);
#endif
        hw_gpio_set_active(HW_COEX_DEBUG_GPIO_CLOCK);
}
#endif

/***************************************************************************************************
 * Static (internal) functions
 **************************************************************************************************/
#if dg_configCOEX_ENABLE_CONFIG == 1

/**
 * Initializes arbiter configuration.
 *
 * \note In order to avoid inconsistencies with the hardware, the initialization values here MUST
 * match the reset values of the hardware.
 */
static inline void hw_coex_config_init(void)
{
        int i;
        hw_coex_config.ctrl = HW_COEX_CTRL_RESET;
        hw_coex_config.ble_pti = 0;
        hw_coex_config.ftdf_pti = 0;
        hw_coex_config.pri[0].mac = HW_COEX_MAC_TYPE_EXT;
        hw_coex_config.pri[0].pti = 0;
        hw_coex_config.pri[1].mac = HW_COEX_MAC_TYPE_BLE;
        hw_coex_config.pri[1].pti = 0;
        hw_coex_config.pri[2].mac = HW_COEX_MAC_TYPE_FTDF;
        hw_coex_config.pri[2].pti = 0;
        for (i = 3; i < HW_COEX_PTI_TABLE_SIZE; i++) {
                hw_coex_config.pri[i].mac = HW_COEX_MAC_TYPE_NONE;
                hw_coex_config.pri[i].pti = 0;
        }
}

/**
 * Pauses arbiter. Assumes that power domain is on.
 *
 * \return true if arbiter had been previously activated.
 */
__RETAINED_CODE static bool hw_coex_pause(void)
{
        if (COEX->COEX_CTRL_REG & REG_MSK(COEX, COEX_CTRL_REG, PRGING_ARBITER)) {
                return false;
        }
        REG_SET_BIT(COEX, COEX_CTRL_REG, PRGING_ARBITER);
        /*
         * This bit is updated with the COEX_CLK, so depending on the relationship between the PCLK
         * and COEX_CLK periods a write operation to this bit may be effective in more than one
         * PCLK clock cycles, e.g. when the COEX_CLK rate is slower than the PCLK.
         *
         * So we verify that the bit has been set.
         */
        while (!(COEX->COEX_CTRL_REG & REG_MSK(COEX, COEX_CTRL_REG, PRGING_ARBITER)));
        return true;
}

/**
 * Resumes arbiter. Assumes that power domain is on.
 */
__attribute__((always_inline)) static inline void hw_coex_resume(void)
{
        // Resume Arbiter
        REG_CLR_BIT(COEX, COEX_CTRL_REG, PRGING_ARBITER);
}

/**
 * Writes configuration to the arbiter HW.
 */
__RETAINED_CODE static void hw_coex_config_write(void)
{
        volatile uint16_t *pri_reg_address = &(COEX->COEX_PRI1_REG);
        uint16_t pri_reg_value;
        uint16_t reg;
        bool was_active;
        int i;

        COEX->COEX_FTDF_PTI_REG = hw_coex_config.ftdf_pti;

        COEX->COEX_BLE_PTI_REG = hw_coex_config.ble_pti;

        /*********** Write COEX_CTRL_REG **********/
        reg = 0; /* Warning! Code here assumes that reset value of COEX_CTRL_REG is 0. */

        if (hw_coex_config.ctrl & HW_COEX_CTRL_BIT_FTDF_FORCE_CCA ) {
                REG_SET_FIELD(COEX, COEX_CTRL_REG, SEL_FTDF_CCA, reg, 1);
        }

        if (hw_coex_config.ctrl & HW_COEX_CTRL_BIT_IGNORE_BLE) {
                REG_SET_FIELD(COEX, COEX_CTRL_REG, IGNORE_BLE, reg, 1);
        }

        if (hw_coex_config.ctrl & HW_COEX_CTRL_BIT_IGNORE_FTDF) {
                REG_SET_FIELD(COEX, COEX_CTRL_REG, IGNORE_FTDF, reg, 1);
        }

        if (hw_coex_config.ctrl & HW_COEX_CTRL_BIT_IGNORE_EXT) {
                REG_SET_FIELD(COEX, COEX_CTRL_REG, IGNORE_EXT, reg, 1);
        }

#if dg_configBLACK_ORCA_IC_REV != BLACK_ORCA_IC_REV_A
        if (hw_coex_config.ctrl & HW_COEX_CTRL_BIT_FTDF_PTI_AUTO) {
                REG_SET_FIELD(COEX, COEX_CTRL_REG, SEL_FTDF_PTI, reg, 1);
        }

        if (hw_coex_config.ctrl & HW_COEX_CTRL_BIT_BLE_PTI_AUTO) {
                REG_SET_FIELD(COEX, COEX_CTRL_REG, SEL_BLE_PTI, reg, 1);
        }
#if dg_configCOEX_ENABLE_STATS == 1
        if (hw_coex_config.ctrl & HW_COEX_CTRL_BIT_BLE_TXRX_MON_ALL) {
                REG_SET_FIELD(COEX, COEX_CTRL_REG, TXRX_MON_BLE_ALL, reg, 1);
        }

        if (hw_coex_config.ctrl & HW_COEX_CTRL_BIT_FTDF_TXRX_MON_ALL) {
                REG_SET_FIELD(COEX, COEX_CTRL_REG, TXRX_MON_FTDF_ALL, reg, 1);
        }

        if (hw_coex_config.ctrl & HW_COEX_CTRL_BIT_DECISION_SW_ALL) {
                REG_SET_FIELD(COEX, COEX_CTRL_REG, DECISION_SW_ALL, reg, 1);
        }
#endif /* dg_configCOEX_ENABLE_STATS == 1 */

#endif /* dg_configBLACK_ORCA_IC_REV != BLACK_ORCA_IC_REV_A */

        COEX->COEX_CTRL_REG = reg;
        /********** Write COEX_CTRL_REG OK *************/

#if dg_configCOEX_ENABLE_STATS == 1
        /************ Write COEX_INT_MASK_REG ************/
        reg = 0; /* Warning! Code here assumes that reset value of COEX_INT_MASK_REG is 0. */

#if (dg_configBLACK_ORCA_IC_REV != BLACK_ORCA_IC_REV_A)
        if (hw_coex_config.ctrl & HW_COEX_CTRL_BIT_DECISION_SW_STATS_ENABLED) {
                REG_SET_FIELD(COEX, COEX_INT_MASK_REG, IRQ_DECISION_SW, reg, 1);
        }

        if (hw_coex_config.ctrl & HW_COEX_CTRL_BIT_BLE_TXRX_MON_ALL) {
                REG_SET_FIELD(COEX, COEX_INT_MASK_REG, IRQ_TXRX_MON, reg, 1);
        }
#else /* (dg_configBLACK_ORCA_IC_REV != BLACK_ORCA_IC_REV_A) */
        reg =
                REG_MSK(COEX, COEX_INT_MASK_REG, COEX_IRQ_ON_SMART_ACT_R)
                | REG_MSK(COEX, COEX_INT_MASK_REG, COEX_IRQ_ON_SMART_ACT_F)
                | REG_MSK(COEX, COEX_INT_MASK_REG, COEX_IRQ_ON_SMART_PRI_R)
                | REG_MSK(COEX, COEX_INT_MASK_REG, COEX_IRQ_ON_SMART_PRI_F)
                | REG_MSK(COEX, COEX_INT_MASK_REG, COEX_IRQ_ON_EXT_ACT_R)
                | REG_MSK(COEX, COEX_INT_MASK_REG, COEX_IRQ_ON_EXT_ACT_F)
                | REG_MSK(COEX, COEX_INT_MASK_REG, COEX_IRQ_ON_FTDF_ACTIVE_R)
                | REG_MSK(COEX, COEX_INT_MASK_REG, COEX_IRQ_ON_FTDF_ACTIVE_F)
                | REG_MSK(COEX, COEX_INT_MASK_REG, COEX_IRQ_ON_BLE_ACTIVE_R)
                | REG_MSK(COEX, COEX_INT_MASK_REG, COEX_IRQ_ON_BLE_ACTIVE_F)
                | REG_MSK(COEX, COEX_INT_MASK_REG, COEX_IRQ_ON_RADIO_BUSY_R)
                | REG_MSK(COEX, COEX_INT_MASK_REG, COEX_IRQ_ON_RADIO_BUSY_F)
                | REG_MSK(COEX, COEX_INT_MASK_REG, COEX_IRQ_ON_CLOSING_BRK)
                | REG_MSK(COEX, COEX_INT_MASK_REG, COEX_IRQ_ON_START_MID)
                | REG_MSK(COEX, COEX_INT_MASK_REG, COEX_IRQ_ON_DECISION_SW)
                | REG_MSK(COEX, COEX_INT_MASK_REG, COEX_IRQ_MASK);
#endif /* (dg_configBLACK_ORCA_IC_REV != BLACK_ORCA_IC_REV_A) */

        COEX->COEX_INT_MASK_REG = reg;

        /************ Write COEX_INT_MASK_REG OK *************/

#endif /* dg_configCOEX_ENABLE_STATS == 1 */
        /************* Write priority table. *************/
        was_active = hw_coex_pause();
        /* Program priorities */
        /* WARNING! The following loop makes the assumption that COEX priority registers
         * are in a contiguous register block.
         */
        pri_reg_value = 0;
        for (i = 0; i < HW_COEX_PTI_TABLE_SIZE; i++) {
                REG_SET_FIELD(COEX, COEX_PRI1_REG, COEX_PRI_PTI, pri_reg_value,
                        hw_coex_config.pri[i].pti);
                REG_SET_FIELD(COEX, COEX_PRI1_REG, COEX_PRI_MAC, pri_reg_value,
                        hw_coex_config.pri[i].mac);
                *pri_reg_address++ = pri_reg_value;
        }

        if (was_active) {
                hw_coex_resume();
        }

        /************* Write priority table OK *************/
}
#endif /* dg_configCOEX_ENABLE_CONFIG == 1 */
/**
 * Checks whether COEX power domain is on and ensures config is initialized.
 */
__RETAINED_CODE static bool hw_coex_get_status(void)
{
        bool pd_and_clk_on;
        GLOBAL_INT_DISABLE();

        // Check if Radio PD is active and RFCU clock is running.
        pd_and_clk_on = !REG_GETF(CRG_TOP, PMU_CTRL_REG, RADIO_SLEEP) &&
                REG_GETF(CRG_TOP, CLK_RADIO_REG, RFCU_ENABLE) ? true : false;

        GLOBAL_INT_RESTORE();
        return pd_and_clk_on;
}

/***************************************************************************************************
 * API function definitions
 **************************************************************************************************/
#if dg_configCOEX_ENABLE_CONFIG == 1
void hw_coex_init(void)
{
        ASSERT_WARNING(hw_coex_initialized == false);
        hw_coex_config_init();
        hw_coex_initialized = true;
#if dg_configCOEX_ENABLE_STATS == 1
        NVIC_ClearPendingIRQ(COEX_IRQn);
        NVIC_EnableIRQ(COEX_IRQn);
#endif
}

void hw_coex_set_ignore_mac(HW_COEX_MAC_TYPE mac_type, bool set)
{
        __typeof__(COEX->COEX_CTRL_REG) reg_mask = 0;
        __typeof__(hw_coex_config.ctrl) config_mask = 0;

        ASSERT_WARNING(hw_coex_initialized == true);

        switch (mac_type) {
        case HW_COEX_MAC_TYPE_BLE:
                reg_mask = REG_MSK(COEX, COEX_CTRL_REG, IGNORE_BLE);
                config_mask = HW_COEX_CTRL_BIT_IGNORE_BLE;
                break;

        case HW_COEX_MAC_TYPE_FTDF:
                reg_mask = REG_MSK(COEX, COEX_CTRL_REG, IGNORE_FTDF);
                config_mask = HW_COEX_CTRL_BIT_IGNORE_FTDF;
                break;

        case HW_COEX_MAC_TYPE_EXT:
                reg_mask = REG_MSK(COEX, COEX_CTRL_REG, IGNORE_EXT);
                config_mask = HW_COEX_CTRL_BIT_IGNORE_EXT;
                break;
        case HW_COEX_MAC_TYPE_NONE:
                // Invalid argument.
                ASSERT_WARNING(0);
                break;
        }

        if (set) {
                hw_coex_config.ctrl |= config_mask;
        } else {
                hw_coex_config.ctrl &= ~config_mask;
        }

        GLOBAL_INT_DISABLE();
        /* Make sure coex PD and clk are on */
        if (hw_coex_get_status()) {
                if (set) {
                        COEX->COEX_CTRL_REG |= reg_mask;
                } else {
                        COEX->COEX_CTRL_REG &= ~reg_mask;
                }
        }
        GLOBAL_INT_RESTORE();
}


void hw_coex_update_ftdf_pti(hw_coex_pti_t ftdf_pti, hw_coex_pti_t * prev_ftdf_pti,
        bool force_decision)
{
        ASSERT_WARNING(hw_coex_initialized == true);
        if (prev_ftdf_pti) {
                *prev_ftdf_pti = hw_coex_config.ftdf_pti;
        }
        hw_coex_config.ftdf_pti = ftdf_pti;
        // Check if we can apply right now.
        GLOBAL_INT_DISABLE();
        if (hw_coex_get_status()) {
                COEX->COEX_FTDF_PTI_REG = hw_coex_config.ftdf_pti;
                if (force_decision) {
                        REG_SETF(COEX, COEX_CTRL_REG, IGNORE_FTDF, 1);
                        while (!REG_GETF(COEX, COEX_CTRL_REG, IGNORE_FTDF));
                        REG_SETF(COEX, COEX_CTRL_REG, IGNORE_FTDF, 0);
                }
        }
        GLOBAL_INT_RESTORE();
}

void hw_coex_update_ble_pti(hw_coex_pti_t ble_pti, hw_coex_pti_t * prev_ble_pti,
        bool force_decision)
{
        ASSERT_WARNING(hw_coex_initialized == true);
        if (prev_ble_pti) {
                *prev_ble_pti = hw_coex_config.ble_pti;
        }
        hw_coex_config.ble_pti = ble_pti;
        // Check if we can apply right now.
        GLOBAL_INT_DISABLE();
        if (hw_coex_get_status()) {
                COEX->COEX_BLE_PTI_REG = hw_coex_config.ble_pti;
                if (force_decision) {
                        REG_SETF(COEX, COEX_CTRL_REG, IGNORE_BLE, 1);
                        while (!REG_GETF(COEX, COEX_CTRL_REG, IGNORE_BLE));
                        REG_SETF(COEX, COEX_CTRL_REG, IGNORE_BLE, 0);
                }
        }
        GLOBAL_INT_RESTORE();
}

void hw_coex_config_set(const hw_coex_config_t *config)
{
        ASSERT_WARNING(hw_coex_initialized == true);
        hw_coex_config = *config;
        GLOBAL_INT_DISABLE();
        /* Apply to HW immediately if active, else it will be applied when activated. */
        if (hw_coex_get_status()) {
                hw_coex_config_write();
        }
        GLOBAL_INT_RESTORE();
}

void hw_coex_config_get(hw_coex_config_t *config)
{
        ASSERT_WARNING(hw_coex_initialized == true);
        *config = hw_coex_config;
}

void hw_coex_config_set_priority(int index, const hw_coex_priority_t * pri,
        hw_coex_priority_t * prev_pri)
{
        uint16_t pri_reg_value;
        bool was_active;

        ASSERT_WARNING(hw_coex_initialized == true);

        if (prev_pri) {
                *prev_pri = hw_coex_config.pri[index];
        }

        hw_coex_config.pri[index] = *pri;

        GLOBAL_INT_DISABLE();
        if (hw_coex_get_status()) {
                was_active = hw_coex_pause();
                pri_reg_value = 0;
                REG_SET_FIELD(COEX, COEX_PRI1_REG, COEX_PRI_PTI, pri_reg_value, pri->pti);
                REG_SET_FIELD(COEX, COEX_PRI1_REG, COEX_PRI_MAC, pri_reg_value, pri->mac);
                *(&(COEX->COEX_PRI1_REG) + index) = pri_reg_value;
                if (was_active) {
                        hw_coex_resume();
                }
        }
        GLOBAL_INT_RESTORE();
}

void hw_coex_config_reset(void)
{
        ASSERT_WARNING(hw_coex_initialized == true);
        hw_coex_config_init();
        GLOBAL_INT_DISABLE();
        /* Apply to HW immediately if active, else it will be applied when activated. */
        if (hw_coex_get_status()) {
                hw_coex_config_write();
        }
        GLOBAL_INT_RESTORE();
}
#endif  /* dg_configCOEX_ENABLE_CONFIG == 1 */

__RETAINED_CODE void hw_coex_apply_config(void)
{
#if dg_configCOEX_ENABLE_CONFIG == 1
        ASSERT_WARNING(hw_coex_initialized == true);
        GLOBAL_INT_DISABLE();
        /* Make sure coex PD and clk are on */
        ASSERT_WARNING(hw_coex_get_status());

        hw_coex_config_write();

        GLOBAL_INT_RESTORE();
#if dg_configCOEX_ENABLE_DIAGS == 1
        hw_coex_diag_enable(dg_configCOEX_DIAGS_MODE);
#endif
#endif
#ifdef HW_COEX_DEBUG
        hw_coex_debug_gpio_setup();
#endif
}
/**
 * \brief Interrupt Handler of the Arbiter module.
 *
 * \return void
 *
 */
void COEX_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

#if dg_configCOEX_ENABLE_STATS == 1
        uint16 int_stat_reg;

        while (1) {
                int_stat_reg = COEX->COEX_INT_STAT_REG;
#ifdef HW_COEX_DEBUG
                uint16_t stat_reg = COEX->COEX_STAT_REG;
#endif
                if (int_stat_reg == 0) {
                        break;
                }
#if dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A
                if (int_stat_reg & REG_MSK(COEX, COEX_INT_STAT_REG, COEX_IRQ_ON_SMART_ACT_R)) {
                        hw_coex_stats.smart_act_r++;
                }

                if (int_stat_reg & REG_MSK(COEX, COEX_INT_STAT_REG, COEX_IRQ_ON_SMART_ACT_F)) {
                        hw_coex_stats.smart_act_f++;
                }

                if (int_stat_reg & REG_MSK(COEX, COEX_INT_STAT_REG, COEX_IRQ_ON_SMART_PRI_R)) {
                        hw_coex_stats.smart_pri_r++;
                }

                if (int_stat_reg & REG_MSK(COEX, COEX_INT_STAT_REG, COEX_IRQ_ON_SMART_PRI_F)) {
                        hw_coex_stats.smart_pri_f++;
                }

                if (int_stat_reg & REG_MSK(COEX, COEX_INT_STAT_REG, COEX_IRQ_ON_EXT_ACT_R)) {
                        hw_coex_stats.ext_act_r++;
                }

                if (int_stat_reg & REG_MSK(COEX, COEX_INT_STAT_REG, COEX_IRQ_ON_EXT_ACT_F)) {
                        hw_coex_stats.ext_act_f++;
                }

                if (int_stat_reg & REG_MSK(COEX, COEX_INT_STAT_REG, COEX_IRQ_ON_FTDF_ACTIVE_R)) {
                        hw_coex_stats.ftdf_active_r++;
                }

                if (int_stat_reg & REG_MSK(COEX, COEX_INT_STAT_REG, COEX_IRQ_ON_FTDF_ACTIVE_F)) {
                        hw_coex_stats.ftdf_active_f++;
                }

                if (int_stat_reg & REG_MSK(COEX, COEX_INT_STAT_REG, COEX_IRQ_ON_BLE_ACTIVE_R)) {
                        hw_coex_stats.ble_active_r++;
                }

                if (int_stat_reg & REG_MSK(COEX, COEX_INT_STAT_REG, COEX_IRQ_ON_BLE_ACTIVE_F)) {
                        hw_coex_stats.ble_active_f++;
                }

                if (int_stat_reg & REG_MSK(COEX, COEX_INT_STAT_REG, COEX_IRQ_ON_RADIO_BUSY_R)) {
                        hw_coex_stats.radio_busy_r++;
                }

                if (int_stat_reg & REG_MSK(COEX, COEX_INT_STAT_REG, COEX_IRQ_ON_RADIO_BUSY_F)) {
                        hw_coex_stats.radio_busy_f++;
                }

                if (int_stat_reg & REG_MSK(COEX, COEX_INT_STAT_REG, COEX_IRQ_ON_CLOSING_BRK)) {
                        hw_coex_stats.closing_brk++;
                }

                if (int_stat_reg & REG_MSK(COEX, COEX_INT_STAT_REG, COEX_IRQ_ON_START_MID)) {
                        hw_coex_stats.start_mid++;
                }

                if (int_stat_reg & REG_MSK(COEX, COEX_INT_STAT_REG, COEX_IRQ_ON_DECISION_SW)) {
                        hw_coex_stats.decision_sw++;
#ifdef HW_COEX_DEBUG
                        hw_coex_debug_decision_update(stat_reg);
#endif
                }
#else
                if (int_stat_reg & REG_MSK(COEX, COEX_INT_STAT_REG, IRQ_TXRX_MON)) {
                        uint8_t ptr = REG_GET_FIELD(COEX, COEX_INT_STAT_REG, TXRX_MON_PTR,
                                int_stat_reg);
                        if (ptr != 0) {
                                if (int_stat_reg & REG_MSK(COEX, COEX_INT_STAT_REG,
                                        TXRX_MON_OVWR)) {
                                        hw_coex_stats.txrx_mon.overflow++;
                                }
                                if (int_stat_reg & REG_MSK(COEX, COEX_INT_STAT_REG, TXRX_MON_TX)) {
                                        if (int_stat_reg & REG_MSK(COEX, COEX_INT_STAT_REG,
                                                TXRX_MON_PASSED)) {
                                                hw_coex_stats.txrx_mon.ptr[ptr - 1].tx_passed++;
                                        } else {
                                                hw_coex_stats.txrx_mon.ptr[ptr - 1].tx_masked++;
                                        }
                                } else {
                                        if (int_stat_reg & REG_MSK(COEX, COEX_INT_STAT_REG,
                                                TXRX_MON_PASSED)) {
                                                hw_coex_stats.txrx_mon.ptr[ptr - 1].rx_passed++;
                                        } else {
                                                hw_coex_stats.txrx_mon.ptr[ptr - 1].rx_masked++;
                                        }
                                }
                        }
                }

                if (int_stat_reg & REG_MSK(COEX, COEX_INT_STAT_REG, IRQ_DECISION_SW)) {
                        hw_coex_stats.decision_sw++;
#ifdef HW_COEX_DEBUG
                        hw_coex_debug_decision_update(stat_reg);
#endif
                }
#endif /* #if dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A */
        }
#endif /* dg_configCOEX_ENABLE_STATS == 1 */

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

#if dg_configIMAGE_SETUP == DEVELOPMENT_MODE && \
        dg_configCOEX_ENABLE_DIAGS == 1
void hw_coex_diag_enable(HW_COEX_DIAG_MODE diag_mode)
{
#if dg_configBLACK_ORCA_IC_REV != BLACK_ORCA_IC_REV_A
        /* Make sure coex PD and clk are on */
        ASSERT_WARNING(hw_coex_get_status());
        /* To use P1_1 or P2_2 in GPIO mode, USBPAD_REG[USBPAD_EN] must be set. */
        REG_SET_BIT(CRG_PER, USBPAD_REG, USBPAD_EN);

        ASSERT_ERROR(diag_mode <= HW_COEX_DIAG_MODE_3);

        /* Bit 2 */
        if (diag_mode == HW_COEX_DIAG_MODE_3) {
                hw_gpio_set_pin_function(HW_GPIO_PORT_2, HW_GPIO_PIN_2, HW_GPIO_MODE_OUTPUT,
                        HW_GPIO_FUNC_BLE_DIAG);
        }

        /* Bits [5:3] */
        hw_gpio_set_pin_function(HW_GPIO_PORT_1, HW_GPIO_PIN_0, HW_GPIO_MODE_OUTPUT,
                HW_GPIO_FUNC_BLE_DIAG);
        hw_gpio_set_pin_function(HW_GPIO_PORT_1, HW_GPIO_PIN_1, HW_GPIO_MODE_OUTPUT,
                HW_GPIO_FUNC_BLE_DIAG);
        hw_gpio_set_pin_function(HW_GPIO_PORT_1, HW_GPIO_PIN_2, HW_GPIO_MODE_OUTPUT,
                HW_GPIO_FUNC_BLE_DIAG);

        if (diag_mode > HW_COEX_DIAG_MODE_1) {
                /* Add GPIOs for channels 6 and 7. */
                hw_gpio_set_pin_function(HW_GPIO_PORT_1, HW_GPIO_PIN_3, HW_GPIO_MODE_OUTPUT,
                        HW_GPIO_FUNC_BLE_DIAG);
                hw_gpio_set_pin_function(HW_GPIO_PORT_2, HW_GPIO_PIN_3, HW_GPIO_MODE_OUTPUT,
                        HW_GPIO_FUNC_BLE_DIAG);
        }

        REG_SETF(COEX, COEX_CTRL_REG, SEL_COEX_DIAG, diag_mode);
#endif
}

#endif

#endif /* dg_configUSE_HW_COEX */
/**
\}
\}
\}
*/
