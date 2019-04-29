/**
 ****************************************************************************************
 *
 * @file rf_ble_functions.c
 *
 * @brief RF empty functions 
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifdef CONFIG_USE_BLE
#include "ble_stack_config.h"

#if RADIO_680

#include "rwip_config.h"        // RW SW configuration

#include <string.h>             // for memcpy
#include "co_utils.h"           // common utility definition
#include "co_math.h"            // common math functions
#include "co_endian.h"          // endian definitions
#include "rf.h"                 // RF interface
#include "plf.h"                // Platform functions

#include "rwip.h"               // for RF API structure definition

#if (BLE_EMB_PRESENT)
#include "reg_blecore.h"        // ble core registers
#include "reg_ble_em_cs.h"      // control structure definitions
#endif //BLE_EMB_PRESENT

#include "ad_rf.h"

#include "em_map_ble_user.h"
#include "em_map_ble.h"

#include "_reg_common_em_et.h"

#include "arch.h"
#include "sdk_defs.h"

#include "sys_tcs.h"

#include "ad_ble.h"

extern bool rf_in_sleep;

#ifdef BLE_PROD_TEST
extern volatile uint32_t prod_test_tx_packet_count;
extern volatile uint32_t prod_test_state;

#endif

/// BLE read burst

/// BLE write burst
__STATIC_INLINE void em_ble_burst_wr(void const *sys_addr, uint16_t em_addr, uint16_t len)
{
        memcpy((void *)(em_addr + REG_COMMON_EM_ET_BASE_ADDR), sys_addr, len);
}

/// BLE EM single write
__STATIC_INLINE void em_ble_wr(uint8_t data, uint16_t em_addr)
{
        *(uint8_t *)(em_addr + REG_COMMON_EM_ET_BASE_ADDR) = data;
}

uint32_t rf_rpl_reg_rd (uint16_t address)
{
        return 0;
}

void rf_rpl_reg_wr (uint16_t address, uint32_t data)
{

}

/*****************************************************************************************
 * @brief Static function - Ripple TX CNTL1 by radio
 ****************************************************************************************
 */
void rf_rpl_set_txcntl1(void)
{

}

/*****************************************************************************************
 * @brief Static function - Ripple RF Power up sequence (all on)
 ****************************************************************************************
 */
void rf_rpl_pw_up(void)
{

}

/*****************************************************************************************
 * @brief Static function - Init modem for Ripple.
 ****************************************************************************************
 */
void rf_rpl_mdm_init(void)
{


}

/**
 ****************************************************************************************
 * @brief Static function - Measure Ripple VCO Frequency
 *
 * @param[in] vco_fc_value  VCO
 * @param[in] vco_freq      Pointer to frequency value.
 ****************************************************************************************
 */
void rf_rpl_measure_vco_freq(uint8_t vco_fc_value, int * vco_freq)
{

}

/**
 ****************************************************************************************
 * @brief Static function - for VCO Calibration
 *
 * @param[in] channel   channel
 * @param[in] vco_val   vco value
 ****************************************************************************************
 */
void rf_rpl_calib_vco_fq(uint8_t channel, uint8_t *vco_val)
{

}

/**
 ****************************************************************************************
 * @brief Static function for calibrating ICP value.
 *
 * @param[in] icp Pointer to value to calibrate.
 ****************************************************************************************
 */
void rf_rpl_calib_icp(uint8_t channel,uint8_t * icp)
{

}

/**
 ****************************************************************************************
 * @brief Static function for status lock.
 *
 * @param[in] chnl  channel
 * @param[in] icp   icp
 * @param[in] vco   vco value
 * @param[in] lock  pointer to lock
 ****************************************************************************************
 */
void rf_rpl_status_lock(uint8_t chnl, uint8_t icp, uint8_t vco, uint8_t *lock)
{

}

/***************************************************************************************
 * @brief Static function for radio PLL auto-calibration.
 ****************************************************************************************
 */
void rf_rpl_pll_autocalib(void)
{

}

/***************************************************************************************
 * @brief Static Ripple radio Calibration function.
 ***************************************************************************************
 */
void rf_rpl_calib(void)
{

}

/***************************************************************************************
 * @brief Static function - Sequencer settings Initialization for Ripple radio
 ****************************************************************************************
*/
void rf_rpl_sequencers_init(void)
{

}

/***************************************************************************************
 * @brief Static function - Tx Gain tables settings
 ****************************************************************************************
 */
void rf_rpl_txgain_set(void)
{

}

/***************************************************************************************
 * @brief Static function - Initialization sequence for Ripple radio
 ****************************************************************************************
 */
void rf_rpl_init_seq(void)
{

}

/**
 *****************************************************************************************
 * @brief Init RF sequence after reset.
 *****************************************************************************************
 */
