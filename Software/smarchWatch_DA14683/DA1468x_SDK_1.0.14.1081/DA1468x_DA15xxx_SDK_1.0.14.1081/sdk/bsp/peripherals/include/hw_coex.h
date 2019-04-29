/**
 ****************************************************************************************
 *
 * @file hw_coex.h
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
\brief Radio MAC Arbiter
*/

#ifndef HW_COEX_H_
#define HW_COEX_H_

#if dg_configUSE_HW_COEX

#include <stdint.h>
#include <stdbool.h>

#if dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A
#define HW_COEX_PTI_TABLE_SIZE          17
#else
#define HW_COEX_PTI_TABLE_SIZE          15
#endif

#if dg_configCOEX_ENABLE_STATS
/** Arbiter statistics (IRQ counters). */
typedef struct {
#if dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A
        /** Rising edge of SMART_ACT. */
        uint32_t smart_act_r;

        /** Falling edge of SMART_ACT. */
        uint32_t smart_act_f;

        /** Rising edge of SMART_PRI. */
        uint32_t smart_pri_r;

        /** Falling edge of SMART_PRI. */
        uint32_t smart_pri_f;

        /** Rising edge of EXT_ACT. */
        uint32_t ext_act_r;

        /** Falling edge of EXT_ACT. */
        uint32_t ext_act_f;

        /** Rising edge of FTDF_ACTIVE internal signal. */
        uint32_t ftdf_active_r;

        /** Falling edge of FTDF_ACTIVE internal signal. */
        uint32_t ftdf_active_f;

        /** Rising edge of BLE_ACTIVE internal signal. */
        uint32_t ble_active_r;

        /** Falling edge of BLE_ACTIVE internal signal. */
        uint32_t ble_active_f;

        /** Rising edge of RADIO_BUSY. */
        uint32_t radio_busy_r;

        /** Falling edge of RADIO_BUSY. */
        uint32_t radio_busy_f;

        /** While entering into "closing" sub-state, the TX_EN or RX_EN are active. */
        uint32_t closing_brk;

        /** When the decision switches to a MAC, and the TX_EN or RX_EN of this MAC are high.
         * This event signals a potential break of a transmission or reception. */
        uint32_t start_mid;
#else

        /** Tx/Rx monitor statistics. */
        struct {
                /** Number of missed Tx/RX monitor events due to overflow. */
                uint32_t overflow;
                struct {
                        /**
                         * Number of Tx transactions of this PTI table entry that have
                         * passed (i.e. won the arbitration).
                         */
                        uint32_t tx_passed;

                        /**
                         * Number of Rx transactions of this PTI table entry that have
                         * passed (i.e. won the arbitration).
                         */
                        uint32_t rx_passed;

                        /**
                         * Number of Tx transactions of this PTI table entry that have been
                         * masked (i.e. lost the arbitration).
                         */
                        uint32_t tx_masked;

                        /**
                         * Number of Rx transactions of this PTI table entry that have been
                         * masked (i.e. lost the arbitration).
                         */
                        uint32_t rx_masked;
                } ptr[HW_COEX_PTI_TABLE_SIZE];
        } txrx_mon;
#endif
        /** When the decision switches to a new MAC, ignoring the intermediate transitions to
         * DECISION==NONE. */
        uint32_t decision_sw;
} hw_coex_stats_t;

/** Arbiter statistics instance. */
extern hw_coex_stats_t hw_coex_stats;

#endif

/** MAC types. */
typedef enum  {
        HW_COEX_MAC_TYPE_NONE = 0,
        HW_COEX_MAC_TYPE_BLE,
        HW_COEX_MAC_TYPE_FTDF,
        HW_COEX_MAC_TYPE_EXT,
} HW_COEX_MAC_TYPE;

/** BLE radio busy signal modes. */
typedef enum {
        /** (decision==BLE) AND rfcu.radio_busy */
        HW_COEX_BLE_RADIO_BUSY_MODE_NORMAL = 0,
        /** Hold to "0" */
        HW_COEX_BLE_RADIO_BUSY_MODE_ZERO,
        /** (decision==FTDF) OR (decision==EXT) OR rfcu.radio_busy */
        HW_COEX_BLE_RADIO_BUSY_MODE_FULL,
        /** (decision==FTDF) OR (decision==EXT) */
        HW_COEX_BLE_RADIO_BUSY_MODE_EXCLUSIVE,
} HW_COEX_RADIO_BUSY_MODE;

#if dg_configCOEX_ENABLE_CONFIG == 1

/**
 * \brief Programs the Arbiter to ignore a MAC or not. By default, all MACs are taken into account.
 *
 * \param [in] mac_type The MAC to be included / excluded.
 * \param [in] set Ignore the MAC if true, include it in priorities evaluation if false.
 *
 * \return void
 */
void hw_coex_set_ignore_mac(HW_COEX_MAC_TYPE mac_type, bool set);

/**
 * \brief Packet Traffic Information (PTI).
 *
 * Values 0 - 7.
 */
