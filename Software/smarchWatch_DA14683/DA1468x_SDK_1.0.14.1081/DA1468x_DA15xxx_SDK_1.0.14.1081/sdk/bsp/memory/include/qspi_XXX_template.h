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
 * @file qspi_XXX_template.h
 *
 * @brief Driver for flash XXXX
 *
 * Copyright (C) 2016-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef _QSPI_XXX_H_
#define _QSPI_XXX_H_

/**
 * \brief The Flash vendor JEDEC ID
 *
 * This is the first byte returned by the 0x9F command
 */
#define XXXXX_ID         0xFF

/**
 * \brief The Flash type JEDEC ID
 *
 * This is the second byte returned by the 0x9F command
 */
#define XXXXX            0xFF

/**
 * \brief The Flash density JEDEC ID
 *
 * This is the third byte returned by the 0x9F command
 */
#define XXXXX_SIZE       0xFF


#if (FLASH_AUTODETECT == 1) || (dg_configFLASH_MANUFACTURER_ID == XXXXX_ID && \
        dg_configFLASH_DEVICE_TYPE == XXXXX && \
        dg_configFLASH_DENSITY == XXXXX_SIZE)

#include "qspi_common.h"

// Custom command opcodes
#define XXX_READ_CUSTOM_CONFIG_REGISTER       0x00
#define XXX_WRITE_CUSTOM_CONFIG_REGISTER      0x10
#define XXX_PAGE_PROGRAM_OPCODE               CMD_QUAD_IO_PAGE_PROGRAM
#define XXX_ERASE_PROGRAM_SUSPEND             0xB0
#define XXX_ERASE_PROGRAM_RESUME              0x30

// Custom register bit flags
#define XXX_CUSTOM_CONFIG_SUS_BIT             5
#define XXX_CUSTOM_CONFIG_SUS_MASK            (1 << XXX_CUSTOM_CONFIG_SUS_BIT)

// Device type using command 0x9F
#define DEVICE_NAME1                          0x80
#define DEVICE_NAME2                          0x95

// Device density using command 0x9F
#define XXX_8Mb_SIZE                          0x10
#define XXX_256Mb_SIZE                        0x15

// Flash power up/down timings
#define XXX_POWER_DOWN_DELAY_US               10
#define XXX_RELEASE_POWER_DOWN_DELAY_US       30
#define XXX_POWER_UP_DELAY_US                 800

#if (dg_configFLASH_POWER_OFF == 1)
/**
 * \brief uCode for handling the QSPI FLASH activation from power off.
 */
        /*
         * Delay 10usec
         * 0x01   // CMD_NBYTES = 0, CMD_TX_MD = 0 (Single), CMD_VALID = 1
         * 0xA0   // CMD_WT_CNT_LS = 160 --> 10000 / 62.5 = 160 = 10usec
         * 0x00   // CMD_WT_CNT_MS = 0
         * Exit from Fast Read mode
         * 0x09   // CMD_NBYTES = 1, CMD_TX_MD = 0 (Single), CMD_VALID = 1
         * 0x00   // CMD_WT_CNT_LS = 0
         * 0x00   // CMD_WT_CNT_MS = 0
         * 0xFF   // Enable Reset
         * (up to 16 words)
         */
        const uint32_t XXX_ucode_wakeup[] = {
                0x09000001 | (((uint16_t)(XXX_POWER_UP_DELAY_US*1000/62.5) & 0xFFFF) << 8),
                0x00FF0000,
        };
#elif (dg_configFLASH_POWER_DOWN == 1)
/**
 * \brief uCode for handling the QSPI FLASH release from power-down.
 */
        /*
         * 0x09   // CMD_NBYTES = 1, CMD_TX_MD = 0 (Single), CMD_VALID = 1
         * 0x30   // CMD_WT_CNT_LS = 3000 / 62.5 = 48 // 3usec
         * 0x00   // CMD_WT_CNT_MS = 0
         * 0xAB   // Release Power Down
         * (up to 16 words)
         */
        const uint32_t XXX_ucode_wakeup[] = {
                0xAB000009 | (((uint16_t)(XXX_RELEASE_POWER_DOWN_DELAY_US*1000/62.5) & 0xFFFF) << 8),
        };
#else
/**
 * \brief uCode for handling the QSPI FLASH exit from the "Continuous Read Mode".
 */
        /*
         * 0x25   // CMD_NBYTES = 4, CMD_TX_MD = 2 (Quad), CMD_VALID = 1
         * 0x00   // CMD_WT_CNT_LS = 0
         * 0x00   // CMD_WT_CNT_MS = 0
         * 0x55   // Clocks 0-1 (A23-16)
         * 0x55   // Clocks 2-3 (A15-8)
         * 0x55   // Clocks 4-5 (A7-0)
         * 0x55   // Clocks 6-7 (M7-0) : M5-4 != '10' ==> Disable "Continuous Read Mode"
         * (up to 16 words)
         */
        const uint32_t XXX_ucode_wakeup[] = {
                0x55000025,
                0x00555555,
        };
#endif

/**
 * \brief This returns true if the flash is in write/erase suspend mode
 */
static bool flash_XXX_is_suspended(void);

/**
 * \brief This is called to initialize the flash
 */