void rf_reset(void)
{

}

/**
 *****************************************************************************************
 * @brief Enable/disable force AGC mechanism
 *
 * @param[in]  True: Enable / False: disable
 *****************************************************************************************
 */
 void rf_force_agc_enable(bool en)
{

}

/**
 *****************************************************************************************
 * @brief Get TX power in dBm from the index in the control structure
 *
 * @param[in] txpwr_idx  Index of the TX power in the control structure
 * @param[in] modulation Modulation: 1 or 2 or 3 MBPS
 *
 * @return The TX power in dBm
 *
 *****************************************************************************************
 */
uint8_t rf_txpwr_dbm_get(uint8_t txpwr_idx, uint8_t modulation)
{
        return 0;
}

static void rf_sleep(void)
{

        ble_deepslcntl_set(ble_deepslcntl_get() |
                                BLE_DEEP_SLEEP_ON_BIT |    // RW BLE Core sleep
                                BLE_RADIO_SLEEP_EN_BIT |   // Radio sleep
                                BLE_OSC_SLEEP_EN_BIT);     // Oscillator sleep

        rf_in_sleep = true;
}

static void RADIOCNTL_Handler(void)
{

}

static uint8_t rf_rssi_convert(uint8_t rssi_reg)
{
        return ((rssi_reg / 2) - 112);
}

/**
 * \brief RF diagnostic port interrupt
 */
void RF_DIAG_Handler(void)
{
#ifdef BLE_PROD_TEST
        volatile uint16 rf_diagirq_stat;

        rf_diagirq_stat = RFCU->RF_DIAGIRQ_STAT_REG;

        if ( rf_diagirq_stat & (1<<1))
        {
                if (prod_test_state == 3) /* TX INTERVAL */ {
                        prod_test_tx_packet_count++;
                        ad_ble_task_notify_from_isr(mainBIT_COMMAND_QUEUE);
                }
        }
#endif
}

/**
 * \brief RF CRC patch
 */
void rf_init_crc_patch(void)
{
        REG_SETF(BLE, BLE_DIAGCNTL2_REG, DIAG6, 0x25);                  // diag6 configured for mode 0x25
        REG_SET_BIT(BLE, BLE_DIAGCNTL2_REG, DIAG6_EN);                  // enable port
        REG_SETF(BLE, BLE_DIAGCNTL3_REG, DIAG6_BIT, 1);                 // txen on diag6

        uint32_t reg = RFCU->RF_DIAGIRQ01_REG;
        reg |= (1 << REG_POS(RFCU, RF_DIAGIRQ01_REG, DIAGIRQ_EDGE_1));  // Falling edge
        REG_SET_FIELD(RFCU, RF_DIAGIRQ01_REG, DIAGIRQ_BSEL_1, reg, 7);  // Select bit #7 (TX_EN)
        REG_SET_FIELD(RFCU, RF_DIAGIRQ01_REG, DIAGIRQ_WSEL_1, reg, 2);  // Select RADIO_DIAG1
        RFCU->RF_DIAGIRQ01_REG = reg;

        reg = RFCU->RF_DIAGIRQ23_REG;
        REG_CLR_FIELD(RFCU, RF_DIAGIRQ23_REG, DIAGIRQ_EDGE_3, reg);     // Rising edge
        REG_SET_FIELD(RFCU, RF_DIAGIRQ23_REG, DIAGIRQ_BSEL_3, reg, 7);  // Select bit #7 (TX_EN)
        REG_SET_FIELD(RFCU, RF_DIAGIRQ23_REG, DIAGIRQ_WSEL_3, reg, 2);  // Select RADIO_DIAG3
        RFCU->RF_DIAGIRQ23_REG = reg;

        (void) RFCU->RF_DIAGIRQ_STAT_REG;

        REG_SETF(RFCU, RF_DIAGIRQ01_REG, DIAGIRQ_MASK_1, 0x1);          // Enable IRQ generation
        REG_SETF(RFCU, RF_DIAGIRQ23_REG, DIAGIRQ_MASK_3, 0x1);          // Enable IRQ generation

        // Enable RF_DIAG_IRQn interrupt - The ISR is implemented with
        // the RF_DIAG_Handler() function
        NVIC_EnableIRQ(RF_DIAG_IRQn);
}

void rf_ble_set_recommended_settings(void)
{
#if dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A
        BLE->BLE_RADIOPWRUPDN_REG = 0x0754054C; // Last review date: Feb 15, 2016 - 12:25:47
#else
        BLE->BLE_RADIOPWRUPDN_REG = 0x07540560;
#endif
}

/**
 *****************************************************************************************
 * @brief RF initialization specifically for the BLE interface
 *
 * @param[in]  Env variable to hold the setup
 *
 *****************************************************************************************
 */
