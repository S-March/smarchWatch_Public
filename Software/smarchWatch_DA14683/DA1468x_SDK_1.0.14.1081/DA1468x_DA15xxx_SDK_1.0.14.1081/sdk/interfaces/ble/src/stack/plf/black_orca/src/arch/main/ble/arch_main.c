/**
 ****************************************************************************************
 *
 * @file arch_main.c
 *
 * @brief Main loop of the application.
 *
 * Copyright (C) RivieraWaves 2009-2012
 *
 * $Rev: 13481 $
 *
 ****************************************************************************************
 */

//0x050508030001fc0a0004700002200401020304
//0x050508030002fc0a0004700020200411121314

/*
 * INCLUDES
 ****************************************************************************************
 */
#ifdef CONFIG_USE_BLE
#include "ble_config.h"

#include <stdlib.h>
#include <stddef.h>     // standard definitions
#include <stdint.h>     // standard integer definition
#include <stdbool.h>    // boolean definition

#include "co_version.h"
#include "rwip_config.h"// RW SW configuration
#include "rwip.h"       // RW definitions
#include "arch.h"
#include "boot.h"       // boot definition
#include "rwip.h"       // BLE initialization
#include "em_map_ble.h"
#include "ke_mem.h"
#include "smpc.h"
#include "llc.h"
#if PLF_UART
#endif //PLF_UART
#include "nvds.h"       // NVDS initialization
#include "flash.h"      // Flash initialization
#if (BLE_EMB_PRESENT)
#include "rf.h"         // RF initialization
#endif // BLE_EMB_PRESENT

#if (BLE_HOST_PRESENT)
#include "rwble_hl.h"   // BLE HL definitions
#include "gapc.h"
#include "smpc.h"
#include "gattc.h"
#include "attc.h"
#include "atts.h"
#include "l2cc.h"
#endif //BLE_HOST_PRESENT

#if (TL_ITF)
#include "h4tl.h"
#endif //TL_ITF

#if (GTL_ITF)
#include "gtl.h"
#endif //GTL_ITF

#if (HCI_PRESENT)
#include "hci.h"             // HCI definition
#endif //HCI_PRESENT

#if (KE_SUPPORT)
#include "ke.h"              // kernel definition
#include "ke_env.h"          // ke_env
#include "ke_event.h"        // kernel event
#include "ke_timer.h"        // definitions for timer
#include "ke_mem.h"          // kernel memory manager
#endif //KE_SUPPORT

#if PLF_DEBUG
#include "dbg.h"        // For dbg_warning function
#endif //PLF_DEBUG


#include "em_map_ble_user.h"
#include "em_map_ble.h"

#include "lld_sleep.h"
#include "rwble.h"
#include "hw_rf.h"
#include "ad_rf.h"

#if (BLE_APP_PRESENT && BLE_PROX_REPORTER)
#include "app_proxr.h"
#endif

#if BLE_BATT_SERVER
//#include "app_batt.h"
#endif

#include "ke_event.h"

#include "pll_vcocal_lut.h"

#include "gapm_task.h"

#if (USE_BLE_SLEEP == 0)
#pragma message "BLE sleep is disabled!"
#endif

#if (dg_configUSE_HW_TRNG == 0)
#warning "HW RNG is disabled!"
#endif
#include "hw_trng.h"

#include "hw_cpm.h"
#include "sys_clock_mgr.h"
#include "sys_power_mgr.h"
#include "llm_util.h"
#include "ad_ble.h"
#ifndef BLE_PROD_TEST
#include "ad_crypto.h"
#include "hw_crypto.h"
#include "hw_ecc.h"
#include "hw_ecc_curves.h"
#endif
#include "sys_trng.h"

/**
 * @addtogroup DRIVERS
 * @{
 */
#if (dg_configUSE_LP_CLK == LP_CLK_RCX)
extern uint16_t rcx_clock_hz;
extern uint16_t rcx_tick_rate_hz;
#endif
extern uint32_t rcx_clock_period;
extern uint32_t ble_slot_duration_in_rcx;

#ifdef BLE_PROD_TEST
void lld_evt_deffered_elt_handler_custom(void);
#endif

/*
 * DEFINES
 ****************************************************************************************
 */

#if BLE_APP_PRESENT
# if !defined(CFG_PRINTF)
#  undef PROGRAM_ENABLE_UART
# else
#  define PROGRAM_ENABLE_UART
# endif
#endif // BLE_APP_PRESENT


#ifdef PROGRAM_ENABLE_UART
#include "uart.h"      // UART initialization
#endif

#define DUMMY_SIZE                      _EM_BLE_END

/*
 * STRUCTURE DEFINITIONS
 ****************************************************************************************
 */

/// Description of unloaded RAM area content
struct unloaded_area_tag
{
        uint32_t error;
};


/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

const struct gap_cfg_user_struct gap_cfg_user_var_struct = {
        .GAP_TMR_LECB_DISCONN_TIMEOUT_VAR = 0x0BB8,
        .GAP_APPEARANCE_VAR = 0x0000,
        .GAP_PPCP_CONN_INTV_MAX_VAR = 0x0064,
        .GAP_PPCP_CONN_INTV_MIN_VAR = 0x00c8,
        .GAP_PPCP_SLAVE_LATENCY_VAR = 0x0000,
        .GAP_PPCP_STO_MULT_VAR = 0x07d0,
        .GAP_TMR_LIM_ADV_TIMEOUT_VAR = 0x4650,
        .GAP_TMR_GEN_DISC_SCAN_VAR = 0x0300,
        .GAP_TMR_LIM_DISC_SCAN_VAR = 0x0300,
        .GAP_TMR_PRIV_ADDR_INT_VAR = 0x3A98,
        .GAP_TMR_CONN_PAUSE_CT_VAR = 0x0064,
        .GAP_TMR_CONN_PAUSE_PH_VAR = 0x01F4,
        .GAP_TMR_CONN_PARAM_TIMEOUT_VAR = 0x0BB8,
        .GAP_TMR_LECB_CONN_TIMEOUT_VAR = 0x0BB8,
        .GAP_TMR_SCAN_FAST_PERIOD_VAR = 0x0C00,
        .GAP_TMR_ADV_FAST_PERIOD_VAR = 0x0BB8,
        .GAP_LIM_DISC_SCAN_INT_VAR = 0x0012,
        .GAP_SCAN_FAST_INTV_VAR = 0x0030,
        .GAP_SCAN_FAST_WIND_VAR = 0x0030,
        .GAP_SCAN_SLOW_INTV1_VAR = 0x00CD,
        .GAP_SCAN_SLOW_INTV2_VAR = 0x019A,
        .GAP_SCAN_SLOW_WIND1_VAR = 0x0012,
        .GAP_SCAN_SLOW_WIND2_VAR = 0x0024,
        .GAP_ADV_FAST_INTV1_VAR = 0x0030,
        .GAP_ADV_FAST_INTV2_VAR = 0x0064,
        .GAP_ADV_SLOW_INTV_VAR = 0x00B0,
        .GAP_INIT_CONN_MIN_INTV_VAR = 0x0018,
        .GAP_INIT_CONN_MAX_INTV_VAR = 0x0028,
        .GAP_INQ_SCAN_INTV_VAR = 0x0012,
        .GAP_INQ_SCAN_WIND_VAR = 0x0012,
        .GAP_CONN_SUPERV_TIMEOUT_VAR = 0x07D0,
        .GAP_CONN_MIN_CE_VAR = 0x0000,
        .GAP_CONN_MAX_CE_VAR = 0xFFFF,
        .GAP_CONN_LATENCY_VAR = 0x0000,
        //.GAP_LE_MASK_VAR = 0x1F,
        .GAP_MAX_LE_MTU_VAR = defaultBLE_MAX_MTU_SIZE,
        .GAP_DEV_NAME_VAR = "RIVIERAWAVES-BLE",
        //.GAP_DEV_NAME_VAR ="DIALOG SEMICONDUCTOR",
};

/// Variable to hold RCX clock period value used in sleep entry calculations
__RETAINED uint32_t ble_rcx_clk_period_sleep;

/// Variable storing the reason of platform reset
static uint32_t reset_reason __RETAINED; /* = RESET_NO_ERROR */

/// Variable storing the FINE Timer correction value
#if dg_configBLACK_ORCA_IC_REV != BLACK_ORCA_IC_REV_A
uint32_t ble_finetim_corr __RETAINED;
#endif