typedef uint16_t hw_coex_pti_t;

/** Arbiter priority. */
typedef struct {
        /** Packet traffic information (PTI). */
        hw_coex_pti_t pti;

        /** MAC type. */
        HW_COEX_MAC_TYPE mac;
} hw_coex_priority_t;

/**
 * \brief Bit map with flags that control arbiter behavior.
 *
 * See HW_COEX_CTRL_* macros for information on each control bit.
 */
typedef uint16_t hw_coex_ctrl_t;

/**
 * \brief Default/reset value for @ref hw_coex_ctrl_t.
 *
 * This MUST match the reset values in the hardware.
 */
#define HW_COEX_CTRL_RESET                              ((hw_coex_ctrl_t) (0))

/**
 * \brief If set, CCA stat towards FTDF core will be forced to busy when the arbiter's decision
 * is NOT the FTDF MAC, else CCA stat will be driven normally by the RF.
 */
#define HW_COEX_CTRL_BIT_FTDF_FORCE_CCA                 ((hw_coex_ctrl_t) (1 << 0))

/** \brief If set, BLE requests to the arbiter will be ignored. */
#define HW_COEX_CTRL_BIT_IGNORE_BLE                     ((hw_coex_ctrl_t) (1 << 1))

/** \brief If set, FTDF requests to the arbiter will be ignored. */
#define HW_COEX_CTRL_BIT_IGNORE_FTDF                    ((hw_coex_ctrl_t) (1 << 2))

/** \brief If set, EXT requests to the arbiter will be ignored. */
#define HW_COEX_CTRL_BIT_IGNORE_EXT                     ((hw_coex_ctrl_t) (1 << 3))

#if dg_configBLACK_ORCA_IC_REV != BLACK_ORCA_IC_REV_A

/**
 * \brief If set, the BLE PTI is supplied by the BLE block. BLE PTI in arbiter configuration is
 * ignored.
 */
#define HW_COEX_CTRL_BIT_BLE_PTI_AUTO                   ((hw_coex_ctrl_t) (1 << 4))

/**
 * \brief If set, the FTDF PTI is supplied by the FTDF block. FTDF PTI in arbiter configuration is
 * ignored.
 */
#define HW_COEX_CTRL_BIT_FTDF_PTI_AUTO                  ((hw_coex_ctrl_t) (1 << 5))

#if dg_configCOEX_ENABLE_STATS == 1

/**
 * \brief If set, Tx/Rx monitor statistics will be enabled.
 */
#define HW_COEX_CTRL_BIT_TXRX_MON_STATS_ENABLED         ((hw_coex_ctrl_t) (1 << 6))

/**
 * \brief If set, decision switch statistics will be enabled.
 */
#define HW_COEX_CTRL_BIT_DECISION_SW_STATS_ENABLED      ((hw_coex_ctrl_t) (1 << 7))

/**
 * \brief If true, BLE Tx/Rx monitor will generate IRQ for all transactions, else it will
 * generate IRQs only for the masked transactions.
 */
#define HW_COEX_CTRL_BIT_BLE_TXRX_MON_ALL               ((hw_coex_ctrl_t) (1 << 8))

/**
 * \brief If true, FTDF Tx/Rx monitor will generate IRQ for all transactions, else it will
 * generate IRQs only for the masked transactions.
 */
#define HW_COEX_CTRL_BIT_FTDF_TXRX_MON_ALL              ((hw_coex_ctrl_t) (1 << 9))

/**
 * \brief If true, the arbiter will generate IRQ when there is any change in its decision.
 *
 * If false, IRQs will be generated only when MAC is switched. Switching to type none does not
 * count as a switch, i.e. the sequence FTDF->NONE->BLE is considered one switch from FTDF
 * to BLE.
 */
#define HW_COEX_CTRL_BIT_DECISION_SW_ALL                ((hw_coex_ctrl_t) (1 << 10))
#endif /* dg_configCOEX_ENABLE_STATS == 1 */
#endif /* dg_configBLACK_ORCA_IC_REV != BLACK_ORCA_IC_REV_A */


/** Arbiter configuration. */
typedef struct {
        /** Control bit map. */
        hw_coex_ctrl_t ctrl;

        /** Current PTI for BLE. Relevant only in non-auto mode. */
        hw_coex_pti_t ble_pti;

        /** Current PTI for FTDF. Relevant only in non-auto mode. */
        hw_coex_pti_t ftdf_pti;

        /** Priorities table. */
        hw_coex_priority_t pri[HW_COEX_PTI_TABLE_SIZE];
} hw_coex_config_t;


/**
 * \brief Initializes internal configuration,.
 *
 * \note Must be called before all other calls that access internal configuration.
 */
void hw_coex_init(void);

/**
 * \brief Restores arbiter configuration.
 *
 * Radio power domain must be on and the RFCU clock is assumed active throughout its execution.
 *
 * \note Must be called after @ref hw_coex_init();
 */
__RETAINED_CODE void hw_coex_apply_config(void);

