/**
 ****************************************************************************************
 *
 * @file sys_clock_mgr.c
 *
 * @brief Clock Manager
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
 \addtogroup SYSTEM
 \{
 \addtogroup Clock-Power_Manager
 \{
 */

#include <stdint.h>
#include <stdio.h>
#include "sdk_defs.h"
#include <stdbool.h>
#include "osal.h"
#include "timers.h"
#include "event_groups.h"
#include "hw_cpm.h"
#include "hw_otpc.h"
#include "hw_qspi.h"
#include "sys_tcs.h"
#include "sys_clock_mgr.h"
#include "sys_power_mgr.h"
#include "qspi_automode.h"

#if defined(CONFIG_USE_BLE)
#include "ad_ble.h"
#endif

#if (CPM_USE_RCX_DEBUG == 1)
#include "logging.h"
#endif

#if (dg_configSYSTEMVIEW)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif

#define BIT_1                           1       // XTAL16M availability
#define BIT_2                           2       // LP clock availability

#define RCX_MIN_HZ                      450
#define RCX_MAX_HZ                      550
#define RCX_MIN_TICK_CYCLES             17
#define RCX_MAX_TICK_CYCLES             30

/* ~4.4 msec for the 1st calibration. This is the maximum allowed value when the 96MHz clock is
 * used. It can be increased when the sys_clk has lower frequency (i.e. multiplied by 2 for 48MHz,
 * 3 for 32MHz and 6 for 16MHz). The bigger it is, the longer it takes to complete the power-up
 * sequence. */
#define RCX_CALIBRATION_CYCLES_PUP      44
/* ~2.5 msec for any subsequent calibration. */
#define RCX_CALIBRATION_CYCLES_WUP      25

/* Total calibration time = N*4.5 msec. Increase N to get a better estimation of the frequency of
 * RCX. */
#define RCX_REPEAT_CALIBRATION_PUP      10

/* Bit field to trigger the RCX Calibration task to start calibration. */
#define RCX_DO_CALIBRATION              1

/*
 * Global and / or retained variables
 */
PRIVILEGED_DATA sys_clk_t cm_sysclk;
PRIVILEGED_DATA ahb_div_t cm_ahbclk;
PRIVILEGED_DATA static apb_div_t cm_apbclk;
PRIVILEGED_DATA static OS_MUTEX xSemaphoreCM;
PRIVILEGED_DATA static OS_EVENT_GROUP xEventGroupCM_xtal16_isr;
PRIVILEGED_DATA static OS_TIMER xLPSettleTimer;

// dg_configUSE_LP_CLK == LP_CLK_RCX
PRIVILEGED_DATA uint16_t rcx_clock_hz;
PRIVILEGED_DATA uint32_t rcx_clock_hz_acc;                      // Accurate RCX freq (1/RCX_ACCURACY_LEVEL accuracy)
PRIVILEGED_DATA uint8_t rcx_tick_period;                        // # of cycles in 1 tick
PRIVILEGED_DATA uint16_t rcx_tick_rate_hz;
PRIVILEGED_DATA uint32_t rcx_clock_period;                      // usec multiplied by 1024 * 1024
PRIVILEGED_DATA uint32_t ble_slot_duration_in_rcx;              // cycles multiplied by 1000000
PRIVILEGED_DATA OS_TASK xRCXCalibTaskHandle;

const uint64_t rcx_period_dividend = 1048576000000;             // 1024 * 1024 * 1000000;

bool cm_rcx_calibration_is_on = false;

/*
 * Local variables
 */
static sys_clk_t cm_sys_clk_next;
static ahb_div_t cm_ahb_clk_next;
static volatile bool cm_xtal16m_settled = false;

static const uint8_t cpu_clk_configuration[4][6] = {
                { (uint8_t)sysclk_RC16,    16,  8,  4,  2, 1 },         // RC16
#if (dg_configEXT_CRYSTAL_FREQ == EXT_CRYSTAL_IS_16M)
                { (uint8_t)sysclk_XTAL16M, 16,  8,  4,  2, 1 },         // XTAL16M
#else
                { (uint8_t)sysclk_XTAL32M, 32, 16,  8,  4, 2 },         // XTAL32M
#endif
                { (uint8_t)sysclk_PLL48,   48, 24, 12,  6, 3 },         // PLL48
                { (uint8_t)sysclk_PLL96,   96, 48, 24, 12, 6 },         // PLL96
                };

/*
 * Forward declarations
 */
static void switch_to_xtal16_safe(void);
static bool __cm_sys_clk_set(sys_clk_t type);
static void __cm_apb_set_clock_divider(apb_div_t div);
static bool __cm_ahb_set_clock_divider(ahb_div_t div);
static void cm_sys_enable_xtal16m(sys_clk_t type);

/*
 * Function definitions
 */

/**
 * \brief Adjust OTP access timings according to the AHB clock frequency.
 *
 * \return void
 *
 * \warning In mirrored mode, the OTP access timings are left unchanged since the system is put to
 *          sleep using the RC16 clock and the AHB divider set to 1, which are the same settings
 *          that the system runs after a power-up or wake-up!
 *
 */
__RETAINED_CODE static void cm_adjust_otp_access_timings(void)
{
        uint32_t clk_freq;

        if (hw_otpc_is_active()) {
                clk_freq = 16 / (1 << cm_ahb_clk_next);
                if (cm_sys_clk_next != sysclk_RC16) {
                        clk_freq *= cm_sys_clk_next;
                }
                /* Ensure AHB clock frequency is proper for OTP access timings */
                ASSERT_WARNING((clk_freq <= 48) && (clk_freq > 0));

                hw_otpc_set_speed(hw_otpc_convert_sys_clk_mhz(clk_freq));
        }
}

/**
 * \brief Switch to RC16.
 *
 * \details Sets RC16 as the system clock.
 *
 * \return void
 *
 */
static void switch_to_rc16(void)
{
        sys_clk_t prev_sysclk = cm_sysclk;

        // fast --> slow clock switch
        hw_cpm_set_sysclk(SYS_CLK_IS_RC16);                     // Set RC16 as sys_clk
        cm_adjust_otp_access_timings();                         // Adjust OTP timings

        if (dg_configFLASH_CONNECTED_TO != FLASH_IS_NOT_CONNECTED) {
                if (prev_sysclk != sysclk_RC16 && prev_sysclk != sysclk_XTAL16M) {
                        qspi_automode_sys_clock_cfg(SYS_CLK_IS_RC16);
                }
                hw_qspi_enable_readpipe(0);                     // Disable read pipe
        }
}

/**
 * \brief Switch to XTAL16M (either 16MHz or 32MHz).
 *
 * \details Sets the XTAL16M as the system clock.
 *
 * \return void
 *
 * \warning It does not block. It assumes that the caller has made sure that the XTAL16M has
 *          settled.
 *
 */
