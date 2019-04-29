/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup QSPI
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file hw_qspi.c
 *
 * @brief Implementation of the QSPI Low Level Driver.
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configUSE_HW_QSPI

#include <stdint.h>
#include "hw_qspi.h"

__RETAINED_RW uint8_t dummy_num[] = { 0, 1, 2, 0, 3 };

#if (dg_configFLASH_POWER_DOWN == 1)
__attribute__((section("text_retained")))
#endif
void hw_qspi_set_bus_mode(HW_QSPI_BUS_MODE mode)
{
        switch (mode) {
        case HW_QSPI_BUS_MODE_SINGLE:
                QSPIC->QSPIC_CTRLBUS_REG = REG_MSK(QSPIC, QSPIC_CTRLBUS_REG, QSPIC_SET_SINGLE);
                break;
        case HW_QSPI_BUS_MODE_DUAL:
                QSPIC->QSPIC_CTRLBUS_REG = REG_MSK(QSPIC, QSPIC_CTRLBUS_REG, QSPIC_SET_DUAL);
                break;
        case HW_QSPI_BUS_MODE_QUAD:
                QSPIC->QSPIC_CTRLBUS_REG = REG_MSK(QSPIC, QSPIC_CTRLBUS_REG, QSPIC_SET_QUAD);
                hw_qspi_set_io2_output(false);
                hw_qspi_set_io3_output(false);
                break;
        }
}

__attribute__((section("text_retained"))) void hw_qspi_set_automode(bool automode)
{
        if (automode) {
                const uint32_t burst_cmd_a = QSPIC->QSPIC_BURSTCMDA_REG;
                const uint32_t burst_cmd_b = QSPIC->QSPIC_BURSTCMDB_REG;
                const uint32_t status_cmd = QSPIC->QSPIC_STATUSCMD_REG;
                const uint32_t erase_cmd_b = QSPIC->QSPIC_ERASECMDB_REG;
                const uint32_t burstbrk = QSPIC->QSPIC_BURSTBRK_REG;
                if (GETBITS32(QSPIC, QSPIC_BURSTCMDA_REG, burst_cmd_a, QSPIC_INST_TX_MD) == HW_QSPI_BUS_MODE_QUAD ||
                        GETBITS32(QSPIC, QSPIC_BURSTCMDA_REG, burst_cmd_a, QSPIC_ADR_TX_MD) == HW_QSPI_BUS_MODE_QUAD ||
                                GETBITS32(QSPIC, QSPIC_BURSTCMDA_REG, burst_cmd_a, QSPIC_DMY_TX_MD) == HW_QSPI_BUS_MODE_QUAD ||
                                GETBITS32(QSPIC, QSPIC_BURSTCMDA_REG, burst_cmd_a, QSPIC_EXT_TX_MD) == HW_QSPI_BUS_MODE_QUAD ||
                                GETBITS32(QSPIC, QSPIC_BURSTCMDB_REG, burst_cmd_b, QSPIC_DAT_RX_MD) == HW_QSPI_BUS_MODE_QUAD ||
                                GETBITS32(QSPIC, QSPIC_BURSTCMDB_REG, burst_cmd_b, QSPIC_DAT_RX_MD) == HW_QSPI_BUS_MODE_QUAD ||
                                GETBITS32(QSPIC, QSPIC_STATUSCMD_REG, status_cmd, QSPIC_RSTAT_RX_MD) == HW_QSPI_BUS_MODE_QUAD ||
                                GETBITS32(QSPIC, QSPIC_STATUSCMD_REG, status_cmd, QSPIC_RSTAT_TX_MD) == HW_QSPI_BUS_MODE_QUAD ||
                                GETBITS32(QSPIC, QSPIC_ERASECMDB_REG, erase_cmd_b, QSPIC_ERS_TX_MD) == HW_QSPI_BUS_MODE_QUAD ||
                                GETBITS32(QSPIC, QSPIC_ERASECMDB_REG, erase_cmd_b, QSPIC_WEN_TX_MD) == HW_QSPI_BUS_MODE_QUAD ||
                                GETBITS32(QSPIC, QSPIC_ERASECMDB_REG, erase_cmd_b, QSPIC_SUS_TX_MD) == HW_QSPI_BUS_MODE_QUAD ||
                                GETBITS32(QSPIC, QSPIC_ERASECMDB_REG, erase_cmd_b, QSPIC_RES_TX_MD) == HW_QSPI_BUS_MODE_QUAD ||
                                GETBITS32(QSPIC, QSPIC_ERASECMDB_REG, erase_cmd_b, QSPIC_EAD_TX_MD) == HW_QSPI_BUS_MODE_QUAD ||
                                GETBITS32(QSPIC, QSPIC_BURSTBRK_REG, burstbrk, QSPIC_BRK_TX_MD) == HW_QSPI_BUS_MODE_QUAD) {
                        hw_qspi_set_io2_output(false);
                        hw_qspi_set_io3_output(false);
                }
        }
        HW_QSPIC_REG_SETF(CTRLMODE, AUTO_MD, automode);
}