/**
 * \brief Manually sets FTDF MAC PTI value. Relevant only when FTDF PTI mode is non-auto.
 *
 * \param [in] ftdf_pti PTI value
 *
 * \param [out] prev_ftdf_pti Previous PTI value. Ignored when NULL.
 *
 * \param [in] force_decision If true, arbiter hardware will be immediately notified of the PTI
 * change. Ignored when arbiter power domain is off.
 *
 * \note Must be called after @ref hw_coex_init();
 */
void hw_coex_update_ftdf_pti(hw_coex_pti_t ftdf_pti, hw_coex_pti_t * prev_ftdf_pti,
        bool force_decision);

/**
 * \brief Manually sets BLE MAC PTI value. Relevant only when BLE PTI mode is non-auto.
 *
 * \param [in] ble_pti PTI value
 *
 * \param [out] prev_ble_pti Previous PTI value. Ignored when NULL.
 *
 * \param [in] force_decision If true, arbiter hardware will be immediately notified of the PTI
 * change. Ignored when arbiter power domain is off.
 *
 * \note Must be called after @ref hw_coex_init();
 *
 */
void hw_coex_update_ble_pti(hw_coex_pti_t ble_pti, hw_coex_pti_t * prev_ble_pti,
        bool force_decision);

/**
 * \brief Re-programs one of the Arbiter's priorities. The Arbiter goes to "suspended" state.
 *
 * \param [in] index Priority table entry index.
 *
 * \param [in] pri New priority field setting.
 *
 * \param [out] prev_pri If pointer is not NULL, the previous priority field setting will be
 * returned.
 *
 * \note Must be called after @ref hw_coex_init();
 */
void hw_coex_config_set_priority(int index, const hw_coex_priority_t * pri,
        hw_coex_priority_t * prev_pri);

/**
 * Resets arbiter config to default values.
 *
 * \note Must be called after @ref hw_coex_init();
 */
void hw_coex_config_reset(void);

/**
 * \brief Sets arbiter configuration.
 *
 * The configuration will be applied immediately if the radio power domain is on and the RFCU clock
 * is enabled.
 *
 * \param [in] config Arbiter configuration.
 *
 * \note Must be called after @ref hw_coex_init();
 */
void hw_coex_config_set(const hw_coex_config_t *config);

/**
 * Gets arbiter configuration.
 *
 * \param [out] config Arbiter configuration.
 *
 * \note Must be called after @ref hw_coex_init();
 */
void hw_coex_config_get(hw_coex_config_t *config);

#endif
#if dg_configIMAGE_SETUP == DEVELOPMENT_MODE

/**
 * \brief Diagnostics mode.
 *
 * Arbiter diagnostics use the BLE diagnostics bus. BLE diagnostics are assigned to the following
 * GPIOs: \n
 * ble_diag_0: P2_0 \n
 * ble_diag_1: P2_1 \n
 * ble_diag_2: P2_2 \n
 * ble_diag_3: P1_0 \n
 * ble_diag_4: P1_1 \n
 * ble_diag_5: P1_2 \n
 * ble_diag_6: P1_3 \n
 * ble_diag_7: P2_3 \n
 */
typedef enum {
        /**
         * Use bits [5:3] of the BLE diagnostics bus. \n
         * bit 5        : Closing pulse \n
         * bits [4:3]   : Decision bits as per @ref HW_COEX_MAC_TYPE\n
         */
        HW_COEX_DIAG_MODE_1 = 1,

        /**
         * Use bits [7:3] of the BLE diagnostics bus. \n
         * bit 7        : FTDF TX/RX enable
         * bit 6        : BLE TX/RX enable
         * bit 5        : Closing pulse \n
         * bits [4:3]   : Decision bits as per @ref HW_COEX_MAC_TYPE \n
         */
        HW_COEX_DIAG_MODE_2 = 2,

        /**
         * Use bits [7:2] of the BLE diagnostics bus. \n
         * bit 7        : Always 0 \n
         * bits [6:3]   : Decision Pointer (corresponding to the PTI table), that is
         *                the registers COEX_PRI1_REG to COEX_PRI15_REG \n
         * bit 2        : Closing pulse OR RADIO BUSY \n
         */
        HW_COEX_DIAG_MODE_3 = 3,
} HW_COEX_DIAG_MODE;

/**
 * \brief Enables arbiter diagnostics.
 *
 * \param [in] diag_mode Diagnostics mode
 *
 * \return void
 */
void hw_coex_diag_enable(HW_COEX_DIAG_MODE diag_mode);

/**
 * \brief Disables arbiter diagnostics.
 *
 * \return void
 */
static inline void hw_coex_diag_disable(void)
{
        REG_SETF(COEX, COEX_CTRL_REG, SEL_COEX_DIAG, 0);
}
#endif /* dg_configIMAGE_SETUP == DEVELOPMENT_MODE */

#endif /* dg_configUSE_HW_COEX */

#endif /* HW_COEX_H_ */


/**
\}
\}
\}
*/