#ifdef RAM_BUILD
/// Reserve space for the BLE ROM variables
__LTO_EXT
#if dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A
unsigned char ble_rom_vars[0x7FE0000 - BLE_VAR_ADDR] __attribute__((section("ble_variables"))) __attribute__((unused));
#else
unsigned char ble_rom_vars[BLE_VAR_SIZE] __attribute__((section("ble_variables"))) __attribute__((unused));
#endif
#else
/// Reserve space for Exchange Memory, this section is linked first in the section "exchange_mem_case"
volatile uint8 dummy[DUMMY_SIZE] __attribute__((section("exchange_mem_case1")));
#endif


extern bool rf_in_sleep;                        // set to '1' when the BLE is sleeping, else 0

void ble_init(uint32_t base);
bool rwip_check_wakeup_boundary(void);
bool rwip_check_wakeup_boundary_rcx(void);
bool rwip_check_wakeup_boundary_any(void);
uint32_t rwip_slot_2_lpcycles_any(uint32_t slot_cnt);
uint32_t lld_sleep_us_2_lpcycles_func_any(uint32_t us);
uint32_t lld_sleep_lpcycles_2_us_func_any(uint32_t lpcycles);

void ble_regs_push(void);
void ble_regs_pop(void);
void patch_rom_functions(void);

#if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE)
uint32_t retained_slp_duration __RETAINED;
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// Hooks based on previous version can be added in the rom_func_addr_table_var[] definition.
////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * LOCAL FUNCTION DECLARATIONS
 ****************************************************************************************
 */
__RETAINED_CODE static uint32_t rwip_slot_2_lpcycles_xtal(uint32_t slot_cnt)
{
        uint32_t lpcycles;

        // Sanity check: The number of slots should not be too high to avoid overflow
        ASSERT_WARNING(slot_cnt < 1000000);

        if (dg_configUSE_LP_CLK == LP_CLK_32000) {
                // Compute the low power clock cycles - case of a 32kHz clock
                lpcycles = slot_cnt * 20;
        }
        else if (dg_configUSE_LP_CLK == LP_CLK_32768) {
                // Compute the low power clock cycles - case of a 32.768kHz clock
                lpcycles = (slot_cnt << 11)/100;
        }

        // Corner case, Sleep duration is exactly on slot boundary so slot interrupt will not
        // be generated on expected slot (but next one).

        // So reduce little bit sleep duration in order to allow fine time compensation
        // Note compensation will be in range of [1 , 2[ lp cycles

        lpcycles--;

        return lpcycles;
}

__RETAINED_CODE static uint32_t rwip_slot_2_lpcycles_rcx(uint32_t slot_cnt)
{
        uint64_t lpcycles;

        lpcycles = (uint64_t)ble_slot_duration_in_rcx * (uint64_t)slot_cnt;
        lpcycles = (uint32_t)(lpcycles / 1000000);

        return (uint32_t)lpcycles;
}

__RETAINED_CODE uint32_t lld_sleep_lpcycles_2_us_func_xtal(uint32_t lpcycles)
{
    uint32_t us;

    if (dg_configUSE_LP_CLK == LP_CLK_32000) {
            // Compute the sleep duration in us - case of a 32kHz clock
            us = 31 * lpcycles + (lpcycles >> 2);
    } else {
            // Compute the sleep duration in us - case of a 32.768kHz clock
            us = 30 * lpcycles + (((lpcycles << 8) + (lpcycles << 3) + lpcycles) >> 9);
    }

    return (us);
}

__RETAINED_CODE uint32_t lld_sleep_lpcycles_2_us_func_rcx(uint32_t lpcycles)
{
        uint64_t res;
        uint32_t usec;

        res = (uint64_t)lpcycles * (uint64_t)ble_rcx_clk_period_sleep;
        res = res >> 20;

        usec = (uint32_t)res;

        return usec;
}

__RETAINED_CODE uint32_t lld_sleep_us_2_lpcycles_func_xtal(uint32_t us)
{
        uint32_t lpcycles;

        if (dg_configUSE_LP_CLK == LP_CLK_32000) {
                // Compute the sleep duration in us - case of a 32kHz clock
                lpcycles = (us * 32) / 1000;
        } else {
                // Compute the sleep duration in us - case of a 32.768kHz clock
                lpcycles = (us * 32768) / 1000000;
        }

        return lpcycles;
}

__RETAINED_CODE uint32_t lld_sleep_us_2_lpcycles_func_rcx(uint32_t us)
{
        uint32_t lpcycles;

        lpcycles = (uint32_t)(((uint64_t)us * rcx_clock_hz_acc) / 1000000) / RCX_ACCURACY_LEVEL;

        return lpcycles;
}

__RETAINED_CODE static void lld_sleep_compensate_core(uint32_t dur_us)
{
        uint32_t fintetime_correction;
        uint32_t slot_cnt;
        uint32_t usec_cnt;

#if dg_configBLACK_ORCA_IC_REV != BLACK_ORCA_IC_REV_A
        // Adjust for any shifts done after sleep...
        dur_us += ble_finetim_corr;
        // Reset for future use
        ble_finetim_corr = 0;
#endif

        // The correction values are then deduced from the sleep duration in us
        slot_cnt = dur_us / LLD_EVT_SLOT_DURATION;
        ble_basetimecntcorr_set(slot_cnt);

        /* If the sleep duration is a multiple of slot then fine timer correction is set to 0 else
         * it is set to the difference. Note that multiplication is much faster than division, so
         * the modulo operator is not used! */
        usec_cnt = dur_us - (slot_cnt * LLD_EVT_SLOT_DURATION);
        fintetime_correction = ((usec_cnt == 0) ? 0 : (LLD_EVT_SLOT_DURATION - usec_cnt));
        ble_finecntcorr_set((uint16_t)fintetime_correction);

        // Start the correction
        ble_deep_sleep_corr_en_setf(1);
}

__RETAINED_CODE uint32_t lld_sleep_lpcycles_2_us_sdk(uint32_t slp_period)
{
        uint32_t dur_us;

        if ((dg_configUSE_LP_CLK == LP_CLK_32000) || (dg_configUSE_LP_CLK == LP_CLK_32768)) {
                dur_us = lld_sleep_lpcycles_2_us_func_xtal(slp_period);
        } else if (dg_configUSE_LP_CLK == LP_CLK_RCX) {
                dur_us = lld_sleep_lpcycles_2_us_func_rcx(slp_period);
        } else {
                dur_us = lld_sleep_lpcycles_2_us_func_any(slp_period);
        }

        return dur_us;
}

__RETAINED_CODE uint32_t lld_sleep_us_2_lpcycles_sdk(uint32_t us)
{
        uint32_t lpcycles;

        if ((dg_configUSE_LP_CLK == LP_CLK_32000) || (dg_configUSE_LP_CLK == LP_CLK_32768)) {
                lpcycles = lld_sleep_us_2_lpcycles_func_xtal(us);
        } else if (dg_configUSE_LP_CLK == LP_CLK_RCX) {
                lpcycles = lld_sleep_us_2_lpcycles_func_rcx(us);
        } else {
                lpcycles = lld_sleep_us_2_lpcycles_func_any(us);
        }

        return lpcycles;
}

__RETAINED_CODE void lld_sleep_compensate_sdk(void)
{
        uint32_t dur_us;
        uint32_t slp_period;

        // Get the number of low power sleep period
        slp_period = ble_deepslstat_get();

        // Convert sleep duration into us
        dur_us = lld_sleep_lpcycles_2_us_sdk(slp_period);

        lld_sleep_compensate_core(dur_us);
}

/**
 ****************************************************************************************
 * @brief BLE diagnostic signals configuration
 ****************************************************************************************
 */