static void switch_to_xtal16(void)
{
        sys_clk_t prev_sysclk = cm_sysclk;

        if (!hw_cpm_sysclk_is_xtal16m()) {
                if (cm_sys_clk_next > cm_sysclk) {              // slow --> fast clock switch
                        cm_adjust_otp_access_timings();         // Adjust OTP timings
                        if (dg_configFLASH_CONNECTED_TO != FLASH_IS_NOT_CONNECTED) {
                                if (prev_sysclk != sysclk_RC16) {
                                        qspi_automode_sys_clock_cfg(cm_sys_clk_next);
                                }
                        }
                        hw_cpm_set_sysclk(SYS_CLK_IS_XTAL16M);  // Set XTAL16 as sys_clk
                }
                else {                                          // fast --> slow clock switch
                        hw_cpm_set_sysclk(SYS_CLK_IS_XTAL16M);  // Set XTAL16 as sys_clk
                        cm_adjust_otp_access_timings();         // Adjust OTP timings

                        if (dg_configFLASH_CONNECTED_TO != FLASH_IS_NOT_CONNECTED) {
                                if (prev_sysclk != sysclk_XTAL16M) {
                                        qspi_automode_sys_clock_cfg(cm_sys_clk_next);
                                }
                                hw_qspi_enable_readpipe(0);     // Disable read pipe
                        }
                }
        }
}

/**
 * \brief Switch to PLL (either 48MHz or 96MHz).
 *
 * \details Waits until the PLL has locked and sets it as the system clock.
 *
 * \return void
 *
 */
static void switch_to_pll(void)
{
        if (hw_cpm_is_pll_locked() == 0) {
                hw_cpm_pll_sys_on();                            // Turn on PLL
        }

        if (cm_sys_clk_next == sysclk_PLL48) {
                hw_cpm_enable_pll_divider();                    // Enable divider (div by 2)
        }
        else {
                hw_cpm_disable_pll_divider();                   // Clear divider (div by 1)
        }

        // Slow --> fast clock switch
        cm_adjust_otp_access_timings();                         // Adjust OTP timings
        if (dg_configFLASH_CONNECTED_TO != FLASH_IS_NOT_CONNECTED) {
                qspi_automode_sys_clock_cfg(cm_sys_clk_next);
                hw_qspi_enable_readpipe(1);                     // Enable read pipe
        }
        hw_cpm_set_sysclk(SYS_CLK_IS_PLL);                      // Set PLL as sys_clk
}

/**
 * \brief The handler of the XTAL32K LP settling timer.
 *
 * \return void
 *
 */
static void vLPTimerCallback(OS_TIMER pxTimer)
{
        OS_ENTER_CRITICAL_SECTION();                            // Critical section
        if (dg_configLP_CLK_SOURCE == LP_CLK_IS_DIGITAL) {
                hw_cpm_lp_set_ext32k();
        } else if ((dg_configUSE_LP_CLK == LP_CLK_32000) || (dg_configUSE_LP_CLK == LP_CLK_32768)) {
                hw_cpm_lp_set_xtal32k();                        // Set XTAL32K as the LP clock
        }
        OS_LEAVE_CRITICAL_SECTION();                            // Exit critical section

        // Inform (blocked) Tasks about the availability of the LP clock.
        OS_EVENT_GROUP_SET_BITS(xEventGroupCM_xtal16_isr, BIT_2);

#if defined(CONFIG_USE_BLE)
        // Inform ble adapter about the availability of the LP clock.
        ad_ble_lpclock_available();
#endif

        // Stop the Timer.
        OS_TIMER_STOP(xLPSettleTimer, OS_TIMER_FOREVER);
}

/**
 * \brief Handle the indication that the XTAL16M has settled.
 *
 * \return void
 *
 */
static OS_BASE_TYPE xtal16m_is_ready(BaseType_t *xHigherPriorityTaskWoken)
{
        int i;
        adapter_call_backs_t *p_Ad;
        OS_BASE_TYPE xResult = OS_FAIL;

        DBG_SET_HIGH(CPM_USE_TIMING_DEBUG, CPMDBG_XTAL16M_SETTLED);

        cm_xtal16m_settled = true;

        // RC32K settling time may have been used. Reset to proper value for the LP clk used.
        if ((sys_tcs_xtal16m_settling_time == 0) || (dg_configUSE_LP_CLK == LP_CLK_RCX)) {
                hw_cpm_set_xtal16m_settling_time(dg_configXTAL16_SETTLE_TIME);
                if ((dg_configUSE_LP_CLK == LP_CLK_RCX) && (sys_tcs_xtal16m_settling_time != 0)) {
                        /* When RCX is used, the TCS setting of the XTALRDY_CTRL_REG is overridden.
                         * The assertion is here to indicate that a value was written in the TCS
                         * that will not be used.
                         */
                        ASSERT_WARNING(0);
                }
        } else {
                hw_cpm_set_xtal16m_settling_time(sys_tcs_xtal16m_settling_time);
        }

        if (xEventGroupCM_xtal16_isr != NULL) {
                // 1. Restore clock settings
                cm_sys_clk_sleep(false);

                // 2. Inform Adapters.
                if (adapters_wake_up_ind_called) {
                        for (i = 0; i < dg_configPM_MAX_ADAPTERS_CNT; i++) {
                                p_Ad = pm_adapters_cb[i];
                                if ((p_Ad != NULL) && (p_Ad->ad_xtal16m_ready_ind != NULL)) {
                                        p_Ad->ad_xtal16m_ready_ind();
                                }
                        }
                } else {
                        call_adapters_xtal16m_ready_ind = true;
                }

                // 3. Inform blocked Tasks
                *xHigherPriorityTaskWoken = pdFALSE;            // Must be initialized to pdFALSE

                xResult = xEventGroupSetBitsFromISR(            // Set bit1 xEventGroupCM_xtal16_isr
                                xEventGroupCM_xtal16_isr,
                                BIT_1, xHigherPriorityTaskWoken);
        }

        if (dg_configUSE_LP_CLK == LP_CLK_RCX) {
                if ((cm_sysclk > sysclk_RC16) && (cm_sysclk < sysclk_LP) && !cm_rcx_calibration_is_on) {
                        // Start calibration
                        cm_calibrate_rcx_start();
                }
        }

        DBG_SET_LOW(CPM_USE_TIMING_DEBUG, CPMDBG_XTAL16M_SETTLED);

        return xResult;
}

/**
 * \brief Calculates the optimum tick rate and the number of LP cycles (RCX) per tick.
 *
 * \param[in] freq The RCX clock frequency (in Hz).
 * \param[out] tick_period The number of LP cycles per tick.
 *
 * \return uint32_t The optimum tick rate.
 *
 */
