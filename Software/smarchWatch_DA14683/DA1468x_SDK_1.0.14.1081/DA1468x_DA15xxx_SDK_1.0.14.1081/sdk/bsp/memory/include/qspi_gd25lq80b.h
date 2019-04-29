/**
 * \addtogroup BSP
 * \{
 * \addtogroup SYSTEM
 * \{
 * \addtogroup MEMORY
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file qspi_gd25lq80b.h
 *
 * @brief QSPI flash driver for the GigaDevice GD25LQ80B
 *
 * Copyright (C) 2016-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifndef _QSPI_GD25LQ80B_H_
#define _QSPI_GD25LQ80B_H_

#ifndef GIGADEVICE_ID
#define GIGADEVICE_ID  0xC8
#endif

#ifndef GD25LQ_SERIES
#define GD25LQ_SERIES  0x60
#endif

#define GD25LQ80B_SIZE 0x14

#if (FLASH_AUTODETECT == 1) || (dg_configFLASH_MANUFACTURER_ID == GIGADEVICE_ID && \
        dg_configFLASH_DEVICE_TYPE == GD25LQ_SERIES && \
        dg_configFLASH_DENSITY == GD25LQ80B_SIZE)

#include "qspi_common.h"

#include "qspi_gigadevice.h"

#if dg_configFLASH_POWER_OFF == 1
#if FLASH_AUTODETECT == 1
#pragma message "Please note that QSPI Flash GD25LQ80B will NOT work properly in FLASH_POWER_OFF mode"
#else
#error "QSPI Flash GD25LQ80B will NOT work properly in FLASH_POWER_OFF mode"
#endif
#endif

// Flash power up/down timings
#define GD25LQ80B_POWER_DOWN_DELAY_US          20

#define GD25LQ80B_RELEASE_POWER_DOWN_DELAY_US  20

#define GD25LQ80B_POWER_UP_DELAY_US            10000

#if (dg_configFLASH_POWER_DOWN == 1)
/**
 * \brief uCode for handling the QSPI FLASH release from power-down.
 */
        /*
         * 0x09   // CMD_NBYTES = 1, CMD_TX_MD = 0 (Single), CMD_VALID = 1
         * 0xD0   // CMD_WT_CNT_LS = 0xD0 --> 45000 / 62.5 = 720   // 45usec for worst case (MX25R3235F)
         * 0x02   // CMD_WT_CNT_MS = 0x02
         * 0xAB   // Release Power Down
         * (up to 16 words)
         */
        const uint32_t GD25LQ80B_ucode_wakeup[] = {
                0xAB000009 | (((uint16_t)(GD25LQ80B_RELEASE_POWER_DOWN_DELAY_US*1000/62.5) & 0xFFFF) << 8),
        };
#else
/**
 * \brief uCode for handling the QSPI FLASH exit from the "Continuous Read Mode".
 */
        /*
         * 0x45   // CMD_NBYTES = 8, CMD_TX_MD = 2 (Quad), CMD_VALID = 1
         * 0x00   // CMD_WT_CNT_LS = 0
         * 0x00   // CMD_WT_CNT_MS = 0
         * 0xFF
         * 0xFF
         * 0xFF
         * 0xFF
         * 0xFF
         * 0xFF
         * 0xFF
         * 0xFF
         */
        const uint32_t GD25LQ80B_ucode_wakeup[] = {
                0xFF000045,
                0x00FFFFFF,
        };
#endif


static void flash_gd25lq80b_initialize(uint8_t device_type, uint8_t device_density);
static void flash_gd25lq80b_sys_clock_cfg(sys_clk_t sys_clk);
static uint8_t flash_gd25lq80b_get_dymmy_bytes(void);

static const qspi_flash_config_t flash_gd25lq80b_config = {
        .manufacturer_id               = GIGADEVICE_ID,
        .device_type                   = GD25LQ_SERIES,
        .device_density                = GD25LQ80B_SIZE,
        .is_suspended                  = flash_gd_is_suspended,
        .initialize                    = flash_gd25lq80b_initialize,
        .deactivate_command_entry_mode = flash_gd_deactivate_command_entry_mode,
        .sys_clk_cfg                   = flash_gd25lq80b_sys_clock_cfg,
        .get_dummy_bytes               = flash_gd25lq80b_get_dymmy_bytes,
        .break_seq_size                = HW_QSPI_BREAK_SEQ_SIZE_1B,
        .address_size                  = HW_QSPI_ADDR_SIZE_24,
        .page_program_opcode           = CMD_QUAD_IO_PAGE_PROGRAM,
        .erase_opcode                  = CMD_SECTOR_ERASE,
        .erase_suspend_opcode          = GD_ERASE_PROGRAM_SUSPEND,
        .erase_resume_opcode           = GD_ERASE_PROGRAM_RESUME,
        .quad_page_program_address     = false,
        .read_erase_progress_opcode    = CMD_READ_STATUS_REGISTER,
        .erase_in_progress_bit         = FLASH_STATUS_BUSY_BIT,
        .erase_in_progress_bit_high_level = true,
#if GIGADEVICE_PERFORMANCE_MODE
        .send_once                     = 1,
        .extra_byte                    = 0x20,
#else
        .send_once                     = 0,
        .extra_byte                    = 0x00,
#endif
        .ucode_wakeup                  = {GD25LQ80B_ucode_wakeup, sizeof(GD25LQ80B_ucode_wakeup)},
        .power_down_delay              = GD25LQ80B_POWER_DOWN_DELAY_US,
        .release_power_down_delay      = GD25LQ80B_RELEASE_POWER_DOWN_DELAY_US,
};

#if (FLASH_AUTODETECT == 0)
        const qspi_flash_config_t * const flash_config = &flash_gd25lq80b_config;
#endif

__RETAINED_CODE static void flash_gd25lq80b_initialize(uint8_t device_type, uint8_t device_density)
{
        flash_activate_command_entry_mode();

        flash_gd_enable_quad_mode();

        flash_deactivate_command_entry_mode();
}

__RETAINED_CODE static void flash_gd25lq80b_sys_clock_cfg(sys_clk_t sys_clk)
{
}

__RETAINED_CODE static uint8_t flash_gd25lq80b_get_dymmy_bytes(void)
{
        return 2;
}
#endif

#endif /* _QSPI_GD25LQ80B_H_ */
/**
 * \}
 * \}
 * \}
 */