void rf_init_sdk(struct rwip_rf_api *api)
{
        uint8 temp_freq_tbl[EM_BLE_FREQ_TABLE_LEN];

        // Initialize the RF driver API structure
        api->reg_rd = rf_rpl_reg_rd;
        api->reg_wr = rf_rpl_reg_wr;
        api->txpwr_dbm_get = rf_txpwr_dbm_get;

        api->txpwr_max = 6;
        api->sleep = rf_sleep;
        api->reset = rf_reset;
#ifdef CFG_BLE
        api->isr = RADIOCNTL_Handler;
        api->force_agc_enable = rf_force_agc_enable;
#endif //CFG_BLE

        api->rssi_convert = rf_rssi_convert;

#if defined(CFG_BT)
        api->txpwr_inc = rf_txpwr_inc;
        api->txpwr_dec = rf_txpwr_dec;
        api->txpwr_epc_req = rf_txpwr_epc_req;
        api->txpwr_cs_get = rf_txpwr_cs_get;
        api->rssi_convert = rf_rssi_convert;
        api->rssi_high_thr = (uint8_t)RPL_RSSI_20dB_THRHLD;
        api->rssi_low_thr = (uint8_t)RPL_RSSI_60dB_THRHLD;
        api->rssi_interf_thr = (uint8_t)RPL_RSSI_70dB_THRHLD;
# ifdef CFG_BTCORE_30
        api->wakeup_delay = RPL_WK_UP_DELAY;
# endif //CFG_BTCORE_30
        api->skew = RPL_RADIO_SKEW;
#endif //CFG_BT

        ad_rf_request_on(true);

//      REG_SETF(BLE, BLE_RADIOCNTL0_REG, DPCORR_EN, 0);  //THIS MAY NOT BE '1', THEN WE MISS 12 BITS IN THE SYNCWORD DURING A RX BURST
        REG_SETF(BLE, BLE_RADIOCNTL1_REG, XRFSEL, 2);

        REG_CLR_BIT(BLE, BLE_CNTL2_REG, SW_RPL_SPI);

#if dg_configBLACK_ORCA_IC_REV != BLACK_ORCA_IC_REV_A && defined(CONFIG_USE_FTDF)
        /* In co-existence scenarios, when BLE interrupts FTDF, this gives time for the FTDF radio
         * to ramp down so as not to interfere with the BLE transmission. */
        REG_SETF(BLE, BLE_CNTL2_REG, BLE_TRANSACTION_START, 0x82);
#endif
        /* Initialize to 0. Use rf_ble_set_ant_trim to set a value to a specific channel */
        memset(temp_freq_tbl, 0, sizeof(temp_freq_tbl));

        /* Apply trim values */
        sys_tcs_apply(tcs_ble);

        rf_ble_set_recommended_settings();
        ad_rf_request_recommended_settings();

        //rwble_diagport_init();
}

void rf_reinit_sdk(void)
{
        rf_in_sleep = false;

        /* Apply trim values */
        sys_tcs_apply(tcs_ble);

        rf_ble_set_recommended_settings();
        ad_rf_request_recommended_settings();


#if LUT_PATCH_ENABLED
        const volatile struct LUT_CFG_struct *pLUT_CFG;

        pLUT_CFG = (const volatile struct LUT_CFG_struct *)(jump_table_struct[lut_cfg_pos]);
        if (!pLUT_CFG->HW_LUT_MODE) {
                enable_rf_diag_irq(RF_DIAG_IRQ_MODE_RXTX);
        }
        else {
                vcocal_ctrl_reg_val = PLLDIG->RF_VCOCAL_CTRL_REG;
# if MGCKMODA_PATCH_ENABLED
                enable_rf_diag_irq(RF_DIAG_IRQ_MODE_TXONLY);    // This just enables the TX_EN int. RX_EN int enable status remains as it was.
# endif //MGCKMODA_PATCH_ENABLED
        }
#else //LUT_PATCH_ENABLED
# if MGCKMODA_PATCH_ENABLED
        enable_rf_diag_irq(RF_DIAG_IRQ_MODE_TXONLY);            // This just enables the TX_EN int. RX_EN int enable status remains as it was
# endif //MGCKMODA_PATCH_ENABLED
#endif //LUT_PATCH_ENABLED
}

void rf_ble_set_ant_trim(uint8_t freq_idx, uint8_t value)
{
        ASSERT_ERROR(freq_idx < EM_BLE_FREQ_TABLE_LEN);

        GLOBAL_INT_DISABLE();
        em_ble_wr((value & 0x7) << 4, EM_FT_OFFSET + freq_idx);
        GLOBAL_INT_RESTORE();

}
#endif //RF_680
#endif /* CONFIG_USE_BLE */