__RETAINED_CODE void ble_diagn_config(void)
{
#if (dg_configBLE_DIAGN_CONFIG == 1)
        BLE->BLE_DIAGCNTL_REG = 0x0;
        BLE->BLE_DIAGCNTL2_REG = 0xa0b1b1b1;
        BLE->BLE_DIAGCNTL3_REG = 0x14320000;
#elif (dg_configBLE_DIAGN_CONFIG == 2)
        BLE->BLE_DIAGCNTL_REG = 0x0;
        BLE->BLE_DIAGCNTL2_REG = 0xa5a58383;
        BLE->BLE_DIAGCNTL3_REG = 0x20100000;
#elif (dg_configBLE_DIAGN_CONFIG == 3)
        BLE->BLE_DIAGCNTL_REG = 0x0;
        BLE->BLE_DIAGCNTL2_REG = 0xa5a5a583;
        BLE->BLE_DIAGCNTL3_REG = 0x54100000;
#elif (dg_configBLE_DIAGN_CONFIG == 4)
        BLE->BLE_DIAGCNTL_REG = 0x0;
        BLE->BLE_DIAGCNTL2_REG = 0x83838383;
        BLE->BLE_DIAGCNTL3_REG = 0x56100000;
#elif (dg_configBLE_DIAGN_CONFIG == 5)
        BLE->BLE_DIAGCNTL_REG = 0x8383;
        BLE->BLE_DIAGCNTL2_REG = 0x0;
        BLE->BLE_DIAGCNTL3_REG = 0x10;
#endif

#if dg_configBLE_DIAGN_CONFIG == 5
        GPIO->P30_MODE_REG = HW_GPIO_MODE_OUTPUT | HW_GPIO_FUNC_BLE_DIAG;
        GPIO->P31_MODE_REG = HW_GPIO_MODE_OUTPUT | HW_GPIO_FUNC_BLE_DIAG;
#else
        /*
         * To use P2_2 & P1_1 USBPAD_REG[USBPAD_EN] must be set.
         * J6 should be also removed.
         */
        if ((BLE->BLE_DIAGCNTL_REG & 0x00800000) || (BLE->BLE_DIAGCNTL2_REG & 0x00000080)) {
                REG_SET_BIT(CRG_PER, USBPAD_REG, USBPAD_EN);
        }

        /*
         * Configure the MODE register for the desired GPIOs. Since this code is executed
         * from within ISR, the configuration is done by directly writing the specific
         * MODE registers instead of using the LLD, to avoid delays from FLASH execution.
         *
         * Note: P2_0 (ble_diag0) & P2_1 (ble_diag1) pins are used by XTAL32, so they
         * cannot be used for diagnostics when XTAL32 is used as LP clock.
         */
#if (dg_configUSE_LP_CLK == LP_CLK_RCX)
        if (BLE->BLE_DIAGCNTL_REG & 0x00000080) {
                GPIO->P20_MODE_REG = HW_GPIO_MODE_OUTPUT | HW_GPIO_FUNC_BLE_DIAG;
        }
        if (BLE->BLE_DIAGCNTL_REG & 0x00008000) {
                GPIO->P21_MODE_REG = HW_GPIO_MODE_OUTPUT | HW_GPIO_FUNC_BLE_DIAG;
        }
#endif
        if (BLE->BLE_DIAGCNTL_REG & 0x00800000) {
                GPIO->P22_MODE_REG = HW_GPIO_MODE_OUTPUT | HW_GPIO_FUNC_BLE_DIAG;
        }
        if (BLE->BLE_DIAGCNTL_REG & 0x80000000) {
                GPIO->P10_MODE_REG = HW_GPIO_MODE_OUTPUT | HW_GPIO_FUNC_BLE_DIAG;
        }
        if (BLE->BLE_DIAGCNTL2_REG & 0x00000080) {
                GPIO->P11_MODE_REG = HW_GPIO_MODE_OUTPUT | HW_GPIO_FUNC_BLE_DIAG;
        }
        if (BLE->BLE_DIAGCNTL2_REG & 0x00008000) {
                GPIO->P12_MODE_REG = HW_GPIO_MODE_OUTPUT | HW_GPIO_FUNC_BLE_DIAG;
        }
        if (BLE->BLE_DIAGCNTL2_REG & 0x00800000) {
                GPIO->P13_MODE_REG = HW_GPIO_MODE_OUTPUT | HW_GPIO_FUNC_BLE_DIAG;
        }
        if (BLE->BLE_DIAGCNTL2_REG & 0x80000000) {
                GPIO->P23_MODE_REG = HW_GPIO_MODE_OUTPUT | HW_GPIO_FUNC_BLE_DIAG;
        }
#endif
}

/**
 ****************************************************************************************
 * @brief Initialization of ble core
 *
 ****************************************************************************************
 */
void init_pwr_and_clk_ble(void)
{
        uint32_t reg_local;
        /*
         * Power up BLE core & reset BLE Timers
         */
        GLOBAL_INT_DISABLE();

	hw_rf_request_on(true);

	reg_local = CRG_TOP->PMU_CTRL_REG;
        REG_CLR_FIELD(CRG_TOP, PMU_CTRL_REG, BLE_SLEEP, reg_local);

        if ((dg_configUSE_BOD == 1) && ((dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A)
                || ((dg_configUSE_AUTO_CHIP_DETECTION == 1) && (CHIP_IS_AE)))) {
                hw_cpm_deactivate_bod_protection();
        }

        CRG_TOP->PMU_CTRL_REG = reg_local;

        if ((dg_configUSE_BOD == 1) && ((dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A)
                || ((dg_configUSE_AUTO_CHIP_DETECTION == 1) && (CHIP_IS_AE)))) {
                hw_cpm_delay_usec(30);
                hw_cpm_activate_bod_protection();
        }

        reg_local = CRG_TOP->CLK_RADIO_REG;
        REG_SET_FIELD(CRG_TOP, CLK_RADIO_REG, BLE_ENABLE, reg_local, 1);
        REG_CLR_FIELD(CRG_TOP, CLK_RADIO_REG, BLE_DIV, reg_local);
        CRG_TOP->CLK_RADIO_REG = reg_local;

        GLOBAL_INT_RESTORE();

        while (!REG_GETF(CRG_TOP, SYS_STAT_REG, BLE_IS_UP)); // Wait for the BLE to wake up

        if (BLE->BLE_DEEPSLCNTL_REG & REG_MSK(BLE, BLE_DEEPSLCNTL_REG, DEEP_SLEEP_STAT)) {
                REG_SET_BIT(BLE, BLE_DEEPSLCNTL_REG, SOFT_WAKEUP_REQ);
                __NOP();
                __NOP();
                __NOP();
                while (BLE->BLE_DEEPSLCNTL_REG & REG_MSK(BLE, BLE_DEEPSLCNTL_REG, DEEP_SLEEP_STAT)) {
                        __NOP();
                }
        }

        // Reset the timing generator
        reg_local = BLE->BLE_RWBLECNTL_REG;
        REG_SET_FIELD(BLE, BLE_RWBLECNTL_REG, MASTER_SOFT_RST, reg_local, 1);
        REG_SET_FIELD(BLE, BLE_RWBLECNTL_REG, MASTER_TGSOFT_RST, reg_local, 1);
        BLE->BLE_RWBLECNTL_REG = reg_local;
        while (ble_master_tgsoft_rst_getf()) {}

        GLOBAL_INT_DISABLE();

        REG_SET_BIT(CRG_TOP, CLK_RADIO_REG, BLE_LP_RESET);      // Apply HW reset to BLE_Timers

        while (!REG_GETF(CRG_TOP, SYS_STAT_REG, RAD_IS_UP)); // Wait for the radio to wake up

        REG_CLR_BIT(CRG_TOP, CLK_RADIO_REG, BLE_LP_RESET);

        GLOBAL_INT_RESTORE();

        /*
         * Just make sure that BLE core is stopped (if already running)
         */
        while (!REG_GETF(CRG_TOP, SYS_STAT_REG, BLE_IS_UP)) {
                __NOP();
        }
        REG_CLR_BIT(BLE, BLE_RWBLECNTL_REG, RWBLE_EN);

        /*
         * Since BLE is stopped (and powered), set CLK_SEL
         */
        reg_local = BLE->BLE_CNTL2_REG;
        REG_SET_FIELD(BLE, BLE_CNTL2_REG, BLE_CLK_SEL, reg_local, 16);

        REG_SET_FIELD(BLE, BLE_CNTL2_REG, BLE_RSSI_SEL, reg_local, 1);
        BLE->BLE_CNTL2_REG = reg_local;

        /*
         * Set spi interface to software
         */
        // no BB_ONLY mode in 680
}

/*
 * EXPORTED FUNCTION DEFINITIONS
 ****************************************************************************************
 */
void conditionally_run_radio_cals(void)
{
}


/*
 * MAIN FUNCTION
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief BLE main function.
 *
 * This function is called right after the booting process has completed.
 ****************************************************************************************
 */
