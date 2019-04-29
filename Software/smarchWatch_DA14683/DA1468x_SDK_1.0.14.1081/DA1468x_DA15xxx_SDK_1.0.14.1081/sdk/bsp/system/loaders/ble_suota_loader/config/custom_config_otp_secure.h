/**
 ****************************************************************************************
 *
 * @file custom_config_otp_secure.h
 *
 * @brief Custom configuration file for secure bootloader executing from OTP.
 *
 * Copyright (C) 2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef CUSTOM_CONFIG_OTP_H_
#define CUSTOM_CONFIG_OTP_H_

#include "bsp_definitions.h"

#define OS_BAREMETAL

/* Choose UART that will be used in loader 0 - no UART, 1 or 2 */
#define LOADER_UART                             2
#if LOADER_UART
        #define CONFIG_RETARGET
        #if LOADER_UART == 2
                #define CONFIG_RETARGET_UART HW_UART2
        #elif LOADER_UART == 1
                #define CONFIG_RETARGET_UART HW_UART1
        #else
                #error "Invalid LOADER_UART configuration!"
        #endif
#endif

#define dg_configUSE_LP_CLK                     LP_CLK_32768
#define dg_configEXEC_MODE                      MODE_IS_MIRRORED
#define dg_configCODE_LOCATION                  NON_VOLATILE_IS_NONE
#define dg_configEXT_CRYSTAL_FREQ               EXT_CRYSTAL_IS_16M

#define dg_configIMAGE_SETUP                    DEVELOPMENT_MODE
#define dg_configEMULATE_OTP_COPY               (0)

#define dg_configUSER_CAN_USE_TIMER1            (1)

#define dg_configUSE_WDOG                       (0)

#define dg_configUSE_DCDC                       (1)

#define dg_configFLASH_CONNECTED_TO             (FLASH_CONNECTED_TO_1V8)
#define dg_configFLASH_POWER_DOWN               (0)

#define dg_configPOWER_1V8_ACTIVE               (1)
#define dg_configPOWER_1V8_SLEEP                (1)

#define dg_configBATTERY_TYPE                   (BATTERY_TYPE_LIMN2O4)
#define dg_configBATTERY_CHARGE_CURRENT         2       // 30mA
#define dg_configBATTERY_PRECHARGE_CURRENT      20      // 2.1mA
#define dg_configBATTERY_CHARGE_NTC             1       // disabled

#define dg_configUSE_USB                        0
#define dg_configUSE_USB_CHARGER                0
#define dg_configALLOW_CHARGING_NOT_ENUM        1

#define dg_configUSE_ProDK                      (1)

#define dg_configUSE_SW_CURSOR                  (0)

#define dg_configNVMS_ADAPTER                   1
#define dg_configNVMS_ADAPTER                   1
#define dg_configCRYPTO_ADAPTER                 0
#define dg_configFLASH_ADAPTER                  1
#define dg_configNVMS_VES                       0
#define dg_configUSE_HW_AES_HASH                1

#define dg_configDISABLE_BACKGROUND_FLASH_OPS   (1)
#define __HEAP_SIZE                             0x0800

#define dg_configDEBUG_TRACE                    0

/* Include bsp default values */
#include "bsp_defaults.h"
/* Include memory layout */
#include "bsp_memory_layout.h"

#endif /* CUSTOM_CONFIG_OTP_H_ */