static uint32_t get_optimum_tick_rate(uint16_t freq, uint8_t *tick_period)
{
        uint32_t optimum_rate = 0;
        uint32_t hz;
        int tick;
        int err = 65536;
        int res;

        for (tick = RCX_MIN_TICK_CYCLES; tick < RCX_MAX_TICK_CYCLES; tick++) {
                hz = 2 * freq / tick;
                if (hz & 1) {
                        hz = hz / 2 + 1;
                } else {
                        hz = hz / 2;
                }

                if ((hz >= RCX_MIN_HZ) && (hz <= RCX_MAX_HZ)) {
                        res = hz * tick * 65536 / freq;
                        res -= 65536;
                        if (res < 0) res *= -1;
                        if (res < err) {
                                err = res;
                                optimum_rate = hz;
                                *tick_period = tick;
                        }
                }
        }

        return optimum_rate;
}

/* ---------------------------------------------------------------------------------------------- */

void cm_check_xtal_startup(void)
{
#ifdef TEST_XTAL16M_KICKING
        uint32_t t1, t2;
        uint64_t rtc_start = rtc_get_fromCPM(&t1, &t2);
#endif

        if (dg_configUSE_WDOG == 0) {
                hw_watchdog_unfreeze();                 // Start watchdog
        }

        while (!hw_cpm_is_xtal16m_started()) {           // Block until XTAL16M starts
#ifdef TEST_XTAL16M_KICKING
                if (rtc_get_fromCPM(&t1, &t2) - rtc_start > 250) {
                        hw_watchdog_freeze();           // Stop watchdog

                        // hang here, to facilitate debugging
                        while (1);
                }
#endif
        }

        if (dg_configUSE_WDOG == 0) {
                hw_watchdog_freeze();                   // Stop watchdog
        }
}

void cm_clk_init_low_level(void)
{
        uint32_t cal_value;
        uint32_t hz_value = 0;
        uint32_t correction;
        uint32_t res;
        int i;

        /* The system is running using RC16 and the XTAL16M is stopped. The XTAL16 will be started
         * with its settling time properly adjusted.
         */

        NVIC_ClearPendingIRQ(XTAL16RDY_IRQn);
        NVIC_EnableIRQ(XTAL16RDY_IRQn);                         // Activate XTAL16 Ready IRQ

        // Setup DIVN
        if (dg_configEXT_CRYSTAL_FREQ == EXT_CRYSTAL_IS_16M) {
                hw_cpm_set_divn(false);                         // External crystal is 16MHz
        }
        else {
                hw_cpm_set_divn(true);                          // External crystal is 32MHz
        }

        /*
         * Low power clock
         */
        // Use the RC32K to count the XTAL16M settling time since the LP is not stable yet.
        hw_cpm_enable_rc32k();
        hw_cpm_lp_set_rc32k();

        hw_cpm_set_xtal16m_settling_time(dg_configXTAL16_SETTLE_TIME_RC32K);
        hw_cpm_enable_xtal16m();                                // Enable XTAL16M

        if (dg_configUSE_LP_CLK == LP_CLK_RCX) {
                hw_cpm_configure_rcx();                         // Configure RCX
                hw_cpm_enable_rcx();                            // Enable RCX
                hw_cpm_disable_xtal32k();                       // Disable XTAL32K
        } else if (dg_configLP_CLK_SOURCE == LP_CLK_IS_DIGITAL) {
                hw_cpm_disable_xtal32k();                       // Disable XTAL32K
                hw_cpm_configure_ext32k_pins();                 // Configure Ext32K pins
        } else if ((dg_configUSE_LP_CLK == LP_CLK_32000) || (dg_configUSE_LP_CLK == LP_CLK_32768)) {
                hw_cpm_configure_xtal32k_pins();                // Configure XTAL32K pins
                hw_cpm_configure_xtal32k();                     // Configure XTAL32K
                hw_cpm_enable_xtal32k();                        // Enable XTAL32K
        } else {
                ASSERT_WARNING(0);                              // Should not be here!
        }

        cm_check_xtal_startup();

        /*
         * Note: If the LP clock is the RCX then we have to wait for the XTAL16M to settle
         *       since we need to estimate the frequency of the RCX before continuing
         *       (calibration procedure).
         */
        if (dg_configUSE_LP_CLK == LP_CLK_RCX) {
                while (!cm_poll_xtal16m_ready());                // Wait for XTAL16M to settle
                hw_cpm_set_sysclk(SYS_CLK_IS_XTAL16M);          // Set XTAL16 as sys_clk

                // Calibrate RCX
                // 1. Run a dummy calibration to make sure the clock has settled
                hw_cpm_start_calibration(CALIBRATE_RCX, RCX_CALIBRATION_CYCLES_WUP);
                cal_value = hw_cpm_get_calibration_data();

                // 2. Run actual calibration
                for (i = 0; i < RCX_REPEAT_CALIBRATION_PUP; i++) {
                        uint64_t max_clk_count;

                        hw_cpm_start_calibration(CALIBRATE_RCX, RCX_CALIBRATION_CYCLES_PUP);
                        cal_value = hw_cpm_get_calibration_data();

                        // Process calibration results
                        max_clk_count = (uint64_t)16000000 * RCX_CALIBRATION_CYCLES_PUP * RCX_ACCURACY_LEVEL;
                        res = (uint32_t)(max_clk_count / cal_value);
                        hz_value += res;
                }
                correction = RCX_REPEAT_CALIBRATION_PUP / 2;
                rcx_clock_hz_acc = (hz_value + correction) / RCX_REPEAT_CALIBRATION_PUP;
                rcx_clock_hz = rcx_clock_hz_acc / RCX_ACCURACY_LEVEL;
                rcx_clock_period = (uint32_t)((rcx_period_dividend * RCX_ACCURACY_LEVEL) / rcx_clock_hz_acc);
                ble_slot_duration_in_rcx = (625 * rcx_clock_hz_acc) / RCX_ACCURACY_LEVEL;
                rcx_tick_rate_hz = get_optimum_tick_rate(rcx_clock_hz, &rcx_tick_period);

                hw_cpm_lp_set_rcx();                            // Set RCX as the LP clock

                hw_cpm_set_sysclk(SYS_CLK_IS_RC16);             // Set RC16 as sys_clk
        }

        if (dg_configUSE_LP_CLK != LP_CLK_RCX) {
                hw_cpm_set_recharge_period((uint16_t)dg_configSET_RECHARGE_PERIOD);
        } else {
                hw_cpm_set_recharge_period(cm_rcx_us_2_lpcycles_low_acc(dg_configSET_RECHARGE_PERIOD));
        }
}