void ble_stack_init(void)
{
#ifndef RAM_BUILD
        platform_initialization();
        _ble_base = (unsigned int) dummy;
#endif

        /*
         * Apply ROM patches
         */
        patch_rom_functions();

        init_pwr_and_clk_ble();

        REG_CLR_BIT(BLE, BLE_CNTL2_REG, SW_RPL_SPI);

#ifndef RAM_BUILD
        /* Don't remove next line otherwise dummy[0] could be optimized away. The dummy array
         * is intended to reserve the needed Exchange Memory space in the retention memory.
         */
        dummy[0] = dummy[0];
#endif

        /*
         ************************************************************************************
         * BLE initialization
         ************************************************************************************
         */

#ifdef UNCALIBRATED_AT_FAB
        GPIO->RF_LNA_CTRL1_REG = 0x24E;
        GPIO->RF_LNA_CTRL2_REG = 0x26;
        GPIO->RF_LNA_CTRL3_REG = 0x7;
        GPIO->RF_REF_OSC_REG = 0x29AC;
        GPIO->RF_RSSI_COMP_CTRL_REG = 0x7777;
        GPIO->RF_VCO_CTRL_REG = 0x1;
#endif

        ble_init(EM_BASE_ADDR);

#if (dg_configBLE_DIAGN_CONFIG > 0)
        ble_diagn_config();
#endif /* (dg_configBLE_DIAGN_CONFIG > 0) */

#ifdef RADIO_RIPPLE
        /* Set spi to HW (Ble)
         * Necessary: So from this point the BLE HW can generate spi burst iso SW
         * SPI BURSTS are necessary for the radio TX and RX burst, done by hardware
         * beause of the accurate desired timing
         */
        REG_SET_BIT(BLE, BLE_CNTL2_REG, SW_RPL_SPI);
#endif

        //Enable BLE core
        REG_SET_BIT(BLE, BLE_RWBLECNTL_REG, RWBLE_EN);

#if RW_BLE_SUPPORT && HCIC_ITF

        // If FW initializes due to FW reset, send the message to Host
//        if (error != RESET_NO_ERROR) {
//                rwble_send_message(error);
//        }
#endif


        if (BLE_USE_TIMING_DEBUG == 1) {
#define BLE_BLE_CNTL2_REG_DIAG5_Pos (5)
                REG_SET_BIT(BLE, BLE_CNTL2_REG, DIAG5);
#undef BLE_BLE_CNTL2_REG_DIAG5_Pos
        }

        /*
         ************************************************************************************
         * Sleep mode initializations
         ************************************************************************************
         */
        if (USE_BLE_SLEEP == 1) {
                rwip_env.sleep_enable = true;
        }
        rwip_env.ext_wakeup_enable = true;

        /*
         ************************************************************************************
         * PLL-LUT and MGC_KMODALPHA
         ************************************************************************************
         */

#ifdef BLE_PROD_TEST
        ke_event_callback_set(KE_EVENT_BLE_EVT_DEFER, &lld_evt_deffered_elt_handler_custom);
#endif
}

/// @} DRIVERS
#ifdef RAM_BUILD
void platform_reset_sdk(uint32_t error)
{
        reset_reason = error;

        ASSERT_ERROR(0);
}
#endif


/*********************************************************************************
 *** WAKEUP_LP_INT ISR
 ***/
#if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE)
__RETAINED uint32_t ble_slp_misses_cnt;
__RETAINED uint32_t ble_slp_misses_max;
__RETAINED uint32_t ble_wakeups_cnt;
#endif

__RETAINED_CODE void ble_lp_isr(void)
{
        /*
         * Since XTAL 16MHz is activated, power-up the Radio Subsystem (including BLE)
         *
         * Note that BLE core clock is masked in order to handle the case where RADIO_PD does not
         * get into power down state. The BLE clock should be active only as long as system is
         * running at XTAL 16MHz. Also the BLE clock should be enabled before powering up the RADIO
         * Power Domain !
         */
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, CLK_RADIO_REG, BLE_ENABLE);  // BLE clock enable
        REG_CLR_BIT(CRG_TOP, PMU_CTRL_REG, BLE_SLEEP);
        GLOBAL_INT_RESTORE();
        while (!REG_GETF(CRG_TOP, SYS_STAT_REG, BLE_IS_UP)) {}

        GLOBAL_INT_DISABLE();
        hw_rf_request_on(true);
        GLOBAL_INT_RESTORE();

        /*
         * BLE is up. The register status can be restored.
         */
        ble_regs_pop();

#if (dg_configBLE_DIAGN_CONFIG > 0)
        ble_diagn_config();
#endif /* (dg_configBLE_DIAGN_CONFIG > 0) */

        /*
         * Check if BLE_SLP_IRQ has already asserted. In this case, we are delayed.
         */
#if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE)
        ble_wakeups_cnt++;

        if (REG_GETF(BLE, BLE_INTSTAT_REG, SLPINTSTAT)) {
                ble_slp_misses_cnt++;

                ASSERT_WARNING(ble_slp_misses_cnt < (BLE_MAX_MISSES_ALLOWED + 1));
        }

        if (ble_wakeups_cnt == BLE_WAKEUP_MONITOR_PERIOD) {
                if (ble_slp_misses_cnt > ble_slp_misses_max) {
                        ble_slp_misses_max = ble_slp_misses_cnt;
                }
                ble_wakeups_cnt = 0;
                ble_slp_misses_cnt = 0;
        }
#endif

        DBG_SET_LOW(BLE_USE_TIMING_DEBUG, CPMDBG_BLE_IRQ);

        /*
         * Wait for BLE_SLP_IRQ to be asserted.
         */
        while (!REG_GETF(BLE, BLE_INTSTAT_REG, SLPINTSTAT)) {}
}


/**
 * @brief       Wake the BLE core via an external request.
 *
 * @details     If the BLE core is sleeping (permanently or not and external wake-up is enabled)
 *              then this function wakes it up.
 *
 * @attention   Return to normal mode if the BLE core was in permanent sleep.
 *
 * @return      bool
 *
 * @retval      The status of the requested operation.
 *              <ul>
 *                  <li> false, if the BLE core is not sleeping
 *                  <li> true, if the BLE core was woken-up successfully
 *              </ul>
 *
 */
bool ble_force_wakeup(void)
{
        bool retval = false;

        GLOBAL_INT_DISABLE();

        // If the BLE is sleeping, wake it up!
        if (REG_GETF(CRG_TOP, CLK_RADIO_REG, BLE_ENABLE) == 0) {                   // BLE clock is off
                if (REG_GETF(GPREG, GP_CONTROL_REG, BLE_WAKEUP_REQ) == 0) {        // No previous wake-up req
                        REG_SET_BIT(GPREG, GP_CONTROL_REG, BLE_WAKEUP_REQ);
                        pm_resource_sleeps_until(PM_BLE_ID, 4);                    // 3-4 LP cycles are needed
                        retval = true;
                }
        }

        GLOBAL_INT_RESTORE();

        return retval;
}


#define CUSTOM_INIT(pos, func, cast)            func = (cast)rom_func_addr_table_var[pos];
#define ODD_TO_NEXT_EVEN(x) ((x) & 0x01 ? x+1 : x)

#include "em_map_ble.h"
#include "rwble_config.h"
extern unsigned int REG_BLE_EM_TX_BUFFER_SIZE;
extern unsigned int REG_BLE_EM_RX_BUFFER_SIZE;

extern uint8_t ble_duplicate_filter_max;
extern bool ble_duplicate_filter_found;
extern uint8_t llm_resolving_list_max;
extern bool length_exchange_needed;

#if (RWBLE_SW_VERSION_MINOR >= 1)
/// HCI command descriptor structure
struct hci_cmd_desc_tab_ref
{
    /// OpCode Group Field (OGF)
    uint8_t ogf;

    /// Number of commands supported in this group
    uint16_t nb_cmds;

    /// Command descriptor table
    const struct hci_cmd_desc_tag* cmd_desc_tab;
};

/// HCI command descriptor structure
struct hci_cmd_desc_tag
{
    /// Command opcode with flags indicating if a special packing or unpacking is needed
    uint16_t opcode;

    /// Destination field (used to find the internal destination task)
    uint8_t dest_field;

    #if (TL_ITF)
    /// Flag indicating if a special packing/unpacking is needed
    uint8_t  special_pack_settings;

    /// Parameters format string (or special unpacker)
    void* par_fmt;

    /// Return parameters format string (or special unpacker)
    void* ret_par_fmt;
    #endif //(TL_ITF)
};

extern struct hci_cmd_desc_tab_ref hci_cmd_desc_root_tab;
extern struct hci_cmd_desc_tab_ref rom_hci_cmd_desc_root_tab;

int dia_rand_func(void)
{
    return rand();

}

void dia_srand_func (unsigned int seed)
{
        srand(seed);
}
#endif /* (RWBLE_SW_VERSION_MINOR >= 1) */

