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
 * @file qspi_mx25u51245.h
 *
 * @brief QSPI flash driver for the Macronix MX25U51245
 *
 * Copyright (C) 2016-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifndef _QSPI_MX25U51245_H_
#define _QSPI_MX25U51245_H_

#ifndef MACRONIX_ID
#define MACRONIX_ID         0xC2
#endif

#ifndef MX25U_MX66U_SERIES
#define MX25U_MX66U_SERIES  0x25
#endif

#define MX25U51245_SIZE     0x3A  // MX66U512 and MX25U512 have the same JEDEC IDs but different register and timing characteristics

#if (FLASH_AUTODETECT == 1) || (dg_configFLASH_MANUFACTURER_ID == MACRONIX_ID && \
        dg_configFLASH_DEVICE_TYPE == MX25U_MX66U_SERIES && \
        dg_configFLASH_DENSITY == MX25U51245_SIZE)

#include "qspi_common.h"

#include "qspi_macronix.h"

#if dg_configFLASH_POWER_OFF == 1
#if FLASH_AUTODETECT == 1
#pragma message "Please note that QSPI Flash MX25U51245 will NOT work properly in FLASH_POWER_OFF mode"
#else
#error "QSPI Flash MX25U51245 will NOT work properly in FLASH_POWER_OFF mode"
#endif
#endif

// Flash power up/down timings
#define MX25U51245_POWER_DOWN_DELAY_US          10

#define MX25U51245_RELEASE_POWER_DOWN_DELAY_US  30

#define MX25U51245_POWER_UP_DELAY_US            3000

#define MX25U51245_DUMMY_BYTES_PLL_96           3
#define MX25U51245_DUMMY_BYTES_NORMAL           2

#if (dg_configFLASH_POWER_OFF == 1)
/**
 * \brief uCode for handling the QSPI FLASH activation from power off.
 */
        /*
         * Delay 3000usec
         * 0x01   // CMD_NBYTES = 0, CMD_TX_MD = 0 (Single), CMD_VALID = 1
         * 0x80   // CMD_WT_CNT_LS = 0x80 --> 3000000 / 62.5 = 48000 // 3000usec
         * 0xBB   // CMD_WT_CNT_MS = 0xBB
         * Exit from Fast Read mode
         * 0x11   // CMD_NBYTES = 2, CMD_TX_MD = 0 (Single), CMD_VALID = 1
         * 0x00   // CMD_WT_CNT_LS = 0
         * 0x00   // CMD_WT_CNT_MS = 0
         * 0xFF   // Enable Reset
         * 0xFF   // Enable Reset
         * (up to 16 words)
         */
        const uint32_t mx25u51245_ucode_wakeup[] = {
                0x11000001 | (((uint16_t)(MX25U51245_POWER_UP_DELAY_US*1000/62.5) & 0xFFFF) << 8),
                0xFFFF0000,
        };
