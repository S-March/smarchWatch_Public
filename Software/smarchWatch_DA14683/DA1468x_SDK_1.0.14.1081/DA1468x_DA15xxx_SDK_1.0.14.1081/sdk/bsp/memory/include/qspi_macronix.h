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
 * @file qspi_macronix.h
 *
 * @brief QSPI flash driver for Macronix flashes - common code
 *
 * Copyright (C) 2016-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifndef _QSPI_MACRONIX_H_
#define _QSPI_MACRONIX_H_

#define MACRONIX_ID                    0xC2

#if (FLASH_AUTODETECT == 1) || (dg_configFLASH_MANUFACTURER_ID == MACRONIX_ID)

#include "qspi_common.h"

#define MACRONIX_PERFORMANCE_MODE      1       // bootrom does not support Macronix performance mode

#define MX_ERASE_PROGRAM_SUSPEND       0xB0
#define MX_ERASE_PROGRAM_RESUME        0x30

#define MX_READ_SECURITY_REGISTER      0x2B
#define MX_READ_CONFIG_REGISTER        0x15

#define MX_STATUS_QE_BIT               6 // Quad Enable
#define MX_STATUS_QE_MASK              (1 << MX_STATUS_QE_BIT)

#define MX_STATUS_SRWD_BIT             7 // Status register write disable
#define MX_STATUS_SRWD_MASK            (1 << MX_STATUS_SRWD_BIT)

/* Suspend */
#define MX_SECURITY_ESB_BIT            3 // Erase suspend bit
#define MX_SECURITY_ESB_MASK           (1 << MX_SECURITY_ESB_BIT)

#define MX_SECURITY_PSB_BIT            2 // Program suspend bit
#define MX_SECURITY_PSB_MASK           (1 << MX_SECURITY_PSB_BIT)

#define MX_CONFIG_DC_BIT               6 // dummy cycle offset
#define MX_CONFIG_DC_MASK              (0x3 << MX_CONFIG_DC_BIT)

#define MX_CONFIG_ODS_BIT              0 // Output driver strength
#define MX_CONFIG_ODS_MASK             (0x7 << MX_CONFIG_ODS_BIT)

#define MX_CONFIG2_HIGH_PERFORMANCE_BIT               1 // Program suspend bit
#define MX_CONFIG2_HIGH_PERFORMANCE_MASK              (1 << MX_CONFIG2_HIGH_PERFORMANCE_BIT)

// Device type using command 0x9F
#define MX25L_SERIES                   0x20

enum flash_mx25u_ods {
        FLASH_MX25U_ODS_146OHM = 0,
        FLASH_MX25U_ODS_76OHM  = 1,
        FLASH_MX25U_ODS_52OHM  = 2,
        FLASH_MX25U_ODS_41OHM  = 3,
        FLASH_MX25U_ODS_34OHM  = 4,
        FLASH_MX25U_ODS_30OHM  = 5,
        FLASH_MX25U_ODS_26OHM  = 6,
        FLASH_MX25U_ODS_24OHM  = 7,
};

enum flash_mx25l_mx66u_ods {
        FLASH_MX25L_MX66U_ODS_90OHM = 1,
        FLASH_MX25L_MX66U_ODS_60OHM = 2,
        FLASH_MX25L_MX66U_ODS_45OHM = 3,
        FLASH_MX25L_MX66U_ODS_20OHM = 5,
        FLASH_MX25L_MX66U_ODS_15OHM = 6,
        FLASH_MX25L_MX66U_ODS_30OHM = 7,
};

__RETAINED_RW static uint8_t flash_mx_status_reg = MX_STATUS_SRWD_MASK; // This bit will be cleared when the Status register is read
__RETAINED static uint8_t flash_mx_conf_reg;

static inline uint8_t flash_mx_read_security_register(void) __attribute__((always_inline)) __attribute__((unused));
static inline uint8_t flash_mx_read_security_register(void)
{
        __DBG_QSPI_VOLATILE__ uint8_t status;
        uint8_t cmd[] = { MX_READ_SECURITY_REGISTER };

        qspi_transact(cmd, 1, &status, 1);

        return status;
}