void cm_sys_clk_init(sys_clk_t type)
{
        ASSERT_WARNING(xSemaphoreCM == NULL);                   // Called only once!

        xSemaphoreCM = xSemaphoreCreateMutex();                 // Create Mutex
        ASSERT_WARNING(xSemaphoreCM != NULL);

        xEventGroupCM_xtal16_isr = OS_EVENT_GROUP_CREATE();     // Create Event Group
        ASSERT_WARNING(xEventGroupCM_xtal16_isr != NULL);

        cm_ahbclk = cm_ahb_get_clock_divider();
        cm_apbclk = cm_apb_get_clock_divider();

        cm_sys_clk_next = type;
        cm_ahb_clk_next = cm_ahbclk;

        ASSERT_WARNING(type != sysclk_LP);                      // Not Applicable!

        if (dg_configUSE_LP_CLK == LP_CLK_RCX) {
                NVIC_SetPendingIRQ(XTAL16RDY_IRQn);             // XTAL16 has already been settled
        }

        OS_ENTER_CRITICAL_SECTION();                            // Critical section

        if (type == sysclk_RC16) {
                if (!hw_cpm_sysclk_is_rc16()) {                 // RC16 is not the System clock
                        switch_to_rc16();
                }
        }
        else {
                // Check that the user's request can be applied.
                if (type == sysclk_XTAL16M) {
                        ASSERT_WARNING(dg_configEXT_CRYSTAL_FREQ == EXT_CRYSTAL_IS_16M);
                }
                else if (type == sysclk_XTAL32M) {
                        ASSERT_WARNING(dg_configEXT_CRYSTAL_FREQ == EXT_CRYSTAL_IS_32M);
                }

                /*
                 * Note: In case that the LP clock is the XTAL32K then we
                 *       simply set the cm_sysclk to the user setting and skip waiting for the
                 *       XTAL16M to settle. In this case, the system clock will be set to the
                 *       XTAL16M (or the PLL) when the XTAL16RDY_IRQn hits. Every task or Adapter
                 *       must block until the requested system clock is available. Sleep may have to
                 *       be blocked as well.
                 */
                if (cm_poll_xtal16m_ready()) {
                        switch_to_xtal16();

                        if ((type == sysclk_PLL48) || (type == sysclk_PLL96)) {
                                switch_to_pll();
                        }
                }
        }

        cm_sysclk = type;

        OS_LEAVE_CRITICAL_SECTION();                            // Exit critical section
}

static void cm_sys_enable_xtal16m(sys_clk_t type)
{
        if (type >= sysclk_XTAL16M) {
                if (!hw_cpm_check_xtal16m_status()) {           // XTAL16M is disabled
                        hw_cpm_enable_xtal16m();                // Enable XTAL16M
                }

                cm_wait_xtal16m_ready();                        // Make sure the XTAL16M has settled
        }
}

bool cm_sys_clk_set(sys_clk_t type)
{
        bool ret;

        ASSERT_WARNING(xSemaphoreCM != NULL);
        ASSERT_WARNING(type != sysclk_LP);                      // Not Applicable!

        cm_sys_enable_xtal16m(type);

        OS_EVENT_WAIT(xSemaphoreCM, OS_EVENT_FOREVER);          // Block forever

        ret = __cm_sys_clk_set(type);

        OS_EVENT_SIGNAL(xSemaphoreCM);

        return ret;
}

static bool __cm_sys_clk_set(sys_clk_t type)
{
        bool ret = false;

        OS_ENTER_CRITICAL_SECTION();                            // Critical section

        do {
                if (type == cm_sysclk) {                        // No transition!
                        ret = true;
                        break;
                }

                // Check if SysTick is ON and if it is affected
                if (dg_configABORT_IF_SYSTICK_CLK_ERR) {
                        if (SysTick->CTRL & SysTick_CTRL_ENABLE_Msk) {
                                break;
                        }
                }

                cm_sys_clk_next = type;
                cm_ahb_clk_next = cm_ahbclk;

                // Check if transition is possible
                if (cm_sys_clk_next == sysclk_RC16) {
                        if (!hw_cpm_is_rc16_allowed()) {
                                break;                          // RC16 is not allowed!
                        }

                        // RC16 can be used as system clock
                        switch_to_rc16();
                }
                else {
                        // Check that the user's request can be applied.
                        if (cm_sys_clk_next == sysclk_XTAL16M) {
                                if (dg_configEXT_CRYSTAL_FREQ != EXT_CRYSTAL_IS_16M) {
                                        break;
                                }
                        }
                        else if (cm_sys_clk_next == sysclk_XTAL32M) {
                                if (dg_configEXT_CRYSTAL_FREQ != EXT_CRYSTAL_IS_32M) {
                                        break;
                                }
                        }

                        switch_to_xtal16();

                        if ((type == sysclk_PLL48) || (type == sysclk_PLL96)) {
                                switch_to_pll();
                        }
                }


                ret = true;
                cm_sysclk = type;
        } while (0);

        OS_LEAVE_CRITICAL_SECTION();                            // Exit critical section

        return ret;
}

void cm_apb_set_clock_divider(apb_div_t div)
{
        ASSERT_WARNING(xSemaphoreCM != NULL);

        OS_EVENT_WAIT(xSemaphoreCM, OS_EVENT_FOREVER);            // Block forever

        __cm_apb_set_clock_divider(div);

        OS_EVENT_SIGNAL(xSemaphoreCM);
}

static void __cm_apb_set_clock_divider(apb_div_t div)
{
        hw_cpm_set_pclk_div((uint32_t)div);
        cm_apbclk = div;
}

bool cm_ahb_set_clock_divider(ahb_div_t div)
{
        bool ret = true;

        ASSERT_WARNING(xSemaphoreCM != NULL);

        OS_EVENT_WAIT(xSemaphoreCM, OS_EVENT_FOREVER);          // Block forever

        ret = __cm_ahb_set_clock_divider(div);

        OS_EVENT_SIGNAL(xSemaphoreCM);

        return ret;
}

static bool __cm_ahb_set_clock_divider(ahb_div_t div)
{
        bool ret = true;
        uint32_t clk_freq;

        OS_ENTER_CRITICAL_SECTION();                            // Critical section

        do {
                if (cm_ahbclk == div) {
                        break;
                }

                clk_freq = 16 / (1 << div);
                if (cm_sysclk != sysclk_RC16) {
                        clk_freq *= cm_sysclk;
                }

                // Cannot allow AHB less than 16MHz when a MAC is active.
                if (hw_cpm_mac_is_active() && (clk_freq < 16)) {
                        ret = false;
                        break;
                }

                // Check if SysTick is ON and if it is affected
                if (dg_configABORT_IF_SYSTICK_CLK_ERR) {
                        if (SysTick->CTRL & SysTick_CTRL_ENABLE_Msk) {
                                ret = false;
                                break;
                        }
                }

                cm_ahb_clk_next = div;

                if (cm_ahbclk < div) {
                        // fast --> slow clock switch
                        hw_cpm_set_hclk_div(div);
                        cm_adjust_otp_access_timings();         // Adjust OTP timings
                }
                else {
                        // slow --> fast clock switch
                        cm_adjust_otp_access_timings();         // Adjust OTP timings
                        hw_cpm_set_hclk_div(div);
                }

                cm_ahbclk = div;

        } while (0);

        OS_LEAVE_CRITICAL_SECTION();                            // Exit critical section

        return ret;
}