void ble_platform_initialization(void)
{
        _ble_base = BLE_VAR_ADDR;

        REG_BLE_EM_RX_BUFFER_SIZE = ODD_TO_NEXT_EVEN(dg_configBLE_DATA_LENGTH_RX_MAX + 11);
        REG_BLE_EM_TX_BUFFER_SIZE = ODD_TO_NEXT_EVEN(dg_configBLE_DATA_LENGTH_TX_MAX + 11);

        use_h4tl = 0;   // 0 = GTL auto
        gap_cfg_user = (struct gap_cfg_user_struct*)&gap_cfg_user_var_struct;
        rom_func_addr_table = (uint32_t *)&rom_func_addr_table_var[0];
        rom_cfg_table = (uint32_t *)&rom_cfg_table_var[0];
        ble_duplicate_filter_max = dg_configBLE_DUPLICATE_FILTER_MAX;
        ble_duplicate_filter_found = true;      //What will be reported if max is exceeded, true means the extra devices are considered to be in the list and will not be reported
        llm_resolving_list_max = LLM_RESOLVING_LIST_MAX;        //this is the maximum spec value but it could require large amounts of heap 255*50=12750

#if (RWBLE_SW_VERSION_MINOR >= 1)
        if (rom_cfg_table[nb_links_user_pos] == 1)
                BLE_TX_DESC_DATA_USER = 5;
        else
                BLE_TX_DESC_DATA_USER = rom_cfg_table[nb_links_user_pos] * 3;

        BLE_TX_DESC_CNTL_USER = rom_cfg_table[nb_links_user_pos];

        LLM_LE_ADV_DUMMY_IDX = BLE_TX_DESC_DATA + BLE_TX_DESC_CNTL - 1;
        LLM_LE_SCAN_CON_REQ_ADV_DIR_IDX =LLM_LE_ADV_DUMMY_IDX +1;
        LLM_LE_SCAN_RSP_IDX=LLM_LE_SCAN_CON_REQ_ADV_DIR_IDX+1;
        LLM_LE_ADV_IDX=LLM_LE_SCAN_RSP_IDX+1;

        memcpy(&hci_cmd_desc_root_tab, &rom_hci_cmd_desc_root_tab,48/*sizeof(hci_cmd_desc_root_tab)*/);

#endif /* (RWBLE_SW_VERSION_MINOR >= 1) */

        /* Control whether LL_LENGTH_REQ will be sent upon established connections */
        length_exchange_needed = dg_configBLE_DATA_LENGTH_REQ_UPON_CONN;

        CUSTOM_INIT(custom_pti_set_func_pos, custom_pti_set, unsigned char (*)(void));

}

/**
 * @brief       Check if the BLE core can enter sleep and, if so, enter sleep.
 *
 * @return      int
 *
 * @retval      The status of the requested operation.
 *              <ul>
 *                  <li> 0, if the BLE core cannot sleep
 *                  <li> 1, if the BLE core was put to sleep
 *                  <li> other, if the BLE core has to stay active but the caller may block
 *              </ul>
 *
 */

uint32_t logged_sleep_duration __RETAINED;
static int ble_rwip_sleep(bool forever, uint32_t *sleep_duration)
{
        uint32_t sleep_duration_in_lp_cycles;
        uint32_t dummy __attribute__((unused)) = MAX_SLEEP_DURATION_PERIODIC_WAKEUP_DEF;
        uint32_t wup_latency;
        uint32_t rem_time __attribute__((unused));
        int result = 0;

        DBG_SWDIAG(SLEEP, ALGO, 0);

        if (forever) {
                *sleep_duration = -1;
        } else {
                *sleep_duration = MAX_SLEEP_DURATION_EXTERNAL_WAKEUP_DEF;
        }

        do
        {
                /************************************************************************
                 **************            CHECK KERNEL EVENTS             **************
                 ************************************************************************/
                // Check if some kernel processing is ongoing
                if (!ke_sleep_check()) {
                        break;
                }

                result = -1;

                DBG_SWDIAG(SLEEP, ALGO, 1);
#if (DEEP_SLEEP)
                /************************************************************************
                 **************             CHECK ENABLE FLAG              **************
                 ************************************************************************/
                // Check sleep enable flag
                if (!rwip_env.sleep_enable) {
                        break;
                }

                /************************************************************************
                 **************              CHECK RW FLAGS                **************
                 ************************************************************************/
                // First check if no pending procedure prevent from going to sleep
                if (rwip_env.prevent_sleep != 0) {
                        break;
                }

                DBG_SWDIAG(SLEEP, ALGO, 2);

                /************************************************************************
                 **************           CHECK EXT WAKEUP FLAG            **************
                 ************************************************************************/
                /* If external wakeup enable, sleep duration can be set to maximum, otherwise
                 * system should be woken-up periodically to poll incoming packets from HCI */
                if ((BLE_APP_PRESENT == 0) && !rwip_env.ext_wakeup_enable) {
                        *sleep_duration = rom_cfg_table_var[max_sleep_duration_periodic_wakeup_pos];
                }

                /************************************************************************
                 **************            CHECK KERNEL TIMERS             **************
                 ************************************************************************/
                // If there's any timer pending, compute the time to wake-up to serve it
                if (ke_env.queue_timer.first != NULL) {
                        *sleep_duration = (BLE_GROSSTARGET_MASK >> 1); /* KE_TIMER_DELAY_MAX */
                }

                /************************************************************************
                 **************           SET WUP_LATENCY (RCX)            **************
                 * The BLE_WUP_LATENCY is written to a local variable here. This is     *
                 * needed when the RCX is used, because the calculation of the latency  *
                 * comes from a function and is not a fixed number of LP cycles.        *
                 ************************************************************************/
                wup_latency = BLE_WUP_LATENCY;

#if dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A
                /************************************************************************
                 **************       DUMMY CHECKS FOR TIMERS AND EA       **************
                 *  to make sure that sleep will be probably allowed and continue with  *
                 *   waiting for the right time in the current slot (power consuming)   *
                 ************************************************************************/
                if (!ke_timer_sleep_check(&dummy, rwip_env.wakeup_delay)) {
                        break;
                }

                if (!ea_sleep_check(&dummy, rwip_env.wakeup_delay)) {
                        break;
                }

                /************************************************************************
                 * Wait until there's enough time for the SLP ISR to restore clocks when
                 * the chip wakes up. Lower AMBA clocks while waiting, if possible, to
                 * reduce the power consumption.
                 *
                 * The "window" inside the BLE slot is calculated such that there is enough
                 * time for the SLP handler to program the clock compensation before the
                 * slot, that the system went to sleep, ends. More specifically,
                 *   window >= <clock restoration time> + <sleep preparation time> +
                 *            <ble core sleep entry time> + <SLP processing>
                 * where (assuming 16MHz is used):
                 *   <clock restoration time>: 0usec
                 *   <sleep period calc time>: 23 - 30usec (RCX) --- ~21usec (XTAL)
                 *   <sleep preparation time>: ~60 - 70usec (RCX) --- ~40 - 45usec (XTAL)
                 *   <ble core sleep entry time>: 2 to 4 LP cycles (61 - 122usec for 32768)
                 *   <SLP processing>: ~85usec (RCX) --- ~55 - 60usec (XTAL)
                 *
                 * So, when the 32768 is used, the "window" must be larger than:
                 *   0 + 61 +  61 + 55 = 177 (minimum)
                 *   0 + 66 + 122 + 60 = 248 (maximum).
                 * Thus, a window of [624, 300] is ok!
                 *
                 * When the RCX is used, the calculation becomes (assuming an RCX period
                 * of 95usec):
                 *   0 + 80 + 190 = 270 (minimum) (or, 80 + 2 * RCX_period)
                 *   0 + 100 + 380 = 480 (maximum) (or, 100 + 4 * RCX_period)
                 * The goal is to have completed the sleep period calculation within the
                 * current slot and put the BLE core to sleep at the next one.
                 *
                 * Thus, the window, which will be used, should guarantee that there will be enough
                 * time for the SLP ISR by ensuring that the actual sleep entry will happen at
                 * the first half of the next slot. It is not yet clear that, although that
                 * the sleep duration calculations are executed in the current slot while
                 * the actual sleep entry happens at the next one, the calculated sleep
                 * duration does not have to be corrected (reduced by 1) before applied to
                 * the hardware!
                 ************************************************************************/
                DBG_SET_HIGH(BLE_USE_TIMING_DEBUG, CPMDBG_BLE_SLEEP_ENTRY);
                if ((dg_configUSE_LP_CLK == LP_CLK_32000)
                                || (dg_configUSE_LP_CLK == LP_CLK_32768)) {
                        if (!rwip_check_wakeup_boundary()) {
                                while (!rwip_check_wakeup_boundary()) {}
                        }
                } else if (dg_configUSE_LP_CLK == LP_CLK_RCX) {
                        if (!rwip_check_wakeup_boundary_rcx()) {
                                while (!rwip_check_wakeup_boundary_rcx()) {}
                        }
                } else { /* LP_CLK_ANY */
                        if (!rwip_check_wakeup_boundary_any()) {
                                while (!rwip_check_wakeup_boundary_any()) {}
                        }
                }
                DBG_SET_LOW(BLE_USE_TIMING_DEBUG, CPMDBG_BLE_SLEEP_ENTRY);
#else
                /* If the remaining time in the slot is less than 312 then the sleep time is
                 * calculated using the next slot as reference. A problem could occur if we were
                 * "close" to 312, so for example the ke_timer_sleep_check() was called for this
                 * slot but the ea_sleep_check() was called when the remaining time was less than
                 * 312, so it would be called for the next slot. This could result into waking up 1
                 * slot earlier without causing any other serious problems (i.e. miss the event).
                 *
                 * So, no special provision is required assuming that the overall sleep programming
                 * delay (starting from this point and until rwip_rf.sleep() is called) is less than
                 * 312usec, which is true.
                 */
#endif
                DBG_SET_HIGH(BLE_USE_TIMING_DEBUG, CPMDBG_BLE_SLEEP_ENTRY);

                // >>> Start of sleep entry delay measurement <<< //

                // Compute the duration up to the next software timer expires
                if (!ke_timer_sleep_check(sleep_duration, rwip_env.wakeup_delay)) {
                        break;
                }

                DBG_SWDIAG(SLEEP, ALGO, 3);

                /************************************************************************
                 **************                 CHECK EA                   **************
                 ************************************************************************/
                if (!ea_sleep_check(sleep_duration, rwip_env.wakeup_delay)) {
                        break;
                }

                DBG_SWDIAG(SLEEP, ALGO, 4);

#if (TL_ITF)
                /************************************************************************
                 **************                 CHECK TL                   **************
                 ************************************************************************/
                // Try to switch off TL
                if (!h4tl_stop()) {
                        break;
                }
#endif //TL_ITF

#if (GTL_ITF)
                /************************************************************************
                 **************                 CHECK TL                   **************
                 ************************************************************************/
                // Try to switch off Transport Layer
                if (!gtl_enter_sleep()) {
                        break;
                }
#endif //GTL_ITF

                DBG_SWDIAG(SLEEP, ALGO, 5);

                DBG_SET_LOW(BLE_USE_TIMING_DEBUG, CPMDBG_BLE_SLEEP_ENTRY);
                DBG_SET_HIGH(BLE_USE_TIMING_DEBUG, CPMDBG_BLE_SLEEP_ENTRY);

                // Sleep can be enabled

                logged_sleep_duration = *sleep_duration;

                /************************************************************************
                 **************          PROGRAM CORE DEEP SLEEP           **************
                 ************************************************************************/
                // Prepare BLE_ENBPRESET_REG for next sleep cycle
                BLE->BLE_ENBPRESET_REG =  (wup_latency << 21)           /*BITFLD_TWEXT*/
                                        | (wup_latency << 10)           /*BITFLD_TWIRQ_SET*/
                                        | (1 << 0);                     /*BITFLD_TWIRQ_RESET*/

                // Put the BLE core into sleep
                if (*sleep_duration == (uint32_t) -1) {
                        // Sleep indefinitely (~36hours with 32KHz LP clock, ~113hours with 10.5Khz RCX)
                        sleep_duration_in_lp_cycles = -1;
                } else {
                        if ((dg_configUSE_LP_CLK == LP_CLK_32768) || (dg_configUSE_LP_CLK == LP_CLK_32000)) {
                                sleep_duration_in_lp_cycles = rwip_slot_2_lpcycles_xtal(*sleep_duration);
                        } else if (dg_configUSE_LP_CLK == LP_CLK_RCX) {
                                sleep_duration_in_lp_cycles = rwip_slot_2_lpcycles_rcx(*sleep_duration);
                        } else { /* LP_CLK_ANY */
                                sleep_duration_in_lp_cycles = rwip_slot_2_lpcycles_any(*sleep_duration);
                        }
                }
                lld_sleep_enter(sleep_duration_in_lp_cycles, rwip_env.ext_wakeup_enable);

                DBG_SWDIAG(SLEEP, SLEEP, 1);

                DBG_SET_LOW(BLE_USE_TIMING_DEBUG, CPMDBG_BLE_SLEEP_ENTRY);
                DBG_SET_HIGH(BLE_USE_TIMING_DEBUG, CPMDBG_BLE_SLEEP_ENTRY);

                /************************************************************************
                 **************               SWITCH OFF RF                **************
                 ************************************************************************/
                rwip_rf.sleep();
                // >>> End of sleep entry delay measurement <<< //

                DBG_SET_LOW(BLE_USE_TIMING_DEBUG, CPMDBG_BLE_SLEEP_ENTRY);
                DBG_SET_HIGH(BLE_USE_TIMING_DEBUG, CPMDBG_BLE_SLEEP_ENTRY);

                result = 1;

#if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE)
                retained_slp_duration = sleep_duration_in_lp_cycles;
#endif
                *sleep_duration = sleep_duration_in_lp_cycles - wup_latency;

                while(!ble_deep_sleep_stat_getf());                             // 2 - 4 LP cycles
                DBG_SET_LOW(BLE_USE_TIMING_DEBUG, CPMDBG_BLE_SLEEP_ENTRY);

                if (*sleep_duration != 0) {
                        pm_resource_sleeps_until(PM_BLE_ID, *sleep_duration);
                }

                while ( !REG_GETF(BLE, BLE_CNTL2_REG, RADIO_PWRDN_ALLOW) ) {}   // 1 LP cycle
                
                REG_SETF(BLE, BLE_CNTL2_REG, MON_LP_CLK, 0); // Clear LP edge flag
                
#if dg_configBLACK_ORCA_IC_REV != BLACK_ORCA_IC_REV_A
                /* The time needed for the rwble_isr() to complete the clock compensation
                 * initialization is 52usec for XTAL32 and 55usec for RCX. Taking into account the
                 * ISR entry delay (which should be minimal anyway), the threshold is set to 60usec.
                 */
                rem_time = GPREG->BLE_FINECNT_SAMP_REG;

                if (rem_time < 60) {
                        ble_finetim_corr = 60 - rem_time;
                        GPREG->BLE_FINECNT_SAMP_REG = 60;
                }
#endif // dg_configBLACK_ORCA_IC_REV != BLACK_ORCA_IC_REV_A
#endif // DEEP_SLEEP
        } while(0);

        return result;
}

