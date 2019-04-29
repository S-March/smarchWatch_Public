/**
 \addtogroup BSP
 \{
 \addtogroup SYSTEM
 \{
 \addtogroup CLOCK_MANAGER
 * 
 * \brief Clock Manager
 *
 \{
 */

/**
 ****************************************************************************************
 *
 * @file sys_clock_mgr.h
 *
 * @brief Clock Manager header file.
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SYS_CLOCK_MGR_H_
#define SYS_CLOCK_MGR_H_

#include <stdint.h>
#include <stdbool.h>
#include "hw_cpm.h"

/**
 * \brief Initialize clocks after power-up.
 *
 * \param[in] type The clock source to use as the system clock.
 *
 * \return void
 *
 * \warning It must be called with interrupts enabled! It is called only once, after power-up.
 * 
 */
void cm_sys_clk_init(sys_clk_t type);

/**
 * \brief Set the system clock.
 * 
 * \details It attempts to set the sys_clk to one of the available options. If it is not possible 
 *          because, for example, it was requested to switch to RC16 and a MAC or the
 *          UART, etc., runs, it returns false. If the request involves turning on the XTAL16M, 
 *          which was turned-off for some reason, then the task will block and the XTAL16M will be 
 *          powered on. The task will resume execution when the XTAL16M settles. The CM will restore 
 *          the last setting after each wake-up, automatically, whenever the XTAL16M settles. 
 *          Note: if the SysTick runs then it is the dg_configABORT_IF_SYSTICK_CLK_ERR setting that 
 *          controls whether the switch will be aborted or not.
 *
 * \param[in] type The clock source to use as the system clock.
 *
 * \return True if the requested clock switch was applied, else false.
 *
 * \warning It may block. It cannot be called from Interrupt Context.
 * 
 */
bool cm_sys_clk_set(sys_clk_t type);

/**
 * \brief Set the CPU clock.
 * 
 * \details It attempts to set the sys_clk and the AMBA High speed bus divider to achieve the CPU
 *          clock that is requested. ARM Cortex-M0 runs using the AHB clock. Any restrictions of the
 *          cm_sys_clk_set(), cm_ahb_set_clock_divider() and cm_apb_set_clock_divider() apply here
 *          as well. The APB bus clock will be set to the maximum frequency.
 *          The function may return false if the requested frequency is not achievable. For 
 *          example, the frequency of 1MHz can be achieved only if the RC16 or the XTAL16M is used.
 *          If the system clock is not RC16 though (see warning) and the crystal connected to the 
 *          XTAL16M is 32MHz then it is not possible to get 1MHz AHB clock frequency. The smallest
 *          that can be achieved is 2MHz.
 *
 * \param[in] clk The CPU clock frequency.
 *
 * \return True if the requested clock switch was applied, else false.
 *
 * \warning Since some frequencies can be achieved with RC16, this function will not change to using 
 *          the XTAL or the PLL, if the RC16 is the current system clock. It is the responsibility 
 *          of the caller to switch to the XTAL or the PLL before calling this function. After 
 *          switching, the function will not revert to using the RC16 at any case. Thus, switching
 *          from/to RC16 may be considered as "manual" while the switching from/to any other system
 *          clock source is done automatically from this function.
 *          The setting of the clocks is done via calls to cm_sys_clk_set(), 
 *          cm_ahb_set_clock_divider() and cm_apb_set_clock_divider().
 *          It may block. It cannot be called from Interrupt Context.
 * 
 */
bool cm_cpu_clk_set(cpu_clk_t clk);

/**
 * \brief Set the system and the AHB bus clock (interrupt safe version).
 * 
 * \details It sets the sys_clk to XTAL16M or PLL and the AHB divider. Actually, it sets the clock
 *          frequency that the ARM Cortex-M0 will run. 
 *
 * \param[in] a The clock source to use as the system clock.
 * \param[in] b The divider of the AHB clock.
 *
 * \return void
 *
 * \warning It is called with interrupts disabled. The caller must have checked that the current 
 *          sys_clk is not the desired one before calling this function.
 * 
 */
void cm_cpu_clk_set_fromISR(sys_clk_t a, ahb_div_t b);

/**
 * \brief Change the divider of the AMBA Peripheral Bus clock.
 * 
 * \details The frequency of the APB clock is: 
 *              (cm_sysclk * 16 / (1 << cm_ahbclk)) / (1 << cm_apbclk).
 *              
 * \param[in] div The new value of the APB divider.
 * 
 * \return void
 *
 * \warning It may block. It cannot be called from Interrupt Context.
 * 
 */
void cm_apb_set_clock_divider(apb_div_t div);

/**
 * \brief Change the divider of the AMBA High speed Bus clock.
 * 
 * \details The frequency of the AHB clock is: 
 *              (cm_sysclk * 16 / (1 << cm_ahbclk)).
 *          Note: if the SysTick runs then it is the dg_configABORT_IF_SYSTICK_CLK_ERR setting that 
 *          controls whether the switch will be aborted or not.
 * 
 * \param[in] div The new value of the AHB divider.
 * 
 * \return True if the divider was changed to the requested value, else false.
 *
 * \warning It may block. It cannot be called from Interrupt Context.
 * 
 */
bool cm_ahb_set_clock_divider(ahb_div_t div);

/**
 * \brief Returns the sys_clk that the system uses at that moment.
 * 
 * \return The real sys_clk used by the system.
 *
 * \warning It may block. It cannot be called from Interrupt Context.
 * 
 */
sys_clk_t cm_sys_clk_get(void);

/**
 * \brief Returns the sys_clk that the system uses at that moment (interrupt safe version).
 * 
 * \return The real sys_clk used by the system.
 *
 */
sys_clk_t cm_sys_clk_get_fromISR(void);

/**
 * \brief Returns the AMBA Peripheral Bus clock divider.
 * 
 * \return The pclk being used.
 *
 * \warning It may block. It cannot be called from Interrupt Context.
 * 
 */
apb_div_t cm_apb_get_clock_divider(void);

/**
 * \brief Returns the AMBA Peripheral Bus clock divider (interrupt safe).
 *
 * \return The pclk being used.
 *
 * \warning It can be called from Interrupt Context.
 *
 */
apb_div_t cm_apb_get_clock_divider_fromISR(void);

/**
 * \brief Returns the AMBA High speed Bus clock divider.
 * 
 * \return The hclk being used.
 *
 * \warning It may block. It cannot be called from Interrupt Context.
 * 
 */
ahb_div_t cm_ahb_get_clock_divider(void);

/**
 * \brief Returns the AMBA High speed Bus clock divider (interrupt safe).
 *
 * \return The hclk being used.
 *
 * \warning It can be called from Interrupt Context.
 *
 */
ahb_div_t cm_ahb_get_clock_divider_fromISR(void);

/**
 * \brief Returns the CPU clock frequency.
 * 
 * \return The CPU clock being used.
 *
 * \warning Any restrictions of the cm_sys_clk_get() and cm_ahb_get_clock_divider() apply here as 
 *          well. It may block. It cannot be called from Interrupt Context.
 * 
 */
cpu_clk_t cm_cpu_clk_get(void);

/**
 * \brief Returns the CPU clock frequency (interrupt safe).
 *
 * \return The CPU clock being used.
 *
 * \warning It can be called from Interrupt Context.
 *
 */
cpu_clk_t cm_cpu_clk_get_fromISR(void);

/**
 * \brief Calibrate RC32K.
 * 
 * \return void
 *
 */
void cm_calibrate_rc32k(void);

/**
 * \brief Start RCX calibration.
 *
 * \return void
 *
 */
__RETAINED_CODE void cm_calibrate_rcx_start(void);

/**
 * \brief Get RCX calibration results and update environment variables.
 *
 * \return True, if calibration has finished and sleep can continue, else false.
 *
 */
bool cm_calibrate_rcx_update(void);

/**
 * \brief Converts usec to RCX cycles.
 *
 * \return The number of RCX cycles for the given time period.
 *
 * \warning Maximum time period is 4.095msec.
 *
 */
__RETAINED_CODE uint32_t cm_rcx_us_2_lpcycles(uint32_t usec);

/**
 * \brief Converts time to RCX cycles.
 *
 * \return The number of RCX cycles for the given time period.
 *
 * \warning This is a low accuracy function. To have good accuracy, the minimum time period should
 *        be 1msec and the maximum 200msec. Above 200msec, the function calculates more RCX cycles
 *        than necessary.
 *
 */
uint32_t cm_rcx_us_2_lpcycles_low_acc(uint32_t usec);

/**
 * \brief Set proper trim values to RC32K, XTAL16M and RC16.
 * 
 * \return void
 *
 * \deprecated This functionality is part of the system initialization.
 *
 */
void cm_set_trim_values(void) DEPRECATED;

/**
 * \brief Block until the XTAL16M is ready. If the XTAL16M is running then the function exits 
 *        immediately.
 * 
 * \return void
 *
 * \warning It may block. It cannot be called from Interrupt Context.
 * 
 */
void cm_wait_xtal16m_ready(void);

/**
 * \brief Initialize the Low Power clock.
 * 
 * \details It initializes and sets as LP clock either the RCX or the XTAL32K. Since the XTAL32K
 *          settling takes a long time, the system is kept in active mode until this completes. 
 * 
 * \return void
 *
 */
void cm_lp_clk_init(void);

/**
 * \brief Start the timer the blocks sleep while the low power clock is settling.
 *
 * \details It starts the timer that blocks system from sleeping for
 *          dg_configINITIAL_SLEEP_DELAY_TIME. This is needed when the XTAL32K is used to make sure
 *          that the clock has settled properly before going back to sleep again.
 *
 * \return void
 *
 */
void cm_lp_clk_timer_start(void);

/**
 * \brief Check if the Low Power clock is available.
 * 
 * \return true if the LP clock is available, else false.
 *
 * \warning It does not block. It cannot be called from Interrupt Context.
 *
 */
bool cm_lp_clk_is_avail(void);

/**
 * \brief Check if the Low Power clock is available, interrupt safe version.
 *
 * \return true if the LP clock is available, else false.
 *
 * \warning It does not block. It can be called from Interrupt Context.
 *
 */
bool cm_lp_clk_is_avail_fromISR(void);

/**
 * \brief Wait until the Low Power clock is available.
 * 
 * \return void
 *
 * \warning It may block. It cannot be called from Interrupt Context.
 *
 */
void cm_wait_lp_clk_ready(void);

/**
 * \brief   Clear the flag that indicates that the Low Power clock is available.
 *
 * \details It is called when the system wakes up from a "forced" deep sleep state and the XTAL32K
 *          is used as the LP clock so that the system won't enter into sleep until the crystal has
 *          settled.
 *
 * \return void
 *
 * \warning It may block. It cannot be called from Interrupt Context.
 *
 */
void cm_lp_clk_wakeup(void);

/**
 * \brief Interrupt handler of the XTAL16RDY_IRQn.
 * 
 * \return void
 *
 */
void XTAL16RDY_Handler(void);

/* ---------------------------------------------------------------------------------------------- */

/*
 * Functions intended to be used only by the Clock and Power Manager or in hooks.
 */

/**
 * \brief Lower all clocks to the lowest frequency possible (best effort).
 * 
 * \details 1. Check which is the lowest system clock that can be used.
 *             The RC16 is the lowest but it does not make sense to use it if the system clock is  
 *             the XTAL or the PLL. Thus, the lowest system clock setting will always be the XTAL if  
 *             the current system clock is not the RC16.
 *  
 *             If the PLL is on then the switch to the XTAL will be done without disabling the PLL. 
 * 
 *             Affects: OTPC, KBSCN*, SPI*, I2C*, ADC*, QSPI and Timer0/2* (* = conditionally) and 
 *             SysTick. Only the Timer0/2 is inquired. If any timer is active and uses the system   
 *             clock then the system clock is left unchanged. 
 *             No block is informed about the change. If there is an active SPI or I2C transaction, 
 *             it may fail.
 *             
 *          2. Check which is the lowest AHB clock that can be used.
 *             When a MAC is active, the lowest AHB clock is 16MHz.
 *             The frequency change will destroy any ongoing IR transaction.
 *             
 *          Note: if the SysTick runs then it is the dg_configABORT_IF_SYSTICK_CLK_ERR setting that 
 *          controls whether the switch will continue or it will be aborted.
 *          
 *          3. The APB clock is always set to the lowest setting.
 * 
 * \return void
 *
 * \warning It must be called with all interrupts disabled. Cannot be called by Application tasks!
 * 
 */
void cm_lower_all_clocks(void);

/**
 * \brief Restore all clocks to the speed set by the user.
 * 
 * \return void
 *
 * \warning It must be called with all interrupts disabled. Cannot be called by Application tasks!
 * 
 */
void cm_restore_all_clocks(void);

/**
 * \brief Wait until the XTAL16M is ready and then switch clocks if necessary.
 *
 * \return void
 *
 * \warning It must be called from Interrupt Context.
 *
 */
void cm_wait_xtal16m_ready_fromISR(void);

/**
 * \brief Check if the XTAL16M is ready.
 *
 * \return True if the XTAL16M has settled, else false.
 *
 */
__RETAINED_CODE bool cm_poll_xtal16m_ready(void);

/**
 * \brief Check if the XTAL16M has started and is settling.
 *
 * \warning A WDOG reset will be issued in case the XTAL has not started properly. The NMI Handler
 *        may be called, depending on the NMI_RST bit setting.
 *
 */
void cm_check_xtal_startup(void);

/**
 * \brief Basic initialization of the system and low power clocks.
 *
 * \details It switches to RC16M, restarts XTAL16M and waits for it to settle. It also sets the
 *          DIVN. Then it sets up the low power clock.
 *
 * \return void
 *
 * \warning It must be called once, before the OS scheduler is started.
 *
 */
void cm_clk_init_low_level(void);

/* ---------------------------------------------------------------------------------------------- */

/*
 * Functions intended to be used only by the Clock and Power Manager.
 */

/**
 * \brief Set the system clock (unprotected).
 * 
 * \details It attempts to set the sys_clk to:
 *              - 16MHz : called when the system is entering power-down mode. The system clock 
 *                        settings of the application are kept in order to be able to restore them.
 *                        When the system clock is RC16, it is not changed. When it is XTAL or PLL,
 *                        it is changed to RC16.
 *                        (It is called with the scheduler stopped and all interrupts disabled in 
 *                        this case.)
 *              - Previous setting : called when the XTAL16M settles.
 *                        (It is called from ISR context with all interrupts disabled in this case.) 
 *
 * \param[in] entering_sleep True if the system is going to sleep, else false.
 *
 * \return void
 *
 * \warning It must be called from Interrupt Context and/or with all interrupts disabled.
 *          The function is internal to the CPM and should not be used externally!
 * 
 */
void cm_sys_clk_sleep(bool entering_sleep);

/**
 * \brief Restore the system clock (unprotected).
 *
 * \details It attempts to restore the sys_clk to PLL-48MHz or PLL-96MHz. Is is assumed that the
 *          system runs at XTAL16M.
 *
 * \param[in] prev_sysclk The sys_clk to use.
 *
 * \warning It must be called from Interrupt Context and/or with all interrupts disabled.
 *          The function is internal to the CPM and should not be used externally!
 *
 */
void cm_sys_restore_sysclk(sys_clk_t prev_sysclk);

/**
 * \brief Clear the Event Groups Bit(s) and the "settled" flag.
 * 
 * \details It pends the clearing of the Event Groups bit(s) to the OS task daemon. In the case of
 *          waking up from the Tick timer, no other task is ready-to-run anyway. In the case
 *          though of waking up from an external interrupt to WKUPCT then another task of equal 
 *          priority to the OS daemon task may also become ready-to-run. But even in this case, the 
 *          first task that is made ready-to-run is the OS daemon task and this is the task that the
 *          scheduler will execute first. 
 * 
 * \return void
 *
 * \warning It must be called from Interrupt Context and with all interrupts disabled.
 *          The priority of the Timers task (OS daemon task) must be the highest!
 *          The function is internal to the CPM and should not be used externally!
 * 
 */
void cm_sys_clk_wakeup(void);

/**
 * \brief Halt until the XTAL16M has settled..
 *
 * \details It executes a WFI() call waiting for the XTAL16RDY_IRQ. Any other interrupts that hit
 *          (the WKUP_GPIO_IRQn should be the only one that can hit at that period), are served.
 *
 */
__RETAINED_CODE void cm_halt_until_xtal16m_ready(void);

/**
 * \brief The XTAL16M has never stopped! Inform the Adapters upon "wake-up".
 *
 * \details The debugger did not detach and the chip did not enter into sleep. Thus, the XTAL16
 *        is still running. Use this to inform Adapters that the XTAL16M is available.
 *
 * \warning It is provided only for debugging purposes since when the debugger is attached, the
 *        XTAL16M does not stop and no XTAL16RDU_IRQn hits! It will never be called in normal
 *        operation.
 *
 */
void cm_sys_xtal16m_running(void);


#define RCX_ACCURACY_LEVEL              8               // Must be a power of 2!

extern bool cm_rcx_calibration_is_on;
extern bool cm_rcx_calibration_done;
extern bool ref_calibration_failed;
extern uint16_t rcx_clock_hz;
extern uint32_t rcx_clock_hz_acc;                       // Accurate RCX freq (1/RCX_ACCURACY_LEVEL accuracy)
extern uint32_t rcx_clock_period;                       // usec multiplied by 1024 * 1024
extern uint32_t ble_slot_duration_in_rcx;               // multiplied by 1000000
extern uint8_t rcx_tick_period;                         // # of cycles in 1 tick
extern uint16_t rcx_tick_rate_hz;

/*
 * RCX frequency table
 *
 *                       min Hz     max Hz     max error
 *                       450        550        0.09%
 *
 *
 * freq        ticks     Hz                 Hz * ticks / freq (== 1sec)     Error
 * ==============================================================================
 * 7642        17        450                1.001046846                     0.10%
 * 7643        17        450                1.000915871                     0.09%
 * 7644        17        450                1.000784929                     0.08%
 * 7645        17        450                1.000654022                     0.07%
 * 7646        17        450                1.000523149                     0.05%
 * 7647        17        450                1.000392311                     0.04%
 * 7648        17        450                1.000261506                     0.03%
 * 7649        17        450                1.000130736                     0.01%
 * 7650        17        450                1                            ** 0.00%
 * 7651        17        450                0.999869298                     0.01%
 * 7652        17        450                0.99973863                      0.03%
 * 7653        17        450                0.999607997                     0.04%
 * 7654        17        450                0.999477397                     0.05%
 * 7655        17        450                0.999346832                     0.07%
 * 7656        17        450                0.999216301                     0.08%
 * 7657        17        450                0.999085804                     0.09%
 * 7658        17        450                0.998955341                     0.10%
 * 7659        17        451                1.001044523                     0.10%
 * 7660        17        451                1.000913838                     0.09%
 * 7661        17        451                1.000783188                     0.08%
 * 7662        17        451                1.000652571                     0.07%
 * 7663        17        451                1.000521989                     0.05%
 * 7664        17        451                1.000391441                     0.04%
 * 7665        17        451                1.000260926                     0.03%
 * 7666        17        451                1.000130446                     0.01%
 * 7667        17        451                1                            ** 0.00%
 * 7668        17        451                0.999869588                     0.01%
 * 7669        17        451                0.99973921                      0.03%
 * 7670        17        451                0.999608866                     0.04%
 * 7671        17        451                0.999478556                     0.05%
 * 7672        17        451                0.999348279                     0.07%
 * 7673        17        451                0.999218037                     0.08%
 * 7674        17        451                0.999087829                     0.09%
 * 7675        17        451                0.998957655                     0.10%
 * 7676        17        452                1.001042209                     0.10%
 * 7677        17        452                1.000911815                     0.09%
 * 7678        17        452                1.000781454                     0.08%
 * 7679        17        452                1.000651126                     0.07%
 * 7680        17        452                1.000520833                     0.05%
 * 7681        17        452                1.000390574                     0.04%
 * 7682        17        452                1.000260349                     0.03%
 * 7683        17        452                1.000130157                     0.01%
 * 7684        17        452                1                            ** 0.00%
 * 7685        17        452                0.999869876                     0.01%
 * 7686        17        452                0.999739787                     0.03%
 * 7687        17        452                0.999609731                     0.04%
 * 7688        17        452                0.999479709                     0.05%
 * 7689        17        452                0.99934972                      0.07%
 * 7690        17        452                0.999219766                     0.08%
 * 7691        17        452                0.999089845                     0.09%
 * 7692        17        452                0.998959958                     0.10%
 * 7693        17        453                1.001039906                     0.10%
 * 7694        17        453                1.0009098                       0.09%
 * 7695        17        453                1.000779727                     0.08%
 * 7696        17        453                1.000649688                     0.06%
 * 7697        17        453                1.000519683                     0.05%
 * 7698        17        453                1.000389712                     0.04%
 * 7699        17        453                1.000259774                     0.03%
 * 7700        17        453                1.00012987                      0.01%
 * 7701        17        453                1                            ** 0.00%
 * 7702        17        453                0.999870164                     0.01%
 * 7703        17        453                0.999740361                     0.03%
 * 7704        17        453                0.999610592                     0.04%
 * 7705        17        453                0.999480857                     0.05%
 * 7706        17        453                0.999351155                     0.06%
 * 7707        17        453                0.999221487                     0.08%
 * 7708        17        453                0.999091853                     0.09%
 * 7709        17        453                0.998962252                     0.10%
 * 7710        17        454                1.001037613                     0.10%
 * 7711        17        454                1.000907794                     0.09%
 * 7712        17        454                1.000778008                     0.08%
 * 7713        17        454                1.000648256                     0.06%
 * 7714        17        454                1.000518538                     0.05%
 * 7715        17        454                1.000388853                     0.04%
 * 7716        17        454                1.000259202                     0.03%
 * 7717        17        454                1.000129584                     0.01%
 * 7718        17        454                1                            ** 0.00%
 * 7719        17        454                0.99987045                      0.01%
 * 7720        17        454                0.999740933                     0.03%
 * 7721        17        454                0.999611449                     0.04%
 * 7722        17        454                0.999481999                     0.05%
 * 7723        17        454                0.999352583                     0.06%
 * 7724        17        454                0.9992232                       0.08%
 * 7725        17        454                0.999093851                     0.09%
 * 7726        17        454                0.998964535                     0.10%
 * 7727        17        455                1.001035331                     0.10%
 * 7728        17        455                1.000905797                     0.09%
 * 7729        17        455                1.000776297                     0.08%
 * 7730        17        455                1.000646831                     0.06%
 * 7731        17        455                1.000517397                     0.05%
 * 7732        17        455                1.000387998                     0.04%
 * 7733        17        455                1.000258632                     0.03%
 * 7734        17        455                1.000129299                     0.01%
 * 7735        17        455                1                            ** 0.00%
 * 7736        17        455                0.999870734                     0.01%
 * 7737        17        455                0.999741502                     0.03%
 * 7738        17        455                0.999612303                     0.04%
 * 7739        17        455                0.999483137                     0.05%
 * 7740        17        455                0.999354005                     0.06%
 * 7741        17        455                0.999224906                     0.08%
 * 7742        17        455                0.999095841                     0.09%
 * 7743        17        455                0.998966809                     0.10%
 * 7744        17        456                1.001033058                     0.10%
 * 7745        17        456                1.000903809                     0.09%
 * 7746        17        456                1.000774593                     0.08%
 * 7747        17        456                1.000645411                     0.06%
 * 7748        17        456                1.000516262                     0.05%
 * 7749        17        456                1.000387147                     0.04%
 * 7750        17        456                1.000258065                     0.03%
 * 7751        17        456                1.000129016                     0.01%
 * 7752        17        456                1                            ** 0.00%
 * 7753        17        456                0.999871018                     0.01%
 * 7754        17        456                0.999742069                     0.03%
 * 7755        17        456                0.999613153                     0.04%
 * 7756        17        456                0.99948427                      0.05%
 * 7757        17        456                0.999355421                     0.06%
 * 7758        17        456                0.999226605                     0.08%
 * 7759        17        456                0.999097822                     0.09%
 * 7760        17        456                0.998969072                     0.10%
 * 7761        17        457                1.001030795                     0.10%
 * 7762        17        457                1.000901829                     0.09%
 * 7763        17        457                1.000772897                     0.08%
 * 7764        17        457                1.000643998                     0.06%
 * 7765        17        457                1.000515132                     0.05%
 * 7766        17        457                1.000386299                     0.04%
 * 7767        17        457                1.0002575                       0.03%
 * 7768        17        457                1.000128733                     0.01%
 * 7769        17        457                1                            ** 0.00%
 * 7770        17        457                0.9998713                       0.01%
 * 7771        17        457                0.999742633                     0.03%
 * 7772        17        457                0.999613999                     0.04%
 * 7773        17        457                0.999485398                     0.05%
 * 7774        17        457                0.99935683                      0.06%
 * 7775        17        457                0.999228296                     0.08%
 * 7776        17        457                0.999099794                     0.09%
 * 7777        17        457                0.998971326                     0.10%
 * 7778        17        458                1.001028542                     0.10%
 * 7779        17        458                1.000899859                     0.09%
 * 7780        17        458                1.000771208                     0.08%
 * 7781        17        458                1.000642591                     0.06%
 * 7782        17        458                1.000514007                     0.05%
 * 7783        17        458                1.000385455                     0.04%
 * 7784        17        458                1.000256937                     0.03%
 * 7785        17        458                1.000128452                     0.01%
 * 7786        17        458                1                            ** 0.00%
 * 7787        17        458                0.999871581                     0.01%
 * 7788        17        458                0.999743195                     0.03%
 * 7789        17        458                0.999614841                     0.04%
 * 7790        17        458                0.999486521                     0.05%
 * 7791        17        458                0.999358234                     0.06%
 * 7792        17        458                0.999229979                     0.08%
 * 7793        17        458                0.999101758                     0.09%
 * 7794        17        458                0.998973569                     0.10%
 * 7795        17        459                1.001026299                     0.10%
 * 7796        17        459                1.000897896                     0.09%
 * 7797        17        459                1.000769527                     0.08%
 * 7798        17        459                1.00064119                      0.06%
 * 7799        17        459                1.000512886                     0.05%
 * 7800        17        459                1.000384615                     0.04%
 * 7801        17        459                1.000256377                     0.03%
 * 7802        17        459                1.000128172                     0.01%
 * 7803        17        459                1                            ** 0.00%
 * 7804        17        459                0.999871861                     0.01%
 * 7805        17        459                0.999743754                     0.03%
 * 7806        17        459                0.99961568                      0.04%
 * 7807        17        459                0.999487639                     0.05%
 * 7808        17        459                0.999359631                     0.06%
 * 7809        17        459                0.999231656                     0.08%
 * 7810        17        459                0.999103713                     0.09%
 * 7811        17        459                0.998975803                     0.10%
 * 7812        17        460                1.001024066                     0.10%
 * 7813        17        460                1.000895943                     0.09%
 * 7814        17        460                1.000767853                     0.08%
 * 7815        17        460                1.000639795                     0.06%
 * 7816        17        460                1.000511771                     0.05%
 * 7817        17        460                1.000383779                     0.04%
 * 7818        17        460                1.00025582                      0.03%
 * 7819        17        460                1.000127894                     0.01%
 * 7820        17        460                1                            ** 0.00%
 * 7821        17        460                0.999872139                     0.01%
 * 7822        17        460                0.999744311                     0.03%
 * 7823        17        460                0.999616515                     0.04%
 * 7824        17        460                0.999488753                     0.05%
 * 7825        17        460                0.999361022                     0.06%
 * 7826        17        460                0.999233325                     0.08%
 * 7827        17        460                0.99910566                      0.09%
 * 7828        17        460                0.998978028                     0.10%
 * 7829        17        461                1.001021842                     0.10%
 * 7830        17        461                1.000893997                     0.09%
 * 7831        17        461                1.000766186                     0.08%
 * 7832        17        461                1.000638407                     0.06%
 * 7833        17        461                1.00051066                      0.05%
 * 7834        17        461                1.000382946                     0.04%
 * 7835        17        461                1.000255265                     0.03%
 * 7836        17        461                1.000127616                     0.01%
 * 7837        17        461                1                            ** 0.00%
 * 7838        17        461                0.999872416                     0.01%
 * 7839        17        461                0.999744865                     0.03%
 * 7840        17        461                0.999617347                     0.04%
 * 7841        17        461                0.999489861                     0.05%
 * 7842        17        461                0.999362408                     0.06%
 * 7843        17        461                0.999234987                     0.08%
 * 7844        17        461                0.999107598                     0.09%
 * 7845        17        461                0.998980242                     0.10%
 * 7846        17        462                1.001019628                     0.10%
 * 7847        17        462                1.000892061                     0.09%
 * 7848        17        462                1.000764526                     0.08%
 * 7849        17        462                1.000637024                     0.06%
 * 7850        17        462                1.000509554                     0.05%
 * 7851        17        462                1.000382117                     0.04%
 * 7852        17        462                1.000254712                     0.03%
 * 7853        17        462                1.00012734                      0.01%
 * 7854        17        462                1                            ** 0.00%
 * 7855        17        462                0.999872693                     0.01%
 * 7856        17        462                0.999745418                     0.03%
 * 7857        17        462                0.999618175                     0.04%
 * 7858        17        462                0.999490965                     0.05%
 * 7859        17        462                0.999363787                     0.06%
 * 7860        17        462                0.999236641                     0.08%
 * 7861        17        462                0.999109528                     0.09%
 * 7862        17        462                0.998982447                     0.10%
 * 7863        17        463                1.001017423                     0.10%
 * 7864        17        463                1.000890132                     0.09%
 * 7865        17        463                1.000762873                     0.08%
 * 7866        17        463                1.000635647                     0.06%
 * 7867        17        463                1.000508453                     0.05%
 * 7868        17        463                1.000381291                     0.04%
 * 7869        17        463                1.000254162                     0.03%
 * 7870        17        463                1.000127065                     0.01%
 * 7871        17        463                1                            ** 0.00%
 * 7872        17        463                0.999872967                     0.01%
 * 7873        17        463                0.999745967                     0.03%
 * 7874        17        463                0.999618999                     0.04%
 * 7875        17        463                0.999492063                     0.05%
 * 7876        17        463                0.99936516                      0.06%
 * 7877        17        463                0.999238289                     0.08%
 * 7878        17        463                0.99911145                      0.09%
 * 7879        17        463                0.998984643                     0.10%
 * 7880        17        464                1.001015228                     0.10%
 * 7881        17        464                1.000888212                     0.09%
 * 7882        17        464                1.000761228                     0.08%
 * 7883        17        464                1.000634276                     0.06%
 * 7884        17        464                1.000507357                     0.05%
 * 7885        17        464                1.000380469                     0.04%
 * 7886        17        464                1.000253614                     0.03%
 * 7887        17        464                1.000126791                     0.01%
 * 7888        17        464                1                            ** 0.00%
 * 7889        17        464                0.999873241                     0.01%
 * 7890        17        464                0.999746515                     0.03%
 * 7891        17        464                0.99961982                      0.04%
 * 7892        17        464                0.999493158                     0.05%
 * 7893        17        464                0.999366527                     0.06%
 * 7894        17        464                0.999239929                     0.08%
 * 7895        17        464                0.999113363                     0.09%
 * 7896        17        464                0.998986829                     0.10%
 * 7897        17        465                1.001013043                     0.10%
 * 7898        17        465                1.0008863                       0.09%
 * 7899        17        465                1.00075959                      0.08%
 * 7900        17        465                1.000632911                     0.06%
 * 7901        17        465                1.000506265                     0.05%
 * 7902        17        465                1.000379651                     0.04%
 * 7903        17        465                1.000253068                     0.03%
 * 7904        17        465                1.000126518                     0.01%
 * 7905        17        465                1                            ** 0.00%
 * 7906        17        465                0.999873514                     0.01%
 * 7907        17        465                0.99974706                      0.03%
 * 7908        17        465                0.999620637                     0.04%
 * 7909        17        465                0.999494247                     0.05%
 * 7910        17        465                0.999367889                     0.06%
 * 7911        17        465                0.999241562                     0.08%
 * 7912        17        465                0.999115268                     0.09%
 * 7913        17        465                0.998989005                     0.10%
 * 7914        17        466                1.001010867                     0.10%
 * 7915        17        466                1.000884397                     0.09%
 * 7916        17        466                1.000757959                     0.08%
 * 7917        17        466                1.000631552                     0.06%
 * 7918        17        466                1.000505178                     0.05%
 * 7919        17        466                1.000378836                     0.04%
 * 7920        17        466                1.000252525                     0.03%
 * 7921        17        466                1.000126247                     0.01%
 * 7922        17        466                1                            ** 0.00%
 * 7923        17        466                0.999873785                     0.01%
 * 7924        17        466                0.999747602                     0.03%
 * 7925        17        466                0.999621451                     0.04%
 * 7926        17        466                0.999495332                     0.05%
 * 7927        17        466                0.999369244                     0.06%
 * 7928        17        466                0.999243189                     0.08%
 * 7929        17        466                0.999117165                     0.09%
 * 7930        17        466                0.998991173                     0.10%
 * 7931        17        467                1.0010087                       0.10%
 * 7932        17        467                1.000882501                     0.09%
 * 7933        17        467                1.000756334                     0.08%
 * 7934        17        467                1.000630199                     0.06%
 * 7935        17        467                1.000504096                     0.05%
 * 7936        17        467                1.000378024                     0.04%
 * 7937        17        467                1.000251984                     0.03%
 * 7938        17        467                1.000125976                     0.01%
 * 7939        17        467                1                            ** 0.00%
 * 7940        17        467                0.999874055                     0.01%
 * 7941        17        467                0.999748143                     0.03%
 * 7942        17        467                0.999622261                     0.04%
 * 7943        17        467                0.999496412                     0.05%
 * 7944        17        467                0.999370594                     0.06%
 * 7945        17        467                0.999244808                     0.08%
 * 7946        17        467                0.999119054                     0.09%
 * 7947        17        467                0.998993331                     0.10%
 * 7948        17        468                1.001006543                     0.10%
 * 7949        17        468                1.000880614                     0.09%
 * 7950        17        468                1.000754717                     0.08%
 * 7951        17        468                1.000628852                     0.06%
 * 7952        17        468                1.000503018                     0.05%
 * 7953        17        468                1.000377216                     0.04%
 * 7954        17        468                1.000251446                     0.03%
 * 7955        17        468                1.000125707                     0.01%
 * 7956        17        468                1                            ** 0.00%
 * 7957        17        468                0.999874324                     0.01%
 * 7958        17        468                0.999748681                     0.03%
 * 7959        17        468                0.999623068                     0.04%
 * 7960        17        468                0.999497487                     0.05%
 * 7961        17        468                0.999371938                     0.06%
 * 7962        17        468                0.99924642                      0.08%
 * 7963        17        468                0.999120934                     0.09%
 * 7964        17        468                0.99899548                      0.10%
 * 7965        17        469                1.001004394                     0.10%
 * 7966        17        469                1.000878735                     0.09%
 * 7967        17        469                1.000753107                     0.08%
 * 7968        17        469                1.00062751                      0.06%
 * 7969        17        469                1.000501945                     0.05%
 * 7970        17        469                1.000376412                     0.04%
 * 7971        17        469                1.00025091                      0.03%
 * 7972        17        469                1.000125439                     0.01%
 * 7973        17        469                1                            ** 0.00%
 * 7974        17        469                0.999874592                     0.01%
 * 7975        17        469                0.999749216                     0.03%
 * 7976        17        469                0.999623872                     0.04%
 * 7977        17        469                0.999498558                     0.05%
 * 7978        17        469                0.999373277                     0.06%
 * 7979        17        469                0.999248026                     0.08%
 * 7980        17        469                0.999122807                     0.09%
 * 7981        17        469                0.998997619                     0.10%
 * 7982        17        470                1.001002255                     0.10%
 * 7983        17        470                1.000876863                     0.09%
 * 7984        17        470                1.000751503                     0.08%
 * 7985        17        470                1.000626174                     0.06%
 * 7986        17        470                1.000500877                     0.05%
 * 7987        17        470                1.00037561                      0.04%
 * 7988        17        470                1.000250376                     0.03%
 * 7989        17        470                1.000125172                     0.01%
 * 7990        17        470                1                            ** 0.00%
 * 7991        17        470                0.999874859                     0.01%
 * 7992        17        470                0.99974975                      0.03%
 * 7993        17        470                0.999624672                     0.04%
 * 7994        17        470                0.999499625                     0.05%
 * 7995        17        470                0.999374609                     0.06%
 * 7996        17        470                0.999249625                     0.08%
 * 7997        17        470                0.999124672                     0.09%
 * 7998        17        470                0.99899975                      0.10%
 * 7999        17        471                1.001000125                     0.10%
 * 8000        17        471                1.000875                        0.09%
 * 8001        17        471                1.000749906                     0.07%
 * 8002        17        471                1.000624844                     0.06%
 * 8003        17        471                1.000499813                     0.05%
 * 8004        17        471                1.000374813                     0.04%
 * 8005        17        471                1.000249844                     0.02%
 * 8006        17        471                1.000124906                     0.01%
 * 8007        17        471                1                            ** 0.00%
 * 8008        17        471                0.999875125                     0.01%
 * 8009        17        471                0.999750281                     0.02%
 * 8010        17        471                0.999625468                     0.04%
 * 8011        17        471                0.999500687                     0.05%
 * 8012        17        471                0.999375936                     0.06%
 * 8013        17        471                0.999251217                     0.07%
 * 8014        17        471                0.999126529                     0.09%
 * 8015        17        471                0.999001871                     0.10%
 * 8016        17        472                1.000998004                     0.10%
 * 8017        17        472                1.000873145                     0.09%
 * 8018        17        472                1.000748316                     0.07%
 * 8019        17        472                1.000623519                     0.06%
 * 8020        17        472                1.000498753                     0.05%
 * 8021        17        472                1.000374018                     0.04%
 * 8022        17        472                1.000249314                     0.02%
 * 8023        17        472                1.000124642                     0.01%
 * 8024        17        472                1                            ** 0.00%
 * 8025        17        472                0.999875389                     0.01%
 * 8026        17        472                0.99975081                      0.02%
 * 8027        17        472                0.999626261                     0.04%
 * 8028        17        472                0.999501744                     0.05%
 * 8029        17        472                0.999377257                     0.06%
 * 8030        17        472                0.999252802                     0.07%
 * 8031        17        472                0.999128378                     0.09%
 * 8032        17        472                0.999003984                     0.10%
 * 8033        17        473                1.000995892                     0.10%
 * 8034        17        473                1.000871297                     0.09%
 * 8035        17        473                1.000746733                     0.07%
 * 8036        17        473                1.0006222                       0.06%
 * 8037        17        473                1.000497698                     0.05%
 * 8038        17        473                1.000373227                     0.04%
 * 8039        17        473                1.000248787                     0.02%
 * 8040        17        473                1.000124378                     0.01%
 * 8041        17        473                1                            ** 0.00%
 * 8042        17        473                0.999875653                     0.01%
 * 8043        17        473                0.999751337                     0.02%
 * 8044        17        473                0.999627051                     0.04%
 * 8045        17        473                0.999502797                     0.05%
 * 8046        17        473                0.999378573                     0.06%
 * 8047        17        473                0.999254381                     0.07%
 * 8048        17        473                0.999130219                     0.09%
 * 8049        17        473                0.999006088                     0.10%
 * 8050        17        474                1.000993789                     0.10%
 * 8051        17        474                1.000869457                     0.09%
 * 8052        17        474                1.000745156                     0.07%
 * 8053        17        474                1.000620887                     0.06%
 * 8054        17        474                1.000496648                     0.05%
 * 8055        17        474                1.000372439                     0.04%
 * 8056        17        474                1.000248262                     0.02%
 * 8057        17        474                1.000124116                     0.01%
 * 8058        17        474                1                            ** 0.00%
 * 8059        17        474                0.999875915                     0.01%
 * 8060        17        474                0.999751861                     0.02%
 * 8061        17        474                0.999627838                     0.04%
 * 8062        17        474                0.999503845                     0.05%
 * 8063        17        474                0.999379883                     0.06%
 * 8064        17        474                0.999255952                     0.07%
 * 8065        17        474                0.999132052                     0.09%
 * 8066        17        474                0.999008182                     0.10%
 * 8067        17        475                1.000991695                     0.10%
 * 8068        17        475                1.000867625                     0.09%
 * 8069        17        475                1.000743587                     0.07%
 * 8070        17        475                1.000619579                     0.06%
 * 8071        17        475                1.000495602                     0.05%
 * 8072        17        475                1.000371655                     0.04%
 * 8073        17        475                1.000247739                     0.02%
 * 8074        17        475                1.000123854                     0.01%
 * 8075        17        475                1                            ** 0.00%
 * 8076        17        475                0.999876176                     0.01%
 * 8077        17        475                0.999752383                     0.02%
 * 8078        17        475                0.999628621                     0.04%
 * 8079        17        475                0.999504889                     0.05%
 * 8080        17        475                0.999381188                     0.06%
 * 8081        17        475                0.999257518                     0.07%
 * 8082        17        475                0.999133878                     0.09%
 * 8083        17        475                0.999010268                     0.10%
 * 8084        17        476                1.000989609                     0.10%
 * 8085        17        476                1.000865801                     0.09%
 * 8086        17        476                1.000742023                     0.07%
 * 8087        17        476                1.000618276                     0.06%
 * 8088        17        476                1.00049456                      0.05%
 * 8089        17        476                1.000370874                     0.04%
 * 8090        17        476                1.000247219                     0.02%
 * 8091        17        476                1.000123594                     0.01%
 * 8092        17        476                1                            ** 0.00%
 * 8093        17        476                0.999876436                     0.01%
 * 8094        17        476                0.999752903                     0.02%
 * 8095        17        476                0.999629401                     0.04%
 * 8096        17        476                0.999505929                     0.05%
 * 8097        18        450                1.000370508                     0.04%
 * 8098        18        450                1.000246975                     0.02%
 * 8099        18        450                1.000123472                     0.01%
 * 8100        18        450                1                            ** 0.00%
 * 8101        18        450                0.999876558                     0.01%
 * 8102        18        450                0.999753147                     0.02%
 * 8103        18        450                0.999629767                     0.04%
 * 8104        18        450                0.999506417                     0.05%
 * 8105        17        477                1.000493523                     0.05%
 * 8106        17        477                1.000370096                     0.04%
 * 8107        17        477                1.0002467                       0.02%
 * 8108        17        477                1.000123335                     0.01%
 * 8109        17        477                1                            ** 0.00%
 * 8110        17        477                0.999876695                     0.01%
 * 8111        17        477                0.999753421                     0.02%
 * 8112        17        477                0.999630178                     0.04%
 * 8113        17        477                0.999506964                     0.05%
 * 8114        18        451                1.000492975                     0.05%
 * 8115        18        451                1.000369686                     0.04%
 * 8116        18        451                1.000246427                     0.02%
 * 8117        18        451                1.000123198                     0.01%
 * 8118        18        451                1                            ** 0.00%
 * 8119        18        451                0.999876832                     0.01%
 * 8120        18        451                0.999753695                     0.02%
 * 8121        18        451                0.999630587                     0.04%
 * 8122        18        451                0.99950751                      0.05%
 * 8123        17        478                1.000369322                     0.04%
 * 8124        17        478                1.000246184                     0.02%
 * 8125        17        478                1.000123077                     0.01%
 * 8126        17        478                1                            ** 0.00%
 * 8127        17        478                0.999876953                     0.01%
 * 8128        17        478                0.999753937                     0.02%
 * 8129        17        478                0.999630951                     0.04%
 * 8130        17        478                0.999507995                     0.05%
 * 8131        17        478                0.999385069                     0.06%
 * 8132        18        452                1.000491884                     0.05%
 * 8133        18        452                1.000368868                     0.04%
 * 8134        18        452                1.000245881                     0.02%
 * 8135        18        452                1.000122926                     0.01%
 * 8136        18        452                1                            ** 0.00%
 * 8137        18        452                0.999877105                     0.01%
 * 8138        18        452                0.999754239                     0.02%
 * 8139        18        452                0.999631404                     0.04%
 * 8140        17        479                1.00036855                      0.04%
 * 8141        17        479                1.00024567                      0.02%
 * 8142        17        479                1.00012282                      0.01%
 * 8143        17        479                1                            ** 0.00%
 * 8144        17        479                0.99987721                      0.01%
 * 8145        17        479                0.999754451                     0.02%
 * 8146        17        479                0.999631721                     0.04%
 * 8147        17        479                0.999509022                     0.05%
 * 8148        17        479                0.999386352                     0.06%
 * 8149        18        453                1.000613572                     0.06%
 * 8150        18        453                1.000490798                     0.05%
 * 8151        18        453                1.000368053                     0.04%
 * 8152        18        453                1.000245339                     0.02%
 * 8153        18        453                1.000122654                     0.01%
 * 8154        18        453                1                            ** 0.00%
 * 8155        18        453                0.999877376                     0.01%
 * 8156        18        453                0.999754782                     0.02%
 * 8157        17        480                1.000367782                     0.04%
 * 8158        17        480                1.000245158                     0.02%
 * 8159        17        480                1.000122564                     0.01%
 * 8160        17        480                1                            ** 0.00%
 * 8161        17        480                0.999877466                     0.01%
 * 8162        17        480                0.999754962                     0.02%
 * 8163        17        480                0.999632488                     0.04%
 * 8164        17        480                0.999510044                     0.05%
 * 8165        17        480                0.99938763                      0.06%
 * 8166        17        480                0.999265246                     0.07%
 * 8167        18        454                1.00061222                      0.06%
 * 8168        18        454                1.000489716                     0.05%
 * 8169        18        454                1.000367242                     0.04%
 * 8170        18        454                1.000244798                     0.02%
 * 8171        18        454                1.000122384                     0.01%
 * 8172        18        454                1                            ** 0.00%
 * 8173        18        454                0.999877646                     0.01%
 * 8174        18        454                0.999755322                     0.02%
 * 8175        17        481                1.000244648                     0.02%
 * 8176        17        481                1.000122309                     0.01%
 * 8177        17        481                1                            ** 0.00%
 * 8178        17        481                0.999877721                     0.01%
 * 8179        17        481                0.999755471                     0.02%
 * 8180        17        481                0.999633252                     0.04%
 * 8181        17        481                0.999511062                     0.05%
 * 8182        17        481                0.999388902                     0.06%
 * 8183        17        481                0.999266773                     0.07%
 * 8184        18        455                1.000733138                     0.07%
 * 8185        18        455                1.000610874                     0.06%
 * 8186        18        455                1.000488639                     0.05%
 * 8187        18        455                1.000366435                     0.04%
 * 8188        18        455                1.00024426                      0.02%
 * 8189        18        455                1.000122115                     0.01%
 * 8190        18        455                1                            ** 0.00%
 * 8191        18        455                0.999877915                     0.01%
 * 8192        17        482                1.000244141                     0.02%
 * 8193        17        482                1.000122055                     0.01%
 * 8194        17        482                1                            ** 0.00%
 * 8195        17        482                0.999877974                     0.01%
 * 8196        17        482                0.999755979                     0.02%
 * 8197        17        482                0.999634012                     0.04%
 * 8198        17        482                0.999512076                     0.05%
 * 8199        17        482                0.99939017                      0.06%
 * 8200        17        482                0.999268293                     0.07%
 * 8201        17        482                0.999146446                     0.09%
 * 8202        18        456                1.000731529                     0.07%
 * 8203        18        456                1.000609533                     0.06%
 * 8204        18        456                1.000487567                     0.05%
 * 8205        18        456                1.000365631                     0.04%
 * 8206        18        456                1.000243724                     0.02%
 * 8207        18        456                1.000121847                     0.01%
 * 8208        18        456                1                            ** 0.00%
 * 8209        18        456                0.999878182                     0.01%
 * 8210        17        483                1.000121803                     0.01%
 * 8211        17        483                1                            ** 0.00%
 * 8212        17        483                0.999878227                     0.01%
 * 8213        17        483                0.999756484                     0.02%
 * 8214        17        483                0.99963477                      0.04%
 * 8215        17        483                0.999513086                     0.05%
 * 8216        17        483                0.999391431                     0.06%
 * 8217        17        483                0.999269806                     0.07%
 * 8218        17        483                0.999148211                     0.09%
 * 8219        18        457                1.000851685                     0.09%
 * 8220        18        457                1.000729927                     0.07%
 * 8221        18        457                1.000608199                     0.06%
 * 8222        18        457                1.0004865                       0.05%
 * 8223        18        457                1.00036483                      0.04%
 * 8224        18        457                1.000243191                     0.02%
 * 8225        18        457                1.000121581                     0.01%
 * 8226        18        457                1                            ** 0.00%
 * 8227        17        484                1.000121551                     0.01%
 * 8228        17        484                1                            ** 0.00%
 * 8229        17        484                0.999878479                     0.01%
 * 8230        17        484                0.999756987                     0.02%
 * 8231        17        484                0.999635524                     0.04%
 * 8232        17        484                0.999514091                     0.05%
 * 8233        17        484                0.999392688                     0.06%
 * 8234        17        484                0.999271314                     0.07%
 * 8235        17        484                0.99914997                      0.09%
 * 8236        17        484                0.999028655                     0.10%
 * 8237        18        458                1.000849824                     0.08%
 * 8238        18        458                1.000728332                     0.07%
 * 8239        18        458                1.00060687                      0.06%
 * 8240        18        458                1.000485437                     0.05%
 * 8241        18        458                1.000364033                     0.04%
 * 8242        18        458                1.00024266                      0.02%
 * 8243        18        458                1.000121315                     0.01%
 * 8244        18        458                1                            ** 0.00%
 * 8245        17        485                1                            ** 0.00%
 * 8246        17        485                0.999878729                     0.01%
 * 8247        17        485                0.999757488                     0.02%
 * 8248        17        485                0.999636275                     0.04%
 * 8249        17        485                0.999515093                     0.05%
 * 8250        17        485                0.999393939                     0.06%
 * 8251        17        485                0.999272815                     0.07%
 * 8252        17        485                0.999151721                     0.08%
 * 8253        17        485                0.999030656                     0.10%
 * 8254        17        486                1.000969227                     0.10%
 * 8255        17        486                1.000847971                     0.08%
 * 8256        17        486                1.000726744                     0.07%
 * 8257        17        486                1.000605547                     0.06%
 * 8258        17        486                1.000484379                     0.05%
 * 8259        17        486                1.00036324                      0.04%
 * 8260        17        486                1.000242131                     0.02%
 * 8261        17        486                1.000121051                     0.01%
 * 8262        17        486                1                            ** 0.00%
 * 8263        17        486                0.999878979                     0.01%
 * 8264        17        486                0.999757986                     0.02%
 * 8265        17        486                0.999637024                     0.04%
 * 8266        17        486                0.99951609                      0.05%
 * 8267        17        486                0.999395186                     0.06%
 * 8268        17        486                0.999274311                     0.07%
 * 8269        17        486                0.999153465                     0.08%
 * 8270        17        486                0.999032648                     0.10%
 * 8271        17        487                1.000967235                     0.10%
 * 8272        17        487                1.000846228                     0.08%
 * 8273        17        487                1.000725251                     0.07%
 * 8274        17        487                1.000604303                     0.06%
 * 8275        17        487                1.000483384                     0.05%
 * 8276        17        487                1.000362494                     0.04%
 * 8277        17        487                1.000241633                     0.02%
 * 8278        17        487                1.000120802                     0.01%
 * 8279        17        487                1                            ** 0.00%
 * 8280        18        460                1                            ** 0.00%
 * 8281        18        460                0.999879242                     0.01%
 * 8282        18        460                0.999758512                     0.02%
 * 8283        18        460                0.999637812                     0.04%
 * 8284        18        460                0.999517141                     0.05%
 * 8285        18        460                0.9993965                       0.06%
 * 8286        18        460                0.999275887                     0.07%
 * 8287        18        460                0.999155303                     0.08%
 * 8288        17        488                1.000965251                     0.10%
 * 8289        17        488                1.000844493                     0.08%
 * 8290        17        488                1.000723764                     0.07%
 * 8291        17        488                1.000603064                     0.06%
 * 8292        17        488                1.000482393                     0.05%
 * 8293        17        488                1.000361751                     0.04%
 * 8294        17        488                1.000241138                     0.02%
 * 8295        17        488                1.000120555                     0.01%
 * 8296        17        488                1                            ** 0.00%
 * 8297        17        488                0.999879475                     0.01%
 * 8298        18        461                1                            ** 0.00%
 * 8299        18        461                0.999879504                     0.01%
 * 8300        18        461                0.999759036                     0.02%
 * 8301        18        461                0.999638598                     0.04%
 * 8302        18        461                0.999518188                     0.05%
 * 8303        18        461                0.999397808                     0.06%
 * 8304        18        461                0.999277457                     0.07%
 * 8305        18        461                0.999157134                     0.08%
 * 8306        17        489                1.000842764                     0.08%
 * 8307        17        489                1.000722282                     0.07%
 * 8308        17        489                1.00060183                      0.06%
 * 8309        17        489                1.000481406                     0.05%
 * 8310        17        489                1.000361011                     0.04%
 * 8311        17        489                1.000240645                     0.02%
 * 8312        17        489                1.000120308                     0.01%
 * 8313        17        489                1                            ** 0.00%
 * 8314        17        489                0.999879721                     0.01%
 * 8315        18        462                1.000120265                     0.01%
 * 8316        18        462                1                            ** 0.00%
 * 8317        18        462                0.999879764                     0.01%
 * 8318        18        462                0.999759558                     0.02%
 * 8319        18        462                0.99963938                      0.04%
 * 8320        18        462                0.999519231                     0.05%
 * 8321        18        462                0.999399111                     0.06%
 * 8322        18        462                0.999279019                     0.07%
 * 8323        17        490                1.000841043                     0.08%
 * 8324        17        490                1.000720807                     0.07%
 * 8325        17        490                1.000600601                     0.06%
 * 8326        17        490                1.000480423                     0.05%
 * 8327        17        490                1.000360274                     0.04%
 * 8328        17        490                1.000240154                     0.02%
 * 8329        17        490                1.000120062                     0.01%
 * 8330        17        490                1                            ** 0.00%
 * 8331        17        490                0.999879966                     0.01%
 * 8332        17        490                0.999759962                     0.02%
 * 8333        18        463                1.000120005                     0.01%
 * 8334        18        463                1                            ** 0.00%
 * 8335        18        463                0.999880024                     0.01%
 * 8336        18        463                0.999760077                     0.02%
 * 8337        18        463                0.999640158                     0.04%
 * 8338        18        463                0.999520269                     0.05%
 * 8339        18        463                0.999400408                     0.06%
 * 8340        18        463                0.999280576                     0.07%
 * 8341        17        491                1.000719338                     0.07%
 * 8342        17        491                1.000599377                     0.06%
 * 8343        17        491                1.000479444                     0.05%
 * 8344        17        491                1.00035954                      0.04%
 * 8345        17        491                1.000239664                     0.02%
 * 8346        17        491                1.000119818                     0.01%
 * 8347        17        491                1                            ** 0.00%
 * 8348        17        491                0.999880211                     0.01%
 * 8349        17        491                0.99976045                      0.02%
 * 8350        18        464                1.000239521                     0.02%
 * 8351        18        464                1.000119746                     0.01%
 * 8352        18        464                1                            ** 0.00%
 * 8353        18        464                0.999880283                     0.01%
 * 8354        18        464                0.999760594                     0.02%
 * 8355        18        464                0.999640934                     0.04%
 * 8356        18        464                0.999521302                     0.05%
 * 8357        18        464                0.999401699                     0.06%
 * 8358        17        492                1.000717875                     0.07%
 * 8359        17        492                1.000598158                     0.06%
 * 8360        17        492                1.000478469                     0.05%
 * 8361        17        492                1.000358809                     0.04%
 * 8362        17        492                1.000239177                     0.02%
 * 8363        17        492                1.000119574                     0.01%
 * 8364        17        492                1                            ** 0.00%
 * 8365        17        492                0.999880454                     0.01%
 * 8366        17        492                0.999760937                     0.02%
 * 8367        17        492                0.999641449                     0.04%
 * 8368        18        465                1.000239006                     0.02%
 * 8369        18        465                1.000119489                     0.01%
 * 8370        18        465                1                            ** 0.00%
 * 8371        18        465                0.99988054                      0.01%
 * 8372        18        465                0.999761108                     0.02%
 * 8373        18        465                0.999641705                     0.04%
 * 8374        18        465                0.999522331                     0.05%
 * 8375        18        465                0.999402985                     0.06%
 * 8376        17        493                1.000596944                     0.06%
 * 8377        17        493                1.000477498                     0.05%
 * 8378        17        493                1.000358081                     0.04%
 * 8379        17        493                1.000238692                     0.02%
 * 8380        17        493                1.000119332                     0.01%
 * 8381        17        493                1                            ** 0.00%
 * 8382        17        493                0.999880697                     0.01%
 * 8383        17        493                0.999761422                     0.02%
 * 8384        17        493                0.999642176                     0.04%
 * 8385        18        466                1.000357782                     0.04%
 * 8386        18        466                1.000238493                     0.02%
 * 8387        18        466                1.000119232                     0.01%
 * 8388        18        466                1                            ** 0.00%
 * 8389        18        466                0.999880796                     0.01%
 * 8390        18        466                0.999761621                     0.02%
 * 8391        18        466                0.999642474                     0.04%
 * 8392        18        466                0.999523356                     0.05%
 * 8393        17        494                1.000595735                     0.06%
 * 8394        17        494                1.000476531                     0.05%
 * 8395        17        494                1.000357356                     0.04%
 * 8396        17        494                1.000238209                     0.02%
 * 8397        17        494                1.00011909                      0.01%
 * 8398        17        494                1                            ** 0.00%
 * 8399        17        494                0.999880938                     0.01%
 * 8400        17        494                0.999761905                     0.02%
 * 8401        17        494                0.9996429                       0.04%
 * 8402        17        494                0.999523923                     0.05%
 * 8403        18        467                1.000357015                     0.04%
 * 8404        18        467                1.000237982                     0.02%
 * 8405        18        467                1.000118977                     0.01%
 * 8406        18        467                1                            ** 0.00%
 * 8407        18        467                0.999881052                     0.01%
 * 8408        18        467                0.999762131                     0.02%
 * 8409        18        467                0.999643239                     0.04%
 * 8410        18        467                0.999524376                     0.05%
 * 8411        17        495                1.000475568                     0.05%
 * 8412        17        495                1.000356633                     0.04%
 * 8413        17        495                1.000237727                     0.02%
 * 8414        17        495                1.00011885                      0.01%
 * 8415        17        495                1                            ** 0.00%
 * 8416        17        495                0.999881179                     0.01%
 * 8417        17        495                0.999762386                     0.02%
 * 8418        17        495                0.999643621                     0.04%
 * 8419        17        495                0.999524884                     0.05%
 * 8420        18        468                1.000475059                     0.05%
 * 8421        18        468                1.000356252                     0.04%
 * 8422        18        468                1.000237473                     0.02%
 * 8423        18        468                1.000118723                     0.01%
 * 8424        18        468                1                            ** 0.00%
 * 8425        18        468                0.999881306                     0.01%
 * 8426        18        468                0.999762639                     0.02%
 * 8427        18        468                0.999644001                     0.04%
 * 8428        17        496                1.000474608                     0.05%
 * 8429        17        496                1.000355914                     0.04%
 * 8430        17        496                1.000237248                     0.02%
 * 8431        17        496                1.00011861                      0.01%
 * 8432        17        496                1                            ** 0.00%
 * 8433        17        496                0.999881418                     0.01%
 * 8434        17        496                0.999762865                     0.02%
 * 8435        17        496                0.999644339                     0.04%
 * 8436        17        496                0.999525842                     0.05%
 * 8437        17        496                0.999407372                     0.06%
 * 8438        18        469                1.000474046                     0.05%
 * 8439        18        469                1.000355492                     0.04%
 * 8440        18        469                1.000236967                     0.02%
 * 8441        18        469                1.000118469                     0.01%
 * 8442        18        469                1                            ** 0.00%
 * 8443        18        469                0.999881559                     0.01%
 * 8444        18        469                0.999763145                     0.02%
 * 8445        18        469                0.99964476                      0.04%
 * 8446        17        497                1.000355198                     0.04%
 * 8447        17        497                1.00023677                      0.02%
 * 8448        17        497                1.000118371                     0.01%
 * 8449        17        497                1                            ** 0.00%
 * 8450        17        497                0.999881657                     0.01%
 * 8451        17        497                0.999763342                     0.02%
 * 8452        17        497                0.999645054                     0.04%
 * 8453        17        497                0.999526795                     0.05%
 * 8454        17        497                0.999408564                     0.06%
 * 8455        18        470                1.000591366                     0.06%
 * 8456        18        470                1.000473037                     0.05%
 * 8457        18        470                1.000354736                     0.04%
 * 8458        18        470                1.000236463                     0.02%
 * 8459        18        470                1.000118217                     0.01%
 * 8460        18        470                1                            ** 0.00%
 * 8461        18        470                0.999881811                     0.01%
 * 8462        18        470                0.999763649                     0.02%
 * 8463        17        498                1.000354484                     0.04%
 * 8464        17        498                1.000236295                     0.02%
 * 8465        17        498                1.000118133                     0.01%
 * 8466        17        498                1                            ** 0.00%
 * 8467        17        498                0.999881894                     0.01%
 * 8468        17        498                0.999763817                     0.02%
 * 8469        17        498                0.999645767                     0.04%
 * 8470        17        498                0.999527745                     0.05%
 * 8471        17        498                0.999409751                     0.06%
 * 8472        17        498                0.999291785                     0.07%
 * 8473        18        471                1.00059011                      0.06%
 * 8474        18        471                1.000472032                     0.05%
 * 8475        18        471                1.000353982                     0.04%
 * 8476        18        471                1.00023596                      0.02%
 * 8477        18        471                1.000117966                     0.01%
 * 8478        18        471                1                            ** 0.00%
 * 8479        18        471                0.999882062                     0.01%
 * 8480        18        471                0.999764151                     0.02%
 * 8481        17        499                1.000235821                     0.02%
 * 8482        17        499                1.000117897                     0.01%
 * 8483        17        499                1                            ** 0.00%
 * 8484        17        499                0.999882131                     0.01%
 * 8485        17        499                0.99976429                      0.02%
 * 8486        17        499                0.999646477                     0.04%
 * 8487        17        499                0.999528691                     0.05%
 * 8488        17        499                0.999410933                     0.06%
 * 8489        17        499                0.999293203                     0.07%
 * 8490        18        472                1.000706714                     0.07%
 * 8491        18        472                1.000588859                     0.06%
 * 8492        18        472                1.000471032                     0.05%
 * 8493        18        472                1.000353232                     0.04%
 * 8494        18        472                1.00023546                      0.02%
 * 8495        18        472                1.000117716                     0.01%
 * 8496        18        472                1                            ** 0.00%
 * 8497        18        472                0.999882311                     0.01%
 * 8498        17        500                1.000235349                     0.02%
 * 8499        17        500                1.000117661                     0.01%
 * 8500        17        500                1                            ** 0.00%
 * 8501        17        500                0.999882367                     0.01%
 * 8502        17        500                0.999764761                     0.02%
 * 8503        17        500                0.999647183                     0.04%
 * 8504        17        500                0.999529633                     0.05%
 * 8505        17        500                0.999412111                     0.06%
 * 8506        17        500                0.999294616                     0.07%
 * 8507        17        500                0.999177148                     0.08%
 * 8508        18        473                1.000705219                     0.07%
 * 8509        18        473                1.000587613                     0.06%
 * 8510        18        473                1.000470035                     0.05%
 * 8511        18        473                1.000352485                     0.04%
 * 8512        18        473                1.000234962                     0.02%
 * 8513        18        473                1.000117467                     0.01%
 * 8514        18        473                1                            ** 0.00%
 * 8515        18        473                0.99988256                      0.01%
 * 8516        17        501                1.000117426                     0.01%
 * 8517        17        501                1                            ** 0.00%
 * 8518        17        501                0.999882602                     0.01%
 * 8519        17        501                0.999765231                     0.02%
 * 8520        17        501                0.999647887                     0.04%
 * 8521        17        501                0.999530572                     0.05%
 * 8522        17        501                0.999413283                     0.06%
 * 8523        17        501                0.999296023                     0.07%
 * 8524        17        501                0.999178789                     0.08%
 * 8525        18        474                1.000821114                     0.08%
 * 8526        18        474                1.00070373                      0.07%
 * 8527        18        474                1.000586373                     0.06%
 * 8528        18        474                1.000469043                     0.05%
 * 8529        18        474                1.000351741                     0.04%
 * 8530        18        474                1.000234467                     0.02%
 * 8531        18        474                1.00011722                      0.01%
 * 8532        18        474                1                            ** 0.00%
 * 8533        17        502                1.000117192                     0.01%
 * 8534        17        502                1                            ** 0.00%
 * 8535        17        502                0.999882835                     0.01%
 * 8536        17        502                0.999765698                     0.02%
 * 8537        17        502                0.999648588                     0.04%
 * 8538        17        502                0.999531506                     0.05%
 * 8539        17        502                0.999414451                     0.06%
 * 8540        17        502                0.999297424                     0.07%
 * 8541        17        502                0.999180424                     0.08%
 * 8542        17        502                0.999063451                     0.09%
 * 8543        18        475                1.000819384                     0.08%
 * 8544        18        475                1.000702247                     0.07%
 * 8545        18        475                1.000585138                     0.06%
 * 8546        18        475                1.000468055                     0.05%
 * 8547        18        475                1.000351                        0.04%
 * 8548        18        475                1.000233973                     0.02%
 * 8549        18        475                1.000116973                     0.01%
 * 8550        18        475                1                            ** 0.00%
 * 8551        17        503                1                            ** 0.00%
 * 8552        17        503                0.999883068                     0.01%
 * 8553        17        503                0.999766164                     0.02%
 * 8554        17        503                0.999649287                     0.04%
 * 8555        17        503                0.999532437                     0.05%
 * 8556        17        503                0.999415615                     0.06%
 * 8557        17        503                0.99929882                      0.07%
 * 8558        17        503                0.999182052                     0.08%
 * 8559        17        503                0.999065311                     0.09%
 * 8560        17        504                1.000934579                     0.09%
 * 8561        17        504                1.000817661                     0.08%
 * 8562        17        504                1.000700771                     0.07%
 * 8563        17        504                1.000583908                     0.06%
 * 8564        17        504                1.000467071                     0.05%
 * 8565        17        504                1.000350263                     0.04%
 * 8566        17        504                1.000233481                     0.02%
 * 8567        17        504                1.000116727                     0.01%
 * 8568        17        504                1                            ** 0.00%
 * 8569        19        451                1                            ** 0.00%
 * 8570        19        451                0.999883314                     0.01%
 * 8571        19        451                0.999766655                     0.02%
 * 8572        19        451                0.999650023                     0.03%
 * 8573        19        451                0.999533419                     0.05%
 * 8574        19        451                0.999416842                     0.06%
 * 8575        19        451                0.999300292                     0.07%
 * 8576        19        451                0.999183769                     0.08%
 * 8577        17        505                1.000932727                     0.09%
 * 8578        17        505                1.000816041                     0.08%
 * 8579        17        505                1.000699382                     0.07%
 * 8580        17        505                1.000582751                     0.06%
 * 8581        17        505                1.000466146                     0.05%
 * 8582        17        505                1.000349569                     0.03%
 * 8583        17        505                1.000233019                     0.02%
 * 8584        17        505                1.000116496                     0.01%
 * 8585        17        505                1                            ** 0.00%
 * 8586        18        477                1                            ** 0.00%
 * 8587        18        477                0.999883545                     0.01%
 * 8588        19        452                1                            ** 0.00%
 * 8589        19        452                0.999883572                     0.01%
 * 8590        19        452                0.999767171                     0.02%
 * 8591        19        452                0.999650797                     0.03%
 * 8592        19        452                0.999534451                     0.05%
 * 8593        19        452                0.999418131                     0.06%
 * 8594        19        452                0.999301838                     0.07%
 * 8595        17        506                1.000814427                     0.08%
 * 8596        17        506                1.000697999                     0.07%
 * 8597        17        506                1.000581598                     0.06%
 * 8598        17        506                1.000465224                     0.05%
 * 8599        17        506                1.000348878                     0.03%
 * 8600        17        506                1.000232558                     0.02%
 * 8601        17        506                1.000116266                     0.01%
 * 8602        17        506                1                            ** 0.00%
 * 8603        18        478                1.000116239                     0.01%
 * 8604        18        478                1                            ** 0.00%
 * 8605        18        478                0.999883788                     0.01%
 * 8606        19        453                1.000116198                     0.01%
 * 8607        19        453                1                            ** 0.00%
 * 8608        19        453                0.999883829                     0.01%
 * 8609        19        453                0.999767685                     0.02%
 * 8610        19        453                0.999651568                     0.03%
 * 8611        19        453                0.999535478                     0.05%
 * 8612        19        453                0.999419415                     0.06%
 * 8613        17        507                1.000696621                     0.07%
 * 8614        17        507                1.00058045                      0.06%
 * 8615        17        507                1.000464306                     0.05%
 * 8616        17        507                1.000348189                     0.03%
 * 8617        17        507                1.000232099                     0.02%
 * 8618        17        507                1.000116036                     0.01%
 * 8619        17        507                1                            ** 0.00%
 * 8620        17        507                0.999883991                     0.01%
 * 8621        18        479                1.000115996                     0.01%
 * 8622        18        479                1                            ** 0.00%
 * 8623        18        479                0.999884031                     0.01%
 * 8624        19        454                1.000231911                     0.02%
 * 8625        19        454                1.000115942                     0.01%
 * 8626        19        454                1                            ** 0.00%
 * 8627        19        454                0.999884085                     0.01%
 * 8628        19        454                0.999768197                     0.02%
 * 8629        19        454                0.999652335                     0.03%
 * 8630        19        454                0.999536501                     0.05%
 * 8631        17        508                1.000579307                     0.06%
 * 8632        17        508                1.000463392                     0.05%
 * 8633        17        508                1.000347504                     0.03%
 * 8634        17        508                1.000231642                     0.02%
 * 8635        17        508                1.000115808                     0.01%
 * 8636        17        508                1                            ** 0.00%
 * 8637        17        508                0.999884219                     0.01%
 * 8638        17        508                0.999768465                     0.02%
 * 8639        18        480                1.000115754                     0.01%
 * 8640        18        480                1                            ** 0.00%
 * 8641        18        480                0.999884273                     0.01%
 * 8642        18        480                0.999768572                     0.02%
 * 8643        19        455                1.000231401                     0.02%
 * 8644        19        455                1.000115687                     0.01%
 * 8645        19        455                1                            ** 0.00%
 * 8646        19        455                0.99988434                      0.01%
 * 8647        19        455                0.999768706                     0.02%
 * 8648        19        455                0.999653099                     0.03%
 * 8649        17        509                1.000462481                     0.05%
 * 8650        17        509                1.000346821                     0.03%
 * 8651        17        509                1.000231187                     0.02%
 * 8652        17        509                1.00011558                      0.01%
 * 8653        17        509                1                            ** 0.00%
 * 8654        17        509                0.999884446                     0.01%
 * 8655        17        509                0.99976892                      0.02%
 * 8656        18        481                1.000231054                     0.02%
 * 8657        18        481                1.000115513                     0.01%
 * 8658        18        481                1                            ** 0.00%
 * 8659        18        481                0.999884513                     0.01%
 * 8660        18        481                0.999769053                     0.02%
 * 8661        19        456                1.00034638                      0.03%
 * 8662        19        456                1.000230894                     0.02%
 * 8663        19        456                1.000115433                     0.01%
 * 8664        19        456                1                            ** 0.00%
 * 8665        19        456                0.999884593                     0.01%
 * 8666        19        456                0.999769213                     0.02%
 * 8667        19        456                0.999653859                     0.03%
 * 8668        17        510                1.000230734                     0.02%
 * 8669        17        510                1.000115354                     0.01%
 * 8670        17        510                1                            ** 0.00%
 * 8671        17        510                0.999884673                     0.01%
 * 8672        17        510                0.999769373                     0.02%
 * 8673        18        482                1.000345901                     0.03%
 * 8674        18        482                1.000230574                     0.02%
 * 8675        18        482                1.000115274                     0.01%
 * 8676        18        482                1                            ** 0.00%
 * 8677        18        482                0.999884753                     0.01%
 * 8678        18        482                0.999769532                     0.02%
 * 8679        18        482                0.999654338                     0.03%
 * 8680        19        457                1.000345622                     0.03%
 * 8681        19        457                1.000230388                     0.02%
 * 8682        19        457                1.000115181                     0.01%
 * 8683        19        457                1                            ** 0.00%
 * 8684        19        457                0.999884846                     0.01%
 * 8685        17        511                1.000230282                     0.02%
 * 8686        17        511                1.000115128                     0.01%
 * 8687        17        511                1                            ** 0.00%
 * 8688        17        511                0.999884899                     0.01%
 * 8689        17        511                0.999769824                     0.02%
 * 8690        17        511                0.999654776                     0.03%
 * 8691        18        483                1.000345185                     0.03%
 * 8692        18        483                1.000230097                     0.02%
 * 8693        18        483                1.000115035                     0.01%
 * 8694        18        483                1                            ** 0.00%
 * 8695        18        483                0.999884991                     0.01%
 * 8696        18        483                0.999770009                     0.02%
 * 8697        18        483                0.999655053                     0.03%
 * 8698        18        483                0.999540124                     0.05%
 * 8699        19        458                1.000344867                     0.03%
 * 8700        19        458                1.000229885                     0.02%
 * 8701        19        458                1.000114929                     0.01%
 * 8702        19        458                1                            ** 0.00%
 * 8703        17        512                1.000114903                     0.01%
 * 8704        17        512                1                            ** 0.00%
 * 8705        17        512                0.999885123                     0.01%
 * 8706        17        512                0.999770273                     0.02%
 * 8707        17        512                0.99965545                      0.03%
 * 8708        18        484                1.000459348                     0.05%
 * 8709        18        484                1.000344471                     0.03%
 * 8710        18        484                1.000229621                     0.02%
 * 8711        18        484                1.000114797                     0.01%
 * 8712        18        484                1                            ** 0.00%
 * 8713        18        484                0.999885229                     0.01%
 * 8714        18        484                0.999770484                     0.02%
 * 8715        18        484                0.999655766                     0.03%
 * 8716        18        484                0.999541074                     0.05%
 * 8717        17        513                1.000458873                     0.05%
 * 8718        17        513                1.000344116                     0.03%
 * 8719        17        513                1.000229384                     0.02%
 * 8720        17        513                1.000114679                     0.01%
 * 8721        17        513                1                            ** 0.00%
 * 8722        17        513                0.999885347                     0.01%
 * 8723        17        513                0.999770721                     0.02%
 * 8724        17        513                0.999656121                     0.03%
 * 8725        17        513                0.999541547                     0.05%
 * 8726        18        485                1.0004584                       0.05%
 * 8727        18        485                1.000343761                     0.03%
 * 8728        18        485                1.000229148                     0.02%
 * 8729        18        485                1.000114561                     0.01%
 * 8730        18        485                1                            ** 0.00%
 * 8731        18        485                0.999885466                     0.01%
 * 8732        18        485                0.999770957                     0.02%
 * 8733        18        485                0.999656475                     0.03%
 * 8734        17        514                1.00045798                      0.05%
 * 8735        17        514                1.000343446                     0.03%
 * 8736        17        514                1.000228938                     0.02%
 * 8737        17        514                1.000114456                     0.01%
 * 8738        17        514                1                            ** 0.00%
 * 8739        19        460                1.00011443                      0.01%
 * 8740        19        460                1                            ** 0.00%
 * 8741        19        460                0.999885597                     0.01%
 * 8742        19        460                0.999771219                     0.02%
 * 8743        19        460                0.999656868                     0.03%
 * 8744        19        460                0.999542543                     0.05%
 * 8745        18        486                1.000343053                     0.03%
 * 8746        18        486                1.000228676                     0.02%
 * 8747        18        486                1.000114325                     0.01%
 * 8748        18        486                1                            ** 0.00%
 * 8749        18        486                0.999885701                     0.01%
 * 8750        18        486                0.999771429                     0.02%
 * 8751        18        486                0.999657182                     0.03%
 * 8752        17        515                1.000342779                     0.03%
 * 8753        17        515                1.000228493                     0.02%
 * 8754        17        515                1.000114233                     0.01%
 * 8755        17        515                1                            ** 0.00%
 * 8756        17        515                0.999885793                     0.01%
 * 8757        17        515                0.999771611                     0.02%
 * 8758        19        461                1.000114181                     0.01%
 * 8759        19        461                1                            ** 0.00%
 * 8760        19        461                0.999885845                     0.01%
 * 8761        19        461                0.999771716                     0.02%
 * 8762        19        461                0.999657612                     0.03%
 * 8763        18        487                1.000342349                     0.03%
 * 8764        18        487                1.000228206                     0.02%
 * 8765        18        487                1.00011409                      0.01%
 * 8766        18        487                1                            ** 0.00%
 * 8767        18        487                0.999885936                     0.01%
 * 8768        18        487                0.999771898                     0.02%
 * 8769        17        516                1.000342114                     0.03%
 * 8770        17        516                1.00022805                      0.02%
 * 8771        17        516                1.000114012                     0.01%
 * 8772        17        516                1                            ** 0.00%
 * 8773        17        516                0.999886014                     0.01%
 * 8774        17        516                0.999772054                     0.02%
 * 8775        17        516                0.99965812                      0.03%
 * 8776        19        462                1.000227894                     0.02%
 * 8777        19        462                1.000113934                     0.01%
 * 8778        19        462                1                            ** 0.00%
 * 8779        19        462                0.999886092                     0.01%
 * 8780        19        462                0.99977221                      0.02%
 * 8781        18        488                1.000341647                     0.03%
 * 8782        18        488                1.000227739                     0.02%
 * 8783        18        488                1.000113856                     0.01%
 * 8784        18        488                1                            ** 0.00%
 * 8785        18        488                0.99988617                      0.01%
 * 8786        18        488                0.999772365                     0.02%
 * 8787        17        517                1.000227609                     0.02%
 * 8788        17        517                1.000113792                     0.01%
 * 8789        17        517                1                            ** 0.00%
 * 8790        17        517                0.999886234                     0.01%
 * 8791        17        517                0.999772495                     0.02%
 * 8792        17        517                0.999658781                     0.03%
 * 8793        19        463                1.000454907                     0.05%
 * 8794        19        463                1.000341142                     0.03%
 * 8795        19        463                1.000227402                     0.02%
 * 8796        19        463                1.000113688                     0.01%
 * 8797        19        463                1                            ** 0.00%
 * 8798        19        463                0.999886338                     0.01%
 * 8799        19        463                0.999772701                     0.02%
 * 8800        18        489                1.000227273                     0.02%
 * 8801        18        489                1.000113623                     0.01%
 * 8802        18        489                1                            ** 0.00%
 * 8803        18        489                0.999886402                     0.01%
 * 8804        17        518                1.000227169                     0.02%
 * 8805        17        518                1.000113572                     0.01%
 * 8806        17        518                1                            ** 0.00%
 * 8807        17        518                0.999886454                     0.01%
 * 8808        17        518                0.999772934                     0.02%
 * 8809        17        518                0.999659439                     0.03%
 * 8810        17        518                0.99954597                      0.05%
 * 8811        17        518                0.999432528                     0.06%
 * 8812        19        464                1.000453926                     0.05%
 * 8813        19        464                1.000340406                     0.03%
 * 8814        19        464                1.000226912                     0.02%
 * 8815        19        464                1.000113443                     0.01%
 * 8816        19        464                1                            ** 0.00%
 * 8817        19        464                0.999886583                     0.01%
 * 8818        19        464                0.999773191                     0.02%
 * 8819        18        490                1.000113392                     0.01%
 * 8820        18        490                1                            ** 0.00%
 * 8821        18        490                0.999886634                     0.01%
 * 8822        17        519                1.000113353                     0.01%
 * 8823        17        519                1                            ** 0.00%
 * 8824        17        519                0.999886673                     0.01%
 * 8825        17        519                0.999773371                     0.02%
 * 8826        17        519                0.999660095                     0.03%
 * 8827        17        519                0.999546845                     0.05%
 * 8828        17        519                0.99943362                      0.06%
 * 8829        17        519                0.999320421                     0.07%
 * 8830        19        465                1.000566251                     0.06%
 * 8831        19        465                1.00045295                      0.05%
 * 8832        19        465                1.000339674                     0.03%
 * 8833        19        465                1.000226424                     0.02%
 * 8834        19        465                1.000113199                     0.01%
 * 8835        19        465                1                            ** 0.00%
 * 8836        19        465                0.999886827                     0.01%
 * 8837        18        491                1.000113161                     0.01%
 * 8838        18        491                1                            ** 0.00%
 * 8839        17        520                1.000113135                     0.01%
 * 8840        17        520                1                            ** 0.00%
 * 8841        17        520                0.999886891                     0.01%
 * 8842        17        520                0.999773807                     0.02%
 * 8843        17        520                0.999660749                     0.03%
 * 8844        17        520                0.999547716                     0.05%
 * 8845        17        520                0.999434709                     0.06%
 * 8846        17        520                0.999321727                     0.07%
 * 8847        17        520                0.999208771                     0.08%
 * 8848        19        466                1.000678119                     0.07%
 * 8849        19        466                1.000565036                     0.06%
 * 8850        19        466                1.000451977                     0.05%
 * 8851        19        466                1.000338945                     0.03%
 * 8852        19        466                1.000225938                     0.02%
 * 8853        19        466                1.000112956                     0.01%
 * 8854        19        466                1                            ** 0.00%
 * 8855        19        466                0.999887069                     0.01%
 * 8856        18        492                1                            ** 0.00%
 * 8857        17        521                1                            ** 0.00%
 * 8858        17        521                0.999887108                     0.01%
 * 8859        17        521                0.999774241                     0.02%
 * 8860        17        521                0.9996614                       0.03%
 * 8861        17        521                0.999548584                     0.05%
 * 8862        17        521                0.999435793                     0.06%
 * 8863        17        521                0.999323028                     0.07%
 * 8864        17        521                0.999210289                     0.08%
 * 8865        17        521                0.999097575                     0.09%
 * 8866        19        467                1.000789533                     0.08%
 * 8867        19        467                1.000676666                     0.07%
 * 8868        19        467                1.000563825                     0.06%
 * 8869        19        467                1.000451009                     0.05%
 * 8870        19        467                1.000338219                     0.03%
 * 8871        19        467                1.000225454                     0.02%
 * 8872        19        467                1.000112714                     0.01%
 * 8873        19        467                1                            ** 0.00%
 * 8874        17        522                1                            ** 0.00%
 * 8875        17        522                0.999887324                     0.01%
 * 8876        17        522                0.999774673                     0.02%
 * 8877        17        522                0.999662048                     0.03%
 * 8878        17        522                0.999549448                     0.05%
 * 8879        17        522                0.999436874                     0.06%
 * 8880        17        522                0.999324324                     0.07%
 * 8881        17        522                0.9992118                       0.08%
 * 8882        17        522                0.999099302                     0.09%
 * 8883        17        523                1.000900597                     0.09%
 * 8884        17        523                1.000787933                     0.08%
 * 8885        17        523                1.000675295                     0.07%
 * 8886        17        523                1.000562683                     0.06%
 * 8887        17        523                1.000450096                     0.05%
 * 8888        17        523                1.000337534                     0.03%
 * 8889        17        523                1.000224997                     0.02%
 * 8890        17        523                1.000112486                     0.01%
 * 8891        17        523                1                            ** 0.00%
 * 8892        18        494                1                            ** 0.00%
 * 8893        18        494                0.999887552                     0.01%
 * 8894        18        494                0.999775129                     0.02%
 * 8895        18        494                0.999662732                     0.03%
 * 8896        18        494                0.99955036                      0.04%
 * 8897        18        494                0.999438013                     0.06%
 * 8898        18        494                0.999325691                     0.07%
 * 8899        18        494                0.999213395                     0.08%
 * 8900        17        524                1.000898876                     0.09%
 * 8901        17        524                1.000786428                     0.08%
 * 8902        17        524                1.000674006                     0.07%
 * 8903        17        524                1.000561608                     0.06%
 * 8904        17        524                1.000449236                     0.04%
 * 8905        17        524                1.000336889                     0.03%
 * 8906        17        524                1.000224568                     0.02%
 * 8907        17        524                1.000112271                     0.01%
 * 8908        17        524                1                            ** 0.00%
 * 8909        17        524                0.999887754                     0.01%
 * 8910        18        495                1                            ** 0.00%
 * 8911        19        469                1                            ** 0.00%
 * 8912        19        469                0.999887792                     0.01%
 * 8913        19        469                0.999775609                     0.02%
 * 8914        19        469                0.999663451                     0.03%
 * 8915        19        469                0.999551318                     0.04%
 * 8916        19        469                0.99943921                      0.06%
 * 8917        19        469                0.999327128                     0.07%
 * 8918        19        469                0.999215071                     0.08%
 * 8919        17        525                1.000672721                     0.07%
 * 8920        17        525                1.000560538                     0.06%
 * 8921        17        525                1.00044838                      0.04%
 * 8922        17        525                1.000336247                     0.03%
 * 8923        17        525                1.00022414                      0.02%
 * 8924        17        525                1.000112057                     0.01%
 * 8925        17        525                1                            ** 0.00%
 * 8926        17        525                0.999887968                     0.01%
 * 8927        18        496                1.00011202                      0.01%
 * 8928        18        496                1                            ** 0.00%
 * 8929        18        496                0.999888005                     0.01%
 * 8930        19        470                1                            ** 0.00%
 * 8931        19        470                0.99988803                      0.01%
 * 8932        19        470                0.999776086                     0.02%
 * 8933        19        470                0.999664167                     0.03%
 * 8934        19        470                0.999552272                     0.04%
 * 8935        19        470                0.999440403                     0.06%
 * 8936        17        526                1.000671441                     0.07%
 * 8937        17        526                1.000559472                     0.06%
 * 8938        17        526                1.000447527                     0.04%
 * 8939        17        526                1.000335608                     0.03%
 * 8940        17        526                1.000223714                     0.02%
 * 8941        17        526                1.000111844                     0.01%
 * 8942        17        526                1                            ** 0.00%
 * 8943        17        526                0.999888181                     0.01%
 * 8944        18        497                1.000223614                     0.02%
 * 8945        18        497                1.000111794                     0.01%
 * 8946        18        497                1                            ** 0.00%
 * 8947        18        497                0.999888231                     0.01%
 * 8948        19        471                1.000111757                     0.01%
 * 8949        19        471                1                            ** 0.00%
 * 8950        19        471                0.999888268                     0.01%
 * 8951        19        471                0.999776561                     0.02%
 * 8952        19        471                0.999664879                     0.03%
 * 8953        19        471                0.999553222                     0.04%
 * 8954        17        527                1.00055841                      0.06%
 * 8955        17        527                1.000446678                     0.04%
 * 8956        17        527                1.000334971                     0.03%
 * 8957        17        527                1.000223289                     0.02%
 * 8958        17        527                1.000111632                     0.01%
 * 8959        17        527                1                            ** 0.00%
 * 8960        17        527                0.999888393                     0.01%
 * 8961        17        527                0.999776811                     0.02%
 * 8962        18        498                1.000223164                     0.02%
 * 8963        18        498                1.00011157                      0.01%
 * 8964        18        498                1                            ** 0.00%
 * 8965        18        498                0.999888455                     0.01%
 * 8966        19        472                1.000223065                     0.02%
 * 8967        19        472                1.00011152                      0.01%
 * 8968        19        472                1                            ** 0.00%
 * 8969        19        472                0.999888505                     0.01%
 * 8970        19        472                0.999777035                     0.02%
 * 8971        19        472                0.999665589                     0.03%
 * 8972        17        528                1.000445831                     0.04%
 * 8973        17        528                1.000334336                     0.03%
 * 8974        17        528                1.000222866                     0.02%
 * 8975        17        528                1.000111421                     0.01%
 * 8976        17        528                1                            ** 0.00%
 * 8977        17        528                0.999888604                     0.01%
 * 8978        17        528                0.999777233                     0.02%
 * 8979        17        528                0.999665887                     0.03%
 * 8980        18        499                1.000222717                     0.02%
 * 8981        18        499                1.000111346                     0.01%
 * 8982        18        499                1                            ** 0.00%
 * 8983        18        499                0.999888679                     0.01%
 * 8984        18        499                0.999777382                     0.02%
 * 8985        19        473                1.000222593                     0.02%
 * 8986        19        473                1.000111284                     0.01%
 * 8987        19        473                1                            ** 0.00%
 * 8988        19        473                0.999888741                     0.01%
 * 8989        19        473                0.999777506                     0.02%
 * 8990        17        529                1.000333704                     0.03%
 * 8991        17        529                1.000222445                     0.02%
 * 8992        17        529                1.00011121                      0.01%
 * 8993        17        529                1                            ** 0.00%
 * 8994        17        529                0.999888815                     0.01%
 * 8995        17        529                0.999777654                     0.02%
 * 8996        17        529                0.999666518                     0.03%
 * 8997        18        500                1.000333444                     0.03%
 * 8998        18        500                1.000222272                     0.02%
 * 8999        18        500                1.000111123                     0.01%
 * 9000        18        500                1                            ** 0.00%
 * 9001        18        500                0.999888901                     0.01%
 * 9002        18        500                0.999777827                     0.02%
 * 9003        18        500                0.999666778                     0.03%
 * 9004        19        474                1.000222124                     0.02%
 * 9005        19        474                1.000111049                     0.01%
 * 9006        19        474                1                            ** 0.00%
 * 9007        19        474                0.999888975                     0.01%
 * 9008        17        530                1.000222025                     0.02%
 * 9009        17        530                1.000111                        0.01%
 * 9010        17        530                1                            ** 0.00%
 * 9011        17        530                0.999889025                     0.01%
 * 9012        17        530                0.999778074                     0.02%
 * 9013        17        530                0.999667147                     0.03%
 * 9014        18        501                1.000443754                     0.04%
 * 9015        18        501                1.000332779                     0.03%
 * 9016        18        501                1.000221828                     0.02%
 * 9017        18        501                1.000110902                     0.01%
 * 9018        18        501                1                            ** 0.00%
 * 9019        20        451                1.000110877                     0.01%
 * 9020        20        451                1                            ** 0.00%
 * 9021        20        451                0.999889148                     0.01%
 * 9022        20        451                0.99977832                      0.02%
 * 9023        19        475                1.000221656                     0.02%
 * 9024        19        475                1.000110816                     0.01%
 * 9025        19        475                1                            ** 0.00%
 * 9026        17        531                1.000110791                     0.01%
 * 9027        17        531                1                            ** 0.00%
 * 9028        17        531                0.999889233                     0.01%
 * 9029        17        531                0.999778492                     0.02%
 * 9030        17        531                0.999667774                     0.03%
 * 9031        17        531                0.999557081                     0.04%
 * 9032        18        502                1.00044287                      0.04%
 * 9033        18        502                1.000332116                     0.03%
 * 9034        18        502                1.000221386                     0.02%
 * 9035        18        502                1.000110681                     0.01%
 * 9036        18        502                1                            ** 0.00%
 * 9037        18        502                0.999889344                     0.01%
 * 9038        18        502                0.999778712                     0.02%
 * 9039        20        452                1.000110632                     0.01%
 * 9040        20        452                1                            ** 0.00%
 * 9041        20        452                0.999889393                     0.01%
 * 9042        20        452                0.99977881                      0.02%
 * 9043        17        532                1.000110583                     0.01%
 * 9044        17        532                1                            ** 0.00%
 * 9045        17        532                0.999889442                     0.01%
 * 9046        17        532                0.999778908                     0.02%
 * 9047        17        532                0.999668398                     0.03%
 * 9048        17        532                0.999557913                     0.04%
 * 9049        18        503                1.000552547                     0.06%
 * 9050        18        503                1.000441989                     0.04%
 * 9051        18        503                1.000331455                     0.03%
 * 9052        18        503                1.000220946                     0.02%
 * 9053        18        503                1.000110461                     0.01%
 * 9054        18        503                1                            ** 0.00%
 * 9055        18        503                0.999889564                     0.01%
 * 9056        18        503                0.999779152                     0.02%
 * 9057        18        503                0.999668764                     0.03%
 * 9058        20        453                1.000220799                     0.02%
 * 9059        20        453                1.000110387                     0.01%
 * 9060        20        453                1                            ** 0.00%
 * 9061        17        533                1                            ** 0.00%
 * 9062        17        533                0.999889649                     0.01%
 * 9063        19        477                1                            ** 0.00%
 * 9064        19        477                0.999889673                     0.01%
 * 9065        19        477                0.999779371                     0.02%
 * 9066        19        477                0.999669093                     0.03%
 * 9067        19        477                0.99955884                      0.04%
 * 9068        18        504                1.000441112                     0.04%
 * 9069        18        504                1.000330797                     0.03%
 * 9070        18        504                1.000220507                     0.02%
 * 9071        18        504                1.000110241                     0.01%
 * 9072        18        504                1                            ** 0.00%
 * 9073        18        504                0.999889783                     0.01%
 * 9074        18        504                0.99977959                      0.02%
 * 9075        17        534                1.000330579                     0.03%
 * 9076        17        534                1.000220361                     0.02%
 * 9077        17        534                1.000110169                     0.01%
 * 9078        17        534                1                            ** 0.00%
 * 9079        17        534                0.999889856                     0.01%
 * 9080        20        454                1                            ** 0.00%
 * 9081        19        478                1.00011012                      0.01%
 * 9082        19        478                1                            ** 0.00%
 * 9083        19        478                0.999889904                     0.01%
 * 9084        19        478                0.999779833                     0.02%
 * 9085        19        478                0.999669785                     0.03%
 * 9086        19        478                0.999559762                     0.04%
 * 9087        18        505                1.000330142                     0.03%
 * 9088        18        505                1.00022007                      0.02%
 * 9089        18        505                1.000110023                     0.01%
 * 9090        18        505                1                            ** 0.00%
 * 9091        18        505                0.999890001                     0.01%
 * 9092        18        505                0.999780026                     0.02%
 * 9093        17        535                1.000219949                     0.02%
 * 9094        17        535                1.000109963                     0.01%
 * 9095        17        535                1                            ** 0.00%
 * 9096        17        535                0.999890062                     0.01%
 * 9097        17        535                0.999780147                     0.02%
 * 9098        20        455                1.000219829                     0.02%
 * 9099        20        455                1.000109902                     0.01%
 * 9100        20        455                1                            ** 0.00%
 * 9101        19        479                1                            ** 0.00%
 * 9102        19        479                0.999890134                     0.01%
 * 9103        19        479                0.999780292                     0.02%
 * 9104        19        479                0.999670475                     0.03%
 * 9105        18        506                1.000329489                     0.03%
 * 9106        18        506                1.000219635                     0.02%
 * 9107        18        506                1.000109806                     0.01%
 * 9108        18        506                1                            ** 0.00%
 * 9109        18        506                0.999890218                     0.01%
 * 9110        18        506                0.999780461                     0.02%
 * 9111        17        536                1.000109757                     0.01%
 * 9112        17        536                1                            ** 0.00%
 * 9113        17        536                0.999890267                     0.01%
 * 9114        17        536                0.999780557                     0.02%
 * 9115        17        536                0.999670872                     0.03%
 * 9116        19        480                1.000438789                     0.04%
 * 9117        19        480                1.000329056                     0.03%
 * 9118        19        480                1.000219346                     0.02%
 * 9119        19        480                1.000109661                     0.01%
 * 9120        19        480                1                            ** 0.00%
 * 9121        19        480                0.999890363                     0.01%
 * 9122        19        480                0.99978075                      0.02%
 * 9123        18        507                1.000328839                     0.03%
 * 9124        18        507                1.000219202                     0.02%
 * 9125        18        507                1.000109589                     0.01%
 * 9126        18        507                1                            ** 0.00%
 * 9127        18        507                0.999890435                     0.01%
 * 9128        17        537                1.000109553                     0.01%
 * 9129        17        537                1                            ** 0.00%
 * 9130        17        537                0.999890471                     0.01%
 * 9131        17        537                0.999780966                     0.02%
 * 9132        17        537                0.999671485                     0.03%
 * 9133        17        537                0.999562028                     0.04%
 * 9134        19        481                1.000547405                     0.05%
 * 9135        19        481                1.000437876                     0.04%
 * 9136        19        481                1.000328371                     0.03%
 * 9137        19        481                1.00021889                      0.02%
 * 9138        19        481                1.000109433                     0.01%
 * 9139        19        481                1                            ** 0.00%
 * 9140        20        457                1                            ** 0.00%
 * 9141        20        457                0.999890603                     0.01%
 * 9142        18        508                1.000218771                     0.02%
 * 9143        18        508                1.000109373                     0.01%
 * 9144        18        508                1                            ** 0.00%
 * 9145        18        508                0.999890651                     0.01%
 * 9146        17        538                1                            ** 0.00%
 * 9147        17        538                0.999890675                     0.01%
 * 9148        17        538                0.999781373                     0.02%
 * 9149        17        538                0.999672095                     0.03%
 * 9150        17        538                0.999562842                     0.04%
 * 9151        17        538                0.999453612                     0.05%
 * 9152        17        538                0.999344406                     0.07%
 * 9153        19        482                1.000546269                     0.05%
 * 9154        19        482                1.000436967                     0.04%
 * 9155        19        482                1.00032769                      0.03%
 * 9156        19        482                1.000218436                     0.02%
 * 9157        19        482                1.000109206                     0.01%
 * 9158        19        482                1                            ** 0.00%
 * 9159        19        482                0.999890818                     0.01%
 * 9160        20        458                1                            ** 0.00%
 * 9161        18        509                1.000109158                     0.01%
 * 9162        18        509                1                            ** 0.00%
 * 9163        17        539                1                            ** 0.00%
 * 9164        17        539                0.999890877                     0.01%
 * 9165        17        539                0.999781779                     0.02%
 * 9166        17        539                0.999672703                     0.03%
 * 9167        17        539                0.999563652                     0.04%
 * 9168        17        539                0.999454625                     0.05%
 * 9169        17        539                0.999345621                     0.07%
 * 9170        17        539                0.999236641                     0.08%
 * 9171        19        483                1.000654236                     0.07%
 * 9172        19        483                1.000545137                     0.05%
 * 9173        19        483                1.000436062                     0.04%
 * 9174        19        483                1.000327011                     0.03%
 * 9175        19        483                1.000217984                     0.02%
 * 9176        19        483                1.00010898                      0.01%
 * 9177        19        483                1                            ** 0.00%
 * 9178        19        483                0.999891044                     0.01%
 * 9179        17        540                1.000108944                     0.01%
 * 9180        17        540                1                            ** 0.00%
 * 9181        17        540                0.999891079                     0.01%
 * 9182        17        540                0.999782183                     0.02%
 * 9183        17        540                0.999673309                     0.03%
 * 9184        17        540                0.99956446                      0.04%
 * 9185        17        540                0.999455634                     0.05%
 * 9186        17        540                0.999346832                     0.07%
 * 9187        17        540                0.999238054                     0.08%
 * 9188        17        540                0.999129299                     0.09%
 * 9189        19        484                1.00076178                      0.08%
 * 9190        19        484                1.000652884                     0.07%
 * 9191        19        484                1.00054401                      0.05%
 * 9192        19        484                1.000435161                     0.04%
 * 9193        19        484                1.000326335                     0.03%
 * 9194        19        484                1.000217533                     0.02%
 * 9195        19        484                1.000108755                     0.01%
 * 9196        19        484                1                            ** 0.00%
 * 9197        17        541                1                            ** 0.00%
 * 9198        18        511                1                            ** 0.00%
 * 9199        18        511                0.999891293                     0.01%
 * 9200        20        460                1                            ** 0.00%
 * 9201        20        460                0.999891316                     0.01%
 * 9202        20        460                0.999782656                     0.02%
 * 9203        20        460                0.999674019                     0.03%
 * 9204        20        460                0.999565406                     0.04%
 * 9205        20        460                0.999456817                     0.05%
 * 9206        20        460                0.999348251                     0.07%
 * 9207        17        542                1.000760291                     0.08%
 * 9208        17        542                1.000651607                     0.07%
 * 9209        17        542                1.000542947                     0.05%
 * 9210        17        542                1.000434311                     0.04%
 * 9211        17        542                1.000325698                     0.03%
 * 9212        17        542                1.000217108                     0.02%
 * 9213        17        542                1.000108542                     0.01%
 * 9214        17        542                1                            ** 0.00%
 * 9215        19        485                1                            ** 0.00%
 * 9216        18        512                1                            ** 0.00%
 * 9217        18        512                0.999891505                     0.01%
 * 9218        18        512                0.999783033                     0.02%
 * 9219        20        461                1.000108472                     0.01%
 * 9220        20        461                1                            ** 0.00%
 * 9221        20        461                0.999891552                     0.01%
 * 9222        20        461                0.999783127                     0.02%
 * 9223        20        461                0.999674726                     0.03%
 * 9224        20        461                0.999566349                     0.04%
 * 9225        20        461                0.999457995                     0.05%
 * 9226        17        543                1.000541947                     0.05%
 * 9227        17        543                1.00043351                      0.04%
 * 9228        17        543                1.000325098                     0.03%
 * 9229        17        543                1.000216708                     0.02%
 * 9230        17        543                1.000108342                     0.01%
 * 9231        17        543                1                            ** 0.00%
 * 9232        17        543                0.999891681                     0.01%
 * 9233        18        513                1.000108307                     0.01%
 * 9234        18        513                1                            ** 0.00%
 * 9235        18        513                0.999891716                     0.01%
 * 9236        18        513                0.999783456                     0.02%
 * 9237        18        513                0.999675219                     0.03%
 * 9238        20        462                1.000216497                     0.02%
 * 9239        20        462                1.000108237                     0.01%
 * 9240        20        462                1                            ** 0.00%
 * 9241        20        462                0.999891787                     0.01%
 * 9242        20        462                0.999783597                     0.02%
 * 9243        20        462                0.99967543                      0.03%
 * 9244        17        544                1.000432713                     0.04%
 * 9245        17        544                1.0003245                       0.03%
 * 9246        17        544                1.00021631                      0.02%
 * 9247        17        544                1.000108143                     0.01%
 * 9248        17        544                1                            ** 0.00%
 * 9249        17        544                0.99989188                      0.01%
 * 9250        17        544                0.999783784                     0.02%
 * 9251        18        514                1.000108096                     0.01%
 * 9252        18        514                1                            ** 0.00%
 * 9253        19        487                1                            ** 0.00%
 * 9254        19        487                0.999891939                     0.01%
 * 9255        19        487                0.999783901                     0.02%
 * 9256        19        487                0.999675886                     0.03%
 * 9257        20        463                1.000324079                     0.03%
 * 9258        20        463                1.000216029                     0.02%
 * 9259        20        463                1.000108003                     0.01%
 * 9260        20        463                1                            ** 0.00%
 * 9261        20        463                0.99989202                      0.01%
 * 9262        20        463                0.999784064                     0.02%
 * 9263        17        545                1.000215913                     0.02%
 * 9264        17        545                1.000107945                     0.01%
 * 9265        17        545                1                            ** 0.00%
 * 9266        17        545                0.999892079                     0.01%
 * 9267        17        545                0.99978418                      0.02%
 * 9268        18        515                1.000215796                     0.02%
 * 9269        18        515                1.000107887                     0.01%
 * 9270        18        515                1                            ** 0.00%
 * 9271        18        515                0.999892137                     0.01%
 * 9272        19        488                1                            ** 0.00%
 * 9273        19        488                0.99989216                      0.01%
 * 9274        19        488                0.999784343                     0.02%
 * 9275        19        488                0.99967655                      0.03%
 * 9276        19        488                0.99956878                      0.04%
 * 9277        20        464                1.00032338                      0.03%
 * 9278        20        464                1.000215564                     0.02%
 * 9279        20        464                1.00010777                      0.01%
 * 9280        20        464                1                            ** 0.00%
 * 9281        17        546                1.000107747                     0.01%
 * 9282        17        546                1                            ** 0.00%
 * 9283        17        546                0.999892276                     0.01%
 * 9284        17        546                0.999784576                     0.02%
 * 9285        17        546                0.999676898                     0.03%
 * 9286        18        516                1.000215378                     0.02%
 * 9287        18        516                1.000107677                     0.01%
 * 9288        18        516                1                            ** 0.00%
 * 9289        18        516                0.999892346                     0.01%
 * 9290        19        489                1.000107643                     0.01%
 * 9291        19        489                1                            ** 0.00%
 * 9292        19        489                0.999892381                     0.01%
 * 9293        19        489                0.999784784                     0.02%
 * 9294        19        489                0.999677211                     0.03%
 * 9295        17        547                1.000430339                     0.04%
 * 9296        17        547                1.000322719                     0.03%
 * 9297        17        547                1.000215123                     0.02%
 * 9298        17        547                1.00010755                      0.01%
 * 9299        17        547                1                            ** 0.00%
 * 9300        20        465                1                            ** 0.00%
 * 9301        20        465                0.999892485                     0.01%
 * 9302        20        465                0.999784992                     0.02%
 * 9303        18        517                1.000322477                     0.03%
 * 9304        18        517                1.000214961                     0.02%
 * 9305        18        517                1.000107469                     0.01%
 * 9306        18        517                1                            ** 0.00%
 * 9307        18        517                0.999892554                     0.01%
 * 9308        18        517                0.999785131                     0.02%
 * 9309        19        490                1.000107423                     0.01%
 * 9310        19        490                1                            ** 0.00%
 * 9311        19        490                0.9998926                       0.01%
 * 9312        19        490                0.999785223                     0.02%
 * 9313        17        548                1.00032213                      0.03%
 * 9314        17        548                1.000214731                     0.02%
 * 9315        17        548                1.000107354                     0.01%
 * 9316        17        548                1                            ** 0.00%
 * 9317        17        548                0.999892669                     0.01%
 * 9318        17        548                0.999785362                     0.02%
 * 9319        20        466                1.000107308                     0.01%
 * 9320        20        466                1                            ** 0.00%
 * 9321        20        466                0.999892715                     0.01%
 * 9322        18        518                1.000214546                     0.02%
 * 9323        18        518                1.000107262                     0.01%
 * 9324        18        518                1                            ** 0.00%
 * 9325        18        518                0.999892761                     0.01%
 * 9326        18        518                0.999785546                     0.02%
 * 9327        19        491                1.000214431                     0.02%
 * 9328        19        491                1.000107204                     0.01%
 * 9329        19        491                1                            ** 0.00%
 * 9330        19        491                0.999892819                     0.01%
 * 9331        17        549                1.000214339                     0.02%
 * 9332        17        549                1.000107158                     0.01%
 * 9333        17        549                1                            ** 0.00%
 * 9334        17        549                0.999892865                     0.01%
 * 9335        17        549                0.999785753                     0.02%
 * 9336        17        549                0.999678663                     0.03%
 * 9337        20        467                1.000321302                     0.03%
 * 9338        20        467                1.000214179                     0.02%
 * 9339        20        467                1.000107078                     0.01%
 * 9340        20        467                1                            ** 0.00%
 * 9341        18        519                1.000107055                     0.01%
 * 9342        18        519                1                            ** 0.00%
 * 9343        18        519                0.999892968                     0.01%
 * 9344        18        519                0.999785959                     0.02%
 * 9345        18        519                0.999678973                     0.03%
 * 9346        19        492                1.000213995                     0.02%
 * 9347        19        492                1.000106986                     0.01%
 * 9348        19        492                1                            ** 0.00%
 * 9349        19        492                0.999893037                     0.01%
 * 9350        17        550                1                            ** 0.00%
 * 9351        17        550                0.99989306                      0.01%
 * 9352        17        550                0.999786142                     0.02%
 * 9353        17        550                0.999679247                     0.03%
 * 9354        17        550                0.999572375                     0.04%
 * 9355        18        520                1.000534474                     0.05%
 * 9356        18        520                1.000427533                     0.04%
 * 9357        18        520                1.000320616                     0.03%
 * 9358        18        520                1.000213721                     0.02%
 * 9359        18        520                1.000106849                     0.01%
 * 9360        18        520                1                            ** 0.00%
 * 9361        18        520                0.999893174                     0.01%
 * 9362        18        520                0.99978637                      0.02%
 * 9363        18        520                0.99967959                      0.03%
 * 9364        19        493                1.000320376                     0.03%
 * 9365        19        493                1.000213561                     0.02%
 * 9366        19        493                1.000106769                     0.01%
 * 9367        19        493                1                            ** 0.00%
 * 9368        19        493                0.999893254                     0.01%
 * 9369        19        493                0.99978653                      0.02%
 * 9370        19        493                0.999679829                     0.03%
 * 9371        19        493                0.999573151                     0.04%
 * 9372        19        493                0.999466496                     0.05%
 * 9373        18        521                1.000533447                     0.05%
 * 9374        18        521                1.000426712                     0.04%
 * 9375        18        521                1.00032                         0.03%
 * 9376        18        521                1.000213311                     0.02%
 * 9377        18        521                1.000106644                     0.01%
 * 9378        18        521                1                            ** 0.00%
 * 9379        18        521                0.999893379                     0.01%
 * 9380        20        469                1                            ** 0.00%
 * 9381        20        469                0.999893402                     0.01%
 * 9382        20        469                0.999786826                     0.02%
 * 9383        19        494                1.000319727                     0.03%
 * 9384        19        494                1.000213129                     0.02%
 * 9385        19        494                1.000106553                     0.01%
 * 9386        19        494                1                            ** 0.00%
 * 9387        19        494                0.99989347                      0.01%
 * 9388        19        494                0.999786962                     0.02%
 * 9389        19        494                0.999680477                     0.03%
 * 9390        19        494                0.999574015                     0.04%
 * 9391        18        522                1.000532425                     0.05%
 * 9392        18        522                1.000425894                     0.04%
 * 9393        18        522                1.000319387                     0.03%
 * 9394        18        522                1.000212902                     0.02%
 * 9395        18        522                1.00010644                      0.01%
 * 9396        18        522                1                            ** 0.00%
 * 9397        18        522                0.999893583                     0.01%
 * 9398        18        522                0.999787189                     0.02%
 * 9399        20        470                1.000106394                     0.01%
 * 9400        20        470                1                            ** 0.00%
 * 9401        20        470                0.999893628                     0.01%
 * 9402        20        470                0.999787279                     0.02%
 * 9403        19        495                1.000212698                     0.02%
 * 9404        19        495                1.000106338                     0.01%
 * 9405        19        495                1                            ** 0.00%
 * 9406        19        495                0.999893685                     0.01%
 * 9407        19        495                0.999787392                     0.02%
 * 9408        19        495                0.999681122                     0.03%
 * 9409        19        495                0.999574875                     0.04%
 * 9410        18        523                1.00042508                      0.04%
 * 9411        18        523                1.000318776                     0.03%
 * 9412        18        523                1.000212495                     0.02%
 * 9413        18        523                1.000106236                     0.01%
 * 9414        18        523                1                            ** 0.00%
 * 9415        18        523                0.999893787                     0.01%
 * 9416        18        523                0.999787596                     0.02%
 * 9417        18        523                0.999681427                     0.03%
 * 9418        20        471                1.000212359                     0.02%
 * 9419        20        471                1.000106168                     0.01%
 * 9420        20        471                1                            ** 0.00%
 * 9421        20        471                0.999893854                     0.01%
 * 9422        19        496                1.000212269                     0.02%
 * 9423        19        496                1.000106123                     0.01%
 * 9424        19        496                1                            ** 0.00%
 * 9425        19        496                0.999893899                     0.01%
 * 9426        19        496                0.999787821                     0.02%
 * 9427        19        496                0.999681765                     0.03%
 * 9428        18        524                1.000424268                     0.04%
 * 9429        18        524                1.000318167                     0.03%
 * 9430        18        524                1.000212089                     0.02%
 * 9431        18        524                1.000106033                     0.01%
 * 9432        18        524                1                            ** 0.00%
 * 9433        18        524                0.999893989                     0.01%
 * 9434        18        524                0.999788001                     0.02%
 * 9435        18        524                0.999682035                     0.03%
 * 9436        20        472                1.000423908                     0.04%
 * 9437        20        472                1.000317898                     0.03%
 * 9438        20        472                1.000211909                     0.02%
 * 9439        20        472                1.000105943                     0.01%
 * 9440        20        472                1                            ** 0.00%
 * 9441        20        472                0.999894079                     0.01%
 * 9442        19        497                1.00010591                      0.01%
 * 9443        19        497                1                            ** 0.00%
 * 9444        19        497                0.999894113                     0.01%
 * 9445        19        497                0.999788248                     0.02%
 * 9446        19        497                0.999682405                     0.03%
 * 9447        18        525                1.000317561                     0.03%
 * 9448        18        525                1.000211685                     0.02%
 * 9449        18        525                1.000105831                     0.01%
 * 9450        18        525                1                            ** 0.00%
 * 9451        18        525                0.999894191                     0.01%
 * 9452        18        525                0.999788405                     0.02%
 * 9453        18        525                0.99968264                      0.03%
 * 9454        18        525                0.999576899                     0.04%
 * 9455        18        525                0.999471179                     0.05%
 * 9456        20        473                1.000423012                     0.04%
 * 9457        20        473                1.000317225                     0.03%
 * 9458        20        473                1.000211461                     0.02%
 * 9459        20        473                1.000105719                     0.01%
 * 9460        20        473                1                            ** 0.00%
 * 9461        19        498                1.000105697                     0.01%
 * 9462        19        498                1                            ** 0.00%
 * 9463        19        498                0.999894325                     0.01%
 * 9464        19        498                0.999788673                     0.02%
 * 9465        19        498                0.999683043                     0.03%
 * 9466        18        526                1.000211282                     0.02%
 * 9467        18        526                1.00010563                      0.01%
 * 9468        18        526                1                            ** 0.00%
 * 9469        18        526                0.999894392                     0.01%
 * 9470        21        451                1.000105597                     0.01%
 * 9471        21        451                1                            ** 0.00%
 * 9472        21        451                0.999894426                     0.01%
 * 9473        21        451                0.999788874                     0.02%
 * 9474        21        451                0.999683344                     0.03%
 * 9475        21        451                0.999577836                     0.04%
 * 9476        20        474                1.000422119                     0.04%
 * 9477        20        474                1.000316556                     0.03%
 * 9478        20        474                1.000211015                     0.02%
 * 9479        20        474                1.000105496                     0.01%
 * 9480        20        474                1                            ** 0.00%
 * 9481        19        499                1                            ** 0.00%
 * 9482        19        499                0.999894537                     0.01%
 * 9483        19        499                0.999789096                     0.02%
 * 9484        18        527                1.000210881                     0.02%
 * 9485        18        527                1.00010543                      0.01%
 * 9486        18        527                1                            ** 0.00%
 * 9487        18        527                0.999894593                     0.01%
 * 9488        18        527                0.999789207                     0.02%
 * 9489        21        452                1.000316156                     0.03%
 * 9490        21        452                1.000210748                     0.02%
 * 9491        21        452                1.000105363                     0.01%
 * 9492        21        452                1                            ** 0.00%
 * 9493        21        452                0.999894659                     0.01%
 * 9494        21        452                0.999789341                     0.02%
 * 9495        21        452                0.999684044                     0.03%
 * 9496        19        500                1.00042123                      0.04%
 * 9497        19        500                1.000315889                     0.03%
 * 9498        19        500                1.000210571                     0.02%
 * 9499        19        500                1.000105274                     0.01%
 * 9500        19        500                1                            ** 0.00%
 * 9501        19        500                0.999894748                     0.01%
 * 9502        18        528                1.000210482                     0.02%
 * 9503        18        528                1.00010523                      0.01%
 * 9504        18        528                1                            ** 0.00%
 * 9505        18        528                0.999894792                     0.01%
 * 9506        18        528                0.999789607                     0.02%
 * 9507        18        528                0.999684443                     0.03%
 * 9508        18        528                0.999579302                     0.04%
 * 9509        21        453                1.000420654                     0.04%
 * 9510        21        453                1.000315457                     0.03%
 * 9511        21        453                1.000210283                     0.02%
 * 9512        21        453                1.00010513                      0.01%
 * 9513        21        453                1                            ** 0.00%
 * 9514        21        453                0.999894892                     0.01%
 * 9515        21        453                0.999789806                     0.02%
 * 9516        21        453                0.999684741                     0.03%
 * 9517        19        501                1.00021015                      0.02%
 * 9518        19        501                1.000105064                     0.01%
 * 9519        19        501                1                            ** 0.00%
 * 9520        20        476                1                            ** 0.00%
 * 9521        20        476                0.999894969                     0.01%
 * 9522        18        529                1                            ** 0.00%
 * 9523        18        529                0.999894991                     0.01%
 * 9524        18        529                0.999790004                     0.02%
 * 9525        18        529                0.999685039                     0.03%
 * 9526        18        529                0.999580097                     0.04%
 * 9527        18        529                0.999475176                     0.05%
 * 9528        18        529                0.999370277                     0.06%
 * 9529        21        454                1.000524714                     0.05%
 * 9530        21        454                1.000419727                     0.04%
 * 9531        21        454                1.000314762                     0.03%
 * 9532        21        454                1.00020982                      0.02%
 * 9533        21        454                1.000104899                     0.01%
 * 9534        21        454                1                            ** 0.00%
 * 9535        21        454                0.999895123                     0.01%
 * 9536        21        454                0.999790268                     0.02%
 * 9537        19        502                1.000104855                     0.01%
 * 9538        19        502                1                            ** 0.00%
 * 9539        18        530                1.000104833                     0.01%
 * 9540        18        530                1                            ** 0.00%
 * 9541        18        530                0.999895189                     0.01%
 * 9542        18        530                0.9997904                       0.02%
 * 9543        18        530                0.999685633                     0.03%
 * 9544        18        530                0.999580889                     0.04%
 * 9545        18        530                0.999476166                     0.05%
 * 9546        18        530                0.999371464                     0.06%
 * 9547        18        530                0.999266785                     0.07%
 * 9548        21        455                1.000733138                     0.07%
 * 9549        21        455                1.000628338                     0.06%
 * 9550        21        455                1.00052356                      0.05%
 * 9551        21        455                1.000418804                     0.04%
 * 9552        21        455                1.00031407                      0.03%
 * 9553        21        455                1.000209358                     0.02%
 * 9554        21        455                1.000104668                     0.01%
 * 9555        21        455                1                            ** 0.00%
 * 9556        21        455                0.999895354                     0.01%
 * 9557        19        503                1                            ** 0.00%
 * 9558        18        531                1                            ** 0.00%
 * 9559        18        531                0.999895387                     0.01%
 * 9560        20        478                1                            ** 0.00%
 * 9561        20        478                0.999895408                     0.01%
 * 9562        20        478                0.999790839                     0.02%
 * 9563        20        478                0.999686291                     0.03%
 * 9564        20        478                0.999581765                     0.04%
 * 9565        20        478                0.999477261                     0.05%
 * 9566        20        478                0.999372779                     0.06%
 * 9567        20        478                0.999268318                     0.07%
 * 9568        18        532                1.00083612                      0.08%
 * 9569        18        532                1.000731529                     0.07%
 * 9570        18        532                1.000626959                     0.06%
 * 9571        18        532                1.000522411                     0.05%
 * 9572        18        532                1.000417885                     0.04%
 * 9573        18        532                1.000313381                     0.03%
 * 9574        18        532                1.000208899                     0.02%
 * 9575        18        532                1.000104439                     0.01%
 * 9576        18        532                1                            ** 0.00%
 * 9577        18        532                0.999895583                     0.01%
 * 9578        18        532                0.999791188                     0.02%
 * 9579        20        479                1.000104395                     0.01%
 * 9580        20        479                1                            ** 0.00%
 * 9581        20        479                0.999895627                     0.01%
 * 9582        20        479                0.999791275                     0.02%
 * 9583        20        479                0.999686946                     0.03%
 * 9584        20        479                0.999582638                     0.04%
 * 9585        20        479                0.999478352                     0.05%
 * 9586        20        479                0.999374087                     0.06%
 * 9587        20        479                0.999269845                     0.07%
 * 9588        18        533                1.000625782                     0.06%
 * 9589        18        533                1.000521431                     0.05%
 * 9590        18        533                1.000417101                     0.04%
 * 9591        18        533                1.000312793                     0.03%
 * 9592        18        533                1.000208507                     0.02%
 * 9593        18        533                1.000104243                     0.01%
 * 9594        18        533                1                            ** 0.00%
 * 9595        19        505                1                            ** 0.00%
 * 9596        19        505                0.99989579                      0.01%
 * 9597        21        457                1                            ** 0.00%
 * 9598        21        457                0.999895812                     0.01%
 * 9599        20        480                1.000104178                     0.01%
 * 9600        20        480                1                            ** 0.00%
 * 9601        20        480                0.999895844                     0.01%
 * 9602        20        480                0.99979171                      0.02%
 * 9603        20        480                0.999687598                     0.03%
 * 9604        20        480                0.999583507                     0.04%
 * 9605        20        480                0.999479438                     0.05%
 * 9606        20        480                0.99937539                      0.06%
 * 9607        19        506                1.000728635                     0.07%
 * 9608        19        506                1.00062448                      0.06%
 * 9609        19        506                1.000520346                     0.05%
 * 9610        19        506                1.000416233                     0.04%
 * 9611        19        506                1.000312142                     0.03%
 * 9612        19        506                1.000208073                     0.02%
 * 9613        19        506                1.000104026                     0.01%
 * 9614        19        506                1                            ** 0.00%
 * 9615        19        506                0.999895996                     0.01%
 * 9616        21        458                1.000207987                     0.02%
 * 9617        21        458                1.000103983                     0.01%
 * 9618        21        458                1                            ** 0.00%
 * 9619        20        481                1.000103961                     0.01%
 * 9620        20        481                1                            ** 0.00%
 * 9621        20        481                0.999896061                     0.01%
 * 9622        20        481                0.999792143                     0.02%
 * 9623        20        481                0.999688247                     0.03%
 * 9624        20        481                0.999584372                     0.04%
 * 9625        20        481                0.999480519                     0.05%
 * 9626        20        481                0.999376688                     0.06%
 * 9627        19        507                1.000623247                     0.06%
 * 9628        19        507                1.000519319                     0.05%
 * 9629        19        507                1.000415412                     0.04%
 * 9630        19        507                1.000311526                     0.03%
 * 9631        19        507                1.000207663                     0.02%
 * 9632        19        507                1.000103821                     0.01%
 * 9633        19        507                1                            ** 0.00%
 * 9634        19        507                0.999896201                     0.01%
 * 9635        19        507                0.999792423                     0.02%
 * 9636        21        459                1.000311333                     0.03%
 * 9637        21        459                1.000207533                     0.02%
 * 9638        21        459                1.000103756                     0.01%
 * 9639        21        459                1                            ** 0.00%
 * 9640        20        482                1                            ** 0.00%
 * 9641        20        482                0.999896276                     0.01%
 * 9642        20        482                0.999792574                     0.02%
 * 9643        20        482                0.999688893                     0.03%
 * 9644        20        482                0.999585234                     0.04%
 * 9645        20        482                0.999481597                     0.05%
 * 9646        19        508                1.000622019                     0.06%
 * 9647        19        508                1.000518296                     0.05%
 * 9648        19        508                1.000414594                     0.04%
 * 9649        19        508                1.000310913                     0.03%
 * 9650        19        508                1.000207254                     0.02%
 * 9651        19        508                1.000103616                     0.01%
 * 9652        19        508                1                            ** 0.00%
 * 9653        19        508                0.999896405                     0.01%
 * 9654        19        508                0.999792832                     0.02%
 * 9655        19        508                0.99968928                      0.03%
 * 9656        20        483                1.00041425                      0.04%
 * 9657        20        483                1.000310655                     0.03%
 * 9658        20        483                1.000207082                     0.02%
 * 9659        20        483                1.00010353                      0.01%
 * 9660        20        483                1                            ** 0.00%
 * 9661        20        483                0.999896491                     0.01%
 * 9662        20        483                0.999793004                     0.02%
 * 9663        20        483                0.999689537                     0.03%
 * 9664        20        483                0.999586093                     0.04%
 * 9665        20        483                0.999482669                     0.05%
 * 9666        19        509                1.000517277                     0.05%
 * 9667        19        509                1.000413779                     0.04%
 * 9668        19        509                1.000310302                     0.03%
 * 9669        19        509                1.000206847                     0.02%
 * 9670        19        509                1.000103413                     0.01%
 * 9671        19        509                1                            ** 0.00%
 * 9672        19        509                0.999896609                     0.01%
 * 9673        19        509                0.999793239                     0.02%
 * 9674        19        509                0.99968989                      0.03%
 * 9675        19        509                0.999586563                     0.04%
 * 9676        20        484                1.000413394                     0.04%
 * 9677        20        484                1.000310013                     0.03%
 * 9678        20        484                1.000206654                     0.02%
 * 9679        20        484                1.000103316                     0.01%
 * 9680        20        484                1                            ** 0.00%
 * 9681        21        461                1                            ** 0.00%
 * 9682        21        461                0.999896716                     0.01%
 * 9683        21        461                0.999793452                     0.02%
 * 9684        21        461                0.999690211                     0.03%
 * 9685        21        461                0.99958699                      0.04%
 * 9686        19        510                1.000412967                     0.04%
 * 9687        19        510                1.000309693                     0.03%
 * 9688        19        510                1.000206441                     0.02%
 * 9689        19        510                1.00010321                      0.01%
 * 9690        19        510                1                            ** 0.00%
 * 9691        19        510                0.999896811                     0.01%
 * 9692        19        510                0.999793644                     0.02%
 * 9693        19        510                0.999690498                     0.03%
 * 9694        19        510                0.999587374                     0.04%
 * 9695        20        485                1.00051573                      0.05%
 * 9696        20        485                1.000412541                     0.04%
 * 9697        20        485                1.000309374                     0.03%
 * 9698        20        485                1.000206228                     0.02%
 * 9699        20        485                1.000103103                     0.01%
 * 9700        20        485                1                            ** 0.00%
 * 9701        21        462                1.000103082                     0.01%
 * 9702        21        462                1                            ** 0.00%
 * 9703        21        462                0.999896939                     0.01%
 * 9704        21        462                0.999793899                     0.02%
 * 9705        21        462                0.999690881                     0.03%
 * 9706        19        511                1.000309087                     0.03%
 * 9707        19        511                1.000206037                     0.02%
 * 9708        19        511                1.000103008                     0.01%
 * 9709        19        511                1                            ** 0.00%
 * 9710        19        511                0.999897013                     0.01%
 * 9711        19        511                0.999794048                     0.02%
 * 9712        19        511                0.999691104                     0.03%
 * 9713        19        511                0.999588181                     0.04%
 * 9714        19        511                0.999485279                     0.05%
 * 9715        20        486                1.000514668                     0.05%
 * 9716        20        486                1.000411692                     0.04%
 * 9717        20        486                1.000308737                     0.03%
 * 9718        20        486                1.000205804                     0.02%
 * 9719        20        486                1.000102891                     0.01%
 * 9720        20        486                1                            ** 0.00%
 * 9721        20        486                0.99989713                      0.01%
 * 9722        21        463                1.000102859                     0.01%
 * 9723        21        463                1                            ** 0.00%
 * 9724        21        463                0.999897162                     0.01%
 * 9725        21        463                0.999794344                     0.02%
 * 9726        19        512                1.000205634                     0.02%
 * 9727        19        512                1.000102807                     0.01%
 * 9728        19        512                1                            ** 0.00%
 * 9729        19        512                0.999897215                     0.01%
 * 9730        19        512                0.99979445                      0.02%
 * 9731        19        512                0.999691707                     0.03%
 * 9732        19        512                0.999588985                     0.04%
 * 9733        19        512                0.999486284                     0.05%
 * 9734        20        487                1.000616396                     0.06%
 * 9735        20        487                1.000513611                     0.05%
 * 9736        20        487                1.000410846                     0.04%
 * 9737        20        487                1.000308103                     0.03%
 * 9738        20        487                1.000205381                     0.02%
 * 9739        20        487                1.00010268                      0.01%
 * 9740        20        487                1                            ** 0.00%
 * 9741        20        487                0.999897341                     0.01%
 * 9742        21        464                1.000205297                     0.02%
 * 9743        21        464                1.000102638                     0.01%
 * 9744        21        464                1                            ** 0.00%
 * 9745        21        464                0.999897383                     0.01%
 * 9746        19        513                1.000102606                     0.01%
 * 9747        19        513                1                            ** 0.00%
 * 9748        19        513                0.999897415                     0.01%
 * 9749        19        513                0.999794851                     0.02%
 * 9750        19        513                0.999692308                     0.03%
 * 9751        19        513                0.999589786                     0.04%
 * 9752        19        513                0.999487285                     0.05%
 * 9753        19        513                0.999384805                     0.06%
 * 9754        20        488                1.000615132                     0.06%
 * 9755        20        488                1.000512558                     0.05%
 * 9756        20        488                1.000410004                     0.04%
 * 9757        20        488                1.000307472                     0.03%
 * 9758        20        488                1.00020496                      0.02%
 * 9759        20        488                1.00010247                      0.01%
 * 9760        20        488                1                            ** 0.00%
 * 9761        20        488                0.999897551                     0.01%
 * 9762        20        488                0.999795124                     0.02%
 * 9763        21        465                1.000204855                     0.02%
 * 9764        21        465                1.000102417                     0.01%
 * 9765        21        465                1                            ** 0.00%
 * 9766        19        514                1                            ** 0.00%
 * 9767        19        514                0.999897614                     0.01%
 * 9768        19        514                0.99979525                      0.02%
 * 9769        19        514                0.999692906                     0.03%
 * 9770        19        514                0.999590583                     0.04%
 * 9771        19        514                0.999488282                     0.05%
 * 9772        19        514                0.999386001                     0.06%
 * 9773        20        489                1.000716259                     0.07%
 * 9774        20        489                1.000613874                     0.06%
 * 9775        20        489                1.000511509                     0.05%
 * 9776        20        489                1.000409165                     0.04%
 * 9777        20        489                1.000306843                     0.03%
 * 9778        20        489                1.000204541                     0.02%
 * 9779        20        489                1.00010226                      0.01%
 * 9780        20        489                1                            ** 0.00%
 * 9781        20        489                0.999897761                     0.01%
 * 9782        20        489                0.999795543                     0.02%
 * 9783        19        515                1.000204436                     0.02%
 * 9784        19        515                1.000102208                     0.01%
 * 9785        19        515                1                            ** 0.00%
 * 9786        21        466                1                            ** 0.00%
 * 9787        21        466                0.999897824                     0.01%
 * 9788        21        466                0.999795668                     0.02%
 * 9789        21        466                0.999693534                     0.03%
 * 9790        21        466                0.99959142                      0.04%
 * 9791        21        466                0.999489327                     0.05%
 * 9792        21        466                0.999387255                     0.06%
 * 9793        20        490                1.000714796                     0.07%
 * 9794        20        490                1.00061262                      0.06%
 * 9795        20        490                1.000510465                     0.05%
 * 9796        20        490                1.00040833                      0.04%
 * 9797        20        490                1.000306216                     0.03%
 * 9798        20        490                1.000204123                     0.02%
 * 9799        20        490                1.000102051                     0.01%
 * 9800        20        490                1                            ** 0.00%
 * 9801        20        490                0.99989797                      0.01%
 * 9802        19        516                1.00020404                      0.02%
 * 9803        19        516                1.00010201                      0.01%
 * 9804        19        516                1                            ** 0.00%
 * 9805        19        516                0.999898011                     0.01%
 * 9806        21        467                1.000101978                     0.01%
 * 9807        21        467                1                            ** 0.00%
 * 9808        21        467                0.999898042                     0.01%
 * 9809        21        467                0.999796106                     0.02%
 * 9810        21        467                0.99969419                      0.03%
 * 9811        21        467                0.999592294                     0.04%
 * 9812        21        467                0.99949042                      0.05%
 * 9813        21        467                0.999388566                     0.06%
 * 9814        20        491                1.000611372                     0.06%
 * 9815        20        491                1.000509424                     0.05%
 * 9816        20        491                1.000407498                     0.04%
 * 9817        20        491                1.000305592                     0.03%
 * 9818        20        491                1.000203707                     0.02%
 * 9819        20        491                1.000101843                     0.01%
 * 9820        20        491                1                            ** 0.00%
 * 9821        20        491                0.999898177                     0.01%
 * 9822        19        517                1.000101812                     0.01%
 * 9823        19        517                1                            ** 0.00%
 * 9824        19        517                0.999898208                     0.01%
 * 9825        19        517                0.999796438                     0.02%
 * 9826        21        468                1.000203542                     0.02%
 * 9827        21        468                1.00010176                      0.01%
 * 9828        21        468                1                            ** 0.00%
 * 9829        21        468                0.99989826                      0.01%
 * 9830        21        468                0.999796541                     0.02%
 * 9831        21        468                0.999694843                     0.03%
 * 9832        21        468                0.999593165                     0.04%
 * 9833        21        468                0.999491508                     0.05%
 * 9834        20        492                1.000610128                     0.06%
 * 9835        20        492                1.000508388                     0.05%
 * 9836        20        492                1.000406669                     0.04%
 * 9837        20        492                1.000304971                     0.03%
 * 9838        20        492                1.000203293                     0.02%
 * 9839        20        492                1.000101636                     0.01%
 * 9840        20        492                1                            ** 0.00%
 * 9841        19        518                1.000101616                     0.01%
 * 9842        19        518                1                            ** 0.00%
 * 9843        19        518                0.999898405                     0.01%
 * 9844        19        518                0.999796831                     0.02%
 * 9845        19        518                0.999695277                     0.03%
 * 9846        21        469                1.000304692                     0.03%
 * 9847        21        469                1.000203108                     0.02%
 * 9848        21        469                1.000101543                     0.01%
 * 9849        21        469                1                            ** 0.00%
 * 9850        21        469                0.999898477                     0.01%
 * 9851        21        469                0.999796975                     0.02%
 * 9852        21        469                0.999695493                     0.03%
 * 9853        21        469                0.999594032                     0.04%
 * 9854        21        469                0.999492592                     0.05%
 * 9855        20        493                1.000507357                     0.05%
 * 9856        20        493                1.000405844                     0.04%
 * 9857        20        493                1.000304352                     0.03%
 * 9858        20        493                1.000202881                     0.02%
 * 9859        20        493                1.00010143                      0.01%
 * 9860        20        493                1                            ** 0.00%
 * 9861        19        519                1                            ** 0.00%
 * 9862        19        519                0.999898601                     0.01%
 * 9863        19        519                0.999797222                     0.02%
 * 9864        19        519                0.999695864                     0.03%
 * 9865        19        519                0.999594526                     0.04%
 * 9866        21        470                1.000405433                     0.04%
 * 9867        21        470                1.000304044                     0.03%
 * 9868        21        470                1.000202675                     0.02%
 * 9869        21        470                1.000101327                     0.01%
 * 9870        21        470                1                            ** 0.00%
 * 9871        21        470                0.999898693                     0.01%
 * 9872        21        470                0.999797407                     0.02%
 * 9873        21        470                0.999696141                     0.03%
 * 9874        21        470                0.999594896                     0.04%
 * 9875        19        520                1.000506329                     0.05%
 * 9876        19        520                1.000405022                     0.04%
 * 9877        19        520                1.000303736                     0.03%
 * 9878        19        520                1.00020247                      0.02%
 * 9879        19        520                1.000101225                     0.01%
 * 9880        19        520                1                            ** 0.00%
 * 9881        19        520                0.999898796                     0.01%
 * 9882        19        520                0.999797612                     0.02%
 * 9883        19        520                0.999696448                     0.03%
 * 9884        19        520                0.999595306                     0.04%
 * 9885        19        520                0.999494183                     0.05%
 * 9886        21        471                1.000505766                     0.05%
 * 9887        21        471                1.000404572                     0.04%
 * 9888        21        471                1.000303398                     0.03%
 * 9889        21        471                1.000202245                     0.02%
 * 9890        21        471                1.000101112                     0.01%
 * 9891        21        471                1                            ** 0.00%
 * 9892        21        471                0.999898908                     0.01%
 * 9893        21        471                0.999797837                     0.02%
 * 9894        21        471                0.999696786                     0.03%
 * 9895        19        521                1.000404245                     0.04%
 * 9896        19        521                1.000303153                     0.03%
 * 9897        19        521                1.000202081                     0.02%
 * 9898        19        521                1.000101031                     0.01%
 * 9899        19        521                1                            ** 0.00%
 * 9900        20        495                1                            ** 0.00%
 * 9901        20        495                0.999899                        0.01%
 * 9902        20        495                0.999798021                     0.02%
 * 9903        20        495                0.999697061                     0.03%
 * 9904        20        495                0.999596123                     0.04%
 * 9905        20        495                0.999495204                     0.05%
 * 9906        21        472                1.000605694                     0.06%
 * 9907        21        472                1.000504694                     0.05%
 * 9908        21        472                1.000403714                     0.04%
 * 9909        21        472                1.000302755                     0.03%
 * 9910        21        472                1.000201816                     0.02%
 * 9911        21        472                1.000100898                     0.01%
 * 9912        21        472                1                            ** 0.00%
 * 9913        21        472                0.999899122                     0.01%
 * 9914        21        472                0.999798265                     0.02%
 * 9915        19        522                1.000302572                     0.03%
 * 9916        19        522                1.000201694                     0.02%
 * 9917        19        522                1.000100837                     0.01%
 * 9918        19        522                1                            ** 0.00%
 * 9919        20        496                1.000100817                     0.01%
 * 9920        20        496                1                            ** 0.00%
 * 9921        22        451                1.000100796                     0.01%
 * 9922        22        451                1                            ** 0.00%
 * 9923        22        451                0.999899224                     0.01%
 * 9924        22        451                0.999798468                     0.02%
 * 9925        22        451                0.999697733                     0.03%
 * 9926        22        451                0.999597018                     0.04%
 * 9927        22        451                0.999496323                     0.05%
 * 9928        21        473                1.000503626                     0.05%
 * 9929        21        473                1.00040286                      0.04%
 * 9930        21        473                1.000302115                     0.03%
 * 9931        21        473                1.00020139                      0.02%
 * 9932        21        473                1.000100685                     0.01%
 * 9933        21        473                1                            ** 0.00%
 * 9934        21        473                0.999899336                     0.01%
 * 9935        19        523                1.000201309                     0.02%
 * 9936        19        523                1.000100644                     0.01%
 * 9937        19        523                1                            ** 0.00%
 * 9938        19        523                0.999899376                     0.01%
 * 9939        20        497                1.000100614                     0.01%
 * 9940        20        497                1                            ** 0.00%
 * 9941        20        497                0.999899406                     0.01%
 * 9942        22        452                1.000201167                     0.02%
 * 9943        22        452                1.000100573                     0.01%
 * 9944        22        452                1                            ** 0.00%
 * 9945        22        452                0.999899447                     0.01%
 * 9946        22        452                0.999798914                     0.02%
 * 9947        22        452                0.999698402                     0.03%
 * 9948        22        452                0.999597909                     0.04%
 * 9949        21        474                1.000502563                     0.05%
 * 9950        21        474                1.00040201                      0.04%
 * 9951        21        474                1.000301477                     0.03%
 * 9952        21        474                1.000200965                     0.02%
 * 9953        21        474                1.000100472                     0.01%
 * 9954        21        474                1                            ** 0.00%
 * 9955        19        524                1.000100452                     0.01%
 * 9956        19        524                1                            ** 0.00%
 * 9957        19        524                0.999899568                     0.01%
 * 9958        20        498                1.000200844                     0.02%
 * 9959        20        498                1.000100412                     0.01%
 * 9960        20        498                1                            ** 0.00%
 * 9961        20        498                0.999899608                     0.01%
 * 9962        20        498                0.999799237                     0.02%
 * 9963        22        453                1.000301114                     0.03%
 * 9964        22        453                1.000200723                     0.02%
 * 9965        22        453                1.000100351                     0.01%
 * 9966        22        453                1                            ** 0.00%
 * 9967        22        453                0.999899669                     0.01%
 * 9968        22        453                0.999799358                     0.02%
 * 9969        22        453                0.999699067                     0.03%
 * 9970        22        453                0.999598796                     0.04%
 * 9971        19        525                1.000401163                     0.04%
 * 9972        19        525                1.000300842                     0.03%
 * 9973        19        525                1.000200541                     0.02%
 * 9974        19        525                1.000100261                     0.01%
 * 9975        19        525                1                            ** 0.00%
 * 9976        19        525                0.999899759                     0.01%
 * 9977        19        525                0.999799539                     0.02%
 * 9978        20        499                1.000200441                     0.02%
 * 9979        20        499                1.00010021                      0.01%
 * 9980        20        499                1                            ** 0.00%
 * 9981        20        499                0.99989981                      0.01%
 * 9982        20        499                0.999799639                     0.02%
 * 9983        20        499                0.999699489                     0.03%
 * 9984        22        454                1.000400641                     0.04%
 * 9985        22        454                1.000300451                     0.03%
 * 9986        22        454                1.00020028                      0.02%
 * 9987        22        454                1.00010013                      0.01%
 * 9988        22        454                1                            ** 0.00%
 * 9989        22        454                0.99989989                      0.01%
 * 9990        22        454                0.9997998                       0.02%
 * 9991        19        526                1.00030027                      0.03%
 * 9992        19        526                1.00020016                      0.02%
 * 9993        19        526                1.00010007                      0.01%
 * 9994        19        526                1                            ** 0.00%
 * 9995        21        476                1.00010005                      0.01%
 * 9996        21        476                1                            ** 0.00%
 * 9997        21        476                0.99989997                      0.01%
 * 9998        20        500                1.00020004                      0.02%
 * 9999        20        500                1.00010001                      0.01%
 * 10000       20        500                1                            ** 0.00%
 * 10001       20        500                0.99990001                      0.01%
 * 10002       20        500                0.99980004                      0.02%
 * 10003       20        500                0.99970009                      0.03%
 * 10004       20        500                0.99960016                      0.04%
 * 10005       22        455                1.00049975                      0.05%
 * 10006       22        455                1.00039976                      0.04%
 * 10007       22        455                1.00029979                      0.03%
 * 10008       22        455                1.00019984                      0.02%
 * 10009       22        455                1.00009991                      0.01%
 * 10010       22        455                1                            ** 0.00%
 * 10011       22        455                0.99990011                      0.01%
 * 10012       19        527                1.00009988                      0.01%
 * 10013       19        527                1                            ** 0.00%
 * 10014       19        527                0.99990014                      0.01%
 * 10015       21        477                1.0001997                       0.02%
 * 10016       21        477                1.00009984                      0.01%
 * 10017       21        477                1                            ** 0.00%
 * 10018       21        477                0.99990018                      0.01%
 * 10019       20        501                1.00009981                      0.01%
 * 10020       20        501                1                            ** 0.00%
 * 10021       20        501                0.99990021                      0.01%
 * 10022       20        501                0.999800439                     0.02%
 * 10023       20        501                0.999700688                     0.03%
 * 10024       20        501                0.999600958                     0.04%
 * 10025       20        501                0.999501247                     0.05%
 * 10026       19        528                1.000598444                     0.06%
 * 10027       19        528                1.000498654                     0.05%
 * 10028       19        528                1.000398883                     0.04%
 * 10029       19        528                1.000299133                     0.03%
 * 10030       19        528                1.000199402                     0.02%
 * 10031       19        528                1.000099691                     0.01%
 * 10032       19        528                1                            ** 0.00%
 * 10033       19        528                0.999900329                     0.01%
 * 10034       19        528                0.999800678                     0.02%
 * 10035       21        478                1.000298954                     0.03%
 * 10036       21        478                1.000199283                     0.02%
 * 10037       21        478                1.000099631                     0.01%
 * 10038       21        478                1                            ** 0.00%
 * 10039       20        502                1.000099612                     0.01%
 * 10040       20        502                1                            ** 0.00%
 * 10041       20        502                0.999900408                     0.01%
 * 10042       20        502                0.999800836                     0.02%
 * 10043       20        502                0.999701284                     0.03%
 * 10044       20        502                0.999601752                     0.04%
 * 10045       20        502                0.99950224                      0.05%
 * 10046       19        529                1.000497711                     0.05%
 * 10047       19        529                1.000398129                     0.04%
 * 10048       19        529                1.000298567                     0.03%
 * 10049       19        529                1.000199025                     0.02%
 * 10050       19        529                1.000099502                     0.01%
 * 10051       19        529                1                            ** 0.00%
 * 10052       19        529                0.999900517                     0.01%
 * 10053       22        457                1.000099473                     0.01%
 * 10054       22        457                1                            ** 0.00%
 * 10055       22        457                0.999900547                     0.01%
 * 10056       22        457                0.999801114                     0.02%
 * 10057       21        479                1.000198866                     0.02%
 * 10058       21        479                1.000099423                     0.01%
 * 10059       21        479                1                            ** 0.00%
 * 10060       20        503                1                            ** 0.00%
 * 10061       20        503                0.999900606                     0.01%
 * 10062       20        503                0.999801232                     0.02%
 * 10063       20        503                0.999701878                     0.03%
 * 10064       20        503                0.999602544                     0.04%
 * 10065       19        530                1.000496771                     0.05%
 * 10066       19        530                1.000397377                     0.04%
 * 10067       19        530                1.000298003                     0.03%
 * 10068       19        530                1.000198649                     0.02%
 * 10069       19        530                1.000099315                     0.01%
 * 10070       19        530                1                            ** 0.00%
 * 10071       19        530                0.999900705                     0.01%
 * 10072       19        530                0.99980143                      0.02%
 * 10073       22        458                1.000297826                     0.03%
 * 10074       22        458                1.000198531                     0.02%
 * 10075       22        458                1.000099256                     0.01%
 * 10076       22        458                1                            ** 0.00%
 * 10077       22        458                0.999900764                     0.01%
 * 10078       20        504                1.000198452                     0.02%
 * 10079       20        504                1.000099216                     0.01%
 * 10080       20        504                1                            ** 0.00%
 * 10081       20        504                0.999900803                     0.01%
 * 10082       20        504                0.999801627                     0.02%
 * 10083       20        504                0.99970247                      0.03%
 * 10084       20        504                0.999603332                     0.04%
 * 10085       19        531                1.000396629                     0.04%
 * 10086       19        531                1.000297442                     0.03%
 * 10087       19        531                1.000198275                     0.02%
 * 10088       19        531                1.000099128                     0.01%
 * 10089       19        531                1                            ** 0.00%
 * 10090       19        531                0.999900892                     0.01%
 * 10091       19        531                0.999801804                     0.02%
 * 10092       19        531                0.999702735                     0.03%
 * 10093       19        531                0.999603686                     0.04%
 * 10094       22        459                1.000396275                     0.04%
 * 10095       22        459                1.000297177                     0.03%
 * 10096       22        459                1.000198098                     0.02%
 * 10097       22        459                1.000099039                     0.01%
 * 10098       22        459                1                            ** 0.00%
 * 10099       20        505                1.00009902                      0.01%
 * 10100       20        505                1                            ** 0.00%
 * 10101       21        481                1                            ** 0.00%
 * 10102       21        481                0.99990101                      0.01%
 * 10103       21        481                0.999802039                     0.02%
 * 10104       21        481                0.999703088                     0.03%
 * 10105       19        532                1.000296883                     0.03%
 * 10106       19        532                1.000197902                     0.02%
 * 10107       19        532                1.000098941                     0.01%
 * 10108       19        532                1                            ** 0.00%
 * 10109       19        532                0.999901078                     0.01%
 * 10110       19        532                0.999802176                     0.02%
 * 10111       19        532                0.999703293                     0.03%
 * 10112       19        532                0.99960443                      0.04%
 * 10113       19        532                0.999505587                     0.05%
 * 10114       20        506                1.000593237                     0.06%
 * 10115       20        506                1.000494315                     0.05%
 * 10116       20        506                1.000395413                     0.04%
 * 10117       20        506                1.000296531                     0.03%
 * 10118       20        506                1.000197668                     0.02%
 * 10119       20        506                1.000098824                     0.01%
 * 10120       20        506                1                            ** 0.00%
 * 10121       21        482                1.000098804                     0.01%
 * 10122       21        482                1                            ** 0.00%
 * 10123       21        482                0.999901215                     0.01%
 * 10124       21        482                0.99980245                      0.02%
 * 10125       19        533                1.000197531                     0.02%
 * 10126       19        533                1.000098756                     0.01%
 * 10127       19        533                1                            ** 0.00%
 * 10128       19        533                0.999901264                     0.01%
 * 10129       19        533                0.999802547                     0.02%
 * 10130       19        533                0.99970385                      0.03%
 * 10131       19        533                0.999605172                     0.04%
 * 10132       19        533                0.999506514                     0.05%
 * 10133       19        533                0.999407875                     0.06%
 * 10134       20        507                1.000592066                     0.06%
 * 10135       20        507                1.00049334                      0.05%
 * 10136       20        507                1.000394633                     0.04%
 * 10137       20        507                1.000295946                     0.03%
 * 10138       20        507                1.000197278                     0.02%
 * 10139       20        507                1.000098629                     0.01%
 * 10140       20        507                1                            ** 0.00%
 * 10141       22        461                1.00009861                      0.01%
 * 10142       22        461                1                            ** 0.00%
 * 10143       21        483                1                            ** 0.00%
 * 10144       21        483                0.99990142                      0.01%
 * 10145       19        534                1.000098571                     0.01%
 * 10146       19        534                1                            ** 0.00%
 * 10147       19        534                0.999901449                     0.01%
 * 10148       19        534                0.999802917                     0.02%
 * 10149       19        534                0.999704404                     0.03%
 * 10150       19        534                0.999605911                     0.04%
 * 10151       19        534                0.999507438                     0.05%
 * 10152       19        534                0.999408983                     0.06%
 * 10153       20        508                1.000689451                     0.07%
 * 10154       20        508                1.0005909                       0.06%
 * 10155       20        508                1.000492368                     0.05%
 * 10156       20        508                1.000393856                     0.04%
 * 10157       20        508                1.000295363                     0.03%
 * 10158       20        508                1.000196889                     0.02%
 * 10159       20        508                1.000098435                     0.01%
 * 10160       20        508                1                            ** 0.00%
 * 10161       20        508                0.999901584                     0.01%
 * 10162       21        484                1.000196812                     0.02%
 * 10163       21        484                1.000098396                     0.01%
 * 10164       21        484                1                            ** 0.00%
 * 10165       19        535                1                            ** 0.00%
 * 10166       19        535                0.999901633                     0.01%
 * 10167       19        535                0.999803285                     0.02%
 * 10168       19        535                0.999704957                     0.03%
 * 10169       19        535                0.999606648                     0.04%
 * 10170       19        535                0.999508358                     0.05%
 * 10171       19        535                0.999410088                     0.06%
 * 10172       19        535                0.999311836                     0.07%
 * 10173       20        509                1.000688096                     0.07%
 * 10174       20        509                1.000589739                     0.06%
 * 10175       20        509                1.0004914                       0.05%
 * 10176       20        509                1.000393082                     0.04%
 * 10177       20        509                1.000294782                     0.03%
 * 10178       20        509                1.000196502                     0.02%
 * 10179       20        509                1.000098241                     0.01%
 * 10180       20        509                1                            ** 0.00%
 * 10181       20        509                0.999901778                     0.01%
 * 10182       19        536                1.000196425                     0.02%
 * 10183       19        536                1.000098203                     0.01%
 * 10184       19        536                1                            ** 0.00%
 * 10185       21        485                1                            ** 0.00%
 * 10186       22        463                1                            ** 0.00%
 * 10187       22        463                0.999901836                     0.01%
 * 10188       22        463                0.999803691                     0.02%
 * 10189       22        463                0.999705565                     0.03%
 * 10190       22        463                0.999607458                     0.04%
 * 10191       22        463                0.999509371                     0.05%
 * 10192       22        463                0.999411303                     0.06%
 * 10193       20        510                1.000686746                     0.07%
 * 10194       20        510                1.000588582                     0.06%
 * 10195       20        510                1.000490436                     0.05%
 * 10196       20        510                1.000392311                     0.04%
 * 10197       20        510                1.000294204                     0.03%
 * 10198       20        510                1.000196117                     0.02%
 * 10199       20        510                1.000098049                     0.01%
 * 10200       20        510                1                            ** 0.00%
 * 10201       20        510                0.99990197                      0.01%
 * 10202       19        537                1.00009802                      0.01%
 * 10203       19        537                1                            ** 0.00%
 * 10204       19        537                0.999901999                     0.01%
 * 10205       21        486                1.000097991                     0.01%
 * 10206       21        486                1                            ** 0.00%
 * 10207       22        464                1.000097972                     0.01%
 * 10208       22        464                1                            ** 0.00%
 * 10209       22        464                0.999902047                     0.01%
 * 10210       22        464                0.999804114                     0.02%
 * 10211       22        464                0.999706199                     0.03%
 * 10212       22        464                0.999608304                     0.04%
 * 10213       22        464                0.999510428                     0.05%
 * 10214       20        511                1.000587429                     0.06%
 * 10215       20        511                1.000489476                     0.05%
 * 10216       20        511                1.000391543                     0.04%
 * 10217       20        511                1.000293628                     0.03%
 * 10218       20        511                1.000195733                     0.02%
 * 10219       20        511                1.000097857                     0.01%
 * 10220       20        511                1                            ** 0.00%
 * 10221       19        538                1.000097838                     0.01%
 * 10222       19        538                1                            ** 0.00%
 * 10223       19        538                0.999902181                     0.01%
 * 10224       19        538                0.999804382                     0.02%
 * 10225       21        487                1.000195599                     0.02%
 * 10226       21        487                1.00009779                      0.01%
 * 10227       21        487                1                            ** 0.00%
 * 10228       21        487                0.999902229                     0.01%
 * 10229       22        465                1.000097761                     0.01%
 * 10230       22        465                1                            ** 0.00%
 * 10231       22        465                0.999902258                     0.01%
 * 10232       22        465                0.999804535                     0.02%
 * 10233       22        465                0.999706831                     0.03%
 * 10234       22        465                0.999609146                     0.04%
 * 10235       20        512                1.00048852                      0.05%
 * 10236       20        512                1.000390778                     0.04%
 * 10237       20        512                1.000293055                     0.03%
 * 10238       20        512                1.000195351                     0.02%
 * 10239       20        512                1.000097666                     0.01%
 * 10240       20        512                1                            ** 0.00%
 * 10241       19        539                1                            ** 0.00%
 * 10242       19        539                0.999902363                     0.01%
 * 10243       19        539                0.999804745                     0.02%
 * 10244       19        539                0.999707146                     0.03%
 * 10245       21        488                1.000292826                     0.03%
 * 10246       21        488                1.000195198                     0.02%
 * 10247       21        488                1.00009759                      0.01%
 * 10248       21        488                1                            ** 0.00%
 * 10249       21        488                0.99990243                      0.01%
 * 10250       22        466                1.000195122                     0.02%
 * 10251       22        466                1.000097551                     0.01%
 * 10252       22        466                1                            ** 0.00%
 * 10253       22        466                0.999902468                     0.01%
 * 10254       22        466                0.999804954                     0.02%
 * 10255       22        466                0.99970746                      0.03%
 * 10256       19        540                1.000390016                     0.04%
 * 10257       19        540                1.000292483                     0.03%
 * 10258       19        540                1.00019497                      0.02%
 * 10259       19        540                1.000097475                     0.01%
 * 10260       19        540                1                            ** 0.00%
 * 10261       19        540                0.999902544                     0.01%
 * 10262       19        540                0.999805106                     0.02%
 * 10263       19        540                0.999707688                     0.03%
 * 10264       19        540                0.999610288                     0.04%
 * 10265       21        489                1.000389674                     0.04%
 * 10266       21        489                1.000292227                     0.03%
 * 10267       21        489                1.000194799                     0.02%
 * 10268       21        489                1.00009739                      0.01%
 * 10269       21        489                1                            ** 0.00%
 * 10270       21        489                0.999902629                     0.01%
 * 10271       21        489                0.999805277                     0.02%
 * 10272       22        467                1.000194704                     0.02%
 * 10273       22        467                1.000097343                     0.01%
 * 10274       22        467                1                            ** 0.00%
 * 10275       22        467                0.999902676                     0.01%
 * 10276       22        467                0.999805372                     0.02%
 * 10277       19        541                1.000194609                     0.02%
 * 10278       19        541                1.000097295                     0.01%
 * 10279       19        541                1                            ** 0.00%
 * 10280       20        514                1                            ** 0.00%
 * 10281       20        514                0.999902733                     0.01%
 * 10282       20        514                0.999805485                     0.02%
 * 10283       20        514                0.999708256                     0.03%
 * 10284       20        514                0.999611046                     0.04%
 * 10285       21        490                1.000486145                     0.05%
 * 10286       21        490                1.000388878                     0.04%
 * 10287       21        490                1.00029163                      0.03%
 * 10288       21        490                1.000194401                     0.02%
 * 10289       21        490                1.000097191                     0.01%
 * 10290       21        490                1                            ** 0.00%
 * 10291       21        490                0.999902828                     0.01%
 * 10292       21        490                0.999805674                     0.02%
 * 10293       22        468                1.00029146                      0.03%
 * 10294       22        468                1.000194288                     0.02%
 * 10295       22        468                1.000097135                     0.01%
 * 10296       22        468                1                            ** 0.00%
 * 10297       19        542                1.000097116                     0.01%
 * 10298       19        542                1                            ** 0.00%
 * 10299       20        515                1.000097097                     0.01%
 * 10300       20        515                1                            ** 0.00%
 * 10301       20        515                0.999902922                     0.01%
 * 10302       20        515                0.999805863                     0.02%
 * 10303       20        515                0.999708823                     0.03%
 * 10304       20        515                0.999611801                     0.04%
 * 10305       20        515                0.999514799                     0.05%
 * 10306       21        491                1.000485154                     0.05%
 * 10307       21        491                1.000388086                     0.04%
 * 10308       21        491                1.000291036                     0.03%
 * 10309       21        491                1.000194005                     0.02%
 * 10310       21        491                1.000096993                     0.01%
 * 10311       21        491                1                            ** 0.00%
 * 10312       21        491                0.999903026                     0.01%
 * 10313       21        491                0.99980607                      0.02%
 * 10314       19        543                1.000290867                     0.03%
 * 10315       19        543                1.000193892                     0.02%
 * 10316       19        543                1.000096937                     0.01%
 * 10317       19        543                1                            ** 0.00%
 * 10318       22        469                1                            ** 0.00%
 * 10319       20        516                1.000096909                     0.01%
 * 10320       20        516                1                            ** 0.00%
 * 10321       20        516                0.99990311                      0.01%
 * 10322       20        516                0.999806239                     0.02%
 * 10323       20        516                0.999709387                     0.03%
 * 10324       20        516                0.999612553                     0.04%
 * 10325       20        516                0.999515738                     0.05%
 * 10326       21        492                1.000581058                     0.06%
 * 10327       21        492                1.000484168                     0.05%
 * 10328       21        492                1.000387297                     0.04%
 * 10329       21        492                1.000290444                     0.03%
 * 10330       21        492                1.000193611                     0.02%
 * 10331       21        492                1.000096796                     0.01%
 * 10332       21        492                1                            ** 0.00%
 * 10333       21        492                0.999903223                     0.01%
 * 10334       19        544                1.000193536                     0.02%
 * 10335       19        544                1.000096759                     0.01%
 * 10336       19        544                1                            ** 0.00%
 * 10337       19        544                0.99990326                      0.01%
 * 10338       20        517                1.000193461                     0.02%
 * 10339       20        517                1.000096721                     0.01%
 * 10340       20        517                1                            ** 0.00%
 * 10341       20        517                0.999903298                     0.01%
 * 10342       20        517                0.999806614                     0.02%
 * 10343       20        517                0.999709949                     0.03%
 * 10344       20        517                0.999613302                     0.04%
 * 10345       23        450                1.000483325                     0.05%
 * 10346       23        450                1.000386623                     0.04%
 * 10347       23        450                1.000289939                     0.03%
 * 10348       23        450                1.000193274                     0.02%
 * 10349       23        450                1.000096628                     0.01%
 * 10350       23        450                1                            ** 0.00%
 * 10351       23        450                0.999903391                     0.01%
 * 10352       21        493                1.0000966                       0.01%
 * 10353       21        493                1                            ** 0.00%
 * 10354       19        545                1.000096581                     0.01%
 * 10355       19        545                1                            ** 0.00%
 * 10356       19        545                0.999903438                     0.01%
 * 10357       19        545                0.999806894                     0.02%
 * 10358       20        518                1.000193087                     0.02%
 * 10359       20        518                1.000096534                     0.01%
 * 10360       20        518                1                            ** 0.00%
 * 10361       22        471                1.000096516                     0.01%
 * 10362       22        471                1                            ** 0.00%
 * 10363       22        471                0.999903503                     0.01%
 * 10364       22        471                0.999807024                     0.02%
 * 10365       22        471                0.999710564                     0.03%
 * 10366       22        471                0.999614123                     0.04%
 * 10367       22        471                0.9995177                       0.05%
 * 10368       23        451                1.000482253                     0.05%
 * 10369       23        451                1.000385765                     0.04%
 * 10370       23        451                1.000289296                     0.03%
 * 10371       23        451                1.000192845                     0.02%
 * 10372       23        451                1.000096413                     0.01%
 * 10373       23        451                1                            ** 0.00%
 * 10374       19        546                1                            ** 0.00%
 * 10375       19        546                0.999903614                     0.01%
 * 10376       19        546                0.999807247                     0.02%
 * 10377       20        519                1.000289101                     0.03%
 * 10378       20        519                1.000192715                     0.02%
 * 10379       20        519                1.000096348                     0.01%
 * 10380       20        519                1                            ** 0.00%
 * 10381       20        519                0.99990367                      0.01%
 * 10382       22        472                1.000192641                     0.02%
 * 10383       22        472                1.000096311                     0.01%
 * 10384       22        472                1                            ** 0.00%
 * 10385       22        472                0.999903707                     0.01%
 * 10386       22        472                0.999807433                     0.02%
 * 10387       22        472                0.999711177                     0.03%
 * 10388       22        472                0.99961494                      0.04%
 * 10389       19        547                1.000385023                     0.04%
 * 10390       19        547                1.000288739                     0.03%
 * 10391       19        547                1.000192474                     0.02%
 * 10392       19        547                1.000096228                     0.01%
 * 10393       19        547                1                            ** 0.00%
 * 10394       21        495                1.000096209                     0.01%
 * 10395       21        495                1                            ** 0.00%
 * 10396       23        452                1                            ** 0.00%
 * 10397       23        452                0.999903818                     0.01%
 * 10398       20        520                1.000192345                     0.02%
 * 10399       20        520                1.000096163                     0.01%
 * 10400       20        520                1                            ** 0.00%
 * 10401       20        520                0.999903855                     0.01%
 * 10402       20        520                0.999807729                     0.02%
 * 10403       22        473                1.000288378                     0.03%
 * 10404       22        473                1.000192234                     0.02%
 * 10405       22        473                1.000096108                     0.01%
 * 10406       22        473                1                            ** 0.00%
 * 10407       22        473                0.999903911                     0.01%
 * 10408       22        473                0.99980784                      0.02%
 * 10409       19        548                1.000288212                     0.03%
 * 10410       19        548                1.000192123                     0.02%
 * 10411       19        548                1.000096052                     0.01%
 * 10412       19        548                1                            ** 0.00%
 * 10413       19        548                0.999903966                     0.01%
 * 10414       21        496                1.000192049                     0.02%
 * 10415       21        496                1.000096015                     0.01%
 * 10416       21        496                1                            ** 0.00%
 * 10417       21        496                0.999904003                     0.01%
 * 10418       23        453                1.000095988                     0.01%
 * 10419       23        453                1                            ** 0.00%
 * 10420       20        521                1                            ** 0.00%
 * 10421       20        521                0.99990404                      0.01%
 * 10422       20        521                0.999808098                     0.02%
 * 10423       20        521                0.999712175                     0.03%
 * 10424       22        474                1.00038373                      0.04%
 * 10425       22        474                1.00028777                      0.03%
 * 10426       22        474                1.000191828                     0.02%
 * 10427       22        474                1.000095905                     0.01%
 * 10428       22        474                1                            ** 0.00%
 * 10429       22        474                0.999904114                     0.01%
 * 10430       19        549                1.000095877                     0.01%
 * 10431       19        549                1                            ** 0.00%
 * 10432       19        549                0.999904141                     0.01%
 * 10433       19        549                0.999808301                     0.02%
 * 10434       21        497                1.000287522                     0.03%
 * 10435       21        497                1.000191663                     0.02%
 * 10436       21        497                1.000095822                     0.01%
 * 10437       21        497                1                            ** 0.00%
 * 10438       21        497                0.999904196                     0.01%
 * 10439       20        522                1.000095795                     0.01%
 * 10440       20        522                1                            ** 0.00%
 * 10441       23        454                1.000095776                     0.01%
 * 10442       23        454                1                            ** 0.00%
 * 10443       23        454                0.999904242                     0.01%
 * 10444       23        454                0.999808502                     0.02%
 * 10445       23        454                0.999712781                     0.03%
 * 10446       19        550                1.000382922                     0.04%
 * 10447       19        550                1.000287164                     0.03%
 * 10448       19        550                1.000191424                     0.02%
 * 10449       19        550                1.000095703                     0.01%
 * 10450       19        550                1                            ** 0.00%
 * 10451       19        550                0.999904315                     0.01%
 * 10452       19        550                0.999808649                     0.02%
 * 10453       19        550                0.999713001                     0.03%
 * 10454       21        498                1.000382629                     0.04%
 * 10455       21        498                1.000286944                     0.03%
 * 10456       21        498                1.000191278                     0.02%
 * 10457       21        498                1.00009563                      0.01%
 * 10458       21        498                1                            ** 0.00%
 * 10459       20        523                1.000095611                     0.01%
 * 10460       20        523                1                            ** 0.00%
 * 10461       20        523                0.999904407                     0.01%
 * 10462       20        523                0.999808832                     0.02%
 * 10463       23        455                1.00019115                      0.02%
 * 10464       23        455                1.000095566                     0.01%
 * 10465       23        455                1                            ** 0.00%
 * 10466       23        455                0.999904453                     0.01%
 * 10467       23        455                0.999808923                     0.02%
 * 10468       23        455                0.999713412                     0.03%
 * 10469       22        476                1.00028656                      0.03%
 * 10470       22        476                1.000191022                     0.02%
 * 10471       22        476                1.000095502                     0.01%
 * 10472       22        476                1                            ** 0.00%
 * 10473       22        476                0.999904516                     0.01%
 * 10474       22        476                0.999809051                     0.02%
 * 10475       22        476                0.999713604                     0.03%
 * 10476       21        499                1.000286369                     0.03%
 * 10477       21        499                1.000190894                     0.02%
 * 10478       21        499                1.000095438                     0.01%
 * 10479       21        499                1                            ** 0.00%
 * 10480       20        524                1                            ** 0.00%
 * 10481       20        524                0.999904589                     0.01%
 * 10482       20        524                0.999809197                     0.02%
 * 10483       20        524                0.999713822                     0.03%
 * 10484       23        456                1.000381534                     0.04%
 * 10485       23        456                1.000286123                     0.03%
 * 10486       23        456                1.00019073                      0.02%
 * 10487       23        456                1.000095356                     0.01%
 * 10488       23        456                1                            ** 0.00%
 * 10489       23        456                0.999904662                     0.01%
 * 10490       23        456                0.999809342                     0.02%
 * 10491       22        477                1.000285959                     0.03%
 * 10492       22        477                1.000190621                     0.02%
 * 10493       22        477                1.000095302                     0.01%
 * 10494       22        477                1                            ** 0.00%
 * 10495       22        477                0.999904717                     0.01%
 * 10496       22        477                0.999809451                     0.02%
 * 10497       20        525                1.000285796                     0.03%
 * 10498       20        525                1.000190512                     0.02%
 * 10499       20        525                1.000095247                     0.01%
 * 10500       20        525                1                            ** 0.00%
 * 10501       20        525                0.999904771                     0.01%
 * 10502       20        525                0.99980956                      0.02%
 * 10503       20        525                0.999714367                     0.03%
 * 10504       20        525                0.999619193                     0.04%
 * 10505       20        525                0.999524036                     0.05%
 * 10506       23        457                1.000475919                     0.05%
 * 10507       23        457                1.000380699                     0.04%
 * 10508       23        457                1.000285497                     0.03%
 * 10509       23        457                1.000190313                     0.02%
 * 10510       23        457                1.000095147                     0.01%
 * 10511       23        457                1                            ** 0.00%
 * 10512       23        457                0.999904871                     0.01%
 * 10513       23        457                0.999809759                     0.02%
 * 10514       22        478                1.000190223                     0.02%
 * 10515       22        478                1.000095102                     0.01%
 * 10516       22        478                1                            ** 0.00%
 * 10517       22        478                0.999904916                     0.01%
 * 10518       20        526                1.00019015                      0.02%
 * 10519       20        526                1.000095066                     0.01%
 * 10520       20        526                1                            ** 0.00%
 * 10521       21        501                1                            ** 0.00%
 * 10522       21        501                0.999904961                     0.01%
 * 10523       21        501                0.99980994                      0.02%
 * 10524       21        501                0.999714937                     0.03%
 * 10525       21        501                0.999619952                     0.04%
 * 10526       21        501                0.999524986                     0.05%
 * 10527       21        501                0.999430037                     0.06%
 * 10528       23        458                1.000569909                     0.06%
 * 10529       23        458                1.000474879                     0.05%
 * 10530       23        458                1.000379867                     0.04%
 * 10531       23        458                1.000284873                     0.03%
 * 10532       23        458                1.000189897                     0.02%
 * 10533       23        458                1.00009494                      0.01%
 * 10534       23        458                1                            ** 0.00%
 * 10535       23        458                0.999905078                     0.01%
 * 10536       22        479                1.000189825                     0.02%
 * 10537       22        479                1.000094904                     0.01%
 * 10538       22        479                1                            ** 0.00%
 * 10539       20        527                1.000094886                     0.01%
 * 10540       20        527                1                            ** 0.00%
 * 10541       21        502                1.000094868                     0.01%
 * 10542       21        502                1                            ** 0.00%
 * 10543       21        502                0.99990515                      0.01%
 * 10544       21        502                0.999810319                     0.02%
 * 10545       21        502                0.999715505                     0.03%
 * 10546       21        502                0.999620709                     0.04%
 * 10547       21        502                0.999525932                     0.05%
 * 10548       21        502                0.999431172                     0.06%
 * 10549       21        502                0.99933643                      0.07%
 * 10550       23        459                1.000663507                     0.07%
 * 10551       23        459                1.000568666                     0.06%
 * 10552       23        459                1.000473844                     0.05%
 * 10553       23        459                1.000379039                     0.04%
 * 10554       23        459                1.000284252                     0.03%
 * 10555       23        459                1.000189484                     0.02%
 * 10556       23        459                1.000094733                     0.01%
 * 10557       23        459                1                            ** 0.00%
 * 10558       23        459                0.999905285                     0.01%
 * 10559       20        528                1.000094706                     0.01%
 * 10560       20        528                1                            ** 0.00%
 * 10561       20        528                0.999905312                     0.01%
 * 10562       21        503                1.000094679                     0.01%
 * 10563       21        503                1                            ** 0.00%
 * 10564       21        503                0.999905339                     0.01%
 * 10565       21        503                0.999810696                     0.02%
 * 10566       21        503                0.99971607                      0.03%
 * 10567       21        503                0.999621463                     0.04%
 * 10568       21        503                0.999526874                     0.05%
 * 10569       21        503                0.999432302                     0.06%
 * 10570       21        503                0.999337748                     0.07%
 * 10571       21        503                0.999243213                     0.08%
 * 10572       20        529                1.000756716                     0.08%
 * 10573       20        529                1.000662064                     0.07%
 * 10574       20        529                1.00056743                      0.06%
 * 10575       20        529                1.000472813                     0.05%
 * 10576       20        529                1.000378215                     0.04%
 * 10577       20        529                1.000283634                     0.03%
 * 10578       20        529                1.000189072                     0.02%
 * 10579       20        529                1.000094527                     0.01%
 * 10580       20        529                1                            ** 0.00%
 * 10581       22        481                1.000094509                     0.01%
 * 10582       22        481                1                            ** 0.00%
 * 10583       21        504                1.000094491                     0.01%
 * 10584       21        504                1                            ** 0.00%
 * 10585       21        504                0.999905527                     0.01%
 * 10586       21        504                0.999811071                     0.02%
 * 10587       21        504                0.999716634                     0.03%
 * 10588       21        504                0.999622214                     0.04%
 * 10589       21        504                0.999527812                     0.05%
 * 10590       21        504                0.999433428                     0.06%
 * 10591       21        504                0.999339061                     0.07%
 * 10592       20        530                1.000755287                     0.08%
 * 10593       20        530                1.000660814                     0.07%
 * 10594       20        530                1.000566358                     0.06%
 * 10595       20        530                1.000471921                     0.05%
 * 10596       20        530                1.000377501                     0.04%
 * 10597       20        530                1.000283099                     0.03%
 * 10598       20        530                1.000188715                     0.02%
 * 10599       20        530                1.000094349                     0.01%
 * 10600       20        530                1                            ** 0.00%
 * 10601       20        530                0.999905669                     0.01%
 * 10602       23        461                1.000094322                     0.01%
 * 10603       23        461                1                            ** 0.00%
 * 10604       22        482                1                            ** 0.00%
 * 10605       21        505                1                            ** 0.00%
 * 10606       21        505                0.999905714                     0.01%
 * 10607       21        505                0.999811445                     0.02%
 * 10608       21        505                0.999717195                     0.03%
 * 10609       21        505                0.999622962                     0.04%
 * 10610       21        505                0.999528746                     0.05%
 * 10611       21        505                0.999434549                     0.06%
 * 10612       21        505                0.999340369                     0.07%
 * 10613       20        531                1.000659568                     0.07%
 * 10614       20        531                1.000565291                     0.06%
 * 10615       20        531                1.000471032                     0.05%
 * 10616       20        531                1.00037679                      0.04%
 * 10617       20        531                1.000282566                     0.03%
 * 10618       20        531                1.000188359                     0.02%
 * 10619       20        531                1.000094171                     0.01%
 * 10620       20        531                1                            ** 0.00%
 * 10621       20        531                0.999905847                     0.01%
 * 10622       20        531                0.999811712                     0.02%
 * 10623       21        506                1.000282406                     0.03%
 * 10624       21        506                1.000188253                     0.02%
 * 10625       21        506                1.000094118                     0.01%
 * 10626       21        506                1                            ** 0.00%
 * 10627       21        506                0.9999059                       0.01%
 * 10628       21        506                0.999811818                     0.02%
 * 10629       21        506                0.999717753                     0.03%
 * 10630       21        506                0.999623706                     0.04%
 * 10631       21        506                0.999529677                     0.05%
 * 10632       21        506                0.999435666                     0.06%
 * 10633       20        532                1.000658328                     0.07%
 * 10634       20        532                1.000564228                     0.06%
 * 10635       20        532                1.000470146                     0.05%
 * 10636       20        532                1.000376081                     0.04%
 * 10637       20        532                1.000282034                     0.03%
 * 10638       20        532                1.000188005                     0.02%
 * 10639       20        532                1.000093994                     0.01%
 * 10640       20        532                1                            ** 0.00%
 * 10641       20        532                0.999906024                     0.01%
 * 10642       20        532                0.999812065                     0.02%
 * 10643       20        532                0.999718125                     0.03%
 * 10644       21        507                1.000281849                     0.03%
 * 10645       21        507                1.000187882                     0.02%
 * 10646       21        507                1.000093932                     0.01%
 * 10647       21        507                1                            ** 0.00%
 * 10648       22        484                1                            ** 0.00%
 * 10649       23        463                1                            ** 0.00%
 * 10650       23        463                0.999906103                     0.01%
 * 10651       23        463                0.999812224                     0.02%
 * 10652       23        463                0.999718363                     0.03%
 * 10653       23        463                0.999624519                     0.04%
 * 10654       23        463                0.999530693                     0.05%
 * 10655       20        533                1.000469263                     0.05%
 * 10656       20        533                1.000375375                     0.04%
 * 10657       20        533                1.000281505                     0.03%
 * 10658       20        533                1.000187652                     0.02%
 * 10659       20        533                1.000093817                     0.01%
 * 10660       20        533                1                            ** 0.00%
 * 10661       20        533                0.9999062                       0.01%
 * 10662       20        533                0.999812418                     0.02%
 * 10663       20        533                0.999718653                     0.03%
 * 10664       21        508                1.000375094                     0.04%
 * 10665       21        508                1.000281294                     0.03%
 * 10666       21        508                1.000187512                     0.02%
 * 10667       21        508                1.000093747                     0.01%
 * 10668       21        508                1                            ** 0.00%
 * 10669       22        485                1.000093729                     0.01%
 * 10670       22        485                1                            ** 0.00%
 * 10671       23        464                1.000093712                     0.01%
 * 10672       23        464                1                            ** 0.00%
 * 10673       23        464                0.999906306                     0.01%
 * 10674       23        464                0.999812629                     0.02%
 * 10675       23        464                0.99971897                      0.03%
 * 10676       20        534                1.000374672                     0.04%
 * 10677       20        534                1.000280978                     0.03%
 * 10678       20        534                1.000187301                     0.02%
 * 10679       20        534                1.000093642                     0.01%
 * 10680       20        534                1                            ** 0.00%
 * 10681       20        534                0.999906376                     0.01%
 * 10682       20        534                0.999812769                     0.02%
 * 10683       20        534                0.99971918                      0.03%
 * 10684       20        534                0.999625608                     0.04%
 * 10685       21        509                1.000374357                     0.04%
 * 10686       21        509                1.000280741                     0.03%
 * 10687       21        509                1.000187143                     0.02%
 * 10688       21        509                1.000093563                     0.01%
 * 10689       21        509                1                            ** 0.00%
 * 10690       21        509                0.999906455                     0.01%
 * 10691       22        486                1.000093537                     0.01%
 * 10692       22        486                1                            ** 0.00%
 * 10693       22        486                0.999906481                     0.01%
 * 10694       23        465                1.00009351                      0.01%
 * 10695       23        465                1                            ** 0.00%
 * 10696       23        465                0.999906507                     0.01%
 * 10697       23        465                0.999813032                     0.02%
 * 10698       20        535                1.000186951                     0.02%
 * 10699       20        535                1.000093467                     0.01%
 * 10700       20        535                1                            ** 0.00%
 * 10701       20        535                0.999906551                     0.01%
 * 10702       20        535                0.999813119                     0.02%
 * 10703       20        535                0.999719705                     0.03%
 * 10704       20        535                0.999626308                     0.04%
 * 10705       21        510                1.000467071                     0.05%
 * 10706       21        510                1.000373622                     0.04%
 * 10707       21        510                1.000280191                     0.03%
 * 10708       21        510                1.000186776                     0.02%
 * 10709       21        510                1.000093379                     0.01%
 * 10710       21        510                1                            ** 0.00%
 * 10711       21        510                0.999906638                     0.01%
 * 10712       22        487                1.000186706                     0.02%
 * 10713       22        487                1.000093345                     0.01%
 * 10714       22        487                1                            ** 0.00%
 * 10715       22        487                0.999906673                     0.01%
 * 10716       23        466                1.000186637                     0.02%
 * 10717       23        466                1.00009331                      0.01%
 * 10718       23        466                1                            ** 0.00%
 * 10719       20        536                1.000093292                     0.01%
 * 10720       20        536                1                            ** 0.00%
 * 10721       20        536                0.999906725                     0.01%
 * 10722       20        536                0.999813468                     0.02%
 * 10723       20        536                0.999720228                     0.03%
 * 10724       20        536                0.999627005                     0.04%
 * 10725       20        536                0.9995338                       0.05%
 * 10726       21        511                1.000466157                     0.05%
 * 10727       21        511                1.000372891                     0.04%
 * 10728       21        511                1.000279642                     0.03%
 * 10729       21        511                1.000186411                     0.02%
 * 10730       21        511                1.000093197                     0.01%
 * 10731       21        511                1                            ** 0.00%
 * 10732       21        511                0.999906821                     0.01%
 * 10733       21        511                0.999813659                     0.02%
 * 10734       22        488                1.000186324                     0.02%
 * 10735       22        488                1.000093153                     0.01%
 * 10736       22        488                1                            ** 0.00%
 * 10737       22        488                0.999906864                     0.01%
 * 10738       20        537                1.000186254                     0.02%
 * 10739       20        537                1.000093119                     0.01%
 * 10740       20        537                1                            ** 0.00%
 * 10741       23        467                1                            ** 0.00%
 * 10742       23        467                0.999906907                     0.01%
 * 10743       23        467                0.999813832                     0.02%
 * 10744       23        467                0.999720774                     0.03%
 * 10745       23        467                0.999627734                     0.04%
 * 10746       23        467                0.999534711                     0.05%
 * 10747       21        512                1.000465246                     0.05%
 * 10748       21        512                1.000372162                     0.04%
 * 10749       21        512                1.000279096                     0.03%
 * 10750       21        512                1.000186047                     0.02%
 * 10751       21        512                1.000093015                     0.01%
 * 10752       21        512                1                            ** 0.00%
 * 10753       21        512                0.999907003                     0.01%
 * 10754       21        512                0.999814023                     0.02%
 * 10755       22        489                1.00027894                      0.03%
 * 10756       22        489                1.000185943                     0.02%
 * 10757       22        489                1.000092963                     0.01%
 * 10758       22        489                1                            ** 0.00%
 * 10759       20        538                1.000092945                     0.01%
 * 10760       20        538                1                            ** 0.00%
 * 10761       20        538                0.999907072                     0.01%
 * 10762       23        468                1.000185839                     0.02%
 * 10763       23        468                1.000092911                     0.01%
 * 10764       23        468                1                            ** 0.00%
 * 10765       23        468                0.999907106                     0.01%
 * 10766       23        468                0.99981423                      0.02%
 * 10767       23        468                0.999721371                     0.03%
 * 10768       23        468                0.999628529                     0.04%
 * 10769       21        513                1.000371437                     0.04%
 * 10770       21        513                1.000278552                     0.03%
 * 10771       21        513                1.000185684                     0.02%
 * 10772       21        513                1.000092833                     0.01%
 * 10773       21        513                1                            ** 0.00%
 * 10774       21        513                0.999907184                     0.01%
 * 10775       21        513                0.999814385                     0.02%
 * 10776       21        513                0.999721604                     0.03%
 * 10777       20        539                1.000278371                     0.03%
 * 10778       20        539                1.000185563                     0.02%
 * 10779       20        539                1.000092773                     0.01%
 * 10780       20        539                1                            ** 0.00%
 * 10781       20        539                0.999907244                     0.01%
 * 10782       20        539                0.999814506                     0.02%
 * 10783       20        539                0.999721784                     0.03%
 * 10784       23        469                1.00027819                      0.03%
 * 10785       23        469                1.000185443                     0.02%
 * 10786       23        469                1.000092713                     0.01%
 * 10787       23        469                1                            ** 0.00%
 * 10788       23        469                0.999907304                     0.01%
 * 10789       23        469                0.999814626                     0.02%
 * 10790       23        469                0.999721965                     0.03%
 * 10791       21        514                1.000278009                     0.03%
 * 10792       21        514                1.000185322                     0.02%
 * 10793       21        514                1.000092653                     0.01%
 * 10794       21        514                1                            ** 0.00%
 * 10795       21        514                0.999907365                     0.01%
 * 10796       21        514                0.999814746                     0.02%
 * 10797       20        540                1.000277855                     0.03%
 * 10798       20        540                1.000185219                     0.02%
 * 10799       20        540                1.000092601                     0.01%
 * 10800       20        540                1                            ** 0.00%
 * 10801       22        491                1.000092584                     0.01%
 * 10802       22        491                1                            ** 0.00%
 * 10803       22        491                0.999907433                     0.01%
 * 10804       22        491                0.999814883                     0.02%
 * 10805       22        491                0.999722351                     0.03%
 * 10806       23        470                1.000370165                     0.04%
 * 10807       23        470                1.000277598                     0.03%
 * 10808       23        470                1.000185048                     0.02%
 * 10809       23        470                1.000092515                     0.01%
 * 10810       23        470                1                            ** 0.00%
 * 10811       23        470                0.999907502                     0.01%
 * 10812       23        470                0.99981502                      0.02%
 * 10813       21        515                1.000184963                     0.02%
 * 10814       21        515                1.000092473                     0.01%
 * 10815       21        515                1                            ** 0.00%
 * 10816       21        515                0.999907544                     0.01%
 * 10817       21        515                0.999815106                     0.02%
 * 10818       20        541                1.000184877                     0.02%
 * 10819       20        541                1.00009243                      0.01%
 * 10820       20        541                1                            ** 0.00%
 * 10821       20        541                0.999907587                     0.01%
 * 10822       22        492                1.000184809                     0.02%
 * 10823       22        492                1.000092396                     0.01%
 * 10824       22        492                1                            ** 0.00%
 * 10825       22        492                0.999907621                     0.01%
 * 10826       22        492                0.99981526                      0.02%
 * 10827       22        492                0.999722915                     0.03%
 * 10828       22        492                0.999630587                     0.04%
 * 10829       23        471                1.000369379                     0.04%
 * 10830       23        471                1.000277008                     0.03%
 * 10831       23        471                1.000184655                     0.02%
 * 10832       23        471                1.000092319                     0.01%
 * 10833       23        471                1                            ** 0.00%
 * 10834       23        471                0.999907698                     0.01%
 * 10835       21        516                1.000092293                     0.01%
 * 10836       21        516                1                            ** 0.00%
 * 10837       21        516                0.999907724                     0.01%
 * 10838       20        542                1.000184536                     0.02%
 * 10839       20        542                1.000092259                     0.01%
 * 10840       20        542                1                            ** 0.00%
 * 10841       20        542                0.999907758                     0.01%
 * 10842       20        542                0.999815532                     0.02%
 * 10843       22        493                1.000276676                     0.03%
 * 10844       22        493                1.000184434                     0.02%
 * 10845       22        493                1.000092208                     0.01%
 * 10846       22        493                1                            ** 0.00%
 * 10847       22        493                0.999907809                     0.01%
 * 10848       22        493                0.999815634                     0.02%
 * 10849       22        493                0.999723477                     0.03%
 * 10850       22        493                0.999631336                     0.04%
 * 10851       23        472                1.000460787                     0.05%
 * 10852       23        472                1.000368596                     0.04%
 * 10853       23        472                1.000276421                     0.03%
 * 10854       23        472                1.000184264                     0.02%
 * 10855       23        472                1.000092123                     0.01%
 * 10856       23        472                1                            ** 0.00%
 * 10857       21        517                1                            ** 0.00%
 * 10858       21        517                0.999907902                     0.01%
 * 10859       20        543                1.00009209                      0.01%
 * 10860       20        543                1                            ** 0.00%
 * 10861       20        543                0.999907927                     0.01%
 * 10862       20        543                0.999815872                     0.02%
 * 10863       20        543                0.999723833                     0.03%
 * 10864       22        494                1.000368189                     0.04%
 * 10865       22        494                1.000276116                     0.03%
 * 10866       22        494                1.00018406                      0.02%
 * 10867       22        494                1.000092022                     0.01%
 * 10868       22        494                1                            ** 0.00%
 * 10869       22        494                0.999907995                     0.01%
 * 10870       22        494                0.999816007                     0.02%
 * 10871       22        494                0.999724036                     0.03%
 * 10872       22        494                0.999632082                     0.04%
 * 10873       21        518                1.000459855                     0.05%
 * 10874       21        518                1.00036785                      0.04%
 * 10875       21        518                1.000275862                     0.03%
 * 10876       21        518                1.000183891                     0.02%
 * 10877       21        518                1.000091937                     0.01%
 * 10878       21        518                1                            ** 0.00%
 * 10879       23        473                1                            ** 0.00%
 * 10880       20        544                1                            ** 0.00%
 * 10881       20        544                0.999908097                     0.01%
 * 10882       20        544                0.99981621                      0.02%
 * 10883       20        544                0.999724341                     0.03%
 * 10884       20        544                0.999632488                     0.04%
 * 10885       22        495                1.000459348                     0.05%
 * 10886       22        495                1.000367444                     0.04%
 * 10887       22        495                1.000275558                     0.03%
 * 10888       22        495                1.000183688                     0.02%
 * 10889       22        495                1.000091836                     0.01%
 * 10890       22        495                1                            ** 0.00%
 * 10891       22        495                0.999908181                     0.01%
 * 10892       22        495                0.999816379                     0.02%
 * 10893       22        495                0.999724594                     0.03%
 * 10894       22        495                0.999632825                     0.04%
 * 10895       21        519                1.000367141                     0.04%
 * 10896       21        519                1.00027533                      0.03%
 * 10897       21        519                1.000183537                     0.02%
 * 10898       21        519                1.00009176                      0.01%
 * 10899       21        519                1                            ** 0.00%
 * 10900       20        545                1                            ** 0.00%
 * 10901       23        474                1.000091735                     0.01%
 * 10902       23        474                1                            ** 0.00%
 * 10903       23        474                0.999908282                     0.01%
 * 10904       23        474                0.999816581                     0.02%
 * 10905       23        474                0.999724897                     0.03%
 * 10906       23        474                0.999633229                     0.04%
 * 10907       22        496                1.000458421                     0.05%
 * 10908       22        496                1.000366703                     0.04%
 * 10909       22        496                1.000275002                     0.03%
 * 10910       22        496                1.000183318                     0.02%
 * 10911       22        496                1.000091651                     0.01%
 * 10912       22        496                1                            ** 0.00%
 * 10913       22        496                0.999908366                     0.01%
 * 10914       22        496                0.999816749                     0.02%
 * 10915       22        496                0.999725149                     0.03%
 * 10916       20        546                1.000366435                     0.04%
 * 10917       20        546                1.000274801                     0.03%
 * 10918       20        546                1.000183184                     0.02%
 * 10919       20        546                1.000091583                     0.01%
 * 10920       20        546                1                            ** 0.00%
 * 10921       20        546                0.999908433                     0.01%
 * 10922       20        546                0.999816883                     0.02%
 * 10923       23        475                1.0001831                       0.02%
 * 10924       23        475                1.000091542                     0.01%
 * 10925       23        475                1                            ** 0.00%
 * 10926       23        475                0.999908475                     0.01%
 * 10927       23        475                0.999816967                     0.02%
 * 10928       23        475                0.999725476                     0.03%
 * 10929       23        475                0.999634001                     0.04%
 * 10930       22        497                1.000365965                     0.04%
 * 10931       22        497                1.000274449                     0.03%
 * 10932       22        497                1.000182949                     0.02%
 * 10933       22        497                1.000091466                     0.01%
 * 10934       22        497                1                            ** 0.00%
 * 10935       22        497                0.999908551                     0.01%
 * 10936       22        497                0.999817118                     0.02%
 * 10937       20        547                1.000274298                     0.03%
 * 10938       20        547                1.000182849                     0.02%
 * 10939       20        547                1.000091416                     0.01%
 * 10940       20        547                1                            ** 0.00%
 * 10941       21        521                1                            ** 0.00%
 * 10942       21        521                0.999908609                     0.01%
 * 10943       21        521                0.999817235                     0.02%
 * 10944       21        521                0.999725877                     0.03%
 * 10945       23        476                1.000274098                     0.03%
 * 10946       23        476                1.000182715                     0.02%
 * 10947       23        476                1.000091349                     0.01%
 * 10948       23        476                1                            ** 0.00%
 * 10949       23        476                0.999908667                     0.01%
 * 10950       23        476                0.999817352                     0.02%
 * 10951       23        476                0.999726052                     0.03%
 * 10952       22        498                1.00036523                      0.04%
 * 10953       22        498                1.000273898                     0.03%
 * 10954       22        498                1.000182582                     0.02%
 * 10955       22        498                1.000091283                     0.01%
 * 10956       22        498                1                            ** 0.00%
 * 10957       22        498                0.999908734                     0.01%
 * 10958       20        548                1.000182515                     0.02%
 * 10959       20        548                1.000091249                     0.01%
 * 10960       20        548                1                            ** 0.00%
 * 10961       21        522                1.000091233                     0.01%
 * 10962       21        522                1                            ** 0.00%
 * 10963       21        522                0.999908784                     0.01%
 * 10964       21        522                0.999817585                     0.02%
 * 10965       21        522                0.999726402                     0.03%
 * 10966       21        522                0.999635236                     0.04%
 * 10967       23        477                1.000364731                     0.04%
 * 10968       23        477                1.000273523                     0.03%
 * 10969       23        477                1.000182332                     0.02%
 * 10970       23        477                1.000091158                     0.01%
 * 10971       23        477                1                            ** 0.00%
 * 10972       23        477                0.999908859                     0.01%
 * 10973       23        477                0.999817734                     0.02%
 * 10974       23        477                0.999726627                     0.03%
 * 10975       22        499                1.000273349                     0.03%
 * 10976       22        499                1.000182216                     0.02%
 * 10977       22        499                1.0000911                       0.01%
 * 10978       22        499                1                            ** 0.00%
 * 10979       20        549                1.000091083                     0.01%
 * 10980       20        549                1                            ** 0.00%
 * 10981       20        549                0.999908934                     0.01%
 * 10982       21        523                1.000091058                     0.01%
 * 10983       21        523                1                            ** 0.00%
 * 10984       21        523                0.999908958                     0.01%
 * 10985       21        523                0.999817934                     0.02%
 * 10986       21        523                0.999726925                     0.03%
 * 10987       21        523                0.999635933                     0.04%
 * 10988       21        523                0.999544958                     0.05%
 * 10989       23        478                1.000455                        0.05%
 * 10990       23        478                1.000363967                     0.04%
 * 10991       23        478                1.000272951                     0.03%
 * 10992       23        478                1.000181951                     0.02%
 * 10993       23        478                1.000090967                     0.01%
 * 10994       23        478                1                            ** 0.00%
 * 10995       23        478                0.99990905                      0.01%
 * 10996       23        478                0.999818116                     0.02%
 * 10997       20        550                1.000272802                     0.03%
 * 10998       20        550                1.000181851                     0.02%
 * 10999       20        550                1.000090917                     0.01%
 * 11000       20        550                1                            ** 0.00%
 * 11001       20        550                0.999909099                     0.01%
 * 11002       21        524                1.000181785                     0.02%
 * 11003       21        524                1.000090884                     0.01%
 * 11004       21        524                1                            ** 0.00%
 * 11005       21        524                0.999909132                     0.01%
 * 11006       21        524                0.999818281                     0.02%
 * 11007       21        524                0.999727446                     0.03%
 * 11008       21        524                0.999636628                     0.04%
 * 11009       21        524                0.999545826                     0.05%
 * 11010       21        524                0.999455041                     0.05%
 * 11011       23        479                1.00054491                      0.05%
 * 11012       23        479                1.00045405                      0.05%
 * 11013       23        479                1.000363207                     0.04%
 * 11014       23        479                1.000272381                     0.03%
 * 11015       23        479                1.000181571                     0.02%
 * 11016       23        479                1.000090777                     0.01%
 * 11017       23        479                1                            ** 0.00%
 * 11018       23        479                0.999909239                     0.01%
 * 11019       23        479                0.999818495                     0.02%
 * 11020       22        501                1.000181488                     0.02%
 * 11021       22        501                1.000090736                     0.01%
 * 11022       22        501                1                            ** 0.00%
 * 11023       22        501                0.999909281                     0.01%
 * 11024       21        525                1.000090711                     0.01%
 * 11025       21        525                1                            ** 0.00%
 * 11026       21        525                0.999909305                     0.01%
 * 11027       21        525                0.999818627                     0.02%
 * 11028       21        525                0.999727965                     0.03%
 * 11029       21        525                0.99963732                      0.04%
 * 11030       21        525                0.999546691                     0.05%
 * 11031       21        525                0.999456078                     0.05%
 * 11032       21        525                0.999365482                     0.06%
 * 11033       23        480                1.00063446                      0.06%
 * 11034       23        480                1.000543774                     0.05%
 * 11035       23        480                1.000453104                     0.05%
 * 11036       23        480                1.00036245                      0.04%
 * 11037       23        480                1.000271813                     0.03%
 * 11038       23        480                1.000181192                     0.02%
 * 11039       23        480                1.000090588                     0.01%
 * 11040       23        480                1                            ** 0.00%
 * 11041       23        480                0.999909428                     0.01%
 * 11042       22        502                1.000181127                     0.02%
 * 11043       22        502                1.000090555                     0.01%
 * 11044       22        502                1                            ** 0.00%
 * 11045       21        526                1.000090539                     0.01%
 * 11046       21        526                1                            ** 0.00%
 * 11047       21        526                0.999909478                     0.01%
 * 11048       21        526                0.999818972                     0.02%
 * 11049       21        526                0.999728482                     0.03%
 * 11050       21        526                0.999638009                     0.04%
 * 11051       21        526                0.999547552                     0.05%
 * 11052       21        526                0.999457112                     0.05%
 * 11053       21        526                0.999366688                     0.06%
 * 11054       21        526                0.99927628                      0.07%
 * 11055       23        481                1.000723654                     0.07%
 * 11056       23        481                1.00063314                      0.06%
 * 11057       23        481                1.000542643                     0.05%
 * 11058       23        481                1.000452161                     0.05%
 * 11059       23        481                1.000361696                     0.04%
 * 11060       23        481                1.000271248                     0.03%
 * 11061       23        481                1.000180815                     0.02%
 * 11062       23        481                1.0000904                       0.01%
 * 11063       23        481                1                            ** 0.00%
 * 11064       23        481                0.999909617                     0.01%
 * 11065       22        503                1.000090375                     0.01%
 * 11066       22        503                1                            ** 0.00%
 * 11067       21        527                1                            ** 0.00%
 * 11068       21        527                0.999909649                     0.01%
 * 11069       21        527                0.999819315                     0.02%
 * 11070       21        527                0.999728997                     0.03%
 * 11071       21        527                0.999638696                     0.04%
 * 11072       21        527                0.99954841                      0.05%
 * 11073       21        527                0.999458141                     0.05%
 * 11074       21        527                0.999367889                     0.06%
 * 11075       21        527                0.999277652                     0.07%
 * 11076       21        527                0.999187432                     0.08%
 * 11077       23        482                1.000812494                     0.08%
 * 11078       23        482                1.000722152                     0.07%
 * 11079       23        482                1.000631826                     0.06%
 * 11080       23        482                1.000541516                     0.05%
 * 11081       23        482                1.000451223                     0.05%
 * 11082       23        482                1.000360946                     0.04%
 * 11083       23        482                1.000270685                     0.03%
 * 11084       23        482                1.00018044                      0.02%
 * 11085       23        482                1.000090212                     0.01%
 * 11086       23        482                1                            ** 0.00%
 * 11087       21        528                1.000090196                     0.01%
 * 11088       21        528                1                            ** 0.00%
 * 11089       21        528                0.999909821                     0.01%
 * 11090       21        528                0.999819657                     0.02%
 * 11091       21        528                0.99972951                      0.03%
 * 11092       21        528                0.99963938                      0.04%
 * 11093       21        528                0.999549265                     0.05%
 * 11094       21        528                0.999459167                     0.05%
 * 11095       21        528                0.999369085                     0.06%
 * 11096       21        528                0.999279019                     0.07%
 * 11097       21        528                0.99918897                      0.08%
 * 11098       21        528                0.999098937                     0.09%
 * 11099       21        529                1.000900982                     0.09%
 * 11100       21        529                1.000810811                     0.08%
 * 11101       21        529                1.000720656                     0.07%
 * 11102       21        529                1.000630517                     0.06%
 * 11103       21        529                1.000540394                     0.05%
 * 11104       21        529                1.000450288                     0.05%
 * 11105       21        529                1.000360198                     0.04%
 * 11106       21        529                1.000270124                     0.03%
 * 11107       21        529                1.000180067                     0.02%
 * 11108       21        529                1.000090025                     0.01%
 * 11109       21        529                1                            ** 0.00%
 * 11110       22        505                1                            ** 0.00%
 * 11111       22        505                0.999909999                     0.01%
 * 11112       22        505                0.999820014                     0.02%
 * 11113       22        505                0.999730046                     0.03%
 * 11114       22        505                0.999640094                     0.04%
 * 11115       22        505                0.999550157                     0.04%
 * 11116       22        505                0.999460237                     0.05%
 * 11117       22        505                0.999370334                     0.06%
 * 11118       22        505                0.999280446                     0.07%
 * 11119       22        505                0.999190575                     0.08%
 * 11120       21        530                1.000899281                     0.09%
 * 11121       21        530                1.00080928                      0.08%
 * 11122       21        530                1.000719295                     0.07%
 * 11123       21        530                1.000629327                     0.06%
 * 11124       21        530                1.000539374                     0.05%
 * 11125       21        530                1.000449438                     0.04%
 * 11126       21        530                1.000359518                     0.04%
 * 11127       21        530                1.000269614                     0.03%
 * 11128       21        530                1.000179727                     0.02%
 * 11129       21        530                1.000089855                     0.01%
 * 11130       21        530                1                            ** 0.00%
 * 11131       22        506                1.000089839                     0.01%
 * 11132       22        506                1                            ** 0.00%
 * 11133       22        506                0.999910177                     0.01%
 * 11134       22        506                0.99982037                      0.02%
 * 11135       22        506                0.999730579                     0.03%
 * 11136       22        506                0.999640805                     0.04%
 * 11137       22        506                0.999551046                     0.04%
 * 11138       22        506                0.999461304                     0.05%
 * 11139       22        506                0.999371577                     0.06%
 * 11140       22        506                0.999281867                     0.07%
 * 11141       22        506                0.999192173                     0.08%
 * 11142       21        531                1.000807754                     0.08%
 * 11143       21        531                1.00071794                      0.07%
 * 11144       21        531                1.000628141                     0.06%
 * 11145       21        531                1.000538358                     0.05%
 * 11146       21        531                1.000448591                     0.04%
 * 11147       21        531                1.000358841                     0.04%
 * 11148       21        531                1.000269107                     0.03%
 * 11149       21        531                1.000179388                     0.02%
 * 11150       21        531                1.000089686                     0.01%
 * 11151       21        531                1                            ** 0.00%
 * 11152       21        531                0.99991033                      0.01%
 * 11153       22        507                1.000089662                     0.01%
 * 11154       22        507                1                            ** 0.00%
 * 11155       23        485                1                            ** 0.00%
 * 11156       23        485                0.999910362                     0.01%
 * 11157       23        485                0.99982074                      0.02%
 * 11158       23        485                0.999731135                     0.03%
 * 11159       23        485                0.999641545                     0.04%
 * 11160       23        485                0.999551971                     0.04%
 * 11161       23        485                0.999462414                     0.05%
 * 11162       23        485                0.999372872                     0.06%
 * 11163       23        485                0.999283347                     0.07%
 * 11164       21        532                1.000716589                     0.07%
 * 11165       21        532                1.000626959                     0.06%
 * 11166       21        532                1.000537346                     0.05%
 * 11167       21        532                1.000447748                     0.04%
 * 11168       21        532                1.000358166                     0.04%
 * 11169       21        532                1.000268601                     0.03%
 * 11170       21        532                1.000179051                     0.02%
 * 11171       21        532                1.000089518                     0.01%
 * 11172       21        532                1                            ** 0.00%
 * 11173       21        532                0.999910499                     0.01%
 * 11174       22        508                1.000178987                     0.02%
 * 11175       22        508                1.000089485                     0.01%
 * 11176       22        508                1                            ** 0.00%
 * 11177       23        486                1.000089469                     0.01%
 * 11178       23        486                1                            ** 0.00%
 * 11179       23        486                0.999910547                     0.01%
 * 11180       23        486                0.999821109                     0.02%
 * 11181       23        486                0.999731688                     0.03%
 * 11182       23        486                0.999642282                     0.04%
 * 11183       23        486                0.999552893                     0.04%
 * 11184       23        486                0.999463519                     0.05%
 * 11185       23        486                0.999374162                     0.06%
 * 11186       21        533                1.000625782                     0.06%
 * 11187       21        533                1.000536337                     0.05%
 * 11188       21        533                1.000446907                     0.04%
 * 11189       21        533                1.000357494                     0.04%
 * 11190       21        533                1.000268097                     0.03%
 * 11191       21        533                1.000178715                     0.02%
 * 11192       21        533                1.00008935                      0.01%
 * 11193       21        533                1                            ** 0.00%
 * 11194       21        533                0.999910666                     0.01%
 * 11195       21        533                0.999821349                     0.02%
 * 11196       22        509                1.000178635                     0.02%
 * 11197       22        509                1.00008931                      0.01%
 * 11198       22        509                1                            ** 0.00%
 * 11199       22        509                0.999910706                     0.01%
 * 11200       23        487                1.000089286                     0.01%
 * 11201       23        487                1                            ** 0.00%
 * 11202       23        487                0.99991073                      0.01%
 * 11203       23        487                0.999821476                     0.02%
 * 11204       23        487                0.999732238                     0.03%
 * 11205       23        487                0.999643017                     0.04%
 * 11206       23        487                0.99955381                      0.04%
 * 11207       23        487                0.99946462                      0.05%
 * 11208       21        534                1.000535332                     0.05%
 * 11209       21        534                1.00044607                      0.04%
 * 11210       21        534                1.000356824                     0.04%
 * 11211       21        534                1.000267594                     0.03%
 * 11212       21        534                1.00017838                      0.02%
 * 11213       21        534                1.000089182                     0.01%
 * 11214       21        534                1                            ** 0.00%
 * 11215       21        534                0.999910834                     0.01%
 * 11216       21        534                0.999821683                     0.02%
 * 11217       22        510                1.000267451                     0.03%
 * 11218       22        510                1.000178285                     0.02%
 * 11219       22        510                1.000089135                     0.01%
 * 11220       22        510                1                            ** 0.00%
 * 11221       22        510                0.999910881                     0.01%
 * 11222       23        488                1.000178221                     0.02%
 * 11223       23        488                1.000089103                     0.01%
 * 11224       23        488                1                            ** 0.00%
 * 11225       23        488                0.999910913                     0.01%
 * 11226       23        488                0.999821842                     0.02%
 * 11227       23        488                0.999732787                     0.03%
 * 11228       23        488                0.999643748                     0.04%
 * 11229       23        488                0.999554724                     0.04%
 * 11230       21        535                1.000445236                     0.04%
 * 11231       21        535                1.000356157                     0.04%
 * 11232       21        535                1.000267094                     0.03%
 * 11233       21        535                1.000178047                     0.02%
 * 11234       21        535                1.000089015                     0.01%
 * 11235       21        535                1                            ** 0.00%
 * 11236       21        535                0.999911                        0.01%
 * 11237       21        535                0.999822017                     0.02%
 * 11238       21        535                0.999733049                     0.03%
 * 11239       22        511                1.000266928                     0.03%
 * 11240       22        511                1.000177936                     0.02%
 * 11241       22        511                1.00008896                      0.01%
 * 11242       22        511                1                            ** 0.00%
 * 11243       22        511                0.999911056                     0.01%
 * 11244       22        511                0.999822127                     0.02%
 * 11245       23        489                1.000177857                     0.02%
 * 11246       23        489                1.000088921                     0.01%
 * 11247       23        489                1                            ** 0.00%
 * 11248       23        489                0.999911095                     0.01%
 * 11249       23        489                0.999822206                     0.02%
 * 11250       23        489                0.999733333                     0.03%
 * 11251       23        489                0.999644476                     0.04%
 * 11252       21        536                1.000355492                     0.04%
 * 11253       21        536                1.000266596                     0.03%
 * 11254       21        536                1.000177715                     0.02%
 * 11255       21        536                1.000088849                     0.01%
 * 11256       21        536                1                            ** 0.00%
 * 11257       21        536                0.999911166                     0.01%
 * 11258       21        536                0.999822349                     0.02%
 * 11259       21        536                0.999733546                     0.03%
 * 11260       22        512                1.00035524                      0.04%
 * 11261       22        512                1.000266406                     0.03%
 * 11262       22        512                1.000177588                     0.02%
 * 11263       22        512                1.000088786                     0.01%
 * 11264       22        512                1                            ** 0.00%
 * 11265       22        512                0.999911229                     0.01%
 * 11266       22        512                0.999822475                     0.02%
 * 11267       23        490                1.000266264                     0.03%
 * 11268       23        490                1.000177494                     0.02%
 * 11269       23        490                1.000088739                     0.01%
 * 11270       23        490                1                            ** 0.00%
 * 11271       23        490                0.999911277                     0.01%
 * 11272       23        490                0.999822569                     0.02%
 * 11273       23        490                0.999733877                     0.03%
 * 11274       21        537                1.000266099                     0.03%
 * 11275       21        537                1.000177384                     0.02%
 * 11276       21        537                1.000088684                     0.01%
 * 11277       21        537                1                            ** 0.00%
 * 11278       21        537                0.999911332                     0.01%
 * 11279       21        537                0.999822679                     0.02%
 * 11280       21        537                0.999734043                     0.03%
 * 11281       21        537                0.999645422                     0.04%
 * 11282       22        513                1.000354547                     0.04%
 * 11283       22        513                1.000265887                     0.03%
 * 11284       22        513                1.000177242                     0.02%
 * 11285       22        513                1.000088613                     0.01%
 * 11286       22        513                1                            ** 0.00%
 * 11287       22        513                0.999911402                     0.01%
 * 11288       22        513                0.999822821                     0.02%
 * 11289       22        513                0.999734255                     0.03%
 * 11290       23        491                1.000265722                     0.03%
 * 11291       23        491                1.000177132                     0.02%
 * 11292       23        491                1.000088558                     0.01%
 * 11293       23        491                1                            ** 0.00%
 * 11294       23        491                0.999911457                     0.01%
 * 11295       23        491                0.999822931                     0.02%
 * 11296       21        538                1.000177054                     0.02%
 * 11297       21        538                1.000088519                     0.01%
 * 11298       21        538                1                            ** 0.00%
 * 11299       21        538                0.999911497                     0.01%
 * 11300       21        538                0.999823009                     0.02%
 * 11301       21        538                0.999734537                     0.03%
 * 11302       21        538                0.99964608                      0.04%
 * 11303       22        514                1.00044236                      0.04%
 * 11304       22        514                1.000353857                     0.04%
 * 11305       22        514                1.000265369                     0.03%
 * 11306       22        514                1.000176897                     0.02%
 * 11307       22        514                1.000088441                     0.01%
 * 11308       22        514                1                            ** 0.00%
 * 11309       22        514                0.999911575                     0.01%
 * 11310       22        514                0.999823165                     0.02%
 * 11311       22        514                0.999734771                     0.03%
 * 11312       23        492                1.000353607                     0.04%
 * 11313       23        492                1.000265182                     0.03%
 * 11314       23        492                1.000176772                     0.02%
 * 11315       23        492                1.000088378                     0.01%
 * 11316       23        492                1                            ** 0.00%
 * 11317       23        492                0.999911637                     0.01%
 * 11318       21        539                1.000088355                     0.01%
 * 11319       21        539                1                            ** 0.00%
 * 11320       21        539                0.999911661                     0.01%
 * 11321       21        539                0.999823337                     0.02%
 * 11322       21        539                0.999735029                     0.03%
 * 11323       21        539                0.999646737                     0.04%
 * 11324       21        539                0.99955846                      0.04%
 * 11325       22        515                1.000441501                     0.04%
 * 11326       22        515                1.00035317                      0.04%
 * 11327       22        515                1.000264854                     0.03%
 * 11328       22        515                1.000176554                     0.02%
 * 11329       22        515                1.000088269                     0.01%
 * 11330       22        515                1                            ** 0.00%
 * 11331       22        515                0.999911747                     0.01%
 * 11332       22        515                0.999823509                     0.02%
 * 11333       22        515                0.999735286                     0.03%
 * 11334       22        515                0.99964708                      0.04%
 * 11335       23        493                1.000352889                     0.04%
 * 11336       23        493                1.000264644                     0.03%
 * 11337       23        493                1.000176414                     0.02%
 * 11338       23        493                1.000088199                     0.01%
 * 11339       23        493                1                            ** 0.00%
 * 11340       21        540                1                            ** 0.00%
 * 11341       21        540                0.999911824                     0.01%
 * 11342       21        540                0.999823664                     0.02%
 * 11343       21        540                0.99973552                      0.03%
 * 11344       21        540                0.999647391                     0.04%
 * 11345       21        540                0.999559277                     0.04%
 * 11346       22        516                1.000528821                     0.05%
 * 11347       22        516                1.000440645                     0.04%
 * 11348       22        516                1.000352485                     0.04%
 * 11349       22        516                1.00026434                      0.03%
 * 11350       22        516                1.000176211                     0.02%
 * 11351       22        516                1.000088098                     0.01%
 * 11352       22        516                1                            ** 0.00%
 * 11353       22        516                0.999911918                     0.01%
 * 11354       22        516                0.999823851                     0.02%
 * 11355       22        516                0.999735799                     0.03%
 * 11356       22        516                0.999647763                     0.04%
 * 11357       21        541                1.000352206                     0.04%
 * 11358       21        541                1.000264131                     0.03%
 * 11359       21        541                1.000176072                     0.02%
 * 11360       21        541                1.000088028                     0.01%
 * 11361       21        541                1                            ** 0.00%
 * 11362       23        494                1                            ** 0.00%
 * 11363       23        494                0.999911995                     0.01%
 * 11364       23        494                0.999824006                     0.02%
 * 11365       23        494                0.999736032                     0.03%
 * 11366       23        494                0.999648073                     0.04%
 * 11367       23        494                0.99956013                      0.04%
 * 11368       22        517                1.000527797                     0.05%
 * 11369       22        517                1.000439792                     0.04%
 * 11370       22        517                1.000351803                     0.04%
 * 11371       22        517                1.000263829                     0.03%
 * 11372       22        517                1.000175871                     0.02%
 * 11373       22        517                1.000087928                     0.01%
 * 11374       22        517                1                            ** 0.00%
 * 11375       22        517                0.999912088                     0.01%
 * 11376       22        517                0.999824191                     0.02%
 * 11377       22        517                0.99973631                      0.03%
 * 11378       21        542                1.000351556                     0.04%
 * 11379       21        542                1.000263644                     0.03%
 * 11380       21        542                1.000175747                     0.02%
 * 11381       21        542                1.000087866                     0.01%
 * 11382       21        542                1                            ** 0.00%
 * 11383       21        542                0.99991215                      0.01%
 * 11384       23        495                1.000087843                     0.01%
 * 11385       23        495                1                            ** 0.00%
 * 11386       23        495                0.999912173                     0.01%
 * 11387       23        495                0.999824361                     0.02%
 * 11388       23        495                0.999736565                     0.03%
 * 11389       23        495                0.999648784                     0.04%
 * 11390       23        495                0.999561018                     0.04%
 * 11391       22        518                1.000438943                     0.04%
 * 11392       22        518                1.000351124                     0.04%
 * 11393       22        518                1.00026332                      0.03%
 * 11394       22        518                1.000175531                     0.02%
 * 11395       22        518                1.000087758                     0.01%
 * 11396       22        518                1                            ** 0.00%
 * 11397       22        518                0.999912258                     0.01%
 * 11398       22        518                0.999824531                     0.02%
 * 11399       22        518                0.999736819                     0.03%
 * 11400       24        475                1                            ** 0.00%
 * 11401       24        475                0.999912288                     0.01%
 * 11402       21        543                1.000087704                     0.01%
 * 11403       21        543                1                            ** 0.00%
 * 11404       21        543                0.999912311                     0.01%
 * 11405       21        543                0.999824638                     0.02%
 * 11406       23        496                1.000175346                     0.02%
 * 11407       23        496                1.000087665                     0.01%
 * 11408       23        496                1                            ** 0.00%
 * 11409       23        496                0.99991235                      0.01%
 * 11410       23        496                0.999824715                     0.02%
 * 11411       23        496                0.999737096                     0.03%
 * 11412       23        496                0.999649492                     0.04%
 * 11413       22        519                1.000438097                     0.04%
 * 11414       22        519                1.000350447                     0.04%
 * 11415       22        519                1.000262812                     0.03%
 * 11416       22        519                1.000175193                     0.02%
 * 11417       22        519                1.000087589                     0.01%
 * 11418       22        519                1                            ** 0.00%
 * 11419       22        519                0.999912427                     0.01%
 * 11420       22        519                0.999824869                     0.02%
 * 11421       21        544                1.000262674                     0.03%
 * 11422       21        544                1.000175101                     0.02%
 * 11423       21        544                1.000087543                     0.01%
 * 11424       21        544                1                            ** 0.00%
 * 11425       25        457                1                            ** 0.00%
 * 11426       25        457                0.99991248                      0.01%
 * 11427       25        457                0.999824976                     0.02%
 * 11428       23        497                1.000262513                     0.03%
 * 11429       23        497                1.000174993                     0.02%
 * 11430       23        497                1.000087489                     0.01%
 * 11431       23        497                1                            ** 0.00%
 * 11432       23        497                0.999912526                     0.01%
 * 11433       23        497                0.999825068                     0.02%
 * 11434       23        497                0.999737625                     0.03%
 * 11435       23        497                0.999650197                     0.03%
 * 11436       22        520                1.000349773                     0.03%
 * 11437       22        520                1.000262307                     0.03%
 * 11438       22        520                1.000174856                     0.02%
 * 11439       22        520                1.00008742                      0.01%
 * 11440       22        520                1                            ** 0.00%
 * 11441       22        520                0.999912595                     0.01%
 * 11442       22        520                0.999825205                     0.02%
 * 11443       21        545                1.000174779                     0.02%
 * 11444       21        545                1.000087382                     0.01%
 * 11445       21        545                1                            ** 0.00%
 * 11446       21        545                0.999912633                     0.01%
 * 11447       24        477                1.000087359                     0.01%
 * 11448       24        477                1                            ** 0.00%
 * 11449       24        477                0.999912656                     0.01%
 * 11450       25        458                1                            ** 0.00%
 * 11451       25        458                0.999912671                     0.01%
 * 11452       25        458                0.999825358                     0.02%
 * 11453       23        498                1.000087313                     0.01%
 * 11454       23        498                1                            ** 0.00%
 * 11455       23        498                0.999912702                     0.01%
 * 11456       23        498                0.999825419                     0.02%
 * 11457       23        498                0.999738151                     0.03%
 * 11458       22        521                1.000349101                     0.03%
 * 11459       22        521                1.000261803                     0.03%
 * 11460       22        521                1.00017452                      0.02%
 * 11461       22        521                1.000087252                     0.01%
 * 11462       22        521                1                            ** 0.00%
 * 11463       22        521                0.999912763                     0.01%
 * 11464       21        546                1.000174459                     0.02%
 * 11465       21        546                1.000087222                     0.01%
 * 11466       21        546                1                            ** 0.00%
 * 11467       21        546                0.999912793                     0.01%
 * 11468       21        546                0.999825602                     0.02%
 * 11469       21        546                0.999738425                     0.03%
 * 11470       24        478                1.000174368                     0.02%
 * 11471       24        478                1.000087176                     0.01%
 * 11472       24        478                1                            ** 0.00%
 * 11473       24        478                0.999912839                     0.01%
 * 11474       25        459                1.000087154                     0.01%
 * 11475       25        459                1                            ** 0.00%
 * 11476       23        499                1.000087138                     0.01%
 * 11477       23        499                1                            ** 0.00%
 * 11478       23        499                0.999912877                     0.01%
 * 11479       23        499                0.999825769                     0.02%
 * 11480       23        499                0.999738676                     0.03%
 * 11481       22        522                1.000261301                     0.03%
 * 11482       22        522                1.000174186                     0.02%
 * 11483       22        522                1.000087085                     0.01%
 * 11484       22        522                1                            ** 0.00%
 * 11485       22        522                0.99991293                      0.01%
 * 11486       21        547                1.000087063                     0.01%
 * 11487       21        547                1                            ** 0.00%
 * 11488       21        547                0.999912953                     0.01%
 * 11489       21        547                0.99982592                      0.02%
 * 11490       21        547                0.999738903                     0.03%
 * 11491       21        547                0.999651901                     0.03%
 * 11492       24        479                1.000348068                     0.03%
 * 11493       24        479                1.000261028                     0.03%
 * 11494       24        479                1.000174004                     0.02%
 * 11495       24        479                1.000086994                     0.01%
 * 11496       24        479                1                            ** 0.00%
 * 11497       24        479                0.999913021                     0.01%
 * 11498       24        479                0.999826057                     0.02%
 * 11499       23        500                1.000086964                     0.01%
 * 11500       23        500                1                            ** 0.00%
 * 11501       23        500                0.999913051                     0.01%
 * 11502       23        500                0.999826117                     0.02%
 * 11503       22        523                1.000260802                     0.03%
 * 11504       22        523                1.000173853                     0.02%
 * 11505       22        523                1.000086919                     0.01%
 * 11506       22        523                1                            ** 0.00%
 * 11507       21        548                1.000086904                     0.01%
 * 11508       21        548                1                            ** 0.00%
 * 11509       21        548                0.999913111                     0.01%
 * 11510       21        548                0.999826238                     0.02%
 * 11511       21        548                0.99973938                      0.03%
 * 11512       21        548                0.999652536                     0.03%
 * 11513       21        548                0.999565708                     0.04%
 * 11514       21        548                0.999478895                     0.05%
 * 11515       24        480                1.000434216                     0.04%
 * 11516       24        480                1.000347343                     0.03%
 * 11517       24        480                1.000260485                     0.03%
 * 11518       24        480                1.000173641                     0.02%
 * 11519       24        480                1.000086813                     0.01%
 * 11520       24        480                1                            ** 0.00%
 * 11521       24        480                0.999913202                     0.01%
 * 11522       23        501                1.00008679                      0.01%
 * 11523       23        501                1                            ** 0.00%
 * 11524       23        501                0.999913225                     0.01%
 * 11525       25        461                1                            ** 0.00%
 * 11526       25        461                0.99991324                      0.01%
 * 11527       22        524                1.000086753                     0.01%
 * 11528       22        524                1                            ** 0.00%
 * 11529       21        549                1                            ** 0.00%
 * 11530       21        549                0.99991327                      0.01%
 * 11531       21        549                0.999826555                     0.02%
 * 11532       21        549                0.999739854                     0.03%
 * 11533       21        549                0.999653169                     0.03%
 * 11534       21        549                0.999566499                     0.04%
 * 11535       21        549                0.999479844                     0.05%
 * 11536       21        549                0.999393204                     0.06%
 * 11537       24        481                1.000606744                     0.06%
 * 11538       24        481                1.000520021                     0.05%
 * 11539       24        481                1.000433313                     0.04%
 * 11540       24        481                1.00034662                      0.03%
 * 11541       24        481                1.000259943                     0.03%
 * 11542       24        481                1.00017328                      0.02%
 * 11543       24        481                1.000086633                     0.01%
 * 11544       24        481                1                            ** 0.00%
 * 11545       23        502                1.000086618                     0.01%
 * 11546       23        502                1                            ** 0.00%
 * 11547       23        502                0.999913397                     0.01%
 * 11548       23        502                0.99982681                      0.02%
 * 11549       21        550                1.000086588                     0.01%
 * 11550       21        550                1                            ** 0.00%
 * 11551       21        550                0.999913427                     0.01%
 * 11552       21        550                0.99982687                      0.02%
 * 11553       21        550                0.999740327                     0.03%
 * 11554       21        550                0.9996538                       0.03%
 * 11555       21        550                0.999567287                     0.04%
 * 11556       21        550                0.999480789                     0.05%
 * 11557       21        550                0.999394306                     0.06%
 * 11558       21        550                0.999307839                     0.07%
 * 11559       21        550                0.999221386                     0.08%
 * 11560       24        482                1.000692042                     0.07%
 * 11561       24        482                1.000605484                     0.06%
 * 11562       24        482                1.000518941                     0.05%
 * 11563       24        482                1.000432414                     0.04%
 * 11564       24        482                1.000345901                     0.03%
 * 11565       24        482                1.000259403                     0.03%
 * 11566       24        482                1.000172921                     0.02%
 * 11567       24        482                1.000086453                     0.01%
 * 11568       24        482                1                            ** 0.00%
 * 11569       23        503                1                            ** 0.00%
 * 11570       23        503                0.99991357                      0.01%
 * 11571       22        526                1.000086423                     0.01%
 * 11572       22        526                1                            ** 0.00%
 * 11573       22        526                0.999913592                     0.01%
 * 11574       25        463                1.000086401                     0.01%
 * 11575       25        463                1                            ** 0.00%
 * 11576       25        463                0.999913614                     0.01%
 * 11577       25        463                0.999827244                     0.02%
 * 11578       25        463                0.999740888                     0.03%
 * 11579       25        463                0.999654547                     0.03%
 * 11580       25        463                0.999568221                     0.04%
 * 11581       25        463                0.99948191                      0.05%
 * 11582       25        463                0.999395614                     0.06%
 * 11583       25        463                0.999309333                     0.07%
 * 11584       23        504                1.000690608                     0.07%
 * 11585       23        504                1.00060423                      0.06%
 * 11586       23        504                1.000517866                     0.05%
 * 11587       23        504                1.000431518                     0.04%
 * 11588       23        504                1.000345185                     0.03%
 * 11589       23        504                1.000258866                     0.03%
 * 11590       23        504                1.000172563                     0.02%
 * 11591       23        504                1.000086274                     0.01%
 * 11592       23        504                1                            ** 0.00%
 * 11593       22        527                1.000086259                     0.01%
 * 11594       22        527                1                            ** 0.00%
 * 11595       22        527                0.999913756                     0.01%
 * 11596       22        527                0.999827527                     0.02%
 * 11597       22        527                0.999741312                     0.03%
 * 11598       25        464                1.000172444                     0.02%
 * 11599       25        464                1.000086214                     0.01%
 * 11600       25        464                1                            ** 0.00%
 * 11601       25        464                0.999913801                     0.01%
 * 11602       25        464                0.999827616                     0.02%
 * 11603       25        464                0.999741446                     0.03%
 * 11604       25        464                0.999655291                     0.03%
 * 11605       25        464                0.999569151                     0.04%
 * 11606       25        464                0.999483026                     0.05%
 * 11607       25        464                0.999396916                     0.06%
 * 11608       23        505                1.000603032                     0.06%
 * 11609       23        505                1.00051684                      0.05%
 * 11610       23        505                1.000430663                     0.04%
 * 11611       23        505                1.000344501                     0.03%
 * 11612       23        505                1.000258353                     0.03%
 * 11613       23        505                1.000172221                     0.02%
 * 11614       23        505                1.000086103                     0.01%
 * 11615       23        505                1                            ** 0.00%
 * 11616       22        528                1                            ** 0.00%
 * 11617       22        528                0.999913919                     0.01%
 * 11618       22        528                0.999827853                     0.02%
 * 11619       22        528                0.999741802                     0.03%
 * 11620       22        528                0.999655766                     0.03%
 * 11621       25        465                1.000344204                     0.03%
 * 11622       25        465                1.000258131                     0.03%
 * 11623       25        465                1.000172073                     0.02%
 * 11624       25        465                1.000086029                     0.01%
 * 11625       25        465                1                            ** 0.00%
 * 11626       25        465                0.999913986                     0.01%
 * 11627       25        465                0.999827987                     0.02%
 * 11628       25        465                0.999742002                     0.03%
 * 11629       25        465                0.999656032                     0.03%
 * 11630       25        465                0.999570077                     0.04%
 * 11631       25        465                0.999484137                     0.05%
 * 11632       22        529                1.000515818                     0.05%
 * 11633       22        529                1.000429812                     0.04%
 * 11634       22        529                1.00034382                      0.03%
 * 11635       22        529                1.000257843                     0.03%
 * 11636       22        529                1.00017188                      0.02%
 * 11637       22        529                1.000085933                     0.01%
 * 11638       22        529                1                            ** 0.00%
 * 11639       22        529                0.999914082                     0.01%
 * 11640       24        485                1                            ** 0.00%
 * 11641       24        485                0.999914097                     0.01%
 * 11642       24        485                0.999828208                     0.02%
 * 11643       24        485                0.999742334                     0.03%
 * 11644       24        485                0.999656475                     0.03%
 * 11645       24        485                0.999570631                     0.04%
 * 11646       25        466                1.000343466                     0.03%
 * 11647       25        466                1.000257577                     0.03%
 * 11648       25        466                1.000171703                     0.02%
 * 11649       25        466                1.000085844                     0.01%
 * 11650       25        466                1                            ** 0.00%
 * 11651       25        466                0.99991417                      0.01%
 * 11652       25        466                0.999828356                     0.02%
 * 11653       25        466                0.999742556                     0.03%
 * 11654       25        466                0.99965677                      0.03%
 * 11655       22        530                1.000429                        0.04%
 * 11656       22        530                1.000343171                     0.03%
 * 11657       22        530                1.000257356                     0.03%
 * 11658       22        530                1.000171556                     0.02%
 * 11659       22        530                1.000085771                     0.01%
 * 11660       22        530                1                            ** 0.00%
 * 11661       23        507                1                            ** 0.00%
 * 11662       23        507                0.999914251                     0.01%
 * 11663       24        486                1.000085741                     0.01%
 * 11664       24        486                1                            ** 0.00%
 * 11665       24        486                0.999914273                     0.01%
 * 11666       24        486                0.999828562                     0.02%
 * 11667       24        486                0.999742864                     0.03%
 * 11668       24        486                0.999657182                     0.03%
 * 11669       24        486                0.999571514                     0.04%
 * 11670       25        467                1.000428449                     0.04%
 * 11671       25        467                1.00034273                      0.03%
 * 11672       25        467                1.000257025                     0.03%
 * 11673       25        467                1.000171336                     0.02%
 * 11674       25        467                1.00008566                      0.01%
 * 11675       25        467                1                            ** 0.00%
 * 11676       25        467                0.999914354                     0.01%
 * 11677       25        467                0.999828723                     0.02%
 * 11678       25        467                0.999743107                     0.03%
 * 11679       22        531                1.000256871                     0.03%
 * 11680       22        531                1.000171233                     0.02%
 * 11681       22        531                1.000085609                     0.01%
 * 11682       22        531                1                            ** 0.00%
 * 11683       22        531                0.999914406                     0.01%
 * 11684       23        508                1                            ** 0.00%
 * 11685       23        508                0.99991442                      0.01%
 * 11686       23        508                0.999828855                     0.02%
 * 11687       24        487                1.000085565                     0.01%
 * 11688       24        487                1                            ** 0.00%
 * 11689       24        487                0.999914449                     0.01%
 * 11690       24        487                0.999828914                     0.02%
 * 11691       24        487                0.999743392                     0.03%
 * 11692       24        487                0.999657886                     0.03%
 * 11693       24        487                0.999572394                     0.04%
 * 11694       24        487                0.999486916                     0.05%
 * 11695       25        468                1.000427533                     0.04%
 * 11696       25        468                1.000341997                     0.03%
 * 11697       25        468                1.000256476                     0.03%
 * 11698       25        468                1.000170969                     0.02%
 * 11699       25        468                1.000085477                     0.01%
 * 11700       25        468                1                            ** 0.00%
 * 11701       25        468                0.999914537                     0.01%
 * 11702       22        532                1.000170911                     0.02%
 * 11703       22        532                1.000085448                     0.01%
 * 11704       22        532                1                            ** 0.00%
 * 11705       22        532                0.999914566                     0.01%
 * 11706       23        509                1.000085426                     0.01%
 * 11707       23        509                1                            ** 0.00%
 * 11708       23        509                0.999914588                     0.01%
 * 11709       23        509                0.999829191                     0.02%
 * 11710       24        488                1.000170794                     0.02%
 * 11711       24        488                1.00008539                      0.01%
 * 11712       24        488                1                            ** 0.00%
 * 11713       24        488                0.999914625                     0.01%
 * 11714       24        488                0.999829264                     0.02%
 * 11715       24        488                0.999743918                     0.03%
 * 11716       24        488                0.999658587                     0.03%
 * 11717       24        488                0.99957327                      0.04%
 * 11718       24        488                0.999487967                     0.05%
 * 11719       25        469                1.000511989                     0.05%
 * 11720       25        469                1.000426621                     0.04%
 * 11721       25        469                1.000341268                     0.03%
 * 11722       25        469                1.000255929                     0.03%
 * 11723       25        469                1.000170605                     0.02%
 * 11724       25        469                1.000085295                     0.01%
 * 11725       25        469                1                            ** 0.00%
 * 11726       22        533                1                            ** 0.00%
 * 11727       22        533                0.999914727                     0.01%
 * 11728       22        533                0.999829468                     0.02%
 * 11729       23        510                1.000085259                     0.01%
 * 11730       23        510                1                            ** 0.00%
 * 11731       23        510                0.999914756                     0.01%
 * 11732       23        510                0.999829526                     0.02%
 * 11733       23        510                0.999744311                     0.03%
 * 11734       24        489                1.000170445                     0.02%
 * 11735       24        489                1.000085215                     0.01%
 * 11736       24        489                1                            ** 0.00%
 * 11737       24        489                0.999914799                     0.01%
 * 11738       24        489                0.999829613                     0.02%
 * 11739       24        489                0.999744442                     0.03%
 * 11740       24        489                0.999659284                     0.03%
 * 11741       24        489                0.999574142                     0.04%
 * 11742       22        534                1.000510986                     0.05%
 * 11743       22        534                1.000425786                     0.04%
 * 11744       22        534                1.000340599                     0.03%
 * 11745       22        534                1.000255428                     0.03%
 * 11746       22        534                1.000170271                     0.02%
 * 11747       22        534                1.000085128                     0.01%
 * 11748       22        534                1                            ** 0.00%
 * 11749       22        534                0.999914886                     0.01%
 * 11750       25        470                1                            ** 0.00%
 * 11751       26        452                1.000085099                     0.01%
 * 11752       26        452                1                            ** 0.00%
 * 11753       23        511                1                            ** 0.00%
 * 11754       23        511                0.999914923                     0.01%
 * 11755       23        511                0.99982986                      0.02%
 * 11756       23        511                0.999744811                     0.03%
 * 11757       24        490                1.000255167                     0.03%
 * 11758       24        490                1.000170097                     0.02%
 * 11759       24        490                1.000085041                     0.01%
 * 11760       24        490                1                            ** 0.00%
 * 11761       24        490                0.999914973                     0.01%
 * 11762       24        490                0.999829961                     0.02%
 * 11763       24        490                0.999744963                     0.03%
 * 11764       24        490                0.99965998                      0.03%
 * 11765       24        490                0.999575011                     0.04%
 * 11766       22        535                1.000339963                     0.03%
 * 11767       22        535                1.00025495                      0.03%
 * 11768       22        535                1.000169952                     0.02%
 * 11769       22        535                1.000084969                     0.01%
 * 11770       22        535                1                            ** 0.00%
 * 11771       22        535                0.999915045                     0.01%
 * 11772       22        535                0.999830105                     0.02%
 * 11773       25        471                1.00016988                      0.02%
 * 11774       25        471                1.000084933                     0.01%
 * 11775       25        471                1                            ** 0.00%
 * 11776       23        512                1                            ** 0.00%
 * 11777       23        512                0.999915089                     0.01%
 * 11778       26        453                1                            ** 0.00%
 * 11779       26        453                0.999915103                     0.01%
 * 11780       26        453                0.999830221                     0.02%
 * 11781       24        491                1.000254647                     0.03%
 * 11782       24        491                1.00016975                      0.02%
 * 11783       24        491                1.000084868                     0.01%
 * 11784       24        491                1                            ** 0.00%
 * 11785       24        491                0.999915146                     0.01%
 * 11786       24        491                0.999830307                     0.02%
 * 11787       24        491                0.999745482                     0.03%
 * 11788       22        536                1.000339328                     0.03%
 * 11789       22        536                1.000254475                     0.03%
 * 11790       22        536                1.000169635                     0.02%
 * 11791       22        536                1.00008481                      0.01%
 * 11792       22        536                1                            ** 0.00%
 * 11793       22        536                0.999915204                     0.01%
 * 11794       22        536                0.999830422                     0.02%
 * 11795       22        536                0.999745655                     0.03%
 * 11796       23        513                1.000254323                     0.03%
 * 11797       23        513                1.000169535                     0.02%
 * 11798       23        513                1.00008476                      0.01%
 * 11799       23        513                1                            ** 0.00%
 * 11800       25        472                1                            ** 0.00%
 * 11801       25        472                0.999915261                     0.01%
 * 11802       25        472                0.999830537                     0.02%
 * 11803       26        454                1.000084724                     0.01%
 * 11804       26        454                1                            ** 0.00%
 * 11805       26        454                0.99991529                      0.01%
 * 11806       24        492                1.000169405                     0.02%
 * 11807       24        492                1.000084696                     0.01%
 * 11808       24        492                1                            ** 0.00%
 * 11809       24        492                0.999915319                     0.01%
 * 11810       24        492                0.999830652                     0.02%
 * 11811       22        537                1.000254001                     0.03%
 * 11812       22        537                1.000169319                     0.02%
 * 11813       22        537                1.000084653                     0.01%
 * 11814       22        537                1                            ** 0.00%
 * 11815       22        537                0.999915362                     0.01%
 * 11816       22        537                0.999830738                     0.02%
 * 11817       22        537                0.999746128                     0.03%
 * 11818       22        537                0.999661533                     0.03%
 * 11819       23        514                1.000253829                     0.03%
 * 11820       23        514                1.000169205                     0.02%
 * 11821       23        514                1.000084595                     0.01%
 * 11822       23        514                1                            ** 0.00%
 * 11823       23        514                0.999915419                     0.01%
 * 11824       25        473                1.000084574                     0.01%
 * 11825       25        473                1                            ** 0.00%
 * 11826       25        473                0.999915441                     0.01%
 * 11827       25        473                0.999830895                     0.02%
 * 11828       26        455                1.00016909                      0.02%
 * 11829       26        455                1.000084538                     0.01%
 * 11830       26        455                1                            ** 0.00%
 * 11831       26        455                0.999915476                     0.01%
 * 11832       24        493                1                            ** 0.00%
 * 11833       24        493                0.999915491                     0.01%
 * 11834       24        493                0.999830995                     0.02%
 * 11835       22        538                1.000084495                     0.01%
 * 11836       22        538                1                            ** 0.00%
 * 11837       22        538                0.999915519                     0.01%
 * 11838       22        538                0.999831053                     0.02%
 * 11839       22        538                0.9997466                       0.03%
 * 11840       22        538                0.999662162                     0.03%
 * 11841       23        515                1.000337809                     0.03%
 * 11842       23        515                1.000253336                     0.03%
 * 11843       23        515                1.000168876                     0.02%
 * 11844       23        515                1.000084431                     0.01%
 * 11845       23        515                1                            ** 0.00%
 * 11846       23        515                0.999915583                     0.01%
 * 11847       23        515                0.999831181                     0.02%
 * 11848       25        474                1.000168805                     0.02%
 * 11849       25        474                1.000084395                     0.01%
 * 11850       25        474                1                            ** 0.00%
 * 11851       25        474                0.999915619                     0.01%
 * 11852       25        474                0.999831252                     0.02%
 * 11853       25        474                0.9997469                       0.03%
 * 11854       24        494                1.000168719                     0.02%
 * 11855       24        494                1.000084353                     0.01%
 * 11856       24        494                1                            ** 0.00%
 * 11857       22        539                1.000084338                     0.01%
 * 11858       22        539                1                            ** 0.00%
 * 11859       22        539                0.999915676                     0.01%
 * 11860       22        539                0.999831366                     0.02%
 * 11861       22        539                0.99974707                      0.03%
 * 11862       22        539                0.999662789                     0.03%
 * 11863       22        539                0.999578521                     0.04%
 * 11864       23        516                1.000337154                     0.03%
 * 11865       23        516                1.000252845                     0.03%
 * 11866       23        516                1.000168549                     0.02%
 * 11867       23        516                1.000084267                     0.01%
 * 11868       23        516                1                            ** 0.00%
 * 11869       23        516                0.999915747                     0.01%
 * 11870       23        516                0.999831508                     0.02%
 * 11871       23        516                0.999747283                     0.03%
 * 11872       25        475                1.000252695                     0.03%
 * 11873       25        475                1.000168449                     0.02%
 * 11874       25        475                1.000084218                     0.01%
 * 11875       25        475                1                            ** 0.00%
 * 11876       25        475                0.999915797                     0.01%
 * 11877       25        475                0.999831607                     0.02%
 * 11878       22        540                1.000168379                     0.02%
 * 11879       22        540                1.000084182                     0.01%
 * 11880       22        540                1                            ** 0.00%
 * 11881       22        540                0.999915832                     0.01%
 * 11882       26        457                1                            ** 0.00%
 * 11883       26        457                0.999915846                     0.01%
 * 11884       26        457                0.999831706                     0.02%
 * 11885       26        457                0.999747581                     0.03%
 * 11886       26        457                0.99966347                      0.03%
 * 11887       23        517                1.000336502                     0.03%
 * 11888       23        517                1.000252355                     0.03%
 * 11889       23        517                1.000168223                     0.02%
 * 11890       23        517                1.000084104                     0.01%
 * 11891       23        517                1                            ** 0.00%
 * 11892       23        517                0.99991591                      0.01%
 * 11893       23        517                0.999831834                     0.02%
 * 11894       23        517                0.999747772                     0.03%
 * 11895       23        517                0.999663724                     0.03%
 * 11896       25        476                1.000336247                     0.03%
 * 11897       25        476                1.000252164                     0.03%
 * 11898       25        476                1.000168095                     0.02%
 * 11899       25        476                1.000084041                     0.01%
 * 11900       25        476                1                            ** 0.00%
 * 11901       22        541                1.000084027                     0.01%
 * 11902       22        541                1                            ** 0.00%
 * 11903       24        496                1.000084012                     0.01%
 * 11904       24        496                1                            ** 0.00%
 * 11905       24        496                0.999916002                     0.01%
 * 11906       24        496                0.999832017                     0.02%
 * 11907       26        458                1.000083984                     0.01%
 * 11908       26        458                1                            ** 0.00%
 * 11909       26        458                0.99991603                      0.01%
 * 11910       26        458                0.999832074                     0.02%
 * 11911       23        518                1.000251868                     0.03%
 * 11912       23        518                1.000167898                     0.02%
 * 11913       23        518                1.000083942                     0.01%
 * 11914       23        518                1                            ** 0.00%
 * 11915       23        518                0.999916072                     0.01%
 * 11916       23        518                0.999832158                     0.02%
 * 11917       23        518                0.999748259                     0.03%
 * 11918       23        518                0.999664373                     0.03%
 * 11919       23        518                0.999580502                     0.04%
 * 11920       22        542                1.00033557                      0.03%
 * 11921       22        542                1.000251657                     0.03%
 * 11922       22        542                1.000167757                     0.02%
 * 11923       22        542                1.000083872                     0.01%
 * 11924       22        542                1                            ** 0.00%
 * 11925       25        477                1                            ** 0.00%
 * 11926       25        477                0.99991615                      0.01%
 * 11927       24        497                1.000083843                     0.01%
 * 11928       24        497                1                            ** 0.00%
 * 11929       24        497                0.999916171                     0.01%
 * 11930       24        497                0.999832355                     0.02%
 * 11931       24        497                0.999748554                     0.03%
 * 11932       26        459                1.000167616                     0.02%
 * 11933       26        459                1.000083801                     0.01%
 * 11934       26        459                1                            ** 0.00%
 * 11935       26        459                0.999916213                     0.01%
 * 11936       23        519                1.00008378                      0.01%
 * 11937       23        519                1                            ** 0.00%
 * 11938       23        519                0.999916234                     0.01%
 * 11939       23        519                0.999832482                     0.02%
 * 11940       23        519                0.999748744                     0.03%
 * 11941       23        519                0.99966502                      0.03%
 * 11942       22        543                1.000334952                     0.03%
 * 11943       22        543                1.000251193                     0.03%
 * 11944       22        543                1.000167448                     0.02%
 * 11945       22        543                1.000083717                     0.01%
 * 11946       22        543                1                            ** 0.00%
 * 11947       22        543                0.999916297                     0.01%
 * 11948       22        543                0.999832608                     0.02%
 * 11949       25        478                1.000083689                     0.01%
 * 11950       25        478                1                            ** 0.00%
 * 11951       24        498                1.000083675                     0.01%
 * 11952       24        498                1                            ** 0.00%
 * 11953       24        498                0.999916339                     0.01%
 * 11954       24        498                0.999832692                     0.02%
 * 11955       24        498                0.999749059                     0.03%
 * 11956       23        520                1.00033456                      0.03%
 * 11957       23        520                1.000250899                     0.03%
 * 11958       23        520                1.000167252                     0.02%
 * 11959       23        520                1.000083619                     0.01%
 * 11960       23        520                1                            ** 0.00%
 * 11961       23        520                0.999916395                     0.01%
 * 11962       23        520                0.999832804                     0.02%
 * 11963       23        520                0.999749227                     0.03%
 * 11964       22        544                1.000334336                     0.03%
 * 11965       22        544                1.000250731                     0.03%
 * 11966       22        544                1.00016714                      0.02%
 * 11967       22        544                1.000083563                     0.01%
 * 11968       22        544                1                            ** 0.00%
 * 11969       22        544                0.999916451                     0.01%
 * 11970       22        544                0.999832916                     0.02%
 * 11971       22        544                0.999749394                     0.03%
 * 11972       25        479                1.000250585                     0.03%
 * 11973       25        479                1.000167043                     0.02%
 * 11974       25        479                1.000083514                     0.01%
 * 11975       25        479                1                            ** 0.00%
 * 11976       24        499                1                            ** 0.00%
 * 11977       24        499                0.999916507                     0.01%
 * 11978       24        499                0.999833027                     0.02%
 * 11979       24        499                0.999749562                     0.03%
 * 11980       23        521                1.000250417                     0.03%
 * 11981       23        521                1.000166931                     0.02%
 * 11982       23        521                1.000083459                     0.01%
 * 11983       23        521                1                            ** 0.00%
 * 11984       23        521                0.999916555                     0.01%
 * 11985       26        461                1.000083438                     0.01%
 * 11986       26        461                1                            ** 0.00%
 * 11987       26        461                0.999916576                     0.01%
 * 11988       22        545                1.000166834                     0.02%
 * 11989       22        545                1.00008341                      0.01%
 * 11990       22        545                1                            ** 0.00%
 * 11991       22        545                0.999916604                     0.01%
 * 11992       22        545                0.999833222                     0.02%
 * 11993       22        545                0.999749854                     0.03%
 * 11994       22        545                0.9996665                       0.03%
 * 11995       22        545                0.99958316                      0.04%
 * 11996       24        500                1.000333444                     0.03%
 * 11997       24        500                1.000250063                     0.03%
 * 11998       24        500                1.000166694                     0.02%
 * 11999       24        500                1.00008334                      0.01%
 * 12000       24        500                1                            ** 0.00%
 * 12001       24        500                0.999916674                     0.01%
 * 12002       24        500                0.999833361                     0.02%
 * 12003       23        522                1.000249938                     0.02%
 * 12004       23        522                1.000166611                     0.02%
 * 12005       23        522                1.000083299                     0.01%
 * 12006       23        522                1                            ** 0.00%
 * 12007       23        522                0.999916715                     0.01%
 * 12008       23        522                0.999833444                     0.02%
 * 12009       23        522                0.999750187                     0.02%
 * 12010       22        546                1.000166528                     0.02%
 * 12011       22        546                1.000083257                     0.01%
 * 12012       22        546                1                            ** 0.00%
 * 12013       22        546                0.999916757                     0.01%
 * 12014       22        546                0.999833528                     0.02%
 * 12015       22        546                0.999750312                     0.02%
 * 12016       22        546                0.999667111                     0.03%
 * 12017       22        546                0.999583923                     0.04%
 * 12018       22        546                0.999500749                     0.05%
 * 12019       24        501                1.000416008                     0.04%
 * 12020       24        501                1.000332779                     0.03%
 * 12021       24        501                1.000249563                     0.02%
 * 12022       24        501                1.000166362                     0.02%
 * 12023       24        501                1.000083174                     0.01%
 * 12024       24        501                1                            ** 0.00%
 * 12025       25        481                1                            ** 0.00%
 * 12026       25        481                0.999916847                     0.01%
 * 12027       23        523                1.000166293                     0.02%
 * 12028       23        523                1.000083139                     0.01%
 * 12029       23        523                1                            ** 0.00%
 * 12030       23        523                0.999916874                     0.01%
 * 12031       23        523                0.999833763                     0.02%
 * 12032       22        547                1.000166223                     0.02%
 * 12033       22        547                1.000083105                     0.01%
 * 12034       22        547                1                            ** 0.00%
 * 12035       22        547                0.999916909                     0.01%
 * 12036       26        463                1.000166168                     0.02%
 * 12037       26        463                1.000083077                     0.01%
 * 12038       26        463                1                            ** 0.00%
 * 12039       26        463                0.999916937                     0.01%
 * 12040       26        463                0.999833887                     0.02%
 * 12041       26        463                0.999750851                     0.02%
 * 12042       26        463                0.999667829                     0.03%
 * 12043       24        502                1.000415179                     0.04%
 * 12044       24        502                1.000332116                     0.03%
 * 12045       24        502                1.000249066                     0.02%
 * 12046       24        502                1.00016603                      0.02%
 * 12047       24        502                1.000083008                     0.01%
 * 12048       24        502                1                            ** 0.00%
 * 12049       24        502                0.999917006                     0.01%
 * 12050       25        482                1                            ** 0.00%
 * 12051       23        524                1.000082981                     0.01%
 * 12052       23        524                1                            ** 0.00%
 * 12053       23        524                0.999917033                     0.01%
 * 12054       22        548                1.00016592                      0.02%
 * 12055       22        548                1.000082953                     0.01%
 * 12056       22        548                1                            ** 0.00%
 * 12057       22        548                0.999917061                     0.01%
 * 12058       22        548                0.999834135                     0.02%
 * 12059       22        548                0.999751223                     0.02%
 * 12060       22        548                0.999668325                     0.03%
 * 12061       26        464                1.000248736                     0.02%
 * 12062       26        464                1.00016581                      0.02%
 * 12063       26        464                1.000082898                     0.01%
 * 12064       26        464                1                            ** 0.00%
 * 12065       26        464                0.999917116                     0.01%
 * 12066       26        464                0.999834245                     0.02%
 * 12067       26        464                0.999751388                     0.02%
 * 12068       24        503                1.000331455                     0.03%
 * 12069       24        503                1.000248571                     0.02%
 * 12070       24        503                1.0001657                       0.02%
 * 12071       24        503                1.000082843                     0.01%
 * 12072       24        503                1                            ** 0.00%
 * 12073       24        503                0.999917171                     0.01%
 * 12074       23        525                1.000082823                     0.01%
 * 12075       23        525                1                            ** 0.00%
 * 12076       23        525                0.999917191                     0.01%
 * 12077       22        549                1.000082802                     0.01%
 * 12078       22        549                1                            ** 0.00%
 * 12079       22        549                0.999917212                     0.01%
 * 12080       22        549                0.999834437                     0.02%
 * 12081       22        549                0.999751676                     0.02%
 * 12082       22        549                0.999668929                     0.03%
 * 12083       22        549                0.999586195                     0.04%
 * 12084       22        549                0.999503476                     0.05%
 * 12085       26        465                1.000413736                     0.04%
 * 12086       26        465                1.000330961                     0.03%
 * 12087       26        465                1.000248201                     0.02%
 * 12088       26        465                1.000165453                     0.02%
 * 12089       26        465                1.00008272                      0.01%
 * 12090       26        465                1                            ** 0.00%
 * 12091       26        465                0.999917294                     0.01%
 * 12092       26        465                0.999834601                     0.02%
 * 12093       24        504                1.000248077                     0.02%
 * 12094       24        504                1.000165371                     0.02%
 * 12095       24        504                1.000082679                     0.01%
 * 12096       24        504                1                            ** 0.00%
 * 12097       23        526                1.000082665                     0.01%
 * 12098       23        526                1                            ** 0.00%
 * 12099       22        550                1.000082651                     0.01%
 * 12100       22        550                1                            ** 0.00%
 * 12101       22        550                0.999917362                     0.01%
 * 12102       22        550                0.999834738                     0.02%
 * 12103       22        550                0.999752128                     0.02%
 * 12104       22        550                0.999669531                     0.03%
 * 12105       22        550                0.999586948                     0.04%
 * 12106       22        550                0.999504378                     0.05%
 * 12107       22        550                0.999421822                     0.06%
 * 12108       22        550                0.99933928                      0.07%
 * 12109       26        466                1.000578082                     0.06%
 * 12110       26        466                1.000495458                     0.05%
 * 12111       26        466                1.000412848                     0.04%
 * 12112       26        466                1.000330251                     0.03%
 * 12113       26        466                1.000247668                     0.02%
 * 12114       26        466                1.000165098                     0.02%
 * 12115       26        466                1.000082542                     0.01%
 * 12116       26        466                1                            ** 0.00%
 * 12117       26        466                0.999917471                     0.01%
 * 12118       24        505                1.000165044                     0.02%
 * 12119       24        505                1.000082515                     0.01%
 * 12120       24        505                1                            ** 0.00%
 * 12121       23        527                1                            ** 0.00%
 * 12122       23        527                0.999917505                     0.01%
 * 12123       23        527                0.999835024                     0.02%
 * 12124       25        485                1.000082481                     0.01%
 * 12125       25        485                1                            ** 0.00%
 * 12126       25        485                0.999917533                     0.01%
 * 12127       25        485                0.999835079                     0.02%
 * 12128       25        485                0.999752639                     0.02%
 * 12129       25        485                0.999670212                     0.03%
 * 12130       25        485                0.999587799                     0.04%
 * 12131       25        485                0.999505399                     0.05%
 * 12132       25        485                0.999423014                     0.06%
 * 12133       25        485                0.999340641                     0.07%
 * 12134       26        467                1.000659304                     0.07%
 * 12135       26        467                1.000576844                     0.06%
 * 12136       26        467                1.000494397                     0.05%
 * 12137       26        467                1.000411963                     0.04%
 * 12138       26        467                1.000329544                     0.03%
 * 12139       26        467                1.000247137                     0.02%
 * 12140       26        467                1.000164745                     0.02%
 * 12141       26        467                1.000082366                     0.01%
 * 12142       26        467                1                            ** 0.00%
 * 12143       23        528                1.000082352                     0.01%
 * 12144       23        528                1                            ** 0.00%
 * 12145       23        528                0.999917662                     0.01%
 * 12146       23        528                0.999835337                     0.02%
 * 12147       23        528                0.999753025                     0.02%
 * 12148       25        486                1.000164636                     0.02%
 * 12149       25        486                1.000082311                     0.01%
 * 12150       25        486                1                            ** 0.00%
 * 12151       25        486                0.999917702                     0.01%
 * 12152       25        486                0.999835418                     0.02%
 * 12153       25        486                0.999753147                     0.02%
 * 12154       25        486                0.99967089                      0.03%
 * 12155       25        486                0.999588647                     0.04%
 * 12156       25        486                0.999506417                     0.05%
 * 12157       25        486                0.9994242                       0.06%
 * 12158       25        486                0.999341997                     0.07%
 * 12159       23        529                1.000657949                     0.07%
 * 12160       23        529                1.000575658                     0.06%
 * 12161       23        529                1.00049338                      0.05%
 * 12162       23        529                1.000411117                     0.04%
 * 12163       23        529                1.000328866                     0.03%
 * 12164       23        529                1.000246629                     0.02%
 * 12165       23        529                1.000164406                     0.02%
 * 12166       23        529                1.000082196                     0.01%
 * 12167       23        529                1                            ** 0.00%
 * 12168       24        507                1                            ** 0.00%
 * 12169       24        507                0.999917824                     0.01%
 * 12170       24        507                0.999835661                     0.02%
 * 12171       24        507                0.999753512                     0.02%
 * 12172       25        487                1.000246467                     0.02%
 * 12173       25        487                1.000164298                     0.02%
 * 12174       25        487                1.000082142                     0.01%
 * 12175       25        487                1                            ** 0.00%
 * 12176       25        487                0.999917871                     0.01%
 * 12177       27        451                1                            ** 0.00%
 * 12178       27        451                0.999917885                     0.01%
 * 12179       27        451                0.999835783                     0.02%
 * 12180       27        451                0.999753695                     0.02%
 * 12181       27        451                0.99967162                      0.03%
 * 12182       27        451                0.999589558                     0.04%
 * 12183       27        451                0.99950751                      0.05%
 * 12184       23        530                1.000492449                     0.05%
 * 12185       23        530                1.000410341                     0.04%
 * 12186       23        530                1.000328246                     0.03%
 * 12187       23        530                1.000246164                     0.02%
 * 12188       23        530                1.000164096                     0.02%
 * 12189       23        530                1.000082041                     0.01%
 * 12190       23        530                1                            ** 0.00%
 * 12191       23        530                0.999917972                     0.01%
 * 12192       24        508                1                            ** 0.00%
 * 12193       24        508                0.999917986                     0.01%
 * 12194       26        469                1                            ** 0.00%
 * 12195       26        469                0.999917999                     0.01%
 * 12196       26        469                0.999836012                     0.02%
 * 12197       25        488                1.000245962                     0.02%
 * 12198       25        488                1.000163961                     0.02%
 * 12199       25        488                1.000081974                     0.01%
 * 12200       25        488                1                            ** 0.00%
 * 12201       25        488                0.99991804                      0.01%
 * 12202       25        488                0.999836092                     0.02%
 * 12203       27        452                1.000081947                     0.01%
 * 12204       27        452                1                            ** 0.00%
 * 12205       27        452                0.999918066                     0.01%
 * 12206       27        452                0.999836146                     0.02%
 * 12207       27        452                0.999754239                     0.02%
 * 12208       27        452                0.999672346                     0.03%
 * 12209       23        531                1.000327627                     0.03%
 * 12210       23        531                1.0002457                       0.02%
 * 12211       23        531                1.000163787                     0.02%
 * 12212       23        531                1.000081887                     0.01%
 * 12213       23        531                1                            ** 0.00%
 * 12214       23        531                0.999918127                     0.01%
 * 12215       24        509                1.000081867                     0.01%
 * 12216       24        509                1                            ** 0.00%
 * 12217       24        509                0.999918147                     0.01%
 * 12218       24        509                0.999836307                     0.02%
 * 12219       26        470                1.00008184                      0.01%
 * 12220       26        470                1                            ** 0.00%
 * 12221       26        470                0.999918174                     0.01%
 * 12222       26        470                0.999836361                     0.02%
 * 12223       25        489                1.000163626                     0.02%
 * 12224       25        489                1.000081806                     0.01%
 * 12225       25        489                1                            ** 0.00%
 * 12226       25        489                0.999918207                     0.01%
 * 12227       25        489                0.999836428                     0.02%
 * 12228       25        489                0.999754661                     0.02%
 * 12229       27        453                1.000163546                     0.02%
 * 12230       27        453                1.000081766                     0.01%
 * 12231       27        453                1                            ** 0.00%
 * 12232       27        453                0.999918247                     0.01%
 * 12233       27        453                0.999836508                     0.02%
 * 12234       23        532                1.000163479                     0.02%
 * 12235       23        532                1.000081733                     0.01%
 * 12236       23        532                1                            ** 0.00%
 * 12237       23        532                0.999918281                     0.01%
 * 12238       23        532                0.999836575                     0.02%
 * 12239       24        510                1.000081706                     0.01%
 * 12240       24        510                1                            ** 0.00%
 * 12241       24        510                0.999918307                     0.01%
 * 12242       24        510                0.999836628                     0.02%
 * 12243       26        471                1.000245038                     0.02%
 * 12244       26        471                1.000163345                     0.02%
 * 12245       26        471                1.000081666                     0.01%
 * 12246       26        471                1                            ** 0.00%
 * 12247       26        471                0.999918347                     0.01%
 * 12248       25        490                1.000163292                     0.02%
 * 12249       25        490                1.000081639                     0.01%
 * 12250       25        490                1                            ** 0.00%
 * 12251       25        490                0.999918374                     0.01%
 * 12252       25        490                0.999836761                     0.02%
 * 12253       25        490                0.999755162                     0.02%
 * 12254       25        490                0.999673576                     0.03%
 * 12255       27        454                1.000244798                     0.02%
 * 12256       27        454                1.000163185                     0.02%
 * 12257       27        454                1.000081586                     0.01%
 * 12258       27        454                1                            ** 0.00%
 * 12259       23        533                1                            ** 0.00%
 * 12260       23        533                0.999918434                     0.01%
 * 12261       23        533                0.999836881                     0.02%
 * 12262       24        511                1.000163106                     0.02%
 * 12263       24        511                1.000081546                     0.01%
 * 12264       24        511                1                            ** 0.00%
 * 12265       24        511                0.999918467                     0.01%
 * 12266       24        511                0.999836948                     0.02%
 * 12267       24        511                0.999755441                     0.02%
 * 12268       24        511                0.999673948                     0.03%
 * 12269       26        472                1.000244519                     0.02%
 * 12270       26        472                1.000162999                     0.02%
 * 12271       26        472                1.000081493                     0.01%
 * 12272       26        472                1                            ** 0.00%
 * 12273       26        472                0.99991852                      0.01%
 * 12274       25        491                1.000081473                     0.01%
 * 12275       25        491                1                            ** 0.00%
 * 12276       25        491                0.99991854                      0.01%
 * 12277       25        491                0.999837094                     0.02%
 * 12278       25        491                0.999755661                     0.02%
 * 12279       23        534                1.00024432                      0.02%
 * 12280       23        534                1.000162866                     0.02%
 * 12281       23        534                1.000081427                     0.01%
 * 12282       23        534                1                            ** 0.00%
 * 12283       23        534                0.999918587                     0.01%
 * 12284       27        455                1.000081407                     0.01%
 * 12285       27        455                1                            ** 0.00%
 * 12286       27        455                0.999918607                     0.01%
 * 12287       24        512                1.000081387                     0.01%
 * 12288       24        512                1                            ** 0.00%
 * 12289       24        512                0.999918626                     0.01%
 * 12290       24        512                0.999837266                     0.02%
 * 12291       24        512                0.999755919                     0.02%
 * 12292       24        512                0.999674585                     0.03%
 * 12293       24        512                0.999593264                     0.04%
 * 12294       26        473                1.000325362                     0.03%
 * 12295       26        473                1.000244002                     0.02%
 * 12296       26        473                1.000162655                     0.02%
 * 12297       26        473                1.000081321                     0.01%
 * 12298       26        473                1                            ** 0.00%
 * 12299       25        492                1.000081307                     0.01%
 * 12300       25        492                1                            ** 0.00%
 * 12301       25        492                0.999918706                     0.01%
 * 12302       25        492                0.999837425                     0.02%
 * 12303       23        535                1.000162562                     0.02%
 * 12304       23        535                1.000081274                     0.01%
 * 12305       23        535                1                            ** 0.00%
 * 12306       23        535                0.999918739                     0.01%
 * 12307       23        535                0.999837491                     0.02%
 * 12308       23        535                0.999756256                     0.02%
 * 12309       24        513                1.000243724                     0.02%
 * 12310       24        513                1.00016247                      0.02%
 * 12311       24        513                1.000081228                     0.01%
 * 12312       24        513                1                            ** 0.00%
 * 12313       24        513                0.999918785                     0.01%
 * 12314       24        513                0.999837583                     0.02%
 * 12315       24        513                0.999756395                     0.02%
 * 12316       24        513                0.999675219                     0.03%
 * 12317       24        513                0.999594057                     0.04%
 * 12318       24        513                0.999512908                     0.05%
 * 12319       26        474                1.000405877                     0.04%
 * 12320       26        474                1.000324675                     0.03%
 * 12321       26        474                1.000243487                     0.02%
 * 12322       26        474                1.000162311                     0.02%
 * 12323       26        474                1.000081149                     0.01%
 * 12324       26        474                1                            ** 0.00%
 * 12325       25        493                1                            ** 0.00%
 * 12326       25        493                0.999918871                     0.01%
 * 12327       23        536                1.000081123                     0.01%
 * 12328       23        536                1                            ** 0.00%
 * 12329       23        536                0.99991889                      0.01%
 * 12330       23        536                0.999837794                     0.02%
 * 12331       23        536                0.999756711                     0.02%
 * 12332       24        514                1.000324359                     0.03%
 * 12333       24        514                1.00024325                      0.02%
 * 12334       24        514                1.000162153                     0.02%
 * 12335       24        514                1.00008107                      0.01%
 * 12336       24        514                1                            ** 0.00%
 * 12337       24        514                0.999918943                     0.01%
 * 12338       27        457                1.00008105                      0.01%
 * 12339       27        457                1                            ** 0.00%
 * 12340       27        457                0.999918963                     0.01%
 * 12341       27        457                0.999837939                     0.02%
 * 12342       27        457                0.999756928                     0.02%
 * 12343       27        457                0.99967593                      0.03%
 * 12344       27        457                0.999594945                     0.04%
 * 12345       25        494                1.000405022                     0.04%
 * 12346       25        494                1.000323992                     0.03%
 * 12347       25        494                1.000242974                     0.02%
 * 12348       25        494                1.00016197                      0.02%
 * 12349       25        494                1.000080978                     0.01%
 * 12350       25        494                1                            ** 0.00%
 * 12351       23        537                1                            ** 0.00%
 * 12352       23        537                0.999919041                     0.01%
 * 12353       23        537                0.999838096                     0.02%
 * 12354       23        537                0.999757164                     0.02%
 * 12355       23        537                0.999676244                     0.03%
 * 12356       24        515                1.000323729                     0.03%
 * 12357       24        515                1.000242777                     0.02%
 * 12358       24        515                1.000161838                     0.02%
 * 12359       24        515                1.000080913                     0.01%
 * 12360       24        515                1                            ** 0.00%
 * 12361       24        515                0.9999191                       0.01%
 * 12362       24        515                0.999838214                     0.02%
 * 12363       24        515                0.99975734                      0.02%
 * 12364       27        458                1.00016176                      0.02%
 * 12365       27        458                1.000080873                     0.01%
 * 12366       27        458                1                            ** 0.00%
 * 12367       27        458                0.99991914                      0.01%
 * 12368       27        458                0.999838292                     0.02%
 * 12369       27        458                0.999757458                     0.02%
 * 12370       23        538                1.000323363                     0.03%
 * 12371       23        538                1.000242503                     0.02%
 * 12372       23        538                1.000161655                     0.02%
 * 12373       23        538                1.000080821                     0.01%
 * 12374       23        538                1                            ** 0.00%
 * 12375       25        495                1                            ** 0.00%
 * 12376       26        476                1                            ** 0.00%
 * 12377       26        476                0.999919205                     0.01%
 * 12378       26        476                0.999838423                     0.02%
 * 12379       26        476                0.999757654                     0.02%
 * 12380       24        516                1.000323102                     0.03%
 * 12381       24        516                1.000242307                     0.02%
 * 12382       24        516                1.000161525                     0.02%
 * 12383       24        516                1.000080756                     0.01%
 * 12384       24        516                1                            ** 0.00%
 * 12385       24        516                0.999919257                     0.01%
 * 12386       24        516                0.999838527                     0.02%
 * 12387       24        516                0.999757811                     0.02%
 * 12388       24        516                0.999677107                     0.03%
 * 12389       27        459                1.000322867                     0.03%
 * 12390       27        459                1.000242131                     0.02%
 * 12391       27        459                1.000161407                     0.02%
 * 12392       27        459                1.000080697                     0.01%
 * 12393       27        459                1                            ** 0.00%
 * 12394       27        459                0.999919316                     0.01%
 * 12395       23        539                1.000161355                     0.02%
 * 12396       23        539                1.000080671                     0.01%
 * 12397       23        539                1                            ** 0.00%
 * 12398       23        539                0.999919342                     0.01%
 * 12399       25        496                1.000080652                     0.01%
 * 12400       25        496                1                            ** 0.00%
 * 12401       26        477                1.000080639                     0.01%
 * 12402       26        477                1                            ** 0.00%
 * 12403       26        477                0.999919374                     0.01%
 * 12404       26        477                0.999838762                     0.02%
 * 12405       26        477                0.999758162                     0.02%
 * 12406       24        517                1.000161212                     0.02%
 * 12407       24        517                1.0000806                       0.01%
 * 12408       24        517                1                            ** 0.00%
 * 12409       24        517                0.999919413                     0.01%
 * 12410       24        517                0.99983884                      0.02%
 * 12411       24        517                0.999758279                     0.02%
 * 12412       24        517                0.999677731                     0.03%
 * 12413       24        517                0.999597196                     0.04%
 * 12414       23        540                1.000483325                     0.05%
 * 12415       23        540                1.000402739                     0.04%
 * 12416       23        540                1.000322165                     0.03%
 * 12417       23        540                1.000241604                     0.02%
 * 12418       23        540                1.000161057                     0.02%
 * 12419       23        540                1.000080522                     0.01%
 * 12420       23        540                1                            ** 0.00%
 * 12421       23        540                0.999919491                     0.01%
 * 12422       23        540                0.999838995                     0.02%
 * 12423       25        497                1.000160992                     0.02%
 * 12424       25        497                1.000080489                     0.01%
 * 12425       25        497                1                            ** 0.00%
 * 12426       25        497                0.999919524                     0.01%
 * 12427       26        478                1.00008047                      0.01%
 * 12428       26        478                1                            ** 0.00%
 * 12429       26        478                0.999919543                     0.01%
 * 12430       24        518                1.000160901                     0.02%
 * 12431       24        518                1.000080444                     0.01%
 * 12432       24        518                1                            ** 0.00%
 * 12433       24        518                0.999919569                     0.01%
 * 12434       24        518                0.999839151                     0.02%
 * 12435       24        518                0.999758745                     0.02%
 * 12436       24        518                0.999678353                     0.03%
 * 12437       24        518                0.999597974                     0.04%
 * 12438       23        541                1.000401994                     0.04%
 * 12439       23        541                1.000321569                     0.03%
 * 12440       23        541                1.000241158                     0.02%
 * 12441       23        541                1.000160759                     0.02%
 * 12442       23        541                1.000080373                     0.01%
 * 12443       23        541                1                            ** 0.00%
 * 12444       23        541                0.99991964                      0.01%
 * 12445       23        541                0.999839293                     0.02%
 * 12446       27        461                1.000080347                     0.01%
 * 12447       27        461                1                            ** 0.00%
 * 12448       27        461                0.999919666                     0.01%
 * 12449       25        498                1.000080328                     0.01%
 * 12450       25        498                1                            ** 0.00%
 * 12451       25        498                0.999919685                     0.01%
 * 12452       26        479                1.000160617                     0.02%
 * 12453       26        479                1.000080302                     0.01%
 * 12454       26        479                1                            ** 0.00%
 * 12455       26        479                0.999919711                     0.01%
 * 12456       24        519                1                            ** 0.00%
 * 12457       24        519                0.999919724                     0.01%
 * 12458       24        519                0.999839461                     0.02%
 * 12459       24        519                0.99975921                      0.02%
 * 12460       24        519                0.999678973                     0.03%
 * 12461       24        519                0.999598748                     0.04%
 * 12462       23        542                1.000320976                     0.03%
 * 12463       23        542                1.000240713                     0.02%
 * 12464       23        542                1.000160462                     0.02%
 * 12465       23        542                1.000080225                     0.01%
 * 12466       23        542                1                            ** 0.00%
 * 12467       23        542                0.999919788                     0.01%
 * 12468       23        542                0.999839589                     0.02%
 * 12469       23        542                0.999759403                     0.02%
 * 12470       23        542                0.99967923                      0.03%
 * 12471       27        462                1.000240558                     0.02%
 * 12472       27        462                1.000160359                     0.02%
 * 12473       27        462                1.000080173                     0.01%
 * 12474       27        462                1                            ** 0.00%
 * 12475       25        499                1                            ** 0.00%
 * 12476       25        499                0.999919846                     0.01%
 * 12477       25        499                0.999839705                     0.02%
 * 12478       24        520                1.000160282                     0.02%
 * 12479       24        520                1.000080135                     0.01%
 * 12480       24        520                1                            ** 0.00%
 * 12481       24        520                0.999919878                     0.01%
 * 12482       24        520                0.999839769                     0.02%
 * 12483       24        520                0.999759673                     0.02%
 * 12484       24        520                0.99967959                      0.03%
 * 12485       23        543                1.000320384                     0.03%
 * 12486       23        543                1.000240269                     0.02%
 * 12487       23        543                1.000160167                     0.02%
 * 12488       23        543                1.000080077                     0.01%
 * 12489       23        543                1                            ** 0.00%
 * 12490       23        543                0.999919936                     0.01%
 * 12491       23        543                0.999839885                     0.02%
 * 12492       23        543                0.999759846                     0.02%
 * 12493       23        543                0.999679821                     0.03%
 * 12494       23        543                0.999599808                     0.04%
 * 12495       25        500                1.00040016                      0.04%
 * 12496       25        500                1.000320102                     0.03%
 * 12497       25        500                1.000240058                     0.02%
 * 12498       25        500                1.000160026                     0.02%
 * 12499       25        500                1.000080006                     0.01%
 * 12500       25        500                1                            ** 0.00%
 * 12501       27        463                1                            ** 0.00%
 * 12502       27        463                0.999920013                     0.01%
 * 12503       24        521                1.000079981                     0.01%
 * 12504       24        521                1                            ** 0.00%
 * 12505       24        521                0.999920032                     0.01%
 * 12506       26        481                1                            ** 0.00%
 * 12507       26        481                0.999920045                     0.01%
 * 12508       26        481                0.999840102                     0.02%
 * 12509       23        544                1.000239827                     0.02%
 * 12510       23        544                1.000159872                     0.02%
 * 12511       23        544                1.00007993                      0.01%
 * 12512       23        544                1                            ** 0.00%
 * 12513       23        544                0.999920083                     0.01%
 * 12514       23        544                0.999840179                     0.02%
 * 12515       23        544                0.999760288                     0.02%
 * 12516       23        544                0.999680409                     0.03%
 * 12517       23        544                0.999600543                     0.04%
 * 12518       23        544                0.99952069                      0.05%
 * 12519       25        501                1.000479272                     0.05%
 * 12520       25        501                1.000399361                     0.04%
 * 12521       25        501                1.000319463                     0.03%
 * 12522       25        501                1.000239578                     0.02%
 * 12523       25        501                1.000159706                     0.02%
 * 12524       25        501                1.000079847                     0.01%
 * 12525       25        501                1                            ** 0.00%
 * 12526       25        501                0.999920166                     0.01%
 * 12527       24        522                1.000079828                     0.01%
 * 12528       24        522                1                            ** 0.00%
 * 12529       24        522                0.999920185                     0.01%
 * 12530       26        482                1.000159617                     0.02%
 * 12531       26        482                1.000079802                     0.01%
 * 12532       26        482                1                            ** 0.00%
 * 12533       26        482                0.999920211                     0.01%
 * 12534       23        545                1.000079783                     0.01%
 * 12535       23        545                1                            ** 0.00%
 * 12536       23        545                0.99992023                      0.01%
 * 12537       23        545                0.999840472                     0.02%
 * 12538       23        545                0.999760727                     0.02%
 * 12539       23        545                0.999680995                     0.03%
 * 12540       23        545                0.999601276                     0.04%
 * 12541       23        545                0.999521569                     0.05%
 * 12542       23        545                0.999441875                     0.06%
 * 12543       25        502                1.00055808                      0.06%
 * 12544       25        502                1.000478316                     0.05%
 * 12545       25        502                1.000398565                     0.04%
 * 12546       25        502                1.000318827                     0.03%
 * 12547       25        502                1.000239101                     0.02%
 * 12548       25        502                1.000159388                     0.02%
 * 12549       25        502                1.000079688                     0.01%
 * 12550       25        502                1                            ** 0.00%
 * 12551       25        502                0.999920325                     0.01%
 * 12552       24        523                1                            ** 0.00%
 * 12553       24        523                0.999920338                     0.01%
 * 12554       27        465                1.000079656                     0.01%
 * 12555       27        465                1                            ** 0.00%
 * 12556       27        465                0.999920357                     0.01%
 * 12557       23        546                1.000079637                     0.01%
 * 12558       23        546                1                            ** 0.00%
 * 12559       23        546                0.999920376                     0.01%
 * 12560       23        546                0.999840764                     0.02%
 * 12561       23        546                0.999761166                     0.02%
 * 12562       23        546                0.999681579                     0.03%
 * 12563       23        546                0.999602006                     0.04%
 * 12564       23        546                0.999522445                     0.05%
 * 12565       23        546                0.999442897                     0.06%
 * 12566       23        546                0.999363361                     0.06%
 * 12567       25        503                1.000636588                     0.06%
 * 12568       25        503                1.00055697                      0.06%
 * 12569       25        503                1.000477365                     0.05%
 * 12570       25        503                1.000397772                     0.04%
 * 12571       25        503                1.000318193                     0.03%
 * 12572       25        503                1.000238626                     0.02%
 * 12573       25        503                1.000159071                     0.02%
 * 12574       25        503                1.000079529                     0.01%
 * 12575       25        503                1                            ** 0.00%
 * 12576       24        524                1                            ** 0.00%
 * 12577       24        524                0.99992049                      0.01%
 * 12578       24        524                0.999840992                     0.02%
 * 12579       23        547                1.000158995                     0.02%
 * 12580       23        547                1.000079491                     0.01%
 * 12581       23        547                1                            ** 0.00%
 * 12582       27        466                1                            ** 0.00%
 * 12583       27        466                0.999920528                     0.01%
 * 12584       26        484                1                            ** 0.00%
 * 12585       26        484                0.99992054                      0.01%
 * 12586       26        484                0.999841093                     0.02%
 * 12587       26        484                0.999761659                     0.02%
 * 12588       26        484                0.999682237                     0.03%
 * 12589       26        484                0.999602828                     0.04%
 * 12590       26        484                0.999523431                     0.05%
 * 12591       26        484                0.999444047                     0.06%
 * 12592       24        525                1.000635324                     0.06%
 * 12593       24        525                1.000555864                     0.06%
 * 12594       24        525                1.000476417                     0.05%
 * 12595       24        525                1.000396983                     0.04%
 * 12596       24        525                1.000317561                     0.03%
 * 12597       24        525                1.000238152                     0.02%
 * 12598       24        525                1.000158755                     0.02%
 * 12599       24        525                1.000079371                     0.01%
 * 12600       24        525                1                            ** 0.00%
 * 12601       24        525                0.999920641                     0.01%
 * 12602       23        548                1.000158705                     0.02%
 * 12603       23        548                1.000079346                     0.01%
 * 12604       23        548                1                            ** 0.00%
 * 12605       23        548                0.999920666                     0.01%
 * 12606       23        548                0.999841345                     0.02%
 * 12607       27        467                1.000158642                     0.02%
 * 12608       27        467                1.000079315                     0.01%
 * 12609       27        467                1                            ** 0.00%
 * 12610       26        485                1                            ** 0.00%
 * 12611       26        485                0.999920704                     0.01%
 * 12612       26        485                0.999841421                     0.02%
 * 12613       26        485                0.99976215                      0.02%
 * 12614       26        485                0.999682892                     0.03%
 * 12615       26        485                0.999603646                     0.04%
 * 12616       26        485                0.999524413                     0.05%
 * 12617       24        526                1.000554807                     0.06%
 * 12618       24        526                1.000475511                     0.05%
 * 12619       24        526                1.000396228                     0.04%
 * 12620       24        526                1.000316957                     0.03%
 * 12621       24        526                1.000237699                     0.02%
 * 12622       24        526                1.000158453                     0.02%
 * 12623       24        526                1.00007922                      0.01%
 * 12624       24        526                1                            ** 0.00%
 * 12625       25        505                1                            ** 0.00%
 * 12626       23        549                1.000079202                     0.01%
 * 12627       23        549                1                            ** 0.00%
 * 12628       28        451                1                            ** 0.00%
 * 12629       28        451                0.999920817                     0.01%
 * 12630       28        451                0.999841647                     0.02%
 * 12631       28        451                0.999762489                     0.02%
 * 12632       26        486                1.000316656                     0.03%
 * 12633       26        486                1.000237473                     0.02%
 * 12634       26        486                1.000158303                     0.02%
 * 12635       26        486                1.000079145                     0.01%
 * 12636       26        486                1                            ** 0.00%
 * 12637       26        486                0.999920867                     0.01%
 * 12638       26        486                0.999841747                     0.02%
 * 12639       26        486                0.999762639                     0.02%
 * 12640       26        486                0.999683544                     0.03%
 * 12641       26        486                0.999604462                     0.04%
 * 12642       24        527                1.000474608                     0.05%
 * 12643       24        527                1.000395476                     0.04%
 * 12644       24        527                1.000316356                     0.03%
 * 12645       24        527                1.000237248                     0.02%
 * 12646       24        527                1.000158153                     0.02%
 * 12647       24        527                1.00007907                      0.01%
 * 12648       24        527                1                            ** 0.00%
 * 12649       24        527                0.999920942                     0.01%
 * 12650       23        550                1                            ** 0.00%
 * 12651       23        550                0.999920955                     0.01%
 * 12652       23        550                0.999841922                     0.02%
 * 12653       23        550                0.999762902                     0.02%
 * 12654       28        452                1.000158053                     0.02%
 * 12655       28        452                1.00007902                      0.01%
 * 12656       28        452                1                            ** 0.00%
 * 12657       28        452                0.999920992                     0.01%
 * 12658       28        452                0.999841997                     0.02%
 * 12659       26        487                1.000236986                     0.02%
 * 12660       26        487                1.000157978                     0.02%
 * 12661       26        487                1.000078983                     0.01%
 * 12662       26        487                1                            ** 0.00%
 * 12663       27        469                1                            ** 0.00%
 * 12664       27        469                0.999921036                     0.01%
 * 12665       27        469                0.999842084                     0.02%
 * 12666       27        469                0.999763145                     0.02%
 * 12667       27        469                0.999684219                     0.03%
 * 12668       24        528                1.000315756                     0.03%
 * 12669       24        528                1.000236798                     0.02%
 * 12670       24        528                1.000157853                     0.02%
 * 12671       24        528                1.00007892                      0.01%
 * 12672       24        528                1                            ** 0.00%
 * 12673       24        528                0.999921092                     0.01%
 * 12674       25        507                1.000078902                     0.01%
 * 12675       25        507                1                            ** 0.00%
 * 12676       25        507                0.999921111                     0.01%
 * 12677       25        507                0.999842234                     0.02%
 * 12678       25        507                0.99976337                      0.02%
 * 12679       25        507                0.999684518                     0.03%
 * 12680       28        453                1.000315457                     0.03%
 * 12681       28        453                1.000236574                     0.02%
 * 12682       28        453                1.000157704                     0.02%
 * 12683       28        453                1.000078846                     0.01%
 * 12684       28        453                1                            ** 0.00%
 * 12685       28        453                0.999921167                     0.01%
 * 12686       26        488                1.000157654                     0.02%
 * 12687       26        488                1.000078821                     0.01%
 * 12688       26        488                1                            ** 0.00%
 * 12689       26        488                0.999921192                     0.01%
 * 12690       27        470                1                            ** 0.00%
 * 12691       27        470                0.999921204                     0.01%
 * 12692       27        470                0.99984242                      0.02%
 * 12693       24        529                1.000236351                     0.02%
 * 12694       24        529                1.000157555                     0.02%
 * 12695       24        529                1.000078771                     0.01%
 * 12696       24        529                1                            ** 0.00%
 * 12697       24        529                0.999921241                     0.01%
 * 12698       24        529                0.999842495                     0.02%
 * 12699       25        508                1.000078746                     0.01%
 * 12700       25        508                1                            ** 0.00%
 * 12701       25        508                0.999921266                     0.01%
 * 12702       25        508                0.999842544                     0.02%
 * 12703       25        508                0.999763835                     0.02%
 * 12704       25        508                0.999685139                     0.03%
 * 12705       25        508                0.999606454                     0.04%
 * 12706       28        454                1.000472218                     0.05%
 * 12707       28        454                1.000393484                     0.04%
 * 12708       28        454                1.000314762                     0.03%
 * 12709       28        454                1.000236053                     0.02%
 * 12710       28        454                1.000157356                     0.02%
 * 12711       28        454                1.000078672                     0.01%
 * 12712       28        454                1                            ** 0.00%
 * 12713       26        489                1.00007866                      0.01%
 * 12714       26        489                1                            ** 0.00%
 * 12715       26        489                0.999921353                     0.01%
 * 12716       27        471                1.000078641                     0.01%
 * 12717       27        471                1                            ** 0.00%
 * 12718       27        471                0.999921371                     0.01%
 * 12719       24        530                1.000078623                     0.01%
 * 12720       24        530                1                            ** 0.00%
 * 12721       24        530                0.99992139                      0.01%
 * 12722       24        530                0.999842792                     0.02%
 * 12723       25        509                1.000157196                     0.02%
 * 12724       25        509                1.000078592                     0.01%
 * 12725       25        509                1                            ** 0.00%
 * 12726       25        509                0.999921421                     0.01%
 * 12727       25        509                0.999842854                     0.02%
 * 12728       25        509                0.999764299                     0.02%
 * 12729       25        509                0.999685757                     0.03%
 * 12730       25        509                0.999607227                     0.04%
 * 12731       25        509                0.999528709                     0.05%
 * 12732       25        509                0.999450204                     0.05%
 * 12733       26        490                1.000549753                     0.05%
 * 12734       26        490                1.00047118                      0.05%
 * 12735       26        490                1.000392619                     0.04%
 * 12736       26        490                1.00031407                      0.03%
 * 12737       26        490                1.000235534                     0.02%
 * 12738       26        490                1.000157011                     0.02%
 * 12739       26        490                1.000078499                     0.01%
 * 12740       26        490                1                            ** 0.00%
 * 12741       26        490                0.999921513                     0.01%
 * 12742       24        531                1.000156961                     0.02%
 * 12743       24        531                1.000078474                     0.01%
 * 12744       24        531                1                            ** 0.00%
 * 12745       24        531                0.999921538                     0.01%
 * 12746       24        531                0.999843088                     0.02%
 * 12747       24        531                0.999764651                     0.02%
 * 12748       25        510                1.000156887                     0.02%
 * 12749       25        510                1.000078438                     0.01%
 * 12750       25        510                1                            ** 0.00%
 * 12751       25        510                0.999921575                     0.01%
 * 12752       25        510                0.999843162                     0.02%
 * 12753       25        510                0.999764761                     0.02%
 * 12754       25        510                0.999686373                     0.03%
 * 12755       25        510                0.999607997                     0.04%
 * 12756       25        510                0.999529633                     0.05%
 * 12757       25        510                0.999451282                     0.05%
 * 12758       25        510                0.999372942                     0.06%
 * 12759       26        491                1.000548632                     0.05%
 * 12760       26        491                1.000470219                     0.05%
 * 12761       26        491                1.000391819                     0.04%
 * 12762       26        491                1.00031343                      0.03%
 * 12763       26        491                1.000235054                     0.02%
 * 12764       26        491                1.000156691                     0.02%
 * 12765       26        491                1.000078339                     0.01%
 * 12766       26        491                1                            ** 0.00%
 * 12767       26        491                0.999921673                     0.01%
 * 12768       24        532                1                            ** 0.00%
 * 12769       24        532                0.999921685                     0.01%
 * 12770       27        473                1.000078309                     0.01%
 * 12771       27        473                1                            ** 0.00%
 * 12772       27        473                0.999921704                     0.01%
 * 12773       25        511                1.00015658                      0.02%
 * 12774       25        511                1.000078284                     0.01%
 * 12775       25        511                1                            ** 0.00%
 * 12776       25        511                0.999921728                     0.01%
 * 12777       25        511                0.999843469                     0.02%
 * 12778       25        511                0.999765221                     0.02%
 * 12779       25        511                0.999686986                     0.03%
 * 12780       25        511                0.999608764                     0.04%
 * 12781       25        511                0.999530553                     0.05%
 * 12782       25        511                0.999452355                     0.05%
 * 12783       25        511                0.999374169                     0.06%
 * 12784       24        533                1.000625782                     0.06%
 * 12785       24        533                1.000547517                     0.05%
 * 12786       24        533                1.000469263                     0.05%
 * 12787       24        533                1.000391022                     0.04%
 * 12788       24        533                1.000312793                     0.03%
 * 12789       24        533                1.000234577                     0.02%
 * 12790       24        533                1.000156372                     0.02%
 * 12791       24        533                1.00007818                      0.01%
 * 12792       24        533                1                            ** 0.00%
 * 12793       24        533                0.999921832                     0.01%
 * 12794       28        457                1.000156323                     0.02%
 * 12795       28        457                1.000078156                     0.01%
 * 12796       28        457                1                            ** 0.00%
 * 12797       27        474                1.000078143                     0.01%
 * 12798       27        474                1                            ** 0.00%
 * 12799       27        474                0.999921869                     0.01%
 * 12800       25        512                1                            ** 0.00%
 * 12801       25        512                0.999921881                     0.01%
 * 12802       25        512                0.999843774                     0.02%
 * 12803       25        512                0.99976568                      0.02%
 * 12804       25        512                0.999687598                     0.03%
 * 12805       25        512                0.999609528                     0.04%
 * 12806       25        512                0.99953147                      0.05%
 * 12807       25        512                0.999453424                     0.05%
 * 12808       24        534                1.00062461                      0.06%
 * 12809       24        534                1.000546491                     0.05%
 * 12810       24        534                1.000468384                     0.05%
 * 12811       24        534                1.00039029                      0.04%
 * 12812       24        534                1.000312207                     0.03%
 * 12813       24        534                1.000234137                     0.02%
 * 12814       24        534                1.000156079                     0.02%
 * 12815       24        534                1.000078034                     0.01%
 * 12816       24        534                1                            ** 0.00%
 * 12817       24        534                0.999921979                     0.01%
 * 12818       26        493                1                            ** 0.00%
 * 12819       26        493                0.999921991                     0.01%
 * 12820       26        493                0.999843994                     0.02%
 * 12821       28        458                1.000233991                     0.02%
 * 12822       28        458                1.000155982                     0.02%
 * 12823       28        458                1.000077985                     0.01%
 * 12824       28        458                1                            ** 0.00%
 * 12825       25        513                1                            ** 0.00%
 * 12826       25        513                0.999922033                     0.01%
 * 12827       25        513                0.999844079                     0.02%
 * 12828       25        513                0.999766137                     0.02%
 * 12829       25        513                0.999688206                     0.03%
 * 12830       25        513                0.999610288                     0.04%
 * 12831       25        513                0.999532383                     0.05%
 * 12832       25        513                0.999454489                     0.05%
 * 12833       24        535                1.000545469                     0.05%
 * 12834       24        535                1.000467508                     0.05%
 * 12835       24        535                1.00038956                      0.04%
 * 12836       24        535                1.000311624                     0.03%
 * 12837       24        535                1.000233699                     0.02%
 * 12838       24        535                1.000155788                     0.02%
 * 12839       24        535                1.000077888                     0.01%
 * 12840       24        535                1                            ** 0.00%
 * 12841       24        535                0.999922124                     0.01%
 * 12842       24        535                0.999844261                     0.02%
 * 12843       26        494                1.000077863                     0.01%
 * 12844       26        494                1                            ** 0.00%
 * 12845       26        494                0.999922149                     0.01%
 * 12846       26        494                0.99984431                      0.02%
 * 12847       26        494                0.999766482                     0.02%
 * 12848       25        514                1.000155666                     0.02%
 * 12849       25        514                1.000077827                     0.01%
 * 12850       25        514                1                            ** 0.00%
 * 12851       25        514                0.999922185                     0.01%
 * 12852       27        476                1                            ** 0.00%
 * 12853       27        476                0.999922197                     0.01%
 * 12854       27        476                0.999844406                     0.02%
 * 12855       27        476                0.999766628                     0.02%
 * 12856       27        476                0.999688861                     0.03%
 * 12857       27        476                0.999611107                     0.04%
 * 12858       24        536                1.000466636                     0.05%
 * 12859       24        536                1.000388833                     0.04%
 * 12860       24        536                1.000311042                     0.03%
 * 12861       24        536                1.000233263                     0.02%
 * 12862       24        536                1.000155497                     0.02%
 * 12863       24        536                1.000077742                     0.01%
 * 12864       24        536                1                            ** 0.00%
 * 12865       24        536                0.99992227                      0.01%
 * 12866       24        536                0.999844552                     0.02%
 * 12867       24        536                0.999766845                     0.02%
 * 12868       26        495                1.000155424                     0.02%
 * 12869       26        495                1.000077706                     0.01%
 * 12870       26        495                1                            ** 0.00%
 * 12871       26        495                0.999922306                     0.01%
 * 12872       26        495                0.999844624                     0.02%
 * 12873       25        515                1.000155364                     0.02%
 * 12874       25        515                1.000077676                     0.01%
 * 12875       25        515                1                            ** 0.00%
 * 12876       25        515                0.999922336                     0.01%
 * 12877       25        515                0.999844684                     0.02%
 * 12878       27        477                1.000077652                     0.01%
 * 12879       27        477                1                            ** 0.00%
 * 12880       28        460                1                            ** 0.00%
 * 12881       28        460                0.999922366                     0.01%
 * 12882       28        460                0.999844745                     0.02%
 * 12883       28        460                0.999767135                     0.02%
 * 12884       24        537                1.000310463                     0.03%
 * 12885       24        537                1.000232829                     0.02%
 * 12886       24        537                1.000155207                     0.02%
 * 12887       24        537                1.000077598                     0.01%
 * 12888       24        537                1                            ** 0.00%
 * 12889       24        537                0.999922414                     0.01%
 * 12890       24        537                0.999844841                     0.02%
 * 12891       24        537                0.999767279                     0.02%
 * 12892       24        537                0.99968973                      0.03%
 * 12893       26        496                1.000232684                     0.02%
 * 12894       26        496                1.000155111                     0.02%
 * 12895       26        496                1.000077549                     0.01%
 * 12896       26        496                1                            ** 0.00%
 * 12897       26        496                0.999922463                     0.01%
 * 12898       25        516                1.000155063                     0.02%
 * 12899       25        516                1.000077525                     0.01%
 * 12900       25        516                1                            ** 0.00%
 * 12901       25        516                0.999922487                     0.01%
 * 12902       25        516                0.999844985                     0.02%
 * 12903       27        478                1.000232504                     0.02%
 * 12904       27        478                1.000154991                     0.02%
 * 12905       27        478                1.000077489                     0.01%
 * 12906       27        478                1                            ** 0.00%
 * 12907       27        478                0.999922523                     0.01%
 * 12908       28        461                1                            ** 0.00%
 * 12909       28        461                0.999922535                     0.01%
 * 12910       24        538                1.000154919                     0.02%
 * 12911       24        538                1.000077453                     0.01%
 * 12912       24        538                1                            ** 0.00%
 * 12913       24        538                0.999922559                     0.01%
 * 12914       24        538                0.999845129                     0.02%
 * 12915       24        538                0.999767712                     0.02%
 * 12916       24        538                0.999690307                     0.03%
 * 12917       24        538                0.999612913                     0.04%
 * 12918       26        497                1.000309645                     0.03%
 * 12919       26        497                1.000232216                     0.02%
 * 12920       26        497                1.000154799                     0.02%
 * 12921       26        497                1.000077393                     0.01%
 * 12922       26        497                1                            ** 0.00%
 * 12923       26        497                0.999922619                     0.01%
 * 12924       25        517                1.000077375                     0.01%
 * 12925       25        517                1                            ** 0.00%
 * 12926       25        517                0.999922637                     0.01%
 * 12927       25        517                0.999845285                     0.02%
 * 12928       25        517                0.999767946                     0.02%
 * 12929       25        517                0.999690618                     0.03%
 * 12930       27        479                1.000232019                     0.02%
 * 12931       27        479                1.000154667                     0.02%
 * 12932       27        479                1.000077328                     0.01%
 * 12933       27        479                1                            ** 0.00%
 * 12934       27        479                0.999922684                     0.01%
 * 12935       24        539                1.00007731                      0.01%
 * 12936       24        539                1                            ** 0.00%
 * 12937       24        539                0.999922702                     0.01%
 * 12938       24        539                0.999845417                     0.02%
 * 12939       24        539                0.999768143                     0.02%
 * 12940       24        539                0.999690881                     0.03%
 * 12941       24        539                0.999613631                     0.04%
 * 12942       26        498                1.000463607                     0.05%
 * 12943       26        498                1.000386309                     0.04%
 * 12944       26        498                1.000309023                     0.03%
 * 12945       26        498                1.00023175                      0.02%
 * 12946       26        498                1.000154488                     0.02%
 * 12947       26        498                1.000077238                     0.01%
 * 12948       26        498                1                            ** 0.00%
 * 12949       26        498                0.999922774                     0.01%
 * 12950       25        518                1                            ** 0.00%
 * 12951       25        518                0.999922786                     0.01%
 * 12952       25        518                0.999845584                     0.02%
 * 12953       25        518                0.999768393                     0.02%
 * 12954       25        518                0.999691215                     0.03%
 * 12955       25        518                0.999614049                     0.04%
 * 12956       24        540                1.000308737                     0.03%
 * 12957       24        540                1.000231535                     0.02%
 * 12958       24        540                1.000154345                     0.02%
 * 12959       24        540                1.000077166                     0.01%
 * 12960       24        540                1                            ** 0.00%
 * 12961       24        540                0.999922845                     0.01%
 * 12962       24        540                0.999845703                     0.02%
 * 12963       28        463                1.000077143                     0.01%
 * 12964       28        463                1                            ** 0.00%
 * 12965       28        463                0.999922869                     0.01%
 * 12966       28        463                0.99984575                      0.02%
 * 12967       28        463                0.999768643                     0.02%
 * 12968       28        463                0.999691548                     0.03%
 * 12969       26        499                1.000385535                     0.04%
 * 12970       26        499                1.000308404                     0.03%
 * 12971       26        499                1.000231285                     0.02%
 * 12972       26        499                1.000154178                     0.02%
 * 12973       26        499                1.000077083                     0.01%
 * 12974       26        499                1                            ** 0.00%
 * 12975       25        519                1                            ** 0.00%
 * 12976       25        519                0.999922935                     0.01%
 * 12977       25        519                0.999845881                     0.02%
 * 12978       25        519                0.99976884                      0.02%
 * 12979       25        519                0.99969181                      0.03%
 * 12980       24        541                1.000308166                     0.03%
 * 12981       24        541                1.000231107                     0.02%
 * 12982       24        541                1.000154059                     0.02%
 * 12983       24        541                1.000077024                     0.01%
 * 12984       24        541                1                            ** 0.00%
 * 12985       24        541                0.999922988                     0.01%
 * 12986       27        481                1.000077006                     0.01%
 * 12987       27        481                1                            ** 0.00%
 * 12988       27        481                0.999923006                     0.01%
 * 12989       27        481                0.999846024                     0.02%
 * 12990       28        464                1.000153965                     0.02%
 * 12991       28        464                1.000076976                     0.01%
 * 12992       28        464                1                            ** 0.00%
 * 12993       28        464                0.999923035                     0.01%
 * 12994       28        464                0.999846083                     0.02%
 * 12995       28        464                0.999769142                     0.02%
 * 12996       25        520                1.000307787                     0.03%
 * 12997       25        520                1.000230822                     0.02%
 * 12998       25        520                1.00015387                      0.02%
 * 12999       25        520                1.000076929                     0.01%
 * 13000       25        520                1                            ** 0.00%
 * 13001       25        520                0.999923083                     0.01%
 * 13002       25        520                0.999846178                     0.02%
 * 13003       25        520                0.999769284                     0.02%
 * 13004       24        542                1.000307598                     0.03%
 * 13005       24        542                1.000230681                     0.02%
 * 13006       24        542                1.000153775                     0.02%
 * 13007       24        542                1.000076882                     0.01%
 * 13008       24        542                1                            ** 0.00%
 * 13009       24        542                0.99992313                      0.01%
 * 13010       24        542                0.999846272                     0.02%
 * 13011       24        542                0.999769426                     0.02%
 * 13012       27        482                1.000153704                     0.02%
 * 13013       27        482                1.000076846                     0.01%
 * 13014       27        482                1                            ** 0.00%
 * 13015       27        482                0.999923166                     0.01%
 * 13016       27        482                0.999846343                     0.02%
 * 13017       28        465                1.000230468                     0.02%
 * 13018       28        465                1.000153633                     0.02%
 * 13019       28        465                1.000076811                     0.01%
 * 13020       28        465                1                            ** 0.00%
 * 13021       28        465                0.999923201                     0.01%
 * 13022       28        465                0.999846414                     0.02%
 * 13023       25        521                1.000153574                     0.02%
 * 13024       25        521                1.000076781                     0.01%
 * 13025       25        521                1                            ** 0.00%
 * 13026       26        501                1                            ** 0.00%
 * 13027       26        501                0.999923236                     0.01%
 * 13028       26        501                0.999846484                     0.02%
 * 13029       24        543                1.000230256                     0.02%
 * 13030       24        543                1.000153492                     0.02%
 * 13031       24        543                1.00007674                      0.01%
 * 13032       24        543                1                            ** 0.00%
 * 13033       24        543                0.999923272                     0.01%
 * 13034       24        543                0.999846555                     0.02%
 * 13035       24        543                0.99976985                      0.02%
 * 13036       24        543                0.999693157                     0.03%
 * 13037       27        483                1.000306819                     0.03%
 * 13038       27        483                1.000230097                     0.02%
 * 13039       27        483                1.000153386                     0.02%
 * 13040       27        483                1.000076687                     0.01%
 * 13041       27        483                1                            ** 0.00%
 * 13042       27        483                0.999923325                     0.01%
 * 13043       27        483                0.999846661                     0.02%
 * 13044       27        483                0.999770009                     0.02%
 * 13045       28        466                1.000229973                     0.02%
 * 13046       28        466                1.000153304                     0.02%
 * 13047       28        466                1.000076646                     0.01%
 * 13048       28        466                1                            ** 0.00%
 * 13049       25        522                1.000076634                     0.01%
 * 13050       25        522                1                            ** 0.00%
 * 13051       25        522                0.999923378                     0.01%
 * 13052       26        502                1                            ** 0.00%
 * 13053       26        502                0.999923389                     0.01%
 * 13054       24        544                1.00015321                      0.02%
 * 13055       24        544                1.000076599                     0.01%
 * 13056       24        544                1                            ** 0.00%
 * 13057       24        544                0.999923413                     0.01%
 * 13058       24        544                0.999846837                     0.02%
 * 13059       24        544                0.999770273                     0.02%
 * 13060       24        544                0.999693721                     0.03%
 * 13061       24        544                0.999617181                     0.04%
 * 13062       27        484                1.000459348                     0.05%
 * 13063       27        484                1.00038276                      0.04%
 * 13064       27        484                1.000306185                     0.03%
 * 13065       27        484                1.000229621                     0.02%
 * 13066       27        484                1.000153069                     0.02%
 * 13067       27        484                1.000076529                     0.01%
 * 13068       27        484                1                            ** 0.00%
 * 13069       27        484                0.999923483                     0.01%
 * 13070       27        484                0.999846978                     0.02%
 * 13071       27        484                0.999770484                     0.02%
 * 13072       25        523                1.000229498                     0.02%
 * 13073       25        523                1.000152987                     0.02%
 * 13074       25        523                1.000076488                     0.01%
 * 13075       25        523                1                            ** 0.00%
 * 13076       28        467                1                            ** 0.00%
 * 13077       28        467                0.99992353                      0.01%
 * 13078       26        503                1                            ** 0.00%
 * 13079       29        451                1                            ** 0.00%
 * 13080       24        545                1                            ** 0.00%
 * 13081       24        545                0.999923553                     0.01%
 * 13082       24        545                0.999847118                     0.02%
 * 13083       24        545                0.999770695                     0.02%
 * 13084       24        545                0.999694283                     0.03%
 * 13085       24        545                0.999617883                     0.04%
 * 13086       24        545                0.999541495                     0.05%
 * 13087       24        545                0.999465118                     0.05%
 * 13088       27        485                1.000534841                     0.05%
 * 13089       27        485                1.0004584                       0.05%
 * 13090       27        485                1.000381971                     0.04%
 * 13091       27        485                1.000305553                     0.03%
 * 13092       27        485                1.000229148                     0.02%
 * 13093       27        485                1.000152753                     0.02%
 * 13094       27        485                1.000076371                     0.01%
 * 13095       27        485                1                            ** 0.00%
 * 13096       27        485                0.999923641                     0.01%
 * 13097       27        485                0.999847293                     0.02%
 * 13098       25        524                1.000152695                     0.02%
 * 13099       25        524                1.000076342                     0.01%
 * 13100       25        524                1                            ** 0.00%
 * 13101       25        524                0.99992367                      0.01%
 * 13102       24        546                1.000152648                     0.02%
 * 13103       24        546                1.000076318                     0.01%
 * 13104       24        546                1                            ** 0.00%
 * 13105       24        546                0.999923693                     0.01%
 * 13106       24        546                0.999847398                     0.02%
 * 13107       29        452                1.000076295                     0.01%
 * 13108       29        452                1                            ** 0.00%
 * 13109       29        452                0.999923717                     0.01%
 * 13110       29        452                0.999847445                     0.02%
 * 13111       29        452                0.999771185                     0.02%
 * 13112       29        452                0.999694936                     0.03%
 * 13113       29        452                0.999618699                     0.04%
 * 13114       29        452                0.999542474                     0.05%
 * 13115       27        486                1.00053374                      0.05%
 * 13116       27        486                1.000457457                     0.05%
 * 13117       27        486                1.000381185                     0.04%
 * 13118       27        486                1.000304925                     0.03%
 * 13119       27        486                1.000228676                     0.02%
 * 13120       27        486                1.000152439                     0.02%
 * 13121       27        486                1.000076214                     0.01%
 * 13122       27        486                1                            ** 0.00%
 * 13123       27        486                0.999923798                     0.01%
 * 13124       25        525                1.000076196                     0.01%
 * 13125       25        525                1                            ** 0.00%
 * 13126       25        525                0.999923815                     0.01%
 * 13127       24        547                1.000076179                     0.01%
 * 13128       24        547                1                            ** 0.00%
 * 13129       24        547                0.999923833                     0.01%
 * 13130       26        505                1                            ** 0.00%
 * 13131       26        505                0.999923844                     0.01%
 * 13132       28        469                1                            ** 0.00%
 * 13133       28        469                0.999923856                     0.01%
 * 13134       28        469                0.999847723                     0.02%
 * 13135       29        453                1.000152265                     0.02%
 * 13136       29        453                1.000076127                     0.01%
 * 13137       29        453                1                            ** 0.00%
 * 13138       29        453                0.999923885                     0.01%
 * 13139       29        453                0.999847781                     0.02%
 * 13140       29        453                0.999771689                     0.02%
 * 13141       29        453                0.999695609                     0.03%
 * 13142       29        453                0.99961954                      0.04%
 * 13143       29        453                0.999543483                     0.05%
 * 13144       27        487                1.000380402                     0.04%
 * 13145       27        487                1.000304298                     0.03%
 * 13146       27        487                1.000228206                     0.02%
 * 13147       27        487                1.000152126                     0.02%
 * 13148       27        487                1.000076057                     0.01%
 * 13149       27        487                1                            ** 0.00%
 * 13150       25        526                1                            ** 0.00%
 * 13151       24        548                1.00007604                      0.01%
 * 13152       24        548                1                            ** 0.00%
 * 13153       24        548                0.999923972                     0.01%
 * 13154       24        548                0.999847955                     0.02%
 * 13155       26        506                1.000076017                     0.01%
 * 13156       26        506                1                            ** 0.00%
 * 13157       26        506                0.999923995                     0.01%
 * 13158       26        506                0.999848001                     0.02%
 * 13159       28        470                1.000075994                     0.01%
 * 13160       28        470                1                            ** 0.00%
 * 13161       28        470                0.999924018                     0.01%
 * 13162       28        470                0.999848047                     0.02%
 * 13163       28        470                0.999772088                     0.02%
 * 13164       29        454                1.00015193                      0.02%
 * 13165       29        454                1.000075959                     0.01%
 * 13166       29        454                1                            ** 0.00%
 * 13167       29        454                0.999924053                     0.01%
 * 13168       29        454                0.999848117                     0.02%
 * 13169       29        454                0.999772192                     0.02%
 * 13170       29        454                0.999696279                     0.03%
 * 13171       25        527                1.000303698                     0.03%
 * 13172       25        527                1.000227756                     0.02%
 * 13173       25        527                1.000151826                     0.02%
 * 13174       25        527                1.000075907                     0.01%
 * 13175       25        527                1                            ** 0.00%
 * 13176       24        549                1                            ** 0.00%
 * 13177       24        549                0.99992411                      0.01%
 * 13178       24        549                0.999848232                     0.02%
 * 13179       24        549                0.999772365                     0.02%
 * 13180       26        507                1.000151745                     0.02%
 * 13181       26        507                1.000075867                     0.01%
 * 13182       26        507                1                            ** 0.00%
 * 13183       26        507                0.999924145                     0.01%
 * 13184       26        507                0.999848301                     0.02%
 * 13185       26        507                0.999772469                     0.02%
 * 13186       28        471                1.000151676                     0.02%
 * 13187       28        471                1.000075832                     0.01%
 * 13188       28        471                1                            ** 0.00%
 * 13189       28        471                0.999924179                     0.01%
 * 13190       28        471                0.99984837                      0.02%
 * 13191       28        471                0.999772572                     0.02%
 * 13192       29        455                1.000227411                     0.02%
 * 13193       29        455                1.000151596                     0.02%
 * 13194       29        455                1.000075792                     0.01%
 * 13195       29        455                1                            ** 0.00%
 * 13196       29        455                0.999924219                     0.01%
 * 13197       29        455                0.99984845                      0.02%
 * 13198       24        550                1.000151538                     0.02%
 * 13199       24        550                1.000075763                     0.01%
 * 13200       24        550                1                            ** 0.00%
 * 13201       24        550                0.999924248                     0.01%
 * 13202       27        489                1.000075746                     0.01%
 * 13203       27        489                1                            ** 0.00%
 * 13204       27        489                0.999924265                     0.01%
 * 13205       27        489                0.999848542                     0.02%
 * 13206       26        508                1.000151446                     0.02%
 * 13207       26        508                1.000075717                     0.01%
 * 13208       26        508                1                            ** 0.00%
 * 13209       26        508                0.999924294                     0.01%
 * 13210       26        508                0.9998486                       0.02%
 * 13211       26        508                0.999772917                     0.02%
 * 13212       26        508                0.999697245                     0.03%
 * 13213       28        472                1.000227049                     0.02%
 * 13214       28        472                1.000151355                     0.02%
 * 13215       28        472                1.000075672                     0.01%
 * 13216       28        472                1                            ** 0.00%
 * 13217       28        472                0.99992434                      0.01%
 * 13218       28        472                0.999848691                     0.02%
 * 13219       28        472                0.999773054                     0.02%
 * 13220       28        472                0.999697428                     0.03%
 * 13221       29        456                1.000226912                     0.02%
 * 13222       29        456                1.000151263                     0.02%
 * 13223       29        456                1.000075626                     0.01%
 * 13224       29        456                1                            ** 0.00%
 * 13225       25        529                1                            ** 0.00%
 * 13226       25        529                0.999924391                     0.01%
 * 13227       25        529                0.999848794                     0.02%
 * 13228       27        490                1.000151194                     0.02%
 * 13229       27        490                1.000075592                     0.01%
 * 13230       27        490                1                            ** 0.00%
 * 13231       27        490                0.99992442                      0.01%
 * 13232       26        509                1.000151149                     0.02%
 * 13233       26        509                1.000075569                     0.01%
 * 13234       26        509                1                            ** 0.00%
 * 13235       26        509                0.999924443                     0.01%
 * 13236       26        509                0.999848897                     0.02%
 * 13237       26        509                0.999773363                     0.02%
 * 13238       26        509                0.99969784                      0.03%
 * 13239       26        509                0.999622328                     0.04%
 * 13240       28        473                1.000302115                     0.03%
 * 13241       28        473                1.000226569                     0.02%
 * 13242       28        473                1.000151035                     0.02%
 * 13243       28        473                1.000075512                     0.01%
 * 13244       28        473                1                            ** 0.00%
 * 13245       28        473                0.9999245                       0.01%
 * 13246       28        473                0.999849011                     0.02%
 * 13247       25        530                1.000226466                     0.02%
 * 13248       25        530                1.000150966                     0.02%
 * 13249       25        530                1.000075477                     0.01%
 * 13250       25        530                1                            ** 0.00%
 * 13251       25        530                0.999924534                     0.01%
 * 13252       29        457                1.00007546                      0.01%
 * 13253       29        457                1                            ** 0.00%
 * 13254       29        457                0.999924551                     0.01%
 * 13255       29        457                0.999849114                     0.02%
 * 13256       27        491                1.000075438                     0.01%
 * 13257       27        491                1                            ** 0.00%
 * 13258       27        491                0.999924574                     0.01%
 * 13259       26        510                1.00007542                      0.01%
 * 13260       26        510                1                            ** 0.00%
 * 13261       26        510                0.999924591                     0.01%
 * 13262       26        510                0.999849193                     0.02%
 * 13263       26        510                0.999773807                     0.02%
 * 13264       26        510                0.999698432                     0.03%
 * 13265       26        510                0.999623068                     0.04%
 * 13266       26        510                0.999547716                     0.05%
 * 13267       28        474                1.000376875                     0.04%
 * 13268       28        474                1.000301477                     0.03%
 * 13269       28        474                1.000226091                     0.02%
 * 13270       28        474                1.000150716                     0.02%
 * 13271       28        474                1.000075352                     0.01%
 * 13272       28        474                1                            ** 0.00%
 * 13273       28        474                0.999924659                     0.01%
 * 13274       25        531                1.000075335                     0.01%
 * 13275       25        531                1                            ** 0.00%
 * 13276       25        531                0.999924676                     0.01%
 * 13277       25        531                0.999849364                     0.02%
 * 13278       25        531                0.999774062                     0.02%
 * 13279       29        458                1.000225921                     0.02%
 * 13280       29        458                1.000150602                     0.02%
 * 13281       29        458                1.000075296                     0.01%
 * 13282       29        458                1                            ** 0.00%
 * 13283       27        492                1.000075284                     0.01%
 * 13284       27        492                1                            ** 0.00%
 * 13285       26        511                1.000075273                     0.01%
 * 13286       26        511                1                            ** 0.00%
 * 13287       26        511                0.999924738                     0.01%
 * 13288       26        511                0.999849488                     0.02%
 * 13289       26        511                0.999774249                     0.02%
 * 13290       26        511                0.999699022                     0.03%
 * 13291       26        511                0.999623806                     0.04%
 * 13292       26        511                0.999548601                     0.05%
 * 13293       26        511                0.999473407                     0.05%
 * 13294       25        532                1.000451331                     0.05%
 * 13295       25        532                1.000376081                     0.04%
 * 13296       25        532                1.000300842                     0.03%
 * 13297       25        532                1.000225615                     0.02%
 * 13298       25        532                1.000150399                     0.02%
 * 13299       25        532                1.000075194                     0.01%
 * 13300       25        532                1                            ** 0.00%
 * 13301       25        532                0.999924818                     0.01%
 * 13302       25        532                0.999849647                     0.02%
 * 13303       25        532                0.999774487                     0.02%
 * 13304       25        532                0.999699339                     0.03%
 * 13305       25        532                0.999624201                     0.04%
 * 13306       27        493                1.00037577                      0.04%
 * 13307       27        493                1.000300594                     0.03%
 * 13308       27        493                1.000225428                     0.02%
 * 13309       27        493                1.000150274                     0.02%
 * 13310       27        493                1.000075131                     0.01%
 * 13311       27        493                1                            ** 0.00%
 * 13312       26        512                1                            ** 0.00%
 * 13313       26        512                0.999924885                     0.01%
 * 13314       26        512                0.999849782                     0.02%
 * 13315       26        512                0.99977469                      0.02%
 * 13316       26        512                0.999699609                     0.03%
 * 13317       26        512                0.99962454                      0.04%
 * 13318       26        512                0.999549482                     0.05%
 * 13319       25        533                1.000450484                     0.05%
 * 13320       25        533                1.000375375                     0.04%
 * 13321       25        533                1.000300278                     0.03%
 * 13322       25        533                1.000225191                     0.02%
 * 13323       25        533                1.000150116                     0.02%
 * 13324       25        533                1.000075053                     0.01%
 * 13325       25        533                1                            ** 0.00%
 * 13326       25        533                0.999924959                     0.01%
 * 13327       28        476                1.000075036                     0.01%
 * 13328       28        476                1                            ** 0.00%
 * 13329       28        476                0.999924976                     0.01%
 * 13330       28        476                0.999849962                     0.02%
 * 13331       28        476                0.999774961                     0.02%
 * 13332       28        476                0.99969997                      0.03%
 * 13333       26        513                1.000375009                     0.04%
 * 13334       26        513                1.000299985                     0.03%
 * 13335       26        513                1.000224972                     0.02%
 * 13336       26        513                1.00014997                      0.01%
 * 13337       26        513                1.000074979                     0.01%
 * 13338       26        513                1                            ** 0.00%
 * 13339       29        460                1.000074968                     0.01%
 * 13340       29        460                1                            ** 0.00%
 * 13341       29        460                0.999925043                     0.01%
 * 13342       29        460                0.999850097                     0.01%
 * 13343       29        460                0.999775163                     0.02%
 * 13344       29        460                0.99970024                      0.03%
 * 13345       25        534                1.000374672                     0.04%
 * 13346       25        534                1.000299715                     0.03%
 * 13347       25        534                1.00022477                      0.02%
 * 13348       25        534                1.000149835                     0.01%
 * 13349       25        534                1.000074912                     0.01%
 * 13350       25        534                1                            ** 0.00%
 * 13351       25        534                0.999925099                     0.01%
 * 13352       25        534                0.99985021                      0.01%
 * 13353       28        477                1.000224669                     0.02%
 * 13354       28        477                1.000149768                     0.01%
 * 13355       28        477                1.000074878                     0.01%
 * 13356       28        477                1                            ** 0.00%
 * 13357       28        477                0.999925133                     0.01%
 * 13358       28        477                0.999850277                     0.01%
 * 13359       28        477                0.999775432                     0.02%
 * 13360       26        514                1.000299401                     0.03%
 * 13361       26        514                1.000224534                     0.02%
 * 13362       26        514                1.000149678                     0.01%
 * 13363       26        514                1.000074833                     0.01%
 * 13364       26        514                1                            ** 0.00%
 * 13365       27        495                1                            ** 0.00%
 * 13366       27        495                0.999925183                     0.01%
 * 13367       29        461                1.000149622                     0.01%
 * 13368       29        461                1.000074806                     0.01%
 * 13369       29        461                1                            ** 0.00%
 * 13370       29        461                0.999925206                     0.01%
 * 13371       29        461                0.999850423                     0.01%
 * 13372       29        461                0.999775651                     0.02%
 * 13373       25        535                1.000149555                     0.01%
 * 13374       25        535                1.000074772                     0.01%
 * 13375       25        535                1                            ** 0.00%
 * 13376       25        535                0.999925239                     0.01%
 * 13377       25        535                0.99985049                      0.01%
 * 13378       25        535                0.999775751                     0.02%
 * 13379       25        535                0.999701024                     0.03%
 * 13380       28        478                1.000298954                     0.03%
 * 13381       28        478                1.000224198                     0.02%
 * 13382       28        478                1.000149454                     0.01%
 * 13383       28        478                1.000074722                     0.01%
 * 13384       28        478                1                            ** 0.00%
 * 13385       28        478                0.99992529                      0.01%
 * 13386       28        478                0.99985059                      0.01%
 * 13387       26        515                1.000224098                     0.02%
 * 13388       26        515                1.000149388                     0.01%
 * 13389       26        515                1.000074688                     0.01%
 * 13390       26        515                1                            ** 0.00%
 * 13391       26        515                0.999925323                     0.01%
 * 13392       27        496                1                            ** 0.00%
 * 13393       27        496                0.999925334                     0.01%
 * 13394       27        496                0.999850679                     0.01%
 * 13395       27        496                0.999776036                     0.02%
 * 13396       29        462                1.000149298                     0.01%
 * 13397       29        462                1.000074644                     0.01%
 * 13398       29        462                1                            ** 0.00%
 * 13399       25        536                1.000074632                     0.01%
 * 13400       25        536                1                            ** 0.00%
 * 13401       25        536                0.999925379                     0.01%
 * 13402       25        536                0.999850769                     0.01%
 * 13403       25        536                0.99977617                      0.02%
 * 13404       25        536                0.999701582                     0.03%
 * 13405       25        536                0.999627005                     0.04%
 * 13406       25        536                0.999552439                     0.04%
 * 13407       28        479                1.00037294                      0.04%
 * 13408       28        479                1.000298329                     0.03%
 * 13409       28        479                1.00022373                      0.02%
 * 13410       28        479                1.000149142                     0.01%
 * 13411       28        479                1.000074566                     0.01%
 * 13412       28        479                1                            ** 0.00%
 * 13413       28        479                0.999925445                     0.01%
 * 13414       26        516                1.000149098                     0.01%
 * 13415       26        516                1.000074543                     0.01%
 * 13416       26        516                1                            ** 0.00%
 * 13417       26        516                0.999925468                     0.01%
 * 13418       27        497                1.000074527                     0.01%
 * 13419       27        497                1                            ** 0.00%
 * 13420       27        497                0.999925484                     0.01%
 * 13421       27        497                0.99985098                      0.01%
 * 13422       25        537                1.000223514                     0.02%
 * 13423       25        537                1.000148998                     0.01%
 * 13424       25        537                1.000074493                     0.01%
 * 13425       25        537                1                            ** 0.00%
 * 13426       25        537                0.999925518                     0.01%
 * 13427       29        463                1                            ** 0.00%
 * 13428       29        463                0.999925529                     0.01%
 * 13429       29        463                0.999851069                     0.01%
 * 13430       29        463                0.99977662                      0.02%
 * 13431       29        463                0.999702182                     0.03%
 * 13432       29        463                0.999627755                     0.04%
 * 13433       29        463                0.999553339                     0.04%
 * 13434       28        480                1.000446628                     0.04%
 * 13435       28        480                1.000372162                     0.04%
 * 13436       28        480                1.000297708                     0.03%
 * 13437       28        480                1.000223264                     0.02%
 * 13438       28        480                1.000148832                     0.01%
 * 13439       28        480                1.00007441                      0.01%
 * 13440       28        480                1                            ** 0.00%
 * 13441       26        517                1.000074399                     0.01%
 * 13442       26        517                1                            ** 0.00%
 * 13443       26        517                0.999925612                     0.01%
 * 13444       26        517                0.999851235                     0.01%
 * 13445       27        498                1.000074377                     0.01%
 * 13446       27        498                1                            ** 0.00%
 * 13447       27        498                0.999925634                     0.01%
 * 13448       25        538                1.000148721                     0.01%
 * 13449       25        538                1.000074355                     0.01%
 * 13450       25        538                1                            ** 0.00%
 * 13451       25        538                0.999925656                     0.01%
 * 13452       25        538                0.999851323                     0.01%
 * 13453       25        538                0.999777001                     0.02%
 * 13454       29        464                1.000148655                     0.01%
 * 13455       29        464                1.000074322                     0.01%
 * 13456       29        464                1                            ** 0.00%
 * 13457       29        464                0.999925689                     0.01%
 * 13458       29        464                0.99985139                      0.01%
 * 13459       29        464                0.999777101                     0.02%
 * 13460       29        464                0.999702823                     0.03%
 * 13461       29        464                0.999628557                     0.04%
 * 13462       26        518                1.000445699                     0.04%
 * 13463       26        518                1.000371388                     0.04%
 * 13464       26        518                1.000297089                     0.03%
 * 13465       26        518                1.0002228                       0.02%
 * 13466       26        518                1.000148522                     0.01%
 * 13467       26        518                1.000074256                     0.01%
 * 13468       26        518                1                            ** 0.00%
 * 13469       26        518                0.999925755                     0.01%
 * 13470       26        518                0.999851522                     0.01%
 * 13471       27        499                1.000148467                     0.01%
 * 13472       27        499                1.000074228                     0.01%
 * 13473       27        499                1                            ** 0.00%
 * 13474       25        539                1.000074217                     0.01%
 * 13475       25        539                1                            ** 0.00%
 * 13476       25        539                0.999925794                     0.01%
 * 13477       25        539                0.999851599                     0.01%
 * 13478       25        539                0.999777415                     0.02%
 * 13479       25        539                0.999703242                     0.03%
 * 13480       29        465                1.00037092                      0.04%
 * 13481       29        465                1.000296714                     0.03%
 * 13482       29        465                1.000222519                     0.02%
 * 13483       29        465                1.000148335                     0.01%
 * 13484       29        465                1.000074162                     0.01%
 * 13485       29        465                1                            ** 0.00%
 * 13486       29        465                0.999925849                     0.01%
 * 13487       29        465                0.999851709                     0.01%
 * 13488       29        465                0.99977758                      0.02%
 * 13489       29        465                0.999703462                     0.03%
 * 13490       26        519                1.000296516                     0.03%
 * 13491       26        519                1.00022237                      0.02%
 * 13492       26        519                1.000148236                     0.01%
 * 13493       26        519                1.000074113                     0.01%
 * 13494       26        519                1                            ** 0.00%
 * 13495       28        482                1.000074102                     0.01%
 * 13496       28        482                1                            ** 0.00%
 * 13497       28        482                0.999925909                     0.01%
 * 13498       28        482                0.99985183                      0.01%
 * 13499       25        540                1.00007408                      0.01%
 * 13500       25        540                1                            ** 0.00%
 * 13501       25        540                0.999925931                     0.01%
 * 13502       25        540                0.999851874                     0.01%
 * 13503       25        540                0.999777827                     0.02%
 * 13504       25        540                0.999703791                     0.03%
 * 13505       25        540                0.999629767                     0.04%
 * 13506       25        540                0.999555753                     0.04%
 * 13507       29        466                1.00051825                      0.05%
 * 13508       29        466                1.000444181                     0.04%
 * 13509       29        466                1.000370124                     0.04%
 * 13510       29        466                1.000296077                     0.03%
 * 13511       29        466                1.000222041                     0.02%
 * 13512       29        466                1.000148017                     0.01%
 * 13513       29        466                1.000074003                     0.01%
 * 13514       29        466                1                            ** 0.00%
 * 13515       29        466                0.999926008                     0.01%
 * 13516       29        466                0.999852027                     0.01%
 * 13517       26        520                1.000221943                     0.02%
 * 13518       26        520                1.000147951                     0.01%
 * 13519       26        520                1.00007397                      0.01%
 * 13520       26        520                1                            ** 0.00%
 * 13521       26        520                0.999926041                     0.01%
 * 13522       26        520                0.999852093                     0.01%
 * 13523       28        483                1.000073948                     0.01%
 * 13524       28        483                1                            ** 0.00%
 * 13525       25        541                1                            ** 0.00%
 * 13526       27        501                1.000073932                     0.01%
 * 13527       27        501                1                            ** 0.00%
 * 13528       27        501                0.999926079                     0.01%
 * 13529       27        501                0.999852169                     0.01%
 * 13530       27        501                0.999778271                     0.02%
 * 13531       27        501                0.999704383                     0.03%
 * 13532       27        501                0.999630505                     0.04%
 * 13533       27        501                0.999556639                     0.04%
 * 13534       27        501                0.999482784                     0.05%
 * 13535       27        501                0.99940894                      0.06%
 * 13536       29        467                1.000517139                     0.05%
 * 13537       29        467                1.00044323                      0.04%
 * 13538       29        467                1.000369331                     0.04%
 * 13539       29        467                1.000295443                     0.03%
 * 13540       29        467                1.000221566                     0.02%
 * 13541       29        467                1.0001477                       0.01%
 * 13542       29        467                1.000073844                     0.01%
 * 13543       29        467                1                            ** 0.00%
 * 13544       29        467                0.999926167                     0.01%
 * 13545       26        521                1.000073828                     0.01%
 * 13546       26        521                1                            ** 0.00%
 * 13547       26        521                0.999926183                     0.01%
 * 13548       26        521                0.999852377                     0.01%
 * 13549       25        542                1.000073806                     0.01%
 * 13550       25        542                1                            ** 0.00%
 * 13551       25        542                0.999926205                     0.01%
 * 13552       28        484                1                            ** 0.00%
 * 13553       27        502                1.000073784                     0.01%
 * 13554       27        502                1                            ** 0.00%
 * 13555       27        502                0.999926226                     0.01%
 * 13556       27        502                0.999852464                     0.01%
 * 13557       27        502                0.999778712                     0.02%
 * 13558       27        502                0.999704971                     0.03%
 * 13559       27        502                0.999631241                     0.04%
 * 13560       27        502                0.999557522                     0.04%
 * 13561       27        502                0.999483814                     0.05%
 * 13562       27        502                0.999410117                     0.06%
 * 13563       26        522                1.00066357                      0.07%
 * 13564       26        522                1.000589797                     0.06%
 * 13565       26        522                1.000516034                     0.05%
 * 13566       26        522                1.000442282                     0.04%
 * 13567       26        522                1.000368541                     0.04%
 * 13568       26        522                1.000294811                     0.03%
 * 13569       26        522                1.000221092                     0.02%
 * 13570       26        522                1.000147384                     0.01%
 * 13571       26        522                1.000073687                     0.01%
 * 13572       26        522                1                            ** 0.00%
 * 13573       26        522                0.999926324                     0.01%
 * 13574       25        543                1.00007367                      0.01%
 * 13575       25        543                1                            ** 0.00%
 * 13576       25        543                0.999926341                     0.01%
 * 13577       25        543                0.999852692                     0.01%
 * 13578       28        485                1.000147297                     0.01%
 * 13579       28        485                1.000073643                     0.01%
 * 13580       28        485                1                            ** 0.00%
 * 13581       27        503                1                            ** 0.00%
 * 13582       27        503                0.999926373                     0.01%
 * 13583       27        503                0.999852757                     0.01%
 * 13584       27        503                0.999779152                     0.02%
 * 13585       27        503                0.999705558                     0.03%
 * 13586       27        503                0.999631974                     0.04%
 * 13587       27        503                0.999558401                     0.04%
 * 13588       27        503                0.99948484                      0.05%
 * 13589       27        503                0.999411289                     0.06%
 * 13590       26        523                1.000588668                     0.06%
 * 13591       26        523                1.000515047                     0.05%
 * 13592       26        523                1.000441436                     0.04%
 * 13593       26        523                1.000367836                     0.04%
 * 13594       26        523                1.000294247                     0.03%
 * 13595       26        523                1.000220669                     0.02%
 * 13596       26        523                1.000147102                     0.01%
 * 13597       26        523                1.000073546                     0.01%
 * 13598       26        523                1                            ** 0.00%
 * 13599       25        544                1.000073535                     0.01%
 * 13600       25        544                1                            ** 0.00%
 * 13601       29        469                1                            ** 0.00%
 * 13602       29        469                0.999926481                     0.01%
 * 13603       29        469                0.999852974                     0.01%
 * 13604       29        469                0.999779477                     0.02%
 * 13605       27        504                1.000220507                     0.02%
 * 13606       27        504                1.000146994                     0.01%
 * 13607       27        504                1.000073492                     0.01%
 * 13608       27        504                1                            ** 0.00%
 * 13609       27        504                0.999926519                     0.01%
 * 13610       27        504                0.999853049                     0.01%
 * 13611       27        504                0.99977959                      0.02%
 * 13612       27        504                0.999706142                     0.03%
 * 13613       27        504                0.999632704                     0.04%
 * 13614       27        504                0.999559277                     0.04%
 * 13615       27        504                0.999485861                     0.05%
 * 13616       26        524                1.000587544                     0.06%
 * 13617       26        524                1.000514063                     0.05%
 * 13618       26        524                1.000440593                     0.04%
 * 13619       26        524                1.000367134                     0.04%
 * 13620       26        524                1.000293686                     0.03%
 * 13621       26        524                1.000220248                     0.02%
 * 13622       26        524                1.000146821                     0.01%
 * 13623       26        524                1.000073405                     0.01%
 * 13624       26        524                1                            ** 0.00%
 * 13625       25        545                1                            ** 0.00%
 * 13626       25        545                0.999926611                     0.01%
 * 13627       25        545                0.999853233                     0.01%
 * 13628       29        470                1.000146757                     0.01%
 * 13629       29        470                1.000073373                     0.01%
 * 13630       29        470                1                            ** 0.00%
 * 13631       29        470                0.999926638                     0.01%
 * 13632       29        470                0.999853286                     0.01%
 * 13633       27        505                1.000146703                     0.01%
 * 13634       27        505                1.000073346                     0.01%
 * 13635       27        505                1                            ** 0.00%
 * 13636       28        487                1                            ** 0.00%
 * 13637       28        487                0.99992667                      0.01%
 * 13638       28        487                0.999853351                     0.01%
 * 13639       28        487                0.999780043                     0.02%
 * 13640       28        487                0.999706745                     0.03%
 * 13641       28        487                0.999633458                     0.04%
 * 13642       28        487                0.999560182                     0.04%
 * 13643       25        546                1.000513084                     0.05%
 * 13644       25        546                1.000439754                     0.04%
 * 13645       25        546                1.000366435                     0.04%
 * 13646       25        546                1.000293126                     0.03%
 * 13647       25        546                1.000219829                     0.02%
 * 13648       25        546                1.000146542                     0.01%
 * 13649       25        546                1.000073265                     0.01%
 * 13650       25        546                1                            ** 0.00%
 * 13651       25        546                0.999926745                     0.01%
 * 13652       25        546                0.999853501                     0.01%
 * 13653       25        546                0.999780268                     0.02%
 * 13654       25        546                0.999707046                     0.03%
 * 13655       29        471                1.000292933                     0.03%
 * 13656       29        471                1.000219684                     0.02%
 * 13657       29        471                1.000146445                     0.01%
 * 13658       29        471                1.000073217                     0.01%
 * 13659       29        471                1                            ** 0.00%
 * 13660       29        471                0.999926794                     0.01%
 * 13661       27        506                1.000073201                     0.01%
 * 13662       27        506                1                            ** 0.00%
 * 13663       28        488                1.00007319                      0.01%
 * 13664       28        488                1                            ** 0.00%
 * 13665       28        488                0.99992682                      0.01%
 * 13666       28        488                0.999853651                     0.01%
 * 13667       28        488                0.999780493                     0.02%
 * 13668       28        488                0.999707346                     0.03%
 * 13669       28        488                0.999634209                     0.04%
 * 13670       25        547                1.000365764                     0.04%
 * 13671       25        547                1.00029259                      0.03%
 * 13672       25        547                1.000219427                     0.02%
 * 13673       25        547                1.000146274                     0.01%
 * 13674       25        547                1.000073131                     0.01%
 * 13675       25        547                1                            ** 0.00%
 * 13676       26        526                1                            ** 0.00%
 * 13677       26        526                0.999926885                     0.01%
 * 13678       26        526                0.99985378                      0.01%
 * 13679       26        526                0.999780686                     0.02%
 * 13680       26        526                0.999707602                     0.03%
 * 13681       26        526                0.99963453                      0.04%
 * 13682       29        472                1.000438532                     0.04%
 * 13683       29        472                1.000365417                     0.04%
 * 13684       29        472                1.000292312                     0.03%
 * 13685       29        472                1.000219218                     0.02%
 * 13686       29        472                1.000146135                     0.01%
 * 13687       29        472                1.000073062                     0.01%
 * 13688       29        472                1                            ** 0.00%
 * 13689       27        507                1                            ** 0.00%
 * 13690       27        507                0.999926954                     0.01%
 * 13691       28        489                1.000073041                     0.01%
 * 13692       28        489                1                            ** 0.00%
 * 13693       28        489                0.99992697                      0.01%
 * 13694       28        489                0.999853951                     0.01%
 * 13695       28        489                0.999780942                     0.02%
 * 13696       25        548                1.000292056                     0.03%
 * 13697       25        548                1.000219026                     0.02%
 * 13698       25        548                1.000146007                     0.01%
 * 13699       25        548                1.000072998                     0.01%
 * 13700       25        548                1                            ** 0.00%
 * 13701       25        548                0.999927013                     0.01%
 * 13702       26        527                1                            ** 0.00%
 * 13703       26        527                0.999927023                     0.01%
 * 13704       26        527                0.999854057                     0.01%
 * 13705       26        527                0.999781102                     0.02%
 * 13706       26        527                0.999708157                     0.03%
 * 13707       26        527                0.999635223                     0.04%
 * 13708       26        527                0.999562299                     0.04%
 * 13709       26        527                0.999489387                     0.05%
 * 13710       27        508                1.000437637                     0.04%
 * 13711       27        508                1.000364671                     0.04%
 * 13712       27        508                1.000291715                     0.03%
 * 13713       27        508                1.000218771                     0.02%
 * 13714       27        508                1.000145836                     0.01%
 * 13715       27        508                1.000072913                     0.01%
 * 13716       27        508                1                            ** 0.00%
 * 13717       29        473                1                            ** 0.00%
 * 13718       29        473                0.999927103                     0.01%
 * 13719       28        490                1.000072892                     0.01%
 * 13720       28        490                1                            ** 0.00%
 * 13721       28        490                0.999927119                     0.01%
 * 13722       28        490                0.999854249                     0.01%
 * 13723       25        549                1.000145741                     0.01%
 * 13724       25        549                1.000072865                     0.01%
 * 13725       25        549                1                            ** 0.00%
 * 13726       25        549                0.999927146                     0.01%
 * 13727       26        528                1.000072849                     0.01%
 * 13728       26        528                1                            ** 0.00%
 * 13729       26        528                0.999927161                     0.01%
 * 13730       26        528                0.999854334                     0.01%
 * 13731       26        528                0.999781516                     0.02%
 * 13732       26        528                0.99970871                      0.03%
 * 13733       26        528                0.999635913                     0.04%
 * 13734       26        528                0.999563128                     0.04%
 * 13735       26        528                0.999490353                     0.05%
 * 13736       27        509                1.00050961                      0.05%
 * 13737       27        509                1.000436777                     0.04%
 * 13738       27        509                1.000363954                     0.04%
 * 13739       27        509                1.000291142                     0.03%
 * 13740       27        509                1.000218341                     0.02%
 * 13741       27        509                1.00014555                      0.01%
 * 13742       27        509                1.00007277                      0.01%
 * 13743       27        509                1                            ** 0.00%
 * 13744       27        509                0.999927241                     0.01%
 * 13745       29        474                1.000072754                     0.01%
 * 13746       29        474                1                            ** 0.00%
 * 13747       28        491                1.000072743                     0.01%
 * 13748       28        491                1                            ** 0.00%
 * 13749       25        550                1.000072733                     0.01%
 * 13750       25        550                1                            ** 0.00%
 * 13751       25        550                0.999927278                     0.01%
 * 13752       25        550                0.999854567                     0.01%
 * 13753       26        529                1.000072711                     0.01%
 * 13754       26        529                1                            ** 0.00%
 * 13755       26        529                0.999927299                     0.01%
 * 13756       26        529                0.999854609                     0.01%
 * 13757       26        529                0.999781929                     0.02%
 * 13758       26        529                0.99970926                      0.03%
 * 13759       26        529                0.999636601                     0.04%
 * 13760       26        529                0.999563953                     0.04%
 * 13761       26        529                0.999491316                     0.05%
 * 13762       26        529                0.999418689                     0.06%
 * 13763       27        510                1.00050861                      0.05%
 * 13764       27        510                1.00043592                      0.04%
 * 13765       27        510                1.00036324                      0.04%
 * 13766       27        510                1.000290571                     0.03%
 * 13767       27        510                1.000217912                     0.02%
 * 13768       27        510                1.000145264                     0.01%
 * 13769       27        510                1.000072627                     0.01%
 * 13770       27        510                1                            ** 0.00%
 * 13771       27        510                0.999927384                     0.01%
 * 13772       27        510                0.999854778                     0.01%
 * 13773       29        475                1.000145212                     0.01%
 * 13774       29        475                1.000072601                     0.01%
 * 13775       29        475                1                            ** 0.00%
 * 13776       28        492                1                            ** 0.00%
 * 13777       28        492                0.999927415                     0.01%
 * 13778       28        492                0.999854841                     0.01%
 * 13779       26        530                1.000072574                     0.01%
 * 13780       26        530                1                            ** 0.00%
 * 13781       26        530                0.999927436                     0.01%
 * 13782       26        530                0.999854883                     0.01%
 * 13783       26        530                0.999782341                     0.02%
 * 13784       26        530                0.999709808                     0.03%
 * 13785       26        530                0.999637287                     0.04%
 * 13786       26        530                0.999564776                     0.04%
 * 13787       26        530                0.999492275                     0.05%
 * 13788       26        530                0.999419785                     0.06%
 * 13789       27        511                1.000580173                     0.06%
 * 13790       27        511                1.000507614                     0.05%
 * 13791       27        511                1.000435066                     0.04%
 * 13792       27        511                1.000362529                     0.04%
 * 13793       27        511                1.000290002                     0.03%
 * 13794       27        511                1.000217486                     0.02%
 * 13795       27        511                1.00014498                      0.01%
 * 13796       27        511                1.000072485                     0.01%
 * 13797       27        511                1                            ** 0.00%
 * 13798       27        511                0.999927526                     0.01%
 * 13799       27        511                0.999855062                     0.01%
 * 13800       27        511                0.999782609                     0.02%
 * 13801       28        493                1.000217376                     0.02%
 * 13802       28        493                1.000144907                     0.01%
 * 13803       28        493                1.000072448                     0.01%
 * 13804       28        493                1                            ** 0.00%
 * 13805       26        531                1.000072438                     0.01%
 * 13806       26        531                1                            ** 0.00%
 * 13807       26        531                0.999927573                     0.01%
 * 13808       26        531                0.999855156                     0.01%
 * 13809       26        531                0.99978275                      0.02%
 * 13810       26        531                0.999710355                     0.03%
 * 13811       26        531                0.99963797                      0.04%
 * 13812       26        531                0.999565595                     0.04%
 * 13813       26        531                0.999493231                     0.05%
 * 13814       26        531                0.999420877                     0.06%
 * 13815       26        531                0.999348534                     0.07%
 * 13816       27        512                1.000579039                     0.06%
 * 13817       27        512                1.000506622                     0.05%
 * 13818       27        512                1.000434216                     0.04%
 * 13819       27        512                1.000361821                     0.04%
 * 13820       27        512                1.000289436                     0.03%
 * 13821       27        512                1.000217061                     0.02%
 * 13822       27        512                1.000144697                     0.01%
 * 13823       27        512                1.000072343                     0.01%
 * 13824       27        512                1                            ** 0.00%
 * 13825       27        512                0.999927667                     0.01%
 * 13826       27        512                0.999855345                     0.01%
 * 13827       27        512                0.999783033                     0.02%
 * 13828       26        532                1.000289268                     0.03%
 * 13829       26        532                1.000216935                     0.02%
 * 13830       26        532                1.000144613                     0.01%
 * 13831       26        532                1.000072301                     0.01%
 * 13832       26        532                1                            ** 0.00%
 * 13833       29        477                1                            ** 0.00%
 * 13834       29        477                0.999927714                     0.01%
 * 13835       29        477                0.999855439                     0.01%
 * 13836       29        477                0.999783174                     0.02%
 * 13837       29        477                0.99971092                      0.03%
 * 13838       29        477                0.999638676                     0.04%
 * 13839       29        477                0.999566443                     0.04%
 * 13840       29        477                0.99949422                      0.05%
 * 13841       29        477                0.999422007                     0.06%
 * 13842       27        513                1.000650195                     0.07%
 * 13843       27        513                1.000577909                     0.06%
 * 13844       27        513                1.000505634                     0.05%
 * 13845       27        513                1.000433369                     0.04%
 * 13846       27        513                1.000361115                     0.04%
 * 13847       27        513                1.000288871                     0.03%
 * 13848       27        513                1.000216638                     0.02%
 * 13849       27        513                1.000144415                     0.01%
 * 13850       27        513                1.000072202                     0.01%
 * 13851       27        513                1                            ** 0.00%
 * 13852       27        513                0.999927808                     0.01%
 * 13853       27        513                0.999855627                     0.01%
 * 13854       27        513                0.999783456                     0.02%
 * 13855       26        533                1.000216528                     0.02%
 * 13856       26        533                1.000144342                     0.01%
 * 13857       26        533                1.000072166                     0.01%
 * 13858       26        533                1                            ** 0.00%
 * 13859       26        533                0.999927845                     0.01%
 * 13860       28        495                1                            ** 0.00%
 * 13861       28        495                0.999927855                     0.01%
 * 13862       29        478                1                            ** 0.00%
 * 13863       29        478                0.999927866                     0.01%
 * 13864       29        478                0.999855741                     0.01%
 * 13865       29        478                0.999783628                     0.02%
 * 13866       29        478                0.999711525                     0.03%
 * 13867       29        478                0.999639432                     0.04%
 * 13868       29        478                0.999567349                     0.04%
 * 13869       29        478                0.999495277                     0.05%
 * 13870       27        514                1.000576784                     0.06%
 * 13871       27        514                1.00050465                      0.05%
 * 13872       27        514                1.000432526                     0.04%
 * 13873       27        514                1.000360412                     0.04%
 * 13874       27        514                1.000288309                     0.03%
 * 13875       27        514                1.000216216                     0.02%
 * 13876       27        514                1.000144134                     0.01%
 * 13877       27        514                1.000072062                     0.01%
 * 13878       27        514                1                            ** 0.00%
 * 13879       27        514                0.999927949                     0.01%
 * 13880       27        514                0.999855908                     0.01%
 * 13881       26        534                1.000216123                     0.02%
 * 13882       26        534                1.000144071                     0.01%
 * 13883       26        534                1.000072031                     0.01%
 * 13884       26        534                1                            ** 0.00%
 * 13885       26        534                0.99992798                      0.01%
 * 13886       26        534                0.99985597                      0.01%
 * 13887       28        496                1.00007201                      0.01%
 * 13888       28        496                1                            ** 0.00%
 * 13889       28        496                0.999928001                     0.01%
 * 13890       29        479                1.000071994                     0.01%
 * 13891       29        479                1                            ** 0.00%
 * 13892       29        479                0.999928016                     0.01%
 * 13893       29        479                0.999856043                     0.01%
 * 13894       29        479                0.999784079                     0.02%
 * 13895       29        479                0.999712127                     0.03%
 * 13896       29        479                0.999640184                     0.04%
 * 13897       29        479                0.999568252                     0.04%
 * 13898       27        515                1.00050367                      0.05%
 * 13899       27        515                1.000431686                     0.04%
 * 13900       27        515                1.000359712                     0.04%
 * 13901       27        515                1.000287749                     0.03%
 * 13902       27        515                1.000215796                     0.02%
 * 13903       27        515                1.000143854                     0.01%
 * 13904       27        515                1.000071922                     0.01%
 * 13905       27        515                1                            ** 0.00%
 * 13906       27        515                0.999928089                     0.01%
 * 13907       27        515                0.999856188                     0.01%
 * 13908       26        535                1.000143802                     0.01%
 * 13909       26        535                1.000071896                     0.01%
 * 13910       26        535                1                            ** 0.00%
 * 13911       26        535                0.999928114                     0.01%
 * 13912       26        535                0.999856239                     0.01%
 * 13913       28        497                1.000215626                     0.02%
 * 13914       28        497                1.00014374                      0.01%
 * 13915       28        497                1.000071865                     0.01%
 * 13916       28        497                1                            ** 0.00%
 * 13917       28        497                0.999928145                     0.01%
 * 13918       29        480                1.000143699                     0.01%
 * 13919       29        480                1.000071844                     0.01%
 * 13920       29        480                1                            ** 0.00%
 * 13921       29        480                0.999928166                     0.01%
 * 13922       29        480                0.999856342                     0.01%
 * 13923       29        480                0.999784529                     0.02%
 * 13924       29        480                0.999712726                     0.03%
 * 13925       29        480                0.999640934                     0.04%
 * 13926       27        516                1.000430849                     0.04%
 * 13927       27        516                1.000359015                     0.04%
 * 13928       27        516                1.000287191                     0.03%
 * 13929       27        516                1.000215378                     0.02%
 * 13930       27        516                1.000143575                     0.01%
 * 13931       27        516                1.000071782                     0.01%
 * 13932       27        516                1                            ** 0.00%
 * 13933       27        516                0.999928228                     0.01%
 * 13934       26        536                1.000143534                     0.01%
 * 13935       26        536                1.000071762                     0.01%
 * 13936       26        536                1                            ** 0.00%
 * 13937       26        536                0.999928249                     0.01%
 * 13938       26        536                0.999856507                     0.01%
 * 13939       26        536                0.999784777                     0.02%
 * 13940       26        536                0.999713056                     0.03%
 * 13941       28        498                1.000215193                     0.02%
 * 13942       28        498                1.000143451                     0.01%
 * 13943       28        498                1.000071721                     0.01%
 * 13944       28        498                1                            ** 0.00%
 * 13945       28        498                0.99992829                      0.01%
 * 13946       28        498                0.99985659                      0.01%
 * 13947       29        481                1.0001434                       0.01%
 * 13948       29        481                1.000071695                     0.01%
 * 13949       29        481                1                            ** 0.00%
 * 13950       29        481                0.999928315                     0.01%
 * 13951       29        481                0.999856641                     0.01%
 * 13952       29        481                0.999784977                     0.02%
 * 13953       29        481                0.999713323                     0.03%
 * 13954       27        517                1.00035832                      0.04%
 * 13955       27        517                1.000286636                     0.03%
 * 13956       27        517                1.000214961                     0.02%
 * 13957       27        517                1.000143297                     0.01%
 * 13958       27        517                1.000071644                     0.01%
 * 13959       27        517                1                            ** 0.00%
 * 13960       27        517                0.999928367                     0.01%
 * 13961       26        537                1.000071628                     0.01%
 * 13962       26        537                1                            ** 0.00%
 * 13963       26        537                0.999928382                     0.01%
 * 13964       26        537                0.999856775                     0.01%
 * 13965       26        537                0.999785177                     0.02%
 * 13966       26        537                0.99971359                      0.03%
 * 13967       28        499                1.000357987                     0.04%
 * 13968       28        499                1.000286369                     0.03%
 * 13969       28        499                1.000214761                     0.02%
 * 13970       28        499                1.000143164                     0.01%
 * 13971       28        499                1.000071577                     0.01%
 * 13972       28        499                1                            ** 0.00%
 * 13973       28        499                0.999928433                     0.01%
 * 13974       28        499                0.999856877                     0.01%
 * 13975       29        482                1.000214669                     0.02%
 * 13976       29        482                1.000143102                     0.01%
 * 13977       29        482                1.000071546                     0.01%
 * 13978       29        482                1                            ** 0.00%
 * 13979       29        482                0.999928464                     0.01%
 * 13980       29        482                0.999856938                     0.01%
 * 13981       29        482                0.999785423                     0.02%
 * 13982       29        482                0.999713918                     0.03%
 * 13983       27        518                1.000214546                     0.02%
 * 13984       27        518                1.000143021                     0.01%
 * 13985       27        518                1.000071505                     0.01%
 * 13986       27        518                1                            ** 0.00%
 * 13987       27        518                0.999928505                     0.01%
 * 13988       26        538                1                            ** 0.00%
 * 13989       26        538                0.999928515                     0.01%
 * 13990       26        538                0.999857041                     0.01%
 * 13991       26        538                0.999785576                     0.02%
 * 13992       26        538                0.999714122                     0.03%
 * 13993       26        538                0.999642678                     0.04%
 * 13994       26        538                0.999571245                     0.04%
 * 13995       28        500                1.00035727                      0.04%
 * 13996       28        500                1.000285796                     0.03%
 * 13997       28        500                1.000214332                     0.02%
 * 13998       28        500                1.000142878                     0.01%
 * 13999       28        500                1.000071434                     0.01%
 * 14000       28        500                1                            ** 0.00%
 * 14001       28        500                0.999928577                     0.01%
 * 14002       28        500                0.999857163                     0.01%
 * 14003       28        500                0.99978576                      0.02%
 * 14004       29        483                1.000214225                     0.02%
 * 14005       29        483                1.000142806                     0.01%
 * 14006       29        483                1.000071398                     0.01%
 * 14007       29        483                1                            ** 0.00%
 * 14008       29        483                0.999928612                     0.01%
 * 14009       29        483                0.999857235                     0.01%
 * 14010       27        519                1.000214133                     0.02%
 * 14011       27        519                1.000142745                     0.01%
 * 14012       27        519                1.000071367                     0.01%
 * 14013       27        519                1                            ** 0.00%
 * 14014       26        539                1                            ** 0.00%
 * 14015       26        539                0.999928648                     0.01%
 * 14016       26        539                0.999857306                     0.01%
 * 14017       26        539                0.999785974                     0.02%
 * 14018       26        539                0.999714653                     0.03%
 * 14019       26        539                0.999643341                     0.04%
 * 14020       26        539                0.99957204                      0.04%
 * 14021       26        539                0.999500749                     0.05%
 * 14022       28        501                1.000427899                     0.04%
 * 14023       28        501                1.000356557                     0.04%
 * 14024       28        501                1.000285225                     0.03%
 * 14025       28        501                1.000213904                     0.02%
 * 14026       28        501                1.000142592                     0.01%
 * 14027       28        501                1.000071291                     0.01%
 * 14028       28        501                1                            ** 0.00%
 * 14029       28        501                0.999928719                     0.01%
 * 14030       28        501                0.999857448                     0.01%
 * 14031       28        501                0.999786188                     0.02%
 * 14032       28        501                0.999714937                     0.03%
 * 14033       29        484                1.000213782                     0.02%
 * 14034       29        484                1.000142511                     0.01%
 * 14035       29        484                1.00007125                      0.01%
 * 14036       29        484                1                            ** 0.00%
 * 14037       29        484                0.99992876                      0.01%
 * 14038       26        540                1.00014247                      0.01%
 * 14039       26        540                1.00007123                      0.01%
 * 14040       26        540                1                            ** 0.00%
 * 14041       26        540                0.99992878                      0.01%
 * 14042       26        540                0.99985757                      0.01%
 * 14043       26        540                0.99978637                      0.02%
 * 14044       26        540                0.999715181                     0.03%
 * 14045       26        540                0.999644001                     0.04%
 * 14046       26        540                0.999572832                     0.04%
 * 14047       26        540                0.999501673                     0.05%
 * 14048       26        540                0.999430524                     0.06%
 * 14049       28        502                1.000498256                     0.05%
 * 14050       28        502                1.000427046                     0.04%
 * 14051       28        502                1.000355847                     0.04%
 * 14052       28        502                1.000284657                     0.03%
 * 14053       28        502                1.000213478                     0.02%
 * 14054       28        502                1.000142308                     0.01%
 * 14055       28        502                1.000071149                     0.01%
 * 14056       28        502                1                            ** 0.00%
 * 14057       28        502                0.999928861                     0.01%
 * 14058       28        502                0.999857732                     0.01%
 * 14059       28        502                0.999786614                     0.02%
 * 14060       28        502                0.999715505                     0.03%
 * 14061       29        485                1.000284475                     0.03%
 * 14062       29        485                1.000213341                     0.02%
 * 14063       29        485                1.000142217                     0.01%
 * 14064       29        485                1.000071104                     0.01%
 * 14065       29        485                1                            ** 0.00%
 * 14066       26        541                1                            ** 0.00%
 * 14067       27        521                1                            ** 0.00%
 * 14068       27        521                0.999928917                     0.01%
 * 14069       27        521                0.999857843                     0.01%
 * 14070       27        521                0.99978678                      0.02%
 * 14071       27        521                0.999715727                     0.03%
 * 14072       27        521                0.999644684                     0.04%
 * 14073       27        521                0.999573652                     0.04%
 * 14074       27        521                0.999502629                     0.05%
 * 14075       27        521                0.999431616                     0.06%
 * 14076       28        503                1.000568343                     0.06%
 * 14077       28        503                1.000497265                     0.05%
 * 14078       28        503                1.000426197                     0.04%
 * 14079       28        503                1.000355139                     0.04%
 * 14080       28        503                1.000284091                     0.03%
 * 14081       28        503                1.000213053                     0.02%
 * 14082       28        503                1.000142025                     0.01%
 * 14083       28        503                1.000071008                     0.01%
 * 14084       28        503                1                            ** 0.00%
 * 14085       28        503                0.999929002                     0.01%
 * 14086       28        503                0.999858015                     0.01%
 * 14087       28        503                0.999787038                     0.02%
 * 14088       28        503                0.99971607                      0.03%
 * 14089       26        542                1.000212932                     0.02%
 * 14090       26        542                1.000141945                     0.01%
 * 14091       26        542                1.000070967                     0.01%
 * 14092       26        542                1                            ** 0.00%
 * 14093       27        522                1.000070957                     0.01%
 * 14094       27        522                1                            ** 0.00%
 * 14095       27        522                0.999929053                     0.01%
 * 14096       27        522                0.999858116                     0.01%
 * 14097       27        522                0.999787189                     0.02%
 * 14098       27        522                0.999716272                     0.03%
 * 14099       27        522                0.999645365                     0.04%
 * 14100       27        522                0.999574468                     0.04%
 * 14101       27        522                0.999503581                     0.05%
 * 14102       27        522                0.999432705                     0.06%
 * 14103       27        522                0.999361838                     0.06%
 * 14104       28        504                1.000567215                     0.06%
 * 14105       28        504                1.000496278                     0.05%
 * 14106       28        504                1.000425351                     0.04%
 * 14107       28        504                1.000354434                     0.04%
 * 14108       28        504                1.000283527                     0.03%
 * 14109       28        504                1.00021263                      0.02%
 * 14110       28        504                1.000141743                     0.01%
 * 14111       28        504                1.000070867                     0.01%
 * 14112       28        504                1                            ** 0.00%
 * 14113       28        504                0.999929143                     0.01%
 * 14114       28        504                0.999858297                     0.01%
 * 14115       26        543                1.00021254                      0.02%
 * 14116       26        543                1.000141683                     0.01%
 * 14117       26        543                1.000070837                     0.01%
 * 14118       26        543                1                            ** 0.00%
 * 14119       26        543                0.999929173                     0.01%
 * 14120       27        523                1.000070822                     0.01%
 * 14121       27        523                1                            ** 0.00%
 * 14122       27        523                0.999929189                     0.01%
 * 14123       29        487                1                            ** 0.00%
 * 14124       29        487                0.999929199                     0.01%
 * 14125       29        487                0.999858407                     0.01%
 * 14126       29        487                0.999787626                     0.02%
 * 14127       29        487                0.999716854                     0.03%
 * 14128       29        487                0.999646093                     0.04%
 * 14129       29        487                0.999575341                     0.04%
 * 14130       29        487                0.9995046                       0.05%
 * 14131       29        487                0.999433869                     0.06%
 * 14132       28        505                1.000566091                     0.06%
 * 14133       28        505                1.000495295                     0.05%
 * 14134       28        505                1.000424508                     0.04%
 * 14135       28        505                1.000353732                     0.04%
 * 14136       28        505                1.000282965                     0.03%
 * 14137       28        505                1.000212209                     0.02%
 * 14138       28        505                1.000141463                     0.01%
 * 14139       28        505                1.000070726                     0.01%
 * 14140       28        505                1                            ** 0.00%
 * 14141       28        505                0.999929284                     0.01%
 * 14142       26        544                1.000141423                     0.01%
 * 14143       26        544                1.000070706                     0.01%
 * 14144       26        544                1                            ** 0.00%
 * 14145       26        544                0.999929304                     0.01%
 * 14146       26        544                0.999858617                     0.01%
 * 14147       27        524                1.000070686                     0.01%
 * 14148       27        524                1                            ** 0.00%
 * 14149       27        524                0.999929324                     0.01%
 * 14150       27        524                0.999858657                     0.01%
 * 14151       29        488                1.000070666                     0.01%
 * 14152       29        488                1                            ** 0.00%
 * 14153       29        488                0.999929344                     0.01%
 * 14154       29        488                0.999858697                     0.01%
 * 14155       29        488                0.999788061                     0.02%
 * 14156       29        488                0.999717434                     0.03%
 * 14157       29        488                0.999646818                     0.04%
 * 14158       29        488                0.999576211                     0.04%
 * 14159       29        488                0.999505615                     0.05%
 * 14160       28        506                1.000564972                     0.06%
 * 14161       28        506                1.000494315                     0.05%
 * 14162       28        506                1.000423669                     0.04%
 * 14163       28        506                1.000353033                     0.04%
 * 14164       28        506                1.000282406                     0.03%
 * 14165       28        506                1.00021179                      0.02%
 * 14166       28        506                1.000141183                     0.01%
 * 14167       28        506                1.000070587                     0.01%
 * 14168       28        506                1                            ** 0.00%
 * 14169       28        506                0.999929423                     0.01%
 * 14170       26        545                1                            ** 0.00%
 * 14171       26        545                0.999929433                     0.01%
 * 14172       26        545                0.999858877                     0.01%
 * 14173       27        525                1.000141113                     0.01%
 * 14174       27        525                1.000070552                     0.01%
 * 14175       27        525                1                            ** 0.00%
 * 14176       27        525                0.999929458                     0.01%
 * 14177       27        525                0.999858926                     0.01%
 * 14178       27        525                0.999788405                     0.02%
 * 14179       29        489                1.000141054                     0.01%
 * 14180       29        489                1.000070522                     0.01%
 * 14181       29        489                1                            ** 0.00%
 * 14182       29        489                0.999929488                     0.01%
 * 14183       29        489                0.999858986                     0.01%
 * 14184       29        489                0.999788494                     0.02%
 * 14185       29        489                0.999718012                     0.03%
 * 14186       29        489                0.99964754                      0.04%
 * 14187       29        489                0.999577078                     0.04%
 * 14188       29        489                0.999506625                     0.05%
 * 14189       26        546                1.00049334                      0.05%
 * 14190       26        546                1.000422833                     0.04%
 * 14191       26        546                1.000352336                     0.04%
 * 14192       26        546                1.000281849                     0.03%
 * 14193       26        546                1.000211372                     0.02%
 * 14194       26        546                1.000140905                     0.01%
 * 14195       26        546                1.000070447                     0.01%
 * 14196       26        546                1                            ** 0.00%
 * 14197       26        546                0.999929563                     0.01%
 * 14198       26        546                0.999859135                     0.01%
 * 14199       26        546                0.999788718                     0.02%
 * 14200       27        526                1.000140845                     0.01%
 * 14201       27        526                1.000070418                     0.01%
 * 14202       27        526                1                            ** 0.00%
 * 14203       27        526                0.999929592                     0.01%
 * 14204       27        526                0.999859195                     0.01%
 * 14205       27        526                0.999788807                     0.02%
 * 14206       27        526                0.999718429                     0.03%
 * 14207       29        490                1.000211164                     0.02%
 * 14208       29        490                1.000140766                     0.01%
 * 14209       29        490                1.000070378                     0.01%
 * 14210       29        490                1                            ** 0.00%
 * 14211       29        490                0.999929632                     0.01%
 * 14212       29        490                0.999859274                     0.01%
 * 14213       29        490                0.999788926                     0.02%
 * 14214       29        490                0.999718587                     0.03%
 * 14215       29        490                0.999648259                     0.04%
 * 14216       26        547                1.00042206                      0.04%
 * 14217       26        547                1.000351692                     0.04%
 * 14218       26        547                1.000281334                     0.03%
 * 14219       26        547                1.000210985                     0.02%
 * 14220       26        547                1.000140647                     0.01%
 * 14221       26        547                1.000070319                     0.01%
 * 14222       26        547                1                            ** 0.00%
 * 14223       28        508                1.000070309                     0.01%
 * 14224       28        508                1                            ** 0.00%
 * 14225       28        508                0.999929701                     0.01%
 * 14226       28        508                0.999859412                     0.01%
 * 14227       27        527                1.000140578                     0.01%
 * 14228       27        527                1.000070284                     0.01%
 * 14229       27        527                1                            ** 0.00%
 * 14230       27        527                0.999929726                     0.01%
 * 14231       27        527                0.999859462                     0.01%
 * 14232       27        527                0.999789207                     0.02%
 * 14233       27        527                0.999718963                     0.03%
 * 14234       27        527                0.999648728                     0.04%
 * 14235       29        491                1.000280998                     0.03%
 * 14236       29        491                1.000210733                     0.02%
 * 14237       29        491                1.000140479                     0.01%
 * 14238       29        491                1.000070235                     0.01%
 * 14239       29        491                1                            ** 0.00%
 * 14240       29        491                0.999929775                     0.01%
 * 14241       29        491                0.99985956                      0.01%
 * 14242       29        491                0.999789355                     0.02%
 * 14243       29        491                0.99971916                      0.03%
 * 14244       26        548                1.00028082                      0.03%
 * 14245       26        548                1.0002106                       0.02%
 * 14246       26        548                1.00014039                      0.01%
 * 14247       26        548                1.00007019                      0.01%
 * 14248       26        548                1                            ** 0.00%
 * 14249       26        548                0.99992982                      0.01%
 * 14250       28        509                1.000140351                     0.01%
 * 14251       28        509                1.000070171                     0.01%
 * 14252       28        509                1                            ** 0.00%
 * 14253       28        509                0.999929839                     0.01%
 * 14254       27        528                1.000140311                     0.01%
 * 14255       27        528                1.000070151                     0.01%
 * 14256       27        528                1                            ** 0.00%
 * 14257       27        528                0.999929859                     0.01%
 * 14258       27        528                0.999859728                     0.01%
 * 14259       27        528                0.999789607                     0.02%
 * 14260       27        528                0.999719495                     0.03%
 * 14261       27        528                0.999649393                     0.04%
 * 14262       27        528                0.999579302                     0.04%
 * 14263       29        492                1.000350557                     0.04%
 * 14264       29        492                1.000280426                     0.03%
 * 14265       29        492                1.000210305                     0.02%
 * 14266       29        492                1.000140193                     0.01%
 * 14267       29        492                1.000070092                     0.01%
 * 14268       29        492                1                            ** 0.00%
 * 14269       29        492                0.999929918                     0.01%
 * 14270       29        492                0.999859846                     0.01%
 * 14271       26        549                1.000210217                     0.02%
 * 14272       26        549                1.000140135                     0.01%
 * 14273       26        549                1.000070062                     0.01%
 * 14274       26        549                1                            ** 0.00%
 * 14275       26        549                0.999929947                     0.01%
 * 14276       26        549                0.999859905                     0.01%
 * 14277       26        549                0.999789872                     0.02%
 * 14278       28        510                1.000140076                     0.01%
 * 14279       28        510                1.000070033                     0.01%
 * 14280       28        510                1                            ** 0.00%
 * 14281       28        510                0.999929977                     0.01%
 * 14282       27        529                1.000070018                     0.01%
 * 14283       27        529                1                            ** 0.00%
 * 14284       27        529                0.999929992                     0.01%
 * 14285       27        529                0.999859993                     0.01%
 * 14286       27        529                0.999790004                     0.02%
 * 14287       27        529                0.999720025                     0.03%
 * 14288       27        529                0.999650056                     0.03%
 * 14289       27        529                0.999580097                     0.04%
 * 14290       29        493                1.000489853                     0.05%
 * 14291       29        493                1.000419845                     0.04%
 * 14292       29        493                1.000349846                     0.03%
 * 14293       29        493                1.000279857                     0.03%
 * 14294       29        493                1.000209878                     0.02%
 * 14295       29        493                1.000139909                     0.01%
 * 14296       29        493                1.00006995                      0.01%
 * 14297       29        493                1                            ** 0.00%
 * 14298       29        493                0.99993006                      0.01%
 * 14299       26        550                1.000069935                     0.01%
 * 14300       26        550                1                            ** 0.00%
 * 14301       26        550                0.999930075                     0.01%
 * 14302       26        550                0.999860159                     0.01%
 * 14303       26        550                0.999790254                     0.02%
 * 14304       28        511                1.000279642                     0.03%
 * 14305       28        511                1.000209717                     0.02%
 * 14306       28        511                1.000139801                     0.01%
 * 14307       28        511                1.000069896                     0.01%
 * 14308       28        511                1                            ** 0.00%
 * 14309       27        530                1.000069886                     0.01%
 * 14310       27        530                1                            ** 0.00%
 * 14311       27        530                0.999930124                     0.01%
 * 14312       27        530                0.999860257                     0.01%
 * 14313       27        530                0.9997904                       0.02%
 * 14314       27        530                0.999720553                     0.03%
 * 14315       27        530                0.999650716                     0.03%
 * 14316       27        530                0.999580889                     0.04%
 * 14317       27        530                0.999511071                     0.05%
 * 14318       27        530                0.999441263                     0.06%
 * 14319       29        494                1.000488861                     0.05%
 * 14320       29        494                1.000418994                     0.04%
 * 14321       29        494                1.000349138                     0.03%
 * 14322       29        494                1.000279291                     0.03%
 * 14323       29        494                1.000209453                     0.02%
 * 14324       29        494                1.000139626                     0.01%
 * 14325       29        494                1.000069808                     0.01%
 * 14326       29        494                1                            ** 0.00%
 * 14327       29        494                0.999930202                     0.01%
 * 14328       29        494                0.999860413                     0.01%
 * 14329       29        494                0.999790634                     0.02%
 * 14330       29        494                0.999720865                     0.03%
 * 14331       28        512                1.000348894                     0.03%
 * 14332       28        512                1.000279096                     0.03%
 * 14333       28        512                1.000209307                     0.02%
 * 14334       28        512                1.000139528                     0.01%
 * 14335       28        512                1.000069759                     0.01%
 * 14336       28        512                1                            ** 0.00%
 * 14337       27        531                1                            ** 0.00%
 * 14338       27        531                0.999930255                     0.01%
 * 14339       27        531                0.99986052                      0.01%
 * 14340       27        531                0.999790795                     0.02%
 * 14341       27        531                0.999721079                     0.03%
 * 14342       27        531                0.999651374                     0.03%
 * 14343       27        531                0.999581677                     0.04%
 * 14344       27        531                0.999511991                     0.05%
 * 14345       27        531                0.999442314                     0.06%
 * 14346       27        531                0.999372647                     0.06%
 * 14347       29        495                1.000557608                     0.06%
 * 14348       29        495                1.000487873                     0.05%
 * 14349       29        495                1.000418148                     0.04%
 * 14350       29        495                1.000348432                     0.03%
 * 14351       29        495                1.000278726                     0.03%
 * 14352       29        495                1.00020903                      0.02%
 * 14353       29        495                1.000139344                     0.01%
 * 14354       29        495                1.000069667                     0.01%
 * 14355       29        495                1                            ** 0.00%
 * 14356       29        495                0.999930343                     0.01%
 * 14357       29        495                0.999860695                     0.01%
 * 14358       29        495                0.999791057                     0.02%
 * 14359       29        495                0.999721429                     0.03%
 * 14360       27        532                1.000278552                     0.03%
 * 14361       27        532                1.000208899                     0.02%
 * 14362       27        532                1.000139256                     0.01%
 * 14363       27        532                1.000069623                     0.01%
 * 14364       27        532                1                            ** 0.00%
 * 14365       27        532                0.999930386                     0.01%
 * 14366       27        532                0.999860782                     0.01%
 * 14367       27        532                0.999791188                     0.02%
 * 14368       27        532                0.999721604                     0.03%
 * 14369       27        532                0.999652029                     0.03%
 * 14370       27        532                0.999582463                     0.04%
 * 14371       27        532                0.999512908                     0.05%
 * 14372       27        532                0.999443362                     0.06%
 * 14373       27        532                0.999373826                     0.06%
 * 14374       29        496                1.000695701                     0.07%
 * 14375       29        496                1.000626087                     0.06%
 * 14376       29        496                1.000556483                     0.06%
 * 14377       29        496                1.000486889                     0.05%
 * 14378       29        496                1.000417304                     0.04%
 * 14379       29        496                1.000347729                     0.03%
 * 14380       29        496                1.000278164                     0.03%
 * 14381       29        496                1.000208609                     0.02%
 * 14382       29        496                1.000139063                     0.01%
 * 14383       29        496                1.000069527                     0.01%
 * 14384       29        496                1                            ** 0.00%
 * 14385       29        496                0.999930483                     0.01%
 * 14386       29        496                0.999860976                     0.01%
 * 14387       29        496                0.999791478                     0.02%
 * 14388       27        533                1.000208507                     0.02%
 * 14389       27        533                1.000138995                     0.01%
 * 14390       27        533                1.000069493                     0.01%
 * 14391       27        533                1                            ** 0.00%
 * 14392       28        514                1                            ** 0.00%
 * 14393       28        514                0.999930522                     0.01%
 * 14394       28        514                0.999861053                     0.01%
 * 14395       28        514                0.999791594                     0.02%
 * 14396       28        514                0.999722145                     0.03%
 * 14397       28        514                0.999652705                     0.03%
 * 14398       28        514                0.999583275                     0.04%
 * 14399       28        514                0.999513855                     0.05%
 * 14400       28        514                0.999444444                     0.06%
 * 14401       28        514                0.999375043                     0.06%
 * 14402       28        514                0.999305652                     0.07%
 * 14403       29        497                1.0006943                       0.07%
 * 14404       29        497                1.000624826                     0.06%
 * 14405       29        497                1.000555363                     0.06%
 * 14406       29        497                1.000485909                     0.05%
 * 14407       29        497                1.000416464                     0.04%
 * 14408       29        497                1.000347029                     0.03%
 * 14409       29        497                1.000277604                     0.03%
 * 14410       29        497                1.000208189                     0.02%
 * 14411       29        497                1.000138783                     0.01%
 * 14412       29        497                1.000069387                     0.01%
 * 14413       29        497                1                            ** 0.00%
 * 14414       29        497                0.999930623                     0.01%
 * 14415       29        497                0.999861256                     0.01%
 * 14416       27        534                1.000138735                     0.01%
 * 14417       27        534                1.000069363                     0.01%
 * 14418       27        534                1                            ** 0.00%
 * 14419       27        534                0.999930647                     0.01%
 * 14420       28        515                1                            ** 0.00%
 * 14421       28        515                0.999930657                     0.01%
 * 14422       28        515                0.999861323                     0.01%
 * 14423       28        515                0.999791999                     0.02%
 * 14424       28        515                0.999722684                     0.03%
 * 14425       28        515                0.99965338                      0.03%
 * 14426       28        515                0.999584084                     0.04%
 * 14427       28        515                0.999514799                     0.05%
 * 14428       28        515                0.999445523                     0.06%
 * 14429       28        515                0.999376256                     0.06%
 * 14430       28        515                0.999306999                     0.07%
 * 14431       28        515                0.999237752                     0.08%
 * 14432       29        498                1.000692905                     0.07%
 * 14433       29        498                1.000623571                     0.06%
 * 14434       29        498                1.000554247                     0.06%
 * 14435       29        498                1.000484932                     0.05%
 * 14436       29        498                1.000415628                     0.04%
 * 14437       29        498                1.000346332                     0.03%
 * 14438       29        498                1.000277047                     0.03%
 * 14439       29        498                1.000207771                     0.02%
 * 14440       29        498                1.000138504                     0.01%
 * 14441       29        498                1.000069247                     0.01%
 * 14442       29        498                1                            ** 0.00%
 * 14443       29        498                0.999930762                     0.01%
 * 14444       27        535                1.000069233                     0.01%
 * 14445       27        535                1                            ** 0.00%
 * 14446       27        535                0.999930777                     0.01%
 * 14447       28        516                1.000069219                     0.01%
 * 14448       28        516                1                            ** 0.00%
 * 14449       28        516                0.999930791                     0.01%
 * 14450       28        516                0.999861592                     0.01%
 * 14451       28        516                0.999792402                     0.02%
 * 14452       28        516                0.999723222                     0.03%
 * 14453       28        516                0.999654051                     0.03%
 * 14454       28        516                0.99958489                      0.04%
 * 14455       28        516                0.999515738                     0.05%
 * 14456       28        516                0.999446597                     0.06%
 * 14457       28        516                0.999377464                     0.06%
 * 14458       28        516                0.999308341                     0.07%
 * 14459       28        516                0.999239228                     0.08%
 * 14460       29        499                1.000760719                     0.08%
 * 14461       29        499                1.000691515                     0.07%
 * 14462       29        499                1.000622321                     0.06%
 * 14463       29        499                1.000553136                     0.06%
 * 14464       29        499                1.00048396                      0.05%
 * 14465       29        499                1.000414794                     0.04%
 * 14466       29        499                1.000345638                     0.03%
 * 14467       29        499                1.000276491                     0.03%
 * 14468       29        499                1.000207354                     0.02%
 * 14469       29        499                1.000138227                     0.01%
 * 14470       29        499                1.000069109                     0.01%
 * 14471       29        499                1                            ** 0.00%
 * 14472       27        536                1                            ** 0.00%
 * 14473       27        536                0.999930906                     0.01%
 * 14474       27        536                0.999861821                     0.01%
 * 14475       28        517                1.000069085                     0.01%
 * 14476       28        517                1                            ** 0.00%
 * 14477       28        517                0.999930925                     0.01%
 * 14478       28        517                0.999861859                     0.01%
 * 14479       28        517                0.999792803                     0.02%
 * 14480       28        517                0.999723757                     0.03%
 * 14481       28        517                0.99965472                      0.03%
 * 14482       28        517                0.999585693                     0.04%
 * 14483       28        517                0.999516675                     0.05%
 * 14484       28        517                0.999447666                     0.06%
 * 14485       28        517                0.999378668                     0.06%
 * 14486       28        517                0.999309678                     0.07%
 * 14487       28        517                0.999240699                     0.08%
 * 14488       27        537                1.000759249                     0.08%
 * 14489       27        537                1.000690179                     0.07%
 * 14490       27        537                1.000621118                     0.06%
 * 14491       27        537                1.000552067                     0.06%
 * 14492       27        537                1.000483025                     0.05%
 * 14493       27        537                1.000413993                     0.04%
 * 14494       27        537                1.00034497                      0.03%
 * 14495       27        537                1.000275957                     0.03%
 * 14496       27        537                1.000206954                     0.02%
 * 14497       27        537                1.00013796                      0.01%
 * 14498       27        537                1.000068975                     0.01%
 * 14499       27        537                1                            ** 0.00%
 * 14500       29        500                1                            ** 0.00%
 *
 */


#endif /* SYS_CLOCK_MGR_H_ */

/**
 \}
 \}
 \}
 */