bool cm_cpu_clk_set(cpu_clk_t clk)
{
        sys_clk_t new_sysclk = sysclk_RC16;
        sys_clk_t old_sysclk = cm_sysclk;
        ahb_div_t new_ahbclk = ahb_div1;
        bool ret = false;
        bool found = false;
        int i, j;

        ASSERT_WARNING(xSemaphoreCM != NULL);

        if (cm_sysclk == sysclk_RC16) {
                i = 0;
        }
        else {
                i = 1;                                          // skip RC16
        }

        for (; (i < 4) && !found; i++) {
                for (j = 1; (j < 6) && !found; j++) {
                        if (cpu_clk_configuration[i][j] == (uint32_t)clk) {
                                new_sysclk = (sys_clk_t)cpu_clk_configuration[i][0];
                                new_ahbclk = (ahb_div_t)(j - 1);
                                found = true;
                        }
                }
        }


        if (found) {
                ASSERT_WARNING(new_sysclk != sysclk_LP);        // Not Applicable!
                cm_sys_enable_xtal16m(new_sysclk);
                OS_EVENT_WAIT(xSemaphoreCM, OS_EVENT_FOREVER);
                ret = __cm_sys_clk_set(new_sysclk);

                if (ret) {
                        ret = __cm_ahb_set_clock_divider(new_ahbclk);

                        if (ret) {
                                __cm_apb_set_clock_divider(apb_div1);
                        }
                        else {
                                ASSERT_WARNING(old_sysclk != sysclk_LP);        // Not Applicable!
                                cm_sys_enable_xtal16m(old_sysclk);
                                __cm_sys_clk_set(old_sysclk);                   // Restore previous setting
                        }

                }
                OS_EVENT_SIGNAL(xSemaphoreCM);
        }

        return ret;
}

void cm_cpu_clk_set_fromISR(sys_clk_t a, ahb_div_t b)
{
        ASSERT_WARNING(a != sysclk_LP);                         // Not Applicable!
        ASSERT_WARNING(a != sysclk_RC16);                       // Not supported!

        cm_sysclk = a;
        cm_ahbclk = b;
        cm_sys_clk_sleep(false);                                // Pretend an XTAL16M settled event
}

sys_clk_t cm_sys_clk_get(void)
{
        sys_clk_t clk;

        ASSERT_WARNING(xSemaphoreCM != NULL);

        OS_EVENT_WAIT(xSemaphoreCM, OS_EVENT_FOREVER);          // Block forever

        OS_ENTER_CRITICAL_SECTION();                            // Critical section

        clk = cm_sys_clk_get_fromISR();

        OS_LEAVE_CRITICAL_SECTION();                            // Exit critical section

        OS_EVENT_SIGNAL(xSemaphoreCM);

        return clk;
}

sys_clk_t cm_sys_clk_get_fromISR(void)
{
        uint32_t hw_clk;
        sys_clk_t clk = sysclk_RC16;

        hw_clk = hw_cpm_get_sysclk();

        switch (hw_clk) {
        case SYS_CLK_IS_RC16:
                clk = sysclk_RC16;
                break;

        case SYS_CLK_IS_XTAL16M:
                if (dg_configEXT_CRYSTAL_FREQ == EXT_CRYSTAL_IS_16M) {
                        clk = sysclk_XTAL16M;
                }
                else {
                        clk = sysclk_XTAL32M;
                }
                break;

        case SYS_CLK_IS_PLL:
                if (hw_cpm_get_pll_divider_status() == 1) {
                        clk = sysclk_PLL48;
                }
                else {
                        clk = sysclk_PLL96;
                }
                break;

        case SYS_CLK_IS_LP:
                // fall-through
        default:
                ASSERT_WARNING(0);
                break;
        }

        return clk;
}

apb_div_t cm_apb_get_clock_divider(void)
{
        apb_div_t clk;

        ASSERT_WARNING(xSemaphoreCM != NULL);

        OS_EVENT_WAIT(xSemaphoreCM, OS_EVENT_FOREVER);            // Block forever

        clk = (apb_div_t)hw_cpm_get_pclk_div();

        OS_EVENT_SIGNAL(xSemaphoreCM);

        return clk;
}

ahb_div_t cm_ahb_get_clock_divider(void)
{
        ahb_div_t clk;

        ASSERT_WARNING(xSemaphoreCM != NULL);

        OS_EVENT_WAIT(xSemaphoreCM, OS_EVENT_FOREVER);          // Block forever

        OS_ENTER_CRITICAL_SECTION();                            // Critical section

        clk = (ahb_div_t)hw_cpm_get_hclk_div();

        OS_LEAVE_CRITICAL_SECTION();                            // Exit critical section

        OS_EVENT_SIGNAL(xSemaphoreCM);

        return clk;
}

cpu_clk_t cm_cpu_clk_get(void)
{
        sys_clk_t curr_sysclk = cm_sys_clk_get();
        ahb_div_t curr_ahbclk = cm_ahb_get_clock_divider();
        uint32_t clk_freq;
        cpu_clk_t clk;

        clk_freq = 16 / (1 << curr_ahbclk);
        if (cm_sysclk != sysclk_RC16) {
                clk_freq *= curr_sysclk;
        }
        clk = (cpu_clk_t)clk_freq;

        return clk;
}

cpu_clk_t cm_cpu_clk_get_fromISR(void)
{
        sys_clk_t curr_sysclk = cm_sys_clk_get_fromISR();
        ahb_div_t curr_ahbclk = cm_ahb_get_clock_divider_fromISR();
        uint32_t clk_freq;
        cpu_clk_t clk;

        clk_freq = 16 / (1 << curr_ahbclk);
        if (cm_sysclk != sysclk_RC16) {
                clk_freq *= curr_sysclk;
        }
        clk = (cpu_clk_t)clk_freq;

        return clk;
}

ahb_div_t cm_ahb_get_clock_divider_fromISR(void)
{
        return (ahb_div_t)hw_cpm_get_hclk_div();
}

apb_div_t cm_apb_get_clock_divider_fromISR(void)
{
        return (apb_div_t)hw_cpm_get_pclk_div();
}

void XTAL16RDY_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        // Disable OSC16M amplitude regulation
        hw_cpm_disable_osc16m_amp_reg();

        if (xSemaphoreCM != NULL) {
                OS_BASE_TYPE xHigherPriorityTaskWoken, xResult;

                xResult = xtal16m_is_ready(&xHigherPriorityTaskWoken);

                if (xResult != OS_FAIL) {
                        /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context
                         * switch should be requested. */
                        OS_EVENT_YIELD(xHigherPriorityTaskWoken);
                }
        }
        else {
                cm_xtal16m_settled = true;                      // Only for RCX initialization
        }

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