/**
 * @brief       Check if the BLE stack has pending actions.
 *
 * @return      bool
 *
 * @retval      The status of the requested operation.
 *              <ul>
 *                  <li> false, if the BLE stack has pending actions
 *                  <li> true, if the BLE stack has finished
 *              </ul>
 *
 */
bool ble_block(void)
{
        bool result = false;

        /************************************************************************
         **************            CHECK KERNEL EVENTS             **************
         ************************************************************************/
        // Check if some kernel processing is ongoing
        if (ke_sleep_check()) {
                result = true;
        }

        return result;
}

/**
 * @brief Puts BLE to sleep
 *
 * @param[in] forever True to put the BLE to permanent sleep, else false.
 * @param[out] wakeup_time The sleep duration in LP cycles.
 *
 * @return      int
 *
 * @retval      The status of the requested operation.
 *              <ul>
 *                  <li> 0, if the BLE core cannot sleep
 *                  <li> 1, if the BLE core was put to sleep
 *                  <li> other, if the BLE core has to stay active but the caller may block
 *              </ul>
 *
 */
int ble_sleep(bool forever, uint32_t *sleep_duration_in_lp_cycles)
{
        int ret = 0;

        if (!rf_in_sleep) {
                ret = ble_rwip_sleep(forever, sleep_duration_in_lp_cycles);
                if (ret == 1) {
                        if (dg_configUSE_LP_CLK == LP_CLK_RCX) {
                                ble_rcx_clk_period_sleep = rcx_clock_period;
                        }

                        ble_regs_push();        // Push the BLE ret.vars to the retention memory
                        
                        while ( !REG_GETF(BLE, BLE_CNTL2_REG, MON_LP_CLK) ) {}   // Wait for LP rising edge
                                                
                        GLOBAL_INT_DISABLE();
                        REG_SET_BIT(CRG_TOP, PMU_CTRL_REG, BLE_SLEEP);
                        GLOBAL_INT_RESTORE();
                        while (!REG_GETF(CRG_TOP, SYS_STAT_REG, BLE_IS_DOWN));

                        GLOBAL_INT_DISABLE();
                        REG_CLR_BIT(CRG_TOP, CLK_RADIO_REG, BLE_ENABLE);
                        GLOBAL_INT_RESTORE();

                        // The BLE interrupts have been cleared. Clear them in the NVIC as well.
                        NVIC_ClearPendingIRQ(BLE_GEN_IRQn);

                        ret = true;
                } else {
                        // BLE stays active
                }
        } else {
                ASSERT_WARNING(!rf_in_sleep);   // The BLE is already sleeping...
        }

        return ret;
}