#elif (dg_configFLASH_POWER_DOWN == 1)
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
        const uint32_t mx25u51245_ucode_wakeup[] = {
                0xAB000009 | (((uint16_t)(MX25U51245_RELEASE_POWER_DOWN_DELAY_US*1000/62.5) & 0xFFFF) << 8),
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
        const uint32_t mx25u51245_ucode_wakeup[] = {
                0xFF000025,
                0x00FFFFFF,
        };

        const uint32_t mx25u51245_ucode_wakeup_32bit_addressing[] = {
                0xFF000045,
                0xFFFFFFFF,
                0x00FFFFFF,
        };

#endif

static void flash_mx25u51245_initialize(uint8_t device_type, uint8_t device_density);
static void flash_mx25u51245_sys_clock_cfg(sys_clk_t sys_clk);
static uint8_t flash_mx25u51245_get_dummy_bytes(void);

static const qspi_flash_config_t flash_mx25u51245_config = {
        .manufacturer_id               = MACRONIX_ID,
        .device_type                   = MX25U_MX66U_SERIES,
        .device_density                = MX25U51245_SIZE,
        .is_suspended                  = flash_mx_is_suspended,
        .initialize                    = flash_mx25u51245_initialize,
        .deactivate_command_entry_mode = flash_mx_deactivate_command_entry_mode,
        .sys_clk_cfg                   = flash_mx25u51245_sys_clock_cfg,
        .get_dummy_bytes               = flash_mx25u51245_get_dummy_bytes,
#if FLASH_FORCE_24BIT_ADDRESSING
        .page_program_opcode           = CMD_QUAD_IO_PAGE_PROGRAM,
        .erase_opcode                  = CMD_SECTOR_ERASE,
        .break_seq_size                = HW_QSPI_BREAK_SEQ_SIZE_1B,
        .address_size                  = HW_QSPI_ADDR_SIZE_24,
#else
        .page_program_opcode           = CMD_QUAD_IO_PAGE_PROGRAM_4B,
        .erase_opcode                  = CMD_SECTOR_ERASE_4B,
        .break_seq_size                = HW_QSPI_BREAK_SEQ_SIZE_2B,
        .address_size                  = HW_QSPI_ADDR_SIZE_32,
#endif
        .erase_suspend_opcode          = MX_ERASE_PROGRAM_SUSPEND,
        .erase_resume_opcode           = MX_ERASE_PROGRAM_RESUME,
        .quad_page_program_address     = true,
        .read_erase_progress_opcode    = CMD_READ_STATUS_REGISTER,
        .erase_in_progress_bit         = FLASH_STATUS_BUSY_BIT,
        .erase_in_progress_bit_high_level = true,
#if MACRONIX_PERFORMANCE_MODE
        .send_once                     = 1,
        .extra_byte                    = 0xA5,
#else
        .send_once                     = 0,
        .extra_byte                    = 0x00,
#endif
#if FLASH_FORCE_24BIT_ADDRESSING
        .ucode_wakeup                  = {mx25u51245_ucode_wakeup, sizeof(mx25u51245_ucode_wakeup)},
#else
#if (dg_configFLASH_POWER_OFF == 0) && (dg_configFLASH_POWER_DOWN==0)
        .ucode_wakeup                  = {mx25u51245_ucode_wakeup_32bit_addressing, sizeof(mx25u51245_ucode_wakeup_32bit_addressing)},
#else
        .ucode_wakeup                  = {mx25u51245_ucode_wakeup, sizeof(mx25u51245_ucode_wakeup)},
#endif
#endif

        .power_down_delay              = MX25U51245_POWER_DOWN_DELAY_US,
        .release_power_down_delay      = MX25U51245_RELEASE_POWER_DOWN_DELAY_US,
};

#if (FLASH_AUTODETECT == 0)
        const qspi_flash_config_t* const flash_config = &flash_mx25u51245_config;
#endif

__RETAINED_RW uint8_t flash_mx25u51245_dummy_bytes = MX25U51245_DUMMY_BYTES_NORMAL;

__RETAINED_CODE static void flash_mx25u51245_initialize(uint8_t device_type, uint8_t device_density)
{
        uint8_t cmd[] = { MX_READ_CONFIG_REGISTER };

        flash_activate_command_entry_mode();
        flash_mx_enable_quad_mode();
        flash_mx_status_reg = flash_read_status_register();  // This will be used when setting the dummy bytes.
        qspi_transact(cmd, 1, &flash_mx_conf_reg, 1);        // This will be used when setting the dummy bytes.
        flash_deactivate_command_entry_mode();
}

__RETAINED_CODE static uint8_t flash_mx25u51245_get_dummy_bytes()
{
        return flash_mx25u51245_dummy_bytes;
}

__RETAINED_CODE static void flash_mx25u51245_sys_clock_cfg(sys_clk_t sys_clk)
{
        if(sys_clk == sysclk_PLL96) {
                flash_mx_configure_dummy_cycles(8); // Three dummy bytes + one extra byte to support 96MHz CPU clock
                flash_mx25u51245_dummy_bytes = MX25U51245_DUMMY_BYTES_PLL_96;
        }
        else {
                flash_mx_configure_dummy_cycles(6); // Two dummy bytes + one extra byte
                flash_mx25u51245_dummy_bytes = MX25U51245_DUMMY_BYTES_NORMAL;
        }

        qspi_automode_set_dummy_bytes_count(flash_mx25u51245_dummy_bytes);
}
#endif

#endif /* _QSPI_MACRONIX_H_ */
/**
 * \}
 * \}
 * \}
 */