void cm_wait_xtal16m_ready(void)
{
        ASSERT_WARNING(xSemaphoreCM != NULL);

        if (!cm_xtal16m_settled) {
            OS_EVENT_GROUP_WAIT_BITS(xEventGroupCM_xtal16_isr,
                                BIT_1,                          // Bit 1
                                OS_EVENT_GROUP_FAIL,            // Don't clear bit after ret
                                OS_EVENT_GROUP_OK,              // Wait for all bits
                                OS_EVENT_GROUP_FOREVER);        // Block forever

                /* If we get here, XTAL16 must have settled */
                ASSERT_WARNING(cm_xtal16m_settled == true);
        }
}

void cm_calibrate_rc32k(void)
{
}

__RETAINED_CODE void cm_calibrate_rcx_start(void)
{
        hw_cpm_start_calibration(CALIBRATE_RCX, RCX_CALIBRATION_CYCLES_WUP);
        cm_rcx_calibration_is_on = true;
}

bool cm_calibrate_rcx_update(void)
{
        if (cm_rcx_calibration_is_on) {
                if (!hw_cpm_calibration_finished()) {
                        return false;
                }

                OS_TASK_NOTIFY_FROM_ISR(xRCXCalibTaskHandle, RCX_DO_CALIBRATION, OS_NOTIFY_SET_BITS);
        }

        cm_rcx_calibration_is_on = false;

        return true;
}

uint32_t cm_rcx_us_2_lpcycles(uint32_t usec)
{
        uint32_t result;

        /* Can only convert up to 4095 usec */
        ASSERT_WARNING(usec < 4096);

        result = ((usec << 20) / rcx_clock_period) + 1;

        return result;
}

uint32_t cm_rcx_us_2_lpcycles_low_acc(uint32_t usec)
{
        uint32_t result;

        result = ((1 << 20) / (rcx_clock_period / usec)) + 1;

        return result;
}

/**
 * \brief RCX Calibration Task function.
 *
 * \param [in] pvParameters ignored.
 *
 */
static void rcx_calibration_task( void *pvParameters )
{
        uint32_t ulNotifiedValue;
        OS_BASE_TYPE xResult __attribute__((unused));
        uint32_t cal_value;
        uint8_t dummy;

        while (1) {
                // Wait for the internal notifications.
                xResult = OS_TASK_NOTIFY_WAIT(0x0, OS_TASK_NOTIFY_ALL_BITS, &ulNotifiedValue,
                                                                        OS_TASK_NOTIFY_FOREVER);

                if (ulNotifiedValue & RCX_DO_CALIBRATION) {
                        uint64_t max_clk_count;

                        OS_ENTER_CRITICAL_SECTION();                            // Critical section

                        cal_value = hw_cpm_get_calibration_data();
                        max_clk_count = (uint64_t)16000000 * RCX_CALIBRATION_CYCLES_WUP * RCX_ACCURACY_LEVEL;
                        rcx_clock_hz_acc = (max_clk_count + (cal_value >> 1)) / cal_value;
                        rcx_clock_hz = rcx_clock_hz_acc / RCX_ACCURACY_LEVEL;
                        rcx_tick_rate_hz = get_optimum_tick_rate(rcx_clock_hz, &dummy);
                        rcx_clock_period = (uint32_t)((rcx_period_dividend * RCX_ACCURACY_LEVEL) / rcx_clock_hz_acc);
                        ble_slot_duration_in_rcx = (625 * (uint32_t)rcx_clock_hz_acc) / RCX_ACCURACY_LEVEL;

                        OS_LEAVE_CRITICAL_SECTION();                            // Exit critical section

#if (CPM_USE_RCX_DEBUG == 1)
                        log_printf(LOG_NOTICE, 1,
                                "clock_hz=%5d, tick_period=%3d, tick_rate_hz=%5d, clock_period=%10d, ble_slot_dur=%d\r\n",
                                rcx_clock_hz, rcx_tick_period, rcx_tick_rate_hz,
                                rcx_clock_period, ble_slot_duration_in_rcx);
#endif
                }
        }
}

void cm_lp_clk_timer_start(void)
{
        /* Start the timer.  No block time is specified, and even if one was
         it would be ignored because the RTOS scheduler has not yet been
         started. */
        if (OS_TIMER_START(xLPSettleTimer, 0) != OS_TIMER_SUCCESS) {
                // The timer could not be set into the Active state.
                OS_ASSERT(0);
        }
}

void cm_set_trim_values(void)
{
        // Handled by the system now. (Deprecated.)
}

void cm_lp_clk_init(void)
{
        ASSERT_WARNING(xSemaphoreCM != NULL);

        OS_EVENT_WAIT(xSemaphoreCM, OS_EVENT_FOREVER);            // Block forever

        xLPSettleTimer = OS_TIMER_CREATE("LPSet",
                                OS_MS_2_TICKS(dg_configINITIAL_SLEEP_DELAY_TIME),
                                OS_TIMER_FAIL,          // Run once
                                (void *) 0,             // Timer id == none
                                vLPTimerCallback);      // Call-back
        OS_ASSERT(xLPSettleTimer != NULL);

        cm_lp_clk_timer_start();

        /* In case of RCX, start the task that will handle the calibration calculations,
         * which require ~340usec@16MHz to complete. */
        if (dg_configUSE_LP_CLK == LP_CLK_RCX) {
                OS_BASE_TYPE status;

                // Create the RCX calibration task
                status = OS_TASK_CREATE("RCXcal",                       // The text name of the task.
                                        rcx_calibration_task,           // The function that implements the task.
                                        ( void * ) NULL,                // No parameter is passed to the task.
                                        configMINIMAL_STACK_SIZE * OS_STACK_WORD_SIZE,  // The size of the stack to allocate.
                                        ( tskIDLE_PRIORITY ),           // The priority assigned to the task.
                                        xRCXCalibTaskHandle);           // The task handle is required.
                OS_ASSERT(status == pdPASS);

                (void) status;                                          // To satisfy the compiler
        }

        OS_EVENT_SIGNAL(xSemaphoreCM);
}

bool cm_lp_clk_is_avail(void)
{
        ASSERT_WARNING(xSemaphoreCM != NULL);

        return (OS_EVENT_GROUP_GET_BITS(xEventGroupCM_xtal16_isr) & BIT_2);
}

__RETAINED_CODE bool cm_lp_clk_is_avail_fromISR(void)
{
        ASSERT_WARNING(xSemaphoreCM != NULL);

        return (OS_EVENT_GROUP_GET_BITS_FROM_ISR(xEventGroupCM_xtal16_isr) & BIT_2);
}

void cm_wait_lp_clk_ready(void)
{
        ASSERT_WARNING(xSemaphoreCM != NULL);

        OS_EVENT_GROUP_WAIT_BITS(xEventGroupCM_xtal16_isr,
                BIT_2,                                          // Bit 2
                OS_EVENT_GROUP_FAIL,                            // Don't clear bit after ret
                OS_EVENT_GROUP_OK,                              // Wait for all bits
                OS_EVENT_GROUP_FOREVER);                        // Block forever
}