enum {
        LLM_P256_STATE_ACQ_ECC = LLM_P256_STATE_PKMULT + 1,
        LLM_P256_STATE_CHECK_PXY,
        LLM_P256_STATE_CHECK_POINT_ON_CURVE,
        LLM_P256_STATE_PKMULT_WAIT,
        LLM_P256_STATE_REL_ECC,
};

typedef struct ble_ecc_op_t {
        bool succeed;
        bool generates_req;
        bool ongoing;
} ble_ecc_op_t;

void crypto_init_func(void)
{

}

#ifndef BLE_PROD_TEST

static volatile ble_ecc_op_t ble_ecc_op;

void ble_ecc_cb(unsigned int status)
{
        status &= ~HW_ECC_STATUS_BUSY;

        ble_ecc_op.succeed = !status;
        ble_ecc_op.ongoing = false;

        if (ble_ecc_op.generates_req) {
                struct llm_p256_req *req = KE_MSG_ALLOC(LLM_P256_REQ, TASK_LLM, TASK_LLM, llm_p256_req);
                ke_msg_send(req);

                /*
                 * Wakeup BLE adapter to process the message
                 */
                ad_ble_notify_gen_irq();
        }
}
#endif /* !BLE_PROD_TEST */

uint8_t llm_create_p256_key_sdk(uint8_t state, uint8_t *A, uint8_t *priv)
{
#ifndef BLE_PROD_TEST
        // Allocate the message for the response
        struct llm_p256_req *req = KE_MSG_ALLOC(LLM_P256_REQ, TASK_LLM, TASK_LLM, llm_p256_req);

        switch (state) {
        case LLM_P256_STATE_IDLE:
                break;
        case LLM_P256_STATE_TRNG:
                memcpy(&req->p256_data[0],              (uint8_t *) hw_ecc_p256_Gx, ECDH_KEY_LEN);
                memcpy(&req->p256_data[ECDH_KEY_LEN],   (uint8_t *) hw_ecc_p256_Gy, ECDH_KEY_LEN);
                break;
        case LLM_P256_STATE_PKMULT:
                memcpy(&req->p256_data, A, ECDH_KEY_LEN * 2);
                break;
        }

        // Send the message
        ke_msg_send(req);

        return CO_ERROR_NO_ERROR;
#else
        // Allocate the message for the response
        struct llm_p256_req *req = KE_MSG_ALLOC(LLM_P256_REQ, TASK_LLM, TASK_LLM, llm_p256_req);

        // Send the message
        ke_msg_send(req);

        return CO_ERROR_NO_ERROR;
#endif /* !BLE_PROD_TEST */
}