static inline void flash_mx_enable_quad_mode(void) __attribute__((always_inline));
static inline void flash_mx_enable_quad_mode(void)
{
        uint8_t status;

        status = flash_read_status_register();
        if (!(status & MX_STATUS_QE_MASK)) {
                flash_write_enable();
                flash_write_status_register(status | MX_STATUS_QE_MASK);
        }
}

static inline void flash_mx_set_output_driver_strength(uint8_t ods_value) __attribute__((always_inline)) __attribute__((unused));
static inline void flash_mx_set_output_driver_strength(uint8_t ods_value)
{
        uint8_t cmd[] = { MX_READ_CONFIG_REGISTER };
        uint8_t status_reg, conf_reg;
        uint8_t new_value;

        ASSERT_ERROR((ods_value & ~MX_CONFIG_ODS_MASK) == 0);

        status_reg = flash_read_status_register();  // This will be used when setting the dummy bytes.
        qspi_transact(cmd, 1, &conf_reg, 1);        // This will be used when setting the dummy bytes.

        new_value = (conf_reg & ~MX_CONFIG_ODS_MASK) | (ods_value << MX_CONFIG_ODS_BIT);
        uint8_t wr_cmd[]= {CMD_WRITE_STATUS_REGISTER, status_reg, new_value };

        flash_write_enable();
        qspi_write(wr_cmd, 3);

        while (flash_is_busy());
}

static inline void flash_mx_configure_dummy_cycles(uint8_t dummy_cycles) __attribute__((always_inline));
static inline void flash_mx_configure_dummy_cycles(uint8_t dummy_cycles)
{
        uint8_t new_value;
        uint8_t dc_value;

        ASSERT_ERROR(dummy_cycles==4 || dummy_cycles==6 || dummy_cycles==8 || dummy_cycles==10);
        ASSERT_ERROR(flash_mx_status_reg != MX_STATUS_SRWD_MASK); // flash_mx_status_reg must be read from device and SRWD must be 0

        switch(dummy_cycles) {
        case 4:
                dc_value = 1;
                break;
        case 8:
                dc_value = 2;
                break;
        case 10:
                dc_value = 3;
                break;
        case 6:
        default:
                dc_value = 0;
                break;
        }

        new_value = (flash_mx_conf_reg & ~MX_CONFIG_DC_MASK) | (dc_value << MX_CONFIG_DC_BIT);
        uint8_t wr_cmd[]= {CMD_WRITE_STATUS_REGISTER, flash_mx_status_reg, new_value };

        flash_activate_command_entry_mode();
        flash_write_enable();
        qspi_write(wr_cmd, 3);
        while (flash_is_busy());
        flash_deactivate_command_entry_mode();
}

static inline void flash_mx_set_high_performance()  __attribute__((always_inline));
static inline void flash_mx_set_high_performance()
{
        uint8_t cmd[] = { MX_READ_CONFIG_REGISTER };
        uint8_t conf_reg[2];
        uint8_t new_value;
        uint8_t status;

        status = flash_read_status_register();
        qspi_transact(cmd, 1, conf_reg, 2);

        new_value = conf_reg[1] | MX_CONFIG2_HIGH_PERFORMANCE_MASK;
        uint8_t wr_cmd[] = {CMD_WRITE_STATUS_REGISTER, status, conf_reg[0], new_value };

        flash_write_enable();
        qspi_write(wr_cmd, 4);

        while (flash_is_busy());
}

__RETAINED_CODE static bool flash_mx_is_suspended(void)
{
        __DBG_QSPI_VOLATILE__ uint8_t status;

        status = flash_mx_read_security_register();
        return (status & (MX_SECURITY_ESB_MASK | MX_SECURITY_PSB_MASK)) != 0;
}


__RETAINED_CODE static void flash_mx_deactivate_command_entry_mode(void)
{

}
#endif

#endif /* _QSPI_MACRONIX_H_ */
/**
 * \}
 * \}
 * \}
 */