void cm_lp_clk_wakeup(void)
{
        OS_EVENT_GROUP_CLEAR_BITS_FROM_ISR(xEventGroupCM_xtal16_isr, BIT_2);
}

/* ---------------------------------------------------------------------------------------------- */

/*
 * Functions intended to be used only by the Clock and Power Manager or in hooks.
 */
__RETAINED_CODE static void cm_apply_lowered_clocks(sys_clk_t new_sysclk, ahb_div_t new_ahbclk)
{
        // First the AHB clock
        if (new_ahbclk != cm_ahbclk) {
                cm_ahb_clk_next = new_ahbclk;

                if (cm_ahbclk < new_ahbclk) {
                        // fast --> slow clock switch
                        hw_cpm_set_hclk_div(new_ahbclk);
                        cm_adjust_otp_access_timings();                 // Adjust OTP timings
                }
                else {
                        // slow --> fast clock switch
                        cm_adjust_otp_access_timings();                 // Adjust OTP timings
                        hw_cpm_set_hclk_div(new_ahbclk);
                }
        }

        // Then the system clock
        if (new_sysclk != cm_sysclk) {
                cm_sys_clk_next = new_sysclk;

                // fast --> slow clock switch
                hw_cpm_set_sysclk(SYS_CLK_IS_XTAL16M);                  // Set XTAL16 as sys_clk
                cm_adjust_otp_access_timings();                         // Adjust OTP timings
        }
        // else cm_sysclk is RC16 as in all other cases it is set to XTAL16M.
}

void cm_lower_all_clocks(void)
{
        sys_clk_t new_sysclk;
        ahb_div_t new_ahbclk = ahb_div1;

        // Cannot lower clocks if the first calibration has not been completed.
        if (((dg_configUSE_LP_CLK == LP_CLK_RCX) && !cm_lp_clk_is_avail_fromISR())
                        || hw_cpm_check_dma()) {
                return;
        }

        // Check which is the lowest system clock that can be used.
        do {
                new_sysclk = cm_sysclk;

                // Check XTAL16 has settled.
                if (!cm_xtal16m_settled) {
                        break;
                }

                // Check Timer0/2
                if (hw_cpm_timer02_uses_sysclk()) {
                        break;
                }

                switch (cm_sysclk) {
                case sysclk_RC16:
                        // fall-through
                case sysclk_XTAL16M:
                        // fall-through
                case sysclk_XTAL32M:
                        // unchanged: new_sysclk = cm_sysclk
                        break;

                case sysclk_PLL48:
                        // fall-through
                case sysclk_PLL96:
                        if (dg_configEXT_CRYSTAL_FREQ == EXT_CRYSTAL_IS_16M) {
                                new_sysclk = sysclk_XTAL16M;
                        }
                        else {
                                new_sysclk = sysclk_XTAL32M;
                        }
                        break;

                case sysclk_LP:
                        // fall-through
                default:
                        // should never reach this point
                        ASSERT_WARNING(0);
                }
        } while (0);

        // Check which is the lowest AHB clock that can be used.
        if (hw_cpm_mac_is_active()) {
                switch (cm_sysclk) {
                case sysclk_XTAL16M:
                        new_ahbclk = ahb_div1;
                        break;

                case sysclk_XTAL32M:
                        // fall-through
                case sysclk_PLL48:
                        new_ahbclk = ahb_div2;
                        break;

                case sysclk_PLL96:
                        new_ahbclk = ahb_div4;
                        break;

                case sysclk_RC16:
                        // fall-through
                case sysclk_LP:
                        // fall-through
                default:
                        // should never reach this point
                        ASSERT_WARNING(0);
                }
        }
        else if (!cm_xtal16m_settled) {
                new_ahbclk = ahb_div16;                               // Use 1MHz AHB clock.
        }
        else {
                new_ahbclk = ahb_div4;                                // Use 4Mhz AHB clock.
        }

        // Check if the SysTick is ON and if it is affected
        if ((dg_configABORT_IF_SYSTICK_CLK_ERR) && (SysTick->CTRL & SysTick_CTRL_ENABLE_Msk)) {
                if ((new_sysclk != cm_sysclk) || (new_ahbclk != cm_ahbclk)) {
                        /*
                         * This is an application error! The SysTick should not run with any of the
                         * sleep modes active! This is because the OS may decide to go to sleep
                         * because all tasks are blocked and nothing is pending, although the
                         * SysTick is running.
                         */
                        new_sysclk = cm_sysclk;
                        new_ahbclk = cm_ahbclk;
                }
        }

        cm_apply_lowered_clocks(new_sysclk, new_ahbclk);
}

__RETAINED_CODE void cm_restore_all_clocks(void)
{
        if ((dg_configUSE_LP_CLK == LP_CLK_RCX) && !cm_lp_clk_is_avail_fromISR()) {
                return;
        }

        // Set the AMBA High speed Bus clock (slow --> fast clock switch)
        if (cm_ahbclk != (ahb_div_t)hw_cpm_get_hclk_div()) {
                cm_ahb_clk_next = cm_ahbclk;

                cm_adjust_otp_access_timings();                         // Adjust OTP timings
                hw_cpm_set_hclk_div(cm_ahbclk);
        }

        // Set the system clock (slow --> fast clock switch)
        if (cm_xtal16m_settled && (cm_sysclk != sysclk_RC16)) {
                cm_sys_clk_next = cm_sysclk;

                cm_adjust_otp_access_timings();                         // Adjust OTP timings
                if (cm_sysclk >= sysclk_PLL48) {
                        hw_cpm_set_sysclk(SYS_CLK_IS_PLL);              // Set PLL as sys_clk
                }
                else {
                        hw_cpm_set_sysclk(SYS_CLK_IS_XTAL16M);          // Set XTAL16 as sys_clk
                }
        }
}

void cm_wait_xtal16m_ready_fromISR(void)
{
        OS_BASE_TYPE xHigherPriorityTaskWoken;

        ASSERT_WARNING(xSemaphoreCM != NULL);

        if (!cm_xtal16m_settled) {
                while (NVIC_GetPendingIRQ(XTAL16RDY_IRQn) == 0) {
                }
                xtal16m_is_ready(&xHigherPriorityTaskWoken);
                NVIC_ClearPendingIRQ(XTAL16RDY_IRQn);
        }
}

__RETAINED_CODE bool cm_poll_xtal16m_ready(void)
{
        return cm_xtal16m_settled;
}

/* ---------------------------------------------------------------------------------------------- */

/*
 * Functions intended to be used only by the Clock and Power Manager.
 */

/**
 * \brief Lower AHB and APB clocks to the minimum frequency.
 *
 * \warning It can be called only at wake-up.
 *
 */