int llm_p256_req_handler_sdk(ke_msg_id_t const msgid,
        struct llm_p256_req const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
#ifndef BLE_PROD_TEST
        struct ke_msg *msg = (struct ke_msg *)co_list_pick(&llm_le_env.p256_req);
        struct llm_p256_req *req;
        bool ble_ecc_op_ongoing;
        uint8_t param_status;
        uint8_t param_p256_data[ECDH_KEY_LEN*2];

        // Check if the popped message is valid. If the message is NULL it means that the LLM
        // was reset during an encryption and therefore we can exit immediately the function
        if (msg == NULL) {
                return(KE_MSG_CONSUMED);
        }

        switch(llm_le_env.llm_p256_state) {
        case LLM_P256_STATE_IDLE:
                break;
        case LLM_P256_STATE_TRNG:
                /*
                 * Generate polling request.
                 * Duplicate the request so we don't miss the arguments:
                 *
                 * HCI_LE_READ_LOCAL_P256_PUBLIC_KEY_CMD_OPCODE:
                 * - (A) req->p256_data holds ecc_p256_G
                 * - (k) our private key
                 */
                req = KE_MSG_ALLOC(LLM_P256_REQ, TASK_LLM, TASK_LLM, llm_p256_req);
                memcpy(req, param, sizeof(struct llm_p256_req));
                ke_msg_send(req);

                /*
                 * Generate our private key
                 */
                sys_trng_get_bytes(&llm_le_env.llm_p256_private_key[0], ECDH_KEY_LEN);

                llm_le_env.llm_p256_state = LLM_P256_STATE_PKMULT;
                break;
        case  LLM_P256_STATE_PKMULT:
        case  LLM_P256_STATE_ACQ_ECC:
                /*
                 * Generate polling request.
                 * Duplicate the request so we don't miss the arguments and prepare Ak operation:
                 *
                 * HCI_LE_READ_LOCAL_P256_PUBLIC_KEY_CMD_OPCODE:
                 * - (A) req->p256_data holds ecc_p256_G
                 * - (k) our private key
                 * HCI_LE_GENERATE_DHKEY_CMD_OPCODE:
                 * - (A) req->p256_data holds remote's public key
                 * - (k) our private key
                 */
                req = KE_MSG_ALLOC(LLM_P256_REQ, TASK_LLM, TASK_LLM, llm_p256_req);
                memcpy(req, param, sizeof(struct llm_p256_req));
                ke_msg_send(req);

                if (ad_crypto_acquire_ecc(0) != OS_MUTEX_TAKEN) {
                        /*
                         * Retry to acquire ECC mutex
                         */
                } else {
                        /*
                         * Schedule ECC operation (Ak, k is our private key)
                         */
                        hw_ecc_write256_r(0, hw_ecc_p256_q, ad_crypto_get_ecc_base_addr());
                        hw_ecc_write256_r(4, hw_ecc_p256_a, ad_crypto_get_ecc_base_addr());
                        hw_ecc_write256_r(5, hw_ecc_p256_b, ad_crypto_get_ecc_base_addr());
                        // first operand
                        hw_ecc_write256_r(6, &req->p256_data[0], ad_crypto_get_ecc_base_addr()); // x coordinate
                        hw_ecc_write256_r(7, &req->p256_data[ECDH_KEY_LEN], ad_crypto_get_ecc_base_addr()); // y coordinate
                        // second operand
                        hw_ecc_write256_r(8, &llm_le_env.llm_p256_private_key[0], ad_crypto_get_ecc_base_addr());
                        hw_ecc_cfg_ops(6, 8, 10);
                        hw_crypto_enable_ecc_interrupt(ble_ecc_cb);
                        hw_ecc_enable_clock();

                        llm_le_env.llm_p256_state = LLM_P256_STATE_CHECK_PXY;
                        hw_ecc_write_command_register(HW_ECC_CMD_CALCR2_TRUE,
                                                      HW_ECC_CMD_SIGNB_POS,
                                                      HW_ECC_CMD_SIGNA_POS,
                                                      HW_ECC_CMD_OP_SIZE_256B,
                                                      HW_ECC_CMD_FIELD_FP,
                                                      HW_ECC_CMD_OP_CHECK_PXY);

                        ble_ecc_op.generates_req = false;
                        ble_ecc_op.ongoing = true;
                        hw_ecc_start();
                }
                break;
        case LLM_P256_STATE_CHECK_PXY:
                /*
                 * Generate empty polling request.
                 * Arguments (req->p256_data) are "Do not care".
                 */
                req = KE_MSG_ALLOC(LLM_P256_REQ, TASK_LLM, TASK_LLM, llm_p256_req);
                ke_msg_send(req);

                GLOBAL_INT_DISABLE();
                ble_ecc_op_ongoing = ble_ecc_op.ongoing;
                GLOBAL_INT_RESTORE();

                if (!ble_ecc_op_ongoing) {
                        if (ble_ecc_op.succeed) {
                                /*
                                 * Start "Check Point on Curve" operation
                                 */
                                llm_le_env.llm_p256_state = LLM_P256_STATE_CHECK_POINT_ON_CURVE;
                                hw_ecc_write_command_register(HW_ECC_CMD_CALCR2_TRUE,
                                                                      HW_ECC_CMD_SIGNB_POS,
                                                                      HW_ECC_CMD_SIGNA_POS,
                                                                      HW_ECC_CMD_OP_SIZE_256B,
                                                                      HW_ECC_CMD_FIELD_FP,
                                                                      HW_ECC_CMD_OP_POINT_ON_CURVE);
                                ble_ecc_op.generates_req = false;
                                ble_ecc_op.ongoing = true;
                                hw_ecc_start();
                        } else {
                                llm_le_env.llm_p256_state = LLM_P256_STATE_REL_ECC;
                        }
                }
                break;
        case LLM_P256_STATE_CHECK_POINT_ON_CURVE:
                GLOBAL_INT_DISABLE();
                ble_ecc_op_ongoing = ble_ecc_op.ongoing;
                GLOBAL_INT_RESTORE();

                if (!ble_ecc_op_ongoing) {
                        if (ble_ecc_op.succeed) {
                                /*
                                 * Start "Point Multiplication" operation
                                 */
                                llm_le_env.llm_p256_state = LLM_P256_STATE_PKMULT_WAIT;
                                hw_ecc_write_command_register(HW_ECC_CMD_CALCR2_TRUE,
                                                                      HW_ECC_CMD_SIGNB_POS,
                                                                      HW_ECC_CMD_SIGNA_POS,
                                                                      HW_ECC_CMD_OP_SIZE_256B,
                                                                      HW_ECC_CMD_FIELD_FP,
                                                                      HW_ECC_CMD_OP_POINT_MLT);

                                if (msg->src_id == HCI_LE_READ_LOCAL_P256_PUBLIC_KEY_CMD_OPCODE) {
                                        /*
                                        * Since this cmd is not registered as a GAPM_OP_CFG by smpm_ecdh_key_create(),
                                        * we have to "block" ad_ble() by generating polling messages to
                                        * block application commands while we generate the public/private key after reset.
                                        *
                                        * We should not start any pairing operations until the key pair is generated.
                                        * (takes ~70ms when running @16Mhz).
                                        */
                                        ble_ecc_op.generates_req = false;
                                        ble_ecc_op.ongoing = true;
                                        hw_ecc_start();
                                } else if (msg->src_id == HCI_LE_GENERATE_DHKEY_CMD_OPCODE) {
                                        /*
                                        * Point multiplication while generating DH can be non-blocking,
                                        * ad_ble() is allowed to execute other cmds while ECC operation is in progress.
                                        * This operation has been registered as GAPM_OP_CFG by gapm_use_p256_block_cmd_handler().
                                        * Other CFG ops will be postponed internally until we finish.
                                        */
                                        ble_ecc_op.generates_req = true;
                                        ble_ecc_op.ongoing = true;
                                        hw_ecc_start();

                                        /*
                                         * Stop here, we should not generate new req
                                         * (ecc_cb will generate one once finished)
                                         */
                                        break;
                                }
                        } else {
                                llm_le_env.llm_p256_state = LLM_P256_STATE_REL_ECC;
                        }
                }

                /*
                 * Generate empty polling request.
                 * Arguments (req->p256_data) are "Do not care".
                 */
                req = KE_MSG_ALLOC(LLM_P256_REQ, TASK_LLM, TASK_LLM, llm_p256_req);
                ke_msg_send(req);
                break;
        case LLM_P256_STATE_PKMULT_WAIT:
                /*
                 * Generate empty polling request.
                 * Arguments (req->p256_data) are "Do not care".
                 */
                req = KE_MSG_ALLOC(LLM_P256_REQ, TASK_LLM, TASK_LLM, llm_p256_req);
                ke_msg_send(req);

                GLOBAL_INT_DISABLE();
                ble_ecc_op_ongoing = ble_ecc_op.ongoing;
                GLOBAL_INT_RESTORE();

                if (!ble_ecc_op_ongoing) {
                        /*
                         * PKMULT completed
                         */
                        llm_le_env.llm_p256_state = LLM_P256_STATE_REL_ECC;
                }
                break;
        case  LLM_P256_STATE_REL_ECC:
                if (ble_ecc_op.succeed) {
                        hw_ecc_read256_r(10, &param_p256_data[0], ad_crypto_get_ecc_base_addr());
                        hw_ecc_read256_r(11, &param_p256_data[ECDH_KEY_LEN], ad_crypto_get_ecc_base_addr());
                        param_status = CO_ERROR_NO_ERROR;
                } else {
                        /*
                         * PKMULTI operation failed, sent an invalid point as public key.
                         * If we generate an error here then hci_le_generate_dhkey_cmp_handler()
                         * will not generate an indication and the state machine will halt.
                         * Instead we invalidate remote's PK here (by randomly generating it)
                         * and let pairing fail during the "DH check" according to spec.
                         */
                        sys_trng_get_bytes(param_p256_data, ECDH_KEY_LEN);
                        param_status = CO_ERROR_INVALID_HCI_PARAM;
                }

                hw_crypto_disable_ecc_interrupt();
                hw_ecc_disable_clock();
                ad_crypto_release_ecc();

                /*
                 * PKMULT completed, "param_p256_data" holds the result and "param_status" the status.
                 */

                //send the LE_READ_LOCAL_P256_PUBLIC_KEY_COMPLETE_EVENT
                //If Event not masked
                if ((msg->src_id == HCI_LE_READ_LOCAL_P256_PUBLIC_KEY_CMD_OPCODE) && llm_util_check_evt_mask(LE_READ_P256_PUBLIC_KEY_EVT_BIT)) {
                                // allocate the event message
                                struct hci_rd_p256_public_key_cmd_cmp_evt *event = KE_MSG_ALLOC(HCI_LE_EVENT, 0, HCI_LE_META_EVT_CODE, hci_rd_p256_public_key_cmd_cmp_evt);

                                // fill event parameters
                                event->subcode = HCI_LE_READ_LOCAL_P256_PUBLIC_KEY_CMP_EVT_SUBCODE;
                                if (param_status == 0) {
                                        event->status = CO_ERROR_NO_ERROR;

                                        //reverse it
                                        for(int i=0; i< ECDH_KEY_LEN; i++) {
                                                event->public_key[i] = param_p256_data[ECDH_KEY_LEN - 1 - i];
                                        }

                                        for(int i=0; i< ECDH_KEY_LEN; i++) {
                                                event->public_key[ECDH_KEY_LEN+i] = param_p256_data[2*ECDH_KEY_LEN - 1 - i];
                                        }
                                } else {
                                        event->status = CO_ERROR_HARDWARE_FAILURE;
                                        event->public_key[0] = param_status;   //put actual failure in first byte
                                }

                                // send the message
                                hci_send_2_host(event);
                }
                //send the LE_GENERATE_DHKEY_COMPLETE_EVENT
                //If Event not masked
                else if((msg->src_id == HCI_LE_GENERATE_DHKEY_CMD_OPCODE) && llm_util_check_evt_mask(LE_GENERATE_DHKEY_EVT_BIT)) {
                                // allocate the event message
                                struct hci_generate_dhkey_cmd_cmp_evt *event = KE_MSG_ALLOC(HCI_LE_EVENT, 0, HCI_LE_META_EVT_CODE, hci_generate_dhkey_cmd_cmp_evt);

                                // fill event parameters
                                event->subcode = HCI_LE_GENERATE_DHKEY_CMP_EVT_SUBCODE;
                                if(param_status == 0) {
                                        event->status = CO_ERROR_NO_ERROR;

                                        //reverse X key
                                        for(int i=0; i< ECDH_KEY_LEN; i++) {
                                                event->dhkey[i] = param_p256_data[ECDH_KEY_LEN - 1 - i];
                                        }
                                }
                                else {
                                        event->status = CO_ERROR_INVALID_HCI_PARAM;

                                        // fill the key with 0xFF
                                        for(int i = 0; i < ECDH_KEY_LEN; i++) {
                                                event->dhkey[i] = 0xFF;
                                        }
                                }

                                // send the message
                                hci_send_2_host(event);
                }

                llm_le_env.llm_p256_state = LLM_P256_STATE_IDLE;

                #if (DEEP_SLEEP)
                // Prevent from going to deep sleep
                rwip_prevent_sleep_clear(RW_P256_ONGOING);
                #endif //DEEP_SLEEP

                msg = (struct ke_msg *)co_list_pop_front(&llm_le_env.p256_req);

                // Free the message
                ke_msg_free(msg);

                // Check if a new operation has to be launched
                msg = (struct ke_msg *)co_list_pick(&llm_le_env.p256_req);
                if (msg != NULL) {
                                // Start the operation
                                llm_p256_start(msg);
                }

                break;
        }

        return (KE_MSG_CONSUMED);
#else
        struct ke_msg *msg = (struct ke_msg *)co_list_pick(&llm_le_env.p256_req);

        if (msg == NULL) {
                return(KE_MSG_CONSUMED);
        }

        llm_le_env.llm_p256_state = LLM_P256_STATE_IDLE;

#if (DEEP_SLEEP)
        rwip_prevent_sleep_clear(RW_P256_ONGOING);
#endif //DEEP_SLEEP

        msg = (struct ke_msg *)co_list_pop_front(&llm_le_env.p256_req);

        // Free the message
        ke_msg_free(msg);

        return (KE_MSG_CONSUMED);
#endif /* !BLE_PROD_TEST */
}
#endif /* CONFIG_USE_BLE */