void hw_qspi_set_wrapping_burst_instruction(uint8_t inst, HW_QSPI_WRAP_LEN len,
                                                                        HW_QSPI_WRAP_SIZE size)
{
        HW_QSPIC_REG_SETF(BURSTCMDA, INST_WB, inst);
        QSPIC->QSPIC_BURSTCMDB_REG =
                (QSPIC->QSPIC_BURSTCMDB_REG &
                        ~(REG_MSK(QSPIC, QSPIC_BURSTCMDB_REG, QSPIC_WRAP_SIZE) |
                                REG_MSK(QSPIC, QSPIC_BURSTCMDB_REG, QSPIC_WRAP_LEN))) |
                BITS32(QSPIC, QSPIC_BURSTCMDB_REG, QSPIC_WRAP_SIZE, size) |
                BITS32(QSPIC, QSPIC_BURSTCMDB_REG, QSPIC_WRAP_LEN, len) |
                BITS32(QSPIC, QSPIC_BURSTCMDB_REG, QSPIC_WRAP_MD, 1);
}

void hw_qspi_set_dummy_bytes_count(uint8_t count)
{
        if (count == 3) {
                HW_QSPIC_REG_SETF(BURSTCMDB, DMY_FORCE, 1);
        } else {
                QSPIC->QSPIC_BURSTCMDB_REG =
                        (QSPIC->QSPIC_BURSTCMDB_REG &
                                ~(REG_MSK(QSPIC, QSPIC_BURSTCMDB_REG, QSPIC_DMY_FORCE) |
                                        REG_MSK(QSPIC, QSPIC_BURSTCMDB_REG, QSPIC_DMY_NUM))) |
                                        BITS32(QSPIC, QSPIC_BURSTCMDB_REG, QSPIC_DMY_NUM,
                                                dummy_num[count]);
        }
}

void hw_qspi_erase_block(uint32_t addr)
{
        if (!hw_qspi_get_automode()) {
                hw_qspi_set_automode(true);
        }
        // Wait for previous erase to end
        while (hw_qspi_get_erase_status() != 0) {
        }

        if (hw_qspi_get_address_size() == HW_QSPI_ADDR_SIZE_32) {
                addr >>= 12;
        } else {
                addr >>= 4;
        }
        // Setup erase block page
        HW_QSPIC_REG_SETF(ERASECTRL, ERS_ADDR, addr);
        // Fire erase
        HW_QSPIC_REG_SETF(ERASECTRL, ERASE_EN, 1);
}

void hw_qspi_set_pads(HW_QSPI_SLEW_RATE rate, HW_QSPI_DRIVE_CURRENT current)
{
        QSPIC->QSPIC_GP_REG =
                BITS16(QSPIC, QSPIC_GP_REG, QSPIC_PADS_SLEW, rate) |
                BITS16(QSPIC, QSPIC_GP_REG, QSPIC_PADS_DRV, current);
}

void hw_qspi_init(const qspi_config *cfg)
{
        hw_qspi_enable_clock();
        hw_qspi_set_automode(false);
        hw_qspi_set_bus_mode(HW_QSPI_BUS_MODE_SINGLE);
        hw_qspi_set_io2_output(true);
        hw_qspi_set_io2(1);
        hw_qspi_set_io3_output(true);
        hw_qspi_set_io3(1);

        if (cfg) {
                hw_qspi_set_address_size(cfg->address_size);
                hw_qspi_set_clock_mode(cfg->idle_clock);
                hw_qspi_set_read_sampling_edge(cfg->sampling_edge);
        }
}

#endif /* dg_configUSE_HW_QSPI */
/**
 * \}
 * \}
 * \}
 */