static inline void cm_lower_amba_clocks(void) __attribute__((always_inline));

static inline void cm_lower_amba_clocks(void)
{
        // Lower the AHB clock (fast --> slow clock switch)
        hw_cpm_set_hclk_div((uint32_t)ahb_div16);
        cm_adjust_otp_access_timings();                 // Adjust OTP timings
}

/**
 * \brief Restore AHB and APB clocks to the maximum (default) frequency.
 *
 * \warning It can be called only at wake-up.
 *
 */
static inline void cm_restore_amba_clocks(void) __attribute__((always_inline));

static inline void cm_restore_amba_clocks(void)
{
        // Restore the AHB clock (slow --> fast clock switch)
        cm_adjust_otp_access_timings();                 // Adjust OTP timings
        hw_cpm_set_hclk_div((uint32_t)ahb_div1);
}

void cm_halt_until_xtal16m_ready(void)
{
        uint32_t ulPreviousMask;

        ASSERT_WARNING(xSemaphoreCM != NULL);

        while (!cm_xtal16m_settled) {
                ulPreviousMask = portSET_INTERRUPT_MASK_FROM_ISR();
                /* System waking up. We ignore this PRIMASK set. */
                DBG_CONFIGURE_LOW(CMN_TIMING_DEBUG, CMNDBG_CRITICAL_SECTION);
                if (!cm_xtal16m_settled) {
                        cm_lower_amba_clocks();
                        __WFI();
                        cm_restore_amba_clocks();
                }
                portCLEAR_INTERRUPT_MASK_FROM_ISR(ulPreviousMask);
        }
}


/**
 * \brief Switch to XTAL16M (either 16MHz or 32MHz) - Interrupt Safe version.
 *
 * \detail Waits until the XTAL16M has settled and sets it as the system clock.
 *
 * \return void
 *
 * \warning It is called from Interrupt Context.
 *
 */
static void switch_to_xtal16_safe(void)
{
        cm_halt_until_xtal16m_ready();

        if (cm_sys_clk_next > cm_sysclk) {                              // slow --> fast clock sw
                cm_adjust_otp_access_timings();                         // Adjust OTP timings
                hw_cpm_set_sysclk(SYS_CLK_IS_XTAL16M);                  // Set XTAL16 as sys_clk
        }
        else {                                                          // fast --> slow clock sw
                hw_cpm_set_sysclk(SYS_CLK_IS_XTAL16M);                  // Set XTAL16 as sys_clk
                cm_adjust_otp_access_timings();                         // Adjust OTP timings
        }
}

void cm_sys_clk_sleep(bool entering_sleep)
{
        cm_ahb_clk_next = ahb_div1;

        if (entering_sleep) {
                // Sleep entry : transition to RC16 clock (cm_sysclk is not altered)!
                if (cm_sysclk != sysclk_RC16) {
                        switch_to_rc16();
                }

                // Make sure that the AHB and APB busses are clocked at 16MHz.
                if (cm_ahbclk != ahb_div1) {
                        // slow --> fast clock switch
                        cm_adjust_otp_access_timings();                 // Adjust OTP timings
                        hw_cpm_set_hclk_div(0);                         // cm_ahbclk is not altered!
                }
                hw_cpm_set_pclk_div(0);                                 // cm_apbclk is not altered!
        }
        else {
                /*
                 * XTAL16M ready: transition to the cm_sysclk, cm_ahbclk and cm_apbclk that were set
                 * by the user.
                 *
                 * Note that when the system wakes up the system clock is RC16 and the AHB / APB are
                 * clocked at highest frequency (because this is what the setting was just before
                 * sleep entry).
                 *
                 */

                sys_clk_t tmp_sys_clk;

                if ((cm_sysclk != sysclk_RC16) && cm_xtal16m_settled) {
                        // Check that the user's request can be applied.
                        if (cm_sysclk == sysclk_XTAL16M) {
                                if (dg_configEXT_CRYSTAL_FREQ != EXT_CRYSTAL_IS_16M) {
                                        ASSERT_ERROR(0);                // Should never happen
                                }
                        }
                        else if (cm_sysclk == sysclk_XTAL32M) {
                                if (dg_configEXT_CRYSTAL_FREQ != EXT_CRYSTAL_IS_32M) {
                                        ASSERT_ERROR(0);                // Should never happen
                                }
                        }

                        tmp_sys_clk = cm_sysclk;

                        if (dg_configEXT_CRYSTAL_FREQ == EXT_CRYSTAL_IS_16M) {
                                cm_sys_clk_next = sysclk_XTAL16M;
                        }
                        else {
                                cm_sys_clk_next = sysclk_XTAL32M;
                        }
                        cm_sysclk = sysclk_RC16;                        // Current clock is RC16
                        switch_to_xtal16_safe();

                        cm_sys_clk_next = tmp_sys_clk;

                        if ((cm_sys_clk_next == sysclk_PLL48)
                                        || (cm_sys_clk_next == sysclk_PLL96)) {
                                switch_to_pll();
                        }
                        cm_sysclk = cm_sys_clk_next;
                }
                else {
                        // If the user uses RC16 as the system clock then there's nothing to be done!

                }

                if (cm_ahbclk != ahb_div1) {
                        cm_ahb_clk_next = cm_ahbclk;

                        // fast --> slow clock switch
                        hw_cpm_set_hclk_div(cm_ahbclk);                 // cm_ahbclk is not altered!
                        cm_adjust_otp_access_timings();                 // Adjust OTP timings
                }
                // else cm_ahbclk == ahb_div1 and nothing has to be done!

                if (cm_apbclk != apb_div1) {
                        hw_cpm_set_pclk_div(cm_apbclk);
                }
                // else cm_apbclk == apb_div1 and nothing has to be done!
        }
}

void cm_sys_restore_sysclk(sys_clk_t prev_sysclk)
{
        ASSERT_ERROR( (prev_sysclk == sysclk_PLL48) || (prev_sysclk == sysclk_PLL96) );

        switch_to_pll();
        cm_sysclk = prev_sysclk;
}

void cm_sys_clk_wakeup(void)
{
        /* Timer task must have the highest priority so that it runs first
         * as soon as the OS scheduler is unblocked.
         * See caller (pm_system_wake_up()) */
        ASSERT_WARNING(configTIMER_TASK_PRIORITY == (configMAX_PRIORITIES - 1));

        OS_EVENT_GROUP_CLEAR_BITS_FROM_ISR(xEventGroupCM_xtal16_isr, BIT_1);
        cm_xtal16m_settled = false;
}

void cm_sys_xtal16m_running(void)
{
        OS_BASE_TYPE xHigherPriorityTaskWoken;
        OS_BASE_TYPE xResult __attribute__((unused));

        xResult = xtal16m_is_ready(&xHigherPriorityTaskWoken);
}


/**
 \}
 \}
 \}
 */