static void flash_XXX_initialize(uint8_t device_type, uint8_t device_density);

/**
 * \brief This is called to put the flash OUT of command entry mode
 *
 * Usually, this is left empty, unless something special needs to be done
 * (Basic work is done by the central flash driver).
 */
static void flash_XXX_deactivate_command_entry_mode(void);

/**
 * \brief This is called each time the system clock is changed
 *
 * This can be used e.g. to change the qspi controller divider or modify the
 * flash dummy bytes, if e.g. the flash cannot cope with the higher clock
 * frequency.
 */
static void flash_XXX_sys_clock_cfg(sys_clk_t sys_clk);

/**
 * \brief This must return the number of dummy bytes required.
 *
 * In most cases, this can return a static value (usually 2). Sometimes,
 * however, the dummy bytes must change (e.g. according to the system clock
 * frequency).
 */
static uint8_t flash_XXX_get_dummy_bytes(void);

/**
 * \brief This structs configures the system for the specific flash
 *
 * \note This struct MUST be const for this to work. Therefore, assignments must
 *       not change (must be read-only)
 */
static const qspi_flash_config_t flash_XXX_config = {
        .manufacturer_id                  = XXXXX_ID,
        .device_type                      = XXXXX,
        .device_density                   = XXXXX_SIZE,
        .is_suspended                     = flash_XXX_is_suspended,
        .initialize                       = flash_XXX_initialize,
        .deactivate_command_entry_mode    = flash_XXX_deactivate_command_entry_mode,
        .sys_clk_cfg                      = flash_XXX_sys_clock_cfg,
        .get_dummy_bytes                  = flash_XXX_get_dummy_bytes,
        .break_seq_size                   = HW_QSPI_BREAK_SEQ_SIZE_1B,
        .address_size                     = HW_QSPI_ADDR_SIZE_24,
        .quad_page_program_address        = true,
        .erase_opcode                     = CMD_SECTOR_ERASE,
        .read_erase_progress_opcode       = CMD_READ_STATUS_REGISTER,
        .erase_suspend_opcode             = XXX_ERASE_PROGRAM_SUSPEND,
        .erase_resume_opcode              = XXX_ERASE_PROGRAM_RESUME,
        .page_program_opcode              = XXX_PAGE_PROGRAM_OPCODE,
        .erase_in_progress_bit            = FLASH_STATUS_BUSY_BIT,
        .erase_in_progress_bit_high_level = true,
        .send_once                        = 1,
        .extra_byte                       = 0xA0,
        .ucode_wakeup                     = {XXX_ucode_wakeup, sizeof(XXX_ucode_wakeup)},
        .power_down_delay                 = XXX_POWER_DOWN_DELAY_US,
        .release_power_down_delay         = XXX_RELEASE_POWER_DOWN_DELAY_US,
        .power_up_delay                   = XXX_POWER_UP_DELAY_US,
};

#if (FLASH_AUTODETECT == 0)
        const qspi_flash_config_t* const flash_config = &flash_XXX_config;
#endif

static inline uint8_t flash_XXX_read_custom_config_register(void) __attribute__((unused)) __attribute__((always_inline));
static inline uint8_t flash_XXX_read_custom_config_register(void)
{
        __DBG_QSPI_VOLATILE__ uint8_t status;
        uint8_t cmd[] = { XXX_READ_CUSTOM_CONFIG_REGISTER };

        qspi_transact(cmd, 1, &status, 1);

        return status;
}

static inline void flash_XXX_write_custom_config_register(uint8_t value) __attribute__((unused)) __attribute__((always_inline));
static inline void flash_XXX_write_custom_config_register(uint8_t value)
{
        __DBG_QSPI_VOLATILE__ uint8_t status;
        uint8_t cmd[] = { XXX_WRITE_CUSTOM_CONFIG_REGISTER, value };

        qspi_write(cmd, 2);

        /* Wait for the Flash to process the command */
        while (flash_is_busy());
}

__RETAINED_CODE static bool flash_XXX_is_suspended(void)
{
        __DBG_QSPI_VOLATILE__ uint8_t status;

        status = flash_XXX_read_custom_config_register();
        return (status & XXX_CUSTOM_CONFIG_SUS_MASK) != 0;
}

__RETAINED_CODE static void flash_XXX_initialize(uint8_t device_type, uint8_t device_density)
{
        /* Handle 24/32 bit addressing modes, enable QUAD mode, etc */
}

__RETAINED_CODE static void flash_XXX_deactivate_command_entry_mode(void)
{
        /* Implement extra steps required. e.g. Re-enable XIP mode */
}

__RETAINED_CODE static void flash_XXX_sys_clock_cfg(sys_clk_t sys_clk)
{
        /* Handle system clock frequency change */
        if (sys_clk == sysclk_PLL96) {
                hw_qspi_set_div(HW_QSPI_DIV_2);
        }
        else {
                hw_qspi_set_div(HW_QSPI_DIV_1);
        }
}

__RETAINED_CODE static uint8_t flash_XXX_get_dummy_bytes(void)
{
        /*
         * Either return a hardcoded value, or the value for the current mode of operation
         * (e.g. based on clock frequency)
         */

        return 2;
}
#endif

#endif /* _QSPI_XXX_H_ */
/**
 * \}
 * \}
 * \}
 */
